#include "TaskManager.h"
#include <algorithm>
#include <iterator>
#include <unordered_map>

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

namespace
{
    using TaskStatusMap = std::map<FC::UnifiedWaferTask::TaskType, std::map<FC::UnifiedWaferTask::Status, std::vector<int>>>;
    const std::vector<int> kEmptyTaskIds;
    const std::pair<FC::UnifiedWaferTask::TaskType, FC::UnifiedWaferTask::Status> kUnknownTaskStatus(
        FC::UnifiedWaferTask::TaskType::UNKNOWN,
        FC::UnifiedWaferTask::Status::UNKNOWN_PROGRESS);

    FC::UnifiedWaferTask makeUnknownTask()
    {
        FC::UnifiedWaferTask task{};
        task.taskId = -1;
        task.taskType = FC::UnifiedWaferTask::TaskType::UNKNOWN;
        task.status = FC::UnifiedWaferTask::Status::UNKNOWN_PROGRESS;
        task.source = FC::UnifiedWaferTask::Location::LP1;
        task.target = FC::UnifiedWaferTask::Location::LP1;
        task.target_pm = FC::UnifiedWaferTask::Location::PM1;
        task.realTimePosition = FC::UnifiedWaferTask::Location::LP1;
        task.sourceSlot = 0;
        task.targetSlot = 0;
        task.targetFeedingSlot = 0;
        task.targetBlankingSlot = 0;
        task.arm = -1;
        task.Aligner_status = FC::UnifiedWaferTask::ALIGNER_READY;
        task.selectPmEnableList = { 0, 0, 0, 0 };
        task.pm1Enabled = false;
        task.pm2Enabled = false;
        task.pm3Enabled = false;
        task.pm4Enabled = false;
        return task;
    }

    const std::vector<int>* findTaskIds(
        const TaskStatusMap& taskTypeStatusMap,
        FC::UnifiedWaferTask::TaskType type,
        FC::UnifiedWaferTask::Status status)
    {
        const auto typeIt = taskTypeStatusMap.find(type);
        if (typeIt == taskTypeStatusMap.end())
        {
            return &kEmptyTaskIds;
        }

        const auto statusIt = typeIt->second.find(status);
        if (statusIt == typeIt->second.end())
        {
            return &kEmptyTaskIds;
        }

        return &statusIt->second;
    }

    template <typename Predicate>
    std::vector<FC::UnifiedWaferTask> collectTasksByIds(
        const std::vector<FC::UnifiedWaferTask>& tasks,
        const std::vector<int>* taskIds,
        Predicate predicate)
    {
        std::vector<FC::UnifiedWaferTask> result;
        if (taskIds == nullptr)
        {
            return result;
        }

        for (int taskId : *taskIds)
        {
            auto it = std::find_if(tasks.begin(), tasks.end(), [taskId, &predicate](const FC::UnifiedWaferTask& task) {
                return task.taskId == taskId && predicate(task);
            });
            if (it != tasks.end())
            {
                result.push_back(*it);
            }
        }

        return result;
    }
}

FC::TaskManager& FC::TaskManager::getInstance()
{     
    static FC::TaskManager instance;
    return instance;
    
}

void FC::TaskManager::addTask(const FC::UnifiedWaferTask& task)
{
    std::unique_lock<std::mutex> lock(mutex_);
    UnifiedWaferTask newTask = task;
    newTask.taskId = nextTaskId_++; 
    newTask.createdAt = std::chrono::system_clock::now();
    tasks_.push_back(newTask);

    taskStatusMap_[newTask.taskId] = { newTask.taskType, newTask.status }; // 初始化任务状态

    updateTaskMaps(newTask.taskId, newTask.taskType, newTask.status);

    lock.unlock();

    cv_.notify_one();
}

void FC::TaskManager::popTask(const UnifiedWaferTask& task)
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (tasks_.size() > 0)
    {
        for (auto task : tasks_)
        {
            //auto it = std::find_if(tasks_.begin(), tasks_.end(), [task](const UnifiedWaferTask& t) { return t.taskId == task.taskId; });
            //if (it != tasks_.end())
            //{
            //    workTasks_.pop_back(*it);
            //}
        }
    }

    lock.unlock();

}

