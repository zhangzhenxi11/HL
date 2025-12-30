## 目标

* 在 PM 配方界面新增两个控制按钮（启动/暂停），并实时显示“工艺次数”。

* 仅围绕 UI 与循环控制逻辑改造，不改动其它模块。

## UI改造

* 文件：[pm\_recipe\_widget.ui](file:///D:/HLPrj/HL/device/src/pm_recipe_widget.ui)

* 顶部按钮区新增：

  * QPushButton start\_pm\_loop\_pbt 文本“启动PM循环”

  * QPushButton pause\_pm\_loop\_pbt 文本“暂停PM循环”

  * QLabel cycle\_count\_lbl 初始文本“工艺次数: 0”

## 代码改造

* 文件：[pm\_recipe\_widget.h](file:///D:/HLPrj/HL/device/include/pm_recipe_widget.h)、[pm\_recipe\_widget.cpp](file:///D:/HLPrj/HL/device/src/pm_recipe_widget.cpp)

* 新增成员：

  * std::thread pmLoopThread

  * std::atomic<bool> running{false}, stopRequested{false}

  * std::condition\_variable cv；std::mutex mtx

* 新增槽：

  * onStartPmLoop()：设置 running=true；若线程未启动则启动；唤醒 cv

  * onPausePmLoop()：设置 running=false

* UI 更新：

  * 定义信号 emitCycleChanged(int)

  * 在主线程槽中更新 cycle\_count\_lbl 文本（避免子线程直接改 UI）

* 析构：stopRequested=true；join 线程

## 循环逻辑

* 依据当前 Tab 选择 PMx，读取 pmMotionProcessData\[PMx] 的行数作为总工艺次数

* 线程体：

  * while (!stopRequested)：若 !running 则 cv.wait；否则按序递增 cycle = 1..N，emitCycleChanged(cycle)

  * 暂不触发具体运动命令，仅实现循环计数与控制框架（后续可接入运动控制）

## 验证

* 加载数据后，点击“启动PM循环”即开始计数；“暂停PM循环”计数停止；切换 Tab 时仍按各自数据行数统计。

