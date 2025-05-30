/**
* @file            fortrend_pump_output_command.h
* @brief           openbox command for sub
* @author		   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/Pump

#include "Kernel/kernel.h"
#include "Kernel/kernel_command.h"
#include "Kernel/kernel_abstract_subsystem.h"
#include "kernel/kernel_command_reject_exception.h"
#include "Kernel/kernel_log.h"
#include "kernel/Fortrend/fortrend_cassette_manager.h"
#include "Poco/Format.h"

#include "Pump/fortrend_pump_output_command.h"
#include "Pump/fortrend_pump_subsystem.h"


namespace FC{
	/**
	* PumpOutputCommandPrivate
	*/
	class PumpOutputCommandPrivate{
	public:
		int chanel;
		bool state;
	};
	

	/**
	* PumpOutputCommand
	*/
	PumpOutputCommand::PumpOutputCommand(int chanel, bool state, KeyencePlcSubSystemHelper* hexHelper)
		:KeyencePlcCommandExecuter(hexHelper)
		, d(new PumpOutputCommandPrivate){
		d->chanel = chanel;
		d->state = state;
	};
	
	/**
	* return true if success else false.
	*/
	PumpOutputCommand::RunResult PumpOutputCommand::onRun() throw(KernelException){
		FortrendPumpSubsystem* sub = dynamic_cast<FortrendPumpSubsystem*>(getSubsystem());
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
		if (ret == IKernelCommand::RunResult::RUN_OK)
		{

		}
		return ret;

	}



}