# EFEM调用流程图与迁移图分析

根据对代码的详细分析，我整理了EFEM系统中消息处理的完整调用流程结构。

## 1. EFEM消息处理总体调用流程

```
┌─────────────────┐     ┌────────────────────┐     ┌────────────────────────┐
│ 接收ASCII消息   │────>│ 数据解析与预处理   │────>│ 命令分发与处理         │
└─────────────────┘     └────────────────────┘     └────────────────────────┘
                                                          │
                                                          ▼
┌─────────────────┐     ┌────────────────────┐     ┌────────────────────────┐
│ 设置命令状态    │<────│ 子系统具体处理     │<────│ 查找对应子系统模块    │
└─────────────────┘     └────────────────────┘     └────────────────────────┘
```

## 2. 详细调用链分析

### 2.1 数据接收与解析阶段

1. **消息接收入口**：`EFEMAsciiApi::process()`
   - 从缓冲区读取数据，识别消息结束符`0x0D`
   - 调用`onDataRecv()`处理接收到的数据

2. **数据预处理**：`EFEMAsciiApi::onDataRecv(const char* data, unsigned int len)`
   - 将原始数据转换为字符串
   - 使用分号(`;`)和回车符(`\r`)分割多个消息
   - 对每个消息调用`processSingleMessage()`

3. **消息解析**：`EFEMAsciiApi::processSingleMessage(const std::string& message)`
   - 解析消息格式（如`INF:STATE/LP1`）
   - 提取命令类型（Type）和基本命令（Base）
   - 创建Command对象并调用`handle()`方法

### 2.2 命令分发与子系统调用阶段

4. **命令分发**：`FortrendAsciiEFEMApi::handle(const std::shared_ptr<Command>& command)`
   - 查找handleMap中对应的处理函数
   - handleMap是一个映射表，将Base枚举值映射到相应的处理函数

5. **命令处理**：各种handle_XXX方法（如`handle_STATE`, `handle_MAPDT`等）
   - 根据命令参数识别目标子系统（LP、WTR、ALIGNER等）
   - 通过kernel获取对应的子系统模块实例
   - 调用子系统的handle方法

6. **子系统处理**：各子系统的handle方法
   - 处理具体命令并设置命令状态
   - 例如`EFEMAlignerSubsystem::handle()`处理寻边器相关命令

### 2.3 命令状态管理

7. **状态设置**：`setCommandState()`方法
   - 在命令处理完成后设置状态（如TRANS_FINISHED）
   - 通知等待线程命令完成

## 3. 关键组件与数据流

### 3.1 核心API类

- **EFEMAsciiApi**：基类，定义消息处理框架
- **FortrendAsciiEFEMApi**：派生类，实现具体的命令分发逻辑
- **各子系统类**：EFEMLPSubsystem、EFEMWaferRobotSubsystem、EFEMAlignerSubsystem等

### 3.2 命令类型与消息格式

消息格式：`Type:Base/param1/param2;`

- **Type**：INF, ABS, EVT, ACK, NAK等
- **Base**：STATE, MAPDT, INIT, LOAD, UNLOAD, ALIGN等
- **参数**：根据命令类型不同而变化

### 3.3 命令状态迁移图

```
TRANS_IDEL ──> TRANS_WAIT_REPLY ──> TRANS_FINISHED
      │                │
      │                └─> TRANS_RESPONSE_TIMEOUT
      │
      └─> TRANS_REQUEST_FAILD
```

## 4. 典型命令处理流程示例（以ALIGN命令为例）

```
1. 接收消息: "INF:ALIGN/ALIGNER"
2. EFEMAsciiApi::onDataRecv 解析消息
3. EFEMAsciiApi::processSingleMessage 创建Command对象
4. FortrendAsciiEFEMApi::handle 查找并调用handle_ALIGN
5. handle_ALIGN 获取EFEMAlignerSubsystem实例
6. EFEMAlignerSubsystem::handle 处理具体命令
7. 创建并执行ALIGN命令
8. 设置命令状态为TRANS_FINISHED
9. 通知等待线程命令完成
```

## 5. 消息响应机制

- 处理完成后通过`sendACK`/`sendNAK`发送确认消息
- 事件通知通过`sendEVT`发送
- 状态更新通过`sendINF`发送

通过这种分层设计和查表分发机制，EFEM系统实现了灵活、可扩展的命令处理框架，能够有效地管理和协调不同子系统之间的通信和操作。