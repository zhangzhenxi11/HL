/**
* @file     fortrend_sunwayrobot_clear_error_command.h
* @brief    clear error command for SunwayRobot
* @author   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/SunwayRobot
#include "Kernel/kernel_log.h"

#include "SunwayRobot/fortrend_sunwayrobot_clear_error_command.h"
#include "SunwayRobot/fortrend_sunwayrobot_subsystem.h"
#include "kernel/kernel_command_reject_exception.h"

#include "SunwayRobot/fortrend_sunwayrobot_subsystem.h"

#include "Poco/Format.h"

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif


namespace FC{

	/**
	* SunwayRobotClearErrorCommandPrivate
	*/
	class SunwayRobotClearErrorCommandPrivate{
	public:

	};

	/**
	* SunwayRobotClearErrorCommand
	*/
	SunwayRobotClearErrorCommand::SunwayRobotClearErrorCommand(SunwaySubSystemHelper* helper)
		:SunwayCommandExecuter(helper)
		, d(new SunwayRobotClearErrorCommandPrivate){
		//setMessageName("ClearError");
		//setDescription("ClearError on SunwayRobot");

	};



	SunwayRobotClearErrorCommand::RunResult SunwayRobotClearErrorCommand::onRun() throw(KernelException){
		FortrendSunwayRobotSubsystem* robot = dynamic_cast<FortrendSunwayRobotSubsystem*>(getSubsystem());
		//
		if (!robot){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, "子系统类型错误", this);
		}
		if (!robot->getHasResetFlag())
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, Poco::format("%s请先将机械手复位", robot->getName()).c_str(), this);
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
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_DATA_OUTOF_RANGE, Poco::format("超时: %s 清除错误超时参数设置错误", robot->getName()), this);
		}

		logInform(getName().c_str(), "清除错误命令开始执行.");

		auto cmd_reset = robot->createResetCommand();
		robot->startCommand(cmd_reset);
		cmd_reset->wait();

		if (cmd_reset->hasError())
		{
			//set alarm data
			AlarmMessage::Ptr alarm(new AlarmMessage(0, 0, "清除错误命令执行失败！"));
			setAlarm(alarm);
			return RunResult::RUN_FAILD;
		}

#if 0
		std::string res;
		std::string error_message;
		int error_code = 0;
		int error_type = 1;

		sendRequest("MOV:HOME;");
		res = recvResponse(timeout);

		if (res != std::string("ACK;")){

			std::string error_str = "ERR";
			if (!handleErrorCode(res, error_str, error_type, error_code)) {
				error_type = 5;
				error_code = 1;
				error_message = ("移动至机械手的原点位置命令执行失败，机械手返回的指令未定义：%s.", res);
				logError(robot->getName().c_str(), "移动至机械手的原点位置命令执行失败，机械手返回的指令未定义：%s", res);
			}
			else
			{
				logError(robot->getName().c_str(), "执行移动至机械手的原点位置时存在一个错误.");
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
			Sleep(500);
			logInform(getName().c_str(), "清除命令执行结束.");
		}
#endif
		return RunResult::RUN_OK;

	}

}
