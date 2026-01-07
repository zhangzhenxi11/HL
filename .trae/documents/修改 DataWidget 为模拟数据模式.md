好的，我将保持原有逻辑不变，新增一个模拟测试函数。

### 计划修改内容
1.  **新增 `onSimulateTest` 槽函数**：在 `DataWidget` 类中添加 `onSimulateTest()`。
2.  **模拟逻辑**：该函数内部逻辑与 `onclick` 类似，但数据来源改为 `qrand()` 生成的随机数，用于更新三个图表。
3.  **定时器连接**：在构造函数中，暂时将定时器的 `timeout` 信号改为连接到新的 `onSimulateTest` 槽函数，以便您可以直接看到效果。
    *   *或者*：我不修改默认连接，而是为您添加一个临时按钮或者在构造函数里直接启动一个专门用于测试的定时器连接到 `onSimulateTest`。
    *   *根据您的意图*：既然要“自动 start”，我会修改构造函数，使其**默认连接到模拟函数并启动**。原有的 `onclick`（真实数据逻辑）保留不动。

**最终确认方案**：
1.  在 `DataWidget` 头文件添加 `void onSimulateTest();`。
2.  在 .cpp 中实现 `onSimulateTest()`：生成随机 Acc/Vel/Pos 数据并调用 `httpUpdate`。
3.  在构造函数中：
    *   注释掉原有的 `connect(..., onclick)`。
    *   添加 `connect(..., onSimulateTest)`。
    *   添加 `d->timer->start(200);`。

这样既保留了真实逻辑代码，又能立即看到模拟效果。