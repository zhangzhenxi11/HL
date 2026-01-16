我将执行以下任务：

### 1. 曲线图支持缩放与拖动 (Echarts)

您提到 `history-line-stack.html` 已经支持 `dataZoom`，而当前的 `line-stack.html` 不支持。这可以通过修改 **HTML 文件** 或 **C++代码** 来实现。
考虑到 C++ 代码中 `httpUpdate` 函数每次都会重置 `option` (通过调用 `onDataReceived` 重新设置整个 `option` 对象)，最佳做法是在 C++ 的 `httpUpdate` 函数构造的 JS 代码中包含 `dataZoom` 配置，或者修改 `line-stack.html` 中的 `onDataReceived` 函数以包含默认的 `dataZoom` 配置。

**方案**：直接修改 `Echarts/line-stack.html` 文件，在 `onDataReceived` 函数构造的 `option` 对象中加入 `dataZoom` 配置，使其与 `history-line-stack.html` 保持一致。这样无需修改 C++ 代码中的字符串拼接逻辑，更稳健。

### 2. 运行时 UI 高亮 (C++)

需要在 `onStartCycle` 运行时，实时更新 UI 状态。

*   **修改文件**: `device/src/pm_recipe_widget.cpp`
*   **具体实现**:
    *   **高亮当前配方（Recipe）**: 在 Sequence Table 中，将当前执行的行（Step）背景色设为 **绿色**。
    *   **高亮当前步骤（Step）**: 在 Inner Table 中，将当前执行的列（Step）单元格背景色设为 **绿色**。
    *   **错误指示**: 如果执行失败（Catch 异常块中），将对应行/列设为 **红色**。
    *   **状态清除**: 在开始新循环前或停止时，清除所有表格的背景色。
    *   **实现方式**: 添加辅助函数 `highlightSequenceRow` 和 `highlightInnerColumn`，利用 `QTableWidgetItem::setBackground`。

### 计划步骤：

1.  **修改 `Echarts/line-stack.html`**:
    *   在 `onDataReceived` 函数内的 `option` 对象中添加 `dataZoom` 配置（参考 `history-line-stack.html`）。

2.  **修改 `device/src/pm_recipe_widget.cpp`**:
    *   添加 `highlightSequenceRow` 和 `highlightInnerColumn` 私有辅助函数。
    *   添加 `clearHighlights` 函数。
    *   在 `onStartCycle` 的循环逻辑中插入高亮调用。
    *   在异常处理块中插入红色高亮调用。

我将先修改 HTML 文件，再修改 C++ 代码。