void FC::TaskManager::addWorkTask(const UnifiedWaferTask& task)
{
    std::unique_lock<std::mutex> lock(mutex_);
    //找到tasks_中的taskId 的task
    if (tasks_.size() > 0)
    {
        for (auto task : tasks_)
        {
            auto it = std::find_if(tasks_.begin(), tasks_.end(), [task](const UnifiedWaferTask& t) { return t.taskId == task.taskId; });
            if (it != tasks_.end()) 
            {
                workTasks_.push_back(*it);
            }
        }
    }

    taskStatusMap_[task.taskId] = { task.taskType, task.status }; // 初始化任务状态

    updateTaskMaps(task.taskId, task.taskType, task.status);

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

std::vector<FC::UnifiedWaferTask> FC::TaskManager::getTasksByLocation(std::vector<UnifiedWaferTask> tasks, UnifiedWaferTask::Location location)
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<UnifiedWaferTask> result;

    for (const auto& task : tasks) {
        if (task.source == location || task.target == location) {
            result.push_back(task);
        }
    }
    return result;
}

void FC::TaskManager::updateTaskStatus(int taskId, UnifiedWaferTask::TaskType newTaskType,UnifiedWaferTask::Status newStatus)
{
    std::lock_guard<std::mutex> lock(mutex_);

    for (auto& task : tasks_) 
    {
        if (task.taskId == taskId)
        {
            // 保存旧类型和状态用于更新映射
            auto oldType = task.taskType;
            auto oldStatus = task.status;

            task.status = newStatus;
            task.taskType = newTaskType;

            taskStatusMap_[taskId] = { newTaskType, newStatus }; // 更新任务状态

            if (newStatus == UnifiedWaferTask::Status::IN_PROGRESS)
            {
                task.startedAt = std::chrono::system_clock::now();
            }

            else if (newStatus == UnifiedWaferTask::Status::COMPLETED)
            {
                task.completedAt = std::chrono::system_clock::now();
            }
            //打标签
            if (task.taskType == UnifiedWaferTask::TaskType::LOADLOCK_TRANSFER)
            {
                if (task.status == UnifiedWaferTask::Status::QUEUED)
                {
                    logInform("TaskManager", "task：%d 上料到loadlock标签.", taskId);
                    task.isLoadingInPlace = true;
                }   
            }
            else if(task.taskType == UnifiedWaferTask::TaskType::LOADLOCK_RETURN)
            {
                if (task.status == UnifiedWaferTask::Status::COMPLETED)
                {
                    logInform("TaskManager", "task：%d 下料到loadlock标签.", taskId);
                    task.isLoadingInPlace = false;
                }
            }
            // 更新映射关系
            updateTaskMaps(taskId, newTaskType, newStatus);
            // 日志记录
            logInform("TaskManager", "Updated task %d: %s/%s -> %s/%s",
                taskId,
                task.typeToString(oldType).c_str(),
                task.statusToString(oldStatus).c_str(),
                task.typeToString(newTaskType).c_str(),
                task.statusToString(newStatus).c_str());

            break;
        }
    }
}

std::pair<FC::UnifiedWaferTask::TaskType, FC::UnifiedWaferTask::Status> FC::TaskManager::getTaskStatusAndType(int taskId)
{
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = taskStatusMap_.find(taskId);
    if (it != taskStatusMap_.end()) {
        return it->second;
    }

    return kUnknownTaskStatus;
    
}

FC::UnifiedWaferTask::TaskType FC::TaskManager::getTaskType(int taskId)
{
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = taskStatusMap_.find(taskId);

    if (it != taskStatusMap_.end())
    {
        return it->second.first;
    }
    return UnifiedWaferTask::TaskType::UNKNOWN;
}

std::vector<FC::UnifiedWaferTask> FC::TaskManager::getAllTasks()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return tasks_;
}

std::vector<FC::UnifiedWaferTask> FC::TaskManager::getAllWorksTasks()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return workTasks_;
}

int FC::TaskManager::getAllTasksSize()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return static_cast<int>(tasks_.size());
}

std::vector<int> FC::TaskManager::getAllTaskIDs()
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<int> allIds;

    for (auto& task : tasks_)
    {
        if (task.taskId < 25 || task.taskId > 0)
        {
            allIds.push_back(task.taskId);
        }
    }
    return allIds;
}


