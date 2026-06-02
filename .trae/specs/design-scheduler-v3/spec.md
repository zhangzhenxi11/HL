﻿﻿﻿﻿﻿# SchedulerV3 Spec

## Why

当前 `SchedulerV2` 已经引入 `TaskManager + UnifiedWaferTask + TM集中调度` 的方向，但仍存在架构混杂、状态机约束不足、线程安全薄弱、异常恢复不完整、扩展多策略能力不足等问题。需要在不影响现有业务逻辑和现有文件的前提下，新增一套 `SchedulerV3` 调度架构，为后续替换和并行验证提供清晰边界。

## What Changes

- 新增 `SchedulerV3` 调度架构，不修改现有 `SchedulerV2` 逻辑与现有主调度文件行为。
- 新增“规划层 + 资源仲裁层 + 执行编排层 + 恢复层”的分层调度模型。
- 新增“FSM 命名状态驱动 + 事件队列通信 + Exec 资源管理”的三段式规格细化。
- 支持路径选择可自由支配 `LoadLock` 腔与 `LoadPort` 上下料路径。
- 支持优先级调度，允许按任务类型、设备状态、回片压力、PM空闲度进行调度排序。
- 支持异常恢复，包含暂停、局部失败、任务重试、人工确认、重建执行上下文。
- 支持可扩展配方模式，首版支持：
  - 策略1：`VTM` 抓 1 片即进 PM
  - 策略2：`VTM` 抓 2 片再进 PM
- 支持面向目标节拍的模式扩展：`LLA` 进、`LLB` 出、`LLA` 长时间满片等待、最大限度保证 PM 有片运行。
- 明确“EFEM 两个机械臂非同时使用配置”为后续版本能力，不纳入本次 V3 首版落地范围。

## Impact

- Affected specs:
  - 调度架构分层
  - 任务状态机建模
  - 路径选择策略
  - 优先级调度
  - 异常恢复
  - 配方模式扩展
- Affected code:
  - `device/src/slot_transfer_cycle_vtm_widget.cpp`
  - `device/src/TaskManager.cpp`
  - `device/include/TaskManager.h`
  - 新增 `SchedulerV3` 相关文件

## ADDED Requirements

### Requirement: SchedulerV3 独立架构
系统 SHALL 以新增文件的方式提供 `SchedulerV3`，不得直接改写当前 `SchedulerV2` 的主调度实现。

#### Scenario: 与现有调度并存
- **WHEN** 引入 `SchedulerV3`
- **THEN** 当前 `SchedulerV2` 代码路径保持不变
- **AND** `SchedulerV3` 以独立类、独立状态模型、独立调度入口存在
- **AND** 允许后续通过配置或入口切换进行灰度验证

### Requirement: SchedulerV3 分层架构
系统 SHALL 将调度拆分为“任务规划层、优先级仲裁层、资源执行层、恢复控制层”四层，避免单一大类同时承担路径规划、状态迁移、硬件执行和异常恢复职责。

#### Scenario: 架构职责分离
- **WHEN** 一个晶圆任务从 LP 进入 PM 再回 LP
- **THEN** 路径选择由规划层负责
- **AND** 执行顺序由优先级仲裁层决定
- **AND** WTR/EFEM/LL/PM 硬件动作由资源执行层负责编排
- **AND** 暂停、失败恢复、重试、重建由恢复控制层负责

### Requirement: 命名状态 FSM 架构
系统 SHALL 使用“命名状态 FSM”替代 `switch-case + step 数字` 风格，至少定义以下 6 个 FSM：

- `Task FSM`
- `WTR FSM`
- `LL FSM`
- `PM FSM`
- `Recovery FSM`
- `EFEM Robot FSM`

#### Scenario: 状态名替代数字 step
- **WHEN** 开发 `SchedulerV3`
- **THEN** 所有状态必须使用具有业务含义的命名状态
- **AND** 禁止使用 `step=0/1/2` 作为状态语义载体

