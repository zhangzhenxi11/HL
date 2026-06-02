# Tasks

## 1. 创建 PMSpindleSignalServerApi 头文件
- [x] 创建 `device/include/pm_spindle_signal_server_api.h`
- [x] 定义 `PMSpindleSignalServerApi` 类，继承自 `TcpServerApi`
- [x] 声明 `notifyAxisArrivedProcess` 和 `notifyAxisDepartedProcess` 接口
- [x] 声明 `sendSignalToClients` 私有方法

## 2. 创建 PMSpindleSignalServerApi 实现文件
- [x] 创建 `device/src/pm_spindle_signal_server_api.cpp`
- [x] 实现构造函数和基类初始化
- [x] 实现 `notifyAxisArrivedProcess` 方法：构建JSON信号并发送
- [x] 实现 `notifyAxisDepartedProcess` 方法：构建JSON信号并发送
- [x] 实现 `sendSignalToClients` 方法：遍历所有连接发送信号

## 3. 集成信号发送逻辑到 pm_recipe_widget.cpp
- [x] 在 `pm_recipe_widget.cpp` 中包含头文件
- [x] 在轴到达 `Process` 位置时调用 `notifyAxisArrivedProcess`
- [x] 在轴离开 `Process` 位置时调用 `notifyAxisDepartedProcess`

## Task Dependencies
- Task 2 依赖 Task 1（必须先有头文件）
- Task 3 依赖 Task 1 和 Task 2