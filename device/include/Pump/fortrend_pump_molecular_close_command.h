/**
* @file            fortrend_pump_molecular_close_command.h
* @brief           molecular close command for pump
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Subsystem/Pump

#ifndef FORTREND_PUMP_MOLECULAR_CLOSE_COMMAND_INCLUDE_
#define FORTREND_PUMP_MOLECULAR_CLOSE_COMMAND_INCLUDE_

#include "kernel/kernel_subsystem_command.h"

#include "KeyencePLC/keyence_plc_command_executer.h"
#include "KeyencePLC/keyence_plc_subsystem_helper.h"

namespace FC{

	/**
	* @brief  molecular close command for Pump
	*/
	class  PumpMolecularCloseCommand : public  KernelSubsystemCommand, public KeyencePlcCommandExecuter{
	public:
		DECLARE_PTR(PumpMolecularCloseCommand)
		PumpMolecularCloseCommand(KeyencePlcSubSystemHelper* helper, std::string name);
		virtual std::string getName()const override { return "CloseMolecular"; }
		std::string getmoduleName();
	protected:
		virtual RunResult onRun() throw(KernelException);		
	private:
		std::string moduleName;
		DECLARE_PRIVATE(PumpMolecularCloseCommand)
	};




}
#endif