#### Scenario: 状态具备 entry/exit 动作
- **WHEN** 任一 FSM 进入或离开某状态
- **THEN** 必须定义对应的 `entry` 动作和 `exit` 动作
- **AND** 必须明确进入条件、可转出状态和超时策略

#### Scenario: 事件驱动状态转移
- **WHEN** FSM 执行状态转移
- **THEN** 转移条件必须是业务事件或硬件反馈事件
- **AND** 不得以“轮询 flag + Sleep”作为状态转移语义

#### Scenario: 全状态异常路径
- **WHEN** 任一 FSM 处于非终态
- **THEN** 该状态都必须存在进入 `ERROR / FAILED / RECOVERING` 类状态的异常路径

### Requirement: SchedulerV3 架构图
系统 SHALL 提供 `SchedulerV3` 的架构图作为规格基础，清晰表达分层、核心对象、任务流、资源流与恢复路径。

#### Scenario: 输出架构图
- **WHEN** 设计 `SchedulerV3` 规格
- **THEN** 规格文档中包含一张文本架构图
- **AND** 架构图至少包含 LP、EFEM、LLA、LLB、WTR、PM、Planner、Dispatcher、Recovery Manager

### Requirement: 路径选择自由支配 LoadLock 和 LoadPort
系统 SHALL 支持调度器在任务规划阶段动态选择目标 `LoadLock` 与 `LoadPort` 路径，而不是将路径固定写死在状态机分支中。

#### Scenario: 动态选择上料路径
- **WHEN** 某片晶圆进入待调度状态
- **THEN** 调度器可以根据当前 LL 可用性、目标模式、优先级和堵塞情况选择进入 `LLA` 或 `LLB`
- **AND** 调度器可以选择对应的 LP 上料路径

#### Scenario: 动态选择回片路径
- **WHEN** PM 工艺完成后需要返回
- **THEN** 调度器可以根据当前返片压力和上下料模式选择合适的 `LoadLock` 和 `LoadPort` 回片路径

### Requirement: Task FSM
系统 SHALL 定义一个覆盖任务完整生命周期的 `Task FSM`，负责表达任务从创建、规划、等待资源、执行、恢复到完成/中止的全过程。

#### Scenario: 任务生命周期完整闭环
- **WHEN** 新任务被提交
- **THEN** 任务进入 `CREATED` 或 `PLANNING`
- **AND** 后续可转入 `READY_TO_DISPATCH / WAITING_FOR_RESOURCES / EXECUTING / RECOVERING / DONE / ABORTED`
- **AND** 任一失败路径必须能进入 `FAILED` 或 `MANUAL_CONFIRM`

### Requirement: WTR FSM
系统 SHALL 定义一个覆盖 `Pick-Place` 全流程的 `WTR FSM`，至少包括“移动到源、抓取、移动到目标、放置、错误、超时”等语义状态。

#### Scenario: WTR 动作完整受控
- **WHEN** Dispatcher 下发一次 `WTR` 抓放任务
- **THEN** `WTR FSM` 依次经历“空闲、去源、到源稳定、抓取、去目标、到目标稳定、放置、完成”
- **AND** 所有移动与抓放状态都带超时

### Requirement: LL FSM
系统 SHALL 定义 `LL FSM`，覆盖接片、关门、冷却、泵下、通气、准备取货、异常等全过程。

#### Scenario: LoadLock 边界流程完整表达
- **WHEN** 某 `LL` 被规划为接片或回片路径
- **THEN** `LL FSM` 明确表达门阀、冷却、真空、准备取货等阶段
- **AND** 这些阶段不得隐藏在零散布尔标志中

### Requirement: PM FSM
系统 SHALL 定义 `PM FSM`，覆盖接收晶圆、准备加工、加工中、加工完成可取货、异常等语义状态。

#### Scenario: PM 持续运行目标
- **WHEN** PM 有可执行任务
- **THEN** 调度器优先保持 PM 进入 `PROCESSING`
- **AND** 在状态机层显式区分“可接片”和“可取片”

### Requirement: Recovery FSM
系统 SHALL 定义 `Recovery FSM`，显式表达“发现错误、冻结、分析、请求人工确认、局部恢复、全局中止、恢复失败”等过程。

