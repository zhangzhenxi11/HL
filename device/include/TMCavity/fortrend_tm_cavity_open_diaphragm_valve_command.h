/**
* @file            fortrend_tm_cavity_open_diaphragm_valve_command.h
* @brief           open diaphragm valve command for TMCavity
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Subsystem/TMCavity

#ifndef FORTREND_TM_CAVITY_OPEN_DIAPHRAGM_VALVE_COMMAND_INCLUDE_
#define FORTREND_TM_CAVITY_OPEN_DIAPHRAGM_VALVE_COMMAND_INCLUDE_

#include "kernel/kernel_subsystem_command.h"

#include "KeyencePLC/keyence_plc_command_executer.h"
#include "KeyencePLC/keyence_plc_subsystem_helper.h"
#include "TMCavity/fortrend_tm_cavity_defined.h"

namespace FC{

	/**
	* @brief  open diaphragm valve command for tm cavity
	*/
	class  TMCavityOpenDiaphragmValveCommand : public  KernelSubsystemCommand, public KeyencePlcCommandExecuter{

	public:
		DECLARE_PTR(TMCavityOpenDiaphragmValveCommand)
		TMCavityOpenDiaphragmValveCommand(const TMCavityValveOpening opening,KeyencePlcSubSystemHelper* helper);
		virtual std::string getName()const override { return "OpenDiaphragmValve"; }

	protected:
		virtual RunResult onRun() throw(KernelException);		
	private:
		DECLARE_PRIVATE(TMCavityOpenDiaphragmValveCommand)
	};




}
#endif
