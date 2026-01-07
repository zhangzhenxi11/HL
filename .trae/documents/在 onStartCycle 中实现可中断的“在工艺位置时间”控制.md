## 数据来源与换算
- 单次在工艺位置时间 perCycleMinutes = params.process_time_min / max(1, params.process_count)
- 保持“分钟”单位，不转换为秒或毫秒；仅在 wait_for 内部用 chrono 的自定义分钟 duration 类型承载该数值

## 线程同步与状态
- 在 QPmRecipeWidgetPrivate 增加：
  - std::condition_variable cycleCv
  - std::mutex cycleMutex
  - 已有：std::atomic<bool> stopRequested, isRunning, currentCycleCount

## 阻塞与唤醒条件
- 阻塞（Z1 工艺位置阶段）：
  - 计算截止时间 deadline = std::chrono::steady_clock::now() + minute_d(perCycleMinutes)
  - 使用 while 循环处理剩余时间，避免虚假唤醒：
    - 计算 remaining = deadline - now
    - cycleCv.wait_for(lock, remaining, [&]{ return stopRequested; })
- 唤醒：
  - 停止：onStopCycle 中设置 stopRequested=true 并调用 cycleCv.notify_all()，使等待立即返回
  - 可扩展：如后续硬件完成事件到来，也可在该事件处理里调用 cycleCv.notify_all() 提前结束等待

## 超时逻辑（到期）
- 若 wait_for 返回 false（谓词未触发，时间到期）→ 表示本次工艺位置时间“到期/超时”正常完成
- 若返回 true（谓词触发：stopRequested）→ 立即中断当前循环，执行收尾（恢复 UI 按钮、退出线程）
- 使用 steady_clock 与 while + remaining 的写法确保精度与可中断性

## 旋转角与步骤
- rotation_angle_deg 取自 params；若设备支持旋转：
  - 在 Z2 执行旋转命令（createRotatingActionCommand(angle)）并等待完成；随后进入 Z1 等待 perCycleMinutes 时间
- 其它移动步骤保持现有：Z2→Z1（工艺）→Z2→Z3

## 代码改动要点（不含实现细节）
- QPmRecipeWidgetPrivate：新增 cycleCv、cycleMutex 成员
- onStartCycle：
  - 读取 cfg.params（process_time_min、process_count、rotation_angle_deg）
  - 计算 perCycleMinutes，并在 Z1 阶段按上述“阻塞/唤醒/超时”逻辑等待
  - UI 当前次数通过 QMetaObject::invokeMethod 更新
- onStopCycle：设置 stopRequested=true 并调用 cycleCv.notify_all()

## 示例验证
- 15 分钟 / 6 次 → 单次 2.5 分钟；20 分钟 / 5 次 → 单次 4 分钟
- 启动后每次进入 Z1 都等待对应分钟数；点击“暂停”即停止并立刻唤醒，不再继续等待

我将按该方案落地到 onStartCycle/私有成员，并进行编译与功能验证。