#### Scenario: 进入恢复流程
- **WHEN** 任一 Exec 上报失败
- **THEN** `Recovery FSM` 接管该任务
- **AND** 根据失败类型决定是局部恢复、重试、人工确认还是中止

### Requirement: EFEM Robot FSM
系统 SHALL 定义 `EFEM Robot FSM`，覆盖大气侧机械臂的取片、放片、等待、超时和异常处理。

#### Scenario: EFEM 执行 LP 与 LL 之间搬运
- **WHEN** Dispatcher 下发 `EFEM` 搬运任务
- **THEN** `EFEM Robot FSM` 以显式状态驱动执行，不得再用 step 数字表达

### Requirement: 优先级调度
系统 SHALL 支持统一优先级调度，优先级评估至少覆盖“已完成工艺待回收、PM 即将空闲、LL 堵塞风险、待上料任务、模式策略偏好”。

#### Scenario: 回片优先于新上料
- **WHEN** 某 `LoadLock` 下层或回片缓冲存在已完成工艺的晶圆
- **THEN** 该回收任务优先级高于新的 LL 上料任务

#### Scenario: 保证 PM 尽量不断片
- **WHEN** PM 即将空闲且存在可送片任务
- **THEN** 调度器优先安排满足互锁条件的送片任务

#### Scenario: LLA 进 LLB 出目标导向
- **WHEN** 当前模式目标为 `LLA进 / LLB出 / LLA尽量常满`
- **THEN** 优先级仲裁倾向优先补充 `LLA`
- **AND** `LLB` 更倾向承担回片与出料任务
- **AND** 若该倾向与安全互锁冲突，以安全互锁优先

### Requirement: 事件驱动层间通信
系统 SHALL 以“事件队列 + 异步消息”替代全局 flag、轮询与跨线程直接协作。

#### Scenario: Planner 到 Dispatcher
- **WHEN** Planner 完成路径与策略规划
- **THEN** 通过 `Planner -> Dispatcher` 事件通道发送任务规划结果

#### Scenario: Dispatcher 到 Exec
- **WHEN** Dispatcher 决定执行下一步
- **THEN** 通过独立事件通道向 `WTR / LL / PM / EFEM Exec` 下发动作事件

#### Scenario: Exec 到 Dispatcher
- **WHEN** 任一 Exec 动作成功、失败或超时
- **THEN** 通过统一结果事件回传 Dispatcher

#### Scenario: Exec 到 Recovery
- **WHEN** 任一 Exec 发现错误或可恢复失败
- **THEN** 事件直接上报 Recovery Manager

#### Scenario: Recovery 到 Dispatcher
- **WHEN** Recovery Manager 做出恢复决策
- **THEN** 通过事件向 Dispatcher 下发继续、重试、中止、人工确认请求

### Requirement: 事件枚举与事件载荷
系统 SHALL 为关键通道定义显式事件枚举、载荷结构、优先级、队列大小、超时与重发策略。

#### Scenario: 关键事件建模
- **WHEN** 定义 `SchedulerV3` 通信协议
- **THEN** 至少覆盖以下事件族：
  - `Planner -> Dispatcher`
  - `Dispatcher -> WTR Exec`
  - `Dispatcher -> LL Exec`
  - `Dispatcher -> PM Exec`
  - `Dispatcher -> EFEM Exec`
  - `Exec -> Dispatcher`
  - `Exec -> Recovery`
  - `Recovery -> Dispatcher`

#### Scenario: 每个事件具备统一元信息
- **WHEN** 定义任一事件
- **THEN** 事件至少包含 `event_id / type / source_module / target_module / priority / timestamp / deadline / payload`

### Requirement: 事件队列与异步约束
系统 SHALL 为各模块定义线程安全事件队列，并约束全部跨模块交互以异步方式完成。

#### Scenario: 禁止同步跨模块等待
- **WHEN** 某模块需要另一个模块完成动作
- **THEN** 通过事件请求 + 响应事件完成协作
- **AND** 不允许出现模块 A 同步等待模块 B，同时模块 B 同步等待模块 A

