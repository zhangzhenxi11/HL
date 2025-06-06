/**
* @file            fortrend_tm_cavity_reset_command.h
* @brief           reset command for tm_cavity
* @author		   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/TMCavity

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

#include "TMCavity/fortrend_tm_cavity_reset_command.h"
#include "TMCavity/fortrend_tm_cavity_subsystem.h"

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif


namespace FC{

	

	/**
	* TMCavityResetCommand
	*/
	TMCavityResetCommand::TMCavityResetCommand(KeyencePlcSubSystemHelper* helper)
		:KeyencePlcCommandExecuter(helper){	
		
	};


	/**
	* return true if success else false.
	*/
	TMCavityResetCommand::RunResult TMCavityResetCommand::onRun() throw(KernelException){
		FortrendTMCavitySubsystem* sub = dynamic_cast<FortrendTMCavitySubsystem*>(getSubsystem());
		if (!sub) throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "子系统类型错误", this);
		
		for (auto robot : sub->getRobots()){
			if (robot->getState() != IKernelSubSystem::State::SUB_NORMAL){
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_STATE_EXCEPTION, 
					Poco::format("机械手: %s 未处于正常状态.", robot->getName()), this);
			}
		}
		
		return IKernelCommand::RunResult::RUN_OK;;

	}



}