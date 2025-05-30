
/**
* @file     fortrend_sunwayrobot_update_command.h
* @brief    update command for SunwayRobot
* @author   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/SunwayRobot

#include "Poco/Format.h"

#include "Kernel/kernel.h"
#include "kernel/kernel_command_reject_exception.h"
#include "Kernel/Fortrend/fortrend_cassette_manager.h"

#include "SunwayRobot/fortrend_sunwayrobot_update_command.h"
#include "SunwayRobot/fortrend_sunwayrobot_subsystem.h"
#include "SunwayRobot/fortrend_sunwayrobot_rq_load_command.h"

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

namespace FC{

/**
* SunwayRobotUpdateCommandPrivate
*/
class SunwayRobotUpdateCommandPrivate{
public:
	
};

/**
* SunwayRobotUpdateCommand
*/
SunwayRobotUpdateCommand::SunwayRobotUpdateCommand(SunwaySubSystemHelper* helper)
	:SunwayCommandExecuter(helper)
	, d(new SunwayRobotUpdateCommandPrivate){
	//setMessageName("Update");
	//setDescription("Update on SunwayRobot");

};



SunwayRobotUpdateCommand::RunResult SunwayRobotUpdateCommand::onRun() throw(KernelException){
	FortrendSunwayRobotSubsystem* sub = dynamic_cast<FortrendSunwayRobotSubsystem*>(getSubsystem());
	//
	if (!sub){
		throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "subsystem type error.", this);
	}
	if (sub->getBusyState())
	{
		throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_BUSY, Poco::format("%s 处于忙碌中.", sub->getName()), this);
	}
	//check modules
	auto cassManager = sub->getKernel()->getKernelModule<FortrendCassetteManager>();
	//get cass
	auto sub_cass = cassManager->getCassette(sub);
	if (!sub_cass){
		throw KernelCommandRejectException(__FILE__, KernelSysException::KR_STATION_WITHOUT_CASS_EXCEPTION, Poco::format("工位: %s 扫描结果不存在.", sub->getName()), this);
	}
	std::shared_ptr<KernelConfiguration> command_config = sub->getConfigure()->createView(getName());
	int timeout = command_config->getInt("timeout", 100000);
	if (timeout < 10){
		throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_DATA_OUTOF_RANGE, Poco::format("超时: %s 获取状态超时参数设置错误", sub->getName()), this);
	}
	//ARM A
	std::string command = "QRY:LOAD/A;";
	sendRequest(command);
	std::string res = recvResponse(timeout);
	if (res == "RPS:LOAD/ON;")
	{
		sub->setObject(0, true);
		sub_cass->setMapping(1, Cassette::Mapping::Present);
	}
	else if (res == "RPS:LOAD/OFF;")
	{
		sub->setObject(0, false);
		sub_cass->setMapping(1, Cassette::Mapping::Empty);
	}
	else
	{
		throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_COMMUNICATION_ERROR, Poco::format("工位: %s 获取手臂A状态通讯错误.", sub->getName()), this);
	}
	//ARM B
	command = "QRY:LOAD/B;";
	sendRequest(command);
	res = recvResponse(timeout);
	if (res == "RPS:LOAD/ON;")
	{
		sub->setObject(1, true);
		sub_cass->setMapping(2, Cassette::Mapping::Present);
	}
	else if (res == "RPS:LOAD/OFF;")
	{
		sub->setObject(1, false);
		sub_cass->setMapping(2, Cassette::Mapping::Empty);
	}
	else
	{
		throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_COMMUNICATION_ERROR, Poco::format("工位: %s 获取手臂B状态通讯错误.", sub->getName()), this);
	}

	return RunResult::RUN_OK;

}

}
