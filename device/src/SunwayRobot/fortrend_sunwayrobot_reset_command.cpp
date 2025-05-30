/**
* @file     fortrend_sunwayrobot_reset_command.h
* @brief    reset command for SunwayRobot
* @author   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/SunwayRobot
#include "Kernel/kernel_log.h"

#include "SunwayRobot/fortrend_sunwayrobot_reset_command.h"
#include "SunwayRobot/fortrend_sunwayrobot_subsystem.h"
#include "kernel/kernel_command_reject_exception.h"

#include "SunwayRobot/fortrend_sunwayrobot_subsystem.h"
#include "SunwayRobot/fortrend_sunwayrobot_update_command.h"

#include "Poco/Format.h"

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif


namespace FC{

/**
* SunwayRobotResetCommandPrivate
*/
class SunwayRobotResetCommandPrivate{
public:
	
};

/**
* SunwayRobotResetCommand
*/
SunwayRobotResetCommand::SunwayRobotResetCommand(SunwaySubSystemHelper* helper)
	:SunwayCommandExecuter(helper)
	, d(new SunwayRobotResetCommandPrivate){
	//setMessageName("Reset");
	//setDescription("Reset on SunwayRobot");

};



SunwayRobotResetCommand::RunResult SunwayRobotResetCommand::onRun() throw(KernelException){
	FortrendSunwayRobotSubsystem* robot = dynamic_cast<FortrendSunwayRobotSubsystem*>(getSubsystem());
	//
	if (!robot){
		throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, "子系统类型错误", this);
	}
	if (robot->getBusyState())
	{
		throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_BUSY, Poco::format("%s 处于忙碌中.", robot->getName()), this);
	}
	//get command configure
	std::shared_ptr<KernelConfiguration> command_config = robot->getConfigure()->createView(getName());

	//fill params
	int timeout = command_config->getInt("timeout", 100000);
	if (timeout < 10){
		throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_DATA_OUTOF_RANGE, Poco::format("超时: %s 复位超时参数错误", robot->getName()), this);
	}

	//测试
	logInform(getName().c_str(), "模拟复位命令执行");
	return RunResult::RUN_OK;

	//ALG
	std::string error_message = "";
	int error_type = 1;
	int error_code = 0;

	logInform(getName().c_str(), "复位命令开始执行");
	std::string res;
	//send
	if (!sendRequest("ACT:RESET;"))
	{
		AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_COMMUNICATION_ERROR, Poco::format("%s 机械手通讯错误", robot->getName())));
		setAlarm(alarm);
		return RunResult::RUN_FAILD;
	};
	//sendRequest("HOME ALL");
	res = recvResponse(timeout);
	if (res != std::string("ACK"))
	{
		if (res == std::string("NAK"))
		{
			error_code = 0x0A;
			error_message = "机械手拒接接收此指令，无法执行此命令";
			//set alarm data
			AlarmMessage::Ptr alarm(new AlarmMessage(error_type, error_code, error_message));
			setAlarm(alarm);
			logInform(robot->getName().c_str(), "复位机械手失败,机械手返回：【%s】", res);
			return RunResult::RUN_FAILD;
		}

		std::string error_str = "ERR";
		if (!handleErrorCode(res, error_str, error_type, error_code)) {
			error_type = 5;
			error_code = 1;
			error_message = ("复位命令执行失败，机械手返回的指令未定义：%s.", res);
			logError(robot->getName().c_str(), "复位命令执行失败，机械手返回的指令未定义：%s", res);
		}
		else
		{
			logError(robot->getName().c_str(), "执行取晶圆时存在一个错误");
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
		Sleep(200);
		if (res != std::string("RPS:RESET;"))
		{
			std::string error_str = "ERR";
			int error_type = 1;
			int error_code = 0;
			try {
				if (!handleErrorCode(res, error_str, error_type, error_code)) {

					error_type = 5;
					error_code = 1;
					error_message = ("复位命令执行失败，机械手返回的指令未定义：%s.", res);
					logError(robot->getName().c_str(), "复位命令执行失败，机械手返回的指令未定义：%s", res);
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
			logError(robot->getName().c_str(), "复位机械手失败,机械手返回：【%s】", res);
			return RunResult::RUN_FAILD;

		}
		else
		{
			if (!sendRequest("ACK;"))
			{
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_COMMUNICATION_ERROR,
					Poco::format("%s 机械手通讯错误", robot->getName()), this);
			}
		}

		
		auto cmd_update = robot->createUpdateCommand();
		robot->startCommand(cmd_update);
		cmd_update->wait();
		if (cmd_update->hasError())
		{
			//set alarm data
			AlarmMessage::Ptr alarm(new AlarmMessage(0, 0, "更新手臂状态命令执行失败！"));
			setAlarm(alarm);
			return RunResult::RUN_FAILD;
		}
		robot->setHasResetFlag(true);
		logInform(getName().c_str(), "复位命令执行结束");
	}
	return RunResult::RUN_OK;

}

}