#### Scenario: 队列溢出策略
- **WHEN** 事件队列达到容量上限
- **THEN** 系统必须有明确策略处理
- **AND** 首选“告警 + 丢弃最低优先级事件 + 记录日志”
- **AND** 不得静默丢失关键事件

### Requirement: 统一超时与重试策略
系统 SHALL 为事件和硬件动作定义统一超时与重试策略表。

#### Scenario: 硬件动作超时
- **WHEN** 动作属于 `Pick / Place`
- **THEN** 首次超时按 15s 处理
- **AND** 默认不自动重试

#### Scenario: 泵下和互锁超时
- **WHEN** 动作属于 `Pump Down / Vacuum Interlock / Door Operation / Planning`
- **THEN** 依据规格中的统一策略表执行超时和有限重试

### Requirement: Exec 层 FSM 实现约定
系统 SHALL 规定所有 Exec 模块采用统一的 FSM 实现框架。

#### Scenario: Exec 模块统一结构
- **WHEN** 开发 `WTR_Exec / LL_Exec / PM_Exec / EFEM_Exec`
- **THEN** 每个 Exec 至少包含：
  - 当前状态
  - `on_event()` 事件入口
  - `tick()` 超时与硬件检查入口
  - `transition_to()` 状态转移接口
  - `on_enter_state()` 与 `on_exit_state()` 动作

#### Scenario: 禁止 step 风格
- **WHEN** 开发任一 Exec
- **THEN** 禁止使用 `step++` 递增状态
- **AND** 禁止在状态分支中继续扩展 `switch(step)` 风格

### Requirement: 资源管理协议
系统 SHALL 引入统一 `ResourceManager` 以管理共享资源获取、释放、超时与冲突检测。

#### Scenario: 单资源显式申请
- **WHEN** 任一 Exec 需要使用共享资源
- **THEN** 必须先通过 `ResourceManager` 请求
- **AND** 动作完成或失败后显式释放

#### Scenario: 禁止嵌套持有多个共享资源
- **WHEN** 任一 Exec 正在持有一个共享资源
- **THEN** 不允许再同步等待获取第二个共享资源
- **AND** 由 Dispatcher 通过排序和拆分动作避免资源嵌套占有

#### Scenario: 冲突检测
- **WHEN** Dispatcher 准备下发动作
- **THEN** 先检查资源冲突与潜在循环等待
- **AND** 检测到冲突时不得直接下发

### Requirement: 检查点与快照恢复
系统 SHALL 为各 Exec 提供检查点保存和恢复入口。

#### Scenario: 安全检查点保存
- **WHEN** Exec 进入安全可恢复阶段
- **THEN** 保存 `checkpoint`
- **AND** 快照至少包含任务 ID、当前状态、资源占有、硬件位置、是否允许恢复

#### Scenario: 基于检查点恢复
- **WHEN** Recovery Manager 判定允许恢复
- **THEN** 可从最近安全检查点恢复执行
- **AND** 不得直接从不安全状态强行恢复

### Requirement: 模式化调度策略
系统 SHALL 支持可扩展的模式策略，而不是把策略直接编码为巨型 step 分支。

#### Scenario: 策略1 一片即进 PM
- **WHEN** 当前模式为“VTM抓1片就进PM”
- **THEN** 一旦存在满足条件的单片任务，调度器允许 WTR 立即送入 PM

#### Scenario: 策略2 两片再进 PM
- **WHEN** 当前模式为“VTM抓2片再进PM”
- **THEN** 调度器按照交互片策略组织任务
- **AND** 允许 LL 上下料路径与当前旧版策略不同

#### Scenario: 模式扩展目标
- **WHEN** 当前模式目标为“最大限度 PM 有片跑，LLA 进，LLB 出，LLA 永远满着等候”
- **THEN** 调度器在优先级与路径选择时倾向维持 `LLA` 的连续补片能力
- **AND** `LLB` 倾向承担回片与出料压力

