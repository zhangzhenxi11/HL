我理解您的情况了。如果 `QTimer` 替代方案也没有起作用（可能是因为 Lambda 捕获上下文的问题或者包含头文件的问题，或者 Qt 版本实在太老对 `QTimer::singleShot` 的 Lambda 重载支持也不完整），那么最稳妥、最兼容旧版 Qt (5.x 早期版本甚至 Qt 4) 的方式确实是**不使用 Lambda**，而是定义**槽函数（Slots）**或**可调用方法（Q_INVOKABLE）**，然后通过字符串形式调用 `invokeMethod`。

**您的提议是正确的**：将 Lambda 中的逻辑提取为 `QPmRecipeWidget` 类中的私有槽函数，然后使用 `QMetaObject::invokeMethod(this, "functionName", Qt::QueuedConnection, Q_ARG(Type, val)...)` 来调用。

### 修改计划

1.  **在 `QPmRecipeWidget` 类中添加私有槽函数 (`device/include/pm_recipe_widget.h`)**:
    *   `void updateSequenceRowHighlight(int pmIndex, int row, QColor color);`
    *   `void updateInnerColumnHighlight(int pmIndex, int col, QColor color);`
    *   `void updateInnerTableLoad(int pmIndex, int row);`
    *   `void updateCycleStoppedState();`

2.  **在 `device/src/pm_recipe_widget.cpp` 中实现这些槽函数**:
    *   将原本 Lambda 中的逻辑移动到这些函数中。这些函数内部直接调用 `d->highlightSequenceRow` 等。

3.  **替换调用点**:
    *   将 `QTimer::singleShot(..., [=](){...})` 替换为 `QMetaObject::invokeMethod(this, "updateSequenceRowHighlight", Qt::QueuedConnection, Q_ARG(int, selectedIndex), Q_ARG(int, sIdx), Q_ARG(QColor, Qt::green));` 等形式。

**注意**：`QColor` 需要注册元类型才能在 `Q_ARG` 中使用（通常 Qt GUI 类型已经注册，但为了保险，可以在构造函数中调用 `qRegisterMetaType<QColor>("QColor");`）。或者，为了简单起见，可以传 `int` 或 `QString` 代表颜色，或者直接在槽函数里构造颜色。这里我们直接传递 `QColor` 应该是没问题的。

### 详细变更

**头文件 (`pm_recipe_widget.h`)**:
```cpp
private slots:
    void updateSequenceRowHighlight(int pmIndex, int row, QColor color);
    void updateInnerColumnHighlight(int pmIndex, int col, QColor color);
    void doLoadRecipeToInnerTable(int pmIndex, int row);
    void onCycleStoppedState();
```

**源文件 (`pm_recipe_widget.cpp`)**:
*   实现上述 Slot。
*   在 `onStartCycle` 的线程循环中：
    *   `highlightSequenceRow` 调用改为 `invokeMethod(..., "updateSequenceRowHighlight", ...)`。
    *   `highlightInnerColumn` 调用改为 `invokeMethod(..., "updateInnerColumnHighlight", ...)`。
    *   `loadRecipeToInnerTable` 调用改为 `invokeMethod(..., "doLoadRecipeToInnerTable", ...)`。
    *   结束时的 UI 状态更新改为 `invokeMethod(..., "onCycleStoppedState", ...)`。

这种方式是绝对标准的 Qt 跨线程调用方式，兼容所有 Qt 5 版本。

我将先修改头文件，再修改源文件。