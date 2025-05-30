/**
* @file            fortrend_pm_cavity_reset_command.h
* @brief           reset command for PMCavity
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Subsystem/PMCavity

#ifndef FORTREND_PM_CAVITY_RESET_COMMAND_INCLUDE_
#define FORTREND_PM_CAVITY_RESET_COMMAND_INCLUDE_

#include "kernel/kernel_subsystem_command.h"
#include "Kernel/kernel_subsystem_reset_command.h"

#include "KeyencePLC/keyence_plc_command_executer.h"
#include "KeyencePLC/keyence_plc_subsystem_helper.h"

namespace FC{



	/**
	* @brief  output command for pm_cavity
	*/
	class  PMCavityResetCommand :public std::enable_shared_from_this<PMCavityResetCommand>, public  KernelSubsystemResetCommand, public KeyencePlcCommandExecuter{

	public:
		DECLARE_PTR(PMCavityResetCommand)
		PMCavityResetCommand(KeyencePlcSubSystemHelper* hexHelper);
		virtual std::string getName()const override { return "Reset"; }

	protected:
		virtual RunResult onRun() throw(KernelException);
	private:
		DECLARE_PRIVATE(PMCavityResetCommand)
	};


}
#endif
