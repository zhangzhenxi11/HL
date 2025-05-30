/**
* @file            fortrend_pump_update_command.h
* @brief           update command for Pump
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Subsystem/Pump

#ifndef FORTREND_PUMP_UPDATE_COMMAND_INCLUDE_
#define FORTREND_PUMP_UPDATE_COMMAND_INCLUDE_
#include "kernel/kernel_subsystem_command.h"
#include "Kernel/kernel_subsystem_update_command.h"
#include "kernel/kernel_exception.h" 

#include "KeyencePLC/keyence_plc_command_executer.h"
#include "KeyencePLC/keyence_plc_subsystem_helper.h"
#include "KYKY/kyky_command_executer.h"
#include "KYKY/kyky_subsystem_helper.h"

namespace FC{

	/**
	* @brief  update command for Pump
	*/
	class  PumpUpdateCommand : public  KernelSubsystemUpdateCommand, public KeyencePlcCommandExecuter{

	public:
		DECLARE_PTR(PumpUpdateCommand)
		PumpUpdateCommand(KeyencePlcSubSystemHelper* helper);
		virtual std::string getName()const override { return "Update"; }

	protected:
		virtual RunResult onRun() throw(KernelException);		
	private:
		DECLARE_PRIVATE(PumpUpdateCommand)
	};




}
#endif
