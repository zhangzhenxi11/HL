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
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_DATA_OUTOF_RANGE, Poco::format("超时: %s 获取手指状态超时参数设置错误", robot->getName()), this);
		}
		
		logInform(robot->getName().c_str(), Poco::format("获取手指%s有无晶圆命令开始执行", str_arm).c_str());
		if (str_arm == "A")
		{
			//ARM A
			std::string command = "QRY:LOAD/A;";
			sendRequest(command);

			std::string res = recvResponseRobotMessage(timeout);

			if (res == "RPS:LOAD/ON;")//有片
			{
				robot->setObject(0, true);
				robot_cass->setMapping(1, Cassette::Mapping::Present);
			}
			else if (res == "RPS:LOAD/OFF;")//无片
			{
				robot->setObject(0, false);
				robot_cass->setMapping(1, Cassette::Mapping::Empty);
			}
			else if (res == "LOAD A ?")
			{
				robot->setObject(0, false);
				robot_cass->setMapping(1, Cassette::Mapping::Unknown);
			}
			else
			{
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_COMMUNICATION_ERROR, Poco::format("工位: %s 获取手臂A状态通讯错误,返回到数据：%s", robot->getName(), res), this);
			}
			
		}
		else{
			
			//ARM B
			std::string command = "QRY:LOAD/B;";
			sendRequest(command);

			std::string res = recvResponse(timeout);

			if (res == "RPS:LOAD/ON;")
			{
				robot->setObject(1, true);
				robot_cass->setMapping(2, Cassette::Mapping::Present);
			}
			else if (res == "RPS:LOAD/OFF;")
			{
				robot->setObject(1, false);
				robot_cass->setMapping(2, Cassette::Mapping::Empty);
			}
			else if (res == "LOAD B ?")
			{
				robot->setObject(1, false);
				robot_cass->setMapping(2, Cassette::Mapping::Unknown);
			}
			else
			{
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_COMMUNICATION_ERROR, Poco::format("工位: %s 获取手臂B状态通讯错误，返回到数据：%s", robot->getName(), res), this);
			}
		}
		return RunResult::RUN_OK;

	}

}
