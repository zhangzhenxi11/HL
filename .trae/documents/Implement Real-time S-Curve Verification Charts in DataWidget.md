我将通过代码编程的方式实现您的需求，您**无需手动修改 .ui 文件**。

### 1. 修改 `QPmRecipeWidget` (添加信号)
*   **增加信号**: 添加 `cycleStarted(const std::string& pmName)` 和 `cycleStopped()` 信号。
*   **触发信号**:
    *   在 `onStartCycle()` 函数开始时发送 `cycleStarted`，将当前选中的 PM 名称传递出去。
    *   在工艺循环线程结束（或被停止）时发送 `cycleStopped`。

### 2. 重构 `DataWidget` (核心实现)
*   **依赖注入**: 修改构造函数接收 `std::shared_ptr<IKernel>`，以便根据 PM 名称动态获取对应的子系统。
*   **UI 编程构建**: 在构造函数中**通过代码**创建 `QTabWidget` 和 3 个 `QWebEngineView`，不再依赖 `.ui` 文件的布局。
    *   **Tab 1 (加速度)**: 显示 Z轴/R轴 加速度。
    *   **Tab 2 (速度)**: 显示 Z轴/R轴 速度。
    *   **Tab 3 (位置)**: 显示 Z轴/R轴 位置。
*   **新增槽函数**:
    *   `onCycleStart(const std::string& pmName)`: 接收信号，获取对应 PM 子系统指针，清空图表，启动定时器 (200ms 刷新率)。
    *   `onCycleStop()`: 停止定时器，停止数据采集。
*   **数据处理**: 修改 `httpUpdate` 逻辑，使其支持向不同 Tab 页的 WebView 发送独立的双轴（Z/R）数据。

### 3. 连接逻辑 (`FortrendDeviceModel`)
*   在 `addMainCompoments` 中，将 `QPmRecipeWidget` 的 `cycleStarted/Stopped` 信号连接到 `DataWidget` 对应的槽函数上。

这样，当您点击“开始”循环时，`DataWidget` 会自动切换到对应的 PM 腔体并开始绘制实时的 S 型曲线。