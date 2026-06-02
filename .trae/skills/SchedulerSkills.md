# Scheduler Skills — VTM 传片调度（重构知识库）

> **来源**：根目录老项目 [`slot_transfer_cycle_vtm_widget.cpp`](../../slot_transfer_cycle_vtm_widget.cpp)  
> **详细架构**：[`device/doc/slot_transfer_cycle_vtm_调度系统分析.md`](../../device/doc/slot_transfer_cycle_vtm_调度系统分析.md)  
> **用途**：重构时保留已验证的调度思想，替换步号魔法数/布尔海/队首假设等实现债。

---

## 0. 一句话 Skill

> 在多设备、多互锁系统中：**共享危险资源（WTR）单点串行执行**；外围线程按设备边界**并行准备**；用**阶段队列 + 允许标志**组织任务流，**不跨线程直控硬件**。

---

## 1. 设计原则

### 1.1 按物理边界拆职责（Boundary Ownership）

| 边界 | 线程/函数 | 独占硬件 | 禁止事项 |
|------|-----------|----------|----------|
| 大气侧 | `startEFEMAction` | EWTR | 不判断 TM 真空、不下发 WTR |
| LL 大气↔真空 | `startLoadLock1/2Action` | LL 阀门/真空/移槽 | 不直接 `wtr->createGet/Put` |
| 真空搬运 | `startRobotAction` | WTR | 不替 PM 跑工艺、不替 EFEM 开 LP |
| 工艺 | `startPMAction` | PM 转位/工艺 | 不并发抢 WTR 命令 |
| 真空协调 | `startVacuumAction` | Pump/TM/LL 抽真空 | 不在多线程重复抽同一路 |
| 轮次 | `startUpdateStatusAction` | 无硬件 | 不介入单片取放时序 |

**原则**：一个线程只承担「本边界内的时序闸门 + 本边界命令」；**边界设备不做全局执行器决策**。

### 1.2 准备并行，执行串行（Prepare Parallel, Execute Serial）

- **可并行**：破/抽真空、mapping、PM 转工艺位、EFEM 开盒、Update 重装填。
- **必须串行**：WTR 全部 `createGetCommand` / `createPutCommand`（全文件仅在 `startRobotAction`）。

**原则**：提高吞吐靠「多腔同时准备」，不是靠「多线程同时动 WTR」。

### 1.3 阶段推进，禁止跨阶段预执行（Staged Commit）

- 调度不是一次算完全路径，而是 **四段队列** 逐段推进（见 §2.1）。
- 每段完成才 **push 下一段队列**；危险动作前检查真空/门阀/槽位/手臂 mapping。

**原则**：当前阶段未 `COMPLETED`（出队/登记），不启动下一阶段硬件动作。

### 1.4 数据驱动，禁止线程互调（Queue + Flag, No Callback Chain）

- **队列**：表达「还有什么片、从哪到哪」（`sequence_*`）。
- **标志**：表达「现在谁被允许动」（`tool_*` / `loadlock_*` / `pm_*` / `*_get_vacuum`）。
- 线程间 **不** 直接调用对方的状态机步骤。

**原则**：用共享数据结构协商，避免 A 等 B 回调、B 再等 A 的嵌套死锁。

### 1.5 软件服从硬件互锁（Hardware-First State Machine）

状态机步号围绕真实顺序建模，典型硬前置：

```text
破真空 → 大气侧接片 → 关门 → 抽 LL 真空 → mapping/移槽 → 开 TM 门
→ TM 真空达标 → WTR 取放 → PM 在取放位 → WTR 交互 → 工艺位工艺
→ 回取放位 → WTR 回收 → LL 破真空 → EFEM 回 FOUP
```

**原则**：互锁条件是 **guard**，不是日志附属字段。

### 1.6 队列表达任务，标志表达执行权（双通道模型）

| 通道 | 回答的问题 | 老项目载体 |
|------|------------|------------|
| 队列 | 有哪些片？去哪？哪只手？ | `RobotTransferWafer` / `LoadLockTransferWafer` |
| 标志 | 此刻能不能做？ | `*_allow_*`、`*_get_vacuum` |

**原则**：重构时不要把「任务存在」和「允许执行」混在一个 bool 里。

---

## 2. 调度规则

### 2.1 四段标准流转（单片闭环）

```text
① ELP → LL     EFEM 消费 sequence_lp*_get_wafer
② LL → PM     WTR  消费 sequence_robot_transfer_wafer[0]
③ PM → LL     WTR  消费 sequence_robot_get_wafer[0]
④ LL → ELP     EFEM 消费 sequence_lp*_put_wafer
```

**队列入队/出队契约**（源码 L227~239）：

