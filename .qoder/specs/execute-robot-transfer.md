# executeRobotTransfer 集中机械手调度重构方案

## Context

当前 `slot_transfer_cycle_vtm_widget.cpp` 中，真空机械手(wtr)的 get/put 指令分散在 6 个线程中（LLA、LLB、PM1-PM4），线程间通过布尔标志 + 互斥锁协调，导致：
- 时序耦合严重，调试困难
- 机械手指令在 LL 和 PM 线程中都存在，职责不清晰
- `wtr_robot_mutex` 需要跨多个线程获取释放，容易死锁

**目标**：将所有 wtr 机械手操作集中到单一 `executeRobotTransfer()` 线程中，LL/PM 线程只负责设置请求标志，机械手线程轮询标志并执行动作，完成后设置完成标志通知请求方。

## 架构概览

```
┌─────────────────────────────────────────────────────────────────┐
│                      重构后架构                                    │
├─────────────────────────────────────────────────────────────────┤
│                                                                  │
│  executeLLATransfer ──┐                                         │
│  executeLLBTransfer ──┤── 设置请求标志 ──→  executeRobotTransfer │
│  executePM1Transfer ──┤                     (唯一操作wtr的线程)  │
│  executePM2Transfer ──┤                           │              │
│  executePM3Transfer ──┤                           ▼              │
│  executePM4Transfer ──┘  ←── 读取完成标志 ←── 执行get/put        │
│                                                                  │
└─────────────────────────────────────────────────────────────────┘
```

**职责划分**：
- **LL线程**：真空检查 → 开TM门阀 → 设置请求标志 → 等完成标志 → 关TM门阀
- **PM线程**：开PM门 → 设置请求标志 → 等完成标志 → 关PM门 → 跑工艺
- **Robot线程**：轮询请求标志 → 执行wtr指令 → 设置完成标志

## 与 UnifiedWaferTask 的关系

**决策**：原子标志做线程间同步，`RobotStatus` 做状态追踪记录。

- `RobotStatus`（PICK_QUEUED/PICK_COMPLETED/PLACE_QUEUED/PLACE_COMPLETED/ACTION_ERROR）用于**记录**机械手当前动作状态，供 UI 展示和日志追踪
- **不作为**线程调度的驱动信号（避免 TaskManager 锁竞争）
- 机械手线程在执行动作前后，调用 `taskManager.updateTaskStatus()` 同步 RobotStatus 状态

示例集成点：
```cpp
// Robot 线程执行取片前
// （由请求方在设标志前更新）
taskManager.updateTaskStatus(taskId, ROBOT_PROCESS, IN_PROGRESS);
// task.robotStatus = PICK_QUEUED  （需要在 task 上增加 robotStatus 字段的写入接口）

// Robot 线程取片完成后
// task.robotStatus = PICK_COMPLETED
```

## 关键文件

| 文件 | 改动说明 |
|------|---------|
| `device/src/slot_transfer_cycle_vtm_widget.cpp` | 主要改动文件：实现 `executeRobotTransfer()`，修改 LL/PM 线程 |
| `device/include/slot_transfer_cycle_vtm_widget.h` | 复用 `executeTMTransfer()` 声明（已存在，line 104，实现为空）|
| `device/include/UnifiedWaferTask.h` | 不修改枚举结构，仅在任务流转时使用现有 `RobotStatus` 标记 |

## 实现细节

### 1. 新增标志变量（在 Private 类成员中）

```cpp
// ===== 机械手请求标志（由 LL/PM 线程设置，Robot线程读取并清除）=====
// 请求类型
struct RobotTransferRequest {
    std::atomic<bool> requested{false};  // 请求标志
    std::atomic<bool> done{false};       // 完成标志
    std::atomic<bool> success{false};    // 执行结果
    std::atomic<int> arm{0};             // 使用哪个手臂 (0=A, 1=B)
    std::atomic<int> slot{1};            // LL slot编号
    std::string targetPm;                // 目标PM（仅exchange用）
};

// LL取片请求：从LL取到机械手
RobotTransferRequest robot_get_from_lla;
RobotTransferRequest robot_get_from_llb;

// PM放片请求：从机械手放到PM
RobotTransferRequest robot_put_to_pm1;
RobotTransferRequest robot_put_to_pm2;
RobotTransferRequest robot_put_to_pm3;
RobotTransferRequest robot_put_to_pm4;

// PM取片请求：从PM取到机械手
RobotTransferRequest robot_get_from_pm1;
RobotTransferRequest robot_get_from_pm2;
RobotTransferRequest robot_get_from_pm3;
RobotTransferRequest robot_get_from_pm4;

// LL放片请求：从机械手放回LL
RobotTransferRequest robot_put_to_lla;
RobotTransferRequest robot_put_to_llb;

// PM交换请求（取出已加工片+放入新片）
RobotTransferRequest robot_exchange_pm1;
RobotTransferRequest robot_exchange_pm2;
RobotTransferRequest robot_exchange_pm3;
RobotTransferRequest robot_exchange_pm4;

// 交换操作需要两个arm信息
struct ExchangeInfo {
    std::atomic<int> getArm{0};   // 取片用的arm
    std::atomic<int> putArm{1};   // 放片用的arm
};
ExchangeInfo exchange_info_pm1, exchange_info_pm2, exchange_info_pm3, exchange_info_pm4;
```

