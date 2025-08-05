/**
* @file            fortrend_pm_cavity_read_process_parameters_command.h
* @brief           read process parameters commmand for PMCavity
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Subsystem/PMCavity

#ifndef FORTREND_PM_CAVITY_READ_PROCESS_PARAMETERS_COMMAND_INCLUDE_
#define FORTREND_PM_CAVITY_READ_PROCESS_PARAMETERS_COMMAND_INCLUDE_

#include "kernel/kernel_subsystem_command.h"

#include "InovancePLC/inovance_plc_command_executer.h"
#include "InovancePLC/inovance_plc_subsystem_helper.h"

#include "KeyencePLC/keyence_plc_command_executer.h"
#include "KeyencePLC/keyence_plc_subsystem_helper.h"
#include "PMCavity/fortrend_pm_cavity_defined.h"

namespace FC{

	/**
	* @brief  read process parameters command for pm cavity
	*/
	class  PMCavityReadProcessParametersCommand : public  KernelSubsystemCommand, public KeyencePlcCommandExecuter {
	public:
		DECLARE_PTR(PMCavityReadProcessParametersCommand)
		PMCavityReadProcessParametersCommand(KeyencePlcSubSystemHelper* helper);
		virtual std::string getName()const override { return "ReadProcessParameters"; }

	protected:
		virtual RunResult onRun() throw(KernelException);		
	private:
		DECLARE_PRIVATE(PMCavityReadProcessParametersCommand)
	};




}
#endif
