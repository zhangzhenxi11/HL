/**
* @file            fortrend_pump_molecular_get_state_command.h
* @brief           molecular get state command for pump
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Subsystem/Pump

#ifndef FORTREND_PUMP_MOLECULAR_GET_STATE_COMMAND_INCLUDE_
#define FORTREND_PUMP_MOLECULAR_GET_STATE_COMMAND_INCLUDE_

#include "kernel/kernel_subsystem_command.h"

#include "KYKY/kyky_command_executer.h"
#include "KYKY/kyky_subsystem_helper.h"

namespace FC{

	/**
	* @brief  molecular get state command for Pump
	*/
	class  PumpMolecularGetStateCommand : public  KernelSubsystemCommand, public KYKYCommandExecuter{
	public:
		DECLARE_PTR(PumpMolecularGetStateCommand)
		PumpMolecularGetStateCommand(KYKYSubSystemHelper* helper);
		virtual std::string getName()const override { return "MolecularGetState"; }

	protected:
		virtual RunResult onRun() throw(KernelException);
	private:
		DECLARE_PRIVATE(PumpMolecularGetStateCommand)
	};




}
#endif
