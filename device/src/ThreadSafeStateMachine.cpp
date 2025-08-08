#include "ThreadSafeStateMachine.h"
#include <iostream>

#include <windows.h>
ThreadSafeStateMachine::ThreadSafeStateMachine(TaskManager& manager, UnifiedWaferTask::Location location, UnifiedWaferTask::TaskType taskType)
	: taskManager_(manager), location_(location), taskType_(taskType)
{

}

void ThreadSafeStateMachine::start()
{
    //循环
    while (!taskManager_.isStopped())  //false
    {
        // 等待任务或超时
        if (taskManager_.waitForTasks()) {
            processTasks();
        }
        // 即使没有新任务也处理现有任务
        else {
            processTasks();
        }
        Sleep(500);
    }
}

void ThreadSafeStateMachine::setConditionChecker(std::function<bool(const UnifiedWaferTask&)> checker)
{
    conditionChecker_ = checker;
}

void ThreadSafeStateMachine::setTaskStarter(std::function<void(UnifiedWaferTask&)> starter)
{
    taskStarter_ = starter;
}

void ThreadSafeStateMachine::setCompletionChecker(std::function<bool(const UnifiedWaferTask&)> checker)
{
    completionChecker_ = checker;
}

void ThreadSafeStateMachine::setCompletionHandler(std::function<void(UnifiedWaferTask&)> handler)
{
    completionHandler_ = handler;
}

void ThreadSafeStateMachine::processPhase(UnifiedWaferTask& task, std::function<void(UnifiedWaferTask&)> phaseStarter, UnifiedWaferTask::TaskType& TaskType)
{
    if (task.status != UnifiedWaferTask::Status::COMPLETED)
    {
        if (taskStarter_)
        {
            phaseStarter(task); // 执行阶段特定的任务  
        }
    }
    else
    {
        //taskManager_.updateTaskStatus(task.taskId, UnifiedWaferTask::Status::COMPLETED, TaskType);
        //if (completionHandler_)
        //{
        //    completionHandler_(task); // 执行回调函数
        //    taskManager_.updateTaskStatus(task.taskId, UnifiedWaferTask::Status::COMPLETED, TaskType);
        //}
    }
}

void ThreadSafeStateMachine::processLocationTasks(UnifiedWaferTask::Location location)
{

}

void ThreadSafeStateMachine::processEFEMPhase(UnifiedWaferTask& task)
{
    auto nextType = UnifiedWaferTask::TaskType::LOADLOCK_TRANSFER;

    processPhase(task, taskStarter_, nextType);
   // processPhase(task, [this](UnifiedWaferTask& t) { taskStarter_(t); });
    //if (task.status != UnifiedWaferTask::Status::COMPLETED)
    //{
    //    if (taskStarter_)
    //    {
    //        taskStarter_(task);//执行EFEM回调函数
    //    }
    //}
    //else
    //{
    //    if (completionHandler_)
    //    {
    //        completionHandler_(task);//执行回调函数
    //        taskManager_.updateTaskStatus(task.taskId, UnifiedWaferTask::Status::COMPLETED, UnifiedWaferTask::TaskType::LOADLOCK_TRANSFER);
    //    }
    //}

}

void ThreadSafeStateMachine::processLLPhase(UnifiedWaferTask& task)
{
    auto nextType =UnifiedWaferTask::TaskType::ROBOT_PROCESS;

    processPhase(task, taskStarter_, nextType);

}

void ThreadSafeStateMachine::processEFEMReturnPhase(UnifiedWaferTask& task)
{
    
    auto nextType = UnifiedWaferTask::TaskType::LOADLOCK_RETURN;

    processPhase(task, taskStarter_, nextType);
}

