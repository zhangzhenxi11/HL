# LL 上进下出连续调度与循环状态解耦 Spec

## Why
当前 LL 的上进下出调度中，`isLoadingInterlock()` 将 LLA/LLB 的上料许可绑定为另一侧经由 LL 的整组 wafer 全部完成下料回 LP 后才解锁，会把双片循环串行化。配合循环完成时将 EFEM_RETURN/COMPLETED 直接重置为 UNKNOWN/UNKNOWN 的逻辑，会在另一片 wafer 仍停留在 VTM/LL/PM 路径内、后续还需要 EFEM 下料锁时，制造状态丢失与锁竞争，最终导致任务状态机混乱并降低 25 片连续跑片效率。

## What Changes
- 将本次优化范围限定为 LL 的“上进下出”模式，不改变其它非该模式的调度语义
- 将 LLA/LLB 上料互锁的目标从等待整组料全部回 LP调整为在不破坏在制 wafer 下料路径的前提下，按 LL 实际可用槽位和未完成下料依赖决定是否允许继续上料
- 明确优先级：LL 下层一旦存在已完成工艺 wafer，优先下料回 LP，其优先级高于新的 LL 上料
- 保留 LL 侧交互准备规则：当 `case 1051` 检测到“一手持工艺完成片、一手为空、LL 上料槽仍有待加工片”时，先完成回 LL，再由 LL 侧最小改动地补一次取片准备，为下一次 PM 交互片预留节拍
- 收敛 LL 侧补取手指策略：保留“回片后立即补取”动作，但立即补取与普通取片都应优先遵守任务/配方配置的 `arm`，不再因为瞬时空手状态动态改写 `task.arm`
- 当立即补取窗口中任务配置的 `arm` 当前不满足发送条件时，允许放弃本轮“立即补取”并回到常规调度，不再为了保住交互片节拍临时改任务手指
- 明确循环完成与重新跑下一轮的判定条件：不得仅因部分 task 已达到 EFEM_RETURN/COMPLETED 就提前触发整组 UNKNOWN/UNKNOWN 重置
- 要求 TaskManager/调度逻辑保留在制 wafer 的可追踪状态，直到同一 LP/同一路径内不再存在依赖 EFEM 下料锁的 task
- 保持 PM 腔持续供片目标：在“下层完成片优先回 LP”的前提下，只要对应 LL 上层存在可上料空槽，且不会阻塞仍在途 wafer 的下料，系统就应允许继续发起 EFEM 上料
- 引导 WTR 长时间保持交互片节奏，优先形成“一只手拿待加工片、一只手拿加工完成片”的连续交换状态，减少堵塞
- 实现风格应保持与当前 `slot_transfer_cycle_vtm_widget.cpp` 一致，优先采用易审核的小范围状态机改动，不引入难读的新结构、过度抽象或复杂语法

