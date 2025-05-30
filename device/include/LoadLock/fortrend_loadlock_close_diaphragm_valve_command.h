/**
* @file            fortrend_loadlock_close_diaphragm_valve_command.h
* @brief           close diaphragm valve command for LoadLock
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Subsystem/LoadLock

#ifndef FORTREND_LOADLOCK_CLOSE_DIAPHRAGM_VALVE_COMMAND_INCLUDE_
#define FORTREND_LOADLOCK_CLOSE_DIAPHRAGM_VALVE_COMMAND_INCLUDE_

#include "kernel/kernel_subsystem_command.h"

#include "KeyencePLC/keyence_plc_command_executer.h"
#include "KeyencePLC/keyence_plc_subsystem_helper.h"
#include "LoadLock/fortrend_loadlock_defined.h"

namespace FC{

	/**
	* @brief  close diaphragm valve command for loadlock
	*/
	class  LoadLockCloseDiaphragmValveCommand : public  KernelSubsystemCommand, public KeyencePlcCommandExecuter{

	public:
		DECLARE_PTR(LoadLockCloseDiaphragmValveCommand)
		LoadLockCloseDiaphragmValveCommand(const LoadLockValveOpening opening, KeyencePlcSubSystemHelper* helper);
		virtual std::string getName()const override { return "CloseDiaphragmValve"; }
		std::vector<IKernelResources* > resources() const override;
	protected:
		virtual RunResult onRun() throw(KernelException);		
	private:
		DECLARE_PRIVATE(LoadLockCloseDiaphragmValveCommand)
	};




}
#endif
