# Checklist

## PMSpindleSignalServerApi 头文件创建
- [x] `pm_spindle_signal_server_api.h` 文件已创建在 `device/include/` 目录
- [x] 类继承自 `TcpServerApi`
- [x] `DECLARE_PTR` 宏已使用
- [x] `notifyAxisArrivedProcess(int pmIndex, const std::string& position)` 方法已声明
- [x] `notifyAxisDepartedProcess(int pmIndex, const std::string& position)` 方法已声明

## PMSpindleSignalServerApi 实现文件创建
- [x] `pm_spindle_signal_server_api.cpp` 文件已创建在 `device/src/` 目录
- [x] 构造函数正确初始化基类 `TcpServerApi`
- [x] `notifyAxisArrivedProcess` 构建正确的JSON格式信号并调用 `sendMessage`
- [x] `notifyAxisDepartedProcess` 构建正确的JSON格式信号并调用 `sendMessage`
- [x] JSON包含 `event`, `pm_index`, `position`, `timestamp` 字段

## pm_recipe_widget.cpp 集成
- [x] 已包含 `pm_spindle_signal_server_api.h` 头文件
- [x] 在轴到达 `Process` 位置后调用 `notifyAxisArrivedProcess`
- [x] 在轴离开 `Process` 位置前调用 `notifyAxisDepartedProcess`
- [x] 信号发送不影响原有流程逻辑

## 代码质量
- [x] 代码遵循项目现有的命名规范和风格
- [x] 无硬编码的敏感信息
- [x] 头文件有适当的 `#include` 保护