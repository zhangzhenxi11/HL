/**
* @file     fortrend_sunwayrobot_put_wafer_command.h
* @brief    put_wafer command for SunwayRobot
* @author   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/SunwayRobot

#include <sstream>
#include <vector>
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
#include "SunwayRobot/fortrend_sunwayrobot_put_wafer_command.h"
#include "SunwayRobot/fortrend_sunwayrobot_subsystem.h"
#include "SunwayRobot/fortrend_sunwayrobot_check_load_command.h"
#include "CoolingCavity/fortrend_cooling_cavity_subsystem.h"

#include "fortrend_device_kernel.h"
#include "Kernel/kernel_event_paramters.h"

#include <iostream>
#include <string>
#include <chrono>
#include <regex>
#include <algorithm>
#include <functional>
#include <stdexcept>

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif



namespace FC{

/**
* SunwayRobotPutWaferCommandPrivate
*/
class SunwayRobotPutWaferCommandPrivate{
public:
	std::string station_name;
	//FortrendSunwayRobotSubsystem* robot;
};

/**
* SunwayRobotPutWaferCommand
*/
SunwayRobotPutWaferCommand::SunwayRobotPutWaferCommand(const std::shared_ptr<FortrendStation>&  station, 
	int arm, int slot, SunwaySubSystemHelper* helper)
	:SunwayCommandExecuter(helper)
	, RobotAbstractPutWaferCommand(station, arm, slot)
	, d(new SunwayRobotPutWaferCommandPrivate){

};

std::vector<IKernelResources* > SunwayRobotPutWaferCommand::resources() const{
	std::vector<IKernelResources* > ret = KernelSubsystemCommand::resources();
	ret.push_back(getStation().get());
	return ret;
}

SunwayRobotPutWaferCommand::RunResult SunwayRobotPutWaferCommand::performRobotOperation(
	const std::function<std::string()>& commandBuilder,
	const std::function<bool()>& onSuccess)
{

	FortrendSunwayRobotSubsystem* robot = dynamic_cast<FortrendSunwayRobotSubsystem*>(getSubsystem());
	if (!robot) {
		throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE,"子系统类型错误", this);
	}
	//get command configure
	std::shared_ptr<KernelConfiguration> command_config = robot->getConfigure()->createView(getName());

	KernelCommandParameter parameter(shared_from_this());

	//fill params
	int timeout = command_config->getInt("timeout", 100000);
	if (timeout < 10) {
		throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_DATA_OUTOF_RANGE, 
			Poco::format("超时: %s 放晶圆超时参数错误", robot->getName()), this);
	}
	// 创建命令块
	auto block = robot->getKernel()->getKernelBlockManager()->createBlock(
		robot->getName(), robot, { (FortrendStation*)getStation().get() }, 0
	);

	
	logInform(robot->getName().c_str(), "放晶圆命令开始,工位%s", d->station_name);
	robot->sendEvent(NEW_EVENT_ID_WITHNAME(EVENT_COMMAND_RUNNING), &parameter);
	// 等待响应或超时
	std::string res;
	std::string error_message;
	int error_code = 0;
	int error_type = 1;
	try {
		// 发送命令并等待响应
		std::string command = commandBuilder();
		sendRequest(command);

		auto startTime = std::chrono::high_resolution_clock::now();
		auto timeout2 = std::chrono::seconds(30);

		res = recvResponse(timeout);
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
				error_message = "机械手放晶圆超时";
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
			logError(robot->getName().c_str(), "放晶圆命令发生错误");

			std::string error_str = "ERR";
			if (!handleErrorCode(res, error_str, error_type, error_code)) {
				error_type = 5;
				error_code = 1;
				error_message = ("放晶圆命令执行失败，机械手返回的指令未定义：%s.", res);
				logError(robot->getName().c_str(), "放晶圆命令执行失败，机械手返回的指令未定义：%s", res);
			}
			else
			{
				logError(robot->getName().c_str(), "执行放晶圆时存在一个错误");
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
			//RPS:PUT;[CR]
			
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
					error_message = "机械手放晶圆返回指令超时";
					error_code = 0x100;
					AlarmMessage::Ptr alarm(new AlarmMessage(1, error_code, error_message));
					setAlarm(alarm);
					return RunResult::RUN_FAILD;
				}
				res = recvResponse(timeout);
				Sleep(200);
			}

			std::string recvMessage = "RPS:PUT;";
			auto found = search(res.begin(),res.end(), recvMessage.begin(), recvMessage.end());

			if (found != res.end()) 
			{
				//找到了
				if (!sendRequest("ACK;"))
				{
					throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_COMMUNICATION_ERROR,
						Poco::format("%s 机械手通讯错误", robot->getName()), this);
				}
				// 成功回调
				if (!onSuccess()) {
					logError(robot->getName().c_str(),"更新晶圆状态失败");
					error_message = "机械手更新晶圆状态失败";
					error_code = 0x101;
					AlarmMessage::Ptr alarm(new AlarmMessage(1, error_code, error_message));
					setAlarm(alarm);
					return RunResult::RUN_FAILD;
				};

				return RunResult::RUN_OK;
			}
			else
			{
				//没找到
				logError(robot->getName().c_str(), "放晶圆返回指令错误");
				return RunResult::RUN_FAILD;
			}
		}
	}
	catch (...)
	{
		robot->getKernel()->getKernelBlockManager()->releaseBlock(robot);
		logError(robot->getName().c_str(), "放晶圆命令执行失败");
		throw;
	}
}

