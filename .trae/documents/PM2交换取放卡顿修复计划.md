# PM2交换取放卡顿修复计划

## Summary
- 目标：在持续供料（LLA/LLB 连续上料）模式下，修复 PM2 取放决策对 task 绑定不准导致的“误判手臂任务/误判可取放/进入等待”问题，并让日志能明确解释“为何拿在手上等待”。
- 核心原则：PM2 调度不再依赖 `TaskManager::getRobotTaskInfo(arm)` 推断“手上 wafer 属于哪个 task”，改用“PM2 待加工任务列表 + WTR 手臂占用状态 + 任务 arm 字段”做确定性绑定（局部修复，影响面最小）。

## Current State Analysis
- 日志证据（2026-05-28）显示：
  - 17:47:05 后，A 臂已从 LLA 取到待加工片，但没有立刻放 PM2；直到 17:47:56 才放入。
  - 真实原因是 PM2 当时 `haswaferpm=1`，必须先执行交换流程：空臂从 PM2 取出旧片，再把新片放入；取 PM2 的 GETOBJECT 回报耗时长，导致新片在手上等待。
  - 同一段日志中，PM2 打印的 `taskId/taskType` 与实际在途任务不一致（例如把历史 task 当成当前手上 task），证明 `getRobotTaskInfo(arm)` 在多 task 且 arm 复用时不可靠。

## Root Cause
- PM2 侧当前通过 `getRobotTaskInfo(arm)` 取得某 arm 的“任务信息”来判断该臂是“待加工片”还是“已完成片”，但该接口仅按 `t.arm == arm` 返回第一条匹配任务，无法保证对应“当前手上 wafer 的 task”，在并行/流水化时会频繁取错。
- 取错会带来：
  - 错误进入/退出交换逻辑（例：误判“无待加工任务”进入等待）。
  - 错误选择 getArm/putArm（极端情况下会导致放片延迟或状态机紊乱）。

## Proposed Changes
### 1) PM2 调度：用“任务列表 + 手臂占用”确定性绑定 task
- 位置：`slot_transfer_cycle_vtm_widget.cpp` 的 `executePM2Transfer` 及其相关分支（step 200/1010/1030/1060/1070/1090 等）。
- 做法：
  - 获取 `pm2PendingTasks`（待加工）与 `pm2ReturnPendingTasks/ReturnCompletedTasks`（待下料）后，用以下规则选手臂：
    - **待加工放入 PM2**：选择 `wtr->hasObject(arm)==true` 且该 arm 对应的 task 在 `pm2PendingTasks` 内（以 `task.arm` 匹配）。
    - **从 PM2 取出待下料**：选择 `wtr->hasObject(arm)==false` 的空臂，并结合待下料 task 的 `task.arm` 决定优先臂（保持现有交互片策略）。
  - 仅在无法建立绑定时才退回等待，并输出明确原因（例如：有片但找不到匹配的 pending task/return task）。

### 2) PM2 日志增强：把“为什么在等”说清楚
- 在 PM2 step 200/1060/1070 分支打印：
  - haswaferpm / arm占用 / pending任务数量 / return任务数量
  - 本次决策：普通放片 or 交换料（以及 getArm/putArm）
  - 若进入等待：输出阻塞条件（例如“pm2PendingTasks=0 但 arm 有片”这类矛盾）

### 3) 交换流程耗时可观测（不改变硬件动作）
- 在 Robot/WTR 侧（触发交换的请求点）补充耗时统计日志：
  - 记录 GETOBJECT 发起时间、收到 RPS 时间、耗时 ms
  - 用于区分“软件决策导致等待” vs “硬件执行耗时”

## Files & Impact
- 主要改动：
  - `d:\HLPrj\HL\device\src\slot_transfer_cycle_vtm_widget.cpp`
- 可能少量改动（仅用于更清晰日志或复用工具函数时）：
  - `d:\HLPrj\HL\device\src\TaskManager.cpp`（仅在需要补充查询接口或日志辅助时；本计划优先不动）

## Assumptions & Decisions
- 决策：按“局部修复(推荐)”执行，不修改 `getRobotTaskInfo` 的全局行为，避免引入其它线程/模块的不可预期变化。
- 假设：`pm2PendingTasks`/return 任务列表能覆盖 PM2 决策所需的 task 视角；手臂占用状态来自 `wtr->hasObject()` 是可靠的。

## Verification
- 日志验证（必做）：
  - 复现 17:47 类似场景时，PM2 日志应解释清楚是否在做交换、getArm/putArm 的选择依据、以及阻塞原因。
  - 不再出现“PM2无片且没有待加工任务，无法判断取放，等待中...”但同时 `haswaferarmX=1` 且实际有 pending task 的矛盾状态。
- 行为验证（现场）：
  - 连续供料模式下，PM2 在 `haswaferpm=0` 时能及时放片；在 `haswaferpm=1` 时能稳定走交换，不因 task 误判进入长时间等待。
  - 若仍有长耗时等待，耗时统计应表明等待主要来自 GETOBJECT/PUTOBJECT 的硬件执行时间而非调度逻辑。

