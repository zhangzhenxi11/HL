/**
* @file            fortrend_loadlock_close_angle_valve_command.h
* @brief           clsoe angle valve command for LoadLock
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Subsystem/LoadLock

#ifndef FORTREND_LOADLOCK_CLOSE_ANGLE_VALVE_COMMAND_INCLUDE_
#define FORTREND_LOADLOCK_CLOSE_ANGLE_VALVE_COMMAND_INCLUDE_

#include "kernel/kernel_subsystem_command.h"

#include "KeyencePLC/keyence_plc_command_executer.h"
#include "KeyencePLC/keyence_plc_subsystem_helper.h"

namespace FC{

	/**
	* @brief  close angle valve command for loadlock
	*/
	class  LoadLockCloseAngleValveCommand : public  KernelSubsystemCommand, public KeyencePlcCommandExecuter{
	public:
		DECLARE_PTR(LoadLockCloseAngleValveCommand)
		LoadLockCloseAngleValveCommand(KeyencePlcSubSystemHelper* helper);
		virtual std::string getName()const override { return "CloseAngleValve"; }
		std::vector<IKernelResources* > resources() const override;

	protected:
		virtual RunResult onRun() throw(KernelException);		
	private:
		DECLARE_PRIVATE(LoadLockCloseAngleValveCommand)
	};




}
#endif
