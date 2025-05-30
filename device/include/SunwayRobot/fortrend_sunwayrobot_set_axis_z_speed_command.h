/**
* @file     fortrend_sunwayrobot_set_axis_z_speed_command.h
* @brief    set_axis_z_speed command for Sunway Robot
* @author   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/SunwayRobot

#ifndef _XLH_FORTREND_SUNWAYROBOT_SET_AXIS_Z_SPEED_INCLUDE_
#define _XLH_FORTREND_SUNWAYROBOT_SET_AXIS_Z_SPEED_INCLUDE_ 

#include "kernel/Fortrend/robot_abstract_command.h"

#include "SunwayRobot/sunway_subsystem_helper.h"
#include "SunwayRobot/sunway_command_executer.h"

namespace FC{

	/**
	* @brief set_speed command for SunwayRobot
	*/
	class SunwayRobotSetAxisZSpeedCommand : public  KernelSubsystemCommand, public SunwayCommandExecuter
	{
	public:
		DECLARE_PTR(SunwayRobotSetAxisZSpeedCommand)
		SunwayRobotSetAxisZSpeedCommand(uint8_t percentage, SunwaySubSystemHelper* hexHelper);
		virtual std::string getName()const override { return "SetAxisZSpeed"; }
		int getSpeed();
	protected:
		virtual RunResult onRun() throw(KernelException);

	private:
		DECLARE_PRIVATE(SunwayRobotSetAxisZSpeedCommand)
	};

}
#endif

