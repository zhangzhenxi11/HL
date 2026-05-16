#include "TaskManager.h"
#include <algorithm>
#include <iterator>
#include <unordered_map>

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

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

std::vector<FC::UnifiedWaferTask>& FC::TaskManager::getAllTasks()
{
    if (tasks_.size() > 0)
    {
        return tasks_;
    }
    else
    {
        std::vector<FC::UnifiedWaferTask> empty{};
        return empty;
    }  
}

std::vector<FC::UnifiedWaferTask>& FC::TaskManager::getAllWorksTasks()
{
    if (workTasks_.size() > 0)
    {
        return workTasks_;
    }
    else
    {
        std::vector<FC::UnifiedWaferTask> empty{};
        return empty;
    }
}

int FC::TaskManager::getAllTasksSize()
{
  return tasks_.size();
}

std::vector<int> FC::TaskManager::getAllTaskIDs()
{
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

    std::copy_if(tasks_.begin(), tasks_.end(), std::back_inserter(result),[](const UnifiedWaferTask& task)
    {
        return task.status == UnifiedWaferTask::Status::QUEUED;
    });

    return result;
}

FC::UnifiedWaferTask FC::TaskManager::getByIDFindTask(int taskID)
{
    auto it = std::find_if(tasks_.begin(), tasks_.end(), [taskID](const UnifiedWaferTask& t) { return t.taskId == taskID; });
    if (it != tasks_.end())
    {
        return (*it);
    }     
}

FC::UnifiedWaferTask FC::TaskManager::getRobotTaskInfo(int arm)
{
    auto it = std::find_if(tasks_.begin(), tasks_.end(), [arm](const UnifiedWaferTask& t) { 
        return t.arm == arm ; });
    if (it != tasks_.end())
    {
        return (*it);
	}
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
    std::vector<UnifiedWaferTask> result;

    auto& efemTasks = taskTypeStatusMap_[UnifiedWaferTask::UNKNOWN];
    for (int taskId : efemTasks[UnifiedWaferTask::UNKNOWN_PROGRESS])
    {
        auto it = std::find_if(tasks_.begin(), tasks_.end(), [taskId](const UnifiedWaferTask& t) { return t.taskId == taskId; });
        if (it != tasks_.end()) result.push_back(*it);
    }

    return result;
}

std::vector<FC::UnifiedWaferTask> FC::TaskManager::getEfemUnkownStatusLLATasks()
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<UnifiedWaferTask> result;

    auto& efemTasks = taskTypeStatusMap_[UnifiedWaferTask::UNKNOWN];
    for (int taskId : efemTasks[UnifiedWaferTask::UNKNOWN_PROGRESS])
    {
        auto it = std::find_if(tasks_.begin(), tasks_.end(), [taskId](const UnifiedWaferTask& t) { 
            return t.taskId == taskId && t.target == UnifiedWaferTask::Location::LLA;
           
        
        });
        if (it != tasks_.end()) result.push_back(*it);
    }

    return result;
}

std::vector<FC::UnifiedWaferTask> FC::TaskManager::getEfemUnkownStatusLLBTasks()
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<UnifiedWaferTask> result;

    auto& efemTasks = taskTypeStatusMap_[UnifiedWaferTask::UNKNOWN];
    for (int taskId : efemTasks[UnifiedWaferTask::UNKNOWN_PROGRESS])
    {
        auto it = std::find_if(tasks_.begin(), tasks_.end(), [taskId](const UnifiedWaferTask& t) {
            return t.taskId == taskId && t.target == UnifiedWaferTask::Location::LLB;


        });
        if (it != tasks_.end()) result.push_back(*it);
    }

    return result;
}

std::vector<FC::UnifiedWaferTask> FC::TaskManager::getEfemPendingTasks()
{
    //先获取TaskType，再找到此类型下所有QUEUED状态下的集合  
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<UnifiedWaferTask> result;

    auto& efemTasks = taskTypeStatusMap_[UnifiedWaferTask::EFEM_TRANSFER];
    for (int taskId : efemTasks[UnifiedWaferTask::QUEUED]) 
    {
        auto it = std::find_if(tasks_.begin(), tasks_.end(),[taskId](const UnifiedWaferTask& t) { return t.taskId == taskId; });
        if (it != tasks_.end()) result.push_back(*it);
    }

    return result;
}

std::vector<FC::UnifiedWaferTask> FC::TaskManager::getEfemCompletedTasks()
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<UnifiedWaferTask> result;

    auto& efemTasks = taskTypeStatusMap_[UnifiedWaferTask::EFEM_TRANSFER];
    for (int taskId : efemTasks[UnifiedWaferTask::COMPLETED]) 
    {
        auto it = std::find_if(tasks_.begin(), tasks_.end(),[taskId](const UnifiedWaferTask& t) { return t.taskId == taskId; });
        if (it != tasks_.end()) result.push_back(*it);
    }

    return result;
}

