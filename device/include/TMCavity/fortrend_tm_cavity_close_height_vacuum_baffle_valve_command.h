/**
* @file            fortrend_tm_cavity_close_ultrahigh_vacuumize_valve_command.h
* @brief           close ultrahighvacuum baffle valve command for TMCavity
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Subsystem/TMCavity

#ifndef FORTREND_TM_CAVITY_CLOSE_HEIGHT_VACUUM_BAFFLE_VALVE_COMMAND_INCLUDE_
#define FORTREND_TM_CAVITY_CLOSE_HEIGHT_VACUUM_BAFFLE_VALVE_COMMAND_INCLUDE_

#include "kernel/kernel_subsystem_command.h"

#include "KeyencePLC/keyence_plc_command_executer.h"
#include "KeyencePLC/keyence_plc_subsystem_helper.h"

namespace FC{

	/**
	* @brief  close ultrahigh vacuum baffle valve command for tm cavity
	*/
	class  TMCavityCloseHeightVacuumBaffleValveCommand : public  KernelSubsystemCommand, public KeyencePlcCommandExecuter{

	public:
		DECLARE_PTR(TMCavityCloseHeightVacuumBaffleValveCommand)
		TMCavityCloseHeightVacuumBaffleValveCommand(KeyencePlcSubSystemHelper* helper);
		virtual std::string getName()const override { return "CloseHeightVacuumBaffleValve"; }

	protected:
		virtual RunResult onRun() throw(KernelException);		
	private:
		DECLARE_PRIVATE(TMCavityCloseHeightVacuumBaffleValveCommand)
	};




}
#endif
