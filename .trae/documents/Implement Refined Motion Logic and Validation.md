收到，校验规则已明确：**仅当 Sequence Table 中的步骤为 `from: Process` 且 `to: Rotate` 时，才检查该步骤对应的配方（Recipe）的内表列数（Steps）是否与全局工艺次数一致。**

### 最终计划：

1.  **修改 `device/src/pm_recipe_widget.cpp` 中的 `onSetParameters`**:
    *   **位置校验**: 遍历外表步骤，如果 `from == to` 且 `from != "Rotate"`，弹出警告并阻止保存。
    *   **工艺次数一致性校验**:
        *   遍历外表步骤，找到所有 `from == "Process"` 且 `to == "Rotate"` 的行。
        *   对于这些行，获取其引用的 `Recipe`。
        *   检查该 `Recipe` 的内表列数 (`recipe.process_count` 或 `motors.size()`) 是否等于全局参数中的 `process_count` (界面第3列)。
        *   如果不一致，弹出警告提示：“Step [X] (Process->Rotate) 配方 [Name] 的列数([N]) 与全局工艺次数([M]) 不一致！”，并阻止保存。

2.  **修改 `device/src/pm_recipe_widget.cpp` 中的 `onStartCycle`**:
    *   **通用序列执行**: 严格按 Sequence Table 顺序执行。
    *   **内表循环**: 对每个 Sequence Step，遍历其 Recipe 的所有列。
    *   **运动逻辑分支**:
        *   **特例 `from="Rotate", to="Process"`**:
            1.  下发参数。
            2.  执行 **旋转** (`rotation_angle_deg`)。
            3.  执行 **Z轴移动** (`process_position_mm`)。
        *   **特例 `from="Rotate", to="Rotate"`**:
            1.  下发参数。
            2.  执行 **旋转** (`rotation_angle_deg`)。
        *   **通用 `from != to`** (且非上述特例):
            1.  下发参数。
            2.  执行 **Z轴移动** (目标位置由 `to` 决定：Transfer/LiftPin/Rotate/Process)。
    *   **运行时校验**: 启动前再次执行上述校验逻辑，确保配置安全。

这个计划精确覆盖了您的特定校验需求和通用运动逻辑。