void FC::TaskManager::clearTasks()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!tasks_.empty())
    {
        tasks_.clear();
        workTasks_.clear();
        taskStatusMap_.clear(); // 清空任务状态映射
        taskTypeStatusMap_.clear();
    }
}

std::vector<FC::UnifiedWaferTask> FC::TaskManager::getPendingTasks()
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<UnifiedWaferTask> result;

    std::copy_if(tasks_.begin(), tasks_.end(), std::back_inserter(result),[](const UnifiedWaferTask& task)
    {
        return task.status == UnifiedWaferTask::Status::QUEUED;
    });

    return result;
}

FC::UnifiedWaferTask FC::TaskManager::getByIDFindTask(int taskID)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = std::find_if(tasks_.begin(), tasks_.end(), [taskID](const UnifiedWaferTask& t) { return t.taskId == taskID; });
    if (it != tasks_.end())
    {
        return (*it);
    }
    return makeUnknownTask();
}

FC::UnifiedWaferTask FC::TaskManager::getRobotTaskInfo(int arm)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = std::find_if(tasks_.begin(), tasks_.end(), [arm](const UnifiedWaferTask& t) { 
        return t.arm == arm ; });
    if (it != tasks_.end())
    {
        return (*it);
	}
    return makeUnknownTask();
}

bool FC::TaskManager::hasPendingTasks()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return std::any_of(tasks_.begin(), tasks_.end(),
        [](const UnifiedWaferTask& task) {
        return task.status == UnifiedWaferTask::Status::QUEUED;
    });
}

std::vector<FC::UnifiedWaferTask> FC::TaskManager::getTasksByTypeAndStatus(UnifiedWaferTask::TaskType type, UnifiedWaferTask::Status status)
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<UnifiedWaferTask> result;

    // 使用 taskTypeStatusMap_ 进行高效查询
    auto typeIt = taskTypeStatusMap_.find(type);
    if (typeIt != taskTypeStatusMap_.end()) 
    {
        auto statusIt = typeIt->second.find(status);
        if (statusIt != typeIt->second.end())
        {
            const auto& taskIds = statusIt->second;
            for (int taskId : taskIds) 
            {
                // 在 tasks_ 中查找任务
                for (const auto& task : tasks_) 
                {
                    if (task.taskId == taskId)
                    {
                        result.push_back(task);
                        break;
                    }
                }
            }
        }
    }
    return result;
}

bool FC::TaskManager::getByTypeAndStatus(UnifiedWaferTask::TaskType type, UnifiedWaferTask::Status status)
{
    std::lock_guard<std::mutex> lock(mutex_);

    // 使用 taskTypeStatusMap_ 进行高效查询
    auto typeIt = taskTypeStatusMap_.find(type);
    if (typeIt != taskTypeStatusMap_.end())
    {
        auto statusIt = typeIt->second.find(status);
        if (statusIt != typeIt->second.end())
        {
            const auto& taskIds = statusIt->second;
            for (int taskId : taskIds)
            {
                // 在 tasks_ 中查找任务并检查位置
                for (const auto& task : tasks_)
                {
                    if (task.taskId == taskId)
                    {
                        break;
                        return true;
                    }
                }
            }
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
    return false;
}

std::vector< FC::UnifiedWaferTask> FC::TaskManager::getTasksByTypeStatusAndLocation(UnifiedWaferTask::TaskType type, 
    UnifiedWaferTask::Status status,
    UnifiedWaferTask::Location location)
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<UnifiedWaferTask> result;

    // 使用 taskTypeStatusMap_ 进行高效查询
    auto typeIt = taskTypeStatusMap_.find(type);
    if (typeIt != taskTypeStatusMap_.end()) 
    {
        auto statusIt = typeIt->second.find(status);
        if (statusIt != typeIt->second.end())
        {
            const auto& taskIds = statusIt->second;
            for (int taskId : taskIds)
            {
                // 在 tasks_ 中查找任务并检查位置
                for (const auto& task : tasks_)
                {
                    if (task.taskId == taskId && task.source == location)
                    {
                        result.push_back(task);
                        break;
                    }
                }
            }
        }
    }
    return result;
}

bool FC::TaskManager::detectionHasNoInitialTypeTasks()
{
    if(tasks_.size() > 0)
    {
        auto it = std::find_if(tasks_.begin(), tasks_.end(), [](const UnifiedWaferTask& t) {
            return t.taskType != UnifiedWaferTask::UNKNOWN;
        });
        if (it != tasks_.end())
        {//找到
            return true;
        }
    }
    else
    {
        logInform("TaskManager", "current no task!",);
        return false;
    }
}


std::vector<FC::UnifiedWaferTask> FC::TaskManager::getEfemUnkownStatusTasks()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return collectTasksByIds(tasks_, findTaskIds(taskTypeStatusMap_, UnifiedWaferTask::UNKNOWN, UnifiedWaferTask::UNKNOWN_PROGRESS),
        [](const UnifiedWaferTask&) { return true; });
}

