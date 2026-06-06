# WTR收发匹配与Pause触发分析计划

## Summary
- 目标：定位真空流程中 WTR 多指令并发时的收发错配根因，解释 `2026-06-05_any_cleaned.txt:L11366-L11367` 的触发来源，并给出覆盖 `RQLoad`、`GetWafer`、`PutWafer` 及其他 WTR 指令的统一消息匹配改造方案。
- 结论基础：
  - `20:18:51.832 / 20:18:51.845` 对应 `QSlotTransferCycleVTMWidgetPrivate::pauseAllThreads()` 的两条日志，定义在 `device/src/slot_transfer_cycle_vtm_widget.cpp:9569-9587`。
  - 按用户确认，本次 `pauseAllThreads()` 视为人为操作点，不纳入“程序自动触发根因链”。
  - 同时间窗内日志还出现了 `LLB 第2槽不是正常片`、`WTR error:[T=5,C=0x1]RPS:GETOBJECT;`、`LLA 查询手指有无晶圆命令执行失败，LLA流程步骤：1999`，这些用于分析串包后果，但不再把 `pauseAllThreads()` 当作自动故障触发点。
  - `sunway_subsystem_helper.cpp` 当前把所有机械手回包都放进同一个 `messageQueue`，`recvResponseRobotMessage()` 只按 FIFO 弹出，不校验“该响应是否属于当前命令”，因此并发命令场景下会串包。

## Current State Analysis

### 1. `pauseAllThreads` 的定位
- `pauseAllThreads()` 位于 `device/src/slot_transfer_cycle_vtm_widget.cpp:9569-9587`，输出：
  - `Cyclelog:start pause All Threads .....`
  - `Cyclelog:pause All Threads  end!`
- `saveCurrentStateSnapshot()` 位于 `device/src/slot_transfer_cycle_vtm_widget.cpp:10743-10758`，输出：
  - `Cycle:✅ 状态快照已保存，可用于调试恢复.`
- `onPause()` 位于 `device/src/slot_transfer_cycle_vtm_widget.cpp:11332-11360`，其内部直接调用 `d->pauseAllThreads()`。
- 用户已明确本次 `L11366-L11367` 属于人为暂停动作，因此本计划把它作为“日志来源解释”处理，不把它继续追为程序自动调用。
- 代码层面仍存在线程异常捕获后自动 `pauseAllThreads()` 的路径，如：
  - `device/src/slot_transfer_cycle_vtm_widget.cpp:4995-5007`
  - `device/src/slot_transfer_cycle_vtm_widget.cpp:6644-6651`
  - `device/src/slot_transfer_cycle_vtm_widget.cpp:7856-7866`
  - `device/src/slot_transfer_cycle_vtm_widget.cpp:9505-9516`
- 但这些路径本次不作为主要修复对象。

### 2. 本次日志时序事实
- `2026-06-05_wtr.txt` 显示：
  - `20:18:21.320` 开始 `GetWafer[]`
  - `20:18:21.357` 收到 `ACK:GETOBJECT/3/1/A/0/0;`
  - `20:18:21.651` 紧接着又开始 `RQLoad[]`
  - `20:18:51.943` `RQLoad(B)` 等待期间先收到 `RPS:GETOBJECT;`
  - `20:18:51.994` 之后才收到 `ACK:LOAD/B;`
  - `20:18:52.102` 再收到 `RPS:LOAD/ON;`
  - `20:18:52.221` 该 `RPS:LOAD/ON;` 又被后续 `GetWafer` 消费，最终报“取晶圆命令执行失败，机械手返回的指令未定义：RPS:LOAD/ON;”
- `2026-06-05_llb.txt` 显示：
  - `20:18:51.924` 先报 `LLB 第2槽不是正常片`
- `2026-06-05_lla.txt` 显示：
  - `20:18:52.188` 报 `LLA 查询手指有无晶圆命令执行失败，LLA流程步骤：1999`
- 两份快照 `snapshot_20260605_121851.json` / `snapshot_20260605_121852.json` 说明：
  - `loadlock1_auto_step = 1999`
  - `loadlock2_auto_step = 1010`
  - `pm2_auto_step = 1065`
  - `robot_auto_step = 2100`
  - `errorLocation` 先后为 `LLB 第2槽不是正常片` 与 `LLA 查询手指有无晶圆命令执行失败`
- 判断：
  - 用户标注的 `L11366-L11367` 对应的是“人为点击/调用暂停后，`pauseAllThreads()` 输出的日志”。
  - 这两行只回答“暂停日志来自哪里”，不再参与程序根因归属。
  - 本次真正要修的是同一时间窗内 WTR 多命令共享队列导致的消息错配。

### 3. WTR 当前通信缺陷
- `device/src/SunwayRobot/sunway_subsystem_helper.cpp`
  - `recvResponse2()` 在后台线程里持续收包，并把按 `\r` 切开的所有消息统一压入 `messageQueue`。
  - `recvResponseRobotMessage()` 只做 `front()/pop()`，没有按命令类型、会话、期望前缀进行过滤。
  - `clearRobotMessage()` 直接清空全局队列，会把别的并发命令尚未消费的合法回包一起删掉。
