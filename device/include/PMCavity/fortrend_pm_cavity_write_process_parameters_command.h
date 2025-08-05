/**
* @file            fortrend_pm_cavity_write_process_parameters_command.h
* @brief           write process parameters commmand for PMCavity
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Subsystem/PMCavity

#ifndef FORTREND_PM_CAVITY_WRITE_PROCESS_PARAMETERS_COMMAND_INCLUDE_
#define FORTREND_PM_CAVITY_WRITE_PROCESS_PARAMETERS_COMMAND_INCLUDE_

#include "kernel/kernel_subsystem_command.h"

#include "InovancePLC/inovance_plc_command_executer.h"
#include "InovancePLC/inovance_plc_subsystem_helper.h"

#include "KeyencePLC/keyence_plc_command_executer.h"
#include "KeyencePLC/keyence_plc_subsystem_helper.h"
#include  "fortrend_pm_cavity_defined.h"

namespace FC{

	/**
	* @brief  write process parameters command for pm cavity
	*/
	class  PMCavityWriteProcessParametersCommand : public  KernelSubsystemCommand, public KeyencePlcCommandExecuter {
	public:
		DECLARE_PTR(PMCavityWriteProcessParametersCommand)
		PMCavityWriteProcessParametersCommand(const PMCavityProcessParameters process_parameters, KeyencePlcSubSystemHelper* helper);
		virtual std::string getName()const override { return "WriteProcessParameters"; }

	protected:
		virtual RunResult onRun() throw(KernelException);		
	private:
		DECLARE_PRIVATE(PMCavityWriteProcessParametersCommand)
	};




}
#endif
