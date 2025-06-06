/**
* @file            fortrend_tm cavity_open_flowmeter_valve_command.h
* @brief           open flowmeter diaphragm valve command for TMCavity
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Subsystem/TMCavity

#ifndef FORTREND_TM_CAVITY_OPEN_FLOWMETER_DIAPHRAGM_VALVE_COMMAND_INCLUDE_
#define FORTREND_TM_CAVITY_OPEN_FLOWMETER_DIAPHRAGM_VALVE_COMMAND_INCLUDE_

#include "kernel/kernel_subsystem_command.h"
#include "KeyencePLC/keyence_plc_command_executer.h"
#include "KeyencePLC/keyence_plc_subsystem_helper.h"

namespace FC{

	/**
	* @brief  open flowmeter diaphragm valve command for tm cavity
	*/
	class  TMCavityOpenFlowmeterDiaphragmValveCommand : public  KernelSubsystemCommand, public KeyencePlcCommandExecuter{

	public:
		DECLARE_PTR(TMCavityOpenFlowmeterDiaphragmValveCommand)
		TMCavityOpenFlowmeterDiaphragmValveCommand(KeyencePlcSubSystemHelper* helper);
		virtual std::string getName()const override { return "OpenFlowmeterDiaphragmValve"; }

	protected:
		virtual RunResult onRun() throw(KernelException);		
	private:
		DECLARE_PRIVATE(TMCavityOpenFlowmeterDiaphragmValveCommand)
	};




}
#endif
