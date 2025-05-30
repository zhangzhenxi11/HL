/**
* @file            fortrend_cooling_cavity_open_diaphragm_valve_command.h
* @brief           open diaphragm valve command for Cooling Cavity
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Subsystem/CoolingCavity

#ifndef FORTREND_COOLING_CAVITY_OPEN_DIAPHRAGM_VALVE_COMMAND_INCLUDE_
#define FORTREND_COOLING_CAVITY_OPEN_DIAPHRAGM_VALVE_COMMAND_INCLUDE_

#include "kernel/kernel_subsystem_command.h"

#include "KeyencePLC/keyence_plc_command_executer.h"
#include "KeyencePLC/keyence_plc_subsystem_helper.h"

namespace FC{

	/**
	* @brief  open diaphragm valve command for cooling_cavity
	*/
	class  CoolingCavityOpenDiaphragmValveCommand : public  KernelSubsystemCommand, public KeyencePlcCommandExecuter{

	public:
		DECLARE_PTR(CoolingCavityOpenDiaphragmValveCommand)
		CoolingCavityOpenDiaphragmValveCommand(KeyencePlcSubSystemHelper* helper);
		virtual std::string getName()const override { return "OpenDiaphragmValve"; }

	protected:
		virtual RunResult onRun() throw(KernelException);		
	private:
		DECLARE_PRIVATE(CoolingCavityOpenDiaphragmValveCommand)
	};




}
#endif
