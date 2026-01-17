用户反馈了具体的 JS 错误日志，这直接指向了问题的核心。

### 错误分析

1.  **`[ECharts] Can't get DOM width or height`**:
    *   **原因**: ECharts 初始化时，容器（`div id="container"`）的宽度或高度为 0。这通常发生在界面刚创建但尚未显示（Show）或布局尚未完成时。
    *   **关联问题**: 这解释了为什么“界面不是全展开的”或“偏小”。如果 WebView 初始化时没有正确的大小，ECharts 渲染就会出错或缩成一团。
    *   **解决方案**:
        *   确保在 `QWebEngineView` 大小确定后再初始化 ECharts，或者在 `resize` 事件中调用 `myChart.resize()`（代码中已有 `window.addEventListener('resize', myChart.resize)`，但可能初始加载时尺寸不对）。
        *   在 `fortrend_data_subsystem_widget.cpp` 中，构造函数里直接加载了 HTML。此时 Widget 可能还没显示，大小为 0。
        *   **关键修复**: 不要依赖 `resizeEvent` 手动调整大小（这会导致时序问题），而是依赖 Qt 的布局系统。同时，确保 HTML 加载完成后，ECharts 能正确获取尺寸。

2.  **`gate XX series not exists`**:
    *   **原因**: `fortrend_datahistory_subsystem_widget.cpp` 中，代码试图更新图例（Legend）或数据，但 Series 名称不匹配。
    *   **历史遗留**: `DataHistoryWidget` 使用了 `gate 01` 等名称，但 HTML 模板可能还是旧的 `line-stack.html` 或者 `history-line-stack.html` 中的配置不匹配。
    *   **解决方案**: 检查 `DataHistoryWidget::httpUpdate` 发送的数据结构与 HTML 中 `onDataReceived` 的期望是否一致。

### 综合解决方案

针对用户提到的“界面偏小”和“偶发无法点击”（可能是 JS 报错导致的死循环或阻塞），以及新的 JS 错误，更新计划如下：

#### 1. 修复 ECharts 尺寸为 0 问题 (DataWidget & DataHistoryWidget)
*   **DataWidget (`fortrend_data_subsystem_widget.cpp`)**:
    *   **必须**保留并信任 `QVBoxLayout`。
    *   **删除** `resizeEvent` 中的 `d->tabWidget->resize(...)`。这是多余的，且可能在布局计算前强制设置错误大小。
    *   **确保布局生效**: 构造函数中 `QVBoxLayout` 已经设置了 `this` 为父对象，这很好。但 `d->tabWidget` 需要正确响应。
*   **DataHistoryWidget (`fortrend_datahistory_subsystem_widget.cpp`)**:
    *   **删除** `resizeEvent` 中的 `load()` (会导致无限重载)。
    *   **删除** `resizeEvent` 中的 `setGeometry`。
    *   **添加布局**: 在构造函数中，为 `DataHistoryWidget` 添加一个 `QVBoxLayout`，并将 `d->ui->qweb` 加入布局。这样 `QWebEngineView` 会自动随窗口最大化而填满，无需手动计算。这样能保证 DOM 有正确的大小。

#### 2. 修复 Series Not Exists 错误
*   检查 `fortrend_datahistory_subsystem_widget.cpp` 的 `httpUpdate`。
*   它发送 `lineName[0]` ("gate 01") 等。
*   检查 `Echarts/history-line-stack.html` (需要读取此文件确认)。如果它和 `line-stack.html` 类似，`onDataReceived` 可能需要动态创建 series 或者 update option 时包含 series 定义。
*   错误日志显示 `Legend data should be same with series name`。这意味着代码更新了 Legend，但没有同步更新 Series 的 name，或者初始化时 Option 里没有这些 Series。

### 执行步骤

1.  **修改 `fortrend_data_subsystem_widget.cpp`**: 删除 `resizeEvent` 中的手动 resize 代码。
2.  **修改 `fortrend_datahistory_subsystem_widget.cpp`**:
    *   删除 `resizeEvent` 函数（完全移除，或只保留父类调用）。
    *   在构造函数中添加 `QVBoxLayout` 来管理 `qweb`，确保自动全屏。
3.  **读取 `Echarts/history-line-stack.html`**: 确认 JS 端如何处理数据更新，以便修复 `series not exists` 错误。

这个计划直接回应了 JS 错误（尺寸为0 -> 布局问题；Series错误 -> 数据匹配问题）以及用户的界面显示问题。