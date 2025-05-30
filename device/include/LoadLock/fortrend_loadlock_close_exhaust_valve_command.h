/**
* @file            fortrend_loadlock_close_exhaust_valve_command.h
* @brief           close exhaust valve command for LoadLock
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Subsystem/LoadLock

#ifndef FORTREND_LOADLOCK_CLOSE_EXHAUST_VALVE_COMMAND_INCLUDE_
#define FORTREND_LOADLOCK_CLOSE_EXHAUST_VALVE_COMMAND_INCLUDE_

#include "kernel/kernel_subsystem_command.h"

#include "KeyencePLC/keyence_plc_command_executer.h"
#include "KeyencePLC/keyence_plc_subsystem_helper.h"

namespace FC{

	/**
	* @brief  close exhaust valve command for loadlock
	*/
	class  LoadLockCloseExhaustValveCommand : public  KernelSubsystemCommand, public KeyencePlcCommandExecuter{

	public:
		DECLARE_PTR(LoadLockCloseExhaustValveCommand)
		LoadLockCloseExhaustValveCommand(KeyencePlcSubSystemHelper* helper);
		virtual std::string getName()const override { return "CloseExhaustValve"; }
		std::vector<IKernelResources* > resources() const override;
	protected:
		virtual RunResult onRun() throw(KernelException);		
	private:
		DECLARE_PRIVATE(LoadLockCloseExhaustValveCommand)
	};




}
#endif
