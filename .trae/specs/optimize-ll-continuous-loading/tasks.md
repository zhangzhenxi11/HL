# Tasks

- [x] 1. 梳理现有连续跑片阻塞链路
  - [x] 核对 slot_transfer_cycle_vtm_widget.cpp 中 LLA/LLB case 10 的上料互锁入口、isLoadingInterlock() 判定条件，以及 case 1030/1040 的循环重跑/状态重置时机
  - [x] 核对 TaskManager.cpp 中 isLoadingInPlace、CollectionPassedThroughLL() 与 EFEM_RETURN/COMPLETED -> UNKNOWN/UNKNOWN 相关状态依赖，明确哪类 task 会被提前丢状态

- [x] 2. 重新定义 LL 上料互锁规则
  - [x] 将互锁目标改为按目标 LL 槽位可用性 + 在制 task 的真实下料冲突判断是否允许继续上料
  - [x] 去除另一侧整组 wafer 全部回 LP 才解锁这一阻塞前提，保留真实冲突场景下的保护
  - [x] 加入优先级规则：LL 下层存在已完成工艺 wafer 时，优先下料回 LP，高于新的 LL 上料

- [x] 3. 重新定义循环完成与状态重置安全点
  - [x] 调整 case 1030/1040 或等效调度路径的进入条件，避免在同批次仍有 wafer 留在 VTM/LL/PM 路径内时提前触发重跑
  - [x] 保证已完成返回的 task 仅在不影响其它在制 task 后续 EFEM_RETURN 流转时，才从 EFEM_RETURN/COMPLETED 重置为 UNKNOWN/UNKNOWN
  - [x] 保证在“下层完成片优先回 LP”的过程中，不因已完成 task 被提前重置而打断另一片 wafer 的后续下料路径

- [ ] 4. 验证连续 25 片调度目标
  - [ ] 验证存在已完成回 LP 的 task 与仍在腔体内待下料 task 并存时，不再出现前者被重置、后者拿不到下料锁的状态机混乱
  - [ ] 验证 LL 下层一旦有完成片，系统优先下料到 LP，而不是继续抢先向 LL 上层上料
  - [ ] 验证只要 LL 下层无更高优先级回 LP 动作，且 LLA/LLB 上层有空槽并无真实冲突，EFEM 可持续上料，PM 不因整组等待而断料
  - [ ] 验证 WTR 尽量维持交互片节奏，优先形成“一只手拿待加工片、一只手拿加工完成片”的持续交换状态
  - [ ] 通过日志或诊断确认未引入新的错误状态迁移

- [x] 5. 修复 EFEM 双 LL 调度缺陷
  - [x] 修复 EFEM 处理 `LLA/LLB` 请求时未按目标 LL 过滤、导致 `LLB` 请求可能被 `LLA` 分支遮蔽的问题
  - [x] 修复按 LL 空槽分配连续上料任务时的索引/遍历错误，确保非连续空槽也能正确补料
  - [x] 修复 `0 < size <= originTaskSize` 这类 C++ 链式比较误用，保证 LLA/LLB 上料条件只在真实满足时生效

- [x] 6. 收紧整机结束与重跑安全点
  - [x] 在整机结束分支增加“无仍在途 task”保护，避免仍有 wafer 在 EFEM/LL/PM 路径内时提前结束流程
  - [x] 复核循环重跑入口与 `originTaskSize` 依赖，避免因结束条件偏宽导致 25 片连续跑片被误截断

- [ ] 7. 增加 LL 回片后的交互准备动作
  - [x] 调整 LLA/LLB `case 1051` 的判定与分流，识别“一手持工艺完成片、一手为空、LL 上料槽仍有待加工片”的场景，先优先完成回 LL
  - [x] 在 LLA/LLB 把工艺完成片放回 LL 成功后、`case 2065` 后续关门/破真空收尾前增加补取片准备，让空手立即从 LL 再取一片待加工 wafer，为下一次 PM 交互片预热
  - [x] 在 `executeTMTransfer()` 中增加对这次 LL 发起交互准备的配合逻辑，保证 TM 不重复下发或打断 LL 线程的最小改动入口
  - [ ] 结合日志验证“回 LL -> 立即补取一片 -> 等 PM 完成后继续交互片”的连续节拍，而不是等回到 `950` 后再重新上料

# Task Dependencies
- Task 2 依赖 Task 1
- Task 3 依赖 Task 1
- Task 5 依赖 Task 2 和 Task 3
- Task 6 依赖 Task 2、Task 3 和 Task 5
- Task 4 依赖 Task 2、Task 3、Task 5 和 Task 6
- Task 7 依赖 Task 2、Task 4、Task 5 和 Task 6
