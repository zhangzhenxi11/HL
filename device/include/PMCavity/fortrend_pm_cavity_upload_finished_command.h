/**
* @file            fortrend_pm_cavity_upload_finished_command.h
* @brief           upload finished commmand for PMCavity
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Subsystem/PMCavity

#ifndef FORTREND_PM_CAVITY_UPLOAD_FINISHED_COMMAND_INCLUDE_
#define FORTREND_PM_CAVITY_UPLOAD_FINISHED_COMMAND_INCLUDE_

#include "kernel/kernel_subsystem_command.h"

#include "KeyencePLC/keyence_plc_command_executer.h"
#include "KeyencePLC/keyence_plc_subsystem_helper.h"
#include "PMCavity/fortrend_pm_cavity_defined.h"

namespace FC{

	/**
	* @brief  upload finished command for pm cavity
	*/
	class  PMCavityUploadFinishedCommand : public  KernelSubsystemCommand, public KeyencePlcCommandExecuter {
	public:
		DECLARE_PTR(PMCavityUploadFinishedCommand)
		PMCavityUploadFinishedCommand(KeyencePlcSubSystemHelper* helper);
		virtual std::string getName()const override { return "UploadFinished"; }

	protected:
		virtual RunResult onRun() throw(KernelException);
	private:
		DECLARE_PRIVATE(PMCavityUploadFinishedCommand)
	};




}
#endif