### 2. executeRobotTransfer() 状态机

```cpp
void QSlotTransferCycleVTMWidgetPrivate::executeRobotTransfer()
{
    // 获取子系统引用（同现有线程模式）
    // wtr, lk1, lk2, pm1-pm4, cassManager 等
    
    int robot_step = 10;
    
    while (!stopRequested)
    {
        Sleep(50);  // 轮询间隔，比现有线程的 500ms 更快
        
        // 暂停等待（与其他线程一致的模式）
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this] { return running.load() || stopRequested.load(); });
        }
        if (stopRequested) break;
        
        switch (robot_step)
        {
        case 10:  // IDLE：按优先级轮询请求标志
        {
            // 优先级1: PUT_TO_LL（释放手臂）
            if (robot_put_to_lla.requested.load()) {
                robot_step = 4000; break;
            }
            if (robot_put_to_llb.requested.load()) {
                robot_step = 4100; break;
            }
            // 优先级2: GET_FROM_PM（取回已完成工艺的片）
            if (robot_get_from_pm1.requested.load()) {
                robot_step = 3000; break;
            }
            if (robot_get_from_pm2.requested.load()) {
                robot_step = 3100; break;
            }
            if (robot_get_from_pm3.requested.load()) {
                robot_step = 3200; break;
            }
            if (robot_get_from_pm4.requested.load()) {
                robot_step = 3300; break;
            }
            // 优先级3: EXCHANGE_AT_PM
            if (robot_exchange_pm1.requested.load()) {
                robot_step = 5000; break;
            }
            if (robot_exchange_pm2.requested.load()) {
                robot_step = 5100; break;
            }
            if (robot_exchange_pm3.requested.load()) {
                robot_step = 5200; break;
            }
            if (robot_exchange_pm4.requested.load()) {
                robot_step = 5300; break;
            }
            // 优先级4: PUT_TO_PM
            if (robot_put_to_pm1.requested.load()) {
                robot_step = 2000; break;
            }
            if (robot_put_to_pm2.requested.load()) {
                robot_step = 2100; break;
            }
            if (robot_put_to_pm3.requested.load()) {
                robot_step = 2200; break;
            }
            if (robot_put_to_pm4.requested.load()) {
                robot_step = 2300; break;
            }
            // 优先级5: GET_FROM_LL
            if (robot_get_from_lla.requested.load()) {
                robot_step = 1000; break;
            }
            if (robot_get_from_llb.requested.load()) {
                robot_step = 1100; break;
            }
        }
        break;
        
        // === 1000-1100: GET_FROM_LL ===
        case 1000:  // 从LLA取片
        {
            int arm = robot_get_from_lla.arm.load();
            int slot = robot_get_from_lla.slot.load();
            auto cmd = wtr->createGetCommand(lk1, arm, slot);
            wtr->startCommand(cmd);
            cmd->wait();
            if (cmd->hasError()) {
                // 错误处理：745错误码清除后重试
                robot_get_from_lla.success.store(false);
            } else {
                robot_get_from_lla.success.store(true);
            }
            robot_get_from_lla.requested.store(false);
            robot_get_from_lla.done.store(true);
            robot_step = 10;
        }
        break;
        
        case 1100:  // 从LLB取片（同上结构，操作lk2）
        { /* 类似1000 */ }
        break;
        
        // === 2000-2300: PUT_TO_PM ===
        case 2000:  // 放片到PM1
        {
            int arm = robot_put_to_pm1.arm.load();
            auto cmd = wtr->createPutCommand(pm1, arm, 1);
            wtr->startCommand(cmd);
            cmd->wait();
            if (cmd->hasError()) {
                // 734/736错误处理
                robot_put_to_pm1.success.store(false);
            } else {
                robot_put_to_pm1.success.store(true);
            }
            robot_put_to_pm1.requested.store(false);
            robot_put_to_pm1.done.store(true);
            robot_step = 10;
        }
        break;
        // case 2100/2200/2300 同上结构，操作pm2/pm3/pm4
        
        // === 3000-3300: GET_FROM_PM ===
        case 3000:  // 从PM1取片
        {
            int arm = robot_get_from_pm1.arm.load();
            auto cmd = wtr->createGetCommand(pm1, arm, 1);
            wtr->startCommand(cmd);
            cmd->wait();
            if (cmd->hasError()) {
                robot_get_from_pm1.success.store(false);
            } else {
                robot_get_from_pm1.success.store(true);
            }
            robot_get_from_pm1.requested.store(false);
            robot_get_from_pm1.done.store(true);
            robot_step = 10;
        }
        break;
        // case 3100/3200/3300 同上
        
        // === 4000-4100: PUT_TO_LL ===
        case 4000:  // 放回LLA
        {
            int arm = robot_put_to_lla.arm.load();
            int slot = robot_put_to_lla.slot.load();
            auto cmd = wtr->createPutCommand(lk1, arm, slot);
            wtr->startCommand(cmd);
            cmd->wait();
            if (cmd->hasError()) {
                robot_put_to_lla.success.store(false);
            } else {
                robot_put_to_lla.success.store(true);
            }
            robot_put_to_lla.requested.store(false);
            robot_put_to_lla.done.store(true);
            robot_step = 10;
        }
        break;
        
        // === 5000-5300: EXCHANGE_AT_PM ===
        case 5000:  // PM1交换（先取后放）
        {
            int getArm = exchange_info_pm1.getArm.load();
            int putArm = exchange_info_pm1.putArm.load();
            
            // 第一步：取出已加工片
            auto cmd_get = wtr->createGetCommand(pm1, getArm, 1);
            wtr->startCommand(cmd_get);
            cmd_get->wait();
            if (cmd_get->hasError()) {
                robot_exchange_pm1.success.store(false);
                robot_exchange_pm1.requested.store(false);
                robot_exchange_pm1.done.store(true);
                robot_step = 10;
                break;
            }
            // 第二步：放入新片
            auto cmd_put = wtr->createPutCommand(pm1, putArm, 1);
            wtr->startCommand(cmd_put);
            cmd_put->wait();
            if (cmd_put->hasError()) {
                robot_exchange_pm1.success.store(false);
            } else {
                robot_exchange_pm1.success.store(true);
            }
            robot_exchange_pm1.requested.store(false);
            robot_exchange_pm1.done.store(true);
            robot_step = 10;
        }
        break;
        // case 5100/5200/5300 同上
        }
    }
}
```

