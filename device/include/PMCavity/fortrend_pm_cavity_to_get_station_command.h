/**
* @file            fortrend_pm_cavity_get_finished_command.h
* @brief           get finished commmand for PMCavity
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Subsystem/PMCavity

#ifndef FORTREND_PM_CAVITY_TO_GET_STATION_COMMAND_INCLUDE_
#define FORTREND_PM_CAVITY_TO_GET_STATION_COMMAND_INCLUDE_

#include "kernel/kernel_subsystem_command.h"

#include "KeyencePLC/keyence_plc_command_executer.h"
#include "KeyencePLC/keyence_plc_subsystem_helper.h"
#include "PMCavity/fortrend_pm_cavity_defined.h"

namespace FC{

	/**
	* @brief  get finished command for pm cavity
	*/
	class  PMCavityToGetStationCommand : public std::enable_shared_from_this<PMCavityToGetStationCommand>, public  KernelSubsystemCommand, public KeyencePlcCommandExecuter{
	public:
		DECLARE_PTR(PMCavityToGetStationCommand)
		PMCavityToGetStationCommand(KeyencePlcSubSystemHelper* helper);
		virtual std::string getName()const override { return "ToGetStation"; }

	protected:
		virtual RunResult onRun() throw(KernelException);
	private:
		DECLARE_PRIVATE(PMCavityToGetStationCommand)
	};




}
#endif