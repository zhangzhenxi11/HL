/**
* @file     fortrend_sunwayrobot_ready_get_wafer_command.h
* @brief    ready get wafer command for sunway robot
* @author   kai
*/

// Library: Fortrend
// Package: SubSystem/SunwayRobot

#ifndef _XLH_FORTREND_SUNWAYROBOT_READY_GET_WAFER_INCLUDE_
#define _XLH_FORTREND_SUNWAYROBOT_READY_GET_WAFER_INCLUDE_ 

#include "kernel/kernel_subsystem_command.h"
#include "Kernel/Fortrend/robot_getwafer_abstract_command.h"


#include "SunwayRobot/sunway_subsystem_helper.h"
#include "SunwayRobot/sunway_command_executer.h"

namespace FC{

	/**
	* @brief ready_get_wafer command for SunwayRobot
	*/
	class  SunwayRobotReadyGetWaferCommand : public  RobotAbstractReadyGetWaferCommand, public SunwayCommandExecuter
	{
	public:
		DECLARE_PTR(SunwayRobotReadyGetWaferCommand)
		SunwayRobotReadyGetWaferCommand(const std::shared_ptr<FortrendStation>&  station, int arm, int slot, SunwaySubSystemHelper* helper);
		virtual std::string getName()const override { return "ReadyGetWafer"; }
	protected:
		virtual RunResult onRun() throw(KernelException);

	private:
		DECLARE_PRIVATE(SunwayRobotReadyGetWaferCommand)
	};

}
#endif

