/**
* @file            fortrend_pm_cavity_open_tm_cavity_door_command.h
* @brief           open tm cavity door command for PMCavity
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Subsystem/PMCavity

#ifndef FORTREND_PM_CAVITY_OPEN_TM_CAVITY_DOOR_COMMAND_INCLUDE_
#define FORTREND_PM_CAVITY_OPEN_TM_CAVITY_DOOR_COMMAND_INCLUDE_

#include "kernel/kernel_subsystem_command.h"

#include "KeyencePLC/keyence_plc_command_executer.h"
#include "KeyencePLC/keyence_plc_subsystem_helper.h"
#include "InovancePLC/inovance_plc_command_executer.h"
#include "InovancePLC/inovance_plc_subsystem_helper.h"

namespace FC{

	/**
	* @brief  open tm cavity door command for pm_cavity
	*/
	class  PMCavityOpenTMCavityDoorCommand : public  KernelSubsystemCommand, public KeyencePlcCommandExecuter, public InovancePlcCommandExecuter {

	public:
		DECLARE_PTR(PMCavityOpenTMCavityDoorCommand)
		PMCavityOpenTMCavityDoorCommand(KeyencePlcSubSystemHelper* keyence_helper, InovancePlcSubSystemHelper* inovance_helper);
		virtual std::string getName()const override { return "OpenTMCavityDoor"; }

	protected:
		virtual RunResult onRun() throw(KernelException);		
	private:
		DECLARE_PRIVATE(PMCavityOpenTMCavityDoorCommand)
	};




}
#endif
