
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

	//HOME
	std::string command = "MOV:HOME;";
	std::string error_message = "";
	int error_type = 1;
	int error_code = 0;

	auto startTime = std::chrono::high_resolution_clock::now();
	auto timeout2 = std::chrono::seconds(30);
	if (!sendRequest(command))
	{
		AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE,
			KernelSysException::KR_MODULE_COMMUNICATION_ERROR,
			Poco::format("%s 机械手通讯错误", sub->getName())));
		setAlarm(alarm);
		return RunResult::RUN_FAILD;
	};
	logInform(sub->getName().c_str(), "机械手HOME命令开始");
	Sleep(500);
	std::string res = recvResponse(timeout);

	while (true)
	{
		auto currentTime = std::chrono::high_resolution_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime);

		if (res != std::string(""))
		{
			break;
		}
		if (elapsed >= timeout2)
		{
			error_message = "机械手HOME超时";
			error_code = 0x100;
			AlarmMessage::Ptr alarm(new AlarmMessage(1, error_code, error_message));
			setAlarm(alarm);
			return RunResult::RUN_FAILD;
		}
		res = recvResponse(timeout);
		Sleep(200);
	}


	if (res != std::string("ACK;"))
	{
		logError(sub->getName().c_str(), "机械手HOME时存在一个错误");
		int error_type = 1;
		int error_code = 0;
		std::string error_message;
		std::string error_str = "ERR";
		if (!handleErrorCode(res, error_str, error_type, error_code)) {
			error_type = 5;
			error_code = 1;
			error_message = ("机械手HOME命令执行失败，机械手返回的指令未定义：%s.", res);
			logError(sub->getName().c_str(), "机械手HOME命令执行失败，机械手返回的指令未定义：%s", res);
		}
		else
		{
			logError(sub->getName().c_str(), "执行机械手HOME时存在一个错误");
			auto error_strucct = getErrorCode(error_type, error_code);
			error_type = error_strucct->type;
			error_code = error_strucct->code;
			error_message = error_strucct->message;
		}
		//set alarm data
		AlarmMessage::Ptr alarm(new AlarmMessage(error_type, error_code, error_message));
		setAlarm(alarm);
		return RunResult::RUN_FAILD;
	}

	else
	{
		auto startTime2 = std::chrono::high_resolution_clock::now();
		auto timeout3 = std::chrono::seconds(60);

		while (true)
		{
			auto currentTime = std::chrono::high_resolution_clock::now();
			auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime2);

			if (res != std::string("ACK;"))
			{
				break;
			}
			if (elapsed >= timeout3)
			{
				error_message = "机械手HOME返回指令超时";
				error_code = 0x100;
				AlarmMessage::Ptr alarm(new AlarmMessage(1, error_code, error_message));
				setAlarm(alarm);
				return RunResult::RUN_FAILD;
			}
			res = recvResponse(timeout);
			Sleep(200);
		}

		std::string recvMessage = "RPS:HOME;";
		auto found = search(res.begin(), res.end(), recvMessage.begin(), recvMessage.end());
		if (found != res.end())
		{
			if (!sendRequest("ACK;"))
			{
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_COMMUNICATION_ERROR,
					Poco::format("%s 机械手通讯错误", sub->getName()), this);
			}
		}
		else
		{

			std::string error_str = "ERR";
			try {
				if (!handleErrorCode(res, error_str, error_type, error_code)) {

					error_type = 5;
					error_code = 1;
					error_message = ("机械手HOME命令执行失败，机械手返回的指令未定义：%s.", res);
					logError(sub->getName().c_str(), "机械手HOME命令执行失败，机械手返回的指令未定义：%s", res);
				}
				else
				{
					auto error_strucct = getErrorCode(error_type, error_code);
					error_type = error_strucct->type;
					error_code = error_strucct->code;
					error_message = error_strucct->message;
				}
			}
			catch (const std::invalid_argument& e) {
				error_message = "处理字符串失败";
				logError(getName().c_str(), "处理字符串失败");
			}

			//set alarm data
			AlarmMessage::Ptr alarm(new AlarmMessage(error_type, error_code, error_message));
			setAlarm(alarm);
			logError(sub->getName().c_str(), "机械手HOME失败,机械手返回：【%s】", res);
			return RunResult::RUN_FAILD;

		}

		return RunResult::RUN_OK;
	}

}

}