| 事件 | 队列操作 |
|------|----------|
| EFEM 放入 LL 成功 | `push` `sequence_loadlock*_transfer_wafer`；`push` `sequence_lp*_put_wafer`（预登记回 FOUP） |
| WTR 放入 PM 成功 | `push` `sequence_robot_get_wafer`（登记待回收） |
| WTR 从 PM 取回并放回 LL 成功 | 从 `sequence_robot_transfer_wafer` **删除**对应项；维护 `sequence_loadlock*_put_wafer` |
| EFEM 回 FOUP 成功 | 该片逻辑闭环；Update 可计轮次 |

### 2.2 标志触发规则（执行权）

| 标志 | 置位方 | 等待方 | 含义 |
|------|--------|--------|------|
| `tool1_allow_get_wafer` | LLA | EFEM | 允许 LP1/TOOL1 上料到 LLA |
| `tool1_allow_put_wafer` | LLA | EFEM | 允许从 LLA 下料回 LP |
| `tool2_*` | LLB | EFEM | 对称 |
| `loadlock1_allow_get_wafer` | LLA | Robot | 允许 WTR 从 LLA 取片 |
| `loadlock1_allow_put_wafer` | LLA | Robot | 允许 WTR 向 LLA 放片 |
| `loadlock2_*` | LLB | Robot | 对称 |
| `pm_allow_get_put_wafer` | PM | Robot | PM 在取放位，允许 WTR 交互 |
| `pm_allow_goto_craft` | Robot 放片后 | PM | 允许 PM 去工艺位 |
| `tm_get_vacuum` / `loadlock*_get_vacuum` | Robot/LL | Vacuum | 请求抽真空 |

**清位纪律**：消费者完成动作后清 false；生产者仅在入口条件满足时置 true。**避免标志粘滞**。

### 2.3 Robot 任务选择规则（`robot_auto_step` 1010）

在队首任务存在时，按 **来源/目标 LL** 分流，而非 FIFO 盲执行：

| 条件（简化） | 跳转段 | 动作 |
|--------------|--------|------|
| `transfer` 队首来自 LLA→LLA，且待 PM 上料 | 2000 | 从 LLA 取 |
| 来自 LLB→LLB | 10000 | 从 LLB 取 |
| `sequence_robot_get_wafer` 待回收 / 最后一片 | 4000 / 6000 / 14000 | PM 交互或回 LL |
| TM 真空未就绪 | 100→120 | 先 `tm_get_vacuum` |

**规则本质**：WTR 线程是 **全局仲裁者**，读 PM 槽位 + 双臂 mapping 决定放/取/交换（`case 4001`）。

### 2.4 PM 与 Robot 握手规则

```text
PM:  取放位 → pm_allow_get_put_wafer = true
Robot: 放 PM 成功 → pm_allow_get_put_wafer = false; pm_allow_goto_craft = true
PM:  工艺完成回取放位 → 再次 pm_allow_get_put_wafer = true
Robot: case 4000 等待 pm_allow_get_put_wafer 才进入取放
```

**规则**：PM 工艺与 WTR 取放 **互斥** 由两个标志串联，不靠隐式时序。

### 2.5 传输路径一致性规则（防交叉污染）

- `RobotTransferWafer.transfer` 与 `sequence_tolk*_wafer[0].transfer` 不一致时，LL `case 10` **Sleep(20000)** 等待（不推进上料/取片）。
- 含义：不同 transfer 模式（如 `lk1_to_lk2` vs `lk1_to_lk1`）不能混跑。

**重构建议**：显式「调度世代/批次 ID」，替代长 Sleep 盲等。

### 2.6 多轮 Cycle 规则（Update 线程）

- 未达 `cycle_times_lla/llb`：从 `sequence_*_copy` / `sequence_robot_transfer_wafer_lp1/lp2` **重装填**。
- 已达上限：`cycleFinished_lla/llb = true` → Robot `case 1020` 停 `running`、开放 UI。

**规则**：单片逻辑在业务线程；**轮次边界**在 Update 单点收口。

---

## 3. 资源竞争处理方式

### 3.1 WTR — 单点串行（最高优先级）

| 做法 | 说明 |
|------|------|
| 收敛 | 全部真空 get/put 仅在 `startRobotAction` |
| 禁止 | LL/PM/EFEM 线程 `wtr->startCommand` |
| 决策集中 | 交换料、手臂选择、`robot_selected_arm` 在 `case 4001` |

**竞争降级**：多线程抢 WTR → 单线程服务队列 + 标志等待。

### 3.2 共享 vector — 互斥锁

| 锁 | 保护对象 |
|----|----------|
| `vec_mutex_robot` | `sequence_robot_transfer_wafer`、`sequence_robot_get_wafer` |
| `vec_mutex_lla` | `sequence_loadlock1_*` |
| `vec_mutex_llb` | `sequence_loadlock2_*` |

