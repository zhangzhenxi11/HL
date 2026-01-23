/**
* @file     fortrend_sunwayrobot_set_load_command.h
* @brief    set load command for sunway robot
* @author   kai
*/

// Library: Fortrend
// Package: SubSystem/SunwayRobot

#ifndef _XLH_FORTREND_SUNWAYROBOT_SET_LOAD_INCLUDE_
#define _XLH_FORTREND_SUNWAYROBOT_SET_LOAD_INCLUDE_ 

#include "kernel/kernel_subsystem_command.h"
#include "Kernel/Fortrend/robot_getwafer_abstract_command.h"

#include "SunwayRobot/sunway_subsystem_helper.h"
#include "SunwayRobot/sunway_command_executer.h"

namespace FC{

	/**
	* @brief set load command for SunwayRobot
	*/
	class  SunwayRobotSetLoadCommand : public  KernelSubsystemCommand, public SunwayCommandExecuter
	{
	public:
		DECLARE_PTR(SunwayRobotSetLoadCommand)
		SunwayRobotSetLoadCommand(int arm, int state,SunwaySubSystemHelper* helper);
		virtual std::string getName()const override { return "SetLoad"; }
	protected:
		virtual RunResult onRun() throw(KernelException);

	private:
		DECLARE_PRIVATE(SunwayRobotSetLoadCommand)
	};

}
#endif
