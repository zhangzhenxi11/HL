/**
* @file            fortrend_pump_clear_error_command.h
* @brief           clear error command for Pump
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Subsystem/Pump

#ifndef FORTREND_PUMP_CLEAR_ERROR_COMMAND_INCLUDE_
#define FORTREND_PUMP_CLEAR_ERROR_COMMAND_INCLUDE_

#include "kernel/kernel_subsystem_command.h"
#include "Kernel/kernel_subsystem_reset_command.h"

#include "KeyencePLC/keyence_plc_command_executer.h"
#include "KeyencePLC/keyence_plc_subsystem_helper.h"

namespace FC{



	/**
	* @brief  clear error command for pump
	*/
	class  PumpClearErrorCommand : public  KernelSubsystemCommand, public KeyencePlcCommandExecuter{

	public:
		DECLARE_PTR(PumpClearErrorCommand)
		PumpClearErrorCommand(KeyencePlcSubSystemHelper* hexHelper);
		virtual std::string getName()const override { return "ClearError"; }

	protected:
		virtual RunResult onRun() throw(KernelException);
	private:
		DECLARE_PRIVATE(PumpClearErrorCommand)
	};


}
#endif
