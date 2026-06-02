﻿﻿﻿﻿﻿# 旧版 `slot_transfer_cycle_vtm_widget.cpp` 调度系统分析

## 1. 分析范围

- 目标文件：`d:\HLPrj\HL\slot_transfer_cycle_vtm_widget.cpp`
- 目标：从旧版实现反推调度设计意图，不修改源码。
- 重点：理解 WTR 为什么作为 LL 与 PM 交互核心，被集中放在单独线程中执行。
- 说明：当前文件中叠加了后续半自动逻辑，但全自动主链路仍保持完整，本文优先分析主链路。

## 2. 调度架构图

```text
                   +------------------------------+
                   | UI / Cycle表格 / 参数配置    |
                   | setTransferSequence()        |
                   +--------------+---------------+
                                  |
                                  v
         +---------------------------------------------------+
         |                共享调度数据区                     |
         |---------------------------------------------------|
         | 配置队列                                          |
         | - sequence_tolk1_wafer / sequence_tolk2_wafer     |
         | - sequence_lp1_transfer_wafer / lp2_transfer      |
         | - sequence_robot_transfer_wafer                   |
         |                                                   |
         | 运行队列                                          |
         | - sequence_lp*_get_wafer / sequence_lp*_put_wafer |
         | - sequence_loadlock*_transfer_wafer               |
         | - sequence_loadlock*_put_wafer                    |
         | - sequence_robot_get_wafer                        |
         |                                                   |
         | 协同标志                                          |
         | - tool*_allow_get/put_wafer                       |
         | - loadlock*_allow_get/put_wafer                   |
         | - pm_allow_get_put_wafer / pm_allow_goto_craft    |
         | - *_get_vacuum / cycleFinished_*                  |
         +-----+----------------+---------------+------------+
               |                |               |
               v                v               v
      +---------------+  +---------------+  +---------------+
      | EFEM线程      |  | LLA线程       |  | LLB线程       |
      | LP <-> LL     |  | LLA状态机     |  | LLB状态机     |
      +-------+-------+  +-------+-------+  +-------+-------+
              \               |                   /
               \              |                  /
                \             v                 /
                 +-----------------------------+
                 | WTR / Robot线程             |
                 | 真空侧唯一搬运执行者        |
                 | LL -> PM -> LL 串行执行     |
                 +-------------+---------------+
                               |
                               v
                        +-------------+
                        | PM线程      |
                        | 工艺与回位  |
                        +-------------+

                  +-------------------+   +-------------------+
                  | Vacuum线程        |   | Update线程        |
                  | 真空控制协调      |   | 轮次收口与重装填  |
                  +-------------------+   +-------------------+
```

## 3. 核心职责划分

### 3.1 UI 与配置层

- 从界面表格生成 LP、LL、Robot 三类初始队列。
- 读取 cycle 次数、模式和 PM 参数。
- 负责启动、暂停、复位整个 Cycle。

设计意图：

- 先把“本轮要跑什么”编译成队列。
- 运行期主要消费内存中的队列，减少对 UI 的实时依赖。

### 3.2 EFEM 线程

- 负责 ELP 与 LL 之间的上下料。
- EWTR 从 LP 取片并放入 LLA/LLB。
- 在回片阶段从 LL 取片并放回 LP。
- 将 LL 上料完成的晶圆推进到 `sequence_loadlock*_transfer_wafer`，并提前登记回片目标到 `sequence_lp*_put_wafer`。

设计意图：

- EFEM 只处理大气侧设备，不直接碰 WTR。
- 上料和下料放在同一线程，避免 LP/EWTR 命令交错。

### 3.3 LLA / LLB 线程

- 各自负责 LoadLock 的破真空、关门、抽真空、mapping、移槽、开关 TM 门。
- 决定何时通知 EFEM 上料或下料。
- 决定何时允许 WTR 从 LL 取片，或向 LL 回片。
- 维护本 LL 的“待送 PM 队列”和“待回片队列”。

设计意图：

