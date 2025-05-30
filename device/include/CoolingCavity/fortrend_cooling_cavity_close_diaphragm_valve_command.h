/**
* @file            fortrend_cooling_cavity_close_diaphragm_valve_command.h
* @brief           close diaphragm valve command for Cooling Cavity
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Subsystem/CoolingCavity

#ifndef FORTREND_COOLING_CAVITY_CLOSE_DIAPHRAGM_VALVE_COMMAND_INCLUDE_
#define FORTREND_COOLING_CAVITY_CLOSE_DIAPHRAGM_VALVE_COMMAND_INCLUDE_

#include "kernel/kernel_subsystem_command.h"

#include "KeyencePLC/keyence_plc_command_executer.h"
#include "KeyencePLC/keyence_plc_subsystem_helper.h"

namespace FC{

	/**
	* @brief  close diaphragm valve command for cooling cavity
	*/
	class  CoolingCavityCloseDiaphragmValveCommand : public  KernelSubsystemCommand, public KeyencePlcCommandExecuter{

	public:
		DECLARE_PTR(CoolingCavityCloseDiaphragmValveCommand)
		CoolingCavityCloseDiaphragmValveCommand(KeyencePlcSubSystemHelper* helper);
		virtual std::string getName()const override { return "CloseDiaphragmValve"; }

	protected:
		virtual RunResult onRun() throw(KernelException);		
	private:
		DECLARE_PRIVATE(CoolingCavityCloseDiaphragmValveCommand)
	};




}
#endif
