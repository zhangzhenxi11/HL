
/**
* @file            fortrend_tm_cavity_update_command.h
* @brief           update command for tm_cavity
* @author		   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/TMCavity

#include "Poco/Format.h"

#include "Kernel/kernel.h"
#include "Kernel/kernel_abstract_subsystem.h"
#include "Kernel/kernel_command.h"
#include "kernel/kernel_command_reject_exception.h"
#include "Kernel/kernel_log.h"
#include "kernel/Fortrend/fortrend_cassette_manager.h"


#include "TMCavity/fortrend_tm_cavity_update_command.h"
#include "TMCavity/fortrend_tm_cavity_subsystem.h"

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif


namespace FC{

	

	/**
	* UpdateCommand
	*/
	TMCavityUpdateCommand::TMCavityUpdateCommand(KeyencePlcSubSystemHelper* hexHelper)
		:KeyencePlcCommandExecuter(hexHelper){
		
	};
	
	/**
	* return true if success else false.
	*/
	TMCavityUpdateCommand::RunResult TMCavityUpdateCommand::onRun() throw(KernelException){
		FortrendTMCavitySubsystem* sub = dynamic_cast<FortrendTMCavitySubsystem*>(getSubsystem());
		//
		if (!sub){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "subsystem type error.", this);
		}
		
		return RunResult::RUN_OK;

	}



}