## Impact
- Affected specs: LL 上进下出优先级；LLA/LLB 上料互锁；LL 回片后补取片节拍；TM 对 LL 交互片配合；EFEM 返回锁竞争处理；循环完成与重跑判定
- Affected code: [slot_transfer_cycle_vtm_widget.cpp](file:///d:/HLPrj/HL/device/src/slot_transfer_cycle_vtm_widget.cpp)；[TaskManager.cpp](file:///d:/HLPrj/HL/device/src/TaskManager.cpp)；[UnifiedWaferTask.h](file:///d:/HLPrj/HL/device/include/UnifiedWaferTask.h)

## ADDED Requirements
### Requirement: LL 连续上料许可基于槽位与在制冲突
系统 SHALL 将 LLA/LLB 的上料许可建立在目标 LL 是否存在可上料空槽以及继续上料是否会抢占仍在途 wafer 所需的 EFEM 下料机会两个条件上，而不是建立在另一侧整组 wafer 是否已经全部回到 LP的条件上。

#### Scenario: 上层有空槽且无下料冲突时继续上料
- **WHEN** 某一 LP 正在执行多片循环，已有 task 完成 EFEM_RETURN/COMPLETED，但另一 task 仍在 VTM/LL/PM 路径内等待后续下料
- **AND** 目标 LL 的上层存在可用于上料的空槽
- **AND** 放行这次上料不会使仍在途 task 丢失其后续 EFEM_RETURN 所需的调度机会
- **THEN** 系统允许继续发起新的 EFEM_TRANSFER / LL 上料，不等待整组 wafer 全部回到 LP

#### Scenario: 存在真实下料冲突时阻止上料
- **WHEN** 仍在途 task 已进入必须保留 EFEM 下料机会的阶段，或者目标 LL 已无可上料空槽
- **THEN** 系统继续保持上料互锁，直到冲突解除

### Requirement: LL 下层完成片优先下料回 LP
系统 SHALL 在 LL 上进下出模式下，将“LL 下层已完成工艺 wafer 下料回 LP”设为高于“继续向 LL 上层上料”的优先级，以避免 WTR 双手持片且 LLA/LLB 上层均被待加工片占满时形成堵塞。

#### Scenario: 下层有完成片时优先下料
- **WHEN** 任一 LL 下层存在已完成工艺、待回 LP 的 wafer
- **THEN** 系统优先分配 EFEM/WTR 资源执行该 wafer 的下料回 LP
- **AND** 在该优先下料动作完成前，不以“上层仍有空槽”为理由抢先执行新的 LL 上料

#### Scenario: 下层清空后恢复连续上料
- **WHEN** 已完成工艺 wafer 从 LL 下层成功下料回 LP，且目标 LL 上层仍存在空槽
- **AND** 当前不存在真实下料冲突
- **THEN** 系统恢复连续上料，继续向 LL 上层补充待加工 wafer

### Requirement: 在制 task 状态在安全点前不得被提前重置
系统 SHALL 在确认同一 LP 当前循环内不存在仍需依赖 EFEM 下料锁的在制 task 之前，禁止把已完成返回的 task 从 EFEM_RETURN/COMPLETED 提前重置为 UNKNOWN/UNKNOWN。

#### Scenario: 存在仍在途 wafer 时延后重置
- **WHEN** 某个 task 已处于 EFEM_RETURN/COMPLETED
- **AND** 同一 LP 或同一调度批次中仍存在尚未完成回 LP 的 task
- **THEN** 系统保留该已完成 task 的完成态，不触发循环重跑所依赖的 UNKNOWN/UNKNOWN 重置

#### Scenario: 全部依赖消失后执行重置
- **WHEN** 当前循环内所有 task 均已脱离对 EFEM 下料锁的依赖，且不会再因状态重置影响在制 wafer 的后续流转
- **THEN** 系统才允许执行 UNKNOWN/UNKNOWN 重置与下一轮循环初始化

### Requirement: 连续跑片时 PM 不因整组等待而断料
系统 SHALL 支持单个 LP 存在 25 片等大批量 wafer 时的连续传输，在满足安全互锁且遵守“下层完成片优先下料回 LP”优先级的前提下持续为 PM 提供后续 wafer，不因整组先回 LP 再开始下一组的策略造成不必要等待。

#### Scenario: 大批量连续跑片
- **WHEN** LP 中仍有待加工 wafer，且 LLA/LLB 中任一目标上层存在空槽
- **THEN** 调度逻辑优先维持 PM 连续供片，只在真实槽位不足、真实下料冲突存在，或 LL 下层存在更高优先级的回 LP 动作时暂停上料

### Requirement: WTR 优先维持交互片节奏
系统 SHALL 在不违反互锁和槽位约束的前提下，优先让 WTR 长时间处于交互片逻辑，尽量保持“一只手携带待加工片、另一只手携带已加工完成片”的节奏，降低因双手占满且 LL 槽位决策不当造成的等待。

#### Scenario: 交互片优先
- **WHEN** PM 仍在工艺中，WTR 双手、LLA/LLB 上下层和 LP 之间存在多种可调度动作
- **THEN** 调度逻辑优先选择能够维持后续连续交换的动作组合
- **AND** 避免出现“LL 上层持续加满待加工片、下层完成片迟迟不下到 LP”导致后续路径被堵死的策略

#### Scenario: 预装下一片等待 PM2 完成
- **WHEN** 某片 wafer 已经放入 `PM2` 进入工艺
- **AND** 另一片待加工 wafer 已从对应 `LLA/LLB` 取到 WTR 手上
- **THEN** 系统允许 WTR 按任务/配方配置的固定 `arm` 暂时持有这片待加工 wafer，等待 `PM2` 当前工艺完成
- **AND** 待 `PM2` 当前 wafer 完成后，系统应按 `PM2` 交换逻辑执行“空手取出完成片、持片手放入待加工片”
- **AND** 不要求为了维持这条节拍去动态改写该待加工 task 的 `arm`

### Requirement: LL 回片过程中要立刻补一次下一轮交互准备
系统 SHALL 在 LL 侧以最小改动方式支持“回 LL 后立即补取一片”的节拍。当 `case 1051` 发现一只手持工艺完成片、另一只手为空，且 LL 的上料槽仍有待加工片时，系统应先让 WTR 把工艺完成片放回 LL；在门仍打开、真空链路尚未进入 `case 2065` 后续关门/破真空收尾阶段之前，LL 线程可尝试从 LL 上料槽补取一片待加工 wafer 到手上，为下一次 PM 工艺完成后的交互片提前准备。`executeTMTransfer()` SHALL 配合该节拍，但触发入口应保留在 LL 线程侧，以满足最小改动约束。

#### Scenario: 回 LL 后立即补取一片等待下一次交互片
- **WHEN** `case 1051` 检测到 PM 中仍有在制 wafer
- **AND** WTR 一只手持有工艺完成片准备回 LL，另一只手为空
- **AND** 当前 LL 的上料槽中仍存在待加工 wafer
- **THEN** LL 线程先优先执行把工艺完成片放回 LL 的动作，不继续原地等待或直接跳回 `950`
- **AND** 在该放回 LL 的动作成功后、`case 2065` 后续关门/破真空收尾逻辑执行前，LL 线程可尝试按该待加工 task 当前配置的 `arm` 立即补取一片待加工 wafer
- **AND** 若该 task 当前配置的 `arm` 不满足发送条件，则系统允许放弃本轮立即补取，进入正常收尾与后续调度，而不是为补取成功临时改写 `task.arm`
- **AND** 一旦立即补取成功，补取后的结果应让系统形成“一手待加工、一手已清空或可继续回片”的下一轮交互准备状态，等待 PM 中 wafer 工艺完成后继续交互片
- **AND** 不允许把补取动作延后到完整走完 `case 2065` 及其后的回 LL 完成链路之后，以避免被关门、破真空与回到 `950` 的等待打断连续性

#### Scenario: 没有后续待加工片时停止预装
- **WHEN** 当前回 LL 动作完成后，`LLA/LLB` 都已无可继续上 PM2 的待加工 wafer
- **THEN** 系统停止新的补取/预装动作
- **AND** 保持 WTR 空手或仅处理当前最后一片回程
- **AND** 待 `PM2` 中最后一片 wafer 工艺完成后，按正常最终取片与回 LL 路径收尾

### Requirement: LL 立即补取与普通取片遵循固定手指语义
系统 SHALL 将 `task.arm` 视为该片 wafer 在 LL 到 PM 交互中的既定手指约束。无论是 `case 1051` 的普通取片，还是 `case 2066` 的回片后立即补取，都应优先按任务/配方配置的 `arm` 执行，不再因为“当前另一只手刚好为空”而动态改写 `task.arm` 去抢节拍。

#### Scenario: 配方手指可用时按固定手指执行
- **WHEN** 某待加工 task 已配置 `arm`
- **AND** 对应 `arm` 在当前发送窗口满足取片条件
- **THEN** LL 普通取片与立即补取都按该 `arm` 发起请求
- **AND** TaskManager 中该 task 的 `arm` 保持不变

#### Scenario: 配方手指不可用时不动态改写任务手指
- **WHEN** 某待加工 task 已配置 `arm`
- **AND** 该 `arm` 在当前窗口被实物、挂起请求或当前 `robot_step` 占用
- **THEN** 系统不得仅因另一只手当前为空而调用 `updateTaskArm()` 改写该 task 的 `arm`
- **AND** 系统应选择等待、跳过本轮立即补取，或回到正常调度，而不是引入新的瞬时手指语义

#### Scenario: TM 配合 LL 发起的交互准备
- **WHEN** LL 线程已在回 LL 动作后立即完成补取片准备
- **THEN** `executeTMTransfer()` 应识别并配合这次由 LL 发起的交互准备，不重复下发冲突动作
- **AND** 对 LL 的交互动作触发点仍保留在 LL 线程内，而不是改成由 TM 线程单独抢占发起

## MODIFIED Requirements
### Requirement: 循环完成与重跑判定
原有当部分 task 达到 EFEM_RETURN/COMPLETED 后即可进入循环完成分支，并在重跑前统一改写为 UNKNOWN/UNKNOWN的语义，修改为：只有当当前循环中所有会受该重置影响的在制 task 都已安全退出 EFEM 下料竞争后，才能进入循环完成、状态重置与下一轮初始化。

### Requirement: isLoadingInterlock() 的判定语义
原有 isLoadingInterlock() 通过另一侧是否仍存在经过 LL 的 wafer 标签/下层有片对整组上料加锁的语义，修改为：仅在检测到真实的槽位占用限制、真实的在制下料冲突，或当前存在更高优先级的 LL 下层回 LP 动作时才加锁，不再把另一侧尚未整组回 LP本身视为阻塞条件。

### Requirement: LL 交互准备的手指选择语义
原有“为保住 LL 回片后立即补取节拍，可以根据实际空手动态改写待加工 task 的 `arm`”的语义，修改为：补取节拍仍保留，但 `task.arm` 继续由配方/任务配置决定；若配置手指在当前窗口不可用，允许放弃本轮立即补取，也不再动态改写 `task.arm`。

## REMOVED Requirements
### Requirement: 整组回 LP 后才允许下一组上料
**Reason**: 该规则会将原本可以并行推进的上料、腔体加工和回片流程串行化，导致 PM 断料与循环效率下降，并放大状态提前重置带来的互锁问题。
**Migration**: 改为基于 LL 实际槽位、在制 task 是否仍需 EFEM 下料机会、LL 下层完成片是否需要优先回 LP，以及循环安全重置点的组合判定来决定是否放行下一片上料。
