
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
	//if (SIMULATION_TEST == 1)
	//{
	//	return RunResult::RUN_OK;
	//}
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

	std::string command = "SET:SERVOS/ON;";
	std::string error_message = "";
	int error_type = 1;
	int error_code = 0;
	std::string res;
	auto startTime = std::chrono::high_resolution_clock::now();
	auto timeout2 = std::chrono::seconds(30);

	clearRobotMessage();
	sendRequest(command);

	logInform(sub->getName().c_str(), "机械手上使能命令开始");
	Sleep(500);

	res = recvResponseRobotMessage(timeout);
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
			error_message = "机械手上使能超时";
			error_code = 0x100;
			AlarmMessage::Ptr alarm(new AlarmMessage(1, error_code, error_message));
			setAlarm(alarm);
			return RunResult::RUN_FAILD;
		}
		res = recvResponseRobotMessage(timeout);
		Sleep(200);
	}

	if (res != "ACK;" && res != "RPS:SERVOS;")
	{
		logError(sub->getName().c_str(), "上使能命令发生错误");

		std::string error_str = "ERR";
		if (!handleErrorCode(res, error_str, error_type, error_code)) {
			error_type = 5;
			error_code = 1;
			error_message = ("上使能命令执行失败，机械手返回的指令未定义：%s.", res);
			logError(sub->getName().c_str(), "上使能命令执行失败，机械手返回的指令未定义：%s", res);
		}
		else
		{
			logError(sub->getName().c_str(), "执行上使能时存在一个错误");
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
		res = recvResponseRobotMessage(timeout);

		auto startTime2 = std::chrono::high_resolution_clock::now();
		auto timeout3 = std::chrono::seconds(30);

		while (true)
		{
			auto currentTime = std::chrono::high_resolution_clock::now();
			auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime2);

			if (res != std::string("ACK;") && (!res.empty()))
			{
				break;
			}
			if (elapsed >= timeout3)
			{
				error_message = "机械手上使能返回指令超时";
				error_code = 0x100;
				AlarmMessage::Ptr alarm(new AlarmMessage(1, error_code, error_message));
				setAlarm(alarm);
				return RunResult::RUN_FAILD;
			}
			res = recvResponseRobotMessage(timeout);
			Sleep(200);
		}


		std::string recvMessage = "RPS:SERVOS;";
		auto found = search(res.begin(), res.end(), recvMessage.begin(), recvMessage.end());

		if (found != res.end())
		{
			//找到了
			if (!sendRequest("ACK;"))
			{
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_COMMUNICATION_ERROR,
					Poco::format("%s 机械手通讯错误", sub->getName()), this);
			}
			logInform(sub->getName().c_str(), "机械手上使能命令执行结束");
		}

	}
	//ARM A
	command = "QRY:LOAD/A;";
	clearRobotMessage();
	sendRequest(command);
	res = recvResponseRobotMessage(timeout);

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
			error_message = "查询机械手有无片子超时";
			error_code = 0x100;
			AlarmMessage::Ptr alarm(new AlarmMessage(1, error_code, error_message));
			setAlarm(alarm);
			return RunResult::RUN_FAILD;
		}
		res = recvResponseRobotMessage(timeout);
		logInform(sub->getName().c_str(), "查询A机械手有无片子ACK：%s", res.c_str());
		Sleep(10);
	}
	logInform(sub->getName().c_str(), "res：%s", res);

	//res = recvResponseRobotMessage(timeout);
	//logInform(sub->getName().c_str(), "查询机械手有无片子RPS：%s", res);

	//等待机械手返回指令
	auto startTime2 = std::chrono::high_resolution_clock::now();
	auto timeout3 = std::chrono::seconds(10);

	while (true)
	{
		auto currentTime = std::chrono::high_resolution_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime2);

		if (res != std::string("ACK;") && !res.empty())
		{
			break;
		}
		if (elapsed >= timeout3)
		{
			error_message = "查询A机械手有无片子指令超时";
			error_code = 0x100;
			AlarmMessage::Ptr alarm(new AlarmMessage(1, error_code, error_message));
			setAlarm(alarm);
			return RunResult::RUN_FAILD;
		}
		res = recvResponseRobotMessage(timeout);
		logInform(sub->getName().c_str(), "查询B机械手有无片子RPS：%s", res.c_str());
		Sleep(20);
	}

	if (res.find("RPS:LOAD/ON;") != std::string::npos)
	{
		sub->setObject(0, true);
		sub_cass->setMapping(1, Cassette::Mapping::Present);
	}
	else if (res.find("RPS:LOAD/OFF;") != std::string::npos)
	{
		sub->setObject(0, false);
		sub_cass->setMapping(1, Cassette::Mapping::Empty);
	}
	else
	{
		sub->setObject(0, false);
		sub_cass->setMapping(1, Cassette::Mapping::Unknown);
		throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_COMMUNICATION_ERROR, Poco::format("工位: %s 获取手臂A状态通讯错误.", sub->getName()), this);
	}

	//ARM B
	command = "QRY:LOAD/B;";
	clearRobotMessage();
	sendRequest(command);
	res = recvResponseRobotMessage(timeout);

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
			error_message = "查询B机械手有无片子超时";
			error_code = 0x100;
			AlarmMessage::Ptr alarm(new AlarmMessage(1, error_code, error_message));
			setAlarm(alarm);
			return RunResult::RUN_FAILD;
		}
		res = recvResponseRobotMessage(timeout);
		logInform(sub->getName().c_str(), "查询B机械手有无片子ACK：%s", res.c_str());
		Sleep(10);
	}
	logInform(sub->getName().c_str(), "res：%s", res.c_str());

	auto startTime3 = std::chrono::high_resolution_clock::now();
	auto timeout4 = std::chrono::seconds(10);

	while (true)
	{
		auto currentTime = std::chrono::high_resolution_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime3);

		if (res != std::string("ACK;") && !res.empty())
		{
			break;
		}
		if (elapsed >= timeout4)
		{
			error_message = "查询B机械手有无片子指令超时";
			error_code = 0x100;
			AlarmMessage::Ptr alarm(new AlarmMessage(1, error_code, error_message));
			setAlarm(alarm);
			return RunResult::RUN_FAILD;
		}
		res = recvResponseRobotMessage(timeout);
		logInform(sub->getName().c_str(), "查询B机械手有无片子RPS：%s", res.c_str());
		Sleep(20);
	}

	if (res.find("RPS:LOAD/ON;") != std::string::npos)
	{
		sub->setObject(1, true);
		sub_cass->setMapping(2, Cassette::Mapping::Present);
	}
	else if (res.find("RPS:LOAD/OFF;") != std::string::npos)
	{
		sub->setObject(1, false);
		sub_cass->setMapping(2, Cassette::Mapping::Empty);
	}
	else
	{
		sub->setObject(1, false);
		sub_cass->setMapping(2, Cassette::Mapping::Unknown);
		throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_COMMUNICATION_ERROR, Poco::format("工位: %s 获取手臂B状态通讯错误.", sub->getName()), this);
	}

	////调用查询指令
	//auto cmd1 = sub->createCheckLoadCommand(0, 2); //1手
	//sub->startCommand(cmd1);
	//cmd1->wait();


	//auto cmd2 = sub->createCheckLoadCommand(1, 2); //1手
	//sub->startCommand(cmd2);
	//cmd2->wait();


	return RunResult::RUN_OK;

}

}
