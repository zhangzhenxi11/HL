/**
* @file            fortrend_loadlock_clear_error_command.h
* @brief           clear error command for loadlock
* @author		   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/LoadLock

#include <Windows.h>

#include "Poco/Format.h"

#include "kernel/kernel.h"
#include "kernel/kernel_api.h"
#include "kernel/kernel_log.h"
#include "kernel/kernel_command_reject_exception.h"
#include "kernel/Fortrend/fortrend_cassette_manager.h"
#include "Kernel/Fortrend/robot_abstract_subsystem.h"
#include "Kernel/kernel_command_runner.h"
#include "Kernel/kernel_subsystem_update_command.h"

#include "LoadLock/fortrend_loadlock_defined.h"
#include "LoadLock/fortrend_loadlock_clear_error_command.h"
#include "LoadLock/fortrend_loadlock_subsystem.h"


#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

namespace FC{



	/**
	* LoadLockClearErrorCommand
	*/
	LoadLockClearErrorCommand::LoadLockClearErrorCommand(KeyencePlcSubSystemHelper* helper)
		:KeyencePlcCommandExecuter(helper){




	};


	/**
	* return true if success else false.
	*/
	LoadLockClearErrorCommand::RunResult LoadLockClearErrorCommand::onRun() throw(KernelException){
		FortrendLoadLockSubsystem* sub = dynamic_cast<FortrendLoadLockSubsystem*>(getSubsystem());
		if (!sub->getHasResetFlag())
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, Poco::format("%s:请先子系统复位", sub->getName()).c_str(), this);
		}
		logInform(getName().c_str(), "清除错误命令开始完成.");
		return IKernelCommand::RunResult::RUN_OK;

	}



}