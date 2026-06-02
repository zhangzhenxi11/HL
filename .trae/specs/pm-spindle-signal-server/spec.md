# PM轴工艺位置信号通知服务 Spec

## Why
在spindle流程中，PM腔的轴（Z轴和R轴）在到达和离开工艺位置时，需要通知外部客户端。当前系统缺少一个对外的TCP服务器接口来发送这些信号。

## What Changes
- 新增 `PMSpindleSignalServerApi` 类，继承自 `TcpServerApi`
- 新增信号通知接口，支持在PM轴到达/离开工艺位置时发送通知
- 在 `pm_recipe_widget.cpp` 的 `startPmMotorRun` 函数中集成信号发送逻辑

## Impact
- 新增文件：
  - `device/include/pm_spindle_signal_server_api.h`
  - `device/src/pm_spindle_signal_server_api.cpp`
- 修改文件：
  - `device/src/pm_recipe_widget.cpp` (集成信号发送)

## ADDED Requirements

### Requirement: PM轴位置信号服务
系统 SHALL 提供一个TCP服务器接口 `PMSpindleSignalServerApi`，当PM轴到达或离开工艺位置时，向已连接的客户端发送位置信号通知。

#### Scenario: 轴到达工艺位置
- **WHEN** PM轴移动到 `Process` 位置完成时
- **THEN** 服务端向所有已连接的客户端发送信号 `PM_AXIS_ARRIVED_PROCESS`

#### Scenario: 轴离开工艺位置
- **WHEN** PM轴从 `Process` 位置移动到其他位置（如 `Rotate`）时
- **THEN** 服务端向所有已连接的客户端发送信号 `PM_AXIS_DEPARTED_PROCESS`

### Requirement: 信号数据格式
信号数据格式为JSON字符串，包含以下字段：
- `event`: 事件类型 (`PM_AXIS_ARRIVED_PROCESS` 或 `PM_AXIS_DEPARTED_PROCESS`)
- `pm_index`: PM腔索引 (0-3，对应PM1-PM4)
- `position`: 位置名称 (`Transfer`, `LiftPin`, `Rotate`, `Process`)
- `timestamp`: 时间戳 (ISO 8601格式)

## MODIFIED Requirements
无

## REMOVED Requirements
无

## 接口设计

```cpp
class PMSpindleSignalServerApi : public TcpServerApi
{
public:
    DECLARE_PTR(PMSpindleSignalServerApi)
    PMSpindleSignalServerApi(IKernel* kernel);

    // 发送轴到达工艺位置信号
    void notifyAxisArrivedProcess(int pmIndex, const std::string& position);

    // 发送轴离开工艺位置信号
    void notifyAxisDepartedProcess(int pmIndex, const std::string& position);
};
```

## 信号发送时机

在 `pm_recipe_widget.cpp` 的 `startPmMotorRun` 函数中：

1. **轴到达工艺位置** - 在 `createLiftingActionCommand(targetPos)` 执行完成且目标位置为 `Process` 时
2. **轴离开工艺位置** - 在从 `Process` 位置开始移动到其他位置之前