**纪律**：push/erase/swap(0) 持锁；**不要**持锁跨 `cmd->wait()`。

### 3.3 布尔标志 — 无锁 + 约定顺序

- 典型模式：生产者 `= true` → 消费者轮询 `Sleep` → 消费者完成 `= false`。
- **风险**：缺少 memory order 时极端平台可能可见性问题；重构宜用 `std::atomic<bool>` 或明确 release/acquire。

### 3.4 真空 — 专用线程 + 请求位

- LL/Robot 只置 `loadlock*_get_vacuum`、`tm_get_vacuum`。
- `startVacuumAction` 统一执行泵/阀命令。
- Robot `case 100~120` 与 Vacuum 通过 `tm_get_vacuum` 握手。

### 3.5 队首仲裁 — 以 index 0 为当前任务

- Robot `case 1010` 以 `sequence_robot_transfer_wafer[0]` / `sequence_robot_get_wafer[0]` 为当前决策对象；必要时在持锁下 **swap 到 index 0** 再执行。
- **竞争点**：swap 与 erase 必须同锁 `vec_mutex_robot`。

### 3.6 EFEM vs LL — 双标志握手

- 上料：`tool*_allow_get_wafer` 互斥于同侧 `tool*_allow_put_wafer` 流程（LL case 判断 `!tool*_allow_get && !tool*_allow_put` 才抽真空等）。
- **避免**：EFEM 与 LL 同时认为「可以开门」。

---

## 4. 异常恢复机制

### 4.1 统一失败止损 — `logFailed*`

```cpp
// 行为：running=false; ispause=true; 开放 execute 按钮; 打 Error 日志
logFailed / logFailedNotNormal / logFailedExcuteCommandHasError
```

| 类型 | 触发 | 恢复路径 |
|------|------|----------|
| 命令失败 | `cmd->hasError()` | 人工检查 → Reset / 重新 Start |
| 子系统非正常 | `getState() != SUB_NORMAL` | 同上 |
| 逻辑错误 | 如 Robot「传输顺序错误」 | 查队列/标志一致性后 Abort 重建序列 |

**原则**：**先停全局 `running`**，再在安全状态下恢复；不在失败链上自动连发下一条硬件命令。

### 4.2 步内回退（局部重试）

- 部分 LL/Robot 失败：`Sleep(2000)` 后回到上一决策步（非全局 logFailed）。
- PM 工艺失败：`pm_auto_step = 2110` → **弹窗人工确认** 后重试。

**原则**：可恢复瞬态错误 → 局部重试；不确定安全 → 全局停机。

### 4.3 操作员干预 — Pause / Reset / Abort

| 操作 | 行为要点 | 适用场景 |
|------|----------|----------|
| **Pause** | `ispause=true`，线程空转等待 | 保留队列/标志现场 |
| **Reset** | `resetAction()` detach：WTR/EWTR/LP/LL/PM 复位命令 | 设备异常后回安全态 |
| **Abort** | `abortCycle`：允许「只放不回」等收尾；可重建 `setTransferSequence` | 放弃当前轮次 |

**Continue 启动**：若 `sequence_robot_transfer_wafer` 非空且非 pause 重建，可 **不重新 setTransferSequence**（保留现场续跑）。

### 4.4 特殊场景 — UPS / Abort 收尾

- `hasUPS`：断电后 LL **不再取料**，仅允许放料路径（源码 loadlock case 判断）。
- `abortCycle`：配合 `sequence_loadlock*_put` 为空等条件，走 **安全下料/收尾** 分支。

**重构建议**：将 UPS/Abort 建模为显式 `SchedulerMode::EmergencyDrain`。

### 4.5 正常轮次恢复 — Update 线程

- 非错误完成：从 `*_copy` 恢复配置，**不重新读 UI**。
- 与 `logFailed` 路径分离，避免「错误停机」与「正常下一轮」代码交织。

---

## 5. 死锁避免策略

### 5.1 共享执行器单线程化（根本策略）

- **问题**：多线程同时 wait WTR 命令完成。
- **策略**：仅 `startRobotAction` 调用 WTR；其他线程只置 `loadlock*_allow_*` / `pm_allow_*`。

### 5.2 申请权与执行权分离（无回调环）

```text
错误：LL 线程 ──直接──► wtr->put ──等待──► PM 线程 ──直接──► wtr->get
正确：LL ──allow=true──► Robot ──命令──► 清 allow / 改队列 ──► PM 见标志
```

- **避免**：线程 A 持锁等待 B，B 等待 A 的命令完成。

### 5.3 固定物理顺序（无跳步）

- 未 `loadlock*_allow_get_wafer` → Robot 不进 2000/10000 取片段。
- 未 `pm_allow_get_put_wafer` → Robot 不进 4000 PM 段。
- TM 真空未达标 → 不进 1000 主搬运（先 100~120）。

