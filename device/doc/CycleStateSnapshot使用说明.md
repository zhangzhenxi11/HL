# Cycle调度系统状态快照功能使用说明

## 📌 功能概述

在调试复杂的Cycle调度系统时，状态快照功能可以：
1. **自动保存**：当程序报错时，自动保存当前的完整运行状态到JSON文件
2. **快速恢复**：从JSON文件加载状态，跳过前面的步骤，直接从报错位置继续调试
3. **节省时间**：避免每次调试都从头开始跑流程，特别是当报错发生在后期步骤时

## 🎯 使用场景

### 场景1：调试executeEFEMTransfer()函数

**问题**：走到 `efem_auto_step = 1541` 时指令执行失败，EFEM机械手、LP都已经动作，TaskManager数据也更新了。

**传统做法**：
```cpp
if(CYCLE_SIM_MODE == 1) {
    // 手动捏造数据
    efem_auto_step = 1541;
    tool_allow_get_wafer_LLA = true;
    // ... 更多手动设置
}
```

**使用快照功能**：
1. ❌ 第一次运行到 `efem_auto_step = 1541` 时报错
   - 系统**自动保存**快照到 `./CycleSnapshots/snapshot_20251028_143025.json`
   - 日志输出：✅ 状态快照已保存，可用于调试恢复

2. ✅ 修复代码后，第二次调试
   - 在 `onStart()` 函数中加载快照
   - 直接从 `efem_auto_step = 1541` 继续执行
   - 所有状态完全恢复（TaskManager、标志位、step等）

##使用方法

### 1. 自动保存快照（已集成）

当程序调用 `logFailed()`, `logFailedNotNormal()`, `logFailedExcuteCommandHasError()` 时，**自动**保存快照。

无需任何额外代码，报错时自动保存！

**保存内容**：
- ✅ 所有线程的step状态（efem_auto_step, loadlock1_auto_step 等）
- ✅ 所有标志位（tool_allow_get_wafer_LLA, lp1_cycle_one_time_finished 等）
- ✅ TaskManager中的所有任务数据（UnifiedWaferTask列表）
- ✅ 任务统计信息（originTaskSize, lp1TaskSize, lp2TaskSize）
- ✅ 错误信息和错误位置
- ✅ 时间戳

**保存位置**：`./CycleSnapshots/snapshot_<时间戳>.json`

### 2. 手动加载快照

#### 方法A：在onStart()中加载最新快照

```cpp
void QSlotTransferCycleVTMWidget::onStart(){
    Q_D(QSlotTransferCycleVTMWidget);
    
    // 方法1：加载最新的快照
    #ifdef DEBUG_LOAD_SNAPSHOT
    auto snapshots = CycleStateSnapshot::listSnapshots();
    if (!snapshots.empty()) {
        // 取最新的快照文件
        d->loadStateSnapshot(snapshots.back());
        logWarn("Cycle", "🔄 已从快照恢复状态，跳过初始化");
    }
    #endif
    
    // ... 正常的启动逻辑
}
```

#### 方法B：加载指定快照文件

```cpp
void QSlotTransferCycleVTMWidget::onStart(){
    Q_D(QSlotTransferCycleVTMWidget);
    
    // 方法2：加载指定的快照
    #ifdef DEBUG_LOAD_SNAPSHOT
    std::string snapshotFile = "./CycleSnapshots/snapshot_20251028_143025.json";
    if (d->loadStateSnapshot(snapshotFile)) {
        logWarn("Cycle", "🔄 已从指定快照恢复: %s", snapshotFile.c_str());
        // 直接启动线程，跳过初始化
        d->startAllThreads();
        return;  // 跳过正常初始化流程
    }
    #endif
    
    // 正常的初始化和启动
    if (!d->reset_finish) {
        QMessageBox::warning(this, "警告", "未执行整机复位.");
        return;
    }
    // ... 
}
```

#### 方法C：添加UI按钮加载快照

```cpp
// 在UI中添加"加载快照"按钮
void QSlotTransferCycleVTMWidget::onLoadSnapshot(){
    Q_D(QSlotTransferCycleVTMWidget);
    
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "选择状态快照文件",
        "./CycleSnapshots/",
        "JSON Files (*.json)"
    );
    
    if (!fileName.isEmpty()) {
        if (d->loadStateSnapshot(fileName.toStdString())) {
            QMessageBox::information(this, "成功", "状态快照加载成功！");
        } else {
            QMessageBox::warning(this, "失败", "状态快照加载失败！");
        }
    }
}
```

### 3. 快照JSON文件示例

```json
{
  "timestamp": "2025-10-28T14:30:25Z",
  "errorMessage": "EALIGNER Timeout command: Update for module",
  "errorLocation": "EFEM - EFEM流程步骤:1541",
  
  "efem_auto_step": 1541,
  "loadlock1_auto_step": 1010,
  "loadlock2_auto_step": 2060,
  "robot_auto_step": 3000,
  "vacuum_auto_step": 100,
  "pm1_auto_step": 10,
  "pm2_auto_step": 10,
  "pm3_auto_step": 10,
  "pm4_auto_step": 10,
  "update_auto_step": 10,
  
  "tool_allow_get_wafer_LLA": true,
  "tool_allow_get_wafer_LLB": false,
  "tool_allow_put_wafer_LLA": false,
  "tool_allow_put_wafer_LLB": false,
  "lp1_cycle_one_time_finished": false,
  "lp2_cycle_one_time_finished": false,
  "cycleFinished_lla": false,
  "cycleFinished_llb": false,
  
  "originTaskSize": 4,
  "lp1TaskSize": 2,
  "lp2TaskSize": 2,
  
  "current_lp_cycle": false,
  "robot_selected_arm": 0,
  
  "allTasks": [
    {
      "taskId": 1,
      "taskType": "EFEM_TRANSFER",
      "status": "IN_PROGRESS",
      "source": "LP1",
      "target": "LLA",
      "target_pm": "PM1",
      "sourceSlot": 1,
      "targetSlot": 1,
      "targetFeedingSlot": 1,
      "targetBlankingSlot": 1,
      "arm": 0,
      "isLoadingInPlace": true,
      "pm1Enabled": true,
      "pm2Enabled": false,
      "pm3Enabled": false,
      "pm4Enabled": false,
      "selectPmEnableList": [1, 0, 0, 0]
    }
    // ... 更多任务
  ]
}
```

