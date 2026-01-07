## 问题原因
- 出错信息表明使用了 `processElapsed += std::chrono::duration<double, ratio<60>>`：`double += duration` 不支持。
- 只有同类型才能 `+=`；若要使用 `duration`，需 `.count()` 转为纯数值。

## 修复策略
- 统一用“分钟”为业务度量，保持为 `double`；仅在阻塞/睡眠处把 `double` 转为 `chrono::duration`。
- 删除私有类中重复声明的 `processElapsed` 成员，避免命名遮蔽与潜在编译冲突。

## 具体改动
1. 引入别名：`using minute_d = std::chrono::duration<double, std::ratio<60>>;`
2. onStartCycle：
   - `double atProcessPosMinutes = cfg.params.process_time_min / std::max(1, cfg.params.process_count);`
   - 启动计时：`d->runTimer(atProcessPosMinutes);`（`runTimer(double minutes)` 内部 `sleep_for(minute_d(minutes))`）
   - 等待返回后累加：`d->processElapsed += atProcessPosMinutes;`
3. 若代码中已有 `processElapsed += minute_d(...)` 或把 `atProcessPosTime` 定义为 `minute_d`：
   - 改为 `processElapsed += minute_d(...).count()` 或直接改回 `double`。
4. onStopCycle 保持：仅在停止时 `stopRequested=true` 并 `notify_all()`，不做“到期唤醒”。
5. 删除 `QPmRecipeWidgetPrivate` 内重复的 `double processElapsed{ 0.0 };`，保留一个定义。

## 验证
- 编译通过，类型一致：`double += double`。
- 流程正确：到期由计时线程唤醒，停止即时唤醒；UI 状态恢复正常。