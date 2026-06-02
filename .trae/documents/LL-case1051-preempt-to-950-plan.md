# LL `case 1051` 抢占回退到 `950` 计划

## Summary
- 目标：修复 `LLA/LLB` 在线程走到 `case 1051` 时，因“PM2 内有片且 WTR 手上已有晶圆”而原地等待，导致无法转去处理已完成工艺片回 LL 的问题。
- 方案：把 `case 1051` 中当前“禁止继续取片”的等待逻辑，改成“抢占回退到 `case 950`”，并同步重排 `case 950` 的优先级，让同一条 LL 线程始终先处理已完成片下料，再处理待回 LL 的片，最后才继续取待加工片。

## Current State Analysis
- 当前 `LLA` 的 `case 950` 顺序是：
  - `loadLockAPendingTasks > 0` 时进 `1000` 取待加工片。
  - `loadLockAReturnPendingTasks > 0` 时进 `2000` 放工艺完成片回 LL。
  - `loadLockAReturnCompletedTasks > 0` 时进 `5000` 下料到 EFEM/LP。
  - 无任务时回 `10`。
- 当前 `LLB` 的 `case 950` 也是同样的顺序。
- 这个顺序会导致：即使本侧已经存在“应优先下料到 LP”的完成片，线程仍可能先去处理新的待加工片，破坏用户要求的“上进下出”优先级。
- 当前 `LLA/LLB` 的 `case 1051` 已增加限制：
  - `PM2` 有片且 `WTR` 任一手有片时，不允许继续从 LL 取片。
  - 但实现方式是 `Sleep(200); break;`，会让线程停留在 `1051`，无法回到 `950` 检查 `ReturnPending`。
- 结合用户描述的节拍：
  - `task0` 工艺完成后会变为 `PM_PROCESS/COMPLETED -> LOADLOCK_RETURN/QUEUED`。
  - 此时 `task1` 在 `PM2` 内，另一只手可能还拿着工艺完成片。
  - 如果 LL 线程卡在 `1051`，就不会优先走 `2000` 把回片放回 LL，形成逻辑性卡死。
- 用户补充的关键目标顺序是：
  - 1. `ReturnCompleted` 优先，即先走 `5000` 下料。
  - 2. 再处理 `ReturnPending`，即走 `2000` 把工艺完成片放回 LL。
  - 3. 最后才处理 `Pending`，即走 `1000` 继续从 LL 取待加工片。

## Proposed Changes

### 1. 重排 `LLA` 的 `case 950` 优先级
- 文件：`d:\HLPrj\HL\device\src\slot_transfer_cycle_vtm_widget.cpp`
- 修改点：`LLA` 的 `case 950`
- 变更内容：
  - 将 `LLA case 950` 的分支顺序调整为：
    - `loadLockAReturnCompletedTasks.size() > 0` 时先走 `5000`
    - `loadLockAReturnPendingTasks.size() > 0 && !abortCycle` 时再走 `2000`
    - `loadLockAPendingTasks.size() > 0 && !abortCycle` 时最后才走 `1000`
    - 都没有时再回 `10`
- 原因：
  - 这是用户明确指定的“上进下出”优先级。
  - `ReturnCompleted` 表示已经回到 LL 且应尽快下料回 LP，不应再被新的取片动作插队。

### 2. 重排 `LLB` 的 `case 950` 优先级
- 文件：`d:\HLPrj\HL\device\src\slot_transfer_cycle_vtm_widget.cpp`
- 修改点：`LLB` 的 `case 950`
- 变更内容：
  - 与 `LLA` 保持完全对称：
    - `ReturnCompleted -> 5000`
    - `ReturnPending -> 2000`
    - `Pending -> 1000`
    - 否则回 `10`
- 原因：
  - 防止双 LL 在线程行为上继续出现不对称。

### 3. 调整 `LLA` 的 `case 1051` 抢占路径
- 文件：`d:\HLPrj\HL\device\src\slot_transfer_cycle_vtm_widget.cpp`
- 修改点：`LLA` 的 `case 1051`
- 变更内容：
  - 将 `if (pm2HasWafer && (armAHasWafer || armBHasWafer))` 这个限制分支从“原地等待”改成“切回 `loadlock1_auto_step = 950`”。
  - 保留节流日志，但日志语义改为“检测到 PM2/WTR 占用，回退到 950 优先处理回片/下料”。
  - 不直接跳 `2000`，而是统一回 `950`，让现有优先级判断决定是走 `1000/2000/5000/10`，避免绕开既有调度入口。
- 原因：
  - `950` 已经是 LL 子流程的汇总调度点。
  - 在 `950` 优先级被重排后，回 `950` 可以自然先走 `5000/2000`，确保回片和下料不再被预取动作压住。

### 4. 调整 `LLB` 的 `case 1051` 抢占路径
- 文件：`d:\HLPrj\HL\device\src\slot_transfer_cycle_vtm_widget.cpp`
- 修改点：`LLB` 的 `case 1051`
- 变更内容：
  - 与 `LLA` 对称处理：把 `pm2HasWafer && (armAHasWafer || armBHasWafer)` 分支改成 `loadlock2_auto_step = 950`，而不是停在 `1051`。
  - 保持日志与行为对称，避免 `LLA/LLB` 节拍分叉。

