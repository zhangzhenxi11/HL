Stop 按钮点击没有触发信号的问题，很可能是因为在点击 Stop 按钮时，程序虽然执行了停止逻辑，但并没有发射 `cycleStopped` 信号，或者信号发射的时机不对。

### 代码分析
在 `d:\HLPrj\HL\device\src\pm_recipe_widget.cpp` 中：

1.  **`onStopCycle` 函数**：
    ```cpp
    void QPmRecipeWidget::onStopCycle()
    {
        Q_D(QPmRecipeWidget);
        if (d->isRunning) {
            d->stopRequested = true;
            d->cycleCv.notify_all();
            d->ui->stop_pm_pbt->setEnabled(false); // Prevent multiple clicks
            
            // 注意：这里没有 emit cycleStopped();
        }
    }
    ```
    当用户点击 Stop 按钮时，只设置了 `stopRequested` 标志位并禁用了按钮，但**没有发射信号**。

2.  **线程逻辑 (`onStartCycle` 中的 Lambda)**：
    ```cpp
    d->cycleThread = std::thread([=](){
        // ... 循环逻辑 ...
        
        // 循环结束或被中断后：
        d->isRunning = false;
        // Restore UI state
        QMetaObject::invokeMethod(d->ui->start_pm_pbt, "setEnabled", Q_ARG(bool, true));
        QMetaObject::invokeMethod(d->ui->stop_pm_pbt, "setEnabled", Q_ARG(bool, false));
        
        QMetaObject::invokeMethod(this, "cycleStopped"); // <--- 这里尝试发射信号
    });
    ```
    线程结束时，通过 `invokeMethod` 调用了 `cycleStopped`。但是 `cycleStopped` 是一个信号，虽然 `invokeMethod` 可以触发信号，但如果 `onStopCycle` 仅仅是设置了标志位，线程可能还在等待某个操作（如 `cmd->wait()` 或 `timer`），导致延迟。
    更重要的是，用户点击 Stop 按钮时期望的是**立即**停止并更新状态。

3.  **问题核心**：
    您说“点击了pm配方界面stop没有触发”，指的是 `DataWidget` 没有收到停止信号。
    在 `onStopCycle` 中，仅仅设置了标志位，依赖后台线程退出时发射信号。
    如果后台线程此时卡在 `cmd->wait()`（例如移动指令正在执行中），它不会立即退出，必须等该指令执行完或超时，才会检查 `stopRequested` 并退出，最后发射信号。这会造成显著的延迟，让用户觉得“没有触发”。

### 解决方案
我们需要在用户点击 Stop 按钮时，**立即**发射 `cycleStopped` 信号，以便 `DataWidget` 能立即停止数据采集，而不必等待后台机械动作完全停止。

**修改计划**：
在 `QPmRecipeWidget::onStopCycle` 函数中，添加 `emit cycleStopped();`。

这样，一旦点击 Stop，`DataWidget` 就会立即收到信号并停止定时器/数据采集。后台线程可以随后慢慢退出，互不影响。

我将立即修改 `d:\HLPrj\HL\device\src\pm_recipe_widget.cpp`。