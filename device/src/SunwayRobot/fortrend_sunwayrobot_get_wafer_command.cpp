/**
* @file     fortrend_sunwayrobot_get_wafer_command.h
* @brief    get_wafer command for sunway robot
* @author   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/SunwayRobot

#include "Poco/Format.h"

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

#include "Aligner/fortrend_aligner_subsystem.h"
#include "LoadLock/fortrend_loadlock_subsystem.h"

#include "PMCavity/fortrend_pm_cavity_subsystem.h"
#include "SunwayRobot/fortrend_sunwayrobot_get_wafer_command.h"
#include "SunwayRobot/fortrend_sunwayrobot_subsystem.h"
#include "SunwayRobot/fortrend_sunwayrobot_check_load_command.h"

#include "fortrend_device_kernel.h"
#include "Kernel/kernel_event_paramters.h"
#include <algorithm>
#include <functional>
#include <chrono>
#include <mutex>

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif



namespace FC{

/**
* SunwayRobotGetWaferCommandPrivate
*/
class SunwayRobotGetWaferCommandPrivate{
public:
	bool busy = true;
	
};

/**
* SunwayRobotGetWaferCommand
*/
SunwayRobotGetWaferCommand::SunwayRobotGetWaferCommand(const std::shared_ptr<FortrendStation>&  station,
	int arm, int slot,SunwaySubSystemHelper* helper)
	:SunwayCommandExecuter(helper)
	, RobotAbstractGetWaferCommand(station, arm, slot)
	, d(new SunwayRobotGetWaferCommandPrivate){


};

std::vector<IKernelResources* > SunwayRobotGetWaferCommand::resources() const{
	std::vector<IKernelResources* > ret = KernelSubsystemCommand::resources();
	ret.push_back(getStation().get());
	return ret;
}

bool SunwayRobotGetWaferCommand::IsBusy(){
	return d->busy;
}

