/**
* @file     fortrend_sunwayrobot_set_load_command.h
* @brief    set load command for SunwayRobot
* @author   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/SunwayRobot
#include "Kernel/kernel_log.h"

#include "Kernel/kernel.h"
#include "Kernel/kernel_log.h"
#include "Kernel/kernel_block.h"
#include "Kernel/kernel_command.h"
#include "Kernel/kernel_block_manager.h"
#include "Kernel/kernel_action_subsystem.h"
#include "Kernel/kernel_default_block_manager.h"
#include "kernel/kernel_command_reject_exception.h"
#include "kernel/Fortrend/abstract_io_subsystem.h"
#include "Kernel/Fortrend/fortrend_station.h"
#include "Kernel/Fortrend/fortrend_wafer_robot_subsystem.h"
#include "Kernel/Fortrend/fortrend_cassette_manager.h"

#include "SunwayRobot/fortrend_sunwayrobot_set_load_command.h"
#include "SunwayRobot/fortrend_sunwayrobot_subsystem.h"
#include "SunwayRobot/fortrend_sunwayrobot_update_command.h"

#include "Poco/Format.h"

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif


namespace FC{

	/**
	* SunwayRobotSetLoadCommandPrivate
	*/
	class SunwayRobotSetLoadCommandPrivate{
	
	public:
		//std::shared_ptr<FortrendStation> station = 0;
		int arm = 0;
		int state = 0;
	};

	/**
	* SunwayRobotSetLoadCommand
	*/
	SunwayRobotSetLoadCommand::SunwayRobotSetLoadCommand(int arm,int state, SunwaySubSystemHelper* helper)
		:SunwayCommandExecuter(helper)
		, d(new SunwayRobotSetLoadCommandPrivate){
		d->arm = arm;
		d->state = state;
		
	};



	SunwayRobotSetLoadCommand::RunResult SunwayRobotSetLoadCommand::onRun() throw(KernelException){
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
		std::string str_arm = (d->arm == 0) ? "B" : "A";
		std::string str_state = (d->state == 0)? "0":"1";//0机械手上无晶圆,1机械手上有晶圆
		
		int timeout = command_config->getInt("timeout", 5000);
		if (timeout < 10){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_DATA_OUTOF_RANGE, Poco::format("超时: %s 获取手指状态超时参数设置错误.", robot->getName()), this);
		}
		/*
		SET:LOAD/[P1]/[P1];\r
		[P1]：手指，A或B
		[P2]：STATE,0机械手上无晶圆,1机械手上有晶圆 (int)
		*/
		logInform(robot->getName().c_str(), Poco::format("设置手指%s有无晶圆命令开始执行.", str_arm).c_str());
		std::string error_message;
		int error_code = 0;
		int error_type = 1;
		
		auto startTime = std::chrono::high_resolution_clock::now();
		auto timeout2 = std::chrono::seconds(30);
		clearRobotMessage();

		
		if (str_arm == "A")
		{
			//ARM A
			std::string command = "SET:LOAD/A/";
			command.append(str_state);
			command.append(";");
			logInform(getSubsystem()->getName().c_str(),"command:%s", command);
			sendRequest(command);

			std::string res = recvResponseRobotMessage(timeout);

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
					error_message = "机械手设置A手指记忆命令超时";
					error_code = 0x100;
					AlarmMessage::Ptr alarm(new AlarmMessage(1, error_code, error_message));
					setAlarm(alarm);
					return RunResult::RUN_FAILD;
				}
				res = recvResponseRobotMessage(timeout);
				Sleep(200);
			}

			if (res.find("ACK") != std::string::npos || res == "RPS:LOAD;")
			{
				if(str_state=="1")
				{
					robot->setObject(0, true);
					robot_cass->setMapping(1, Cassette::Mapping::Present);
				}
				else
				{
					robot->setObject(0, false);
					robot_cass->setMapping(1, Cassette::Mapping::Empty);
				}
			}
			else
			{
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_COMMUNICATION_ERROR, Poco::format("工位: %s 设置手臂A状态通讯错误,返回到数据：%s", robot->getName(), res), this);
			}
			
		}
		else{
			
			auto startTime = std::chrono::high_resolution_clock::now();

			//ARM B
			std::string command = "SET:LOAD/B/";
			command.append(str_state);
			command.append(";");
			logInform(getSubsystem()->getName().c_str(), "command:%s", command);

			sendRequest(command);

			std::string res = recvResponseRobotMessage(timeout);

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
					error_message = "机械手设置B手指记忆命令超时";
					error_code = 0x101;
					AlarmMessage::Ptr alarm(new AlarmMessage(1, error_code, error_message));
					setAlarm(alarm);
					return RunResult::RUN_FAILD;
				}
				res = recvResponseRobotMessage(timeout);
				Sleep(200);
			}

			if (res.find("ACK") != std::string::npos || res == "RPS:LOAD;")
			{
				if (str_state == "1")
				{
					robot->setObject(1, true);
					robot_cass->setMapping(2, Cassette::Mapping::Present);
				}
				else
				{
					robot->setObject(1, false);
					robot_cass->setMapping(2, Cassette::Mapping::Empty);
				}
			}
			else
			{
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_COMMUNICATION_ERROR, Poco::format("工位: %s 设置手臂B状态通讯错误，返回到数据：%s", robot->getName(), res), this);
			}
		}
		return RunResult::RUN_OK;

	}

}
