/**
* @file            fortrend_loadlock_open_tm_cavity_door_command.h
* @brief           open door command for LoadLock
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Subsystem/LoadLock

#ifndef FORTREND_LOADLOCK_OPEN_TM_CAVITY_DOOR_COMMAND_INCLUDE_
#define FORTREND_LOADLOCK_OPEN_TM_CAVITY_DOOR_COMMAND_INCLUDE_

#include "kernel/kernel_subsystem_command.h"

#include "KeyencePLC/keyence_plc_command_executer.h"
#include "KeyencePLC/keyence_plc_subsystem_helper.h"

namespace FC{

	/**
	* @brief  open tm cavity door command for loadlock
	*/
	class  LoadLockOpenTMCavityDoorCommand : public  KernelSubsystemCommand, public KeyencePlcCommandExecuter{

	public:
		DECLARE_PTR(LoadLockOpenTMCavityDoorCommand)
		LoadLockOpenTMCavityDoorCommand(KeyencePlcSubSystemHelper* helper);
		virtual std::string getName()const override { return "OpenTMCavityDoor"; }

	protected:
		virtual RunResult onRun() throw(KernelException);		
	private:
		DECLARE_PRIVATE(LoadLockOpenTMCavityDoorCommand)
	};




}
#endif
