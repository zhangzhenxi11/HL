/**
* @file     fortrend_sunwayrobot_update_command.h
* @brief    update command for sunway robot
* @author   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/SunwayRobot

#ifndef _XLH_FORTREND_SUNWAYROBOT_UPDATE_INCLUDE_
#define _XLH_FORTREND_SUNWAYROBOT_UPDATE_INCLUDE_ 
#include "Kernel/Fortrend/hex_command_executer.h"
#include "kernel/kernel_subsystem_command.h"
#include "kernel/kernel_subsystem_update_command.h"

#include "SunwayRobot/sunway_subsystem_helper.h"
#include "SunwayRobot/sunway_command_executer.h"

namespace FC{

/**
* @brief update command for SunwayRobot
*/
	class  SunwayRobotUpdateCommand : public  KernelSubsystemUpdateCommand, public SunwayCommandExecuter
{
public:
	DECLARE_PTR(SunwayRobotUpdateCommand)
	SunwayRobotUpdateCommand(SunwaySubSystemHelper* helper);
	virtual std::string getName()const override {return "Update";}
protected:
	virtual RunResult onRun() throw(KernelException);

private:
	DECLARE_PRIVATE(SunwayRobotUpdateCommand)
};

}
#endif

