# VTM机械手4个PM腔交换手逻辑优化方案

## 1. 硬件与现有实现分析

### 1.1 硬件特性
- 青蛙腿式机械手，前后各一个手指（A手和B手）
- 同一时刻只能一个手指动作
- 底座旋转180度由硬件底层实现，软件只需发送手臂参数
- 支持`createGetCommand`和`createPutCommand`命令，接受手臂参数（1或2）

### 1.2 现有实现
- 同事实现：单个PM腔，使用集中式`startRobotAction`函数
- 用户实现：4个PM腔，使用分散式`executePMxTransfer`函数
- 交换手逻辑已在同事代码中实现，用户代码中为占位符

## 2. 设计方案

### 2.1 交换手逻辑实现

**核心思路**：基于同事的实现，为每个PM腔完善交换手逻辑，处理各种状态组合

**状态机设计**：

| 状态组合 | 操作逻辑 |
|---------|---------|
| PM无片 + 手臂1有片 | 手臂1放料 |
| PM无片 + 手臂2有片 | 手臂2放料 |
| PM有片 + 手臂1无片 + 手臂2有片 | 手臂1先取，手臂2后放 |
| PM有片 + 手臂1有片 + 手臂2无片 | 手臂2先取，手臂1后放 |
| PM有片 + 手臂1无片 + 手臂2无片 | 手臂1取料 |

### 2.2 集中式vs分散式设计

**建议方案**：继续使用分散式设计，完善`executePMxTransfer`函数

**理由**：
- 现有代码结构已经采用分散式设计，修改成本低
- 每个PM腔的逻辑相对独立，分散式设计更直观
- 便于单独调试和维护每个PM腔的逻辑
- 避免引入复杂的任务队列和调度机制

**优化点**：
- 提取公共逻辑到辅助函数，提高代码复用性
- 统一状态机设计，确保各PM腔逻辑一致性
- 实现简单的协调机制，避免操作冲突

### 2.3 传输效率优化

**核心优化**：
1. 实现完整的交换手逻辑，减少不必要的手臂移动
2. 优化状态判断，快速响应PM腔状态变化
3. 实现智能的手臂选择策略，减少旋转次数
4. 优化命令执行顺序，提高并行度

## 3. 具体实现步骤

### 3.1 第一步：完善单个PM腔的交换手逻辑

1. 取消注释并完善`executePMxTransfer`函数中的交换手逻辑（slot_transfer_cycle_vtm_widget.cpp:4888-4918）
2. 实现完整的状态机，处理所有可能的状态组合
3. 测试单个PM腔的交换手功能

**实现代码示例**：

```cpp
bool haswaferpm = cassManager->getCassette(pm1.get())->getMapping(1) == Cassette::Present;   //pm中有片
bool haswaferarm1 = cassManager->getCassette(wtr.get())->getMapping(1) == Cassette::Present; //arm1有片
bool haswaferarm2 = cassManager->getCassette(wtr.get())->getMapping(2) == Cassette::Present; //arm2有片

if (!haswaferpm && haswaferarm1) { // 手臂1放料
    robot_selected_arm = 0;
    auto cmd = wtr->createPutCommand(pm1, robot_selected_arm, 1);
    wtr->startCommand(cmd);
    cmd->wait();
    // 处理命令结果
    pm1_auto_step.store(1010);
}
elif (!haswaferpm && haswaferarm2) { // 手臂2放料
    robot_selected_arm = 1;
    auto cmd = wtr->createPutCommand(pm1, robot_selected_arm, 1);
    wtr->startCommand(cmd);
    cmd->wait();
    // 处理命令结果
    pm1_auto_step.store(1030);
}
elif (haswaferpm && !haswaferarm1 && haswaferarm2) { // 手1先取，手2后放
    // 手臂1取料
    robot_selected_arm = 0;
    auto cmd_get = wtr->createGetCommand(pm1, robot_selected_arm, 1);
    wtr->startCommand(cmd_get);
    cmd_get->wait();
    // 处理命令结果
    if (cmd_get->hasError()) {
        // 错误处理
    } else {
        // 手臂2放料
        robot_selected_arm = 1;
        auto cmd_put = wtr->createPutCommand(pm1, robot_selected_arm, 1);
        wtr->startCommand(cmd_put);
        cmd_put->wait();
        // 处理命令结果
    }
    pm1_auto_step.store(1050);
}
// 其他状态组合类似处理
```

### 3.2 第二步：统一4个PM腔的逻辑

1. 为`executePM2Transfer`、`executePM3Transfer`和`executePM4Transfer`函数添加相同的交换手逻辑
2. 确保各PM腔的状态机设计一致
3. 实现简单的协调机制，避免操作冲突

### 3.3 第三步：优化传输效率

1. 优化状态判断逻辑，减少不必要的检查
2. 实现智能的手臂选择策略，优先使用最近使用的手臂
3. 优化命令执行顺序，提高并行度
4. 实现错误处理和重试机制，提高系统可靠性

### 3.4 第四步：测试和调试

1. 对每个PM腔进行单独测试
2. 进行4个PM腔的联合测试
3. 优化调度算法，提高整体效率
4. 确保系统的稳定性和可靠性

## 4. 预期效果

- 完善4个PM腔的交换手逻辑，提高传输效率
- 充分利用双手指的特性，实现高效的取放片操作
- 保持代码结构清晰，便于维护和扩展
- 提高系统的稳定性和可靠性

## 5. 关键代码位置

- **主要修改文件**：slot_transfer_cycle_vtm_widget.cpp
- **需要完善的函数**：executePM1Transfer、executePM2Transfer、executePM3Transfer、executePM4Transfer
- **参考代码**：004_slot_transfer_cycle_vtm_widget.cpp中的startRobotAction函数（特别是4000-4090行的交换手逻辑）

## 6. 注意事项

1. 确保同一时刻只有一个手臂动作
2. 实现精细的状态管理，避免操作冲突
3. 考虑各种异常情况的处理
4. 确保线程安全，避免竞态条件
5. 充分测试各种状态组合，确保逻辑的正确性
6. 优化调度算法，平衡各个PM腔的优先级

## 7. 结论

基于现有代码结构和硬件限制，建议继续使用分散式设计，完善每个PM腔的交换手逻辑。这种方案修改成本低，便于维护和调试，同时能够充分利用双手指的特性，提高传输效率。

通过实现完整的状态机和优化命令执行顺序，可以确保机械手高效、安全地完成4个PM腔的取放片操作。