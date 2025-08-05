/**
* @file            fortrend_pm_cavity_clear_state_command.h
* @brief           clear state command for PMCavity
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Subsystem/PMCavity

#ifndef FORTREND_PM_CAVITY_CLEAR_STATE_COMMAND_INCLUDE_
#define FORTREND_PM_CAVITY_CLEAR_STATE_COMMAND_INCLUDE_
#include "kernel/kernel_subsystem_command.h"
#include "kernel/kernel_exception.h" 

#include "InovancePLC/inovance_plc_command_executer.h"
#include "InovancePLC/inovance_plc_subsystem_helper.h"

#include "KeyencePLC/keyence_plc_command_executer.h"
#include "KeyencePLC/keyence_plc_subsystem_helper.h"
namespace FC{

	/**
	* @brief  clear state command for pm_cavity
	*/
	class  PMCavityClearStateCommand : public  KernelSubsystemCommand, public KeyencePlcCommandExecuter {

	public:
		DECLARE_PTR(PMCavityClearStateCommand)
		PMCavityClearStateCommand(KeyencePlcSubSystemHelper* hexHelper);
		virtual std::string getName()const override { return "ClearState"; }

	protected:
		virtual RunResult onRun() throw(KernelException);
	private:
		DECLARE_PRIVATE(PMCavityClearStateCommand)
	};




}
#endif