- LL 不只是缓存腔，也是大气侧与真空侧的节拍边界。
- LL 线程做时序闸门，WTR 线程做最终搬运执行。

### 3.4 Robot / WTR 线程

- 真空侧唯一搬运执行者。
- 统一执行：从 LL 取片、向 PM 放片、从 PM 取片、向 LL 回片。
- 维护 `sequence_robot_transfer_wafer` 和 `sequence_robot_get_wafer` 两条核心队列。
- 在 PM 段根据 PM 是否有片、双手臂是否带片，决定放片、取片还是交换料。

设计意图：

- WTR 是全系统最关键的共享硬件资源，必须集中串行控制。
- 真空侧复杂互锁和交换片逻辑必须放在一个地方统一判断。

### 3.5 PM 线程

- 控制 PM 在上下料位和工艺位之间切换。
- WTR 放片完成后启动工艺。
- 工艺完成后回到上下料位，重新允许 WTR 取片。

设计意图：

- PM 线程只关注“什么时候能接片、什么时候做工艺、什么时候能出片”。
- 工艺与搬运解耦。

### 3.6 Vacuum 线程

- 响应 `loadlock1_get_vacuum`、`loadlock2_get_vacuum`、`tm_get_vacuum` 等标志。
- 负责 LL 与 TM 的抽真空协调。

设计意图：

- 真空控制抽成公共能力，避免多个线程重复控制。

### 3.7 Update 线程

- 判断 LP1 / LP2 当前一轮是否完成。
- 达到轮次上限则收尾，否则从 copy 队列恢复下一轮。
- 更新界面上的 cycle 完成计数。

设计意图：

- 把“多轮次收口逻辑”独立出来，不污染主搬运状态机。

## 4. 状态机设计

### 4.1 总体特征

旧版系统采用整数步号状态机，每个线程维护自己的 `*_auto_step`：

- `efem_auto_step`
- `loadlock1_auto_step`
- `loadlock2_auto_step`
- `robot_auto_step`
- `pm_auto_step`
- `vacuum_auto_step`
- `update_auto_step`

特点：

- 优点：硬件动作顺序表达直接，容易落地。
- 缺点：语义依赖步号记忆，后期维护成本高。

### 4.2 EFEM 状态机

- `100~159`：LLA 上料
- `200~257`：LLA 下料
- `1000~1599`：LLB 上料
- `2000~2507`：LLB 下料

典型上料链路：

1. 打开 LP 盒门并获取 MAP。
2. 把待上料项转入 `sequence_lp*_get_wafer`。
3. EWTR 从 LP 取片。
4. LL 移到目标槽位并打开 Cassette Door。
5. EWTR 把片放入 LL。
6. 同时推进到：
   - `sequence_loadlock*_transfer_wafer`
   - `sequence_lp*_put_wafer`

### 4.3 LLA / LLB 状态机

两套状态机高度对称，主干结构一致：

- `10 / 20 / 100 / 300`：判断上料需求，必要时先破真空，进入可接片状态。
- `301 / 302 / 350`：置位 `tool*_allow_get_wafer`，呼叫 EFEM 上料。
- `400 / 410 / 500 / 510`：关门、抽真空。
- `800 / 810 / 900 / 950`：mapping 后决定当前是“允许 WTR 取片”还是“等待回片”。
- `1000~1070`：允许 WTR 从 LL 取片，取走后把任务移入 `sequence_loadlock*_put_wafer`。
- `2000~2070`：允许 WTR 向 LL 回片，放回后删除已完成项。
- `5000~5025`：破真空并置位 `tool*_allow_put_wafer`，呼叫 EFEM 回片。

### 4.4 Robot / WTR 状态机

这是最核心的状态机。

一级分流：

- `10`：空闲等待
- `100~120`：必要时抽 TM 真空
- `1000 / 1010`：决定当前要处理哪类搬运任务

二级分流：

