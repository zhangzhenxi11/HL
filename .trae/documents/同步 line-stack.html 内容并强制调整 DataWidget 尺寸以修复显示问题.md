后台打印的 JS 错误信息揭示了问题的根本原因：**图例 (Legend) 名称不匹配**。

### 错误分析
错误信息 `js: gate 01 series not exists. Legend data should be same with series name or data name.` 表明：
1.  **ECharts 配置** (`line-stack.html`)：在 `option.legend.data` 中硬编码了 `['gate 01', 'gate 02', ...]`。
2.  **C++ 发送的数据** (`httpUpdate`)：发送的 Series 名称是 `Z-Acc`, `R-Acc` 等。
3.  **冲突**：ECharts 试图显示 "gate 01" 的图例，但找不到对应的 Series 数据，因此报错。虽然数据可能已经设置进去了，但图例系统的报错可能会阻止图表的正常渲染或显示。

### 解决方案
我需要修改 `DataWidget::httpUpdate` 生成的 JS 代码，使其不仅更新 Series 数据，**同时更新 Legend 数据**，确保两者名称一致。

### 修改计划
1.  **修改 `DataWidget::httpUpdate`**：
    *   在构建 JS `option` 对象时，显式添加 `legend: { data: [...] }` 字段。
    *   将传入的 `zName` 和 `rName` 放入 Legend 数组中。
    *   更新后的 JS 代码逻辑应类似：
        ```javascript
        var option = {
            xAxis: { ... },
            legend: { data: ['Z-Acc', 'R-Acc'] }, // 新增这行
            series: [ ... ]
        };
        myChart.setOption(option);
        ```
    *   由于 `httpUpdate` 只是生成 JS 字符串，我将修改字符串拼接逻辑来实现这一点。

这样可以消除 JS 报错，并确保图例能正确显示当前的曲线名称。同时，我也将按之前的计划添加 `resizeEvent` 的尺寸强制调整，以防万一。