/**
* @file            fortrend_loadlock_close_tm_cavity_door_command.h
* @brief           clsoe door command for LoadLock
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Subsystem/LoadLock

#ifndef FORTREND_LOADLOCK_CLOSE_TM_CAVITY_DOOR_COMMAND_INCLUDE_
#define FORTREND_LOADLOCK_CLOSE_TM_CAVITY_DOOR_COMMAND_INCLUDE_

#include "kernel/kernel_subsystem_command.h"

#include "KeyencePLC/keyence_plc_command_executer.h"
#include "KeyencePLC/keyence_plc_subsystem_helper.h"

namespace FC{

	/**
	* @brief  close tm cavity door command for loadlock
	*/
	class  LoadLockCloseTMCavityDoorCommand : public  KernelSubsystemCommand, public KeyencePlcCommandExecuter{
	public:
		DECLARE_PTR(LoadLockCloseTMCavityDoorCommand)
		LoadLockCloseTMCavityDoorCommand(KeyencePlcSubSystemHelper* helper);
		virtual std::string getName()const override { return "CloseTMCavityDoor"; }
		//std::vector<IKernelResources* > resources() const override; //2026-2-2 ¼ÓÉÏWTR»¥Ëø
	protected:
		virtual RunResult onRun() throw(KernelException);		
	private:
		DECLARE_PRIVATE(LoadLockCloseTMCavityDoorCommand)
	};




}
#endif
