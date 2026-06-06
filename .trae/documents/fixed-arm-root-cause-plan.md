# 固定手臂被破坏根因与彻底治理方案

## Summary

* 目标：解释为什么当前工程里“固定手臂（Fixed Arm）”会被破坏，并给出不靠局部补丁、能够长期稳定落地的彻底治理方案。

* 成功标准：

  * `task.arm` 只在任务创建时确定，之后不再被运行时逻辑改写。

  * PM2 / LLA / LLB / WTR 的所有取放、交换、立即补取决策都只以 `task.arm` 为调度语义来源。

  * 物理手臂实时状态（A/B 当前是否持片）与任务绑定手臂解耦，不再通过改 `task.arm` 修正现场。

  * 遇到“物理状态与任务语义不一致”时，系统进入显式等待/告警路径，而不是换臂或覆写任务。

  * 正常时序下可持续运行；异常时不死锁、不偷偷换臂、不继续污染任务语义。

## Current State Analysis

### 1. `task.arm` 的设计语义本来是“任务创建时绑定一次”

* 任务结构定义在 [UnifiedWaferTask.h](file:///d:/HLPrj/HL/device/include/UnifiedWaferTask.h#L12-L79)。

* `arm` 字段是任务的一部分，语义是“该片绑定的机械手臂”：[UnifiedWaferTask.h:L66](file:///d:/HLPrj/HL/device/include/UnifiedWaferTask.h#L66-L66)。

* 当前显式初始化 `task.arm` 的主入口在 [slot\_transfer\_cycle\_vtm\_widget.cpp:L10677-L10687](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp#L10677-L10687)：

  * UI 创建任务时根据 A/B 选择写入 `task.arm = 0/1`。

* 这说明 `task.arm` 的原始设计应是“建任务时确定，后续沿任务生命周期传递”。

### 2. 运行时可改写 `task.arm` 的后门仍然存在

* `TaskManager` 提供了直接改手臂的接口：[TaskManager.h](file:///d:/HLPrj/HL/device/include/TaskManager.h#L194-L194)、[TaskManager.cpp:L162-L176](file:///d:/HLPrj/HL/device/src/TaskManager.cpp#L162-L176)。

* 该接口没有语义约束，也没有区分“初始化赋值”和“运行时纠偏”，只要拿到 `taskId` 就能覆写 `task.arm`。

* 这意味着系统层面对“固定手臂不可变”没有制度化保护，任何流程代码都可以破坏这一约束。

### 3. 历史代码确实通过运行时改 `task.arm` 来追随现场物理状态

* 旧版 LLA 逻辑中存在“回片后立即补取，若 pending task 不是空闲手臂，则动态改成空闲手臂”：

  * [slot\_transfer\_cycle\_vtm\_widget.cpp.bak:L3428-L3434](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp.bak#L3428-L3434)

* 旧版 LLA 还存在“目标手臂被占用则切换到另一只空闲手臂，并覆写任务 arm”：

  * [slot\_transfer\_cycle\_vtm\_widget.cpp.bak:L3473-L3482](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp.bak#L3473-L3482)

* 旧版 LLB 存在同样逻辑：

  * [slot\_transfer\_cycle\_vtm\_widget.cpp.bak:L4954-L4959](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp.bak#L4954-L4959)

  * [slot\_transfer\_cycle\_vtm\_widget.cpp.bak:L4999-L5008](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp.bak#L4999-L5008)

* 这些旧逻辑说明：固定手臂被破坏不是偶发 bug，而是历史上明确存在过“为了跑通现场，动态换臂并回写任务”的调度策略。

### 4. PM2 路径后来又形成了第二类破坏：用“物理状态”替代“任务语义”

* 当前 PM2 主调度在 [slot\_transfer\_cycle\_vtm\_widget.cpp:L7305-L7488](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp#L7305-L7488)。

* 这里同时读取：

  * 物理状态：`wtr->hasObject(0/1)`、PM 腔体是否有片。

  * 任务状态：`pendingTasks / completedTasks / returnPendingTasks`。

* 当前版本虽然已比旧版收紧，但仍保留“运行时再做匹配/校验”的结构，这说明系统仍在用“现场物理手臂占用”参与决策，而不只是单纯执行 `task.arm`。

* 特别是在 PM2 交换成功后，当前仍调用 [syncPm2ReturnTaskArm](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp#L887-L925) 检查“实际取片 arm 是否和任务 arm 一致”。这本质上是在弥补“交换决策可能脱离任务 arm”的后果，而不是从源头消除该问题。

### 5. LL 立即补取和 PM2 交换本质上共用一个根因

* 当前 LLA/LLB 的立即补取已经改成“occupiedArm 对应 return task + 同 arm pending task”：

  * [LLA](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp#L4049-L4078)

  * [LLB](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp#L5691-L5719)

* 这说明系统已经认识到：立即补取本质上应是“回片手臂继续补取同 arm 任务”。

* PM2 交换本质也是同一问题：必须用“完成片绑定 arm”和“待加工片绑定 arm”配对，而不是根据哪只手现在空/有片去动态换臂。

### 6. 根因总结

* 根因不是单点 if 判断错误，而是三层架构问题叠加：

  * **数据模型问题**：`task.arm` 被设计成可变字段，缺少不可变约束。

  * **职责边界问题**：调度层把“任务语义”和“现场恢复/纠偏”混在一起，用改 arm 的方式追现场。

  * **决策来源问题**：PM/LL/WTR 多处同时参考“任务 arm”和“物理手臂状态”，造成双重真相（two sources of truth）。

## Proposed Changes

### A. 将 `task.arm` 升级为不可变调度语义

**文件**

* [UnifiedWaferTask.h](file:///d:/HLPrj/HL/device/include/UnifiedWaferTask.h)

* [TaskManager.h](file:///d:/HLPrj/HL/device/include/TaskManager.h)

* [TaskManager.cpp](file:///d:/HLPrj/HL/device/src/TaskManager.cpp)

**改动**

* 明确把 `UnifiedWaferTask::arm` 定义为“任务绑定手臂（binding arm）”，只允许在任务创建时赋值。

* 删除或禁用 `TaskManager::updateTaskArm(...)` 的运行时改写能力。

* 若兼容期内必须保留接口，则改为：

  * 仅记录错误日志并拒绝执行；

  * 或仅允许在任务尚未进入 `LOADLOCK_TRANSFER/PM_PROCESS/LOADLOCK_RETURN` 生命周期前调用。

**原因**

* 只要 `updateTaskArm` 还存在，系统就永远有“为了跑通现场而改 arm”的退路，固定手臂不可能真正落地。

### B. 把“物理状态”从“任务语义”里彻底解耦

**文件**

* [UnifiedWaferTask.h](file:///d:/HLPrj/HL/device/include/UnifiedWaferTask.h)

* [CycleStateSnapshot.cpp](file:///d:/HLPrj/HL/device/src/CycleStateSnapshot.cpp)（如需序列化新增字段）

* [slot\_transfer\_cycle\_vtm\_widget.cpp](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp)

**改动**

* 不再用 `task.arm` 承担“当前实际在哪只手上”的含义。

* 如确有现场追踪需求，新增独立字段或临时运行态结构，例如：

  * `expectedArm` / `bindingArm`：任务绑定手臂，创建后不可变。

  * `runtimeCarrierArm`：仅用于调试或状态快照，表示当前物理载片手臂，可为空。

* 快照恢复也必须区分“任务绑定 arm”和“物理运行态 arm”，禁止恢复流程时顺手覆盖绑定 arm。

**原因**

* 当前最大的问题是把“这片应由谁做”与“这片此刻在哪只手上”混成一个字段，导致任何纠偏都在污染调度语义。

### C. 重写 PM2 调度为“任务驱动”，不再以物理手臂状态决定 arm

**文件**

* [slot\_transfer\_cycle\_vtm\_widget.cpp](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp)

**改动**

* 重构 PM2 `step 200` 的决策逻辑，只允许以下路径：

  * **上料**：从 `pm2Snapshot.pendingTasks` 中选第一条符合当前时序的任务，其 `task.arm` 决定走 `1010` 还是 `1030`。

  * **下料**：从 `returnPending/completed` 中选当前 PM2 对应任务，其 `task.arm` 决定走 `1040` 还是 `1050`。

  * **交换**：必须先显式选出一对任务：

    * 一条“PM2 当前完成片/待回片任务”

    * 一条“下一片待加工任务”

    * 两条任务的 arm 必须正好构成 `{A取,B放}` 或 `{B取,A放}` 才允许进入 `1060/1070`

  * 否则一律等待，不进入交换。

* 去掉当前“先看手上哪只 arm 有 pending，再反推交换方向”的结构。

* `1065/1075` 不再需要 `syncPm2ReturnTaskArm()` 做事后补救；交换方向在发起前就应已由任务对配准保证正确。

**原因**

* 彻底解决固定手臂问题的关键不是在成功后检查，而是在发起动作前保证“任务对 + arm 对”已经闭环。

### D. 重写 LL 立即补取为“任务对配准”，不再参考空闲手臂做改 arm 决策

**文件**

* [slot\_transfer\_cycle\_vtm\_widget.cpp](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp)

**改动**

* 把 LLA/LLB 立即补取逻辑抽成统一函数（例如 `planImmediateRepick(loadLockName, snapshot, armAHasWafer, armBHasWafer)`）。

* 规则固定为：

  * 先找“当前持片手臂对应的 return task”

  * 再找“同 arm 的下一条 pending task”

  * 找不到就等待，不允许把 pending task 改给另一只手

* 删除所有历史“空手优先”“另一只手空就换 arm”的残余思路。

**原因**

* 这部分是固定手臂最容易被破坏的高频路径，也是历史上 `updateTaskArm(...)` 真正被使用过的地方。

### E. 为 WTR/TM 增加“语义校验层”，但不承担纠偏职责

**文件**

* [slot\_transfer\_cycle\_vtm\_widget.cpp](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp)

* 如有必要，可扩展到 WTR 命令封装层

**改动**

* Robot 执行前继续保留最终校验，但校验只负责：

  * 判断目标手臂当前是否满足执行条件；

  * 不满足时返回“等待/请求取消/错误”；

  * 不得重选 arm。

* 调度层必须根据“当前要执行的 task.arm”决定原请求步骤重试还是等待，不允许在 Robot 层换臂。

**原因**

* Robot/TM 层应该是“执行守门员”，不是“调度纠偏器”。

### F. 用显式异常状态代替“继续跑 + 事后同步 arm”

**文件**

* [slot\_transfer\_cycle\_vtm\_widget.cpp](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp)

* 视需要扩展 [UnifiedWaferTask.h](file:///d:/HLPrj/HL/device/include/UnifiedWaferTask.h)

**改动**

* 遇到以下情形时，进入明确异常分支：

  * PM2 交换前无法找到 arm 配对一致的任务对；

  * LL 立即补取找不到同 arm 的 pending task；

  * Robot 执行前发现“请求 arm 当前已不满足执行条件”且该任务不允许换臂。

* 异常分支行为应是：

  * 记录结构化日志（taskId / expectedArm / actualArm / step / pm/ll name）

  * 回到稳定等待点或进入错误态

  * 不改 `task.arm`

**原因**

* 固定手臂的真正落地不是“永远不停机”，而是“正常路径持续运行，异常路径显式暴露且不污染语义”。

## Assumptions & Decisions

* 决策 1：`task.arm` 是任务绑定语义，不是现场纠偏变量。

* 决策 2：不再接受“为了继续跑而改 task.arm”。

* 决策 3：PM2、LLA、LLB 的交换/立即补取都统一采用“任务对配准”而非“物理手臂占用驱动”。

* 决策 4：遇到固定手臂语义已被破坏的现场，系统允许等待/报警，但不允许偷偷换臂继续跑。

* 假设 1：PM2 当前主瓶颈与最典型破坏路径一致，是该轮治理的主收口点。

* 假设 2：`流程调度时序.txt` 代表目标时序，且其前提是每片任务从创建到回 LL 全程固定 arm。

## Verification Steps

### 1. 静态验证

* 确认全工程不再有任何运行时改 `task.arm` 的可执行入口：

  * `rg -n "updateTaskArm\\(" device/src device/include`

  * `rg -n "task\\.arm\\s*=" device/src device/include`

* 确认 PM2 / LLA / LLB 决策代码只读 `task.arm`，不写 `task.arm`。

### 2. 时序验证

* 以 [流程调度时序.txt](file:///d:/HLPrj/HL/流程调度时序.txt) 为验收基线，逐项核对：

  * `task0` 完成后必须 `A取PM2 + B放task1`

  * `task1` 完成后必须 `B取PM2 + A放task2`

  * `task2/task3/task4` 继续按文档交替

* 输出 WTR 时序对照表：

  * 序号

  * 期望步骤

  * 实际步骤（日志行号）

  * 偏差结论

### 3. 异常验证

* 人工制造以下场景，验证系统不换臂：

  * `task.arm=A`，但 A 臂执行前被占用、B 臂空闲

  * PM2 完成片 arm 与待加工片 arm 无法构成合法交换对

  * LL 回片后找不到同 arm 的下一片 pending task

* 期望结果：

  * 不调用 `updateTaskArm`

  * 不发错误交换/取放请求

  * 不死锁

  * 明确等待/报警

### 4. 回归验证

* 验证以下正常路径不受影响：

  * LLA/LLB 常规上料

  * PM2 正常交互料

  * PM2 最后一片完成后取回 LL

  * LL 下料到 EFEM

