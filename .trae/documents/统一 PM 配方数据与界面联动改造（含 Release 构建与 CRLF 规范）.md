## 关键理解与约束

* PM腔参数界面固定 4 行，分别代表 PM1/PM2/PM3/PM4。

* “工艺次数”来自 PM腔参数界面对应列，是源数据；电机运动参数（Tab 中每个 PM 的表）行数必须与该“工艺次数”保持一致。

* 增加/删除按钮保持原有位置与使用习惯，但会自动同步“工艺次数”，确保二者一致。

## 新 JSON 格式（以 PM 为单位）

* 文件名：pm\_recipe\_parameters.json（保持不变）。

* 结构：

```json
{
  "PM1": {
    "params": {
      "take_position_mm": 60.0,
      "rotation_angle_deg": 120,
      "process_count": 3,
      "rotate_position_mm": 80.0,
      "process_position_mm": 100.0,
      "process_time_min": 10.0
    },
    "motors": [
      { "lifting_axis_acce1": 0.1, "lifting_axis_acce2": 0.2, "lifting_axis_acce3": 0.3, "lifting_axis_acce4": 0.4,
        "rotating_axis_acce1": 0.1, "rotating_axis_acce2": 0.2, "rotating_axis_acce3": 0.3, "rotating_axis_acce4": 0.4 },
      { ... },
      { ... }
    ]
  },
  "PM2": { ... },
  "PM3": { ... },
  "PM4": { ... }
}
```

* 说明：

  * params 对应 pm\_recipe\_widget.ui 的 PM腔参数行（每 PM 一个对象）。

  * motors 的数组长度严格等于 params.process\_count；保存时强制对齐（超出截断，不足补零行）。

  * 兼容旧格式：若发现旧字段集合（仅有 lifting/rotating），则作为 motors 读取；params 按 UI 当前值填充。

## 代码改动与接口

* 数据模型（pm\_recipe\_widget.h）：

  * struct PMMotorRow { 八个 double 字段 }。

  * struct PMParams { double take\_position\_mm, rotate\_position\_mm, process\_position\_mm, process\_time\_min; int rotation\_angle\_deg; int process\_count; }。

  * struct PMRecipeConfig { PMParams params; std::vector<PMMotorRow> motors; }。

  * std::map\<std::string, PMRecipeConfig> pmRecipeConfigMap\_;

  * 方法：std::map\<std::string, PMRecipeConfig>& getPMRecipeConfigMap();

* 载入 onLoadParameters：

  * 读取每个 PM 的 params，设置 pm\_cavity\_param\_edit\_tbw 对应行；读取 motors，设置每个 Tab 的表。

  * 若 motors 行数与 process\_count 不一致，按 process\_count 进行扩缩并刷新界面。

* 保存 onSetParameters：

  * 从 pm\_cavity\_param\_edit\_tbw 抽取每 PM 的 params（含 process\_count）。

  * 从各 Tab 抽取 motors；再按 process\_count 进行扩缩（截断/补齐），写入 pmRecipeConfigMap\_ 并序列化到 JSON。

* 清空 onClearParameters：

  * 清空 pm\_cavity\_param\_edit\_tbw 的数值为默认；各 Tab 的表行数重置为 0；pmRecipeConfigMap\_ 清空。

* 增加/删除：

  * 仍作用于当前 Tab 的电机参数表；完成后自动更新对应 PM 的 process\_count，使其与当前 Tab 行数一致（保持一致性）。

## UI 接线（按 pm\_recipe\_widget.ui 当前命名）

* 顶部：add\_an\_item\_pbt、delete\_the\_selected\_item\_pbt、clear\_prams\_pbt、load\_prams\_pbt、save\_prams\_pbt。

  * add/delete → 当前 Tab 电机参数表增删行，并同步该 PM 的 process\_count。

  * clear/load/save → 同时作用两个界面并使用新 JSON 格式。

* PM腔参数列映射：

  * 第 1 列：PM腔（只读，PM1..PM4）。

  * 第 2 列：take\_position\_mm（QDoubleSpinBox）。

  * 第 3 列：rotation\_angle\_deg（QComboBox：0/60/120/…/360）。

  * 第 4 列：process\_count（QSpinBox）。

  * 第 5 列：rotate\_position\_mm（QDoubleSpinBox）。

  * 第 6 列：process\_position\_mm（QDoubleSpinBox）。

  * 第 7 列：process\_time\_min（QDoubleSpinBox）。

* 电机参数：每 Tab 保持 8 列（liftingAcce1..4、rotatingAcce1..4）。

## 启动/暂停/当前次数联动

* 当前 PM = TabWidget 当前页；

* onStartCycle：

  * 先保存一次确保数据最新；读取当前 PM 的 params.process\_count 作为循环次数上限；

  * 循环内按 motors\[i] 的 8 参数执行动作；更新右侧 “当前次数” 显示为 i+1 / process\_count；

* onStopCycle：不变；UI 使能按线程状态更新。

## 对接调度 executePMxTransfer

* 暴露 getPMRecipeConfigMap()，由 QSlotTransferCycleVTMWidget 在 executePMxTransfer 中读取当前 PM 的 params + motors。

* 保持后期优化空间（可加 recipeUpdated() 信号）。

## CRLF 与编码

* 我将对所有修改/新增的 .h/.cpp 保持 Windows CRLF；

## Release 构建

* VS：人工在 VS 中切换 Release 编译。

## 验证

* 载入/保存：PM1 设置 process\_count=N 时，PM1 Tab 行数= N；保存后的 JSON motors 数量=N；行尾为 CRLF。

* 启动：当前次数显示为 1..N，暂停恢复按钮状态正确。

请确认该方案，确认后我将开始编码落地、保证 CRLF，并以 Release 完成编译验证。
