/**
* @file     fortrend_sunwayrobot_reset_command.h
* @brief    reset command for sunway robot
* @author   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/SunwayRobot

#ifndef _XLH_FORTREND_SUNWAYROBOT_RESET_INCLUDE_
#define _XLH_FORTREND_SUNWAYROBOT_RESET_INCLUDE_ 

#include "Kernel/Fortrend/hex_command_executer.h"
#include "kernel/kernel_subsystem_command.h"
#include "kernel/kernel_subsystem_reset_command.h"

#include "SunwayRobot/sunway_subsystem_helper.h"
#include "SunwayRobot/sunway_command_executer.h"

namespace FC{

/**
* @brief reset command for SunwayRobot
*/
	class  SunwayRobotResetCommand : public  KernelSubsystemResetCommand, public SunwayCommandExecuter
{
public:
	DECLARE_PTR(SunwayRobotResetCommand)
	SunwayRobotResetCommand(SunwaySubSystemHelper* helper);

	virtual std::string getName()const override {return "Reset";}
protected:
	virtual RunResult onRun() throw(KernelException);

private:
	DECLARE_PRIVATE(SunwayRobotResetCommand)
};

}
#endif

