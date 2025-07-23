#pragma once
// UnifiedWaferTask.h
#include <chrono>
#include "kernel/kernel.h"


namespace FC {
    struct EFEMSubState
    {
        enum EFEMSubStep {
            EFEM_STEP_OPEN_LOADPORT = 0,
            EFEM_STEP_GET_MAP,
            EFEM_STEP_PROCESS_MAPPING, //mapping
            EFEM_STEP_JUDGMENT_SINGLE_DOUBLE_CONDITIONS,
            EFEM_STEP_CHECK_TOOL1_UP , //给lla 上料
            EFEM_STEP_CHECK_TOOL1_DOWN,//给lla 下料
            EFEM_STEP_CHECK_TOOL2_UP,  //给llb 上料
            EFEM_STEP_CHECK_TOOL2_DOWN,//给llb 下料
            EFEM_STEP_PREPARE_GET_WAFER,//取片
            EFEM_STEP_GET_WAFER_SINGLE,//单取 //Feeding
            EFEM_STEP_PUT_WAFER_SINGLE,//单放
            EFEM_STEP_GET_WAFER_DOUBLE,//双取
            EFEM_STEP_PUT_WAFER_DOUBLE,//双放
            EFEM_STEP_GET_WAFER_SINGLE_BLANKING,//单取 
            EFEM_STEP_PUT_WAFER_SINGLE_BLANKING,//单放
            EFEM_STEP_GET_WAFER_ALIGNER,//取ALIGNER
            EFEM_STEP_PUT_WAFER_ALIGNER,//放ALIGNER
            EFEM_STEP_ALIGNER,         //寻边
            EFEM_STEP_OPEN_LOADLOCK,   //LL开门
            EFEM_STEP_CLOSE_LOADPORT,
            EFEM_STEP_END
            // ... 其他步骤
        };

        EFEMSubStep efemSubStep = EFEM_STEP_OPEN_LOADPORT;
        std::string sourceLpName;  // "ELP1" 或 "ELP2"
        int sourceSlot = -1;
        int targetSlot = -1;
        int armSelection = 0;  // 0=A, 1=B

        // 当前正在执行的命令
        std::shared_ptr<IKernelCommand> commandInProgress;

    };

    //1.破LLA真空 2.开casste门 3.mapping ，获取无晶圆槽号，4.efem_csr 放片 5.efem_csr 取片
    struct LLSubState
    {
        enum LLSubStep
        {
            LL_STEP_BREAK_VACUUM = 0,//破真空
            LL_STEP_OPEN_CASSTEDOOR, //打开casste门
            LL_STEP_CLOSE_CASSTEDOOR,
            LL_STEP_MAPPING,
            LL_STEP_EFEM_PUT_WAFER,
            LL_STEP_EFEM_GET_WAFER,
            LL_STEP_VACUUMING, //抽真空
            LL_STEP_VACUUM_CSR_PUT_WAFER,
            LL_STEP_VACUUM_CSR_GET_WAFER,
            LL_STEP_OPEN_TRANS_DOOR,
            LL_STEP_CLOSE_TRANS_DOOR,
            LL_STEP_END
            //....
        };

        LLSubStep llSubStep = LL_STEP_BREAK_VACUUM;
        std::string sourceLpName;  // "ELP1" 或 "ELP2"
        int sourceSlot = -1;
        int targetSlot = -1;
        int efemArmSelection = 0;  // 0=A, 1=B
        int vacuumArmSelection = 0;  // 0=A, 1=B
        // 当前正在执行的命令
        std::shared_ptr<IKernelCommand> commandInProgress;
    };

    struct PMSubState
    {
        //1.开门  2.升降电机下降到取放片位  3.csr放片  4.关门  5.开始工艺 6.下料，开门，7.csr取片  8.再放新片 
        enum PMSubStep
        {
            PM_STEP_VACUUM_CSR_PUT_WAFER,
            PM_STEP_VACUUM_CSR_GET_WAFER,
            PM_STEP_OPEN_TRANS_DOOR,
            PM_STEP_CLOSE_TRANS_DOOR,
            PM_STEP_DOWN_POS1,
            PM_STEP_UP_POS2,
            PM_STEP_DOWN_POS3,
            PM_STEP_PROCESS
            //....
        };
    };
    

    struct UnifiedWaferTask {
        enum Location { LP1, LP2, LLA, LLB, PM1, PM2, PM3, PM4 };

        enum Status { 
            QUEUED,      //待工艺
            IN_PROGRESS, //工艺中
            IN_ERROR,    //工艺报错
            //IN_CONTIUNE, //工艺跳转
            COMPLETED,   //工艺完成
            UNKNOWN_PROGRESS
        }; //工艺状态

        enum TaskType {
            EFEM_TRANSFER,      // EFEM上料
            LOADLOCK_TRANSFER,  // LOADLOCK上料
            ROBOT_PROCESS,      // 真空机械ROBOT
            PM_PROCESS,         // PM工艺
            LOADLOCK_RETURN,    // LOADLOCK下料
            EFEM_RETURN,        // EFEM下料
            UNKNOWN
        };//传输类型状态


        int taskId; //ID
        TaskType taskType; 
        int currentStep = 0;  // 当前步骤索引
        Location source; //来源模组
        Location target; //目标模组
        Status status;
        int sourceSlot;  
        int targetSlot; 
        int arm;         // 真空机械臂选择0=A, 1=B

        // 工艺参数
        bool pm1Enabled;
        bool pm2Enabled;
        bool pm3Enabled;
        bool pm4Enabled;

        EFEMSubState EfemState; //efem
        LLSubState   LLState;    //LLA,LLB
        PMSubState   pmState;    //PM1,PM2,PM3,PM4

        // 时间戳
        std::chrono::system_clock::time_point createdAt;
        std::chrono::system_clock::time_point startedAt;
        std::chrono::system_clock::time_point completedAt;

        // 位置转换工具函数
        static std::string locationToString(Location loc) {
            switch (loc) {
            case Location::LP1: return "LP1";
            case Location::LP2: return "LP2";
            case Location::LLA: return "LLA";
            case Location::LLB: return "LLB";
            case Location::PM1: return "PM1";
            case Location::PM2: return "PM2";
            case Location::PM3: return "PM3";
            case Location::PM4: return "PM4";
            default: return "UNKNOWN";
            }
        }

        // 状态转换工具函数
        static std::string statusToString(Status stat) 
        {
            switch (stat) {
            case Status::QUEUED: return "QUEUED";
            case Status::IN_PROGRESS: return "IN_PROGRESS";
            case Status::COMPLETED: return "COMPLETED";
            default: return "UNKNOWN";
            }
        }
    };

    




} // namespace FC