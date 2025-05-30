/**
* @file            fortrend_pump_mechanicl_open_command.h
* @brief           mechanical open command for pump
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Subsystem/Pump

#ifndef FORTREND_PUMP_MECHANICAL_OPEN_COMMAND_INCLUDE_
#define FORTREND_PUMP_MECHANICAL_OPEN_COMMAND_INCLUDE_

#include "kernel/kernel_subsystem_command.h"

#include "KeyencePLC/keyence_plc_command_executer.h"
#include "KeyencePLC/keyence_plc_subsystem_helper.h"

namespace FC{

	/**
	* @brief  mechanical open command for Pump
	*/
	class  PumpMechanicalOpenCommand : public  KernelSubsystemCommand, public KeyencePlcCommandExecuter{
	public:
		DECLARE_PTR(PumpMechanicalOpenCommand)
		PumpMechanicalOpenCommand(KeyencePlcSubSystemHelper* helper);
		virtual std::string getName()const override { return "OpenMechanical"; }

	protected:
		virtual RunResult onRun() throw(KernelException);		
	private:
		DECLARE_PRIVATE(PumpMechanicalOpenCommand)
	};




}
#endif