void ThreadSafeStateMachine::processTasks()
{
    auto tasks = taskManager_.getTasksByLocation(location_); //找到该位置所有tasks  LP1, LP2, LLA, LLB, PM1, PM2, PM3, PM4 

    //UnifiedWaferTask::TaskType currentTaskType = tasks[taskIndex].taskType;

    int taskSize = tasks.size();

    while (taskIndex < taskSize)
    {
        auto task = tasks.at(taskIndex); //拿到任务队列中第一个任务task

        std::cout << "taskIndex:" << taskIndex << "taskId:" << task.taskId << std::endl;

        //找taskID 对应下的任务传输类型 
        UnifiedWaferTask::TaskType currentTaskType =  taskManager_.getTaskType(task.taskId);

        if (currentTaskType != UnifiedWaferTask::TaskType::UNKNOWN)
        {
            switch (taskType_)
            {
                case UnifiedWaferTask::TaskType::EFEM_TRANSFER: //只有lp1,lp2进入这个case
                {
                    // 处理当前task的EFEM阶段，直到任务完成
                    while (task.status != UnifiedWaferTask::Status::COMPLETED)
                    {
                        if (task.status == UnifiedWaferTask::Status::IN_ERROR)
                        {
                            break;
                        }
                        else
                        {
                            processEFEMPhase(task);
                        }
                    }

                    if (task.status == UnifiedWaferTask::Status::COMPLETED)
                    {
                        currentTaskType = UnifiedWaferTask::TaskType::LOADLOCK_TRANSFER;
                        taskType_ = currentTaskType;
                    }
                    else if (task.status == UnifiedWaferTask::Status::IN_ERROR)
                    {
                        std::cout << "END" << std::endl;
                    }
                    break;
                }
                case UnifiedWaferTask::TaskType::ROBOT_PROCESS://LLA,LLB,PM1,PM2,PM3,PM4进入
            {
                std::cout << "ROBOT_PROCESS" << std::endl;
                break;
            }
                case UnifiedWaferTask::TaskType::PM_PROCESS:
                {
                    std::cout << "PM_PROCESS" << std::endl;
                    break;
                }
                case UnifiedWaferTask::TaskType::LOADLOCK_RETURN:
            {
                std::cout << "LOADLOCK_RETURN" << std::endl;
                while (task.status != UnifiedWaferTask::Status::COMPLETED)
                {
                    if (task.status == UnifiedWaferTask::Status::IN_ERROR)
                    {
                        break;
                    }
                    else
                    {
                        processEFEMReturnPhase(task);
                    }
                }

                if (task.status == UnifiedWaferTask::Status::COMPLETED)
                {
                    currentTaskType = UnifiedWaferTask::TaskType::LOADLOCK_TRANSFER;
                    taskType_ = currentTaskType;
                }
                else if (task.status == UnifiedWaferTask::Status::IN_ERROR)
                {
                    std::cout << "END" << std::endl;
                }
                break;

            }
            case UnifiedWaferTask::TaskType::EFEM_RETURN:
            {


                std::cout << "EFEM_RETURN" << std::endl;
                break;
            }
                case UnifiedWaferTask::TaskType::LOADLOCK_TRANSFER:
            {
                std::cout << "LOADLOCK_TRANSFER" << std::endl;
                while (task.status != UnifiedWaferTask::Status::COMPLETED)
                {
                    if (task.status == UnifiedWaferTask::Status::IN_ERROR)
                    {
                        break;
                    }
                    else
                    {
                        processLLPhase(task);
                    }
                }
                if (task.status == UnifiedWaferTask::Status::COMPLETED)
                {
                    currentTaskType = UnifiedWaferTask::TaskType::LOADLOCK_TRANSFER;
                    taskType_ = currentTaskType;
                }
                else if (task.status == UnifiedWaferTask::Status::IN_ERROR)
                {
                    std::cout << "END" << std::endl;
                }
                currentTaskType = UnifiedWaferTask::TaskType::EFEM_TRANSFER;
                taskType_ = currentTaskType;
                break;
            }
                default:
                    break;
            }

            taskIndex++;
        }
        else
        {
            //error
            break;
        }

        Sleep(1000);
        
    }

}
