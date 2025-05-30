/**
* @file            fortrend_loadlock_open_exhaust_door_command.h
* @brief           open exhaust valve command for LoadLock
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Subsystem/LoadLock

#ifndef FORTREND_LOADLOCK_OPEN_EXHAUST_VALVE_COMMAND_INCLUDE_
#define FORTREND_LOADLOCK_OPEN_EXHAUST_VALVE_COMMAND_INCLUDE_

#include "kernel/kernel_subsystem_command.h"

#include "KeyencePLC/keyence_plc_command_executer.h"
#include "KeyencePLC/keyence_plc_subsystem_helper.h"

namespace FC{

	/**
	* @brief  open exhaust valve command for loadlock
	*/
	class  LoadLockOpenExhaustValveCommand : public  KernelSubsystemCommand, public KeyencePlcCommandExecuter{

	public:
		DECLARE_PTR(LoadLockOpenExhaustValveCommand)
		LoadLockOpenExhaustValveCommand(KeyencePlcSubSystemHelper* helper);
		virtual std::string getName()const override { return "OpenExhaustValve"; }
		std::vector<IKernelResources* > resources() const override;
	protected:
		virtual RunResult onRun() throw(KernelException);		
	private:
		DECLARE_PRIVATE(LoadLockOpenExhaustValveCommand)
	};




}
#endif
