#include "TaskManager.h"
#include <algorithm>
#include <iterator>

FC::TaskManager& FC::TaskManager::getInstance()
{     
    static FC::TaskManager instance;
    return instance;
    
}

void FC::TaskManager::addTask(const FC::UnifiedWaferTask& task)
{
    std::unique_lock<std::mutex> lock(mutex_);
    UnifiedWaferTask newTask = task;
    newTask.taskId = nextTaskId_++; //1,2,3
    newTask.createdAt = std::chrono::system_clock::now();
    tasks_.push_back(newTask);

    taskStatusMap_[newTask.taskId] = { newTask.taskType, newTask.status }; // 初始化任务状态
    lock.unlock();
    cv_.notify_one();
}

std::vector< FC::UnifiedWaferTask> FC::TaskManager::getTasksByLocation(FC::UnifiedWaferTask::Location location)
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<UnifiedWaferTask> result;

    for (const auto& task : tasks_) {
        if (task.source == location || task.target == location) {
            result.push_back(task);
        }
    }
    return result;
}

void FC::TaskManager::updateTaskStatus(int taskId, UnifiedWaferTask::Status newStatus, UnifiedWaferTask::TaskType newTaskType)
{
    std::lock_guard<std::mutex> lock(mutex_);

    for (auto& task : tasks_) {
        if (task.taskId == taskId) {
            task.status = newStatus;
            task.taskType = newTaskType;
            taskStatusMap_[taskId] = { newTaskType, newStatus }; // 更新任务状态
            if (newStatus == UnifiedWaferTask::Status::IN_PROGRESS) {
                task.startedAt = std::chrono::system_clock::now();
            }
            else if (newStatus == UnifiedWaferTask::Status::COMPLETED) {
                task.completedAt = std::chrono::system_clock::now();
            }
            break;
        }
    }
}

const std::pair<FC::UnifiedWaferTask::TaskType, FC::UnifiedWaferTask::Status>& FC::TaskManager::getTaskStatusAndType(int taskId)
{
    
        std::lock_guard<std::mutex> lock(mutex_);

        auto it = taskStatusMap_.find(taskId);
        if (it != taskStatusMap_.end()) {
            return it->second;
        }

        // 返回静态默认值
        static const auto defaultStatus = std::make_pair(
            UnifiedWaferTask::TaskType::UNKNOWN,
            UnifiedWaferTask::Status::UNKNOWN_PROGRESS
        );
        return defaultStatus;
    
}

FC::UnifiedWaferTask::TaskType& FC::TaskManager::getTaskType(int taskId)
{
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = taskStatusMap_.find(taskId);

    if (it != taskStatusMap_.end())
    {
        return it->second.first;
    }
    UnifiedWaferTask::TaskType unkown = UnifiedWaferTask::TaskType::UNKNOWN;
    return unkown;
}


void FC::TaskManager::clearTasks()
{
    if (!tasks_.empty())
    {
        tasks_.clear();
        taskStatusMap_.clear(); // 清空任务状态映射
    }
}

std::vector<FC::UnifiedWaferTask> FC::TaskManager::getPendingTasks()
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<UnifiedWaferTask> result;

    std::copy_if(tasks_.begin(), tasks_.end(), std::back_inserter(result),
        [](const UnifiedWaferTask& task) {
        return task.status == UnifiedWaferTask::Status::QUEUED;
    });

    return result;
}

bool FC::TaskManager::hasPendingTasks()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return std::any_of(tasks_.begin(), tasks_.end(),
        [](const UnifiedWaferTask& task) {
        return task.status == UnifiedWaferTask::Status::QUEUED;
    });
}

bool FC::TaskManager::waitForTasks(int timeoutMs)
{
    std::unique_lock<std::mutex> lock(mutex_);
    return cv_.wait_for(lock, std::chrono::milliseconds(timeoutMs), [this] { return !tasks_.empty(); });//不为空返回true
}

bool FC::TaskManager::hasTasks()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return !tasks_.empty();
}

void FC::TaskManager::stop()
{
    stopped_ = true;
    cv_.notify_all(); // 通知所有线程退出
}

bool FC::TaskManager::isStopped() const
{
    return stopped_;
}
