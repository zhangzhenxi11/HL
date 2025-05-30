/**
* @file            fortrend_pump_molecular_get_state_command.h
* @brief           molecular get state command for Pump
* @author		   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/Pump

#include "Poco/Format.h"

#include "Kernel/kernel.h"
#include "Kernel/kernel_command.h"
#include "kernel/kernel_command_reject_exception.h"
#include "Kernel/kernel_log.h"
#include "kernel/Fortrend/fortrend_cassette_manager.h"

#include "Pump/fortrend_pump_molecular_get_state_command.h"
#include "Pump/fortrend_pump_subsystem.h"

#include "TMCavity/fortrend_tm_cavity_subsystem.h"

#include <windows.h>

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif



namespace FC{



	/**
	* PumpMolecularGetStateCommand
	*/
	PumpMolecularGetStateCommand::PumpMolecularGetStateCommand(KYKYSubSystemHelper* helper)
		:KYKYCommandExecuter(helper){
		//setMessageName("MolecularGetState");
		//setDescription("align the pump");
	};


	/**
	* return true if success else false.
	*/
	PumpMolecularGetStateCommand::RunResult PumpMolecularGetStateCommand::onRun() throw(KernelException){
		FortrendPumpSubsystem* sub = dynamic_cast<FortrendPumpSubsystem*>(getSubsystem());
		//
		if (!sub){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "子系统类型错误", this);
		}
		//get command configure
		std::shared_ptr<KernelConfiguration> command_config = sub->getConfigure()->createView(getName());
		FortrendTMCavitySubsystem* tm = dynamic_cast<FortrendTMCavitySubsystem*>((sub->getKernel()->getKernelModule<FortrendTMCavitySubsystem>("TMCavity")).get());
		if (!tm){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "传输腔子系统类型错误", this);
		}


		IKernelCommand::RunResult ret = IKernelCommand::RunResult::RUN_OK;
		
		return ret;

	}



}