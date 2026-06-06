#pragma once
// UnifiedWaferTask.h
#include <chrono>
#include "kernel/kernel.h"
#include <array>
#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

namespace FC {
    
    struct UnifiedWaferTask {
        enum Location { LP1, LP2, LLA, LLB, PM1, PM2, PM3, PM4 };//位置

        enum Status { 
            QUEUED,      //待做
            IN_PROGRESS, //过程中
            IN_ERROR,    //过程报错
            COMPLETED,   //完成
            UNKNOWN_PROGRESS//未知过程
        }; //工艺状态

        enum TaskType {
            EFEM_TRANSFER,      // EFEM上料
            LOADLOCK_TRANSFER,  // LOADLOCK上料
            ROBOT_PROCESS,      // 真空机械ROBOT
            PM_PROCESS,         // PM腔
            LOADLOCK_RETURN,    // LOADLOCK下料
            EFEM_RETURN,        // EFEM下料
            UNKNOWN
        };//传输类型状态

        //2026-1-06 新增机械手动过状态
        enum RobotStatus
        {
            PICK_QUEUED,       //准备取片
            PICK_COMPLETED,    //取片完成
            PLACE_QUEUED,      //准备放片
            PLACE_COMPLETED,   //放片完成
            ACTION_ERROR       //错误
        };

        enum AlignerStatus
        {
            ALIGNER_READY,
            ALIGNER_PROCESS,
            ALIGNER_COMPLETED
        };

        int taskId; //ID
        TaskType taskType; 
        Status status;
        int currentStep = 3;  // 当前步骤索引
        Location source;  //来源模组   lp
        Location target;  //目标模组   loadlock
        Location target_pm;//目标模组2  PM
        //2026-5-17
		Location realTimePosition; // 任务的实时位置，初始值为source，过程中根据实际位置更新

        int sourceSlot;  //来源模组槽
        int targetSlot;  //目标模组槽

        int targetFeedingSlot;  //目标模组上料槽
        int targetBlankingSlot; //目标模组下料槽

        int arm;         // binding arm: 0=A, 1=B (immutable after creation)
        AlignerStatus Aligner_status;

        std::array<int, 4> selectPmEnableList;

        //上下料到LOADLOCK 到位标签 ，true：上料到位  false:下料到位(初始状态)
        bool isLoadingInPlace = false;
        

        // 工艺参数
        bool pm1Enabled;
        bool pm2Enabled;
        bool pm3Enabled;
        bool pm4Enabled;
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
            case Status::IN_ERROR: return "IN_ERROR";
            case Status::COMPLETED: return "COMPLETED";
            default: return "UNKNOWN";
            }
        }
        // 类型转换工具函数
        static std::string typeToString(TaskType stat)
        {
            switch (stat) {
            case TaskType::EFEM_TRANSFER: return "EFEM_TRANSFER";
            case TaskType::LOADLOCK_TRANSFER: return "LOADLOCK_TRANSFER";
            case TaskType::ROBOT_PROCESS:return "ROBOT_PROCESS";
            case TaskType::PM_PROCESS: return "PM_PROCESS";
            case TaskType::LOADLOCK_RETURN: return "LOADLOCK_RETURN";
            case TaskType::EFEM_RETURN: return "EFEM_RETURN";
            default: return "UNKNOWN";
            }
        }
    };


} // namespace FC
