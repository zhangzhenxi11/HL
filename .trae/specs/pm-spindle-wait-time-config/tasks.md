# Tasks

## 1. 扩展PMMotorRow数据结构
- [ ] 在 `pm_recipe_widget.h` 的 `PMMotorRow` 结构中新增两个Wait时间参数（每次工艺可能不同）
  - pre_process_wait_s (前处理Wait时间/soak)
  - post_process_wait_s (后处理Wait时间/time7)

## 2. 修改电机运动参数表格(innerTable)
- [ ] 在 `pm_recipe_widget.cpp` 的 `initInnerTable` 函数中将行数从8行扩展到10行
- [ ] 行8：前处理Wait时间/s (soak)
- [ ] 行9：后处理Wait时间/s (time7)
- [ ] 每列（每次工艺）可以设置不同的Wait时间

## 3. 修改数据保存/加载逻辑
- [ ] 在 `saveInnerTableToRecipe` 中保存两个Wait时间参数
- [ ] 在 `loadRecipeToInnerTable` 中加载两个Wait时间参数

## 4. 修改运动流程执行逻辑
- [ ] 在 `startPmMotorRun` 函数中实现每次工艺的完整时间执行
  - 从第i列读取前处理Wait时间(pre_process_wait_s)并执行
  - 执行工艺时间(Dep) - 使用原有"总工艺时间/工艺次数"逻辑
  - 从第i列读取后处理Wait时间(post_process_wait_s)并执行

## 5. 测试验证
- [ ] 测试每次工艺的Wait时间配置是否能正确保存和加载
- [ ] 测试运动流程中时间执行顺序是否正确：前处理Wait(第i次) -> 工艺时间(第i次) -> 后处理Wait(第i次)

# Task Dependencies
- Task 2 依赖 Task 1（先有数据结构）
- Task 3 依赖 Task 1 和 Task 2
- Task 4 依赖 Task 1
- Task 5 依赖 Task 4