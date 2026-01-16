收到，既然第二个问题已修复，我将专注于更新实时数据图表的 X 轴时间显示格式。

### 1. 更新实时数据图表（X轴时间格式）
**文件**: `device/src/Data/fortrend_data_subsystem_widget.cpp`
- **操作**: 修改 `DataWidget::onclick` 和 `DataWidget::onSimulateTest` 函数。
- **变更**: 
    - 获取当前时间戳 `now`。
    - 使用 `QDateTime::fromMSecsSinceEpoch(now).toString("HH:mm:ss:zzz")` 生成时间字符串。
    - 将此格式化后的字符串（如 "14:30:05:123"）添加到 `d->timeList`，替代原有的相对毫秒数。
- **结果**: 曲线图 X 轴将按要求显示“小时:分钟:秒:毫秒”。