- `2000+`：从 LLA 取片
- `10000+`：从 LLB 取片
- `4000+`：与 PM 交互
- `6000+`：向 LLB 回片
- `14000+`：向 LLA 回片
- `20000`：清理已完成 Robot 主队列头项

PM 段不是单纯“放片”或“取片”，而是根据：

- PM 是否已有片
- A/B 手臂是否带片

来决定：

- 直接放 PM
- 先取 PM 旧片再放入新片
- 只从 PM 取回后回 LL

这说明旧系统已经在 WTR 线程内部实现了“交换料策略”的雏形。

### 4.5 PM 状态机

- `10`：等待
- `100 / 200`：去上下料位并允许 WTR 交互
- `2000 / 2100`：去工艺位并执行工艺
- `2200`：工艺完成后返回上下料位
- `2110`：工艺失败后的人工确认重试

### 4.6 Update 状态机

- `10`：轮询 LP1 / LP2 当前一轮是否完成
- `1030`：处理 LP1 一轮完成
- `1040`：处理 LP2 一轮完成

未达到 cycle 次数时：

- 把 `sequence_robot_transfer_wafer_lp1/lp2` 重新拼回 Robot 母队列
- 把 `sequence_lp*_transfer_wafer_copy` 和 `sequence_tolk*_wafer_copy` 恢复到运行队列

达到上限时：

- 标记 `cycleFinished_lla` / `cycleFinished_llb`
- 清掉对应 copy 队列，停止该侧继续循环

## 5. 任务流转过程

### 5.1 队列视角

单片晶圆的典型流转是：

```text
配置输入
-> sequence_tolk*_wafer / sequence_lp*_transfer_wafer / sequence_robot_transfer_wafer

EFEM上料阶段
-> sequence_lp*_get_wafer

LL待送PM阶段
-> sequence_loadlock*_transfer_wafer

PM回收登记阶段
-> sequence_robot_get_wafer

LL待回片阶段
-> sequence_loadlock*_put_wafer

EFEM回片阶段
-> sequence_lp*_put_wafer
```

### 5.2 典型单片主流程

1. `setTransferSequence()` 生成本轮 LP、LL、Robot 初始任务。
2. LL 线程进入可接片状态，置位 `tool*_allow_get_wafer`。
3. EFEM 把晶圆从 LP 放入目标 LL。
4. LL 关门、抽真空、mapping、移槽，置位 `loadlock*_allow_get_wafer`。
5. WTR 从 LL 取片并送入 PM。
6. WTR 放入 PM 后，把任务登记到 `sequence_robot_get_wafer`，表示后续还要回收。
7. PM 线程执行工艺，完成后回上下料位，重新允许交互。
8. WTR 从 PM 取回并放回目标 LL。
9. LL 破真空，置位 `tool*_allow_put_wafer`。
10. EFEM 从 LL 取片并回放 LP，形成完整闭环。

## 6. 线程模型

### 6.1 启动线程

旧版启动时会拉起：

- Vacuum 线程
- Robot 线程
- LLA 线程
- LLB 线程
- PM 线程
- Update 线程
- EFEM 线程

这是一种典型的“按设备角色拆线程”模型。

### 6.2 协同机制

线程之间主要靠三类机制协作：

- 队列：表达“还有哪些片要处理”
- 布尔标志：表达“现在谁可以执行下一步”
- 少量互斥锁：保护共享 vector

典型标志包括：

- `tool1_allow_get_wafer`
- `tool2_allow_put_wafer`
- `loadlock1_allow_get_wafer`
- `loadlock2_allow_put_wafer`
- `pm_allow_get_put_wafer`
- `pm_allow_goto_craft`

本质分层：

- 上层并行：EFEM、LLA、LLB、PM、Vacuum、Update
- 下层串行：WTR / Robot

## 7. 为什么 WTR 放在一个线程中是合理的

### 7.1 WTR 是真空侧唯一共享硬件

业务上虽然可能出现：

- LLA 同时准备好待送 PM 的片
- LLB 也准备好待送 PM 的片
- PM 内的片还可能等待回收

