/**
* @file     fortrend_sunwayrobot_reset_command.h
* @brief    reset command for SunwayRobot
* @author   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/SunwayRobot
#include "Kernel/kernel_log.h"

#include "Kernel/kernel.h"
#include "Kernel/kernel_log.h"
#include "Kernel/kernel_block.h"
#include "Kernel/kernel_block_manager.h"
#include "Kernel/kernel_default_block_manager.h"
#include "kernel/kernel_command_reject_exception.h"
#include "kernel/Fortrend/abstract_io_subsystem.h"
#include "Kernel/Fortrend/fortrend_station.h"
#include "Kernel/Fortrend/fortrend_wafer_robot_subsystem.h"
#include "Kernel/Fortrend/fortrend_cassette_manager.h"
#include "Kernel/kernel.h"
#include "Kernel/kernel_action_subsystem.h"
#include "Kernel/kernel_command.h"
#include "kernel/kernel_command_reject_exception.h"


#include "SunwayRobot/fortrend_sunwayrobot_rq_load_command.h"
#include "SunwayRobot/fortrend_sunwayrobot_subsystem.h"
#include "SunwayRobot/fortrend_sunwayrobot_update_command.h"
#include "kernel/kernel_command_reject_exception.h"

#include "Poco/Format.h"

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif


namespace FC{

	/**
	* SunwayRobotRQLoadCommandPrivate
	*/
	class SunwayRobotRQLoadCommandPrivate{
	
	public:
		//std::shared_ptr<FortrendStation> station = 0;
		int arm = 0;
	};

	/**
	* SunwayRobotRQLoadCommand
	*/
	SunwayRobotRQLoadCommand::SunwayRobotRQLoadCommand(int arm, SunwaySubSystemHelper* helper)
		:SunwayCommandExecuter(helper)
		, d(new SunwayRobotRQLoadCommandPrivate){
		//setMessageName("RQLoad");
		//setDescription("RQLoad on SunwayRobot");
		//d->station = station;
		d->arm = arm;
		
	};



	SunwayRobotRQLoadCommand::RunResult SunwayRobotRQLoadCommand::onRun() throw(KernelException){
		FortrendSunwayRobotSubsystem* robot = dynamic_cast<FortrendSunwayRobotSubsystem*>(getSubsystem());
		//
		if (!robot){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "子系统类型错误", this);
		}
		//check subsystem state
		if (robot->getState() != IKernelSubSystem::State::SUB_NORMAL){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_STATE_EXCEPTION, Poco::format("%s 不在正常状态.", robot->getName()), this);
		}
		if (d->arm != 0 && d->arm != 1)
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_SAFE_ALARM, Poco::format("%s 获取手指有无晶圆手臂只能是0或者1.", robot->getName()), this);
		}
		if (robot->getBusyState())
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_BUSY, Poco::format("%s 处于忙碌中.", robot->getName()), this);
		}
		//check modules
		auto cassManager = robot->getKernel()->getKernelModule<FortrendCassetteManager>();
		//get cass
		auto robot_cass = cassManager->getCassette(robot);
		//get command configure
		std::shared_ptr<KernelConfiguration> command_config = robot->getConfigure()->createView(getName());
		//fill params
		std::string str_arm = (d->arm == 0) ? "A" : "B";
		int timeout = command_config->getInt("timeout", 5000);
		if (timeout < 10){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_DATA_OUTOF_RANGE, Poco::format("超时: %s 获取手指状态超时参数设置错误.", robot->getName()), this);
		}
		std::string command = "";
		int error_type = 1;
		int error_code = 0;
		std::string error_message;
		command = "QRY:LOAD/";
		command.append(str_arm);
		command.append(";");

		logInform(robot->getName().c_str(), Poco::format("查询手指%s有无晶圆命令开始执行.", str_arm).c_str());

		clearRobotMessage();
		sendRequest(command);

		std::string res = recvResponseRobotMessage(timeout);
		if (res != std::string("ACK;") && res.find("QRY:LOAD") == std::string::npos)
		{
			logError(robot->getName().c_str(), Poco::format("执行查询手指%s有无晶圆命令存在一个错误.", str_arm).c_str());

			std::string error_str = "ERR";
			if (!handleErrorCode(res, error_str, error_type, error_code)) {
				error_type = 5;
				error_code = 1;
				error_message = ("执行查询手指命令执行失败，机械手返回的指令未定义：%s.", res);
				logError(robot->getName().c_str(), "执行查询手指命令执行失败，机械手返回的指令未定义：%s", res);
			}
			else
			{
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
			// RPS:LOAD/ON;  或者 RPS:LOAD/OFF;
			//等待机械手返回指令
			auto startTime2 = std::chrono::high_resolution_clock::now();
			auto timeout3 = std::chrono::seconds(30);

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
					error_message = "机械手返回指令超时";
					error_code = 0x100;
					AlarmMessage::Ptr alarm(new AlarmMessage(1, error_code, error_message));
					setAlarm(alarm);
					return RunResult::RUN_FAILD;
				}
				res = recvResponseRobotMessage(timeout);
				Sleep(200);
			}

			std::string robot_staus = "";
			if (res == "RPS:LOAD/ON;")
			{
				robot_staus = "ON";
			}
			else if (res == "RPS:LOAD/OFF;")
			{
				robot_staus = "OFF";
			}
			else
			{
				error_code = 101;
				error_message = ("执行查询手指命令执行失败，机械手返回的指令未定义：%s.", res);
				logError(robot->getName().c_str(), "执行查询手指命令执行失败，机械手返回的指令未定义：%s", res);
				AlarmMessage::Ptr alarm(new AlarmMessage(error_type, error_code, error_message));
				setAlarm(alarm);
				return RunResult::RUN_FAILD;
			}

			if (str_arm == "A")
			{
				if (robot_staus == "ON")
				{
					robot->setObject(0, true);
					logInform(robot->getName().c_str(), Poco::format("查询手指%s有晶圆.", str_arm).c_str());
					robot_cass->setMapping(1, Cassette::Mapping::Present);
				}
				else if (robot_staus == "OFF")
				{
					robot->setObject(0, false);
					logInform(robot->getName().c_str(), Poco::format("查询手指%s无晶圆.", str_arm).c_str());
					robot_cass->setMapping(1, Cassette::Mapping::Empty);
				}
				else
				{
					robot->setObject(0, false);
					robot_cass->setMapping(1, Cassette::Mapping::Unknown);
					throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_COMMUNICATION_ERROR,
						Poco::format("工位: %s 获取手臂A状态通讯错误,返回到数据：%s", robot->getName(), res), this);
				}
			}
			else if (str_arm == "B")
			{
				if (robot_staus == "ON")
				{
					robot->setObject(1, true);
					logInform(robot->getName().c_str(), Poco::format("查询手指%s有晶圆.", str_arm).c_str());
					robot_cass->setMapping(2, Cassette::Mapping::Present);
				}
				else if (robot_staus == "OFF")
				{
					robot->setObject(1, false);
					logInform(robot->getName().c_str(), Poco::format("查询手指%s无晶圆.", str_arm).c_str());
					robot_cass->setMapping(2, Cassette::Mapping::Empty);
				}
				else
				{
					robot->setObject(1, false);
					robot_cass->setMapping(2, Cassette::Mapping::Unknown);
					throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_COMMUNICATION_ERROR,
						Poco::format("工位: %s 获取手臂B状态通讯错误，返回到数据：%s",
							robot->getName(), res), this);
				}
			}

			Sleep(200);

			logInform(robot->getName().c_str(), Poco::format("查询手指%s有无晶圆命令执行结束.", str_arm).c_str());
		}
		return RunResult::RUN_OK;

	}

}
