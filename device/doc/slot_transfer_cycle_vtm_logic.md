# 多任务传输调度流程逻辑总结

本文档基于 `slot_transfer_cycle_vtm_widget.cpp` 文件，总结了多任务传输调度系统的核心架构、状态机流程及交互逻辑。

## 1. 核心调度架构

该系统采用了 **多线程 + 状态机** 的架构来实现多任务并行调度。

*   **并行任务（Threads）**
    系统启动了多个独立的线程/任务来分别控制不同的硬件子系统，互不阻塞但通过标志位同步。
    *   `executeEFEMTransfer()`: 负责 EFEM 端的机器人调度（LoadPort <-> LoadLock）。
    *   `executeLLATransfer()`: 负责 LoadLock A 的真空/大气循环及与 TM/PM 的交互。
    *   `executeLLBTransfer()`: 负责 LoadLock B 的真空/大气循环及与 TM/PM 的交互。
    *   `executePMxTransfer()`: (PM1-PM4) 负责各个工艺腔室的工艺流程。
    *   `executeTMTransfer()`: 负责传输模块的调度。

*   **同步机制**
    *   **标志位（Flags）**: 使用 `tool_allow_get_wafer_LLA`、`pm1_allow_get_put_wafer` 等布尔标志位在不同线程间握手。
        *   例如：LLA 准备好后置位标志，EFEM 线程检测到标志后执行取放片，完成后复位标志。
    *   **TaskManager**: 全局任务管理器，用于跟踪每个 Wafer 的状态（QUEUED, COMPLETED）和位置，各线程根据任务状态决定下一步动作。

## 2. 详细调度流程逻辑 (以 LoadLock A 为例)

`executeLLATransfer` 是核心函数之一，通过 `loadlock1_auto_step` 变量维护一个状态机：

### 2.1 初始化与判断 (Step 10)
*   检查任务队列。
*   **需上料且无片**：跳转至破真空流程（Step 20）。
*   **有片需处理**：跳转至关门/抽真空流程（Step 400）。
*   **任务完成**：跳转至结束检查（Step 6000）。

### 2.2 上料流程 (Load Cycle)
*   **破真空 (Step 20 -> 100)**: 检查真空度，执行自动破真空 (`createAutoBreakVacuumCommand`)。
*   **请求上料 (Step 300 -> 301)**: 打开 Cassette Door，设置 `tool_allow_get_wafer_LLA = true`，呼叫 EFEM 线程。
*   **等待上料 (Step 302)**: 循环等待直到 EFEM 完成操作（标志位复位）。
*   **上料完成 (Step 350)**: 更新任务状态，准备进入抽真空流程。

### 2.3 抽真空与传输准备 (Vacuum Cycle)
*   **关门 (Step 400)**: 关闭 Cassette Door。
*   **抽真空 (Step 410 -> 500 -> 510)**: 启动真空泵，循环检测真空度是否达到设定值。
*   **Mapping (Step 800 -> 810)**: (可选) 执行晶圆映射。
*   **任务分发 (Step 900 -> 950)**: 根据任务类型决定下一步：
    *   取晶圆去 PM (Step 1000)
    *   从 PM 放回晶圆 (Step 2000)

### 2.4 取晶圆去工艺 (Get Wafer to PM)
*   **准备 (Step 1000 - 1040)**: 选定 Slot，检查 Mapping，确认真空平衡。
*   **开传输门 (Step 1050)**: 打开 TM Cavity Door。
*   **机器人取片 (Step 1051)**: 调用真空机器人 (`WTR`) 从 LL 取片。
*   **调度 PM (Step 1052)**: 更新任务，设置对应 PM 的标志位 (`pmX_allow_get_put_wafer = true`)，通知 PM 开始工作，并关闭 TM 门。

### 2.5 从工艺放回晶圆 (Put Wafer from PM)
*   **准备 (Step 2000 - 2030)**: 选定空 Slot，确认真空平衡。
*   **开传输门 (Step 2050)**: 打开 TM Cavity Door。
*   **机器人放片 (Step 2060)**: 真空机器人将处理好的 Wafer 放回 LL。
*   **完成 (Step 2070)**: 更新任务状态，关闭 TM 门。

### 2.6 下料流程 (Unload Cycle)
*   **准备下料 (Step 5000)**: 确认所有工艺完成。
*   **破真空 (Step 5021)**: 执行破真空。
*   **开门呼叫 (Step 5022 - 5023)**: 打开 Cassette Door，呼叫 EFEM 下料 (`tool_allow_put_wafer_LLA = true`)。
*   **等待下料 (Step 5024)**: 等待 EFEM 取走 Wafer。
*   **结束 (Step 5025 - 6000)**: 关门，完成一次循环。