但物理上只有一个 WTR 控制器和一套机械运动上下文。任何时刻它都只能安全执行一条真空搬运动作链。

### 7.2 如果分散到多个线程直接控制 WTR，会有什么风险

1. 命令竞争
   - 多线程同时下发 `get/put`，容易造成控制器状态错乱。

2. 手臂状态错乱
   - A/B 手臂是否带片、当前目标槽位、下一步准备取还是放，都依赖上一条命令的结果。

3. 真空门阀互锁失效
   - WTR 执行动作前必须确认 TM 真空、LL 真空、门阀状态都满足，分散判断容易在边缘时序出错。

4. 全局优先级失控
   - PM 已有片、手臂也带片时，需要全局决定是先取、先放还是做交换料。

### 7.3 单线程 WTR 的价值

把所有真空侧搬运集中在 `startRobotAction()`，本质上就是一个“硬件命令串行化器”。

好处是：

- LL / PM / EFEM 线程负责提出需求
- WTR 线程负责统一串行执行
- 业务准备可以并行
- 危险动作必须串行
- 真空互锁、门阀互锁、交换片逻辑可以在一个状态机中闭环处理

所以这不是“性能浪费”，而是标准的“准备并行、执行串行”分层设计。

## 8. 优点分析

### 8.1 优点

- 贴合设备现实：状态机与硬件动作顺序高度一致，现场工程师容易对 step 定位。
- WTR 单线程设计正确：把最危险的共享资源从并发访问中隔离出来。
- LLA / LLB 可并行准备：虽然 WTR 串行，但 LL 和 PM 可以并行做准备动作。
- 队列分层清晰：可以沿着不同 `sequence_*` 容器追踪晶圆处于哪个阶段。
- Update 线程收口合理：把多轮 cycle 重装填逻辑从主搬运流程中分离。

## 9. 缺点分析

### 9.1 缺点

- 状态机步号可读性差：大量 `case 100/1010/14120/5025` 语义不直观。
- 布尔标志过多：置位和清位分散，依赖阅读经验判断。
- 队列一致性依赖流程假设：很多地方默认队首就是当前正确任务。
- 线程生命周期管理较弱：启动使用 `std::thread(...).detach()`，暂停和复位主要依赖 `running` 标志。
- 业务规则散落：优先级与切换规则分散在线程 case 中，不利于抽象成统一调度策略。
- 扩展性有限：当前结构更适合“2 LL + 1 PM + 1 WTR”的旧系统格局。

## 10. 设计意图总结

从旧版实现反推，它的核心设计意图可以总结为四点：

1. LL 是边界调度器
   - 既管理大气侧接片，也管理真空侧出片和回片。

2. WTR 是真空核心执行器
   - 它不是普通设备，而是 LL 与 PM 的唯一共享桥梁。

3. 调度策略强调“准备并行、执行串行”
   - EFEM、LL、PM 并行推进；WTR 串行执行危险搬运动作。

4. 旧系统优先保证可运行和可控
   - 先把联锁和时序跑通，再谈更高层抽象。

## 11. 最终结论

这份旧版 `slot_transfer_cycle_vtm_widget.cpp` 虽然代码风格偏厚重、状态机分支很多，但它的核心调度思想是清楚且合理的：

- EFEM、LL、PM、Vacuum、Update 分线程并行推进各自职责。
- WTR 作为 LL 与 PM 之间交互的核心硬件，被集中收敛到 Robot 线程中串行执行。
- LL 负责边界条件和节拍闸门，WTR 负责真空侧搬运落地，PM 负责工艺处理，Update 负责轮次收口。

因此，从设计意图角度看：

> 旧系统真正想解决的问题，不是“最优算法”，而是“在多设备、多边界、多互锁条件下，稳定且安全地完成 LP -> LL -> PM -> LL -> LP 的闭环流转”。

在这个目标下，WTR 放在一个线程中，是整套旧版调度系统最关键、也最合理的设计决策之一。
