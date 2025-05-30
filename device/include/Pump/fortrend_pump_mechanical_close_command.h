/**
* @file            fortrend_pump_mechanicl_close_command.h
* @brief           mechanical close command for pump
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Subsystem/Pump

#ifndef FORTREND_PUMP_MECHANICAL_CLOSE_COMMAND_INCLUDE_
#define FORTREND_PUMP_MECHANICAL_CLOSE_COMMAND_INCLUDE_

#include "kernel/kernel_subsystem_command.h"

#include "KeyencePLC/keyence_plc_command_executer.h"
#include "KeyencePLC/keyence_plc_subsystem_helper.h"

namespace FC{

	/**
	* @brief  mechanical close command for Pump
	*/
	class  PumpMechanicalCloseCommand : public  KernelSubsystemCommand, public KeyencePlcCommandExecuter{
	public:
		DECLARE_PTR(PumpMechanicalCloseCommand)
		PumpMechanicalCloseCommand(KeyencePlcSubSystemHelper* helper);
		virtual std::string getName()const override { return "CloseMechanical"; }

	protected:
		virtual RunResult onRun() throw(KernelException);
	private:
		DECLARE_PRIVATE(PumpMechanicalCloseCommand)
	};




}
#endif