### 5. 保留目标手臂占用保护，但区分两类限制语义
- 文件：`d:\HLPrj\HL\device\src\slot_transfer_cycle_vtm_widget.cpp`
- 修改点：`LLA/LLB` 的 `case 1051`
- 变更内容：
  - `PM2有片 + WTR任一手有片`：视为“应切回回片链路”的抢占条件，回 `950`。
  - `desiredArm` 已有片：仍保留防御性保护，继续短暂等待或保持在 `1051`，避免对同一只已占用手再次发取片请求。
- 原因：
  - 前者是节拍调度问题，应切换流程。
  - 后者是机械手占用保护问题，属于局部防呆，不一定意味着当前就该进 `2000`。

### 6. 针对用户给出的四任务状态做流程推演并据此校验改动
- 文件：`d:\HLPrj\HL\device\src\slot_transfer_cycle_vtm_widget.cpp`
- 推演场景：
  - `task0: PM_PROCESS/COMPLETED -> LOADLOCK_RETURN/QUEUED`
  - `task3: EFEM_TRANSFER/COMPLETED -> LOADLOCK_TRANSFER/QUEUED`
  - `task2: EFEM_TRANSFER/COMPLETED -> LOADLOCK_TRANSFER/QUEUED`
  - `task1: LOADLOCK_TRANSFER/COMPLETED -> PM_PROCESS/QUEUED`
  - 同时满足：
    - `pm2Has=1`，表示 `task1` 在 `PM2`
    - `armA_has=0, armB_has=1`，表示 `task0` 的工艺后片在 `B` 手
    - `task2` 在 `LLA` 待上 PM
    - `task3` 在 `LLB` 待上 PM
- 预期流程：
  - `LLA/LLB` 若处在 `1051`，因 `pm2Has=1 && armB_has=1`，不再原地等，而是回 `950`
  - 回到 `950` 后，因为此时本侧若已有 `ReturnCompleted`，先走 `5000`；若仅有 `ReturnPending`，则先走 `2000`
  - `task0` 优先完成“放回 LL -> 下料到 LP”
  - 待 `PM2` 中的 `task1` 工艺完成后，再发生下一轮交互片
  - 只有回片/下料链路处理完，本侧才重新去处理 `task2/task3` 的 `Pending -> 1000`
- 目标：
  - 避免线程卡死在 `1051`
  - 避免 `task2/task3` 抢在 `task0` 回片/下料之前继续预取
  - 保持“回片/下料优先，待加工片最后处理”的节拍

### 7. 日志增强，便于验证是否真的回到了回片链路
- 文件：`d:\HLPrj\HL\device\src\slot_transfer_cycle_vtm_widget.cpp`
- 修改点：`LLA/LLB` 的 `case 1051` 和必要时 `case 950`
- 变更内容：
  - 在从 `1051` 回退到 `950` 时打印一次明确日志，包含：
    - `pm2Has`
    - `armA_has`
    - `armB_has`
    - 当前 LL 名称
  - 保持 `case 950` 原有流程日志，便于串联验证“1051回退 -> 950 -> 2000/5000”。
- 原因：
  - 用户要现场跑节拍，需要能从日志直接看出是否发生了预期的流程切换。

## Assumptions & Decisions
- 决策：`case 1051` 不直接跳到 `2000`，统一回 `950` 再分流。
- 决策：`case 950` 的优先级按用户要求固定为 `ReturnCompleted > ReturnPending > Pending`。
- 决策：`PM2有片 + WTR任一手有片` 被定义为“禁止继续预取下一片，并优先让回片/下料链路推进”的节拍信号。
- 假设：`desiredArm` 已占用时继续停留在 `1051` 是可接受的，因为此时强行回 `950` 也未必能推进，仍需等待 `WTR` 占用释放。
- 范围外：
  - 不修改 `PM2` 线程的交换片状态机。
  - 不修改 `Robot` 线程底层 `GETOBJECT/PUTOBJECT` 命令逻辑。
  - 不引入新的 task 状态或新的全局互锁。

## Verification Steps
- 静态验证：
  - 检查 `LLA/LLB case 950` 的判断顺序已调整为 `ReturnCompleted -> ReturnPending -> Pending -> 10`。
  - 检查 `LLA/LLB case 1051` 的“PM2有片 + 手上有片”分支已从 `Sleep+break` 改为设置 `auto_step = 950`。
  - 确认 `desiredArm` 占用保护仍保留，未误删。
  - 运行诊断，确保 `slot_transfer_cycle_vtm_widget.cpp` 无新增错误。
- 日志验证：
  - 复现用户描述场景：`task0` 工艺完成、`task1` 在 `PM2`、`WTR` 某手持工艺后片。
  - 确认日志出现“1051回退到950优先处理回片/下料”的记录。
  - 确认随后进入 `950 -> 5000` 或 `950 -> 2000`，而不是持续停在 `1051`。
- 节拍验证：
  - 工艺完成片能优先回到 `LLA/LLB` 并继续下料到 `LP`。
  - 回片/下料完成后，线程还能重新回到取片链路，继续从 `LLA/LLB` 取下一片。
  - 不再出现“1051 条件长期成立导致卡死”的现象。