SunwayRobotGetWaferCommand::RunResult SunwayRobotGetWaferCommand::onRun() throw(KernelException){
	FortrendSunwayRobotSubsystem* robot = dynamic_cast<FortrendSunwayRobotSubsystem*>(getSubsystem());
	KernelCommandParameter parameter(shared_from_this());
	//
	if (!robot){
		throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "子系统类型错误", this);
	}

	std::lock_guard<std::mutex> lock (robot->robot_mutex); //加锁

	//check subsystem state
	if (robot->getState() != IKernelSubSystem::State::SUB_NORMAL){
		throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_STATE_EXCEPTION, Poco::format("%s 不在正常状态.", robot->getName()), this);
	}
	//check station state
	if (auto sub = std::dynamic_pointer_cast<KernelAbstractSubSystem>(getStation())){
		if (sub->getState() != IKernelSubSystem::SUB_NORMAL){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_STATE_EXCEPTION, Poco::format("工位： %s 不在正常状态.", getStation()->getName()), this);
		}
	}
	if (robot->getBusyState())
	{
		throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_BUSY, Poco::format("%s 处于忙碌中.", robot->getName()), this);
	}

	//check door open
	if (auto sub = std::dynamic_pointer_cast<FortrendAbstractStation>(getStation())) 
	{
		if (!sub->hasDoorOpend())
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_DOOR_EXCEPTION,
				Poco::format("工位： %s 当前门阀处于关闭状态（逻辑错误）.", getStation()->getName()), this);
		}
	}
	
	if (robot->getWithWaferModeEnable())
	{
		//arm status
		if (robot->hasObject(getArm())) {
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_STATION_WITHOUT_CASS_EXCEPTION,
				Poco::format("机械手手臂%d 当前存在晶圆.", getArm()), this);
		}
	}
	//check modules
	auto cassManager = robot->getKernel()->getKernelModule<FortrendCassetteManager>();
	//get cass
	auto station_cass = cassManager->getCassette(getStation().get());
	if (!station_cass){
		throw KernelCommandRejectException(__FILE__, KernelSysException::KR_STATION_WITHOUT_CASS_EXCEPTION, 
			Poco::format("工位: %s 晶圆盒为空.", getStation()->getName()), this);
	}


	int mapping_slot = getSlot();

	if (getStation()->getName().find("PM") != std::string::npos)
	{
		if (auto sub = std::dynamic_pointer_cast<FortrendPMCavitySubsystem>(getStation())){
			if (!sub)
			{
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "PM子系统类型错误", this);
			}
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
			if (!sub)
			{
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "Loadlock子系统类型错误", this);
			}
			if (!sub->getLoadLockCavitySafeSignal())
			{
				logInform(sub->getName().c_str(), "Loadlock腔未检测到安全信号 %d ,延迟50ms重新检测.", sub->getLoadLockCavitySafeSignal());
				Sleep(50);
				if (!sub->getLoadLockCavitySafeSignal())
				{
					throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR,Poco::format("%s腔未发出安全信号.", getStation()->getName()).c_str(), this);
				}
			}	
		}
	}

	if (robot->getWithWaferModeEnable() && station_cass->getMapping(mapping_slot) != Cassette::Mapping::Present)
	{
		//补丁
		if (getStation()->getName().find("PM") == std::string::npos)
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_STATION_WITHOUT_CASS_EXCEPTION,
				Poco::format("工位: %s 槽 %d 当前不存在晶圆.", getStation()->getName(), mapping_slot), this);
		}

	}

	if (getStation()->getName().find("LL") != std::string::npos)
	{
		if (getSlot() != mapping_slot)
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR,
				Poco::format("取%s工位时升降轴位置与指定的槽号不匹配.", getStation()->getName()), this);
		}
	}

	try{
		//get command configure
		std::shared_ptr<KernelConfiguration> command_config = robot->getConfigure()->createView(getName());
		//fill params
		std::string str_arm = (getArm() == 0) ? "A" : "B";
		int timeout = command_config->getInt("timeout", 300000);
		std::string command = "";
		std::string station_name = getStation()->getName();
		std::string error_message = "";
		int error_type = 1;
		int error_code = 0;
		std::string res;
		if (station_name == "LLA" || station_name == "LLB")
		{
			//MOV:GETOBJECT/3/1/1/0/0;[CR]
			//2026-1-22 MOV:GETOBJECT/[P1]/[P2]/[P3]/[P4]/[P5];
			//[P1] 工位，范围：1~30(int)
			//[P2]	槽位（默认1）。范围：1~25(int)
			//[P3]	手指(0:B, 1 : A)(int)
			//[P4]	默认为0
			//[P5]	默认为0

			command = "MOV:GETOBJECT/";
			command.append(std::to_string(getStation()->getStationId(robot->getName())));
			command.append("/");
			command.append(std::to_string(mapping_slot));
			command.append("/");
			command.append(str_arm);
			command.append("/0/0");
			command.append(";");

			logInform(robot->getName().c_str(), "取晶圆命令:%s,开始.", command.c_str());
			d->busy = false;
			robot->sendEvent(NEW_EVENT_ID_WITHNAME(EVENT_COMMAND_RUNNING), &parameter);
		
			auto startTime = std::chrono::high_resolution_clock::now();
			auto timeout2 = std::chrono::seconds(120); //--->60 改成120

			clearRobotMessage();
			if (!sendRequest(command))
			{
				AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE,
					KernelSysException::KR_MODULE_COMMUNICATION_ERROR,
					Poco::format("%s 机械手通讯错误.", robot->getName())));
				setAlarm(alarm);
				return RunResult::RUN_FAILD;
			};

			
			res = recvResponseRobotMessage(timeout);
			logInform(robot->getName().c_str(), "取晶圆ACK：%s", res.c_str());

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
					error_message = "机械手取晶圆超时";
					error_code = 0x100;
					AlarmMessage::Ptr alarm(new AlarmMessage(1, error_code, error_message));
					setAlarm(alarm);
					return RunResult::RUN_FAILD;
				}
				res = recvResponseRobotMessage(timeout);
				logInform(robot->getName().c_str(), "机械手取晶圆ACK：%s", res.c_str());
				Sleep(10);
			}
			logInform(robot->getName().c_str(), "res：%s", res.c_str());

			//if (res != "ACK;" && res != "RPS:GET;")
			if (res != "ACK;")
			{
				logError(robot->getName().c_str(), "执行取晶圆时存在一个错误.");
				int error_type = 1;
				int error_code = 0;
				std::string error_message;
				std::string error_str = "ERR";
				if (!handleErrorCode(res, error_str, error_type, error_code)) {
					error_type = 5;
					error_code = 1;
					error_message = ("取晶圆命令执行失败，机械手返回的指令未定义：%s.", res);
					logError(robot->getName().c_str(), "取晶圆命令执行失败，机械手返回的指令未定义：%s", res);
				}
				else
				{
					logError(robot->getName().c_str(), "执行取晶圆时存在一个错误.");
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
			{//"ACK;"
				
				res = recvResponseRobotMessage(timeout);
				//等待机械手返回指令
				auto startTime2 = std::chrono::high_resolution_clock::now();
				auto timeout3 = std::chrono::seconds(120);

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
						error_message = "机械手取晶圆返回指令超时";
						error_code = 0x100;
						AlarmMessage::Ptr alarm(new AlarmMessage(1, error_code, error_message));
						setAlarm(alarm);
						return RunResult::RUN_FAILD;
					}
					res = recvResponseRobotMessage(timeout);
					Sleep(20);
				}
				logInform(robot->getName().c_str(), "获取取晶圆RPS：%s", res.c_str());

				std::string recvMessage = "RPS:GETOBJECT;";
				auto found = search(res.begin(), res.end(), recvMessage.begin(), recvMessage.end());
				if (found != res.end())
				{
					//找到
					if (!sendRequest("ACK;"))
					{
						throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_COMMUNICATION_ERROR,
							Poco::format("%s 机械手通讯错误.", robot->getName()), this);
					}
				}
				else
				{
					std::string error_str = "ERR";
					try {
						if (!handleErrorCode(res, error_str, error_type, error_code)) {

							error_type = 10;
							error_code = 1;
							error_message = ("取晶圆命令执行失败，机械手返回的指令未定义：%s.", res);
							logError(robot->getName().c_str(), "取晶圆命令执行失败，机械手返回的指令未定义：%s", res);
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
						logError(getName().c_str(), "处理字符串失败.");
					}

					//set alarm data
					AlarmMessage::Ptr alarm(new AlarmMessage(error_type, error_code, error_message));
					setAlarm(alarm);
					logError(robot->getName().c_str(), "机械手取晶圆失败,机械手返回：【%s】", res);
					return RunResult::RUN_FAILD;
				}


				int slot = 1;
				logInform(robot->getName().c_str(), "取晶圆手指Arm indenx： %d", getArm());
				station_cass->setMapping(mapping_slot, Cassette::Mapping::Empty);

				robot->setObject(getArm(), true);
				auto robot_cass = cassManager->getCassette(robot);
				robot_cass->setMapping(getArm() + 1, Cassette::Mapping::Present);

				robot_cass->setPodSize(station_cass->getPodSize());//PM腔的工艺次数，需要本地存储，暂时使用PodSize字段
				logInform(robot->getName().c_str(), "获取晶圆命令执行结束 %s %d %s", station_name, slot, station_cass->getBoxId());
			}
			
			robot->getKernel()->getKernelBlockManager()->releaseBlock(robot);
			return RunResult::RUN_OK;
		}
		else//PM
		{
			int stationid = getStation()->getStationId(robot->getName()); 
			//if (station_name == "PM")stationid = 15;
			command = "MOV:GETOBJECT/";
			command.append(std::to_string(stationid));
			command.append("/");
			command.append(std::to_string(getSlot()));
			command.append("/");
			command.append(str_arm);
			command.append("/0/0");
			command.append(";");

			//robot->getKernel()->getKernelBlockManager()->createBlock(robot->getName(), robot, { (FortrendStation*)getStation().get() }, 0);
			logInform(robot->getName().c_str(), "取晶圆命令开始.");
			d->busy = false;
			robot->sendEvent(NEW_EVENT_ID_WITHNAME(EVENT_COMMAND_RUNNING), &parameter);

			clearRobotMessage();
			sendRequest(command);
			res = recvResponseRobotMessage(timeout);
			logInform(robot->getName().c_str(), "取晶圆ACK：%s", res.c_str());

			auto startTime = std::chrono::high_resolution_clock::now();
			auto timeout2 = std::chrono::seconds(120);

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
					error_message = "机械手取晶圆超时";
					error_code = 0x100;
					AlarmMessage::Ptr alarm(new AlarmMessage(1, error_code, error_message));
					setAlarm(alarm);
					return RunResult::RUN_FAILD;
				}
				res = recvResponseRobotMessage(timeout);
				Sleep(20);
			}

			logInform(robot->getName().c_str(), "取晶圆ACK：%s", res.c_str());

			if (res != std::string("ACK;"))
			{
				logError(robot->getName().c_str(), "执行取晶圆时存在一个错误.");

				std::string error_str = "ERR";
				try {
					if (!handleErrorCode(res, error_str, error_type, error_code)) {

						error_type = 5;
						error_code = 1;
						error_message = ("取晶圆命令执行失败，机械手返回的指令未定义：%s.", res);
						logError(robot->getName().c_str(), "取晶圆命令执行失败，机械手返回的指令未定义：%s", res);
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
					logError(getName().c_str(), "处理字符串失败.");
				}

				//set alarm data
				AlarmMessage::Ptr alarm(new AlarmMessage(error_type, error_code, error_message));
				setAlarm(alarm);
				return RunResult::RUN_FAILD;
		
			}
			else
			{
				auto startTime2 = std::chrono::high_resolution_clock::now();
				auto timeout3 = std::chrono::seconds(120);

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
						error_message = "机械手取晶圆返回指令超时";
						error_code = 0x100;
						AlarmMessage::Ptr alarm(new AlarmMessage(1, error_code, error_message));
						setAlarm(alarm);
						return RunResult::RUN_FAILD;
					}
					res = recvResponseRobotMessage(timeout);
					Sleep(200);
				}
				logInform(robot->getName().c_str(), "取晶圆RPS：%s", res.c_str());
				//res = recvResponseRobotMessage(timeout);

				std::string recvMessage = "RPS:GETOBJECT;";
				auto found = search(res.begin(), res.end(), recvMessage.begin(), recvMessage.end());
				if (found != res.end())
				{
					//找到
					if (!sendRequest("ACK;"))
					{
						throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_COMMUNICATION_ERROR,
							Poco::format("%s 机械手通讯错误.", robot->getName()), this);
					}
				}
				else
				{
#if 0
					std::string error_str = "ERR";
					try {
						if (!handleErrorCode(res, error_str, error_type, error_code)) {

							error_type = 5;
							error_code = 1;
							error_message = ("取晶圆命令执行失败，机械手返回的指令未定义：%s.", res);
							logError(robot->getName().c_str(), "取晶圆命令执行失败，机械手返回的指令未定义：%s", res);
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
						logError(getName().c_str(), "处理字符串失败.");
					}

					//set alarm data
					AlarmMessage::Ptr alarm(new AlarmMessage(error_type, error_code, error_message));
					setAlarm(alarm);
					logError(robot->getName().c_str(), "机械手取晶圆失败,机械手返回：【%s】", res);
					return RunResult::RUN_FAILD;
#endif
				}

				int slot = 1;
				station_cass->setMapping(slot, Cassette::Mapping::Empty);

				robot->setObject(getArm(), true);
				auto robot_cass = cassManager->getCassette(robot);
				robot_cass->setMapping(getArm()+1, Cassette::Mapping::Present);

				robot_cass->setPodSize(station_cass->getPodSize());

				logInform(robot->getName().c_str(), "获取晶圆命令执行结束 %s %s", station_name, station_cass->getBoxId());
			}
		}
	}
	catch (KernelBlockException&e){
		throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_CAN_NOT_LOCK_RESOURCE, e.getMessage(), this);
	}
	catch (...){
		throw;
	}
	

}

}