std::vector<FC::UnifiedWaferTask> FC::TaskManager::getEfemUnkownStatusLLATasks()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return collectTasksByIds(tasks_, findTaskIds(taskTypeStatusMap_, UnifiedWaferTask::UNKNOWN, UnifiedWaferTask::UNKNOWN_PROGRESS),
        [](const UnifiedWaferTask& task) { return task.target == UnifiedWaferTask::Location::LLA; });
}

std::vector<FC::UnifiedWaferTask> FC::TaskManager::getEfemUnkownStatusLLBTasks()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return collectTasksByIds(tasks_, findTaskIds(taskTypeStatusMap_, UnifiedWaferTask::UNKNOWN, UnifiedWaferTask::UNKNOWN_PROGRESS),
        [](const UnifiedWaferTask& task) { return task.target == UnifiedWaferTask::Location::LLB; });
}

std::vector<FC::UnifiedWaferTask> FC::TaskManager::getEfemPendingTasks()
{
    //先获取TaskType，再找到此类型下所有QUEUED状态下的集合  
    std::lock_guard<std::mutex> lock(mutex_);
    return collectTasksByIds(tasks_, findTaskIds(taskTypeStatusMap_, UnifiedWaferTask::EFEM_TRANSFER, UnifiedWaferTask::QUEUED),
        [](const UnifiedWaferTask&) { return true; });
}

std::vector<FC::UnifiedWaferTask> FC::TaskManager::getEfemCompletedTasks()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return collectTasksByIds(tasks_, findTaskIds(taskTypeStatusMap_, UnifiedWaferTask::EFEM_TRANSFER, UnifiedWaferTask::COMPLETED),
        [](const UnifiedWaferTask&) { return true; });
}

std::vector<FC::UnifiedWaferTask> FC::TaskManager::getEfemRuturnPendingTasks()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return collectTasksByIds(tasks_, findTaskIds(taskTypeStatusMap_, UnifiedWaferTask::EFEM_RETURN, UnifiedWaferTask::QUEUED),
        [](const UnifiedWaferTask&) { return true; });
}

std::vector<FC::UnifiedWaferTask> FC::TaskManager::getEfemRuturnCompletedTasks()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return collectTasksByIds(tasks_, findTaskIds(taskTypeStatusMap_, UnifiedWaferTask::EFEM_RETURN, UnifiedWaferTask::COMPLETED),
        [](const UnifiedWaferTask&) { return true; });
}

std::vector<FC::UnifiedWaferTask> FC::TaskManager::getLoadLockPendingTasks(std::string LLName)
{
    std::lock_guard<std::mutex> lock(mutex_);
   /* logInform("TaskManager","LLName:%s", LLName.c_str());*/
    auto loc = stringToLocation(LLName);
    return collectTasksByIds(tasks_, findTaskIds(taskTypeStatusMap_, UnifiedWaferTask::LOADLOCK_TRANSFER, UnifiedWaferTask::QUEUED),
        [loc](const UnifiedWaferTask& task) { return task.target == loc; });
}

std::vector<FC::UnifiedWaferTask> FC::TaskManager::getLoadLockCompletedTasks(std::string LLName)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto loc = stringToLocation(LLName);
    return collectTasksByIds(tasks_, findTaskIds(taskTypeStatusMap_, UnifiedWaferTask::LOADLOCK_TRANSFER, UnifiedWaferTask::COMPLETED),
        [loc](const UnifiedWaferTask& task) { return task.target == loc; });
}

