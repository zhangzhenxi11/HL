/**
* @file            fortrend_tm_cavity_close_diaphragm_valve_command.h
* @brief           close diaphragm valve command for TMCavity
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Subsystem/TMCavity

#ifndef FORTREND_TM_CAVITY_CLOSE_DIAPHRAGM_VALVE_COMMAND_INCLUDE_
#define FORTREND_TM_CAVITY_CLOSE_DIAPHRAGM_VALVE_COMMAND_INCLUDE_

#include "kernel/kernel_subsystem_command.h"

#include "KeyencePLC/keyence_plc_command_executer.h"
#include "KeyencePLC/keyence_plc_subsystem_helper.h"

#include "TMCavity/fortrend_tm_cavity_defined.h"

namespace FC{

	/**
	* @brief  close diaphragm valve command for tm cavity
	*/
	class  TMCavityCloseDiaphragmValveCommand : public  KernelSubsystemCommand, public KeyencePlcCommandExecuter{

	public:
		DECLARE_PTR(TMCavityCloseDiaphragmValveCommand)
		TMCavityCloseDiaphragmValveCommand(const TMCavityValveOpening opening, KeyencePlcSubSystemHelper* helper);
		virtual std::string getName()const override { return "CloseDiaphragmValve"; }

	protected:
		virtual RunResult onRun() throw(KernelException);		
	private:
		DECLARE_PRIVATE(TMCavityCloseDiaphragmValveCommand)
	};




}
#endif
