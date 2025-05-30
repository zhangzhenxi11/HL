/**
* @file     fortrend_sunwayrobot_set_speed_command.h
* @brief    set_speed command for Sunway Robot
* @author   kai
*/

// Library: Fortrend
// Package: SubSystem/SunwayRobot

#include "kernel/kernel_log.h"
#include "kernel/kernel_command_reject_exception.h"

#include "SunwayRobot/fortrend_sunwayrobot_subsystem.h"
#include "SunwayRobot/fortrend_sunwayrobot_set_speed_command.h"

#include "Poco/Format.h"

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif


namespace FC{

	/**
	*SunwayRobotSetSpeedCommandPrivate
	*/
	class SunwayRobotSetSpeedCommandPrivate{
public:
	uint8_t percentage;
	};

	/**
	*SunwayRobotSetSpeedCommand
	*/
	SunwayRobotSetSpeedCommand::SunwayRobotSetSpeedCommand(uint8_t percentage, SunwaySubSystemHelper* hexHelper)
		:SunwayCommandExecuter(hexHelper)
		, d(new SunwayRobotSetSpeedCommandPrivate){
		d->percentage = percentage;
	};

	int SunwayRobotSetSpeedCommand::getSpeed(){
		return d->percentage;
	}

	SunwayRobotSetSpeedCommand::RunResult SunwayRobotSetSpeedCommand::onRun() throw(KernelException){
		FortrendSunwayRobotSubsystem* sub = dynamic_cast<FortrendSunwayRobotSubsystem*>(getSubsystem());
		//
		if (!sub){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, "子系统类型错误.", this);
		}
		if (sub->getBusyState())
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_BUSY, Poco::format("%s 处于忙碌中.", sub->getName()), this);
		}
		//get command configure
		std::shared_ptr<KernelConfiguration> command_config = sub->getConfigure()->createView(getName());

		//fill params
		
		int timeout = command_config->getInt("timeout", 100000);
		if (timeout < 10){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_DATA_OUTOF_RANGE, 
				Poco::format("超时: %s 设置速度超时参数设置错误", sub->getName()), this);
		}
		//SET:RUN_SPEED/50;
		std::string macro = "SET:RUN_SPEED/";
		macro.append(std::to_string(d->percentage));
		macro.append(";");
		logInform(sub->getName().c_str(), "设置速度命令执行开始");
		//ALG
		sendRequest(macro);
		
		std::string res = recvResponse(timeout);
		std::string error_message;
		int error_code = 0;
		int error_type = 1;

		if (res == "ACK;"){

			auto startTime = std::chrono::high_resolution_clock::now();
			auto timeout2 = std::chrono::seconds(10);

			res = recvResponse(timeout);
			while (true)
			{
				auto currentTime = std::chrono::high_resolution_clock::now();
				auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime);

				if (!res.empty())
				{
					break;
				}
				if (elapsed >= timeout2)
				{
					error_message = "机械手设置速度超时";
					error_code = 0x102;
					AlarmMessage::Ptr alarm(new AlarmMessage(1, error_code, error_message));
					setAlarm(alarm);
					return RunResult::RUN_FAILD;
				}
				res = recvResponse(timeout);
				Sleep(200);
			}
			std::string recvMessage = "RPS:RUN_SPEED;";
			auto found = search(res.begin(), res.end(), recvMessage.begin(), recvMessage.end());
			if (found != res.end())
			{

				if (!sendRequest("ACK;"))
				{
					throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_COMMUNICATION_ERROR,
						Poco::format("%s 机械手通讯错误", sub->getName()), this);
				}
			}
			logInform(sub->getName().c_str(), "设置速度命令执行完成");
			return RunResult::RUN_OK;
		}
		else
		{
			std::string error_str = "ERR";
			if (!handleErrorCode(res, error_str, error_type, error_code)) {
				error_type = 5;
				error_code = 1;
				error_message = ("设置速度命令执行失败，机械手返回的指令未定义：%s.", res);
				logError(sub->getName().c_str(), "设置速度命令执行失败，机械手返回的指令未定义：%s", res);
			}
			else
			{
				logError(sub->getName().c_str(), "设置速度命令时存在一个错误");
				auto error_strucct = getErrorCode(error_type, error_code);
				error_type = error_strucct->type;
				error_code = error_strucct->code;
				error_message = error_strucct->message;
			}
			//set alarm data
			AlarmMessage::Ptr alarm(new AlarmMessage(error_type, error_code, error_message));
			setAlarm(alarm);
		}
		return RunResult::RUN_FAILD;

	}

}
