/**
* @file            fortrend_loadlock_openbox_command.h
* @brief           openbox command for loadlock
* @author		   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/LoadLock

#include "Kernel/kernel.h"
#include "Kernel/kernel_command.h"
#include "Kernel/kernel_abstract_subsystem.h"
#include "kernel/kernel_command_reject_exception.h"
#include "Kernel/kernel_log.h"
#include "kernel/Fortrend/fortrend_cassette_manager.h"
#include "Poco/Format.h"

#include "LoadLock/fortrend_loadlock_output_command.h"
#include "LoadLock/fortrend_loadlock_subsystem.h"


#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

namespace FC{
	/**
	* LoadLockOutputCommandPrivate
	*/
	class LoadLockOutputCommandPrivate{
	public:
		int chanel;
		bool state;
	};
	

	/**
	* LoadLockOutputCommand
	*/
	LoadLockOutputCommand::LoadLockOutputCommand(int chanel, bool state, KeyencePlcSubSystemHelper* hexHelper)
		:KeyencePlcCommandExecuter(hexHelper)
		, d(new LoadLockOutputCommandPrivate){
		d->chanel = chanel;
		d->state = state;
	};
	
	/**
	* return true if success else false.
	*/
	LoadLockOutputCommand::RunResult LoadLockOutputCommand::onRun() throw(KernelException){
		FortrendLoadLockSubsystem* sub = dynamic_cast<FortrendLoadLockSubsystem*>(getSubsystem());
		//
		if (!sub){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "subsystem type error.", this);
		}
		
		
		
		//get command configure
		std::shared_ptr<KernelConfiguration> command_config = sub->getConfigure()->createView(getName());

		//fill params
		int macroId = command_config->getInt("macroid", -1);
		int timeout = command_config->getInt("macrotimeout", -1);
		if (timeout > 0){
			
		}

		if (macroId == -1){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, Poco::format("macro: %s not defined", getName()), this);
		}
		IKernelCommand::RunResult ret = IKernelCommand::RunResult::RUN_OK;
		return ret;

	}



}