std::vector<FC::UnifiedWaferTask> FC::TaskManager::getEfemRuturnPendingTasks()
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<UnifiedWaferTask> result;

    auto& efemTasks = taskTypeStatusMap_[UnifiedWaferTask::EFEM_RETURN];
    for (int taskId : efemTasks[UnifiedWaferTask::QUEUED])
    {
        auto it = std::find_if(tasks_.begin(), tasks_.end(), [taskId](const UnifiedWaferTask& t) { return t.taskId == taskId; });
        if (it != tasks_.end()) result.push_back(*it);
    }
    return result;
}

std::vector<FC::UnifiedWaferTask> FC::TaskManager::getEfemRuturnCompletedTasks()
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<UnifiedWaferTask> result;

    auto& efemTasks = taskTypeStatusMap_[UnifiedWaferTask::EFEM_RETURN];
    for (int taskId : efemTasks[UnifiedWaferTask::COMPLETED])
    {
        auto it = std::find_if(tasks_.begin(), tasks_.end(), [taskId](const UnifiedWaferTask& t) { return t.taskId == taskId; });
        if (it != tasks_.end()) result.push_back(*it);
    }
    return result;
}

std::vector<FC::UnifiedWaferTask> FC::TaskManager::getLoadLockPendingTasks(std::string LLName)
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<UnifiedWaferTask> result;

   /* logInform("TaskManager","LLName:%s", LLName.c_str());*/
    auto loc = stringToLocation(LLName);

    // 获取所有在LoadLock上料的任务
    auto& robotTasks = taskTypeStatusMap_[UnifiedWaferTask::LOADLOCK_TRANSFER];
    for (int taskId : robotTasks[UnifiedWaferTask::QUEUED])
    {
        auto it = std::find_if(tasks_.begin(), tasks_.end(),[taskId, loc](const UnifiedWaferTask& t)
        {
            return t.taskId == taskId && t.target == loc;
        });
        if (it != tasks_.end()) 
        {
            result.push_back(*it);
        }
    }

    return result;
}

std::vector<FC::UnifiedWaferTask> FC::TaskManager::getLoadLockCompletedTasks(std::string LLName)
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<UnifiedWaferTask> result;
    auto loc = stringToLocation(LLName);

    // 获取所有在LoadLock上料完成任务
    auto& efemUpCompletedTasks = taskTypeStatusMap_[UnifiedWaferTask::LOADLOCK_TRANSFER];
    for (int taskId : efemUpCompletedTasks[UnifiedWaferTask::COMPLETED])
    {
        auto it = std::find_if(tasks_.begin(), tasks_.end(), [taskId, loc, LLName](const UnifiedWaferTask& t)
        {
            
            return t.taskId == taskId && t.target == loc;
        });
        if (it != tasks_.end())
        {
            //logInform("TaskManager", "LOADLOCK_TRANSFER/COMPLETED taskId:%d", (*it).taskId);
            result.push_back(*it);
        }
    }

    return result;
}

std::vector<FC::UnifiedWaferTask> FC::TaskManager::getLoadLockReturnPendingTasks(std::string LLName)
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<UnifiedWaferTask> result;
    auto loc = stringToLocation(LLName);

    // 获取所有在LoadLock待下料EFEM搬运的任务
    auto& efemReturnTasks = taskTypeStatusMap_[UnifiedWaferTask::LOADLOCK_RETURN];
    for (int taskId : efemReturnTasks[UnifiedWaferTask::QUEUED]) {
        auto it = std::find_if(tasks_.begin(), tasks_.end(), [taskId, loc, LLName](const UnifiedWaferTask& t)
        {
            
            return t.taskId == taskId && t.target == loc;
        });
        if (it != tasks_.end()) 
        {
            //logInform("TaskManager", "找到LOADLOCK_RETURN/QUEUED taskId:%d", (*it).taskId);
            result.push_back(*it);
        }
    }

    return result;
}

std::vector<FC::UnifiedWaferTask> FC::TaskManager::getLoadLockReturnCompletedTasks(std::string LLName)
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<UnifiedWaferTask> result;
    auto loc = stringToLocation(LLName);

    // 获取所有在LoadLock下料完成的任务
    auto& efemReturnCompletedTasks = taskTypeStatusMap_[UnifiedWaferTask::LOADLOCK_RETURN];
    for (int taskId : efemReturnCompletedTasks[UnifiedWaferTask::COMPLETED]) {
        auto it = std::find_if(tasks_.begin(), tasks_.end(), [taskId, loc, LLName](const UnifiedWaferTask& t)
        {
            return t.taskId == taskId && t.target == loc;
        });
        if (it != tasks_.end()) 
        {
            //logInform("TaskManager", "LOADLOCK_RETURN/COMPLETED taskId:%d", (*it).taskId);
            result.push_back(*it);
        }
    }

    return result;
}