### 3. 修改 executeLLATransfer（示例）

**原来的 case 1051**（直接操作机械手取片）：
```cpp
// 删除以下代码：
wtr_robot_mutex.lock();
auto cmd = wtr->createGetCommand(lk1, robot_selected_arm, loadlock1_move_slot_index);
wtr->startCommand(cmd);
cmd->wait();
wtr_robot_mutex.unlock();
```

**替换为**：
```cpp
case 1051:
{
    // 设置请求参数
    robot_get_from_lla.arm.store(robot_selected_arm);
    robot_get_from_lla.slot.store(loadlock1_move_slot_index);
    robot_get_from_lla.done.store(false);
    robot_get_from_lla.success.store(false);
    // 发起请求
    robot_get_from_lla.requested.store(true);
    loadlock1_auto_step = 1051_wait;  // 新增等待步骤
}
break;

case 1051_wait:  // 用实际数字如 1055
{
    if (robot_get_from_lla.done.load()) {
        if (robot_get_from_lla.success.load()) {
            loadlock1_auto_step = 1052;  // 成功，继续
        } else {
            // 错误处理
            logFailed(wtr->getName(), "机械手从LLA取片失败");
            loadlock1_auto_step = -100;
        }
    }
    // else: 继续等待（下一轮循环）
}
break;
```

**同样模式用于**：
- LLA 的 case 2055（从PM取片）→ `robot_get_from_pmX.requested = true`
- LLA 的 case 2060（放回LL）→ `robot_put_to_lla.requested = true`
- LLB 的对应 case

### 4. 修改 executePM1Transfer（示例）

**原来的 case 1010**（直接操作机械手放片到PM）：
```cpp
// 删除以下代码：
wtr_robot_mutex.lock();
auto cmd = wtr->createPutCommand(pm1, robot_selected_arm, 1);
wtr->startCommand(cmd);
cmd->wait();
wtr_robot_mutex.unlock();
```