## 3. 调度流程图 (Mermaid)

以下是基于代码逻辑生成的 Mermaid 流程图，清晰展示了 LLA 线程的状态流转以及与 EFEM 的交互。

```mermaid
flowchart TD
    subgraph "LoadLock A Thread (executeLLATransfer)"
        Start((开始)) --> Step10{Step 10: 检查任务}
        
        %% 上料分支
        Step10 -- "需上料 (无片)" --> Step20[Step 20: 准备破真空]
        Step20 --> Step100[Step 100: 执行自动破真空]
        Step100 --> Step300[Step 300: 准备上料]
        Step300 --> Step301[Step 301: 开门 & 呼叫EFEM]
        Step301 --> Step302{Step 302: 等待EFEM上料}
        Step302 -- "标志位复位" --> Step350[Step 350: 上料完成更新任务]
        Step350 --> Step400
        
        %% 抽真空分支
        Step10 -- "有片 (需处理)" --> Step400[Step 400: 关闭Cassette门]
        Step400 --> Step410{Step 410: 检查真空?}
        Step410 -- "未达标" --> Step500[Step 500: 启动抽真空]
        Step500 --> Step510{Step 510: 等待真空达标}
        Step510 -- "达标" --> Step800[Step 800: Mapping检查]
        Step510 -- "未达标" --> Step510
        Step410 -- "已达标" --> Step800
        
        %% 任务分发
        Step800 --> Step900{Step 900: 任务决策}
        Step900 -- "待工艺 (LL->PM)" --> Step950_Get
        Step900 -- "工艺回传 (PM->LL)" --> Step950_Put
        Step900 -- "需下料" --> Step5000
        
        %% 取晶圆流程
        subgraph "Process: Get Wafer (LL -> PM)"
            Step950_Get[Step 950: 准备取片] --> Step1000[Step 1000: 选定Slot]
            Step1000 --> Step1040{Step 1040: 检查平衡}
            Step1040 -- "OK" --> Step1050[Step 1050: 打开TM门]
            Step1050 --> Step1051[Step 1051: 机器人取片动作]
            Step1051 --> Step1052[Step 1052: 通知PM & 关TM门]
            Step1052 --> Step950_Loop[回到 Step 950]
        end
        
        %% 放晶圆流程
        subgraph "Process: Put Wafer (PM -> LL)"
            Step950_Put[Step 950: 准备放片] --> Step2000[Step 2000: 选定Slot]
            Step2000 --> Step2030{Step 2030: 检查平衡}
            Step2030 -- "OK" --> Step2050[Step 2050: 打开TM门]
            Step2050 --> Step2060[Step 2060: 机器人放片动作]
            Step2060 --> Step2070[Step 2070: 更新状态 & 关TM门]
            Step2070 --> Step2080[回到 Step 950]
        end
        
        %% 下料流程
        Step5000[Step 5000: 准备下料] --> Step5021[Step 5021: 破真空]
        Step5021 --> Step5022[Step 5022: 开门]
        Step5022 --> Step5023[Step 5023: 呼叫EFEM下料]
        Step5023 --> Step5024{Step 5024: 等待EFEM下料}
        Step5024 -- "标志位复位" --> Step5025[Step 5025: 关门]
        Step5025 --> Step6000{Step 6000: 循环结束?}
        Step6000 -- "继续" --> Step10
    end

    subgraph "EFEM Thread"
        EFEM_Idle[空闲/轮询]
        EFEM_Load[执行上料动作]
        EFEM_Unload[执行下料动作]
    end

    %% 交互信号
    Step301 -.-> |Set tool_allow_get_wafer_LLA| EFEM_Load
    EFEM_Load -.-> |Reset Flag| Step302
    
    Step5023 -.-> |Set tool_allow_put_wafer_LLA| EFEM_Unload
    EFEM_Unload -.-> |Reset Flag| Step5024
    
    Step1052 -.-> |Set pmX_allow_get_put| PM_Process[PM Process]
```

## 4. 总结

该文件通过 **LoadLock A** 和 **LoadLock B** 两个并行线程作为中枢，连接大气端（EFEM）和真空端（TM/PM）。
- **EFEM** 负责“喂料”和“收料”。
- **LoadLock** 线程作为“气闸”，在真空和大气状态间切换，并协调机器人将晶圆送入或取出工艺腔。
- **状态机设计** 保证了流程的严谨性，每个步骤（如开关门、抽充气）都有明确的检查和错误处理逻辑。
