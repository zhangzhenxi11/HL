/**
* @file            fortrend_pump_clear_error_command.h
* @brief           clear error command for sub
* @author		   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/Pump

#include "Poco/Format.h"

#include "kernel/kernel.h"
#include "kernel/kernel_api.h"
#include "kernel/kernel_log.h"
#include "kernel/kernel_command_reject_exception.h"
#include "kernel/Fortrend/fortrend_cassette_manager.h"
#include "Kernel/Fortrend/robot_abstract_subsystem.h"
#include "Kernel/kernel_command_runner.h"
#include "Kernel/kernel_subsystem_update_command.h"

#include "Pump/fortrend_pump_clear_error_command.h"
#include "Pump/fortrend_pump_subsystem.h"

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif


namespace FC{



	/**
	* PumpClearErrorCommand
	*/
	PumpClearErrorCommand::PumpClearErrorCommand(KeyencePlcSubSystemHelper* helper)
		:KeyencePlcCommandExecuter(helper){




	};


	/**
	* return true if success else false.
	*/
	PumpClearErrorCommand::RunResult PumpClearErrorCommand::onRun() throw(KernelException){
		FortrendPumpSubsystem* sub = dynamic_cast<FortrendPumpSubsystem*>(getSubsystem());
		
		//get command configure
		std::shared_ptr<KernelConfiguration> command_config = sub->getConfigure()->createView(getName());

		
		IKernelCommand::RunResult ret = IKernelCommand::RunResult::RUN_OK;
		sub->clearAlarm();
		return ret;
	}



}