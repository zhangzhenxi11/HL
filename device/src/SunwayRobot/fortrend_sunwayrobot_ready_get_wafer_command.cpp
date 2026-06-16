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
#include "Kernel/kernel_block_manager.h"
#include "Kernel/kernel_log.h"
#include "Kernel/kernel_event_paramters.h"
#include "LoadLock/fortrend_loadlock_subsystem.h"
#include "PMCavity/fortrend_pm_cavity_subsystem.h"

#include "Poco/Format.h"
#include "fortrend_device_kernel.h"

#include <algorithm>
#include <chrono>
#include <functional>
#include <mutex>

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
//std::vector<IKernelResources*> SunwayRobotReadyGetWaferCommand::resources() const
//{
//	std::vector<IKernelResources* > ret = KernelSubsystemCommand::resources();
//	ret.push_back(getStation().get());
//	return ret;
//}

SunwayRobotReadyGetWaferCommand::RunResult SunwayRobotReadyGetWaferCommand::performRobotOperation(
	const std::function<std::string()>& commandBuilder,
	const std::function<bool()>& onSuccess)
{
	if (robotRobotOperation(commandBuilder) == RunResult::RUN_OK)
	{
		if (!onSuccess())
		{
			FortrendSunwayRobotSubsystem* robot = dynamic_cast<FortrendSunwayRobotSubsystem*>(getSubsystem());
			if (!robot) {
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "子系统类型错误", this);
			}
			logError(robot->getName().c_str(), "准备取晶圆后更新状态失败.");
			AlarmMessage::Ptr alarm(new AlarmMessage(1, 0x101, "机械手准备取晶圆后更新状态失败."));
			setAlarm(alarm);
			return RunResult::RUN_FAILD;
		}
		return RunResult::RUN_OK;
	}
	return RunResult::RUN_FAILD;
}
bool SunwayRobotReadyGetWaferCommand::updateWaferMapping()
{
	return true;
}

SunwayRobotReadyGetWaferCommand::RunResult SunwayRobotReadyGetWaferCommand::robotRobotOperation(const std::function<std::string()>& commandBuilder)
{
	FortrendSunwayRobotSubsystem* robot = dynamic_cast<FortrendSunwayRobotSubsystem*>(getSubsystem());
	if (!robot) {
		throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "子系统类型错误", this);
	}

	std::shared_ptr<KernelConfiguration> command_config = robot->getConfigure()->createView(getName());
	KernelCommandParameter parameter(shared_from_this());
	int timeout = command_config->getInt("timeout", 100000);
	if (timeout < 10) {
		throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_DATA_OUTOF_RANGE,
			Poco::format("超时: %s 准备取晶圆超时参数错误.", robot->getName()), this);
	}

	std::string command = commandBuilder();
	std::string verificationMessage = "RPS:RETREATLPOS;";
	std::string error_message = "准备取晶圆命令执行失败.";
	int error_type = 1;
	int error_code = 0;

	logInform(robot->getName().c_str(), "准备取晶圆命令开始:command:%s", command.c_str());
	robot->sendEvent(NEW_EVENT_ID_WITHNAME(EVENT_COMMAND_RUNNING), &parameter);

	if (!sendRequest(command))
	{
		AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE,
			KernelSysException::KR_MODULE_COMMUNICATION_ERROR,
			Poco::format("%s 机械手通讯错误.", robot->getName())));
		setAlarm(alarm);
		return RunResult::RUN_FAILD;
	}

	const std::string ackPrefix = "ACK:" + command.substr(command.find(':') + 1);
	const std::string commandContext = "ReadyGet-" + std::to_string(getStation()->getStationId(robot->getName())) + "-" + std::to_string(getSlot());
	std::string res = recvResponseRobotMessageMatching(timeout,
		{ ackPrefix, verificationMessage, "ERR", "NAK" }, commandContext);

	auto startTime = std::chrono::high_resolution_clock::now();
	auto timeoutWindow = std::chrono::seconds(120);
	while (true)
	{
		auto currentTime = std::chrono::high_resolution_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime);

		if (!res.empty())
		{
			break;
		}
		if (elapsed >= timeoutWindow)
		{
			AlarmMessage::Ptr alarm(new AlarmMessage(1, 0x100, error_message));
			setAlarm(alarm);
			return RunResult::RUN_FAILD;
		}
		res = recvResponseRobotMessageMatching(timeout,
			{ ackPrefix, verificationMessage, "ERR", "NAK" }, commandContext);
		Sleep(10);
	}
	logInform(robot->getName().c_str(), "准备取晶圆ACK:%s", res.c_str());

	if (res.find("ACK") == std::string::npos)
	{
		std::string error_str = "ERR";
		if (!handleErrorCode(res, error_str, error_type, error_code)) {
			error_type = 5;
			error_code = 1;
			error_message += Poco::format(",机械手返回的指令未定义:%s", res);
		}
		else
		{
			auto error_struct = getErrorCode(error_type, error_code);
			error_type = error_struct->type;
			error_code = error_struct->code;
			error_message = error_struct->message;
		}
		AlarmMessage::Ptr alarm(new AlarmMessage(error_type, error_code, error_message));
		setAlarm(alarm);
		return RunResult::RUN_FAILD;
	}

	res = recvResponseRobotMessageMatching(timeout,
		{ verificationMessage, "ERR", "NAK" }, commandContext + "-RPS");
	auto startTime2 = std::chrono::high_resolution_clock::now();
	while (true)
	{
		auto currentTime = std::chrono::high_resolution_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime2);

		if (res.find("ACK") == std::string::npos && !res.empty())
		{
			break;
		}
		if (elapsed >= timeoutWindow)
		{
			AlarmMessage::Ptr alarm(new AlarmMessage(1, 0x100, "准备取晶圆返回指令超时."));
			setAlarm(alarm);
			return RunResult::RUN_FAILD;
		}
		res = recvResponseRobotMessageMatching(timeout,
			{ verificationMessage, "ERR", "NAK" }, commandContext + "-RPS");
		Sleep(10);
	}
	logInform(robot->getName().c_str(), "准备取晶圆RPS:%s", res.c_str());

	auto found = search(res.begin(), res.end(), verificationMessage.begin(), verificationMessage.end());
	if (found != res.end())
	{
		if (!sendRequest("ACK;"))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_COMMUNICATION_ERROR,
				Poco::format("%s 机械手通讯错误.", robot->getName()), this);
		}
		return RunResult::RUN_OK;
	}

	std::string error_str = "ERR";
	if (!handleErrorCode(res, error_str, error_type, error_code)) {
		error_type = 5;
		error_code = 1;
		error_message += Poco::format(",机械手返回的指令未定义:%s", res);
	}
	else
	{
		auto error_struct = getErrorCode(error_type, error_code);
		error_type = error_struct->type;
		error_code = error_struct->code;
		error_message = error_struct->message;
	}
	AlarmMessage::Ptr alarm(new AlarmMessage(error_type, error_code, error_message));
	setAlarm(alarm);
	return RunResult::RUN_FAILD;
}


