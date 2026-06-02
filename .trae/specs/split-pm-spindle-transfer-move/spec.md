# PM Spindle Process->Transfer 两段定位与角度互锁 Spec

## Why
当前 Process->Transfer 的定位逻辑在 Z 轴已离开旋转位后才尝试执行 R 轴旋转，导致“取放片前必须到达指定角度（lift pin 角度 IO）”这一前置条件无法稳定满足。

## What Changes
- 在 `pm_recipe_widget.cpp` 的运行配方逻辑中，对 `from == "Process" && to == "Transfer"` 的步骤执行流程进行调整：
  - Z 轴定位拆分为两段：`Process -> Rotate(Z)` 与 `Rotate(Z) -> Transfer(Z)`
  - 在旋转位（Rotate 面位）读取并检查“指定角度 IO”，未到位则执行旋转到该角度
- 旋转命令执行后，使用 `getPMCavityRAxleLocation()` 做到位校验；未到位则继续旋转重试，最多 3 次，仍未到位则报错并停止所有动作
- 下降到 Transfer 取放片位后，再次基于 `getPMCavityRAxleLocation()` 做最终到位校验
- 失败场景下保持现有的停止/高亮/异常抛出风格，不改变其它步骤的执行语义

## Impact
- Affected specs: PM spindle 最终取放片互锁；Process 结束后的 Transfer 过渡动作
- Affected code: [pm_recipe_widget.cpp](file:///d:/HLPrj/HL/device/src/pm_recipe_widget.cpp)；间接依赖 [fortrend_pm_cavity_subsystem.h](file:///d:/HLPrj/HL/device/include/PMCavity/fortrend_pm_cavity_subsystem.h) 暴露的 IO 读取接口

## ADDED Requirements
### Requirement: Process->Transfer 两段定位
当执行序列步骤 `from == "Process" && to == "Transfer"` 时，系统 SHALL 按两段 Z 轴定位执行并在旋转位插入角度互锁。

#### Scenario: 成功（角度已满足）
- **WHEN** 执行 `Process -> Transfer` 步骤
- **THEN** 先将 Z 轴定位到 Rotate 位置（`cfg.params.rotate_position_mm`）
- **AND** 在 Rotate 面位检测为真时，读取 lift pin 指定角度 IO（`getPmLiftPinSafeAnglePos()`）并判定角度已到位
- **AND** 直接将 Z 轴继续定位到 Transfer 取放片位置（`cfg.params.take_position_mm`）
- **AND** 在 Transfer 位置再次校验当前 R 轴坐标 IO（`getPMCavityRAxleLocation()`）满足目标角度

#### Scenario: 成功（角度未满足，需补旋）
- **WHEN** 执行 `Process -> Transfer` 步骤且在 Rotate 位读取到角度未到位
- **THEN** 系统在 Rotate 面位执行 `createRotatingActionCommand(targetAngle)` 完成旋转到目标角度
- **AND** 旋转命令执行完成后立即对比 `getPMCavityRAxleLocation()` 与目标角度；若未到位则继续旋转重试，最多 3 次
- **AND** 旋转到位后再下降到 Transfer 取放片位置
- **AND** 在 Transfer 位置再次校验 `getPMCavityRAxleLocation()` 与目标角度一致（允许项目约定的容差）

#### Scenario: 失败（未在旋转位）
- **WHEN** `Process -> Transfer` 步骤执行到“需要检查/执行旋转”的阶段但 Rotate 面位检测为假（`getRotatingimumPlaneLevelSignal() == false`）
- **THEN** 系统 SHALL 将该步骤视为失败并停止当前执行（与现有异常/高亮逻辑一致）

#### Scenario: 失败（最终角度校验失败）
- **WHEN** 已完成旋转并下降到 Transfer 取放片位置
- **THEN** 若 `getPMCavityRAxleLocation()` 与目标角度不匹配（超出容差），系统 SHALL 将该步骤视为失败并停止当前执行
 
#### Scenario: 失败（最多 3 次仍旋转不到位）
- **WHEN** 在 Rotate 位执行旋转命令后进行到位校验，连续 3 次尝试后仍未到位（超出容差）
- **THEN** 系统 SHALL 报错并停止所有动作（与现有异常/高亮逻辑一致）

## MODIFIED Requirements
### Requirement: Process->Transfer 的“单次定位移动”语义
原先将 `Process -> Transfer` 视为一次 Z 轴定位后再尝试旋转的流程，修改为：在到达 Transfer 前必须先进入 Rotate 位并完成角度互锁后才允许下降到 Transfer。

## REMOVED Requirements
无
