# Spindle模组运动流程完善与配方界面适配 Spec

## Why
当前PM配方界面在Process位置的工艺等待时间仅支持单一的总工艺时间/最后一次时间配置，无法满足spindle工艺流程中的多种时间需求。需要区分工艺时间和工艺位置时间，并增加额外的Wait时间配置。

## Clarification (用户纠正)
根据VTM-spindle模组运动流程表：
- **总工艺时间** = 每次在工艺位置的工艺时间(Dep)之和
- **工艺位置的时间** = 前处理时间 + 工艺时间(Dep) + 后处理时间
- **Wait时间是额外的，不在工艺时间内**
- **每次达到工艺位置时wait时间可能不同**，需要在UI中支持为每次工艺配置不同的wait时间

即：每次工艺位置总时间 = 前处理Wait(soak) + 工艺时间(Dep) + 后处理Wait(time7)

## What Changes
- 扩展 `PMParams` 结构，增加前处理/后处理Wait时间参数（Wait时间额外于工艺时间）
- 修改 `pm_recipe_widget.ui` 界面，增加前处理/后处理Wait时间编辑列
- 保留原有的"总工艺时间/s"作为工艺时间(Dep)总和
- 修改 `pm_recipe_widget.cpp` 运动流程，执行前处理->工艺->后处理->Wait时间

## Impact
- 修改文件：
  - `device/include/pm_recipe_widget.h` - 数据结构扩展
  - `device/src/pm_recipe_widget.cpp` - 运动流程完善和UI逻辑
  - `device/src/pm_recipe_widget.ui` - 界面增加wait时间编辑控件

## ADDED Requirements

### Requirement: Wait时间参数配置
系统 SHALL 提供在电机运动参数表格（innerTable）中为每次工艺配置不同Wait时间的功能，Wait时间额外于工艺时间。

#### Scenario: 配置每次工艺的Wait时间
- **WHEN** 用户在电机参数表格的每列（对应每次工艺）中设置前处理Wait和后处理Wait
- **THEN** 系统保存每个工艺的Wait时间参数，在执行对应工艺时应用

### Requirement: Wait时间类型定义
根据spindle工艺流程，每次工艺（每列）需要配置：

| 行号 | 参数名 | 说明 | 用途 |
|-----|--------|------|------|
| 8 | pre_process_wait | 前处理Wait时间 | soak浸润时间 |
| 9 | post_process_wait | 后处理Wait时间 | time7后处理时间 |

**注意**：工艺时间(Dep)由原有的"总工艺时间/s"参数决定，Wait时间是额外的。

#### Scenario: 运动流程中执行时间
- **WHEN** PM轴到达Process位置执行第i次工艺时
- **THEN** 系统执行：前处理Wait(第i次) -> 工艺时间(第i次Dep) -> 后处理Wait(第i次)

## MODIFIED Requirements

### Requirement: PM配方界面时间参数
原需求：支持"总工艺时间/s"和"最后一次时间/s"配置
新需求：在保留工艺时间配置基础上，增加前处理/后处理Wait时间配置

### Requirement: 时间计算逻辑
保持原有的"总工艺时间/工艺次数"逻辑计算每次工艺时间(Dep)，Wait时间作为额外部分叠加

## 界面设计

### 方案说明
由于**每次达到工艺位置时wait时间可能不同**，需要支持为每次工艺配置不同的wait时间。

采用在电机运动参数表格（innerTable）中增加两行的方式：
- 电机参数表格每列对应一次工艺（第1次、第2次...第N次）
- 每列可以设置不同的前处理Wait和后处理Wait

### 电机运动参数表格扩展
在 `pm_recipe_widget.cpp` 的 `initInnerTable` 函数中，为每个PM的innerTable增加两行：

| 行号 | 参数名称 | 说明 |
|-----|---------|------|
| 0 | lifting_acc | Z轴加速度 |
| 1 | lifting_dec | Z轴减速度 |
| 2 | lifting_jerk | Z轴加加速度 |
| 3 | lifting_vel | Z轴速度 |
| 4 | rotating_acc | R轴加速度 |
| 5 | rotating_dec | R轴减速度 |
| 6 | rotating_jerk | R轴加加速度 |
| 7 | rotating_vel | R轴速度 |
| **8** | **pre_process_wait** | **前处理Wait时间/s (soak)** |
| **9** | **post_process_wait** | **后处理Wait时间/s (time7)** |

用户可以在每列（每次工艺）中设置不同的Wait时间。

### PM腔参数表格保持不变
保留原有列：
- 总工艺时间/s (工艺时间Dep的总和)
- 最后一次时间/s (最后一次工艺时间)

## 实现方案

### 数据结构扩展 (pm_recipe_widget.h)

扩展 `PMMotorRow` 结构，增加Wait时间参数：

```cpp
struct PMMotorRow {
    double lifting_acc = 0.0;
    double lifting_dec = 0.0;
    double lifting_jerk = 0.0;
    double lifting_vel = 0.0;
    double rotating_acc = 0.0;
    double rotating_dec = 0.0;
    double rotating_jerk = 0.0;
    double rotating_vel = 0.0;
    
    // 新增Wait时间参数（每次工艺可能不同）
    double pre_process_wait_s = 0.0;   // 前处理Wait时间 (soak)
    double post_process_wait_s = 0.0;  // 后处理Wait时间 (time7)
};
```

### 界面修改 (pm_recipe_widget.cpp)

在 `initInnerTable` 函数中，将innerTable的行数从8行扩展到10行：
- 行0-7：保留原有电机参数
- 行8：新增前处理Wait时间
- 行9：新增后处理Wait时间

每列（每次工艺）可以设置不同的Wait时间。

### 运动流程修改 (pm_recipe_widget.cpp)

在 `startPmMotorRun` 函数中，当到达Process位置执行第i次工艺时：
1. 从第i列读取前处理Wait时间并执行 (pre_process_wait_s)
2. 执行工艺时间 (Dep) - 使用原有"总工艺时间/工艺次数"逻辑
3. 从第i列读取后处理Wait时间并执行 (post_process_wait_s)