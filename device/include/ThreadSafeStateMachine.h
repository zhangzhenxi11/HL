#pragma once
#include "TaskManager.h"
#include <functional>
#include <atomic>
using namespace FC;

class ThreadSafeStateMachine
{
public:
	ThreadSafeStateMachine(TaskManager& manager,UnifiedWaferTask::Location location, UnifiedWaferTask::TaskType taskType);

	void start();

	// 设置自定义条件检查器
	void setConditionChecker(std::function<bool(const UnifiedWaferTask&)> checker);

	// 设置任务启动器
	void setTaskStarter(std::function<void(UnifiedWaferTask&)> starter);

	// 设置任务完成检查器
	void setCompletionChecker(std::function<bool(const UnifiedWaferTask&)> checker);

	// 设置任务完成处理器
	void setCompletionHandler(std::function<void(UnifiedWaferTask&)> handler);


	void ThreadSafeStateMachine::processPhase(UnifiedWaferTask& task, std::function<void(UnifiedWaferTask&)> phaseStarter, UnifiedWaferTask::TaskType& TaskType);

	// 处理特定位置的任务
	void processLocationTasks(UnifiedWaferTask::Location location);

	void processEFEMPhase(UnifiedWaferTask& task);

	void processLLPhase(UnifiedWaferTask& task);

	void processEFEMReturnPhase(UnifiedWaferTask& task);

private:
	void processTasks();

private:
	TaskManager& taskManager_;
	UnifiedWaferTask::TaskType  taskType_;
	UnifiedWaferTask::Location location_;
	int taskIndex = 1;
	std::function<bool(const UnifiedWaferTask&)> conditionChecker_ = nullptr;
	std::function<void(UnifiedWaferTask&)> taskStarter_ = nullptr;
	std::function<bool(const UnifiedWaferTask&)> completionChecker_ = nullptr;
	std::function<void(UnifiedWaferTask&)> completionHandler_ = nullptr;




};