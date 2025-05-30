/**
* @file            fortrend_pump_reset_command.h
* @brief           reset command for Pump
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Subsystem/Pump

#ifndef FORTREND_PUMP_RESET_COMMAND_INCLUDE_
#define FORTREND_PUMP_RESET_COMMAND_INCLUDE_

#include "kernel/kernel_subsystem_command.h"
#include "Kernel/kernel_subsystem_reset_command.h"

#include "KeyencePLC/keyence_plc_command_executer.h"
#include "KeyencePLC/keyence_plc_subsystem_helper.h"

namespace FC{



	/**
	* @brief  reset command for pump
	*/
	class  PumpResetCommand : public  KernelSubsystemResetCommand, public KeyencePlcCommandExecuter{

	public:
		DECLARE_PTR(PumpResetCommand)
		PumpResetCommand(KeyencePlcSubSystemHelper* hexHelper);
		virtual std::string getName()const override { return "Reset"; }

	protected:
		virtual RunResult onRun() throw(KernelException);
	private:
		DECLARE_PRIVATE(PumpResetCommand)
	};


}
#endif
