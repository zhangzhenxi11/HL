#pragma once
#include <vector>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include "./device/UnifiedWaferTask.h"

namespace FC {

    // 前置声明
    class slot_transfer_cycle_vtm_widget;
    struct UnifiedWaferTask;

    class TaskManager {
    public:
        // 单例模式访问
        static TaskManager& getInstance();

        // 删除拷贝构造函数和赋值操作符
        TaskManager(const TaskManager&) = delete;
        void operator=(const TaskManager&) = delete;

        // 添加新任务：一片晶圆是一个task
        void addTask(const UnifiedWaferTask& task);

        // 获取特定位置的任务
        std::vector<UnifiedWaferTask> getTasksByLocation(UnifiedWaferTask::Location location);

        // 更新任务状态（线程安全）
        void updateTaskStatus(int taskId, UnifiedWaferTask::Status newStatus, UnifiedWaferTask::TaskType newTaskType);

        // 通过任务ID获取任务状态和类型
       
        const std::pair<UnifiedWaferTask::TaskType, UnifiedWaferTask::Status>& getTaskStatusAndType(int taskId);

        UnifiedWaferTask::TaskType& getTaskType(int taskId);

        //清空任务
        void clearTasks();

        // 获取待处理任务
        std::vector<UnifiedWaferTask> getPendingTasks();

        // 检查是否有待处理任务
        bool hasPendingTasks();

        // 等待新任务（带超时）
        bool waitForTasks(int timeoutMs = 100);

        // 检查是否有任务
        bool hasTasks();

        // 设置停止标志
        void stop();

        bool isStopped() const;

    private:
        TaskManager() = default; // 私有构造函数
        ~TaskManager() = default;

        std::vector<UnifiedWaferTask> tasks_; //任务队列
        std::map<int, std::pair<UnifiedWaferTask::TaskType, UnifiedWaferTask::Status>> taskStatusMap_; //   taskID:{TYPE:STATUS}  任务状态映射
        std::mutex mutex_;
        std::condition_variable cv_;
        std::atomic<bool> stopped_{ false };
        int nextTaskId_ = 0;
    };
}