SunwayRobotReadyGetWaferCommand::RunResult SunwayRobotReadyGetWaferCommand::onRun() throw(KernelException){
	FortrendSunwayRobotSubsystem* robot = dynamic_cast<FortrendSunwayRobotSubsystem*>(getSubsystem());
	if (!robot){
		throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, "subsystem type error.", this);
	}

	std::lock_guard<std::mutex> lock(robot->robot_mutex);

	if (robot->getState() != IKernelSubSystem::State::SUB_NORMAL){
		throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_STATE_EXCEPTION, Poco::format("%s 不在正常状态.", robot->getName()), this);
	}
	if (auto sub = std::dynamic_pointer_cast<KernelAbstractSubSystem>(getStation())){
		if (sub->getState() != IKernelSubSystem::SUB_NORMAL){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_STATE_EXCEPTION, Poco::format("工位： %s 不在正常状态.", getStation()->getName()), this);
		}
	}
	if (robot->getBusyState())
	{
		throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_BUSY, Poco::format("%s 处于忙碌中.", robot->getName()), this);
	}

	//if (auto sub = std::dynamic_pointer_cast<FortrendAbstractStation>(getStation()))
	//{
	//	if (!sub->hasDoorOpend())
	//	{
	//		throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_DOOR_EXCEPTION,
	//			Poco::format("工位： %s 当前门阀处于关闭状态（逻辑错误）.", getStation()->getName()), this);
	//	}
	//}

	/*std::shared_ptr<KernelConfiguration> command_config = robot->getConfigure()->createView(getName());
	if (getStation()->getName().find("PM") != std::string::npos)
	{
		if (auto sub = std::dynamic_pointer_cast<FortrendPMCavitySubsystem>(getStation())){
			if (!sub->getPMCavitySafeSignal())
			{
				logInform(sub->getName().c_str(), "PM腔未检测到安全信号 %d ,延迟50ms重新检测.", sub->getPMCavitySafeSignal());
				Sleep(50);
				if (!sub->getPMCavitySafeSignal())
				{
					throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR,
						Poco::format("%s腔未发出安全信号.", getStation()->getName()).c_str(), this);
				}
			}
		}
	}
	else
	{
		if (auto sub = std::dynamic_pointer_cast<FortrendLoadLockSubsystem>(getStation()))
		{
			if (!sub->getLoadLockCavitySafeSignal())
			{
				logInform(sub->getName().c_str(), "Loadlock腔未检测到安全信号 %d ,延迟50ms重新检测.", sub->getLoadLockCavitySafeSignal());
				Sleep(50);
				if (!sub->getLoadLockCavitySafeSignal())
				{
					throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR,
						Poco::format("%s腔未发出安全信号.", getStation()->getName()).c_str(), this);
				}
			}
		}
	}*/

	std::string str_arm = (getArm() == 0) ? "A" : "B";
	RunResult result = performRobotOperation(
		[this, robot, str_arm]() -> std::string {
			std::string command = "MOV:RETREATLPOS/";
			command.append(std::to_string(getStation()->getStationId(robot->getName())));
			command.append("/");
			command.append(std::to_string(getSlot()));
			command.append("/");
			command.append(str_arm);
			command.append(";");
			return command;
		},
		[this]() -> bool {
			return updateWaferMapping();
		}
	);

	robot->getKernel()->getKernelBlockManager()->releaseBlock(robot);
	return result;

}

}