**避免**：「门未开 / 真空未达标」下进入取放 → 硬件忙等 → 软件各线程永久 Sleep 轮询。

### 5.4 单阶段单等待点（不跨层持资源）

- LL 线程等待 EFEM 时 **只** 等 `tool*_allow_get` 变 false，不同时等 WTR。
- Robot 等待 PM 时 **只** 等 `pm_allow_get_put_wafer`，不在此期间占着 LL 移槽步骤。

### 5.5 显式标志轮询代替复杂锁环

- 老项目用 `Sleep(10~500)` 轮询，**无** condition_variable 双向等待链。
- **代价**：CPU 占用；**收益**：不易形成 wait 环。

**重构建议**：保留「单向等待」（消费者等生产者标志），用 `wait_for` + predicate 替代盲 Sleep，**禁止**双向 `cv.wait` 嵌套。

### 5.6 传输路径锁（防逻辑死锁）

- `transfer` 不一致 → LL 长等待（20s），**故意不推进** 而非强行取放。
- **重构**：不一致时进入 `SchedulerFault::RouteMismatch` 显式故障态，而非无限 Sleep。

### 5.7 Update 与主路径解耦

- 轮次重装填在 `startUpdateStatusAction`，不在 Robot/LL 的取放 case 内嵌套。
- **避免**：单片未完成时触发「清空母队列」类操作。

### 5.8 队首假设与饥饿

- 风险：某 LL 任务永远不在 `[0]` → Robot 1010 报「传输顺序错误」停滞。
- **缓解**：`transfer` 一致才推进；重构用 **公平调度器**（`pickNextJob()`）替代强 `[0]` 假设。

---

## 6. 重构对照表（保留 / 替换）

| 老项目做法 | 保留的思想 | 建议替换为 |
|------------|------------|------------|
| `sequence_*` 多向量 | 四段阶段队列 | 统一 `WaferJob` + 阶段枚举 + 单任务表 |
| `*_allow_*` bool 海 | 执行权与任务分离 | `Gate` / `Permit` 对象，带 owner 与超时 |
| `robot_auto_step` 魔法数 | WTR 单线程 + 分支决策 | 表驱动状态 + 显式 `RobotOp` 请求队列 |
| `Sleep` 轮询 | 单向等待 | `wait_for(predicate)` + 日志心跳 |
| 队首 `[0]` | 当前任务唯一 | 调度器 `pickNextJob()` |
| `vec_mutex_*` | 队列持锁纪律 | 短锁 + 命令层不持锁 |
| `logFailed` 停全局 | 失败止损 | 分级：`Retryable` / `Fatal` / `OperatorConfirm` |

### 6.1 重构不可破坏的不变量（Invariants）

1. **WTR 命令单线程下发**（允许改为请求-响应，但不允许多执行者）。
2. **四段顺序**：ELP→LL → LL→PM → PM→LL → LL→ELP 不可逆跳段。
3. **PM 工艺与 WTR 取放互斥**（工艺位期间不得 PM 取放允许为 true）。
4. **真空抽吸与 WTR 伸臂互斥**（Vacuum 等待原点/空闲）。
5. **放 PM 成功必须登记回收**（`robot_get` 等价物）。
6. **EFEM 回 FOUP 依赖预先登记的 put 队列**。

### 6.2 建议目标架构（示意）

```text
SchedulerService
├── JobStore          // 替代 sequence_* + 未来 TaskManager
├── GateRegistry      // 替代 *_allow_*
├── RobotExecutor     // 单线程，替代 startRobotAction
├── EfemWorker / LlWorker / PmWorker / VacuumWorker / CycleWorker
└── FailurePolicy     // 替代 logFailed + Abort/UPS
```

---

## 7. 检查清单（重构 PR 自检）

- [ ] 真空中仅一个模块调用 `WTR::startCommand`？
- [ ] 每个 `allow` 标志都有唯一置位者与清位者？
- [ ] 队列修改是否都在 mutex 内且不含 `wait()`？
- [ ] PM 工艺中 `pm_allow_get_put_wafer` 能否为 true？（应为否）
- [ ] 放 PM 后是否必登记回收任务？
- [ ] 失败路径是否区分「重试」与「停 running」？
- [ ] 是否存在双向等待（A 等 B 标志，B 等 A 标志）且无超时？

---

## 8. 参考

- 源码：`slot_transfer_cycle_vtm_widget.cpp`（根目录）
- 分析文档：`device/doc/slot_transfer_cycle_vtm_调度系统分析.md`
- WTR 集中调度示意：`device/doc/重构wtr调度.md`（与新实现对照时可读）

---

*本文档从老项目提炼调度 Skill，供 Trae/Cursor Agent 在重构 VTM Scheduler 时作为约束与检查依据。*
