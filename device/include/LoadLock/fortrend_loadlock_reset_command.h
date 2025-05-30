/**
* @file            fortrend_loadlock_reset_command.h
* @brief           reset command for LoadLock
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Subsystem/LoadLock

#ifndef FORTREND_LOADLOCK_RESET_COMMAND_INCLUDE_
#define FORTREND_LOADLOCK_RESET_COMMAND_INCLUDE_

#include "kernel/kernel_subsystem_command.h"
#include "Kernel/kernel_subsystem_reset_command.h"

#include "KeyencePLC/keyence_plc_command_executer.h"
#include "KeyencePLC/keyence_plc_subsystem_helper.h"

namespace FC{



	/**
	* @brief  output command for loadlock
	*/
	class  LoadLockResetCommand : public  KernelSubsystemResetCommand, public KeyencePlcCommandExecuter{

	public:
		DECLARE_PTR(LoadLockResetCommand)
		LoadLockResetCommand(KeyencePlcSubSystemHelper* hexHelper);
		virtual std::string getName()const override { return "Reset"; }

	protected:
		virtual RunResult onRun() throw(KernelException);
	private:
		DECLARE_PRIVATE(LoadLockResetCommand)
	};


}
#endif
