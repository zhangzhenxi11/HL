/**
* @file            fortrend_tm_cavity_open_inserting_plate_valve_command.h
* @brief           open inserting plate valve command for LoadLock
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Subsystem/LoadLock

#ifndef FORTREND_LOADLOCK_OPEN_INSERTING_PLATE_VALVE_COMMAND_INCLUDE_
#define FORTREND_LOADLOCK_OPEN_INSERTING_PLATE_VALVE_COMMAND_INCLUDE_

#include "kernel/kernel_subsystem_command.h"

#include "KeyencePLC/keyence_plc_command_executer.h"
#include "KeyencePLC/keyence_plc_subsystem_helper.h"

namespace FC{

	/**
	* @brief  open inserting plate valve command for tm cavity
	*/
	class  LoadLockOpenInsertingPlateValveCommand : public  KernelSubsystemCommand, public KeyencePlcCommandExecuter{

	public:
		DECLARE_PTR(LoadLockOpenInsertingPlateValveCommand)
		LoadLockOpenInsertingPlateValveCommand(KeyencePlcSubSystemHelper* helper);
		virtual std::string getName()const override { return "OpenInsertingPlateValve"; }
		std::vector<IKernelResources* > resources() const override;
	protected:
		virtual RunResult onRun() throw(KernelException);		
	private:
		DECLARE_PRIVATE(LoadLockOpenInsertingPlateValveCommand)
	};




}
#endif
