
/**
* @file            fortrend_cooling_cavity_update_command.h
* @brief           update command for cooling_cavity
* @author		   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/CoolingCavity

#include "Poco/Format.h"

#include "Kernel/kernel.h"
#include "Kernel/kernel_abstract_subsystem.h"
#include "Kernel/kernel_command.h"
#include "kernel/kernel_command_reject_exception.h"
#include "Kernel/kernel_log.h"
#include "kernel/Fortrend/fortrend_cassette_manager.h"


#include "CoolingCavity/fortrend_cooling_cavity_update_command.h"
#include "CoolingCavity/fortrend_cooling_cavity_subsystem.h"

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif


namespace FC{

	

	/**
	* CoolingCavityUpdateCommand
	*/
	CoolingCavityUpdateCommand::CoolingCavityUpdateCommand(KeyencePlcSubSystemHelper* hexHelper)
		:KeyencePlcCommandExecuter(hexHelper){
		
	};
	
	/**
	* return true if success else false.
	*/
	CoolingCavityUpdateCommand::RunResult CoolingCavityUpdateCommand::onRun() throw(KernelException){
		FortrendCoolingCavitySubsystem* sub = dynamic_cast<FortrendCoolingCavitySubsystem*>(getSubsystem());
		//
		if (!sub){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "子系统类型错误", this);
		}
		
		
		
		//get command configure
		std::shared_ptr<KernelConfiguration> command_config = sub->getConfigure()->createView(sub->getName());

		
		IKernelCommand::RunResult ret = IKernelCommand::RunResult::RUN_OK;
		if (ret == IKernelCommand::RunResult::RUN_OK)
		{

		}
		return ret;

	}



}