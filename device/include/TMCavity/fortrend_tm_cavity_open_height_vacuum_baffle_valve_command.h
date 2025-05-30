/**
* @file            fortrend_tm_cavity_open_ultrahigh_vacuumize_valve_command.h
* @brief           open ultrahigh vacuum baffle valve command for TMCavity
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Subsystem/TMCavity

#ifndef FORTREND_TM_CAVITY_OPEN_HEIGHT_VACUUM_BAFFLE_VALVE_COMMAND_INCLUDE_
#define FORTREND_TM_CAVITY_OPEN_HEIGHT_VACUUM_BAFFLE_VALVE_COMMAND_INCLUDE_

#include "kernel/kernel_subsystem_command.h"

#include "KeyencePLC/keyence_plc_command_executer.h"
#include "KeyencePLC/keyence_plc_subsystem_helper.h"

namespace FC{

	/**
	* @brief  open ultrahigh vacuum baffle valve command for tm cavity
	*/
	class  TMCavityOpenHeightVacuumBaffleValveCommand : public  KernelSubsystemCommand, public KeyencePlcCommandExecuter{

	public:
		DECLARE_PTR(TMCavityOpenHeightVacuumBaffleValveCommand)
		TMCavityOpenHeightVacuumBaffleValveCommand(KeyencePlcSubSystemHelper* helper);
		virtual std::string getName()const override { return "OpenHeightVacuumBaffleValve"; }

	protected:
		virtual RunResult onRun() throw(KernelException);
	private:
		DECLARE_PRIVATE(TMCavityOpenHeightVacuumBaffleValveCommand)
	};




}
#endif
