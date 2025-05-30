/**
* @file            fortrend_pm_cavity_upload_finished_command.h
* @brief           upload finished commmand for PMCavity
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Subsystem/PMCavity

#ifndef FORTREND_PM_CAVITY_TO_PUT_STATION_COMMAND_INCLUDE_
#define FORTREND_PM_CAVITY_TO_PUT_STATION_COMMAND_INCLUDE_

#include "kernel/kernel_subsystem_command.h"

#include "KeyencePLC/keyence_plc_command_executer.h"
#include "KeyencePLC/keyence_plc_subsystem_helper.h"
#include "PMCavity/fortrend_pm_cavity_defined.h"

namespace FC{

	/**
	* @brief  upload finished command for pm cavity
	*/
	class  PMCavityToPutStationCommand :public std::enable_shared_from_this<PMCavityToPutStationCommand>, public  KernelSubsystemCommand, public KeyencePlcCommandExecuter{
	public:
		DECLARE_PTR(PMCavityToPutStationCommand)
		PMCavityToPutStationCommand(KeyencePlcSubSystemHelper* helper,int stationNum);
		virtual std::string getName()const override { return "ToPutStation"; }
		int stationid;
	protected:
		virtual RunResult onRun() throw(KernelException);
	private:
		DECLARE_PRIVATE(PMCavityToPutStationCommand)
	};




}
#endif