### Requirement: 异常恢复
系统 SHALL 提供显式异常恢复模型，覆盖“暂停、继续、任务失败、硬件失败、人工确认、任务重建、局部重试、全局中止”。

#### Scenario: 单步硬件动作失败
- **WHEN** 任一硬件动作失败
- **THEN** 调度器进入显式失败状态
- **AND** 冻结该任务的继续推进
- **AND** 记录失败阶段、资源占用快照和恢复建议

#### Scenario: 人工确认后局部恢复
- **WHEN** 人工确认设备状态已恢复
- **THEN** 调度器可从最近的安全检查点恢复
- **AND** 不要求整轮任务全部重建

#### Scenario: 全局中止后重建
- **WHEN** 操作员执行中止
- **THEN** 调度器停止继续下发新动作
- **AND** 将当前任务转入可恢复或待人工处理状态
- **AND** 允许后续基于快照重新构建执行上下文

#### Scenario: Recovery Manager 可中断执行
- **WHEN** Recovery Manager 判定当前动作需立即终止
- **THEN** 它必须能向 Dispatcher 和相关 Exec 发出中止或冻结事件
- **AND** 任一执行模块不得忽略该恢复决策

### Requirement: 资源竞争集中仲裁
系统 SHALL 将共享资源竞争收敛到统一仲裁层，不允许 EFEM、LL、PM 线程直接抢占共享硬件。

#### Scenario: WTR 单点仲裁
- **WHEN** 多个任务同时请求 WTR
- **THEN** 所有请求进入统一仲裁队列
- **AND** 由同一调度单元决定执行顺序

#### Scenario: 真空与门阀资源仲裁
- **WHEN** 多个资源操作同时依赖真空、门阀、槽位条件
- **THEN** 调度器通过显式资源状态判断决定是否允许执行
- **AND** 不依赖散落的跨线程共享布尔标志直接协商

### Requirement: EFEM 单臂配置为后续版本
系统 SHALL 将“EFEM 两个机械臂不同时使用配置”标记为后续版本能力，不纳入 `SchedulerV3` 首版实现范围，但必须在架构上预留扩展点。

#### Scenario: 首版范围控制
- **WHEN** 设计 `SchedulerV3` 首版
- **THEN** 首版不要求实现 EFEM 单臂配置切换
- **AND** 架构中保留 EFEM 臂策略接口或配置扩展点

## MODIFIED Requirements

### Requirement: 调度中心角色
现有调度中心职责需要从“主文件中的大状态机 + 分散标志协作”升级为“可组合的规划与执行调度架构”，但此升级必须通过新增 `SchedulerV3` 文件实现，不得直接破坏当前业务运行路径。

#### Scenario: 新旧版本并行
- **WHEN** `SchedulerV3` 进入开发和联调阶段
- **THEN** 现有调度系统仍可独立运行
- **AND** `SchedulerV3` 作为新增路径逐步替换或验证

## REMOVED Requirements

### Requirement: 首版直接支持 EFEM 单臂配置
**Reason**: 用户明确要求该能力在后续版本增加，本次 `SchedulerV3` 首版聚焦架构、路径、优先级、恢复和策略模式。
**Migration**: 在 `SchedulerV3` 中预留 EFEM 臂策略扩展点，后续版本在不破坏主架构的前提下增加该能力。

## SchedulerV3 架构图

