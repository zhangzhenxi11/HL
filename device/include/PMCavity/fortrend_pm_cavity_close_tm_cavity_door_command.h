/**
* @file            fortrend_pm_cavity_close_tm_cavity_door_command.h
* @brief           clsoe tm cavity door command for PMCavity
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Subsystem/PMCavity

#ifndef FORTREND_PM_CAVITY_CLOSE_TM_CAVITY_DOOR_COMMAND_INCLUDE_
#define FORTREND_PM_CAVITY_CLOSE_TM_CAVITY_DOOR_COMMAND_INCLUDE_

#include "kernel/kernel_subsystem_command.h"

#include "KeyencePLC/keyence_plc_command_executer.h"
#include "KeyencePLC/keyence_plc_subsystem_helper.h"

namespace FC{

	/**
	* @brief  close tm cavity door command for pm cavity
	*/
	class  PMCavityCloseTMCavityDoorCommand : public  KernelSubsystemCommand, public KeyencePlcCommandExecuter{
	public:
		DECLARE_PTR(PMCavityCloseTMCavityDoorCommand)
		PMCavityCloseTMCavityDoorCommand(KeyencePlcSubSystemHelper* helper);
		virtual std::string getName()const override { return "CloseTMCavityDoor"; }

	protected:
		virtual RunResult onRun() throw(KernelException);		
	private:
		DECLARE_PRIVATE(PMCavityCloseTMCavityDoorCommand)
	};




}
#endif
