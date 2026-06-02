﻿﻿﻿﻿﻿# Tasks

- [ ] Task 1: 固化 SchedulerV3 总体架构与边界
  - [ ] SubTask 1.1: 明确 `SchedulerV3 Facade`、`Task Planner`、`Priority Dispatcher`、`Recovery Manager`、`Task Snapshot Store` 的职责边界
  - [ ] SubTask 1.2: 明确 `EFEM / LL / WTR / PM` 执行层与调度层之间的接口边界
  - [ ] SubTask 1.3: 明确新文件接入方式，确保不修改当前 `SchedulerV2` 主逻辑

- [ ] Task 2: 设计 SchedulerV3 任务模型与状态机
  - [ ] SubTask 2.1: 定义任务阶段、执行状态、失败状态、恢复状态和终态
  - [ ] SubTask 2.2: 定义合法状态迁移表，替代任意 `TaskType/Status` 跳转
  - [ ] SubTask 2.3: 定义任务快照内容，覆盖资源占用、路径选择、失败点和恢复点
  - [ ] SubTask 2.4: 细化 6 个命名状态 FSM：`Task / WTR / LL / PM / Recovery / EFEM Robot`
  - [ ] SubTask 2.5: 为每个 FSM 定义 entry、exit、超时、错误路径和终态
  - [ ] SubTask 2.6: 输出 FSM 对比基线，包括状态数、超时数、错误路径覆盖

- [ ] Task 3: 设计路径选择与优先级调度
  - [ ] SubTask 3.1: 定义 LP / LLA / LLB / PM 的路径选择规则
  - [ ] SubTask 3.2: 定义优先级维度，覆盖回片优先、PM不断片、堵塞避免和模式偏好
  - [ ] SubTask 3.3: 定义 `LLA进 / LLB出 / LLA尽量常满` 目标下的调度倾向

- [ ] Task 4: 设计模式化策略扩展
  - [ ] SubTask 4.1: 定义策略1“VTM抓1片就进PM”的规划规则
  - [ ] SubTask 4.2: 定义策略2“VTM抓2片再进PM”的规划规则
  - [ ] SubTask 4.3: 定义后续模式扩展点，避免再写回巨型 step 状态机

- [ ] Task 5: 设计异常恢复机制
  - [ ] SubTask 5.1: 定义暂停、继续、中止的行为边界
  - [ ] SubTask 5.2: 定义单步失败、人工确认、局部恢复、全局重建流程
  - [ ] SubTask 5.3: 定义恢复检查点和恢复前置条件

- [ ] Task 6: 设计事件驱动通信协议
  - [ ] SubTask 6.1: 定义事件通道：`Planner -> Dispatcher -> Exec -> Recovery`
  - [ ] SubTask 6.2: 定义事件枚举、事件载荷、事件优先级和队列容量
  - [ ] SubTask 6.3: 定义异步响应、成功/失败/超时事件和重发策略
  - [ ] SubTask 6.4: 定义队列溢出策略、中央事件分发器和死锁规避约束

- [ ] Task 7: 设计 Exec 层与资源管理协议
  - [ ] SubTask 7.1: 定义 `WTR_Exec / LL_Exec / PM_Exec / EFEM_Exec` 的统一接口约束
  - [ ] SubTask 7.2: 定义 `ResourceManager` 的申请、释放、超时和冲突检测协议
  - [ ] SubTask 7.3: 定义检查点保存、恢复入口和安全恢复状态
  - [ ] SubTask 7.4: 明确禁用项：`step++`、全局 flag 轮询、跨模块同步等待

- [ ] Task 8: 定义首版范围与后续版本边界
  - [ ] SubTask 8.1: 明确 EFEM 单臂配置不纳入 V3 首版
  - [ ] SubTask 8.2: 为 EFEM 单臂配置保留策略扩展点
  - [ ] SubTask 8.3: 明确首版优先交付“路径选择 + 优先级 + 恢复 + 模式策略 + 事件驱动 + FSM”

- [ ] Task 9: 验证规格完整性
  - [ ] SubTask 9.1: 检查规格是否满足“保留现有逻辑、不改当前文件、新增调度文件”的约束
  - [ ] SubTask 9.2: 检查规格是否覆盖 6 个 FSM、事件驱动通信、Exec/资源管理
  - [ ] SubTask 9.3: 检查规格是否覆盖路径选择、优先级、异常恢复、模式化策略和扩展点
  - [ ] SubTask 9.4: 检查规格是否给出清晰架构图、通信拓扑和实现边界

# Task Dependencies

- [Task 2: 设计 SchedulerV3 任务模型与状态机] depends on [Task 1: 固化 SchedulerV3 总体架构与边界]
- [Task 3: 设计路径选择与优先级调度] depends on [Task 1: 固化 SchedulerV3 总体架构与边界]
- [Task 4: 设计模式化策略扩展] depends on [Task 2: 设计 SchedulerV3 任务模型与状态机]
- [Task 5: 设计异常恢复机制] depends on [Task 2: 设计 SchedulerV3 任务模型与状态机]
- [Task 6: 设计事件驱动通信协议] depends on [Task 2: 设计 SchedulerV3 任务模型与状态机]
- [Task 7: 设计 Exec 层与资源管理协议] depends on [Task 2: 设计 SchedulerV3 任务模型与状态机]
- [Task 7: 设计 Exec 层与资源管理协议] depends on [Task 6: 设计事件驱动通信协议]
- [Task 8: 定义首版范围与后续版本边界] depends on [Task 1: 固化 SchedulerV3 总体架构与边界]
- [Task 9: 验证规格完整性] depends on [Task 1: 固化 SchedulerV3 总体架构与边界]
- [Task 9: 验证规格完整性] depends on [Task 2: 设计 SchedulerV3 任务模型与状态机]
- [Task 9: 验证规格完整性] depends on [Task 3: 设计路径选择与优先级调度]
- [Task 9: 验证规格完整性] depends on [Task 4: 设计模式化策略扩展]
- [Task 9: 验证规格完整性] depends on [Task 5: 设计异常恢复机制]
- [Task 9: 验证规格完整性] depends on [Task 6: 设计事件驱动通信协议]
- [Task 9: 验证规格完整性] depends on [Task 7: 设计 Exec 层与资源管理协议]
- [Task 9: 验证规格完整性] depends on [Task 8: 定义首版范围与后续版本边界]
