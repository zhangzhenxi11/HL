/**
/**
 * @file            CycleStateSnapshot.h
 * @brief           Cycle调度系统状态快照保存与恢复
 * @author          xielonghua
 * @date            2025-10-28
 */

#ifndef _CYCLE_STATE_SNAPSHOT_H_
#define _CYCLE_STATE_SNAPSHOT_H_

#include <string>
#include <vector>
#include <chrono>
#include "UnifiedWaferTask.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/Array.h"

namespace FC {

/**
 * @brief Cycle调度系统状态快照
 * 
 * 用于在调试时保存和恢复系统运行状态，避免每次从头开始调试
 */
class CycleStateSnapshot {
public:
    /**
     * @brief 状态快照结构
     */
    struct Snapshot {
        // 时间戳
        std::string timestamp;
        std::string errorMessage;  // 错误信息
        std::string errorLocation; // 错误位置（模块名 + step）
        
        // 各线程的step状态
        int efem_auto_step = 0;
        int loadlock1_auto_step = 0;
        int loadlock2_auto_step = 0;
        int robot_auto_step = 0;
        int vacuum_auto_step = 0;
        int pm1_auto_step = 0;
        int pm2_auto_step = 0;
        int pm3_auto_step = 0;
        int pm4_auto_step = 0;
        int update_auto_step = 0;
        
        // 任务管理器状态
        std::vector<UnifiedWaferTask> allTasks;  // 所有任务
        
        // 标志位状态
        bool tool_allow_get_wafer_LLA = false;
        bool tool_allow_get_wafer_LLB = false;
        bool tool_allow_put_wafer_LLA = false;
        bool tool_allow_put_wafer_LLB = false;
        bool lp1_cycle_one_time_finished = false;
        bool lp2_cycle_one_time_finished = false;
        bool cycleFinished_lla = false;
        bool cycleFinished_llb = false;
        
        // 任务大小统计
        int originTaskSize = 0;
        int lp1TaskSize = 0;
        int lp2TaskSize = 0;
        
        // 其他重要状态
        bool current_lp_cycle = false;  // 当前循环的LP
        int robot_selected_arm = 0;
    };
    
    /**
     * @brief 保存当前状态快照到JSON文件
     * @param snapshot 状态快照
     * @param filePath JSON文件路径（可选，默认为 snapshot_时间戳.json）
     * @return true=保存成功, false=保存失败
     */
    static bool saveSnapshot(const Snapshot& snapshot, const std::string& filePath = "");
    
    /**
     * @brief 从JSON文件加载状态快照
     * @param filePath JSON文件路径
     * @param snapshot 输出参数，加载的状态快照
     * @return true=加载成功, false=加载失败
     */
    static bool loadSnapshot(const std::string& filePath, Snapshot& snapshot);
    
    /**
     * @brief 将Snapshot转换为JSON对象
     */
    static Poco::JSON::Object::Ptr snapshotToJson(const Snapshot& snapshot);
    
    /**
     * @brief 从JSON对象解析Snapshot
     */
    static bool jsonToSnapshot(const Poco::JSON::Object::Ptr& jsonObj, Snapshot& snapshot);
    
    /**
     * @brief 将UnifiedWaferTask转换为JSON对象
     */
    static Poco::JSON::Object::Ptr taskToJson(const UnifiedWaferTask& task);
    
    /**
     * @brief 从JSON对象解析UnifiedWaferTask
     */
    static bool jsonToTask(const Poco::JSON::Object::Ptr& jsonObj, UnifiedWaferTask& task);
    
    /**
     * @brief 获取默认快照保存目录
     */
    static std::string getDefaultSnapshotDir();
    
    /**
     * @brief 列出所有保存的快照文件
     */
    static std::vector<std::string> listSnapshots();
    
    /**
     * @brief 生成快照文件名（基于时间戳）
     */
    static std::string generateSnapshotFileName();

private:
    CycleStateSnapshot() = default;
};

} // namespace FC

#endif // _CYCLE_STATE_SNAPSHOT_H_
