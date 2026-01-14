收到，我将更新 PM 配方界面的数据结构和 UI 实现，以支持完整的 S 型曲线参数设置。

### 修改目标
将原本的 `PMMotorRow`（之前可能只包含部分加速度参数）重构为包含完整的运动控制参数：
*   **加速度 (Acceleration)**
*   **减速度 (Deceleration)**
*   **加加速度 (Jerk)**
*   **最大速度 (Velocity)**

由于涉及 Z 轴（升降）和 R 轴（旋转），每行需要包含这两组轴的参数。

### 具体修改计划

1.  **修改头文件 (`pm_recipe_widget.h`)**
    *   更新 `struct PMMotorRow` 定义：
        ```cpp
        struct PMMotorRow {
            // Z轴 (Lifting)
            double lifting_acc = 0.0;
            double lifting_dec = 0.0;
            double lifting_jerk = 0.0;
            double lifting_vel = 0.0;
            
            // R轴 (Rotating)
            double rotating_acc = 0.0;
            double rotating_dec = 0.0;
            double rotating_jerk = 0.0;
            double rotating_vel = 0.0;
        };
        ```

2.  **修改源文件 (`pm_recipe_widget.cpp`)**
    *   **表格初始化 (`initTableWidget`)**：
        *   更新表头 (Headers) 为：
            `Z-Acc`, `Z-Dec`, `Z-Jerk`, `Z-Vel`, `R-Acc`, `R-Dec`, `R-Jerk`, `R-Vel`。
        *   设置列数为 8 列。
    *   **JSON 序列化/反序列化 (`onLoadParameters`, `onSetParameters`)**：
        *   更新 JSON 键名，使其与新结构体成员对应（如 `lifting_acc`, `lifting_dec` 等）。
        *   确保读写逻辑覆盖所有 8 个参数。
    *   **表格填充逻辑**：
        *   更新 `addAnItem` 或表格刷新逻辑，使用新的参数填充单元格。

### 注意事项
*   这会改变 JSON 配置文件的格式。旧的配置文件可能无法正确读取，需要重新生成或手动修改。
*   界面上的 SpinBox 范围和步长将根据参数类型（速度 vs 加速度 vs Jerk）进行合理设置。

我将立即开始修改代码。