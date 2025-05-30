/**
* @file            fortrend_loadlock_open_cassette_door_command.h
* @brief           open door command for LoadLock
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Subsystem/LoadLock

#ifndef FORTREND_LOADLOCK_OPEN_CASSETTE_DOOR_COMMAND_INCLUDE_
#define FORTREND_LOADLOCK_OPEN_CASSETTE_DOOR_COMMAND_INCLUDE_

#include "kernel/kernel_subsystem_command.h"

#include "KeyencePLC/keyence_plc_command_executer.h"
#include "KeyencePLC/keyence_plc_subsystem_helper.h"

namespace FC{

	/**
	* @brief  open cassette door command for loadlock
	*/
	class  LoadLockOpenCassetteDoorCommand : public  KernelSubsystemCommand, public KeyencePlcCommandExecuter{

	public:
		DECLARE_PTR(LoadLockOpenCassetteDoorCommand)
		LoadLockOpenCassetteDoorCommand(KeyencePlcSubSystemHelper* helper);
		virtual std::string getName()const override { return "OpenCassetteDoor"; }

	protected:
		virtual RunResult onRun() throw(KernelException);		
	private:
		DECLARE_PRIVATE(LoadLockOpenCassetteDoorCommand)
	};




}
#endif