```text
                               +---------------------------+
                               |   SchedulerV3 Facade      |
                               |  启动/暂停/继续/中止入口   |
                               +-------------+-------------+
                                             |
                    +------------------------+------------------------+
                    |                                                 |
                    v                                                 v
        +--------------------------+                     +--------------------------+
        |   Task Snapshot Store    |                     |     Recovery Manager     |
        |  任务/资源/阶段快照中心  |<------------------->|  暂停/失败/恢复/重建     |
        +-------------+------------+                     +-------------+------------+
                      |                                                  |
                      v                                                  |
        +--------------------------+                                     |
        |      Task Planner        |                                     |
        |  路径规划/模式选择/拆单   |                                     |
        |  选择 LP / LLA / LLB / PM |                                     |
        +-------------+------------+                                     |
                      |                                                  |
                      v                                                  |
        +--------------------------+                                     |
        |   Priority Dispatcher    |-------------------------------------+
        |  优先级仲裁/饥饿避免/抢占 |
        +------+------+------+-----+
               |      |      |
               |      |      |
               v      v      v
      +-----------+ +-----------+ +----------------+
      | EFEM Exec | |  LL Exec  | |   WTR/TM Exec  |
      | LP<->LL   | | LLA / LLB | | 共享真空搬运核 |
      +-----+-----+ +-----+-----+ +--------+-------+
            |             |                 |
            |             |                 v
            |             |         +---------------+
            |             +-------->|   PM Exec     |
            |                       | PM1..PM4      |
            |                       +-------+-------+
            |                               |
            +-------------------------------+
                            任务闭环


  任务流：
  LP任务 -> Planner -> Dispatcher -> EFEM/LL/WTR/PM 执行 -> Snapshot -> Recovery

  资源流：
  WTR / LL / PM / EFEM / 真空 / 门阀状态 -> Snapshot Store -> Dispatcher 决策

  模式策略：
  Strategy#1: 抓1片即进PM
  Strategy#2: 抓2片再进PM
  Future: EFEM单臂配置 / 更多配方模式

  目标节拍：
  优先保持 PM 不断片
  倾向 LLA 进 / LLB 出
  让 LLA 长时间保持可供片状态
```

## FSM 规格细化

### FSM 总览

| FSM | 作用 | 首版目标 |
| --- | --- | --- |
| Task FSM | 表达任务全生命周期 | 替代 `TaskType/Status` 任意跳转 |
| WTR FSM | 表达 VTM Pick-Place 闭环 | 替代 WTR step 编号 |
| LL FSM | 表达 LLA/LLB 边界流程 | 显式门阀/冷却/真空状态 |
| PM FSM | 表达 PM 接片、加工、出片 | 保持 PM 不断片 |
| Recovery FSM | 表达错误接管与恢复 | 支持局部恢复与人工确认 |
| EFEM Robot FSM | 表达大气侧机械臂搬运 | 支持 LP/LL 路径调度 |

### FSM 统一约束

- 所有 FSM 必须使用命名状态。
- 每个状态必须定义：
  - 进入条件
  - 可转出状态
  - entry 动作
  - exit 动作
  - 超时目标
- 所有硬件交互必须带超时。
- 任何非终态必须能进入错误或恢复路径。
- 状态转移条件必须是事件，不是“轮询 flag”。

## 事件通信规格细化

### 模块通信拓扑

```text
Planner -> Dispatcher -> WTR_Exec
                      -> LL_Exec
                      -> PM_Exec
                      -> EFEM_Exec

WTR_Exec / LL_Exec / PM_Exec / EFEM_Exec
    -> Dispatcher
    -> Recovery Manager

Recovery Manager -> Dispatcher

所有跨模块通信通过中央事件分发器与线程安全队列完成
```

### 事件通道

- 通道1：`Planner -> Dispatcher`
- 通道2：`Dispatcher -> WTR Exec`
- 通道3：`Dispatcher -> LL Exec`
- 通道4：`Dispatcher -> PM Exec`
- 通道5：`Dispatcher -> EFEM Exec`
- 通道6：`Exec -> Dispatcher`
- 通道7：`Exec -> Recovery Manager`
- 通道8：`Recovery Manager -> Dispatcher`

### 关键事件族