## 🚀 调试流程示例

### 完整调试流程

```plaintext
第1次运行（发现问题）:
├─ 启动 Cycle
├─ EFEM step: 10 -> 100 -> 150 -> ... -> 1541
├─ ❌ 报错: "EALIGNER Timeout command"
├─ 📸 自动保存快照: snapshot_20251028_143025.json
└─ 暂停

修复代码:
├─ 修改 executeEFEMTransfer() 中 step 1541 的超时处理
└─ 增加重试逻辑

第2次运行（验证修复）:
├─ #define DEBUG_LOAD_SNAPSHOT 启用快照加载
├─ 启动 Cycle
├─ 🔄 自动加载快照: snapshot_20251028_143025.json
├─ 跳过: step 10 ~ 1540
├─ 直接从 step 1541 开始执行
├─ ✅ 验证修复后的代码
└─ 继续运行到 step 1600

节省时间:
├─ 传统方式: 每次从头跑 ~5分钟到达 step 1541
├─ 快照方式: 直接加载 ~2秒到达 step 1541
└─ 💾 节省: 98% 调试时间
```

## ⚙️ 高级用法

### 1. 条件性加载快照

```cpp
// 只在特定条件下加载快照
if (ui->debug_mode_checkbox->isChecked() && 
    ui->load_snapshot_checkbox->isChecked()) {
    
    auto snapshots = CycleStateSnapshot::listSnapshots();
    if (!snapshots.empty()) {
        d->loadStateSnapshot(snapshots.back());
    }
}
```

### 2. 列出所有快照

```cpp
void QSlotTransferCycleVTMWidget::onListSnapshots(){
    auto snapshots = CycleStateSnapshot::listSnapshots();
    
    qDebug() << "可用的状态快照:";
    for (const auto& file : snapshots) {
        qDebug() << "  -" << QString::fromStdString(file);
    }
}
```

### 3. 手动保存快照（用于测试）

```cpp
// 在某个特定位置手动保存快照
if (efem_auto_step == 1500) {  // 关键节点
    d->saveCurrentStateSnapshot(
        "手动保存：EFEM到达关键步骤",
        "EFEM - step:1500"
    );
}
```

## 📋 注意事项

1. **快照文件管理**
   - 快照文件保存在 `./CycleSnapshots/` 目录
   - 文件名格式：`snapshot_YYYYMMDD_HHMMSS.json`
   - 建议定期清理旧快照文件

2. **调试宏控制**
   ```cpp
   // 在调试时启用
   #define DEBUG_LOAD_SNAPSHOT
   
   // 发布时注释掉
   // #define DEBUG_LOAD_SNAPSHOT
   ```

3. **快照恢复后的状态**
   - ✅ 所有step和标志位完全恢复
   - ✅ TaskManager任务数据完全恢复
   - ⚠️ 硬件设备状态**不会**恢复（需要手动处理或模拟）
   - ⚠️ 模块指针（wtr, lk1, lk2等）**不会**恢复（会在线程中重新获取）

4. **与CYCLE_SIM_MODE结合**
   ```cpp
   if(CYCLE_SIM_MODE == 1) {
       // 模拟模式：跳过硬件操作
       // 配合快照加载，可以纯软件调试状态机逻辑
   }
   ```

## 🎉 优势总结

| 对比项 | 传统方式 | 快照方式 |
|--------|---------|---------|
| 调试准备 | 手动编写捏造数据代码 | 自动保存，无需准备 |
| 状态恢复 | 部分恢复，易遗漏 | 完整恢复，零遗漏 |
| 调试效率 | 每次从头跑流程 | 直接跳到报错位置 |
| 代码侵入 | 需要大量 if 判断 | 仅需简单启用 |
| 可复现性 | 难以精确复现 | 完全精确复现 |
| 时间节省 | 0% | 80%~98% |

## 📞 常见问题

**Q: 快照文件太大怎么办？**
A: 可以修改 `CycleStateSnapshot::Snapshot` 结构，只保存必要的状态，不保存整个TaskManager。

**Q: 如何在生产环境禁用快照保存？**
A: 使用编译宏控制：
```cpp
#ifdef DEBUG_MODE
    saveCurrentStateSnapshot(errorMsg, errorLocation);
#endif
```

**Q: 快照加载后设备状态不一致怎么办？**
A: 结合 `CYCLE_SIM_MODE=1` 模拟模式使用，或在加载快照后手动初始化设备状态。

**Q: 可以保存多个快照吗？**
A: 可以！每次报错都会生成新的快照文件（带时间戳），可以保存多个不同状态的快照。

---

**作者**：xielonghua  
**日期**：2025-10-28  
**版本**：v1.0
