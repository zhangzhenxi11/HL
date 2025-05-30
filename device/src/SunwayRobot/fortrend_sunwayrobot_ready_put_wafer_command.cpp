/**
* @file     fortrend_sunwayrobot_ready_put_wafer_command.h
* @brief    ready_put_wafer command for SunwayRobot
* @author   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/SunwayRobot

#include "SunwayRobot/fortrend_sunwayrobot_ready_put_wafer_command.h"
#include "SunwayRobot/fortrend_sunwayrobot_subsystem.h"
#include "kernel/kernel_command_reject_exception.h"

#include "Poco/Format.h"


namespace FC{

/**
* SunwayRobotReadyPutWaferCommandPrivate
*/
class SunwayRobotReadyPutWaferCommandPrivate{
public:
	
};

/**
* SunwayRobotReadyPutWaferCommand
*/
SunwayRobotReadyPutWaferCommand::SunwayRobotReadyPutWaferCommand(const std::shared_ptr<FortrendStation>&  station, int arm, int slot, SunwaySubSystemHelper* helper)
	:SunwayCommandExecuter(helper)
	, RobotAbstractReadyPutWaferCommand(station, arm, slot)
	, d(new SunwayRobotReadyPutWaferCommandPrivate){
	//setMessageName("Ready_put_wafer");
	//setDescription("Ready_put_wafer on SunwayRobot");

};



SunwayRobotReadyPutWaferCommand::RunResult SunwayRobotReadyPutWaferCommand::onRun() throw(KernelException){
	FortrendSunwayRobotSubsystem* fliper = dynamic_cast<FortrendSunwayRobotSubsystem*>(getSubsystem());
	//
	if (!fliper){
		throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, "subsystem type error.", this);
	}

	//get command configure
	std::shared_ptr<KernelConfiguration> command_config = fliper->getConfigure()->createView(getName());

	AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_RESPONSE_ERROR, "Robot ready command not undefined."));
	setAlarm(alarm);
	return RunResult::RUN_FAILD;
}

}
