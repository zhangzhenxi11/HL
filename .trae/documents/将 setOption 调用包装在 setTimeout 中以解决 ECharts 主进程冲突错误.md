错误信息 `js: [ECharts] setOption should not be called during main process` 通常发生在 ECharts 渲染过程（或内部更新循环）尚未完成时，又同步调用了 `setOption`。

### 错误分析
1.  **触发原因**：
    *   这个错误常见于在 ECharts 的事件回调中直接调用 `setOption`，或者在某些高频更新场景下，上一次渲染还没结束，下一次更新就来了。
    *   在我们的场景中，可能是 `onDataReceived` 被调用的频率过高（200ms 一次），或者与初始化的 `myChart.setOption(option)` 发生了冲突。

2.  **具体场景**：
    *   `line-stack.html` 底部有一次初始调用：`myChart.setOption(option);`。
    *   随后 C++ 定时器（200ms）触发 `onSimulateTest` -> 调用 `onDataReceived` -> 调用 `myChart.setOption(option, true);`。
    *   如果页面刚加载完，初始渲染正在进行中，此时 C++ 立即注入了新的数据更新请求，可能会导致冲突。

### 解决方案
我们需要确保 `setOption` 的调用是异步的，或者避免冲突。最简单且有效的方法是将 `setOption` 的调用放入 `setTimeout` 中，使其在下一个 JS 事件循环中执行。

**计划修改 `line-stack.html`**：
将 `onDataReceived` 中的 `myChart.setOption(option, true);` 修改为：
```javascript
setTimeout(function() {
    myChart.setOption(option, true);
}, 0);
```
这样可以确保当前的执行栈清空后再执行图表更新，避免“main process”冲突。

我将立即执行此修改。