std::vector<FC::UnifiedWaferTask> FC::TaskManager::getPMPendingTasks(std::string PM)
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<UnifiedWaferTask> result;
    auto loc = stringToLocation(PM);

    // 获取所有在PM等待处理的任务
    auto& pmTasks = taskTypeStatusMap_[UnifiedWaferTask::PM_PROCESS];
    for (int taskId : pmTasks[UnifiedWaferTask::QUEUED]) 
    {
        auto it = std::find_if(tasks_.begin(), tasks_.end(), [taskId, loc, PM](const UnifiedWaferTask& t)
        {
            
            return t.taskId == taskId && t.target_pm == loc;
        });
        if (it != tasks_.end())
        {
            //logInform("TaskManager", "PM_PROCESS/QUEUED taskId:%d", (*it).taskId);
            result.push_back(*it);
        }
    }
    return result;
}

std::vector<FC::UnifiedWaferTask> FC::TaskManager::getPMProcessTasks(std::string PM)
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<UnifiedWaferTask> result;
    auto loc = stringToLocation(PM);

    // 获取正在PM工艺的任务
    auto& pmTasks = taskTypeStatusMap_[UnifiedWaferTask::PM_PROCESS];
    for (int taskId : pmTasks[UnifiedWaferTask::IN_PROGRESS]) {
        auto it = std::find_if(tasks_.begin(), tasks_.end(), [taskId, loc, PM](const UnifiedWaferTask& t) {

            
            return t.taskId == taskId && t.target_pm == loc;
        });
        if (it != tasks_.end()) result.push_back(*it);
    }

    return result;
}

std::vector<FC::UnifiedWaferTask> FC::TaskManager::getPMCompletedTasks(std::string PM)
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<UnifiedWaferTask> result;
    auto loc = stringToLocation(PM);

    // 获取所有在PM完成工艺的任务
    auto& pmTasks = taskTypeStatusMap_[UnifiedWaferTask::PM_PROCESS];
    for (int taskId : pmTasks[UnifiedWaferTask::COMPLETED]) {
        auto it = std::find_if(tasks_.begin(), tasks_.end(),[taskId, loc,PM](const UnifiedWaferTask& t) {

            return t.taskId == taskId && t.target_pm == loc;
        });
        if (it != tasks_.end())
        {
            //logInform("TaskManager", "PM_PROCESS/QUEUED taskId:%d", (*it).taskId);
            result.push_back(*it);
        }
    }

    return result;
}

std::vector<FC::UnifiedWaferTask> FC::TaskManager::getLoadLockPendingTasks()
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<UnifiedWaferTask> result;

    // 获取所有在LoadLock上料的任务
    auto& robotTasks = taskTypeStatusMap_[UnifiedWaferTask::LOADLOCK_TRANSFER];
    for (int taskId : robotTasks[UnifiedWaferTask::QUEUED])
    {
        auto it = std::find_if(tasks_.begin(), tasks_.end(), [taskId](const UnifiedWaferTask& t)
        {
            return t.taskId == taskId;
        });
        if (it != tasks_.end())
        {
            result.push_back(*it);
        }
    }

    return result;
}

std::vector<FC::UnifiedWaferTask> FC::TaskManager::getLoadLockCompletedTasks()
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<UnifiedWaferTask> result;

    // 获取所有在LoadLock上料完成任务
    auto& efemUpCompletedTasks = taskTypeStatusMap_[UnifiedWaferTask::LOADLOCK_TRANSFER];
    for (int taskId : efemUpCompletedTasks[UnifiedWaferTask::COMPLETED])
    {
        auto it = std::find_if(tasks_.begin(), tasks_.end(), [taskId](const UnifiedWaferTask& t)
        {
            return t.taskId == taskId;
        });
        if (it != tasks_.end()) result.push_back(*it);
    }

    return result;
}

std::vector<FC::UnifiedWaferTask> FC::TaskManager::getLoadLockReturnPendingTasks()
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<UnifiedWaferTask> result;


    // 获取所有在LoadLock待下料EFEM搬运的任务
    auto& efemReturnTasks = taskTypeStatusMap_[UnifiedWaferTask::LOADLOCK_RETURN];
    for (int taskId : efemReturnTasks[UnifiedWaferTask::QUEUED]) {
        auto it = std::find_if(tasks_.begin(), tasks_.end(), [taskId](const UnifiedWaferTask& t)
        {
          
            return t.taskId == taskId;
        });
        if (it != tasks_.end()) result.push_back(*it);
    }

    return result;
}

std::vector<FC::UnifiedWaferTask> FC::TaskManager::getLoadLockReturnCompletedTasks()
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<UnifiedWaferTask> result;
  

    // 获取所有在LoadLock下料完成的任务
    auto& efemReturnCompletedTasks = taskTypeStatusMap_[UnifiedWaferTask::LOADLOCK_RETURN];
    for (int taskId : efemReturnCompletedTasks[UnifiedWaferTask::COMPLETED]) {
        auto it = std::find_if(tasks_.begin(), tasks_.end(), [taskId](const UnifiedWaferTask& t)
        {
            return t.taskId == taskId;
        });
        if (it != tasks_.end()) result.push_back(*it);
    }

    return result;
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



