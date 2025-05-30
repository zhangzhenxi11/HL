/**
* @file            fortrend_loadlock_close_cassette_door_command.h
* @brief           clsoe door command for LoadLock
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Subsystem/LoadLock

#ifndef FORTREND_LOADLOCK_CLOSE_CASSETTE_DOOR_COMMAND_INCLUDE_
#define FORTREND_LOADLOCK_CLOSE_CASSETTE_DOOR_COMMAND_INCLUDE_

#include "kernel/kernel_subsystem_command.h"

#include "KeyencePLC/keyence_plc_command_executer.h"
#include "KeyencePLC/keyence_plc_subsystem_helper.h"

namespace FC{

	/**
	* @brief  close cassette door command for loadlock
	*/
	class  LoadLockCloseCassetteDoorCommand : public  KernelSubsystemCommand, public KeyencePlcCommandExecuter{
	public:
		DECLARE_PTR(LoadLockCloseCassetteDoorCommand)
		LoadLockCloseCassetteDoorCommand(KeyencePlcSubSystemHelper* helper);
		virtual std::string getName()const override { return "CloseCassetteDoor"; }
		std::vector<IKernelResources* > resources() const override;
		

	protected:
		virtual RunResult onRun() throw(KernelException);		
	private:
		DECLARE_PRIVATE(LoadLockCloseCassetteDoorCommand)
	};




}
#endif