- `device/src/SunwayRobot/fortrend_sunwayrobot_get_wafer_command.cpp`
  - 先 `clearRobotMessage()`，再 `sendRequest("MOV:GETOBJECT/...")`，之后假定队列里先来 ACK、再来 `RPS:GETOBJECT;`。
- `device/src/SunwayRobot/fortrend_sunwayrobot_rq_load_command.cpp`
  - 先 `clearRobotMessage()`，再 `sendRequest("QRY:LOAD/A|B;")`，之后假定队列里先来 ACK、再来 `RPS:LOAD/ON|OFF;`。
- `device/src/SunwayRobot/fortrend_sunwayrobot_update_command.cpp`
  - 多次使用 `clearRobotMessage() + sendRequest() + recvResponseRobotMessage()` 查询 A/B 手指状态。
- `device/src/SunwayRobot/fortrend_sunwayrobot_put_wafer_command.cpp`
  - 放片/AWC 查询同样依赖“先清队列，再盲取 ACK/RPS”。
- `device/src/SunwayRobot/fortrend_sunwayrobot_home_command.cpp`
  - HOME 流程存在 `clearRobotMessage()` 和“任何 ACK / 任何非 ACK” 的消费方式。
- 其他依赖同一套 `recvResponseRobotMessage()` 的 WTR 命令还包括：
  - `fortrend_sunwayrobot_check_load_command.cpp`
  - `fortrend_sunwayrobot_reset_command.cpp`
  - `fortrend_sunwayrobot_set_load_command.cpp`
  - `fortrend_sunwayrobot_set_speed_command.cpp`
- 风险本质：
  - 多个命令共享一个无类型 FIFO；
  - “谁先调用 recv 就先吃掉哪条消息”；
  - `clearRobotMessage()` 还是全局破坏动作；
  - 代码只判断“是不是 ACK / RPS”，没判断“是不是当前命令对应的 ACK / RPS”。

## Proposed Changes

### 1. 改造 `sunway_subsystem_helper.cpp`：从“全局 FIFO”改为“可按规则匹配的接收缓存”
- 文件：`device/src/SunwayRobot/sunway_subsystem_helper.cpp`
- 修改内容：
  - 保留后台接收线程 `recvResponse2()` 按 `\r` 拆包的方式，但将存储结构从单纯 `queue<string>` 扩展为“受互斥保护的消息缓存容器”（可继续用 `deque/list/vector`）。
  - 新增一个“按匹配规则取消息”的 helper，例如：
    - 输入：超时、匹配谓词/期望前缀集合、可选日志标签
    - 行为：在缓存中查找第一条匹配消息；若没有，则等待条件变量并继续查找；只移除被当前调用成功匹配到的那条消息。
  - 新增更窄粒度的接口，而不是继续暴露“盲取一条”：
    - `waitMessageMatching(...)`
    - `waitAckForCommand(...)`
    - `waitRpsMatching(...)`
  - 弱化/淘汰 `clearRobotMessage()` 的全清策略，改为仅在必要时清理“明显过期且无人可能消费”的残留消息；默认不再在命令发送前粗暴清队列。
- 目的：
  - 让 `RQLoad` 只取 `ACK:LOAD/*` 和 `RPS:LOAD/*`
  - 让 `GetWafer` 只取 `ACK:GETOBJECT/...` 和 `RPS:GETOBJECT;`
  - 即使两条命令并发，其消息也不会互相误吃。

### 2. 改造 `sunway_command_executer.cpp`：补一层匹配式包装接口
- 文件：`device/src/SunwayRobot/sunway_command_executer.cpp`
- 修改内容：
  - 在执行器层增加对应包装函数，把 helper 的“匹配式取消息”接口暴露给各命令实现。
  - 保持旧接口可编译，但新改命令优先走新接口，避免大面积一次性改动。
- 目的：
  - 让命令层改动最小，且收发语义集中在执行器/助手层。

### 3. 统一改造所有 WTR 命令：按期望消息匹配消费
- 文件：
  - `device/src/SunwayRobot/fortrend_sunwayrobot_rq_load_command.cpp`
  - `device/src/SunwayRobot/fortrend_sunwayrobot_get_wafer_command.cpp`
  - `device/src/SunwayRobot/fortrend_sunwayrobot_put_wafer_command.cpp`
  - `device/src/SunwayRobot/fortrend_sunwayrobot_update_command.cpp`
  - `device/src/SunwayRobot/fortrend_sunwayrobot_check_load_command.cpp`
  - `device/src/SunwayRobot/fortrend_sunwayrobot_home_command.cpp`
  - `device/src/SunwayRobot/fortrend_sunwayrobot_reset_command.cpp`
  - `device/src/SunwayRobot/fortrend_sunwayrobot_set_load_command.cpp`
  - `device/src/SunwayRobot/fortrend_sunwayrobot_set_speed_command.cpp`
