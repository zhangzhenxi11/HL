# LLB放片失败后未隔离导致EFEM撞手指修复方案

## Summary
- 目标：修复 `WTR` 放片到 `LLB` 失败后，系统未及时进入故障隔离态，导致 `LLA/EFEM` 线程继续推进并去 `LLB` 取片，形成撞手指风险的问题。
- 范围：以 [slot_transfer_cycle_vtm_widget.cpp](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp) 为唯一主修改文件，重点覆盖 `LLB 2060/2065`、Robot `4100 PUT_TO_LLB`、EFEM 调度入口、必要的全局故障闸门与日志。
- 成功标准：
  - 一旦 `PUT_TO_LLB` 失败，系统立即进入“LLB 故障占用/停止推进”状态。
  - `EFEM` 不再因为 `tool_allow_put_wafer_LLB` 或队列残留而继续从 `LLB` 取片。
  - `LLA/LLB/WTR/EFEM` 至少在本轮循环内全部停在安全态，直到人工复位或统一恢复流程处理。
  - 日志能明确说明：哪个步骤失败、为什么禁止后续线程继续推进、是哪一侧 LL 被故障隔离。

## Current State Analysis
- 额外检查结果：当前 [slot_transfer_cycle_vtm_widget.cpp](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp) 里显式的 `case 2065` 只有两处：
  - [LLA 2065](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp#L4487-L4501)
  - [LLB 2065](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp#L6114-L6127)
  - 两者现状一致，都是：
    - 等待 `robot_put_to_ll*.done`
    - 若 `success=false`
    - 则 `logFailed(...)`
    - `Sleep(2000)`
    - 再把 `loadlock*_auto_step = 950`
  - 这说明 2065 失败在当前设计里被统一视作“可回跳重试”的普通失败，而不是“必须立刻停线程”的安全故障。
- 现场日志显示，`WTR` 在 `21:28:06` 发起 `PUTOBJECT/6/1/A`，并在 `21:28:31` 收到错误：
  - [2026-06-04_cleaned.txt:L53284-L53371](file:///d:/HLPrj/HL/2026-06-04_cleaned.txt#L53284-L53371)
  - 关键证据：
    - `WTR:step 2065,等待放LLB晶圆完成...`
    - `WTR:RPS: res:ALARM:ERROR/...检测到纠偏長度过大...`
    - `WTR:放晶圆命令执行失败.`
- 但失败后并没有进入全局停机或 LLB 故障隔离；相反，`LLA` 线程很快又继续判断 `WTR空闲`：
  - [2026-06-04_cleaned.txt:L53372-L53373](file:///d:/HLPrj/HL/2026-06-04_cleaned.txt#L53372-L53373)
  - 后续 [2026-06-04_cleaned.txt:L53418-L53420](file:///d:/HLPrj/HL/2026-06-04_cleaned.txt#L53418-L53420) 仍持续进入 `LLA step 1051`
- 更危险的是，`EFEM` 在 `21:28:39` 继续去 `LLB` 取片：
  - [2026-06-04_cleaned.txt:L53422-L53436](file:///d:/HLPrj/HL/2026-06-04_cleaned.txt#L53422-L53436)
  - 这正对应用户描述的“WTR 停在 LLB，没有停止所有线程，EFEM 去 LLB 取片，很容易撞手指”。
- 代码层根因一：Robot `PUT_TO_LLB` 失败后只回填 `success=false`，没有触发全局故障闸门。
  - [slot_transfer_cycle_vtm_widget.cpp:L8947-L8968](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp#L8947-L8968)
  - 当前逻辑：
    - `cmd->hasError()` 时仅 `robot_put_to_llb.success=false`
    - 然后仍然 `requested=false`、`done=true`、`robot_step=10`
  - 这会把 Robot 线程直接放回“空闲调度”状态，但现场机械手很可能仍停在危险位置。
- 代码层根因二：`LLB 2065` 失败后虽然会通过 `logFailed()` 进入 `pauseAllThreads()`，但它没有形成“LLB 当前故障占用/硬件隔离”语义，随后本线程仍继续执行失败分支并回退到 `950`。
  - [slot_transfer_cycle_vtm_widget.cpp:L6114-L6138](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp#L6114-L6138)
  - `logFailed()` 内部确实会调用 `pauseAllThreads()`，见 [slot_transfer_cycle_vtm_widget.cpp:L10507-L10518](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp#L10507-L10518)
  - `pauseAllThreads()` 也确实会把 `running=false`、`ispause=true`，见 [slot_transfer_cycle_vtm_widget.cpp:L9406-L9424](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp#L9406-L9424)
  - 当前逻辑：
    - `robot_put_to_llb.success=false` 时
    - `llbImmediateRepick.reset();`
    - 调 `logFailed(...)`，触发 `pauseAllThreads()`
    - 但紧接着当前失败分支仍继续执行 `Sleep(2000)` 和 `loadlock2_auto_step = 950`
  - 这说明当前实现的“暂停”是**协作式暂停**，不是“立刻把硬件故障点隔离并冻结所有后续调度条件”。
- 代码层根因三：`pauseAllThreads()` 只对各线程循环入口的 `cv.wait(running||stopRequested)` 生效，无法替代硬件故障闸门。
  - 多个主线程循环都是在 while 顶部等待 `running`，例如：
    - [LLA 线程](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp#L3273-L3280)
    - [LLB 线程](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp#L4912-L4917)
    - [PM1 线程](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp#L6540-L6545)
  - 这类暂停机制只能在“线程回到循环等待点”时阻塞后续迭代；对于：
    - 已经发出的硬件命令
    - 当前分支里仍会继续执行的 `Sleep / step 赋值 / 标志位残留`
    - 其他线程已经拿到的旧请求条件
    并不能提供硬件级隔离。
- 代码层根因四：`EFEM` 入口只看 `tool_allow_put_wafer_LLB`/任务队列，不看“LLB 当前是否处于 WTR 故障占用态”。
  - [slot_transfer_cycle_vtm_widget.cpp:L1491-L1524](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp#L1491-L1524)
  - 当前逻辑里，只要：
    - `tool_allow_put_wafer_LLB == true`
    - 或 `efemReturnPendingLLBTasks.size() > 0`
    - 就会把 `current_loadlock = "LLB"` 并推进到 `efem_auto_step = 200`
- 代码层根因五：`EFEM 243` 真正执行取 `LLB` 时，没有任何跨线程安全互锁来挡住“LLB 仍被 VTM/WTR 占用”的场景。
  - [slot_transfer_cycle_vtm_widget.cpp:L2730-L2745](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp#L2730-L2745)
  - 这里直接 `ewtr->createGetCommand(lk, 1, efemReturnPendingTasks.at(0).targetBlankingSlot)`
- `stopAllThreads()` 虽然存在，但当前这条失败链走的是 `pauseAllThreads()`，不是 `stopAllThreads()`。
  - [slot_transfer_cycle_vtm_widget.cpp:L9426-L9447](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp#L9426-L9447)
- 结合现场与代码，可归纳成一句话：
  - **当前系统虽然会“暂停线程”，但没有把“放LLB失败”升级成“现场机械手位置未知/存在实物占用”的危险故障隔离态。**

## Proposed Changes
### 1. 新增“LLA/LLB 故障占用闸门”，把放片失败从普通回退升级为安全隔离态
- 文件：[slot_transfer_cycle_vtm_widget.cpp](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp)
- 修改内容：
  - 在 `Private` 中新增最小状态位，例如：
    - `std::atomic<bool> llbTransferFaultActive{ false };`
    - `std::atomic<bool> llaTransferFaultActive{ false };`
    - `std::atomic<bool> wtrUnsafePositionActive{ false };`
  - 额外保存失败原因字符串或错误码，供日志打印与 UI/复位流程使用。
- 为什么这样改：
  - 现场本质不是“普通重试失败”，而是“机械手可能停在危险位置”。
  - 需要一个跨线程都能看见的统一故障闸门，而不是散落在某个 `auto_step` 里。

### 2. 把 Robot `PUT_TO_LLB` 失败路径改成“置故障闸门 + 不再宣告可安全继续”
- 文件：[slot_transfer_cycle_vtm_widget.cpp](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp)
- 位置：Robot `4100`，见 [slot_transfer_cycle_vtm_widget.cpp:L8947-L8968](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp#L8947-L8968)
- 修改内容：
  - 当 `cmd->hasError()` 时：
    - 置 `llbTransferFaultActive = true`
    - 置 `wtrUnsafePositionActive = true`
    - 记录故障原因和 `alarm code/message`
    - `robot_put_to_llb.success = false`
    - 保留 `done=true` 用于通知等待方，但日志明确标识“进入故障隔离”
  - 不再把这类失败仅当作一般错误码记录后返回空闲。
- 为什么这样改：
  - 这一步是最接近第一现场的地方，必须在这里统一声明“此后禁止其他线程继续碰 LLB/WTR”。

### 3. 修改 `LLB 2065`：失败后不再继续执行“暂停后回 950 重试”，而是切到安全停机/等待复位分支
- 文件：[slot_transfer_cycle_vtm_widget.cpp](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp)
- 位置：`LLB case 2065`，见 [slot_transfer_cycle_vtm_widget.cpp:L6114-L6138](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp#L6114-L6138)
- 修改内容：
  - `robot_put_to_llb.success == false` 时：
    - 允许保留 `logFailed(...) -> pauseAllThreads()` 这条现有链
    - 但要把它升级为“暂停 + 置故障隔离闸门 + 不再继续回退重试”
    - 即：
      - `llbTransferFaultActive = true`
      - `wtrUnsafePositionActive = true`
      - 清理本次请求标志与立即补取状态
      - 不再执行 `loadlock2_auto_step = 950`
      - 改为跳到新的故障等待步骤，例如 `9090/9990`
      - 该步骤只打印“等待人工复位/统一恢复”，不再推进工艺动作
  - 视现有线程生命周期，决定是否额外引入 `stopAllThreads()`；但无论是否调用，都必须有独立故障闸门，不能只依赖 `pauseAllThreads()`。
- 为什么这样改：
  - 这次现场已经证明，“回 950 重试”会放开其他线程，直接制造碰撞风险。
  - 这里必须优先保证安全，不是优先保节拍。

### 3.1 同步修改 `LLA 2065`：与 `LLB 2065` 保持相同的“失败即停”语义
- 文件：[slot_transfer_cycle_vtm_widget.cpp](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp)
- 位置：`LLA case 2065`，见 [slot_transfer_cycle_vtm_widget.cpp:L4487-L4501](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp#L4487-L4501)
- 修改内容：
  - 当 `robot_put_to_lla.success == false` 时：
    - 保留 `logFailed(...)`
    - 删除 `Sleep(2000)` 与 `loadlock1_auto_step = 950`
    - 不再把失败视为普通回跳
    - 直接进入与 `LLB` 同构的故障等待/停止线程语义
    - 同时置位 `llaTransferFaultActive`，必要时也置位 `wtrUnsafePositionActive`
- 为什么这样改：
  - 用户已明确要求“像流程中的 2065 失败了都不要重试会跳转，直接停止线程”
  - 不能只修 `LLB`，否则 `LLA` 仍保留同类风险。

### 3.2 统一规则：所有“等待 WTR/Robot 放片完成”的 2065 分支，失败后都不允许 `Sleep + 回 950`
- 文件：[slot_transfer_cycle_vtm_widget.cpp](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp)
- 修改内容：
  - 抽象出统一原则，而不是只在单点写死：
    - `case 2065` 失败 = 安全故障
    - 不允许自动重试
    - 不允许 `loadlock*_auto_step = 950`
    - 不允许在本线程里继续推进后续调度
  - 具体实现可选两种，但方案固定为“停止线程优先”：
    - 方案 A：设置统一停止标志并进入故障等待步
    - 方案 B：调用统一停止接口后，让线程在循环入口阻塞
  - 无论用 A 还是 B，行为标准都一致：**2065 失败后线程只允许停，不允许跳转重试**
- 为什么这样改：
  - 这次需求本质是“把 2065 失败从工艺失败改成安全停机事件”
  - 需要规则级约束，避免未来另一侧或类似流程再次写回“950 重试”

### 4. 在 `LLA/LLB 10/950/1051/2055/2060` 等入口统一检查故障闸门，禁止继续发新请求
- 文件：[slot_transfer_cycle_vtm_widget.cpp](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp)
- 修改内容：
  - 在 `LLA/LLB` 主循环前置增加统一判断：
    - 若 `llbTransferFaultActive` 为真，则：
      - `LLB` 线程不再推进
      - `LLA` 线程也不得继续基于 `wtr->isBusy()` 发新取片请求
      - 日志打印“LLB 当前处于放片失败故障隔离，禁止继续调度”
  - 不再单纯依赖 `wtr->isBusy()` 作为“是否可安全继续”的唯一判据。
- 为什么这样改：
  - 现场已经证明，`wtr->isBusy()==false` 不等于“机械手已经安全离开 LLB”。
  - 故障态必须比 `busy` 更高优先级。

### 5. 在 EFEM 入口增加“LLB 故障占用”互锁，故障态时禁止推进 `LLB` 上下料
- 文件：[slot_transfer_cycle_vtm_widget.cpp](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp)
- 位置：EFEM 主调度入口，见 [slot_transfer_cycle_vtm_widget.cpp:L1491-L1524](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp#L1491-L1524)
- 修改内容：
  - 在进入：
    - `tool_allow_put_wafer_LLB || efemReturnPendingLLBTasks.size() > 0`
    - `tool_allow_get_wafer_LLB || efemPendingLLBTasks.size() > 0`
    之前，先检查：
    - `llbTransferFaultActive`
    - `wtrUnsafePositionActive`
  - 若任一为真：
    - `EFEM` 不得把 `current_loadlock` 设为 `LLB`
    - 保持等待并打明确日志
    - 必要时直接把 `tool_allow_put_wafer_LLB/tool_allow_get_wafer_LLB` 置回 `false`
- 为什么这样改：
  - 这是阻断“EFEM 去 LLB 取片撞手指”的关键门口。
  - 必须在调度入口就挡住，而不是等 `243` 真正发命令时才发现。

### 6. 在 EFEM 执行 `243/244` 之前再做一次硬互锁校验
- 文件：[slot_transfer_cycle_vtm_widget.cpp](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp)
- 位置：`case 243/244`，见 [slot_transfer_cycle_vtm_widget.cpp:L2730-L2763](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp#L2730-L2763)
- 修改内容：
  - 在 `createGetCommand(lk, ...)` 前再检查一次：
    - 当前目标是否为 `LLB`
    - 若是 `LLB`，且 `llbTransferFaultActive || wtrUnsafePositionActive`
    - 直接打断并回退等待，不发命令
  - 对 `LLA` 也可同步加同构保护，避免未来同类问题只修一侧。
- 为什么这样改：
  - EFEM 入口拦截是第一道门，这里是第二道门。
  - 涉及硬件碰撞风险时，必须允许双保险。

### 7. 统一故障恢复语义：只有显式复位/恢复才允许清除故障闸门
- 文件：[slot_transfer_cycle_vtm_widget.cpp](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp)
- 修改内容：
  - `llbTransferFaultActive / wtrUnsafePositionActive` 不应在普通循环中自动清掉。
  - 只允许在：
    - `resetAll` / 已有整机复位流程
    - 或用户明确的恢复步骤
    中清零。
- 为什么这样改：
  - 如果失败后自动清掉，系统会再次回到“误认为可以继续调度”的危险状态。

### 8. 增强日志与证据链，后续可直接判定“已故障隔离”
- 文件：[slot_transfer_cycle_vtm_widget.cpp](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp)
- 修改内容：
  - 新增关键日志：
    - `PUT_TO_LLB failed -> enter fault isolation`
    - `LLB fault active, skip LLB scheduling`
    - `EFEM blocked by LLB fault isolation`
    - `waiting manual reset after LLB put failure`
  - 日志中带上：
    - `robot_step`
    - `loadlock2_auto_step`
    - `arm`
    - `slot`
    - `alarm code/message`
- 为什么这样改：
  - 以后复盘时可以一眼看出系统有没有真的进入隔离态，而不是只能靠猜。

## Assumptions & Decisions
- 假设一：这次问题的最高优先级是“避免撞手指”，高于“失败后自动重试恢复节拍”。
- 假设二：`WTR` 放 `LLB` 失败后，不能再相信 `wtr->isBusy()==false` 足以代表安全。
- 决策一：本轮采用“最小语义改动但安全优先”的方案，不重构全部状态机，只在关键入口新增故障闸门和等待步骤。
- 决策二：不只修 `LLB`；当前显式 `2065` 的 `LLA/LLB` 两处都统一改成“失败即停线程”，不再 `950` 重试。
- 决策三：保留现有 `logFailed() -> pauseAllThreads()` 机制，但不把它当作唯一安全保障；必须额外增加故障隔离闸门与故障等待步骤。`stopAllThreads()` 是否直接调用，以“避免线程自 join 死锁/线程上下文不清”为前提另行决定。
- 决策四：禁止用“仅把 `tool_allow_put_wafer_LLB=false`”作为唯一修复，因为这只能影响 EFEM 请求标志，挡不住其基于队列残留再次推进。

## Verification Steps
- 日志回放验证
  - 使用当前现场日志 [2026-06-04_cleaned.txt:L53284-L53371](file:///d:/HLPrj/HL/2026-06-04_cleaned.txt#L53284-L53371) 与 [2026-06-04_cleaned.txt:L53422-L53436](file:///d:/HLPrj/HL/2026-06-04_cleaned.txt#L53422-L53436) 建立对照。
  - 验证修复后应满足：
    - `PUT_TO_LLB` 失败后立即打出“进入故障隔离”日志
    - 不再出现 `LLA step 1051 WTR空闲，准备取片`
    - 不再出现 `EWTR:start command ... GetWafer[from LLB ...]`
- 代码静态验证
  - 检查 `Robot 4100` 失败路径是否置位 `llbTransferFaultActive/wtrUnsafePositionActive`
  - 检查 `LLA 2065` 与 `LLB 2065` 是否都不再有 `Sleep(2000)` 和 `loadlock*_auto_step = 950` 的失败回跳
  - 检查 EFEM `1491-1524` 与 `243/244` 是否都增加了 LLB 故障互锁
- 时序验证
  - 输出 `WTR 时序对照表`：
    - 期望：放 LLB 失败 -> 进入隔离 -> 所有相关线程等待/停机
    - 实际：日志行号对照
    - 偏差：若任何线程仍推进到 `LLB` 动作，视为修复失败
- 工程验证
  - 修改后检查 [slot_transfer_cycle_vtm_widget.cpp](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp) 无新增诊断错误。
  - 保持文件编码与 CRLF 换行不变。