std::vector<FC::UnifiedWaferTask> FC::TaskManager::getLoadLockReturnPendingTasks(std::string LLName)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto loc = stringToLocation(LLName);
    return collectTasksByIds(tasks_, findTaskIds(taskTypeStatusMap_, UnifiedWaferTask::LOADLOCK_RETURN, UnifiedWaferTask::QUEUED),
        [loc](const UnifiedWaferTask& task) { return task.egressLoadLock == loc; });
}

std::vector<FC::UnifiedWaferTask> FC::TaskManager::getLoadLockReturnCompletedTasks(std::string LLName)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto loc = stringToLocation(LLName);
    return collectTasksByIds(tasks_, findTaskIds(taskTypeStatusMap_, UnifiedWaferTask::LOADLOCK_RETURN, UnifiedWaferTask::COMPLETED),
        [loc](const UnifiedWaferTask& task) { return task.egressLoadLock == loc; });
}

std::vector<FC::UnifiedWaferTask> FC::TaskManager::getPMPendingTasks(std::string PM)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto loc = stringToLocation(PM);
    return collectTasksByIds(tasks_, findTaskIds(taskTypeStatusMap_, UnifiedWaferTask::PM_PROCESS, UnifiedWaferTask::QUEUED),
        [loc](const UnifiedWaferTask& task) { return task.target_pm == loc; });
}

std::vector<FC::UnifiedWaferTask> FC::TaskManager::getPMProcessTasks(std::string PM)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto loc = stringToLocation(PM);
    return collectTasksByIds(tasks_, findTaskIds(taskTypeStatusMap_, UnifiedWaferTask::PM_PROCESS, UnifiedWaferTask::IN_PROGRESS),
        [loc](const UnifiedWaferTask& task) { return task.target_pm == loc; });
}

std::vector<FC::UnifiedWaferTask> FC::TaskManager::getPMCompletedTasks(std::string PM)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto loc = stringToLocation(PM);
    return collectTasksByIds(tasks_, findTaskIds(taskTypeStatusMap_, UnifiedWaferTask::PM_PROCESS, UnifiedWaferTask::COMPLETED),
        [loc](const UnifiedWaferTask& task) { return task.target_pm == loc; });
}

std::vector<FC::UnifiedWaferTask> FC::TaskManager::getLoadLockPendingTasks()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return collectTasksByIds(tasks_, findTaskIds(taskTypeStatusMap_, UnifiedWaferTask::LOADLOCK_TRANSFER, UnifiedWaferTask::QUEUED),
        [](const UnifiedWaferTask&) { return true; });
}

std::vector<FC::UnifiedWaferTask> FC::TaskManager::getLoadLockCompletedTasks()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return collectTasksByIds(tasks_, findTaskIds(taskTypeStatusMap_, UnifiedWaferTask::LOADLOCK_TRANSFER, UnifiedWaferTask::COMPLETED),
        [](const UnifiedWaferTask&) { return true; });
}

std::vector<FC::UnifiedWaferTask> FC::TaskManager::getLoadLockReturnPendingTasks()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return collectTasksByIds(tasks_, findTaskIds(taskTypeStatusMap_, UnifiedWaferTask::LOADLOCK_RETURN, UnifiedWaferTask::QUEUED),
        [](const UnifiedWaferTask&) { return true; });
}

std::vector<FC::UnifiedWaferTask> FC::TaskManager::getLoadLockReturnCompletedTasks()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return collectTasksByIds(tasks_, findTaskIds(taskTypeStatusMap_, UnifiedWaferTask::LOADLOCK_RETURN, UnifiedWaferTask::COMPLETED),
        [](const UnifiedWaferTask&) { return true; });
}

bool FC::TaskManager::waitForTasks(int timeoutMs)
{
    std::unique_lock<std::mutex> lock(mutex_);
    return cv_.wait_for(lock, std::chrono::milliseconds(timeoutMs), [this] { return !tasks_.empty(); });//不为空返回true
}

bool FC::TaskManager::waitLLBForTasks(int timeoutMs)
{
    std::unique_lock<std::mutex> lock(mutex_);
    return cv_.wait_for(lock, std::chrono::milliseconds(timeoutMs), [this] { return !tasks_.size() >= 2; });//不为空返回true
}

bool FC::TaskManager::hasTasks()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return !tasks_.empty();
}