bool SunwayRobotPutWaferCommand::updateWaferMapping()
{
	FortrendSunwayRobotSubsystem* robot = dynamic_cast<FortrendSunwayRobotSubsystem*>(getSubsystem());
	auto cassManager = getSubsystem()->getKernel()->getKernelModule<FortrendCassetteManager>();
	auto station_cass = cassManager->getCassette(getStation().get());
	auto robot_cass = cassManager->getCassette(robot);
	
	int slot = 1;
	if (getStation()->getName().find("LL") != std::string::npos)
	{
		//loadlock自动更新cassete,可注释
		auto loadlock = std::dynamic_pointer_cast<FortrendLoadLockSubsystem>(getStation());
		station_cass->setMapping(getSlot(), Cassette::Present);
	}
	else {
		station_cass->setMapping(1, Cassette::Present);
	}


	robot->setObject(getArm(), false);

	//AWC 
	if (d->station_name.find("PM") != std::string::npos)
	{
		auto pm = std::dynamic_pointer_cast<FortrendPMCavitySubsystem>(getStation());

		int wafer_slot = robot->getWaferSlot(getArm());

		pm->setWaferSlot(wafer_slot);

		// PM Cavity Record AWC Data	QRY:AWC_PARAM/[P1];	  工位参数，范围：1~30

		std::string command = "QRY:AWC_PARAM/";
		command.append(std::to_string(getStation()->getStationId(robot->getName())));
		logInform(robot->getName().c_str(), "AWC command:%s", command);

		sendRequest(command);
		std::string rps = recvResponse(2000);
# if 0
		std::smatch match;
		std::regex regexPattern("OFFSET\\s*(-?\\d{1,4})\\s*(-?\\d{1,4})\\s*(-?\\d{1,4})\\s*(-?\\d{1,4})");
		try {
			if (std::regex_search(rps, match, regexPattern)) {
				int r = 0, t = 0, x = 0, y = 0;

				if (match[1].matched) r = std::stoi(match[1].str());
				if (match[2].matched) t = std::stoi(match[2].str());
				if (match[3].matched) x = std::stoi(match[3].str());
				if (match[4].matched) y = std::stoi(match[4].str());

				FortrendSunwayRobotSubsystem::AWCRecordData awc_value;
				awc_value.R = r * 1.0 / 1000.0;
				awc_value.T = t * 1.0 / 1000.0;
				awc_value.X = x * 1.0 / 1000.0;
				awc_value.Y = y * 1.0 / 1000.0;

				// 设置AWC记录数据
				robot->setAWCRecordData(1, awc_value);
				logInform1("AWC", "R:%f T:%f X:%f Y:%f", awc_value.R, awc_value.T, awc_value.X, awc_value.Y);
			}
			else {
				logError(robot->getName().c_str(), "解析AWC数据异常：未找到匹配的OFFSET数据");
				return false;
			}
		}
		catch (const std::invalid_argument& e) {
			logError(robot->getName().c_str(), "解析AWC数据异常：数据格式错误");
			return false;
		}
		catch (const std::out_of_range& e) {
			logError(robot->getName().c_str(), "解析AWC数据异常：数据超出范围");
			return false;
		}
		catch (const std::exception& e) {
			logError(robot->getName().c_str(), "解析AWC数据异常：%s", std::string(e.what()));
			return false;
		}

#endif
	}

	//对机械手的虚拟cassette，放晶圆完成后，更新状态
	auto robot_mapping_res = robot_cass->getAllMapping();
	robot_mapping_res[getArm()] = Cassette::Empty;

	std::vector<int> robot_all_alots;
	for (size_t i = 0; i < robot_mapping_res.size(); i++)
	{
		robot_all_alots.push_back(i + 1);
	}
	robot_cass->setMapping(robot_all_alots, robot_mapping_res);

	station_cass->setPodSize(robot_cass->getPodSize());//PM腔的工艺次数，需要本地存储，暂时使用PodSize字段
	//Loadlock的cassete状态会自动更新

	logInform(robot->getName().c_str(), "放晶圆命令执行结束 %s %d %s", d->station_name, slot, station_cass->getBoxId());
	return true;
}


