/**
* @file            fortrend_tm_cavity_close_ultrahigh_vacuumize_valve_command.h
* @brief           close ultrahighvacuum baffle valve command for LoadLock
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Subsystem/LoadLock

#ifndef FORTREND_LOADLOCK_CLOSE_HEIGHT_VACUUM_BAFFLE_VALVE_COMMAND_INCLUDE_
#define FORTREND_LOADLOCK_CLOSE_HEIGHT_VACUUM_BAFFLE_VALVE_COMMAND_INCLUDE_

#include "kernel/kernel_subsystem_command.h"

#include "KeyencePLC/keyence_plc_command_executer.h"
#include "KeyencePLC/keyence_plc_subsystem_helper.h"

namespace FC{

	/**
	* @brief  close ultrahigh vacuum baffle valve command for tm cavity
	*/
	class  LoadLockCloseHeightVacuumBaffleValveCommand : public  KernelSubsystemCommand, public KeyencePlcCommandExecuter{

	public:
		DECLARE_PTR(LoadLockCloseHeightVacuumBaffleValveCommand)
		LoadLockCloseHeightVacuumBaffleValveCommand(KeyencePlcSubSystemHelper* helper);
		virtual std::string getName()const override { return "CloseHeightVacuumBaffleValve"; }
		std::vector<IKernelResources* > resources() const override;
	protected:
		virtual RunResult onRun() throw(KernelException);		
	private:
		DECLARE_PRIVATE(LoadLockCloseHeightVacuumBaffleValveCommand)
	};




}
#endif
