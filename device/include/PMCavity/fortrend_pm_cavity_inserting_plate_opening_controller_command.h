/**
* @file            fortrend_pm_cavity_inserting_plate_openning_controller_command.h
* @brief           inserting palte opening controller finished commmand for PMCavity
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Subsystem/PMCavity

#ifndef FORTREND_PM_CAVITY_INSERTING_PLATE_OPENING_CONTROLLER_COMMAND_INCLUDE_
#define FORTREND_PM_CAVITY_INSERTING_PLATE_OPENING_CONTROLLER_COMMAND_INCLUDE_

#include "kernel/kernel_subsystem_command.h"

#include "KeyencePLC/keyence_plc_command_executer.h"
#include "KeyencePLC/keyence_plc_subsystem_helper.h"
#include "PMCavity/fortrend_pm_cavity_defined.h"

namespace FC{

	/**
	* @brief  upload finished command for pm cavity
	*/
	class  PMCavityInsertingPlateOpeningControllerCommand : public  KernelSubsystemCommand, public KeyencePlcCommandExecuter {
	public:
		DECLARE_PTR(PMCavityInsertingPlateOpeningControllerCommand)
		PMCavityInsertingPlateOpeningControllerCommand(float percentage, KeyencePlcSubSystemHelper* helper);
		virtual std::string getName()const override { return "InsertingPlateOpeningController"; }

	protected:
		virtual RunResult onRun() throw(KernelException);
	private:
		DECLARE_PRIVATE(PMCavityInsertingPlateOpeningControllerCommand)
	};




}
#endif