# HL 设备传输流程调度与线程协调分析

本文档基于 `slot_transfer_cycle_vtm_widget.cpp` 的源码分析，整理了多线程 + 状态机模式下的整体调度流程、线程协调机制以及单线程执行逻辑。

## 1. 晶圆整体调度流程图（生命周期）

该图展示了一片晶圆从装载口（LP）出发，经过真空腔体处理，最后回到装载口的完整生命周期流转。不同线程通过修改任务（Task）的状态来进行接力。

```mermaid
graph TD
    Start(("流程开始")) --> EFEM_Load["EFEM 线程: EWTR 从 LP 取片并寻边"]
    EFEM_Load --> EFEM_Put_LL["EFEM 线程: EWTR 将片放入 LLA/LLB"]
    
    EFEM_Put_LL -->|"任务状态变更为 LOADLOCK_TRANSFER QUEUED"| LL_Vacuum["LL 线程: 关闭大气门, 执行抽真空"]
    LL_Vacuum --> LL_WTR_Get["LL 线程: 驱动 WTR 从 LL 取片"]
    
    LL_WTR_Get -->|"任务状态变更为 PM_PROCESS QUEUED"| PM_WTR_Put["PM 线程: 驱动 WTR 将片放入 PM 腔体"]
    PM_WTR_Put --> PM_Process["PM 线程: 执行工艺流程 startPmMotorRun"]
    PM_Process --> PM_WTR_Get["PM 线程: 驱动 WTR 从 PM 取出加工完成的片"]
    
    PM_WTR_Get -->|"任务状态变更为 LOADLOCK_RETURN QUEUED"| LL_WTR_Put["LL 线程: 驱动 WTR 将片放回 LLA/LLB"]
    LL_WTR_Put --> LL_Vent["LL 线程: 关闭真空门, 执行破真空"]
    
    LL_Vent -->|"任务状态变更为 EFEM_TRANSFER QUEUED"| EFEM_Unload["EFEM 线程: EWTR 从 LL 取片"]
    EFEM_Unload --> EFEM_Put_LP["EFEM 线程: EWTR 放片回 LP 槽位"]
    EFEM_Put_LP --> EndNode(("单片 Cycle 结束: COMPLETED"))
```

## 2. 工作线程协调机制图

该图展示了 9 个工作线程之间是如何通过**条件变量、互斥锁和 TaskManager** 进行同步和资源竞争保护的。

```mermaid
graph TD
    subgraph GlobalControl ["全局调度控制 (UI & 主线程)"]
        UI["UI 控制面板"] -->|"Start/Pause/Reset"| Flags("全局标志: running, stopRequested")
        UI -->|"唤醒所有阻塞线程"| CV["条件变量 cv.notify_all"]
    end

    subgraph WorkerThreads ["独立工作线程 (While 循环)"]
        EFEM["EFEM 线程"]
        LLA["LLA 线程"]
        LLB["LLB 线程"]
        PM1["PM1-PM4 线程"]
        Vac["Vacuum 线程"]
    end

    Flags -.->|"检查标志位 cv.wait"| EFEM
    Flags -.->|"检查标志位 cv.wait"| LLA
    Flags -.->|"检查标志位 cv.wait"| LLB
    Flags -.->|"检查标志位 cv.wait"| PM1

    subgraph HardwareMutex ["硬件资源与互斥锁 (Mutex)"]
        EWTR["大气机械手 EWTR"]
        WTR["真空机械手 WTR"]
        EFEM_Lock(("efem_robot_mutex"))
        WTR_Lock(("wtr_robot_mutex"))
    end
    
    EFEM -.->|"申请上锁"| EFEM_Lock
    EFEM_Lock ==> EWTR
    
    LLA -.->|"申请上锁"| WTR_Lock
    LLB -.->|"申请上锁"| WTR_Lock
    PM1 -.->|"申请上锁"| WTR_Lock
    WTR_Lock ==> WTR
    
    subgraph TaskManagerCenter ["任务通信中枢 (TaskManager)"]
        TaskDB[("TaskManager 管理任务队列")]
    end
    
    EFEM <-->|"读写队列状态"| TaskDB
    LLA <-->|"读写队列状态"| TaskDB
    LLB <-->|"读写队列状态"| TaskDB
    PM1 <-->|"读写队列状态"| TaskDB
    
    classDef lock fill:#f9f,stroke:#333,stroke-width:2px;
    class EFEM_Lock,WTR_Lock lock;
```

## 3. 单个工作线程的执行流程图（以 EFEM 为例）

每一个工作线程的内部都是一个独立的“状态机（State Machine）”，根据对应的 `xxx_auto_step` 变量执行不同的阶段。

```mermaid
graph TD
    Start(("线程启动: executeEFEMTransfer")) --> LoopHead{"While (!stopRequested)"}
    
    LoopHead --> WaitCV{"等待条件变量 running == true?"}
    WaitCV -->|"False (暂停)"| WaitCV
    WaitCV -->|"True (运行中)"| CheckReset{"是否收到 Reset 信号?"}
    
    CheckReset -->|"Yes"| DoReset["将 efem_auto_step 重置为 10, 清理局部状态"] --> LoopHead
    CheckReset -->|"No"| Switch["Switch (efem_auto_step) 状态机分支"]
    
    Switch -->|"Step 10"| Step10["扫描任务列表, 决定下料或上料"]
    Switch -->|"Step 11"| Step11["优先级判断: 优先响应下料请求"]
    Switch -->|"Step 100-132"| Step100["执行上料子流程: 1.打开LP门 2.EWTR取片 3.寻边 4.放入LLA/LLB"]
    Switch -->|"Step 200+"| Step200["执行下料子流程: 1.EWTR从LLA/LLB取片 2.放回LP槽位"]
    
    Step10 --> UpdateState["更新任务状态 taskManager.updateTaskStatus"]
    Step11 --> UpdateState
    Step100 --> UpdateState
    Step200 --> UpdateState
    
    UpdateState --> LoopHead
    
    LoopHead -.->|"stopRequested == true"| EndNode(("线程安全退出"))
```

### 机制核心说明：
1. **启停控制（CV 同步）**：所有工作线程在每次循环头部调用 `cv.wait()` 检查 `running` 和 `stopRequested`。当用户点击“暂停”或“重置”时，主线程修改标志位并调用 `cv.notify_all()` 唤醒线程。
2. **硬件防撞（Mutex 互斥锁）**：由于多个线程（LLA、LLB、PM1~PM4）都需要操作唯一的真空机械手（WTR），在调用 WTR 的动作指令前，必须申请 `wtr_robot_mutex` 锁，确保同一时间只有一个线程在驱动手臂。
3. **工序交接（TaskManager）**：线程之间没有直接的代码互相调用，而是通过修改 `TaskManager` 中的 `UnifiedWaferTask::TaskType` 和 `Status` 来实现解耦的异步流水线接力。