void FC::TaskManager::stop()
{
     stopped_ = true;
}

void FC::TaskManager::start()
{
    stopped_ = false;
    cv_.notify_all();
}

bool FC::TaskManager::isStopped() const
{
    return stopped_;
}

void FC::TaskManager::setLoadingInPlaceFlag(int taskID,bool flag)
{
    auto task = getByIDFindTask(taskID);
    task.isLoadingInPlace = flag; //true: 上料到loadLock完成， false: 下料到loadLock完成
}
bool FC::TaskManager::getLoadingInPlaceFlag(int taskID)
{
    auto task = getByIDFindTask(taskID);
    return  task.isLoadingInPlace;
}

bool FC::TaskManager::CollectionPassedThroughLL(const std::string& LLName)
{
    //std::lock_guard<std::mutex> lock(mutex_);
    std::vector<UnifiedWaferTask> result;
    auto loc = stringToLocation(LLName);

    result = getTasksByLocation(loc); //LLA/LLB
    for (auto task : result)
    {
        if (task.isLoadingInPlace)
        {
            return true; 
            break;
        }
    }
    return false;
}

bool FC::TaskManager::hasLoadLockLowerPriorityReturn(const std::string& LLName)
{
    std::lock_guard<std::mutex> lock(mutex_);
    const auto loc = stringToLocation(LLName);

    return std::any_of(tasks_.begin(), tasks_.end(), [loc](const UnifiedWaferTask& task) {
        if (task.egressLoadLock != loc || task.targetBlankingSlot != 1)
        {
            return false;
        }

        const bool loadLockReturnReady =
            task.taskType == UnifiedWaferTask::TaskType::LOADLOCK_RETURN &&
            task.status == UnifiedWaferTask::Status::COMPLETED;

        const bool efemReturnWaiting =
            task.taskType == UnifiedWaferTask::TaskType::EFEM_RETURN &&
            task.status != UnifiedWaferTask::Status::COMPLETED &&
            task.status != UnifiedWaferTask::Status::UNKNOWN_PROGRESS;

		return loadLockReturnReady || efemReturnWaiting; //如果有LoadLock下料完成的任务，或者有EFEM搬回任务正在进行中（不论是否完成），都认为存在优先级较低的下料任务
    });
}

bool FC::TaskManager::hasEfemUnloadInProgress(const std::string& LLName)
{
    std::lock_guard<std::mutex> lock(mutex_);
    const auto loc = stringToLocation(LLName);

    return std::any_of(tasks_.begin(), tasks_.end(), [loc](const UnifiedWaferTask& task) {
        return task.egressLoadLock == loc &&
            task.taskType == UnifiedWaferTask::TaskType::EFEM_RETURN &&
            task.status != UnifiedWaferTask::Status::COMPLETED &&
            task.status != UnifiedWaferTask::Status::UNKNOWN_PROGRESS;
    });
}

std::vector<FC::UnifiedWaferTask> FC::TaskManager::getEfemRuturnCompletedTasksBySource(UnifiedWaferTask::Location source)
{
    std::lock_guard<std::mutex> lock(mutex_);
    return collectTasksByIds(tasks_, findTaskIds(taskTypeStatusMap_, UnifiedWaferTask::EFEM_RETURN, UnifiedWaferTask::COMPLETED),
        [source](const UnifiedWaferTask& task) { return task.source == source; });
}

// 判断是否可以重置EFEM搬回任务，条件是：同一来源的所有任务要么是未知状态，要么是已完成的搬回任务，并且至少有一个已完成的搬回任务
bool FC::TaskManager::canResetEfemReturnCompletedTasks(UnifiedWaferTask::Location source)
{
    std::lock_guard<std::mutex> lock(mutex_);
    bool hasCompletedReturnTask = false;

    for (const auto& task : tasks_)
    {
        if (task.source != source)
        {
            continue;
        }

        const bool isUnknownTask =
            task.taskType == UnifiedWaferTask::TaskType::UNKNOWN &&
            task.status == UnifiedWaferTask::Status::UNKNOWN_PROGRESS;
        const bool isCompletedReturnTask =
            task.taskType == UnifiedWaferTask::TaskType::EFEM_RETURN &&
            task.status == UnifiedWaferTask::Status::COMPLETED;

        if (isCompletedReturnTask)
        {
            hasCompletedReturnTask = true;
            continue;
        }

        if (!isUnknownTask)
        {
            return false;
        }
    }

    return hasCompletedReturnTask;
}

