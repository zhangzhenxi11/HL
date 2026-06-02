# 计划：LLA/LLB 在 case 950 空闲时返回 case 10 以持续触发上料

## Summary
当前 LLA/LLB 线程在 `case 950` 仅处理三类分支（取片/放片/下料回 LP），当三类队列都为空时只 `Sleep(500)` 并停留在 `case 950`，不会回到 `case 10` 去评估“是否需要破真空并呼叫 EFEM 上料”，导致 LL 线程表现为“单任务”：只能服务一片料从 `EFEM->LL->PM->LL->EFEM` 跑完后才可能开始下一片，无法在上一片处于 PM/WTR 期间提前把下一片上到 LL（无法持续供料）。

本计划把 `case 950` 明确定位为“调度点（dispatcher）”：当本侧 LL 当前没有需要立即执行的取/放/下料动作时，就允许“后退”回 `case 10` 去触发下一片上料（利用 PM 工艺时间窗口提前上片），从而让单个 LL 线程具备“多任务”能力，而不是等待上一片回到 LL 后才继续。

## Goal & Success Criteria
- **Goal**：LLA/LLB 在上进下出模式下具备“多任务调度”能力：上一片已被 WTR 从 LL 取走并进入 PM 工艺后，LL 线程可以继续触发下一片 EFEM 上料到本 LL 上层，而不是卡在 `case 950` 空转等待。
- **Success Criteria**：
  - 当 `loadLock*Pending/ReturnPending/ReturnCompleted` 均为空时，LL 线程不会长期停留在 `case 950`，而会回到 `case 10` 重新评估并在条件满足时发起 `tool_allow_get_wafer_*` 上料请求。
  - 仍保持优先级：LL 下层完成片（return completed / 需要下料回 LP）优先于新的上料动作；真实下料冲突或对侧下层优先回 LP 时，仍由 `isLoadingInterlock()` 阻止上料。

## Current State Analysis
- `LLA case 950`（[slot_transfer_cycle_vtm_widget.cpp](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp#L3107-L3128)）与 `LLB case 950`（[slot_transfer_cycle_vtm_widget.cpp](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp#L4460-L4482)）逻辑结构一致：
  - 有 `PendingTasks` -> 进入取片 `1000`
  - 有 `ReturnPendingTasks` -> 进入放片 `2000`
  - 有 `ReturnCompletedTasks` -> 进入下料回 LP `5000`
  - 否则：仅 `Sleep(500)`，保持 `auto_step == 950` 不变
- `case 10` 才会评估 `isLoadingInterlock()` 与“本侧是否已空且 LP 是否仍有 UNKNOWN 待上料片”的条件，并在满足时进入 `20/300/301` 发起 `tool_allow_get_wafer_* = true`，从而呼叫 EFEM 上料。
- 结论：当 LL 在真空阶段进入 `950` 且队列暂时为空时，缺少回到 `case 10` 的路径，导致不会再触发新一轮上料请求。

## Proposed Changes
### 0) 关键决策：让 case 950 能“后退”回 case 10
`case 950` 仍然优先处理本侧 LL 的即时动作（取/放/下料）。只有当本侧 LL 没有即时动作要做时，才允许把 `auto_step` 设置回 `10`，触发“上料判定与请求”。这样 LL 线程就能在上一片处于 PM/WTR 时，继续上下一片（多任务调度），而不是等待上一片回到 LL 才开始。

### 1) LLA：case 950 增加调度回退分支
- 文件：`d:\HLPrj\HL\device\src\slot_transfer_cycle_vtm_widget.cpp`
- 位置：`executeLLATransfer()` 的 `case 950`
- 改动：
  - 在 `Pending/ReturnPending/ReturnCompleted` 三个分支之后，新增 `else`（调度回退）：
    - 当本侧 `loadLockA` 三类队列都为 0 时，将 `loadlock1_auto_step` 置为 `10`（回到决策态），使其可以触发下一片上料请求
    - 若 `current_lp_cycle` 语义要求优先进入 `6000` 做循环收尾，则保持与 `case 10` 一致的跳转策略
  - **不** 在 `case 950` 直接呼叫 EFEM 上料；只负责回退到 `case 10`，由 `case 10 -> 20/300/301` 统一发起上料请求（保持职责边界）

### 2) LLB：case 950 增加调度回退分支
- 文件：`d:\HLPrj\HL\device\src\slot_transfer_cycle_vtm_widget.cpp`
- 位置：`executeLLBTransfer()` 的 `case 950`
- 改动：
  - 与 LLA 同样的 `else` 回退逻辑：
    - 三类队列都为空时：`loadlock2_auto_step = 10`
    - 需要进入循环收尾时：跳 `6000`
  - 保留 `UpdateLLBSubTransferDatas()`

### 3) 行为约束（不改变既有优先级）
- 不改动 `case 10` 内部的互锁与上料条件，只补足 `950 -> 10` 的状态机回路
- 继续依赖 `isLoadingInterlock()` 来表达跨侧“真实下料冲突 + 下层完成片优先回 LP”的互锁
- `ReturnCompletedTasks.size() > 0` 仍然优先进入 `5000`（下料回 LP），确保“下层有完成片优先下料”的需求不被破坏

## Assumptions & Decisions
- 决策：当 `case 950` 处于空闲且三类队列均为空时，回到 `case 10` 由其统一决策是否需要上料（包括破真空、呼叫 EFEM 等）。
- 假设：在 `case 10` 中发起上料请求（`tool_allow_get_wafer_* = true`）已经包含必要的互锁判断与后续动作（例如破真空），因此 `950` 不需要直接呼叫 EFEM。
- 假设：`case 950` 作为真空阶段“任务分发点”，允许在无任务时回退到 `case 10`，不会破坏真空状态机的安全条件（实际破真空仍由 `case 20/100/300` 控制）。

## Verification Steps
### 静态验证
- 仅修改 `slot_transfer_cycle_vtm_widget.cpp`，确保文件编码与 CRLF 保持不变
- 通过 IDE 诊断确认无新增编译错误

### 行为验证（现场/仿真）
- 场景 A：LL 处理完一片后进入 `case 950` 且三类队列为空
  - 期望：`auto_step` 回到 `10`，满足上料条件时可重新发起 `tool_allow_get_wafer_*`，EFEM 开始下一片上料
- 场景 A2：上一片已被 WTR 取走、正在 PM 工艺中（本 LL 三类队列为空）
  - 期望：LL 线程在 `case 950` 也会回退到 `case 10` 并发起下一片上料请求；不需要等待上一片回 LL 再进入 `case 950` 决策
- 场景 B：LL 下层存在完成片（`ReturnCompletedTasks > 0`）
  - 期望：仍优先进入 `5000` 走下料回 LP，不会被“回到 10 的上料逻辑”抢占
- 场景 C：对侧存在真实下料冲突或下层完成片优先回 LP 约束
  - 期望：即使回到 `10`，`isLoadingInterlock()` 仍会阻止本侧上料请求
