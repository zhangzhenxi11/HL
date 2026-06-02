# LL发送前手指占用与PM2优先级联动修改计划

## Summary

本次修改目标不是单纯补“手指有没有片”的判断，而是同时解决两类问题：

- `LLA/LLB` 在发送 `robot_get_from_xxx` 请求前，没有识别“目标手指已经被别的 in-flight 请求占用”
- `LLA/LLB` 线程没有像 `PM2 step 200` 那样判断“当前应该让 PM2 / 回片优先，LL 两边都先不要再动”

也就是说，这次要把 `LL` 线程的发送前判断升级成两层：

- `手指资源层`：实际持片 + 逻辑占用 + in-flight 执行占用
- `全局调度层`：当 `PM2` 当前状态要求“先等 PM2 完成/先回片/先从 PM2 取片”时，`LL` 线程不得再发新的 LL 取片请求

## Current State Analysis

### 1. 现场问题不是单纯“手指已持片”

- 在 [2026-06-02_any_cleaned.txt:L33270-L33274](file:///d:/HLPrj/HL/_collected_latest_20260602_103605/2026-06-02_any_cleaned.txt#L33270-L33274)，`LLA` 已于 `10:30:22` 发起 `A 手从 LLA 取片`
- 在 [2026-06-02_any_cleaned.txt:L33283-L33292](file:///d:/HLPrj/HL/_collected_latest_20260602_103605/2026-06-02_any_cleaned.txt#L33283-L33292)，`LLB` 又在 `10:30:23` 发起 `LLB回片后立即补取一片, arm=0`
- 到 [2026-06-02_any_cleaned.txt:L33310-L33323](file:///d:/HLPrj/HL/_collected_latest_20260602_103605/2026-06-02_any_cleaned.txt#L33310-L33323)，`A 手` 真正落成 `has` 后，Robot 线程继续执行这笔 `LLB arm=0` 请求，于是底层报 `0x3000 手臂已有片`

结论：

- 当前错误不是“底层乱动”
- 而是上层在 `A 手` 已经被占用但尚未反映到 `hasObject()` 时，又错误地下发了第二笔 `A 手` 请求

### 2. 当前 `LL` 线程发送点的检查不够

#### 2.1 `LLA/LLB` 立即补取发送点

- `LLA` 立即补取发送在 [slot_transfer_cycle_vtm_widget.cpp:L4040-L4072](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp#L4040-L4072)
- `LLB` 立即补取发送在 [slot_transfer_cycle_vtm_widget.cpp:L5578-L5611](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp#L5578-L5611)

这两处当前只检查：

- `tmDoor` 是否打开
- `wtr->hasObject(targetArm)` 是否为真

缺少：

- 目标手指是否已有别的 `robot_get/put/exchange` 请求挂起
- Robot 线程是否正在执行会占用该手指的动作
- 当前全局是否处于“应让 PM2/回片优先”的窗口

#### 2.2 `LLA/LLB step 1051` 普通取片发送点

- 普通从 `LLA/LLB` 发取片请求的核心入口在 `step 1051`
- 这部分虽然已有“PM2 有片时回退到 `950`”的保护日志：
  - [slot_transfer_cycle_vtm_widget.cpp:L3511](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp#L3511)
  - [slot_transfer_cycle_vtm_widget.cpp:L5048](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp#L5048)
- 但这类保护还不够完整，尤其没有和 `PM2 step 200` 的全局判断完全对齐

### 3. `PM2 step 200` 已经有更完整的全局态快照

- 在 [slot_transfer_cycle_vtm_widget.cpp:L6600-L6608](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp#L6600-L6608)，`PM2 step 200` 已经输出完整快照：
  - `haswaferpm`
  - `armA_has`
  - `armA_pending`
  - `armB_has`
  - `armB_pending`
  - `pending`
  - `return_pending`

这说明 `PM2` 线程已经具备“此刻应该谁动、谁该等”的判断基础。

你指出的问题本质上就是：

- `PM2` 线程知道当前应该先等 `PM2`
- 但 `LL` 线程在自己的发送点没有复用这套判断
- 于是 `LLB` 仍然能在错误时机发出“立即补取”请求

### 4. 可参考的已有“实际状态检测”步骤

- [slot_transfer_cycle_vtm_widget.cpp:L3886-L3922](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp#L3886-L3922) 的 `2055` 已经体现出“执行前先查实际手指状态”的思路
- 本次计划会保留这个思路，但把它前移到“发送请求之前”，并且加上“全局 PM2 优先级判断”

## Proposed Changes

### 1. `device/src/slot_transfer_cycle_vtm_widget.cpp`

本次修改只聚焦这个文件。

### 1.1 新增统一的手指占用判定辅助函数

新增一个辅助函数，用于所有 `LLA/LLB` 发送 `robot_get_from_xxx` 前调用。

目标：

- 判断目标 `arm` 是否真的可用
- 不只看 `wtr->hasObject(arm)`
- 还要看：
  - `robot_get_from_lla/llb` 是否已有同 arm 请求
  - `robot_put_to_lla/llb` 是否已有同 arm 请求
  - `robot_get_from_pm1~4` / `robot_put_to_pm1~4` / `robot_exchange_pm1~4` 是否占用同 arm
  - 当前 `robot_step` 是否正处于执行同 arm 的窗口

行为：

- 若 arm 在逻辑上已被占用，则禁止发送
- 返回可打印的阻塞原因，供日志使用

### 1.2 新增统一的“LL当前是否必须等待PM2/回片优先”判定辅助函数

新增一个辅助函数，让 `LLA/LLB` 在线程内部也能做和 `PM2 step 200` 一致的全局判断。

判定输入要和 [slot_transfer_cycle_vtm_widget.cpp:L6600-L6608](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp#L6600-L6608) 对齐，至少包括：

- `haswaferpm`
- `armA_has`
- `armA_pending`
- `armB_has`
- `armB_pending`
- `pm2PendingTasks.size()`
- `loadlockReturnPendingTasks.size()`

判定目标：

- 当当前快照表明“应该等 PM2 完成、应该先回片、应该由 B 手去 PM2 取片”时
- `LLA/LLB` 都不得继续发新的 `GET_FROM_LL` 请求
- 包括普通 `1051` 和 `2066` 立即补取都要被拦住

这一步是本轮计划新增的核心决策，不再只做“局部手指冲突”修补。

### 1.3 修改 `LLA step 1051` 普通取片发送点

目标位置：

- `LLA case 1051` 的发送点：[slot_transfer_cycle_vtm_widget.cpp:L3528-L3557](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp#L3528-L3557)

计划修改：

- 在发送 `robot_get_from_lla` 前，先做两层判断：
  - `是否必须等待 PM2/回片优先`
  - `目标 arm 是否被逻辑占用`
- 若任一不满足：
  - 不发送请求
  - 保持等待/回退，不推进到发送后的等待步
  - 打日志说明：
    - 当前快照
    - 为什么必须等 PM2
    - 或者哪个请求正占用目标 arm
- 若预分配 arm 被占用而另一只手可用，则允许沿用原有动态切臂逻辑；切臂后仍需重新做这两层检查

### 1.4 修改 `LLB step 1051` 普通取片发送点

目标位置：

- `LLB case 1051` 的发送点：[slot_transfer_cycle_vtm_widget.cpp:L5057-L5091](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp#L5057-L5091)

计划修改：

- 与 `LLA 1051` 完全一致：
  - 先判断是否必须等 `PM2`
  - 再判断目标 arm 是否空闲
- 如果当前正确策略是“B 手应去 PM2，而不是 LL 再取一片”，则 `LLB` 在这里必须停住

### 1.5 修改 `LLA 2066` 回片后立即补取发送点

目标位置：

- [slot_transfer_cycle_vtm_widget.cpp:L4040-L4072](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp#L4040-L4072)

计划修改：

- 当前只做 `tmDoor + hasObject` 检查
- 改为发送前依次判断：
  - `tmDoor`
  - `是否必须等待 PM2/回片优先`
  - `目标 arm 是否逻辑占用`
- 若不满足：
  - 不发 `robot_get_from_lla.requested`
  - 不进入 `2067` 的补取等待
  - 直接跳过本轮立即补取，进入收尾/后续正常调度

### 1.6 修改 `LLB 2066` 回片后立即补取发送点

目标位置：

- [slot_transfer_cycle_vtm_widget.cpp:L5578-L5611](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp#L5578-L5611)

计划修改：

- 与 `LLA 2066` 同样处理
- 重点要拦住这次现场对应的错误情况：
  - `PM2` 当前仍应优先
  - 或 `A 手` 已被别的请求占用
  - 则 `LLB` 不得再发 `arm=0` 的立即补取

### 1.7 增强日志，直接把“为何LL线程被拦住”打出来

计划增加的日志点：

- `LLA/LLB 1051` 发送前
  - 记录 `PM2 step 200` 同维度快照
  - 记录是否因 `PM2 优先` 被拦住
  - 记录是否因 `arm` 占用被拦住
- `LLA/LLB 2066` 立即补取前
  - 记录 `immediateRepick.arm`
  - 记录 `hasObject`
  - 记录当前 `robot_step`
  - 记录是谁占用了该 arm
- 若决定“现在两只手都不该再动，应等待 PM2”
  - 日志必须明确写出该结论，而不是只打一个泛泛的 wait

## Assumptions & Decisions

- 本轮只修改 `slot_transfer_cycle_vtm_widget.cpp`
- 目标不是重构整个调度框架，而是在现有框架上把 `LL` 线程的判断补齐到和 `PM2 step 200` 一致
- 本轮优先级判断遵循你的要求：
  - 若当前状态应等待 `PM2`
  - 则 `LLA/LLB` 两边都不应继续运动
  - 应等到后续 `PM2` 完成后，再由正确手臂去执行 `PM2` 相关动作
- 立即补取属于可让步优化，不得高于 `PM2/回片` 优先级
- 安全优先于节拍：宁可跳过一次立即补取，也不能在错误时机多发一笔 `LL` 取片请求

## Verification

### 1. 复现场景验证

复现与本次一致的窗口，确认：

- 当 `PM2` 状态要求优先等待/回片/取 `PM2` 时
- `LLA/LLB 1051` 不再继续发新的 `GET_FROM_LL`
- `LLA/LLB 2066` 不再发“立即补取”请求

### 2. 日志验证

确认日志能直接看出：

- 当前是否命中“等待 PM2”条件
- 当前被拦的是哪一个 step
- 当前被拦的是哪一只手
- 当前是哪一类请求正在占用该手

### 3. 回归验证

确认以下路径仍能正常执行：

- `LLA` 普通取片
- `LLB` 普通取片
- `LLA` 回片后立即补取
- `LLB` 回片后立即补取
- `PM2` 完成后由正确手臂去执行下一步，而不是被 `LL` 抢跑
