# Tasks

- [x] 1. 梳理并定位 Process->Transfer 现有执行路径
  - [x] 确认 `pm_recipe_widget.cpp` 中 `from == "Process" && to == "Transfer"` 当前的 Z/R 动作顺序与问题点（旋转位互锁在 Z 离开后才触发）
  - [x] 确认目标角度来源（`getPmLiftPinSafeAnglePos()`）与最终校验来源（`getPMCavityRAxleLocation()`）

- [x] 2. 实现 Process->Transfer 两段 Z 定位 + 旋转角度互锁
  - [x] 对 `Process->Transfer` 分支绕过“标准 From!=To 的单段 Z 移动”，改为：
    - [x] Z 定位到 Rotate（`cfg.params.rotate_position_mm`）
    - [x] 检查 `getRotatingimumPlaneLevelSignal()`，失败则按现有风格中止
    - [x] 检查角度是否满足，不满足则执行 `createRotatingActionCommand(targetAngle)`
    - [x] 旋转命令执行后立刻用 `getPMCavityRAxleLocation()` 校验；未到位则继续旋转，最多 3 次尝试，仍未到位则报错并停止
    - [x] Z 定位到 Transfer（`cfg.params.take_position_mm`）
    - [x] 在 Transfer 位置再次校验 `getPMCavityRAxleLocation()` 是否到位（含容差策略）
  - [x] 保持 stopRequested/高亮/异常处理与现有一致

- [x] 3. 回归验证与构建验证
  - [x] 验证 Process->Rotate 与其它 from/to 步骤逻辑不受影响
  - [x] 通过 IDE 诊断确保无编译错误

# Task Dependencies
- Task 2 依赖 Task 1（先明确现状与目标值来源）
- Task 3 依赖 Task 2
