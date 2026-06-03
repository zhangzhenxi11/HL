# 收敛PM2预装与交互片判定计划

## Summary
- 目标：针对 `LLA/LLB -> PM2` 的连续供料场景，修正“PM2仍在加工时应允许预装下一片在手等待交换”的调度判定，使其与 [流程调度时序.txt](file:///d:/HLPrj/HL/流程调度时序.txt#L9-L35) 一致。
- 范围：仅调整 [slot_transfer_cycle_vtm_widget.cpp](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp) 中 `1051/2066/PM2 step 200/PM2快照与优先级判定`，不改 Robot 执行层命令接口，不改 EFEM/TaskManager 总体状态机结构。
- 成功标准：
  - `WTR` 在 `PM2` 加工期间可按固定 `arm` 预装下一片在手等待。
  - `PM2` 完成后优先形成交互片，而不是“先从 PM2 取完成片，再去 LL 取下一片”。
  - 日志分析时能稳定输出 `WTR` 时序对照表，便于验收。

## Current State Analysis
- 日志已证明当前偏差：在 [any_cleaned.txt:L136596-L136599](file:///d:/HLPrj/HL/_collected_latest_20260603_165016/2026-06-03_any_cleaned.txt#L136596-L136599) 中，`LLA step 1051` 因“`PM2` 有片且双手为空，应先从 `PM2` 取片”而禁止预装；随后在 [any_cleaned.txt:L136876-L136886](file:///d:/HLPrj/HL/_collected_latest_20260603_165016/2026-06-03_any_cleaned.txt#L136876-L136886) 中，实际顺序变成“先从 `PM2` 取完成片，再从 `LLA` 取下一片”，与 [流程调度时序.txt:L27-L35](file:///d:/HLPrj/HL/流程调度时序.txt#L27-L35) 不一致。
- `1051` 当前已能识别“持完成片回 LL 后继续由同一固定 `arm` 补取”的方向，见 [slot_transfer_cycle_vtm_widget.cpp:L3797-L3844](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp#L3797-L3844)。但普通取片分支仍直接调用 `shouldLlWaitForPm2Priority()`，见 [slot_transfer_cycle_vtm_widget.cpp:L3866-L3891](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp#L3866-L3891)，会在 `PM2` 有片且双手空时阻塞预装。
- `2066` 当前已经引入 `canLlImmediateRepickWaitForPm2Craft()` 来放宽“回 LL 后立即补取”的窗口，见 [slot_transfer_cycle_vtm_widget.cpp:L4433-L4457](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp#L4433-L4457)。但该放宽只覆盖“已经进入回片后立即补取流程”的路径，无法覆盖 `1051` 普通取片分支。
- `PM2ScheduleSnapshot` 与 `shouldLlWaitForPm2Priority()` 仍把“`PM2` 有片且双手空”统一归类为“应先从 `PM2` 取片”，见 [slot_transfer_cycle_vtm_widget.cpp:L1030-L1080](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp#L1030-L1080)。它缺少“`PM2` 当前是加工中，可允许另一只固定手先预装”的明确状态表达。
- `PM2 step 200` 的决策入口在 [slot_transfer_cycle_vtm_widget.cpp:L7025-L7167](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp#L7025-L7167)。这里能基于 `pm2PendingTasks / pm2CompletedTasks / returnPendingTasks / hasObject()` 决定取、放、交换，但没有把“加工中 / 完成待取”区分回传给 `LL` 侧优先级判定。

## Proposed Changes
### 1. 收紧 `PM2` 优先级规则，只在“完成待取/已排队待回片”时阻塞 `LL` 预装
- 文件：[slot_transfer_cycle_vtm_widget.cpp](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp)
- 位置：`PM2ScheduleSnapshot`、`tryBuildPm2ScheduleSnapshot()`、`shouldLlWaitForPm2Priority()`
- 修改内容：
  - 在 `PM2ScheduleSnapshot` 中显式区分两类状态：
    - `PM2` 加工中，尚未进入 `LOADLOCK_RETURN/QUEUED`
    - `PM2` 完成待取/已有回片任务
  - 将 `shouldLlWaitForPm2Priority()` 的阻塞条件从“只要 `PM2` 有片且双手空就阻塞”收紧为：
    - 仅当 `pm2CompletedCount > 0` 或 `returnPendingCount > 0` 时，强制 `PM2` 优先。
    - 若只是“`PM2` 加工中 + 双手空 + LL 侧有符合固定 `arm` 的待加工片”，则允许 `LL` 预装。
- 原因：这是当前日志偏差的直接根因；若不改，`1051` 仍会在最关键的预装窗口被挡掉。

### 2. 让 `1051` 普通取片分支复用“允许预装窗口”判定
- 文件：[slot_transfer_cycle_vtm_widget.cpp](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp)
- 位置：`LLA case 1051`、`LLB case 1051`
- 修改内容：
  - 在普通取片分支（当前 `pm2HasWafer == false || 双手全空` 这条路径）增加与 `2066` 一致的“预装窗口”判定。
  - 当满足以下条件时，不再因为 `shouldLlWaitForPm2Priority()` 而阻塞：
    - `PM2` 正在加工而非完成待取。
    - 双手当前为空。
    - 当前 `desiredArm` 或在 pending 队列中能找到符合固定 `arm` 的下一片。
    - 没有 `returnPending/completed` 的抢占优先级。
  - `LLA/LLB` 两边保持对称实现。
- 原因：从日志看，异常不是发生在 `2066`，而是发生在 `1051`。因此必须把“允许预装窗口”前移到 `1051`。

### 3. 保持固定 `arm`，但把“下一片选择规则”明确为“同 arm 的第一片待加工任务”
- 文件：[slot_transfer_cycle_vtm_widget.cpp](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp)
- 位置：`LLA/LLB case 1051`
- 修改内容：
  - 维持当前已收敛的固定手臂策略，不恢复动态改 `task.arm`。
  - 明确 `pending` 队列查找规则：在持续供料场景下，选择第一片 `task.arm == occupiedArm/desiredArm` 的待加工任务，而不是仅依赖队首顺序。
  - 日志输出补充 `selectedTaskId / selectedArm / skippedReason`，便于后续比对时序。
- 原因：用户已经明确前提是“`LL` 会一直有上的料”，因此选择规则要稳定匹配固定 `arm`，而不是受队列头部顺序误伤。

### 4. 用 `PM2 step 200` 的真实判定结果反哺 `LL` 侧，而不是各自猜测
- 文件：[slot_transfer_cycle_vtm_widget.cpp](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp)
- 位置：`PM2 step 200` 决策入口、`tryBuildPm2ScheduleSnapshot()`
- 修改内容：
  - 梳理 `PM2 step 200` 中“无片时放片 / 有片时取片 / 有片且单手持料时交换”的现有判定。
  - 在不大改状态机结构的前提下，让 `tryBuildPm2ScheduleSnapshot()` 能基于 `pm2PendingTasks / pm2CompletedTasks / loadlockReturnPendingTasks` 提供一个更贴近 `PM2 step 200` 语义的状态摘要。
  - `LL` 侧仅依据这一摘要判断“当前是否允许预装”，避免 `LL` 与 `PM2` 各自独立推断导致冲突。
- 原因：当前 `LL` 侧和 `PM2` 侧都在根据局部信息判断，导致“`PM2` 认为可继续保持交换节拍，`LL` 却先把自己阻塞住”的不一致。

### 5. 固化日志分析输出：默认生成 `WTR` 时序对照表
- 文件：无代码改动优先；若已有诊断/分析输出入口可配置，则按最小范围接入
- 修改内容：
  - 保持当前项目规则：后续分析同类日志问题时，默认输出“期望步骤 vs 实际步骤”的 `WTR` 时序表。
  - 若实现阶段需要补日志，优先补在 `1051/2066/PM2 step 200` 的关键转折点，确保能从日志直接生成时序表。
- 原因：本次问题定位高度依赖 `WTR` 顺序分析，后续验收也需要同样格式。

## Assumptions & Decisions
- 假设一：本次目标以“连续供料、`LLA/LLB` 始终有后续待加工片”为主，不优先处理最后一片收尾路径。
- 假设二：保持固定 `arm` 是硬约束，不允许为保节拍动态修改 `task.arm`。
- 决策一：优先修改判定逻辑，而不是重构整体状态机；避免扩大风险面。
- 决策二：优先在 `1051` 解决“预装窗口被挡掉”的问题，因为日志证明问题首先发生在这里。
- 决策三：`PM2` 完成待取与 `PM2` 加工中必须在快照层面显式区分，否则 `LL` 无法做正确预装判断。

## Verification Steps
- 静态验证
  - 检查 `LLA/LLB case 1051` 是否在“`PM2` 加工中 + 双手空 + 有固定 arm 待加工片”场景下允许发送 `robot_get_from_ll*`。
  - 检查 `shouldLlWaitForPm2Priority()` 是否只在“完成待取 / 待回片”场景下强制 `PM2` 优先。
  - 检查 `2066` 与 `1051` 的预装窗口规则是否一致，不产生新的互相打架分支。
- 日志验证
  - 用 [流程调度时序.txt](file:///d:/HLPrj/HL/流程调度时序.txt#L9-L35) 作为对照，输出 `WTR` 时序表。
  - 重点确认顺序变为：
    - `放 PM2`
    - `PM2 加工期间预装下一片在手`
    - `PM2 完成后直接交互`
  - 不应再出现 [any_cleaned.txt:L136598-L136599](file:///d:/HLPrj/HL/_collected_latest_20260603_165016/2026-06-03_any_cleaned.txt#L136598-L136599) 这种“`PM2` 有片且双手空，应先从 `PM2` 取片”的阻塞日志。
- 诊断验证
  - 修改后检查 [slot_transfer_cycle_vtm_widget.cpp](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp) 无新增诊断错误。
  - 再次复核文件编码与换行保持原样。
