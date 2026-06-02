# Checklist

- [x] `Process -> Transfer` 步骤的 Z 轴定位被拆分为 “先到 Rotate 再到 Transfer”，不再直接 `Process -> Transfer` 一步到位
- [x] 在 Rotate 位检查 `getRotatingimumPlaneLevelSignal()`，为假时按现有错误处理流程中止
- [x] 在 Rotate 位读取 `getPmLiftPinSafeAnglePos()` 作为目标角度，并在角度未到位时执行旋转到该角度
- [x] 每次 `createRotatingActionCommand(targetAngle)` 执行完成后，立刻使用 `getPMCavityRAxleLocation()` 做到位校验；未到位继续旋转，最多 3 次，仍未到位则报错并停止
- [x] 下降到 Transfer 取放片位后，使用 `getPMCavityRAxleLocation()` 与目标角度对比做最终到位校验（含容差策略）
- [x] stopRequested 触发时可中断该步骤，不发生死锁或 UI 卡住
- [x] 通过 IDE 诊断确认无新增编译错误