- 修改内容：
  - 去掉发送前的 `clearRobotMessage()` 依赖。
  - 发送后等待“当前命令专属 ACK”，而不是任何 ACK。
  - 继续等待“当前命令专属 RPS”，而不是任何非 ACK 消息。
  - 对于 `RQLoad`：
    - ACK 只接受 `ACK:LOAD/A;` / `ACK:LOAD/B;`
    - RPS 只接受 `RPS:LOAD/ON;` / `RPS:LOAD/OFF;`
  - 对于 `GetWafer`：
    - ACK 只接受 `ACK:GETOBJECT/...`
    - RPS 只接受 `RPS:GETOBJECT;`
  - 对于 `PutWafer`：
    - ACK 只接受对应动作的 ACK
    - RPS 只接受 `RPS:PUTOBJECT;` 或对应查询类 RPS（如 AWC）
  - 对于 `Update`：
    - A/B 手指状态查询分别只消费 `QRY:LOAD/A`、`QRY:LOAD/B` 对应回包
  - 对于 `Home / Reset / SetLoad / SetSpeed / CheckLoad`：
    - 分别建立各自命令到 ACK/RPS 的期望映射，统一走匹配式消费接口
  - 若等待期间收到别的合法消息：
    - 不报错；
    - 不丢弃；
    - 留在缓存中供对应命令后续消费。
- 目的：
  - 直接修掉本次日志中
    - `RQLoad(B)` 误吃 `RPS:GETOBJECT;`
    - `GetWafer` 误吃 `RPS:LOAD/ON;`
    这两处错配。
  - 同时把其他 WTR 指令的同类隐患一并收口，避免只修单点后又在 `PutWafer`、`Home`、`Reset` 等场景复发。

### 4. 加固日志，便于后续复盘
- 文件：
  - `device/src/SunwayRobot/sunway_subsystem_helper.cpp`
  - 上述三个命令文件
- 修改内容：
  - 增加“命令名/期望消息/实际匹配结果/缓存命中情况”的调试日志。
  - 对“消息不匹配但被保留”的场景打 debug/info 日志，而不是直接报错。
  - 在最终报错时输出：
    - 当前命令
    - 期望前缀
    - 实际首条不匹配消息
    - 缓存中尚存消息摘要
- 目的：
  - 后续能快速证明是“设备异常”还是“消息错配”。

## Assumptions & Decisions
- 假设 1：Sunway 机械手协议允许不同命令的 ACK/RPS 在时间上交错返回；当前日志已证明至少在软件侧观察到交错消费现象。
- 假设 2：`RPS:GETOBJECT;` 不带唯一 request id，因此不能靠事务号强关联，只能靠“命令上下文 + 消息类型/前缀”做软匹配。
- 决策 1：不使用“全局大锁把所有命令串行化”作为首选方案。
  - 原因：这会掩盖并发消费问题，但代价是直接牺牲 WTR 与上层流程的并发能力。
  - 仅当协议层确认绝不支持并发时，才退回串行化。
- 决策 2：优先采用“共享接收线程 + 按规则匹配消费”的方案。
  - 原因：改动面可控，且最符合现有架构。
- 决策 3：`pause All Threads` 本次按用户确认视为人为暂停操作，只解释日志来源，不纳入程序修复范围。
- 决策 4：修复范围覆盖所有通过 `recvResponseRobotMessage()` 取包的 WTR 命令，不只局限于 `RQLoad / GetWafer / Update`。

## Verification Steps
- 静态验证
  - 检查 `RQLoad`、`GetWafer`、`Update` 不再直接依赖 `clearRobotMessage()`。
  - 检查 helper 中不存在“无条件 front/pop 消费任意消息”的高风险路径。
- 日志回放验证
  - 用本次日志场景复盘，确认：
    - `RQLoad(B)` 遇到 `RPS:GETOBJECT;` 时不会立即报错，而会继续等待 `ACK:LOAD/B;` / `RPS:LOAD/ON;`
    - `GetWafer` 不会再消费 `RPS:LOAD/ON;`
    - `PutWafer`、`Home`、`Reset` 等命令遇到非本命令消息时会跳过并保留，而不是误判失败
- 运行验证
  - 执行真空流程与 PM2 交换场景，重点观察 `20:18:21` 这类 “GetWafer 未完成时又发起 RQLoad” 的窗口。
  - 查看是否仍出现：
    - `执行查询手指命令执行失败，机械手返回的指令未定义：RPS:GETOBJECT;`
    - `取晶圆命令执行失败，机械手返回的指令未定义：RPS:LOAD/ON;`
- 回归验证
  - 回归 `Home / Reset / PutWafer / CheckLoad / SetLoad / SetSpeed / Update / RQLoad / GetWafer`，确认未引入超时或消息滞留。
  - 对修改文件执行编译与诊断检查。

## 预期交付
- 一份明确解释：
  - `L11366-L11367` 为什么出现；
  - 它与本次串包问题的因果关系。
- 一套可实施代码改造：
  - 收包缓存支持按命令匹配消费；
  - `RQLoad`、`GetWafer`、`PutWafer` 及其他 WTR 命令不再互吃对方回包；
  - 保留现有接收线程模型，尽量减少调度层影响。
