/**
* @file            fortrend_loadlock_open_angle_valve_command.h
* @brief           open angle valve command for LoadLock
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Subsystem/LoadLock

#ifndef FORTREND_LOADLOCK_OPEN_ANGLE_VALVE_COMMAND_INCLUDE_
#define FORTREND_LOADLOCK_OPEN_ANGLE_VALVE_COMMAND_INCLUDE_

#include "kernel/kernel_subsystem_command.h"

#include "KeyencePLC/keyence_plc_command_executer.h"
#include "KeyencePLC/keyence_plc_subsystem_helper.h"

namespace FC{

	/**
	* @brief  open angle valve command for loadlock
	*/
	class  LoadLockOpenAngleValveCommand : public  KernelSubsystemCommand, public KeyencePlcCommandExecuter{

	public:
		DECLARE_PTR(LoadLockOpenAngleValveCommand)
		LoadLockOpenAngleValveCommand(KeyencePlcSubSystemHelper* helper);
		virtual std::string getName()const override { return "OpenAngleValve"; }
		std::vector<IKernelResources* > resources() const override;
	protected:
		virtual RunResult onRun() throw(KernelException);		
	private:
		DECLARE_PRIVATE(LoadLockOpenAngleValveCommand)
	};




}
#endif
