/**
* @file            fortrend_pm_cavity_update_command.h
* @brief           update command for PMCavity
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Subsystem/PMCavity

#ifndef FORTREND_PM_CAVITY_UPDATE_COMMAND_INCLUDE_
#define FORTREND_PM_CAVITY_UPDATE_COMMAND_INCLUDE_
#include "kernel/kernel_subsystem_command.h"
#include "Kernel/kernel_subsystem_update_command.h"
#include "kernel/kernel_exception.h" 

#include "KeyencePLC/keyence_plc_command_executer.h"
#include "KeyencePLC/keyence_plc_subsystem_helper.h"

namespace FC{

	/**
	* @brief  update command for pm_cavity
	*/
	class  PMCavityUpdateCommand : public  KernelSubsystemUpdateCommand, public KeyencePlcCommandExecuter{

	public:
		DECLARE_PTR(PMCavityUpdateCommand)
		PMCavityUpdateCommand(KeyencePlcSubSystemHelper* hexHelper);
		virtual std::string getName()const override { return "Update"; }

	protected:
		virtual RunResult onRun() throw(KernelException);
	private:
		DECLARE_PRIVATE(PMCavityUpdateCommand)
	};




}
#endif