SunwayRobotPutWaferCommand::RunResult SunwayRobotPutWaferCommand::onRun() throw(KernelException){
	FortrendSunwayRobotSubsystem* robot = dynamic_cast<FortrendSunwayRobotSubsystem*>(getSubsystem());
	if (!robot){
		throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "子系统类型错误", this);
	}

	//check subsystem state
	if (robot->getState() != IKernelSubSystem::State::SUB_NORMAL){
		throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_STATE_EXCEPTION, "机械手未处于正常状态", this);
	}
	//check station state
	if (auto sub = std::dynamic_pointer_cast<KernelAbstractSubSystem>(getStation())){
		if (sub->getState() != IKernelSubSystem::SUB_NORMAL){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_STATE_EXCEPTION, 
				Poco::format("工位： %s 未处于正常状态.", getStation()->getName()), this);
		}
	}

	if (robot->getBusyState())
	{
		throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_BUSY,
			Poco::format("%s 处于忙碌中.", robot->getName()), this);
	}
	//check door open
	if (WTR_SIM_MODE == 0)
	{
		if (auto sub = std::dynamic_pointer_cast<FortrendAbstractStation>(getStation())) {
			if (!sub->hasDoorOpend()) {
				if (getStation()->getName() != "PM")
					throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_DOOR_EXCEPTION,
						Poco::format("工位： %s 门阀未处于打开状态（逻辑错误）.", getStation()->getName()), this);
			}
		}
	}

	
	//check modules
	auto cassManager = robot->getKernel()->getKernelModule<FortrendCassetteManager>();
	//get cass
	auto station_cass = cassManager->getCassette(getStation().get());

	if (!station_cass){
		throw KernelCommandRejectException(__FILE__, KernelSysException::KR_STATION_WITHOUT_CASS_EXCEPTION, 
			Poco::format("工位 %s晶圆盒不存在.", getStation()->getName()), this);
	}

	int mapping_slot = getSlot();

	//if (getStation()->getName().find("LL") != std::string::npos)
	//{
	//	if (auto sub = std::dynamic_pointer_cast<FortrendLoadLockSubsystem>(getStation())){
	//		int loadlock_move_slot = sub->getLastMoveSlot();
	//		if (loadlock_move_slot == 0 || loadlock_move_slot>4){
	//			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_DATA_OUTOF_RANGE,
	//				Poco::format("工位 %s 移动的槽号错误.", getStation()->getName()), this);
	//		}
	//		else
	//			mapping_slot = loadlock_move_slot;
	//	}
	//}

	if (getStation()->getName().find("PM") != std::string::npos)
	{
		if (auto sub = std::dynamic_pointer_cast<FortrendPMCavitySubsystem>(getStation())){
			if (!sub)
			{
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "PM子系统类型错误", this);
			}
			if (!sub->getPMCavitySafeSignal())
			{
				logInform(sub->getName().c_str(), "PM腔未检测到安全信号 %d ,延迟50ms重新检测", sub->getPMCavitySafeSignal());
				Sleep(50);
				if (!sub->getPMCavitySafeSignal())
				{
					throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR,
						Poco::format("%s腔未发出安全信号", getStation()->getName()).c_str(), this);
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
				logInform(sub->getName().c_str(), "Loadlock腔未检测到安全信号 %d ,延迟50ms重新检测", sub->getLoadLockCavitySafeSignal());
				Sleep(50);
				if (!sub->getLoadLockCavitySafeSignal())
				{
					throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR,
						Poco::format("%s腔未发出安全信号", getStation()->getName()).c_str(), this);
				}
			}

		}
	}
	if (station_cass->getMapping(mapping_slot) != Cassette::Mapping::Empty)
	{
		throw KernelCommandRejectException(__FILE__, KernelSysException::KR_STATION_WITHOUT_CASS_EXCEPTION, 
			Poco::format("工位： %s 槽 %d 当前不为空.", getStation()->getName(), mapping_slot), this);
	}

	//fill params
	d->station_name = getStation()->getName();
	std::string str_arm = (getArm() == 0) ? "A" : "B";
	// LL 工位处理
	if (getStation()->getName().find("LL") != std::string::npos)
	{
		return performRobotOperation(

		[this, robot,str_arm]()->std::string {
			//指令回调函数
			std::string command = "MOV:PUT/";
			command.append(std::to_string(getStation()->getStationId(robot->getName())));
			command.append("/").append(str_arm).append("/");
			command.append(std::to_string(getSlot())).append(";");
			return command;   //MOV:GET/12/A/1; 
		},

		[this, robot]()->bool {
			//更新状态回调函数
			logInform(robot->getName().c_str(), "获取晶圆命令执行结束 %s %d", getStation()->getName(), getSlot());
			return updateWaferMapping();
			
		}

		);
	}
	// PM 工位处理
	else if (getStation()->getName() == "PM")
	{
		return performRobotOperation(
			//指令回调函数
			[this, robot, str_arm]() -> std::string {
			int stationId = getStation()->getStationId(robot->getName());

			stationId = (getStation()->getName() == "PM") ? 15 : stationId;

			std::string command = "MOV:PUT/";

			command.append(std::to_string(stationId))
				.append("/").append(str_arm).append("/")
				.append(std::to_string(getSlot())).append(";");

			return command;
		},

			[this, robot]()->bool {
			//更新状态回调函数
			logInform(robot->getName().c_str(), "获取晶圆命令执行结束 %s %d", getStation()->getName(), getSlot());
			return updateWaferMapping();
			
		}
		);
	}
	return RunResult::RUN_OK;
}

}
