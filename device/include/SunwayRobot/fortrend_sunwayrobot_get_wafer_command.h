/**
* @file     fortrend_sunwayrobot_get_wafer_command.h
* @brief    get wafer command for sunway robot
* @author   kai
*/

// Library: Fortrend
// Package: SubSystem/SunwayRobot

#ifndef _XLH_FORTREND_SUNWAYROBOT_GET_WAFER_INCLUDE_
#define _XLH_FORTREND_SUNWAYROBOT_GET_WAFER_INCLUDE_ 

#include "kernel/kernel_subsystem_command.h"
#include "Kernel/Fortrend/robot_getwafer_abstract_command.h"

#include "SunwayRobot/sunway_subsystem_helper.h"
#include "SunwayRobot/sunway_command_executer.h"


namespace FC{

/**
* @brief get_wafer command for SunwayRobot
*/
	class  SunwayRobotGetWaferCommand : public std::enable_shared_from_this<SunwayRobotGetWaferCommand>, public  RobotAbstractGetWaferCommand, public SunwayCommandExecuter
{
public:
	DECLARE_PTR(SunwayRobotGetWaferCommand)
	SunwayRobotGetWaferCommand(const std::shared_ptr<FortrendStation>&  station, int arm, int slot,SunwaySubSystemHelper* helper);
	virtual std::string getName()const override {return "GetWafer";}
	bool IsBusy();
	std::vector<IKernelResources* > resources() const override;
protected:
	virtual RunResult onRun() throw(KernelException);

private:
	DECLARE_PRIVATE(SunwayRobotGetWaferCommand)
};

}
#endif

