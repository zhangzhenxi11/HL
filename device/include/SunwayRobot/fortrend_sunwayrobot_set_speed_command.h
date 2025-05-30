/**
* @file     fortrend_sunwayrobot_set_speed_command.h
* @brief    set_speed command for Sunway Robot
* @author   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/SunwayRobot

#ifndef _XLH_FORTREND_SUNWAYROBOT_SET_SPEED_INCLUDE_
#define _XLH_FORTREND_SUNWAYROBOT_SET_SPEED_INCLUDE_ 

#include "kernel/Fortrend/robot_abstract_command.h"

#include "SunwayRobot/sunway_subsystem_helper.h"
#include "SunwayRobot/sunway_command_executer.h"

namespace FC{

	/**
	* @brief set_speed command for SunwayRobot
	*/
	class SunwayRobotSetSpeedCommand : public  RobotAbstractSetSpeedCommand, public SunwayCommandExecuter
	{
	public:
		DECLARE_PTR(SunwayRobotSetSpeedCommand)
		SunwayRobotSetSpeedCommand(uint8_t percentage, SunwaySubSystemHelper* hexHelper);
		virtual std::string getName()const override { return "SetSpeed"; }
		int getSpeed();
	protected:
		virtual RunResult onRun() throw(KernelException);

	private:
		DECLARE_PRIVATE(SunwayRobotSetSpeedCommand)
	};

}
#endif

