/**
 * @file            CycleStateSnapshot.cpp
 * @brief           Cycle调度系统状态快照保存与恢复实现
 * @author          xielonghua
 * @date            2025-10-28
 */

#include "CycleStateSnapshot.h"
#include "TaskManager.h"
#include "Poco/JSON/Parser.h"
#include "Poco/JSON/Stringifier.h"
#include "Poco/File.h"
#include "Poco/Path.h"
#include "Poco/DirectoryIterator.h"
#include "Poco/DateTime.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/DateTimeFormat.h"
#include <fstream>
#include <sstream>
#include <iostream>

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

namespace FC {

// 获取默认快照保存目录
std::string CycleStateSnapshot::getDefaultSnapshotDir() {
    return "./CycleSnapshots/";
}

// 生成快照文件名
std::string CycleStateSnapshot::generateSnapshotFileName() {
    Poco::DateTime now;
    std::string timestamp = Poco::DateTimeFormatter::format(now, "%Y%m%d_%H%M%S");
    return "snapshot_" + timestamp + ".json";
}

// 将UnifiedWaferTask转换为JSON对象
Poco::JSON::Object::Ptr CycleStateSnapshot::taskToJson(const UnifiedWaferTask& task) {
    Poco::JSON::Object::Ptr jsonObj = new Poco::JSON::Object();
    
    jsonObj->set("taskId", task.taskId);
    jsonObj->set("taskType", UnifiedWaferTask::typeToString(task.taskType));
    jsonObj->set("status", UnifiedWaferTask::statusToString(task.status));
    jsonObj->set("currentStep", task.currentStep);
    jsonObj->set("source", UnifiedWaferTask::locationToString(task.source));
    jsonObj->set("destination", UnifiedWaferTask::locationToString(task.destination));
    jsonObj->set("target", UnifiedWaferTask::locationToString(task.target));
    jsonObj->set("egressLoadLock", UnifiedWaferTask::locationToString(task.egressLoadLock));
    jsonObj->set("target_pm", UnifiedWaferTask::locationToString(task.target_pm));
    jsonObj->set("sourceSlot", task.sourceSlot);
    jsonObj->set("destinationSlot", task.destinationSlot);
    jsonObj->set("targetSlot", task.targetSlot);
    jsonObj->set("targetFeedingSlot", task.targetFeedingSlot);
    jsonObj->set("targetBlankingSlot", task.targetBlankingSlot);
    jsonObj->set("arm", task.arm);
    jsonObj->set("isLoadingInPlace", task.isLoadingInPlace);
    jsonObj->set("pm1Enabled", task.pm1Enabled);
    jsonObj->set("pm2Enabled", task.pm2Enabled);
    jsonObj->set("pm3Enabled", task.pm3Enabled);
    jsonObj->set("pm4Enabled", task.pm4Enabled);
    
    // selectPmEnableList数组
    Poco::JSON::Array::Ptr pmArray = new Poco::JSON::Array();
    for (int i = 0; i < 4; ++i) {
        pmArray->add(task.selectPmEnableList[i]);
    }
    jsonObj->set("selectPmEnableList", pmArray);
    
    return jsonObj;
}

// 从JSON对象解析UnifiedWaferTask
bool CycleStateSnapshot::jsonToTask(const Poco::JSON::Object::Ptr& jsonObj, UnifiedWaferTask& task) {
    try {
        task.taskId = jsonObj->getValue<int>("taskId");
        
        // 解析枚举类型（需要字符串转枚举的辅助函数）
        std::string taskTypeStr = jsonObj->getValue<std::string>("taskType");
        std::string statusStr = jsonObj->getValue<std::string>("status");
        std::string sourceStr = jsonObj->getValue<std::string>("source");
        std::string destinationStr = jsonObj->optValue<std::string>("destination", sourceStr);
        std::string targetStr = jsonObj->getValue<std::string>("target");
        std::string egressLoadLockStr = jsonObj->optValue<std::string>("egressLoadLock", targetStr);
        std::string targetPmStr = jsonObj->getValue<std::string>("target_pm");
        
        // 这里需要实现字符串到枚举的转换
        // 简化处理：假设有辅助函数（实际需要实现）
        task.source = TaskManager::getInstance().stringToLocation(sourceStr);
        task.destination = TaskManager::getInstance().stringToLocation(destinationStr);
        task.target = TaskManager::getInstance().stringToLocation(targetStr);
        task.egressLoadLock = TaskManager::getInstance().stringToLocation(egressLoadLockStr);
        task.target_pm = TaskManager::getInstance().stringToLocation(targetPmStr);
        task.currentStep = jsonObj->getValue<int>("currentStep");
        task.sourceSlot = jsonObj->getValue<int>("sourceSlot");
        task.destinationSlot = jsonObj->optValue<int>("destinationSlot", task.sourceSlot);
        task.targetSlot = jsonObj->getValue<int>("targetSlot");
        task.targetFeedingSlot = jsonObj->getValue<int>("targetFeedingSlot");
        task.targetBlankingSlot = jsonObj->getValue<int>("targetBlankingSlot");
        task.arm = jsonObj->getValue<int>("arm");
        task.isLoadingInPlace = jsonObj->getValue<bool>("isLoadingInPlace");
        task.pm1Enabled = jsonObj->getValue<bool>("pm1Enabled");
        task.pm2Enabled = jsonObj->getValue<bool>("pm2Enabled");
        task.pm3Enabled = jsonObj->getValue<bool>("pm3Enabled");
        task.pm4Enabled = jsonObj->getValue<bool>("pm4Enabled");
        
        // 解析selectPmEnableList数组
        Poco::JSON::Array::Ptr pmArray = jsonObj->getArray("selectPmEnableList");
        for (int i = 0; i < 4 && i < pmArray->size(); ++i) {
            task.selectPmEnableList[i] = pmArray->getElement<int>(i);
        }
        
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "解析Task JSON失败: " << e.what() << std::endl;
        return false;
    }
}

// 将Snapshot转换为JSON对象
Poco::JSON::Object::Ptr CycleStateSnapshot::snapshotToJson(const Snapshot& snapshot) {
    Poco::JSON::Object::Ptr jsonObj = new Poco::JSON::Object();
    
    // 基本信息
    jsonObj->set("timestamp", snapshot.timestamp);
    jsonObj->set("errorMessage", snapshot.errorMessage);
    jsonObj->set("errorLocation", snapshot.errorLocation);
    
    // 各线程step状态
    jsonObj->set("efem_auto_step", snapshot.efem_auto_step);
    jsonObj->set("loadlock1_auto_step", snapshot.loadlock1_auto_step);
    jsonObj->set("loadlock2_auto_step", snapshot.loadlock2_auto_step);
    jsonObj->set("robot_auto_step", snapshot.robot_auto_step);
    jsonObj->set("vacuum_auto_step", snapshot.vacuum_auto_step);
    jsonObj->set("pm1_auto_step", snapshot.pm1_auto_step);
    jsonObj->set("pm2_auto_step", snapshot.pm2_auto_step);
    jsonObj->set("pm3_auto_step", snapshot.pm3_auto_step);
    jsonObj->set("pm4_auto_step", snapshot.pm4_auto_step);
    jsonObj->set("update_auto_step", snapshot.update_auto_step);
    
    // 标志位状态
    jsonObj->set("tool_allow_get_wafer_LLA", snapshot.tool_allow_get_wafer_LLA);
    jsonObj->set("tool_allow_get_wafer_LLB", snapshot.tool_allow_get_wafer_LLB);
    jsonObj->set("tool_allow_put_wafer_LLA", snapshot.tool_allow_put_wafer_LLA);
    jsonObj->set("tool_allow_put_wafer_LLB", snapshot.tool_allow_put_wafer_LLB);
    jsonObj->set("lp1_cycle_one_time_finished", snapshot.lp1_cycle_one_time_finished);
    jsonObj->set("lp2_cycle_one_time_finished", snapshot.lp2_cycle_one_time_finished);
    jsonObj->set("cycleFinished_lla", snapshot.cycleFinished_lla);
    jsonObj->set("cycleFinished_llb", snapshot.cycleFinished_llb);
    
    // 任务统计
    jsonObj->set("originTaskSize", snapshot.originTaskSize);
    jsonObj->set("lp1TaskSize", snapshot.lp1TaskSize);
    jsonObj->set("lp2TaskSize", snapshot.lp2TaskSize);
    
    // 其他状态
    jsonObj->set("current_lp_cycle", snapshot.current_lp_cycle);
    jsonObj->set("robot_selected_arm", snapshot.robot_selected_arm);
    
    // 任务列表
    Poco::JSON::Array::Ptr tasksArray = new Poco::JSON::Array();
    for (const auto& task : snapshot.allTasks) {
        tasksArray->add(taskToJson(task));
    }
    jsonObj->set("allTasks", tasksArray);
    
    return jsonObj;
}

// 从JSON对象解析Snapshot
bool CycleStateSnapshot::jsonToSnapshot(const Poco::JSON::Object::Ptr& jsonObj, Snapshot& snapshot) {
    try {
        // 基本信息
        snapshot.timestamp = jsonObj->getValue<std::string>("timestamp");
        snapshot.errorMessage = jsonObj->getValue<std::string>("errorMessage");
        snapshot.errorLocation = jsonObj->getValue<std::string>("errorLocation");
        
        // 各线程step状态
        snapshot.efem_auto_step = jsonObj->getValue<int>("efem_auto_step");
        snapshot.loadlock1_auto_step = jsonObj->getValue<int>("loadlock1_auto_step");
        snapshot.loadlock2_auto_step = jsonObj->getValue<int>("loadlock2_auto_step");
        snapshot.robot_auto_step = jsonObj->getValue<int>("robot_auto_step");
        snapshot.vacuum_auto_step = jsonObj->getValue<int>("vacuum_auto_step");
        snapshot.pm1_auto_step = jsonObj->getValue<int>("pm1_auto_step");
        snapshot.pm2_auto_step = jsonObj->getValue<int>("pm2_auto_step");
        snapshot.pm3_auto_step = jsonObj->getValue<int>("pm3_auto_step");
        snapshot.pm4_auto_step = jsonObj->getValue<int>("pm4_auto_step");
        snapshot.update_auto_step = jsonObj->getValue<int>("update_auto_step");
        
        // 标志位状态
        snapshot.tool_allow_get_wafer_LLA = jsonObj->getValue<bool>("tool_allow_get_wafer_LLA");
        snapshot.tool_allow_get_wafer_LLB = jsonObj->getValue<bool>("tool_allow_get_wafer_LLB");
        snapshot.tool_allow_put_wafer_LLA = jsonObj->getValue<bool>("tool_allow_put_wafer_LLA");
        snapshot.tool_allow_put_wafer_LLB = jsonObj->getValue<bool>("tool_allow_put_wafer_LLB");
        snapshot.lp1_cycle_one_time_finished = jsonObj->getValue<bool>("lp1_cycle_one_time_finished");
        snapshot.lp2_cycle_one_time_finished = jsonObj->getValue<bool>("lp2_cycle_one_time_finished");
        snapshot.cycleFinished_lla = jsonObj->getValue<bool>("cycleFinished_lla");
        snapshot.cycleFinished_llb = jsonObj->getValue<bool>("cycleFinished_llb");
        
        // 任务统计
        snapshot.originTaskSize = jsonObj->getValue<int>("originTaskSize");
        snapshot.lp1TaskSize = jsonObj->getValue<int>("lp1TaskSize");
        snapshot.lp2TaskSize = jsonObj->getValue<int>("lp2TaskSize");
        
        // 其他状态
        snapshot.current_lp_cycle = jsonObj->getValue<bool>("current_lp_cycle");
        snapshot.robot_selected_arm = jsonObj->getValue<int>("robot_selected_arm");
        
        // 任务列表
        Poco::JSON::Array::Ptr tasksArray = jsonObj->getArray("allTasks");
        snapshot.allTasks.clear();
        for (size_t i = 0; i < tasksArray->size(); ++i) {
            Poco::JSON::Object::Ptr taskObj = tasksArray->getObject(i);
            UnifiedWaferTask task;
            if (jsonToTask(taskObj, task)) {
                snapshot.allTasks.push_back(task);
            }
        }
        
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "解析Snapshot JSON失败: " << e.what() << std::endl;
        return false;
    }
}

// 保存快照到JSON文件
bool CycleStateSnapshot::saveSnapshot(const Snapshot& snapshot, const std::string& filePath) {
    try {
        // 确定保存路径
        std::string savePath = filePath;
        if (savePath.empty()) {
            std::string dir = getDefaultSnapshotDir();
            Poco::File dirFile(dir);
            if (!dirFile.exists()) {
                dirFile.createDirectories();
            }
            savePath = dir + generateSnapshotFileName();
        }
        
        // 转换为JSON
        Poco::JSON::Object::Ptr jsonObj = snapshotToJson(snapshot);
        
        // 写入文件
        std::ofstream ofs(savePath);
        if (!ofs.is_open()) {
            std::cerr << "无法打开文件: " << savePath << std::endl;
            return false;
        }
        
        Poco::JSON::Stringifier::stringify(jsonObj, ofs, 2);  // 缩进2空格，便于阅读
        ofs.close();
        
        std::cout << "✅ 状态快照已保存: " << savePath << std::endl;
        std::cout << "   错误信息: " << snapshot.errorMessage << std::endl;
        std::cout << "   错误位置: " << snapshot.errorLocation << std::endl;
        
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "保存快照失败: " << e.what() << std::endl;
        return false;
    }
}

// 从JSON文件加载快照
bool CycleStateSnapshot::loadSnapshot(const std::string& filePath, Snapshot& snapshot) {
    try {
        // 检查文件是否存在
        Poco::File file(filePath);
        if (!file.exists()) {
            std::cerr << "快照文件不存在: " << filePath << std::endl;
            return false;
        }
        
        // 读取文件
        std::ifstream ifs(filePath);
        if (!ifs.is_open()) {
            std::cerr << "无法打开文件: " << filePath << std::endl;
            return false;
        }
        
        // 解析JSON
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(ifs);
        Poco::JSON::Object::Ptr jsonObj = result.extract<Poco::JSON::Object::Ptr>();
        ifs.close();
        
        // 转换为Snapshot
        bool success = jsonToSnapshot(jsonObj, snapshot);
        
        if (success) {
            std::cout << "✅ 状态快照已加载: " << filePath << std::endl;
            std::cout << "   时间戳: " << snapshot.timestamp << std::endl;
            std::cout << "   错误信息: " << snapshot.errorMessage << std::endl;
            std::cout << "   任务数量: " << snapshot.allTasks.size() << std::endl;
        }
        
        return success;
    }
    catch (const std::exception& e) {
        std::cerr << "加载快照失败: " << e.what() << std::endl;
        return false;
    }
}

// 列出所有快照文件
std::vector<std::string> CycleStateSnapshot::listSnapshots() {
    std::vector<std::string> snapshots;
    
    try {
        std::string dir = getDefaultSnapshotDir();
        Poco::File dirFile(dir);
        if (!dirFile.exists()) {
            return snapshots;
        }
        
        Poco::DirectoryIterator it(dir);
        Poco::DirectoryIterator end;
        
        while (it != end) {
            if (it->isFile() && it.path().getExtension() == "json") {
                snapshots.push_back(it.path().toString());
            }
            ++it;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "列出快照文件失败: " << e.what() << std::endl;
    }
    
    return snapshots;
}

} // namespace FC
