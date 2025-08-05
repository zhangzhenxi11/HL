/**
* @file            fortrend_pm_cavity_get_finished_command.h
* @brief           get finished commmand for PMCavity
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Subsystem/PMCavity

#ifndef FORTREND_PM_CAVITY_GET_FINISHED_COMMAND_INCLUDE_
#define FORTREND_PM_CAVITY_GET_FINISHED_COMMAND_INCLUDE_

#include "kernel/kernel_subsystem_command.h"

#include "InovancePLC/inovance_plc_command_executer.h"
#include "InovancePLC/inovance_plc_subsystem_helper.h"

#include "KeyencePLC/keyence_plc_command_executer.h"
#include "KeyencePLC/keyence_plc_subsystem_helper.h"
#include "PMCavity/fortrend_pm_cavity_defined.h"

namespace FC{

	/**
	* @brief  get finished command for pm cavity
	*/
	class  PMCavityGetFinishedCommand : public  KernelSubsystemCommand, public KeyencePlcCommandExecuter {
	public:
		DECLARE_PTR(PMCavityGetFinishedCommand)
		PMCavityGetFinishedCommand(KeyencePlcSubSystemHelper* helper);
		virtual std::string getName()const override { return "GetFinished"; }

	protected:
		virtual RunResult onRun() throw(KernelException);
	private:
		DECLARE_PRIVATE(PMCavityGetFinishedCommand)
	};




}
#endif