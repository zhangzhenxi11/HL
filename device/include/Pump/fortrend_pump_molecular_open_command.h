/**
* @file            fortrend_pump_molecular_open_command.h
* @brief           molecular open command for pump
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Subsystem/Pump

#ifndef FORTREND_PUMP_MOLECULAR_OPEN_COMMAND_INCLUDE_
#define FORTREND_PUMP_MOLECULAR_OPEN_COMMAND_INCLUDE_

#include "kernel/kernel_subsystem_command.h"

#include "KeyencePLC/keyence_plc_command_executer.h"
#include "KeyencePLC/keyence_plc_subsystem_helper.h"

namespace FC{
	/**
	* @brief  molecular open command for Pump
	*/
	class  PumpMolecularOpenCommand : public  KernelSubsystemCommand, public KeyencePlcCommandExecuter{
	public:
		DECLARE_PTR(PumpMolecularOpenCommand)
		PumpMolecularOpenCommand(KeyencePlcSubSystemHelper* helper, std::string name);
		virtual std::string getName()const override { return "OpenMolecular"; }
		std::string getmoduleName();
	protected:
		virtual RunResult onRun() throw(KernelException);		
	private:
		std::string moduleName;
		DECLARE_PRIVATE(PumpMolecularOpenCommand)
	};




}
#endif