**替换为**：
```cpp
case 1010:
{
    robot_put_to_pm1.arm.store(robot_selected_arm);
    robot_put_to_pm1.done.store(false);
    robot_put_to_pm1.success.store(false);
    robot_put_to_pm1.requested.store(true);
    pm1_auto_step.store(1015);  // 等待步骤
}
break;

case 1015:
{
    if (robot_put_to_pm1.done.load()) {
        if (robot_put_to_pm1.success.load()) {
            pm1_allow_get_put_wafer = false;
            pm1_auto_step.store(2000);  // 进入工艺
        } else {
            logFailed(wtr->getName(), "放片到PM1失败");
            pm1_auto_step.store(-100);
        }
    }
}
break;
```

**交换场景（case 1060/1070）**：
```cpp
case 1060:
{
    exchange_info_pm1.getArm.store(0);  // A臂取
    exchange_info_pm1.putArm.store(1);  // B臂放
    robot_exchange_pm1.done.store(false);
    robot_exchange_pm1.success.store(false);
    robot_exchange_pm1.requested.store(true);
    pm1_auto_step.store(1065);  // 等待
}
break;

case 1065:
{
    if (robot_exchange_pm1.done.load()) {
        if (robot_exchange_pm1.success.load()) {
            pm1_auto_step.store(2000);
        } else {
            pm1_auto_step.store(-100);
        }
    }
}
break;
```

### 5. 线程生命周期管理

**创建线程**（在 `startProcessingThreads()` 中添加）：
```cpp
std::thread thd_robot(&QSlotTransferCycleVTMWidget::executeRobotTransfer, this);
thd_robot.detach();
```

**暂停/停止**：无需额外处理，`executeRobotTransfer` 内部已使用同样的 `cv.wait()` + `stopRequested` 模式。

**重置**：在重置逻辑中清除所有请求标志：
```cpp
// 在 resetAction 或相关重置逻辑中
robot_get_from_lla.requested.store(false);
robot_get_from_lla.done.store(false);
// ... 清除所有标志
```

### 6. 移除 wtr_robot_mutex

由于只有 `executeRobotTransfer` 线程操作 `wtr`，可以**安全移除** `wtr_robot_mutex` 的所有 lock/unlock 调用。但建议分两步：
1. 先实现 `executeRobotTransfer` 并迁移所有 wtr 调用
2. 确认工作正常后再删除 `wtr_robot_mutex` 声明

### 7. 复用 executeTMTransfer

头文件中已有 `void executeTMTransfer();`（line 104），源文件中实现为空（line 7126-7129）。直接将其实现替换为机械手集中调度逻辑，无需改头文件声明。

### 8. RobotStatus 追踪集成

在 LL/PM 线程设置请求标志时，同步更新 TaskManager 中对应任务的状态：

```cpp
// executeLLATransfer case 1051 中（设置 GET_FROM_LL 请求前）：
taskManager.updateTaskStatus(taskId, UnifiedWaferTask::ROBOT_PROCESS, UnifiedWaferTask::IN_PROGRESS);
// 可选：通过新增接口设置 task.robotStatus = PICK_QUEUED

// executeRobotTransfer 执行完成后，由请求方在检测到 done 后更新：
// task.robotStatus = PICK_COMPLETED （记录用）
```

这样 `RobotStatus` 枚举不需要修改，作为任务内部的附加状态记录使用。

## 实施步骤

1. **在 Private 类中添加 `RobotTransferRequest` 结构体和所有请求标志变量**
2. **实现 `executeRobotTransfer()`** 状态机（可复用现有空的 `executeTMTransfer`）
3. **修改 `executeLLATransfer`**：将 wtr 操作替换为设置标志 + 等待完成
4. **修改 `executeLLBTransfer`**：同上
5. **修改 `executePM1Transfer`**：将 wtr 操作替换为设置标志 + 等待完成
6. **修改 `executePM2-4Transfer`**：同上
7. **在 `startProcessingThreads()` 中启动 Robot 线程**
8. **在重置逻辑中清除所有标志**
9. **移除所有 `wtr_robot_mutex.lock()/unlock()` 调用**
10. **编译测试**

## 验证方法

1. **编译验证**：确保无编译错误
2. **单步调试**：
   - 设置断点在 `executeRobotTransfer` 的 case 10 中，确认能正确检测到请求标志
   - 设置断点在 LL/PM 的等待步骤中，确认 done 标志能正确收到
3. **功能测试**：
   - 单片流程：LP → EFEM → LL → Robot取 → Robot放PM → 工艺 → Robot取PM → Robot放LL → EFEM → LP
   - 多片并行：多个PM同时工艺完成，验证优先级轮询是否正确
   - 交换场景：PM有片且arm有新片，验证 exchange 原子操作
   - 暂停/恢复：在机械手执行中暂停，恢复后继续
   - 停止：验证所有标志正确清除，线程退出
4. **异常测试**：模拟机械手指令错误（错误码745/734），验证错误标志传回
