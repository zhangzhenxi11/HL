# PM2 Process->Rotate 首次到达即工艺优化计划

## Summary
- 目标：将当前 PM2 spindle 流程中“工艺等待（Pre/Dep/Post）”触发时机从“离开 Process 去 Rotate 前”调整为“每次到达 Process 面后立即触发”，满足 `第一次达到 Process 就算第一次工艺`，之后每次 `Rotate -> Process` 返回继续累计工艺次数。
- 范围：仅调整 `QPmRecipeWidget::startPmMotorRun` 中与 `Process/Rotate` 相关的运行时逻辑与日志，不改 UI 参数结构，不改配方文件结构。
- 成功标准：日志顺序从旧逻辑 `... Rotate->Process(不工艺) -> Process->Rotate(工艺)` 变为 `... Rotate->Process(工艺) -> Process->Rotate(旋转) -> Rotate->Process(工艺)`，且 Step 序号与工艺次数一致。

## Current State Analysis
- 主执行逻辑位于 `d:\HLPrj\HL\device\src\pm_recipe_widget.cpp`，`startPmMotorRun` 的线程循环负责 Step/内表列执行。
- 现状关键点：
  - `from == "Process" && to == "Rotate"` 分支中，先执行 Pre/Dep/Post 等待，再执行旋转并升回 Process（`pm_recipe_widget.cpp` 约 1233-1334 行）。
  - `from != to && to != "Transfer"` 的标准 Z 轴移动分支只负责移动与 `notifyAxisArrivedProcess`，未触发工艺等待（约 1337-1358 行）。
  - 这会导致“首次到达 Process 面”未立即工艺，而是在下一次离开 Process 时才工艺，不符合当前需求。
- 参数与校验现状：
  - `process_count` 作为全局工艺次数用于时间拆分（`avgTime/lastTime`）与 `Process->Rotate` 内表列数一致性校验（约 824-833、1060-1071 行）。
  - 现有 PM2 日志已体现旧顺序（用户给出的 `PM2_spindle日志.txt`）。

## Proposed Changes

### 1) 提取“执行一次工艺等待”公共逻辑
- 文件：`d:\HLPrj\HL\device\src\pm_recipe_widget.cpp`
- 做法：
  - 在 `startPmMotorRun` 的线程 lambda 内，抽出一个局部可复用流程（可为局部 lambda），统一执行：
    - `MaximumPlaneLevelSignal` 检查
    - `Pre Process Wait (soak)`
    - `Process Wait (Dep)`（按工艺次数索引计算 waitTime）
    - `Post Process Wait (time7)`
    - stopRequested/超时等待中断处理
  - 目标：避免同样的工艺计时代码在分支中散落，减少后续维护风险。

### 2) 将工艺触发点改为“到达 Process 后立即执行”
- 文件：`d:\HLPrj\HL\device\src\pm_recipe_widget.cpp`
- 做法：
  - 在标准 Z 移动分支中，当 `to == "Process"` 且移动成功并 `notifyAxisArrivedProcess` 后，立即调用“执行一次工艺等待”逻辑。
  - 在 `from == "Process" && to == "Rotate"` 分支中移除原先 Pre/Dep/Post 执行段，仅保留旋转相关动作（以及必要的面位保护），使其职责变为“从 Process 离开到 Rotate 并返回 Process”中的运动部分，不再承担工艺计时入口。
- 结果：
  - 首次 `Rotate -> Process` 到达后立即工艺（满足“第一次到达即第一次工艺”）。
  - 后续每次旋转回到 Process 均立即工艺，形成累计。

### 3) 明确工艺计数索引与日志步号
- 文件：`d:\HLPrj\HL\device\src\pm_recipe_widget.cpp`
- 做法：
  - 引入本次 Step 内独立“工艺完成计数”变量（例如 `processExecCount`），在每次成功完成一次 Process 工艺后递增。
  - `Process Wait (Dep): ... (Step x/y)` 中 `x` 使用 `processExecCount + 1`，`y` 使用 `processCount`。
  - 超出 `processCount` 时采用保护策略（默认夹到 `lastTime`，并记录告警日志），防止配方异常导致索引越界语义混乱。
- 原因：
  - 当前代码直接用内表列索引 `i` 打印工艺 Step，和“到达 Process 次数”并非严格同一概念，调整后可确保日志与需求语义一致。

### 4) 保持现有配置/校验兼容
- 文件：`d:\HLPrj\HL\device\src\pm_recipe_widget.cpp`
- 做法：
  - 保留 `Process->Rotate` 内表列数与 `process_count` 的现有校验逻辑不变（约 824-833 行），避免影响既有配置习惯。
  - 不改 JSON 字段、不改 UI 控件、不改 `PMParams` 结构，仅调整运行时触发时机。

## Assumptions & Decisions
- 决策 1：工艺次数定义为“到达 Process 并完成一次 Pre/Dep/Post”计 1 次，不再以“离开 Process 前”计次。
- 决策 2：首次工艺由首次 `to == Process` 成功到位触发；旋转动作本身不触发工艺，只负责把腔体带回工艺面。
- 决策 3：若 Process 面位信号失败，记录日志并按现有策略中断/跳过，不新增 UI 交互弹窗。
- 假设：配方步骤中存在能到达 Process 的路径（常见为 `Rotate -> Process`），否则不会发生工艺等待。

## Verification
- 代码级静态检查：
  - 确认 `Process Wait (Dep)` 仅在 `to == "Process"` 到位后触发；
  - 确认 `from == "Process" && to == "Rotate"` 分支不再重复触发工艺等待，避免双计数。
- 运行日志验收（PM2）：
  - 期望看到首次到达 Process 后即出现：
    - `Pre Process Wait (soak): ... (Step 1/N)`
    - `Process Wait (Dep): ... (Step 1/N)`
    - `Post Process Wait (time7): ... (Step 1/N)`
  - 下一次 `Rotate -> Process` 后出现 `Step 2/N`，依次递增。
  - 不应再出现“首次达到 Process 未工艺，下一步才 Step 1/N”现象。
- 回归检查：
  - `to == Transfer` 分支旋转回安全角逻辑不受影响；
  - `notifyAxisArrivedProcess/notifyAxisDepartedProcess` 调用顺序保持一致；
  - `stopRequested` 下流程可及时退出，不引入死等。
