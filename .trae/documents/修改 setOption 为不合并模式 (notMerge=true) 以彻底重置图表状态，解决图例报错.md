尽管我们已经修正了 JS 函数的参数顺序和 `legend.data` 的更新逻辑，但报错 `js: Z-Pos series not exists...` 依然存在。这非常令人困惑，因为 `legendNames` 和 `datalen` (series) 应该是同步更新的。

### 深入分析
报错信息表明：ECharts 试图显示某个图例项（例如 `Z-Pos`），但在当前的 `series` 列表中找不到 `name` 为 `Z-Pos` 的系列。

**可能性 1：`setOption` 的合并行为 (Merge Behavior)**
ECharts 的 `setOption` 默认是**合并模式** (`notMerge = false`)。
*   如果在初始化时（或之前的更新中）设置了 `legend: { data: ['gate 01', ...] }`，而没有对应的 `series`。
*   当我们调用 `onDataReceived` 更新时，虽然传入了新的 `legend` 和 `series`，但如果 ECharts 内部状态混乱，或者合并逻辑有问题，可能会导致旧的图例项残留，或者新的图例项在某些瞬间找不到对应的系列。
*   **但是**，报错信息中的系列名称是 `Z-Pos` / `R-Pos`，这是**新**的数据名称。说明新的 `legend` 数据生效了，但 ECharts 认为对应的 `series` 还没准备好？

**可能性 2：JS 执行时机与 Series Name 设置**
*   C++ 代码中：
    ```cpp
    jscode += "[{name:'" + zName + "',data: " + lineDataZ + ...
    ```
    这里确实设置了 `name` 属性。
*   JS 代码中：
    ```javascript
    series: datalen
    ```
    直接把对象数组赋给了 `series`。

**最可疑点：JS 数组字符串拼接的语法错误？**
请再看一眼 C++ 代码：
```cpp
jscode += "[{name:'" + zName + "',data: " + lineDataZ + ",type: 'line',stack: 'Total',smooth: true}";
jscode += ",{name:'" + rName + "',data: " + lineDataR + ",type: 'line',stack: 'Total',smooth: true}";
jscode += "])";
```
生成的 JS 类似：
`onDataReceived(..., [{name:'Z-Acc', ...}, {name:'R-Acc', ...}])`
这看起来没问题。

**但是，请注意报错信息中的重复性**：
`js: Z-Pos series not exists...`
`js: R-Pos series not exists...`
`js: Z-Acc series not exists...`
`js: R-Acc series not exists...`
`js: Z-Vel series not exists...`
`js: R-Vel series not exists...`

这说明**所有**的图表（Acc, Vel, Pos）都在报错。这意味着 `series` 数组虽然传进去了，但 ECharts **没有正确识别出 series 的 name**，或者 `series` 根本没有生效。

**为什么 series 会不生效？**
在 JS 中：
```javascript
series: datalen
```
如果 `datalen` 为 `null` 或格式不对，`series` 就为空。
但是 C++ 传的是 `[...]` 数组。

**让我们尝试一种更暴力的 `setOption` 方式**：
使用 `myChart.setOption(option, true)`。
第二个参数 `true` 表示 `notMerge`，即**不合并**，完全重置图表配置。这可以清除任何之前的残留状态，确保图表状态与当前传入的 option 完全一致。这通常能解决很多莫名其妙的更新问题。

**修改计划**：
修改 `line-stack.html` 中的 `myChart.setOption(option)` 为 `myChart.setOption(option, true)`。

此外，为了进一步排查，我将在 JS 中添加 `console.log(JSON.stringify(datalen))` 来查看接收到的 series 数据到底是什么样子的。虽然我看不到控制台，但这是一种标准的调试思路。鉴于只能修改代码，我将直接应用 `notMerge=true` 的修复。

**同时**，检查一下 `DataWidget.cpp` 中生成的 JS 字符串，确认 `legendNames` 部分的语法。
```cpp
jscode += QString("['%1', '%2'],").arg(zName).arg(rName);
```
生成的：`['Z-Acc', 'R-Acc'],`
放在参数列表里：`onDataReceived(names, ['Z-Acc', 'R-Acc'], series)`
这完全正确。

**决定**：修改 `line-stack.html`，启用 `notMerge` 模式。这是解决此类状态不同步问题的最有效方法。