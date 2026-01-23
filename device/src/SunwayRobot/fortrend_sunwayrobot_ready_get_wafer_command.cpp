/**
* @file     fortrend_sunwayrobot_ready_get_wafer_command.h
* @brief    ready_get_wafer command for SunwayRobot
* @author   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/SunwayRobot

#include "SunwayRobot/fortrend_sunwayrobot_ready_get_wafer_command.h"
#include "SunwayRobot/fortrend_sunwayrobot_subsystem.h"
#include "kernel/kernel_command_reject_exception.h"

#include "Poco/Format.h"

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif


namespace FC{

/**
* SunwayRobotReadyGetWaferCommandPrivate
*/
class SunwayRobotReadyGetWaferCommandPrivate{
public:
	
};

/**
* SunwayRobotReadyGetWaferCommand
*/
SunwayRobotReadyGetWaferCommand::SunwayRobotReadyGetWaferCommand(const std::shared_ptr<FortrendStation>&  station, int arm, int slot, SunwaySubSystemHelper* helper)
	:SunwayCommandExecuter(helper)
	, RobotAbstractReadyGetWaferCommand(station, arm, slot)
	, d(new SunwayRobotReadyGetWaferCommandPrivate){
	//setMessageName("Ready_get_wafer");
	//setDescription("Ready_get_wafer on SunwayRobot");

}
std::vector<IKernelResources*> SunwayRobotReadyGetWaferCommand::resources() const
{
	std::vector<IKernelResources* > ret = KernelSubsystemCommand::resources();
	ret.push_back(getStation().get());
	return ret;
}

SunwayRobotReadyGetWaferCommand::RunResult SunwayRobotReadyGetWaferCommand::performRobotOperation(
	const std::function<std::string()>& commandBuilder,
	const std::function<bool()>& onSuccess)
{
	return RunResult::RUN_OK;

}
bool SunwayRobotReadyGetWaferCommand::updateWaferMapping()
{

	return true;
}

SunwayRobotReadyGetWaferCommand::RunResult SunwayRobotReadyGetWaferCommand::robotRobotOperation(const std::function<std::string()>& commandBuilder)
{

	return RunResult::RUN_OK;
}


SunwayRobotReadyGetWaferCommand::RunResult SunwayRobotReadyGetWaferCommand::onRun() throw(KernelException){
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
