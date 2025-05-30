/**
* @file            fortrend_cooling_cavity_reset_command.h
* @brief           reset command for CoolingCavity
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Subsystem/CoolingCavity

#ifndef FORTREND_COOLING_CAVITY_RESET_COMMAND_INCLUDE_
#define FORTREND_COOLING_CAVITY_RESET_COMMAND_INCLUDE_

#include "kernel/kernel_subsystem_command.h"
#include "Kernel/kernel_subsystem_reset_command.h"

#include "KeyencePLC/keyence_plc_command_executer.h"
#include "KeyencePLC/keyence_plc_subsystem_helper.h"

namespace FC{



	/**
	* @brief  output command for cooling_cavity
	*/
	class  CoolingCavityResetCommand : public  KernelSubsystemResetCommand, public KeyencePlcCommandExecuter{

	public:
		DECLARE_PTR(CoolingCavityResetCommand)
		CoolingCavityResetCommand(KeyencePlcSubSystemHelper* hexHelper);
		virtual std::string getName()const override { return "Reset"; }

	protected:
		virtual RunResult onRun() throw(KernelException);
	private:
		DECLARE_PRIVATE(CoolingCavityResetCommand)
	};


}
#endif
