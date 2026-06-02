# PM2交换闪退与Dump补强计划

## Summary

本次问题表现为：循环运行到 `PM2:PM2交换料：A取B放.` 后应用直接退出，`any_cleaned` 在该行后无后续业务日志，直到数小时后出现新一轮启动日志；同时未生成可用 dump，导致无法从异常栈直接定位。

基于现有日志与源码，当前判断这不是“WTR 命令已经发出后机械手报错”的问题，而更像是 **PM2 调度线程在进入交换分支前后发生进程级异常**，且现有 dump 机制覆盖范围不足，导致异常现场丢失。

## Current State Analysis

### 1. 日志时间线

* 总日志在 `2026-06-02_any_cleaned.txt` 的 [28994-28996](file:///d:/HLPrj/HL/_collected_latest_20260602_085046/2026-06-02_any_cleaned.txt#L28994-L28996) 停在：

  * `PM2调度200: haswaferpm=1, armA_has=0, armA_pending=0, armB_has=1, armB_pending=1, pending=1, return_pending=1`

  * `PM2交换料：A取B放.`

* 该文件在紧接着的 [28997-29008](file:///d:/HLPrj/HL/_collected_latest_20260602_085046/2026-06-02_any_cleaned.txt#L28997-L29008) 已经进入下一次程序启动初始化，说明原进程是异常退出，不是正常回到空闲态。

* `LLA` 在同一窗口持续停留在 `step 1051`，反复看到 `pm2Has=1, armA_has=0, armB_has=1`，最后一条停在 [4286-4288](file:///d:/HLPrj/HL/_collected_latest_20260602_085046/2026-06-02_lla.txt#L4286-L4288)。

* `TaskManager` 最后状态更新停在 [872-875](file:///d:/HLPrj/HL/_collected_latest_20260602_085046/2026-06-02_taskmanager.txt#L872-L875)，即 `task 14` 从 `PM_PROCESS/COMPLETED` 切到 `LOADLOCK_RETURN/QUEUED`。

* `WTR` 日志在该时间窗没有出现 `Robot线程：step：5100,PM2交换片...`，说明进程退出点大概率发生在 **PM2 调度线程刚决定进入交换分支之后、WTR Robot 线程真正消费请求之前**。

### 2. 直接对应的源码路径

* 崩溃前最后一条业务日志 `PM2交换料：A取B放.` 来自 [slot\_transfer\_cycle\_vtm\_widget.cpp:6639-6643](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp#L6639-L6643)。

* 该分支随后仅做一件事：`pm2_auto_step.store(1060);`，下一轮进入 [1060 分支](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp#L6785-L6801)，并设置：

  * `exchange_info_pm2.getArm = 0`

  * `exchange_info_pm2.putArm = 1`

  * `robot_exchange_pm2.requested = true`

* WTR Robot 线程通过 [slot\_transfer\_cycle\_vtm\_widget.cpp:8022-8024](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp#L8022-L8024) 轮询 `robot_exchange_pm2.requested`，进入 [5100 分支](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp#L8363-L8417) 执行交换。

### 3. 当前最可疑的崩溃源头

按优先级排序，当前最值得先修的点是：

* **工作线程直接访问 UI 控件**

  * `case 200` 内直接读取 `ui->simulation_cbx->checkState()`，见 [slot\_transfer\_cycle\_vtm\_widget.cpp:6494-6499](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp#L6494-L6499)。

  * 同文件还有多处后台线程直接访问 `ui->simulation_cbx`，共 17 处。

  * 在 Qt 中，非 UI 线程直接访问 QWidget/QObject UI 状态属于未定义行为，可能表现为随机闪退、无稳定复现点、也不一定能稳定进入 `SetUnhandledExceptionFilter`。

* **PM2 占片判断存在潜在空指针解引用**

  * 例如 [slot\_transfer\_cycle\_vtm\_widget.cpp:6502-6504](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp#L6502-L6504)、[3391-3393](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp#L3391-L3393)、[4920-4922](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp#L4920-L4922)。

  * 代码只判断了 `pm2 != nullptr && cassManager != nullptr`，但没有判断 `cassManager->getCassette(pm2.get())` 是否为空；若返回空指针，`->getMapping(1)` 会直接崩溃。

* **PM2 交换窗口缺少“交换进行中”防重入保护**

  * `case 200` 选择 `1060/1070` 后，LLA/LLB 线程仍在高频跑 `1051`，日志表明在交换决策前后仍反复读取和更新任务状态。

  * 当前缺少一个明确的“PM2 exchange in-flight”门闩，来阻止 LL 线程在交换请求已经发出、但 Robot 线程尚未消费时继续干扰同一批任务与手臂状态。

  * 这更像逻辑竞争窗口，虽然不一定直接导致崩溃，但会放大崩溃概率并严重降低现场可解释性。

### 4. 为什么没有 dump

* 目前 dump 注册只在 [main.cpp:35-36](file:///d:/HLPrj/HL/app/main.cpp#L35-L36) 使用 `SetUnhandledExceptionFilter`。

* dump 实现在 [CCreateDump.cpp](file:///d:/HLPrj/HL/app/CCreateDump.cpp) 中，存在以下缺口：

  * dump 文件名只在启动时生成一次，且为相对路径，如 `2026-06-02_...app_dump.dmp`，没有固定输出目录，排查时不直观。

  * `CreateFileW` / `MiniDumpWriteDump` 失败时没有任何可见日志落地。

  * 仅覆盖 `SetUnhandledExceptionFilter`，未覆盖 `std::terminate`、`abort`、`purecall`、`invalid parameter`、`signal(SIGABRT/SEGV/FPE/ILL)` 等常见异常退出路径。

  * 某些 Qt/第三方库触发的快速终止并不一定稳定走到当前过滤器。

## Proposed Changes

### 1. `device/src/slot_transfer_cycle_vtm_widget.cpp`

目标：先把最像“闪退根因”的未定义行为和空指针风险去掉，再缩小 PM2 交换竞争窗口。

计划修改：

* 新增一个线程安全的“仿真模式缓存值”，由 UI 线程更新，工作线程只读缓存，不再直接读 `ui->simulation_cbx->checkState()`。

* 替换本文件所有后台线程里的 `ui->simulation_cbx->checkState()` 访问，至少覆盖 PM2、LLA、LLB、EFEM、TM 等流程线程入口。

* 抽出一个安全的 PM cassette 状态读取辅助函数，例如：

  * 输入：`FortrendCassetteManager*`、`FortrendPMCavitySubsystem*`

  * 输出：是否有片 + 是否读取成功

  * 处理：判空 `cassManager`、`pm`、`cassette`

  * 日志：若 cassette 为空，打印明确错误并进入等待/回退，不直接崩溃

* 在 PM2 `case 200 -> 1060/1070` 之间增加“交换请求已发出/交换执行中”的保护：

  * 条件可基于 `robot_exchange_pm2.requested`、`pm2_auto_step` 当前值，或新增单独原子标志

  * LLA/LLB 的 `1051` 若观察到 PM2 正在交换，则不再继续修改手臂/任务，只回退等待

* 在 `1060/1070/5100` 的前后补充更细日志：

  * 进入交换前的任务快照

  * `requested` 被置位时间

  * Robot 线程真正消费该请求的时间

  * 若状态不满足则记录完整上下文后回退

为什么这样改：

* 能消除 Qt 非 UI 线程访问控件带来的未定义行为。

* 能消除 PM cassette 为空时的直接空指针崩溃。

* 能减少交换请求发出到 Robot 线程消费之间的竞争窗口。

### 2. `app/CCreateDump.cpp`

目标：即使再次闪退，也必须尽量拿到 dump 或至少拿到 dump 失败原因。

计划修改：

* dump 输出改为固定目录，例如程序目录下 `dumps/`。

* 每次异常时动态生成唯一文件名，而不是只在启动时拼一次名字。

* 在写 dump 前显式创建输出目录。

* 对 `CreateFileW` 和 `MiniDumpWriteDump` 的失败路径补充可见日志或 `OutputDebugString`。

* 将当前 `MiniDumpNormal` 升级为更适合现场分析的类型，优先考虑保留线程/句柄/间接内存信息的组合。

为什么这样改：

* 先解决“明明崩了但不知道 dump 到哪了”的问题。

* 即使 dump 写失败，也能知道是路径、权限还是 API 失败。

### 3. `app/CCreateDump.h`

目标：为 dump 扩展新的安装入口和辅助方法。

计划修改：

* 增加初始化/安装全部异常处理器的方法声明。

* 增加生成 dump 绝对路径、写 dump 的统一辅助接口。

### 4. `app/main.cpp`

目标：启动时一次性装齐异常处理器。

计划修改：

* 保留现有 `SetUnhandledExceptionFilter`，并改为调用新的统一安装接口。

* 安装补充处理器：

  * `std::set_terminate`

  * `_set_purecall_handler`

  * `_set_invalid_parameter_handler`

  * `signal(SIGABRT)`

  * `signal(SIGSEGV)`

  * `signal(SIGFPE)`

  * `signal(SIGILL)`

  * 视实现复杂度决定是否加 `AddVectoredExceptionHandler`

为什么这样改：

* 当前过滤器覆盖面太窄，很多“闪退”路径不会留下 dump。

## Assumptions & Decisions

* 本轮先按“高概率根因 + 先提升可观测性”的策略处理，不假设单一根因已被百分百锁定。

* 优先修复 PM2 交换路径，而不是一次性清理整个工程所有线程/UI 访问问题；但 `slot_transfer_cycle_vtm_widget.cpp` 内的同类 UI 访问会一起替换，避免同类问题残留。

* 先不扩大到全工程所有 WTR 调用加锁改造；`slot_transfer_cycle_vtm_widget.cpp` 当前已通过 Robot 线程集中串行化主要 PM2 交换动作。若修复后仍有偶发问题，再单独对 `wtr_robot_mutex` 做全文件审计。

* 本轮目标不是“直接证明唯一崩溃指令”，而是：

  * 去掉最明显的未定义行为/空指针风险

  * 缩小 PM2 交换竞争窗口

  * 确保下次异常一定留下更强现场

## Verification

### 日志验证

* 复跑相同循环场景，确认在进入 `PM2交换料：A取B放/B取A放` 后：

  * 能看到 `1060/1070` 进入日志

  * 能看到 `robot_exchange_pm2.requested` 置位日志

  * 能看到 Robot 线程 `5100` 消费日志

* 确认 LLA/LLB 在 PM2 交换进行中不会继续反复改写同一批任务/手臂状态。

### dump 验证

* 人工触发一次受控异常，确认程序目录下 `dumps/` 能生成 `.dmp`。

* 若未生成，也要能在调试输出/日志中看到失败原因。

### 回归验证

* 验证正常 PM2 上料、回片、交换三条路径仍可跑通：

  * 单臂上料到 PM2

  * 单臂从 PM2 回 LL

  * `A取B放` / `B取A放` 交换

* 验证 `TaskManager` 中 `PM_PROCESS -> LOADLOCK_RETURN` 的切换后，LLA/LLB 仍能继续推进，不引入新卡死。