// 判断是否存在除 已完成搬回任务以外的其他未完成任务
bool FC::TaskManager::hasInFlightTasksExceptCompletedEfemReturn()
{
    std::lock_guard<std::mutex> lock(mutex_);

    return std::any_of(tasks_.begin(), tasks_.end(), [](const UnifiedWaferTask& task) {
        const bool isUnknownTask =
            task.taskType == UnifiedWaferTask::TaskType::UNKNOWN &&
            task.status == UnifiedWaferTask::Status::UNKNOWN_PROGRESS;

        const bool isCompletedReturnTask =
            task.taskType == UnifiedWaferTask::TaskType::EFEM_RETURN &&
            task.status == UnifiedWaferTask::Status::COMPLETED;

        return !isUnknownTask && !isCompletedReturnTask;
    });
}

FC::UnifiedWaferTask::Location FC::TaskManager::stringToLocation(const std::string& locStr)
{
    static const std::unordered_map<std::string, UnifiedWaferTask::Location> locMap = {
        {"LP1", UnifiedWaferTask::Location::LP1},
        {"LP2", UnifiedWaferTask::Location::LP2},
        {"LLA", UnifiedWaferTask::Location::LLA},
        {"LLB", UnifiedWaferTask::Location::LLB},
        {"PM1", UnifiedWaferTask::Location::PM1},
        {"PM2", UnifiedWaferTask::Location::PM2},
        {"PM3", UnifiedWaferTask::Location::PM3},
        {"PM4", UnifiedWaferTask::Location::PM4}
    };

    auto it = locMap.find(locStr);
    return (it != locMap.end()) ? it->second : UnifiedWaferTask::Location::LP1;
}

void FC::TaskManager::lessTaskIdSortAlgorithm()
{
    std::sort(tasks_.begin(), tasks_.end(),[](const UnifiedWaferTask& a, const UnifiedWaferTask& b) {
        return a.taskId < b.taskId;
    });
}

void FC::TaskManager::largeTaskIdSortAlgorithm()
{
    std::sort(tasks_.begin(), tasks_.end(), [](const UnifiedWaferTask& a, const UnifiedWaferTask& b) {
        return a.taskId > b.taskId;
    });

}


void FC::TaskManager::updateTaskMaps(int taskId, UnifiedWaferTask::TaskType type, UnifiedWaferTask::Status status)
{
    // 移除旧映射
    for (auto& typeEntry : taskTypeStatusMap_)
    {
        for (auto& statusEntry : typeEntry.second)
        {
            auto& taskIds = statusEntry.second;
            auto newEnd = std::remove(taskIds.begin(), taskIds.end(), taskId);
            taskIds.erase(newEnd, taskIds.end());
        }
    }

    // 2. 确保新类型条目存在
    if (taskTypeStatusMap_.find(type) == taskTypeStatusMap_.end())
    {
        taskTypeStatusMap_[type] = std::map<UnifiedWaferTask::Status, std::vector<int>>();
    }

    // 3. 确保新状态条目存在
    auto& statusMap = taskTypeStatusMap_[type];
    if (statusMap.find(status) == statusMap.end())
    {
        statusMap[status] = std::vector<int>();
    }

    // 4. 添加任务ID到新映射
    auto& taskIDList = statusMap[status]; //vector taskID

    // 避免重复添加
    if (std::find(taskIDList.begin(), taskIDList.end(), taskId) == taskIDList.end())
    {
        taskIDList.push_back(taskId);

        // 5. 查找并更新任务状态（可选）
        auto it = std::find_if(tasks_.begin(), tasks_.end(),[taskId](const UnifiedWaferTask& t) { return t.taskId == taskId; });

        if (it != tasks_.end()) 
        {
            it->taskType = type;
            it->status = status;
            logInform("TaskManager", "TaskID:%d, taskType:%s,taskStatus:%s", taskId, (*it).typeToString(type).c_str(), (*it).statusToString(status).c_str());
        }

        //排序算法
        lessTaskIdSortAlgorithm();
    }
}   



