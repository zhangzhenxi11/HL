/**
* @file            fortrend_tm_cavity_close_pid_command.h
* @brief           close pid command for TMCavity
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Subsystem/TMCavity

#ifndef FORTREND_TM_CAVITY_CLOSE_PID_COMMAND_INCLUDE_
#define FORTREND_TM_CAVITY_CLOSE_PID_COMMAND_INCLUDE_

#include "kernel/kernel_subsystem_command.h"

#include "KeyencePLC/keyence_plc_command_executer.h"
#include "KeyencePLC/keyence_plc_subsystem_helper.h"
#include "TMCavity/fortrend_tm_cavity_defined.h"

namespace FC{

	/**
	* @brief  close PID command for tm cavity
	*/
	class  TMCavityClosePIDCommand : public  KernelSubsystemCommand, public KeyencePlcCommandExecuter{

	public:
		DECLARE_PTR(TMCavityClosePIDCommand)
		TMCavityClosePIDCommand(KeyencePlcSubSystemHelper* helper);
		virtual std::string getName()const override { return "ClosePID"; }

	protected:
		virtual RunResult onRun() throw(KernelException);		
	private:
		DECLARE_PRIVATE(TMCavityClosePIDCommand)
	};




}
#endif
