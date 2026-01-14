我将按照您的要求，采用“主从表”设计来实现 PM 配方界面。

### 1. 修改 `device/include/pm_recipe_widget.h`

* **重构数据结构**:

  * 保留 `PMMotorRow` (存储单次运动的 Acc/Dec/Jerk/Vel)。

  * 新增 `PMRecipeDetails` 结构体：包含 `process_count` (int) 和 `motors` (`std::vector<PMMotorRow>`)。这对应“内表”的数据。

  * 新增 `PMStep` 结构体：包含 `from_pos` (起始), `to_pos` (目的), `recipe_name` (配方名)。这对应“外表”的一行。

  * 更新 `PMRecipeConfig`：

    * `positions`: 增加 `lift_pin_position_mm`。

    * `steps`: `std::vector<PMStep>` (整个流程序列)。

    * `recipes`: `std::map<std::string, PMRecipeDetails>` (配方库)。

### 2. 修改 `device/src/pm_recipe_widget.cpp`

* **界面布局改造**:

  * **顶部**: 增加 "LiftPin Position" 设置。

  * **中部 (外表 - Sequence)**: 显示流程步骤。

    * 列：起始位置、目的位置、配方名称 (显示为按钮或下拉框)。

    * 功能：点击某行，下方内表刷新为对应的配方参数。

    * 操作：支持“增加步骤”、“删除步骤”。

  * **下部 (内表 - Recipe Details)**: 显示选中配方的详细参数。

    * **行列转置**: 行固定为 8 个参数 (Z/R 的 Acc/Dec/Jerk/Vel)，列对应 `step_1` 到 `step_N`。

    * 操作：支持“增加 Step (列)”、“删除 Step (列)”。

* **交互逻辑**:

  * 实现主从表联动：点击外表行 -> 读取 `recipe_name` -> 从 Map 中获取 `PMRecipeDetails` -> 填充内表。

  * 内表修改 -> 更新 Map 中的数据。

* **执行逻辑 (onStartCycle)**:

  * 遍历 `steps` 列表。

  * 对于每一步，根据 `recipe_name` 获取参数。

  * 判断逻辑：如果 `process_count > 1` (即内表有多列)，则执行循环逻辑 (对应 Process 阶段)；否则执行单次移动逻辑。

* **数据持久化**: 更新 JSON 结构以匹配新的 schema。

### 3. 修改 `device/src/Data/fortrend_data_subsystem_widget.cpp`

* 注释掉加速度曲线更新代码。

