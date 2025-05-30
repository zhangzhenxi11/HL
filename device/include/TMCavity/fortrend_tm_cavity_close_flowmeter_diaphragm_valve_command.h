/**
* @file            fortrend_tm cavity_close_flowmeter_diaphragm_valve_command.h
* @brief           clsoe flowmeter diaphragm valve command for TMCavity
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Subsystem/TMCavity

#ifndef FORTREND_TM_CAVITY_CLOSE_FLOWMETER_DIAPGRAGM_VALVE_COMMAND_INCLUDE_
#define FORTREND_TM_CAVITY_CLOSE_FLOWMETER_DIAPGRAGM_VALVE_COMMAND_INCLUDE_

#include "kernel/kernel_subsystem_command.h"

#include "KeyencePLC/keyence_plc_command_executer.h"
#include "KeyencePLC/keyence_plc_subsystem_helper.h"

namespace FC{

	/**
	* @brief  close flowmeter diaphragm valve command for tm cavity
	*/
	class  TMCavityCloseFlowmeterDiaphragmValveCommand : public  KernelSubsystemCommand, public KeyencePlcCommandExecuter{
	public:
		DECLARE_PTR(TMCavityCloseFlowmeterDiaphragmValveCommand)
		TMCavityCloseFlowmeterDiaphragmValveCommand(KeyencePlcSubSystemHelper* helper);
		virtual std::string getName()const override { return "CloseFlowmeterDiaphragmValve"; }

	protected:
		virtual RunResult onRun() throw(KernelException);		
	private:
		DECLARE_PRIVATE(TMCavityCloseFlowmeterDiaphragmValveCommand)
	};




}
#endif
