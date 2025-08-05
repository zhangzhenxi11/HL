/**
* @file     fortrend_sunwayrobot_home_command.h
* @brief    home command for sunway robot
* @author   kai
*/

// Library: Fortrend
// Package: SubSystem/SunwayRobot

#ifndef _XLH_FORTREND_SUNWAYROBOT_HOME_INCLUDE_
#define _XLH_FORTREND_SUNWAYROBOT_HOME_INCLUDE_ 

#include "kernel/kernel_subsystem_command.h"

#include "SunwayRobot/sunway_subsystem_helper.h"
#include "SunwayRobot/sunway_command_executer.h"
namespace FC{

/**
* @brief Home command for SunwayRobot
*/
	class  SunwayRobotHomeCommand : public  KernelSubsystemCommand, public SunwayCommandExecuter
{
public:
	DECLARE_PTR(SunwayRobotHomeCommand)
	SunwayRobotHomeCommand(SunwaySubSystemHelper* helper);
	virtual std::string getName()const override {return "Home";}
	bool IsBusy();
	std::vector<IKernelResources* > resources() const override;
protected:
	virtual RunResult onRun() throw(KernelException);

private:
	DECLARE_PRIVATE(SunwayRobotHomeCommand)
};

}
#endif