- `EVENT_TASK_PLANNED`
- `EVENT_TASK_CANCEL`
- `EVENT_WTR_EXECUTE_PICK_PLACE`
- `EVENT_WTR_RELEASE_RESOURCE`
- `EVENT_WTR_EMERGENCY_STOP`
- `EVENT_LL_RECEIVE_WAFER`
- `EVENT_LL_START_PUMP_DOWN`
- `EVENT_LL_START_VENT`
- `EVENT_LL_READY_FOR_PICKUP`
- `EVENT_LL_RELEASE_RESOURCE`
- `EVENT_PM_RECEIVE_WAFER`
- `EVENT_PM_READY_FOR_PICKUP`
- `EVENT_EFEM_ROBOT_PICK`
- `EVENT_EFEM_ROBOT_PLACE`
- `EVENT_ACTION_SUCCESS`
- `EVENT_ACTION_FAILED`
- `EVENT_ACTION_TIMEOUT`
- `EVENT_RESOURCE_RELEASED`
- `EVENT_HARDWARE_ERROR`
- `EVENT_RECOVERABLE_FAILURE`
- `EVENT_UNRECOVERABLE_FAILURE`
- `EVENT_RESUME_EXECUTION`
- `EVENT_RETRY_TASK`
- `EVENT_ABORT_TASK`
- `EVENT_REQUEST_MANUAL_CONFIRM`

### 事件元信息约束

- 每个事件必须具备：
  - `event_id`
  - `event_type`
  - `source_module_id`
  - `target_module_id`
  - `priority`
  - `timestamp_ms`
  - `deadline_ms`
  - `payload`
  - `payload_size`
  - `requires_response`

### 事件处理约束

- 模块之间默认异步通信。
- 所有事件处理必须具备超时。
- 队列满时不得静默丢弃高优先级事件。
- `Recovery Manager` 事件优先级至少为 `HIGH`，紧急中止为 `CRITICAL`。

## Exec 与资源管理规格细化

### Exec 层统一接口约束

每个 Exec 模块都必须具备：

- `on_event(const Event&)`
- `tick(current_time_ms)`
- `transition_to(next_state)`
- `on_enter_state(state)`
- `on_exit_state(state)`
- `check_timeout()`
- `save_checkpoint()`

### 资源列表基线

| 资源名 | 容量 | 所属仲裁 |
| --- | --- | --- |
| `WTR` | 1 | Dispatcher |
| `LLA` | 1 | Dispatcher |
| `LLB` | 1 | Dispatcher |
| `PM1..PM4` | 各 1 | Dispatcher |
| `EFEM_Robot` | 1 | Dispatcher |
| `Vacuum_LLA` | 1 | Dispatcher |
| `Vacuum_LLB` | 1 | Dispatcher |
| `Vacuum_TM` | 1 | Dispatcher |

### 资源协议基线

- 显式申请
- 显式释放
- 超时失败
- 禁止嵌套持有多个共享资源
- Dispatcher 负责冲突检测

### 检查点基线

- 保存点应优先选择安全状态：
  - `IDLE`
  - 到位稳定完成
  - 抓取完成后
  - 放置完成后
- 不允许从高风险中间动作直接盲恢复

## 统一超时与重试策略表

| 事件类型 | 首次超时 | 是否可重试 | 重试次数 | 重试间隔 | 最终处理 |
| --- | --- | --- | --- | --- | --- |
| 硬件动作 `Pick/Place` | 15s | 否 | 0 | - | 失败并进入恢复 |
| 泵下操作 | 30s | 是 | 3 | 5s | 多次失败后进入恢复 |
| 真空互锁 | 10s | 是 | 2 | 2s | 失败并进入恢复 |
| 门阀操作 | 5s | 是 | 2 | 1s | 失败并进入恢复 |
| 晶圆驻留 | 动态 | 否 | 0 | - | 超期进入失败 |
| 任务规划 | 5s | 是 | 1 | 2s | 失败后进入恢复/人工确认 |

## 禁用项

- 禁止 `step++` 递增状态
- 禁止 `switch(step)` 大状态机继续扩张
- 禁止全局 flag 轮询替代事件
- 禁止跨模块同步阻塞等待
- 禁止共享资源无超时占有

## 首版范围进一步说明

### 首版必须覆盖

- SchedulerV3 独立文件化
- 6 个 FSM 的命名状态设计
- 事件队列与异步通信规格
- Exec 层统一接口
- 资源管理协议
- 检查点与恢复路径
- 路径选择
- 优先级调度
- 两种策略模式

### 首版暂不实现

- EFEM 单臂配置落地
- 更多配方模式
- 完整代码实现
- 对现有 `SchedulerV2` 的替换
