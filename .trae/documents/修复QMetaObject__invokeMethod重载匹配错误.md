收到，Qt 5.6.3 确实**不支持**在 `QMetaObject::invokeMethod` 中使用 Lambda 表达式（该功能是在 Qt 5.10 引入的）。

但是，Qt 5.4 引入了对 `QTimer::singleShot` 的 Lambda 支持。
所以，**只替换那些使用 Lambda 的 `invokeMethod` 为 `QTimer::singleShot`** 是正确的解决方案。标准槽的字符串形式调用保持不变。

### 最终计划：

1.  **引入头文件**：
    *   在 `device/src/pm_recipe_widget.cpp` 中添加 `#include <QTimer>`。

2.  **替换 Lambda 调用**：
    *   将所有 `QMetaObject::invokeMethod(this, [=](){ ... });`
    *   替换为 `QTimer::singleShot(0, this, [=](){ ... });`

    具体涉及的地方有：
    *   高亮 Sequence Row: `d->highlightSequenceRow(...)`
    *   高亮 Inner Column: `d->highlightInnerColumn(...)`
    *   加载内表: `loadRecipeToInnerTable(...)`
    *   停止信号: `cycleStopped` (如果是通过 Lambda 调用的)

3.  **保留标准槽调用**：
    *   `QMetaObject::invokeMethod(..., "setValue", ...)` 保持不变。
    *   `QMetaObject::invokeMethod(..., "setEnabled", ...)` 保持不变。

这样既解决了编译错误，又保证了线程安全。