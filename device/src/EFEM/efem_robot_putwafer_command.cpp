// Library: Fortrend
// Package: CommandImp/Hex/Robot
//
// PutObject command for fortrend foup robot
//
// author xielonghua
//

#include "EFEM/efem_robot_putwafer_command.h"
#include "Kernel/Fortrend/fortrend_station.h"
#include "EFEM/efem_wafer_robot_subsystem.h"
#include "EFEM/efem_aligner_subsystem.h"
#include "Kernel/Fortrend/fortrend_cassette_manager.h"
#include "Kernel/Fortrend/robot_command_helper.h"
#include "Kernel/kernel.h"
#include "Kernel/kernel_log.h"
#include "Kernel/kernel_block.h"
#include "Kernel/kernel_block_manager.h"
#include "LoadLock/fortrend_loadlock_subsystem.h" 
#include "Kernel/Fortrend/loadport_abstract_subsystem.h"
#include "fortrend_device_kernel.h"
#include "Kernel/kernel_event_paramters.h"
#include "Poco/Format.h"
#include <iostream>

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif
KERNEL_NS_BEGIN



void handleRobotPutStationName(std::string& stationName_, int& slotnum_)
{
	/*
		TOOLA1点位id：5，(LLA上层的第二层）
		TOOLA2点位id:6,（LLA最下面的第一层)
		TOOLB1点位id：7，(LLB上层的第二层)
		TOOLB2点位id：8,(LLB最下层的第一层)
		*/
	if (stationName_ == "ELP1" || stationName_ == "ELP2")
	{
		stationName_ == "ELP1" ? "LP1" : "LP2";
		slotnum_ = slotnum_;
	}
	else if (stationName_ == "EALIGNER")
	{
		stationName_ = "ALIGNER";
		slotnum_ = 1;
	}
	else {
		if (stationName_ == "LLA" && slotnum_ == 1)
		{
			stationName_ = "TOOLA2";
		}
		else if (stationName_ == "LLA" && slotnum_ == 2)
		{
			stationName_ = "TOOLA1";
		}
		else if (stationName_ == "LLB" && slotnum_ == 1)
		{
			stationName_ = "TOOLB2";
		}
		else
		{
			stationName_ = "TOOLB1";
		}
		slotnum_ = 1;
	}
}

class EFEMRobotReadyPutWaferCommandPrivate{
public:
	std::string name;
	std::string description;
	std::string TOOLA1 = "TOOLA1";
	std::string TOOLA2 = "TOOLA2";
	std::string TOOLB1 = "TOOLB1";
	std::string TOOLB2 = "TOOLB2";
};


/**
* HexEFEMRobotReadyPutWaferCommand
*/
EFEMRobotReadyPutWaferCommand::EFEMRobotReadyPutWaferCommand(const std::shared_ptr<FortrendStation>&  station, int arm,int slot, HexSubSystemHelper* hexHelper)
	:RobotAbstractReadyPutWaferCommand(station, arm, slot)
, HexCommandExecuter(hexHelper)
, d(new EFEMRobotReadyPutWaferCommandPrivate){
	d->name = "ReadyPutWafer";
	d->description = Poco::format("from %s slot %d use arm %d", station->getName(), slot,arm);
	setMessageName(d->name);
	setDescription(d->description);
};



std::string EFEMRobotReadyPutWaferCommand::getName() const {
	return d->name;
};


std::string EFEMRobotReadyPutWaferCommand::getDescription() const {
	return d->description;
};


/**
* return true if success else false.
*/
EFEMRobotReadyPutWaferCommand::RunResult EFEMRobotReadyPutWaferCommand::onRun() throw(KernelException){
	EFEMWaferRobotSubsystem* robot = dynamic_cast<EFEMWaferRobotSubsystem*>(getSubsystem());
	
	if (beforeRun() != RUN_OK){
		return RUN_REJECT;
	}

	//get command configure
	std::shared_ptr<KernelConfiguration> command_config = robot->getConfigure()->createView(getName());

	//fill params
	int macroId = command_config->getInt("macroid", -1);
	int timeout = command_config->getInt("macrotimeout", -1);
	if (timeout > 0){
		robot->timeout = timeout;
	}
	robot->primaryMessageName = this->getName();
	//std::string stationName = getStation()->getName().erase(0, 1);
	int slotn = getSlot();
	int armn = getArm();
	std::string robotName = robot->getName();
	/*
		param1:WTR name
		param2:station name
		param3:槽号
		param4:手指，1：上手指  2：下手指
		param5: DOWN到取料位，UP到放料位
		MOV:GOTO/WTR/TOOLA1/1/2/DOWN;  ---> robot2移动到TOOLA1待取片位置
		MOV:GOTO/WTR/TOOLA1/1/2/UP;   --->  robot2移动到TOOLA1待放片位置
	*/
	std::string stationName = getStation()->getName();
	handleRobotPutStationName(stationName, slotn);

	std::string str = Poco::format("MOV:GOTO/%s/%s/%d/%d/%s", robotName, stationName, slotn, armn, std::string("UP"));//DOWN到取料位，UP到放料位
	str.push_back(';');
	bool result = robot->api->sendMessage(str.data(), str.size());
	RunResult ret = RunResult::RUN_OK;
	if (!result){
		AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_STATE_EXCEPTION, Poco::format("%s GOTO command failed to send, please check the communication!", robot->getName())));
		setAlarm(alarm);
		ret = RunResult::RUN_FAILD;
		logError(robot->getName().c_str(), "%s GOTO command failed to send, please check the communication!", robot->getName());
		return ret;
	}
	robot->setCommandState(EFEMAsciiApi::State::TRANS_WAIT_REPLY);
	robot->timestamp = std::chrono::system_clock::now();
	robot->wait();
	if (robot->getCommandState() == EFEMAsciiApi::State::TRANS_RESPONSE_TIMEOUT || robot->getCommandState() == EFEMAsciiApi::State::TRANS_REQUEST_FAILD){
		AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_STATE_EXCEPTION, Poco::format("%s GOTO command failed or timed out.", robot->getName())));
		setAlarm(alarm);
		logError(robot->getName().c_str(), "%s GOTO command failed or timed out.", robot->getName());
		ret = RunResult::RUN_FAILD;
		return ret;
	}

	//执行Load指令
	 str = Poco::format("MOV:UNLOAD/%s/%s/%d/%d/%d", robotName,stationName, slotn, 0, armn);//参数三，0不执行寻边，1先执行寻边，目标工站是Aligner时生效
	str.push_back(';');
	result = robot->api->sendMessage(str.data(), str.size());
	if (!result){
		AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_STATE_EXCEPTION, Poco::format("%s UNLOAD command failed to send, please check the communication!", robot->getName())));
		setAlarm(alarm);
		ret = RunResult::RUN_FAILD;
		logError(robot->getName().c_str(), "%s UNLOAD command failed to send, please check the communication!", robot->getName());
		return ret;
	}
	robot->setCommandState(EFEMAsciiApi::State::TRANS_WAIT_REPLY);
	robot->timestamp = std::chrono::system_clock::now();
	robot->wait();
	if (robot->getCommandState() == EFEMAsciiApi::State::TRANS_RESPONSE_TIMEOUT || robot->getCommandState() == EFEMAsciiApi::State::TRANS_REQUEST_FAILD){
		AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_STATE_EXCEPTION, Poco::format("%s UNLOAD command failed or timed out.", robot->getName())));
		setAlarm(alarm);
		logError(robot->getName().c_str(), "%s UNLOAD command failed or timed out.", robot->getName());
		ret = RunResult::RUN_FAILD;
		return ret;
	}

	return ret;
}


/**
* EFEMRobotPutWaferCommand
*/
EFEMRobotPutWaferCommand::EFEMRobotPutWaferCommand(const std::shared_ptr<FortrendStation>&  station, int arm,int slot, HexSubSystemHelper* hexHelper)
:RobotAbstractPutWaferCommand(station, arm,slot)
, HexCommandExecuter(hexHelper)
, d(new EFEMRobotReadyPutWaferCommandPrivate){
	d->name = "PutWafer";
	d->description = Poco::format("from %s slot %d use arm %d", station->getName(), slot, arm);
	setMessageName(d->name);
	setDescription(d->description);
};



std::string EFEMRobotPutWaferCommand::getName() const {
	return d->name;
};



std::string EFEMRobotPutWaferCommand::getDescription() const {
	return d->description;
};

std::vector<IKernelResources* > EFEMRobotPutWaferCommand::resources() const{
	//return KernelSubsystemCommand::resources();
	std::vector<IKernelResources* > ret = KernelSubsystemCommand::resources();
	ret.push_back(getStation().get());
	return ret;
}
/**
* return true if success else false.
*/
EFEMRobotPutWaferCommand::RunResult EFEMRobotPutWaferCommand::onRun() throw(KernelException){
	EFEMWaferRobotSubsystem* robot = dynamic_cast<EFEMWaferRobotSubsystem*>(getSubsystem());
	KernelCommandParameter parameter(shared_from_this());

	//get command configure
	std::shared_ptr<KernelConfiguration> command_config = robot->getConfigure()->createView(getName());
	//fill params
	//int macroId = command_config->getInt("macroid", -1);
	//int timeout = command_config->getInt("macrotimeout", -1);

	////check modules
	auto cassManager = robot->getKernel()->getKernelModule<FortrendCassetteManager>();
	std::string stationName = getStation()->getName();
	int slotnum = getSlot();
	////get cass
	Cassette::Ptr robot_cass1 = cassManager->getCassette(robot);
	if (!robot_cass1){
		throw KernelCommandRejectException(__FILE__, KernelSysException::KR_STATION_WITHOUT_CASS_EXCEPTION, Poco::format("No Cassette %s", robot->getName()), this);
	}

	if (!getStation()){
		throw KernelCommandRejectException(__FILE__, KernelSysException::KR_STATION_WITHOUT_CASS_EXCEPTION, Poco::format("No Station %s", robot->getName()), this);
	}
	int stationid = getStation()->getStationId(robot->getName());

	////get cass
	Cassette::Ptr station_cass = cassManager->getCassette(getStation().get());
	if (!station_cass){
		throw KernelCommandRejectException(__FILE__, KernelSysException::KR_STATION_WITHOUT_CASS_EXCEPTION, Poco::format("Station %s is not present.", getStation()->getName()), this);
	}

	////slot status
	Cassette::Mapping mapStat = station_cass->getMapping(getSlot());

	//if (timeout > 0){
	//	this->setReplyTimeout(timeout);
	//}
	if (stationName == "ELP1" || stationName == "ELP2"){
		if (!getStation()->hasBoxPresent()){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_STATION_WITHOUT_CASS_EXCEPTION, Poco::format("Station %s box not present now(sensor).", getStation()->getName()), this);
		}

		////box placement 
		if (!getStation()->hasBoxPlacement()){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_STATION_WITHOUT_CASS_EXCEPTION, Poco::format("Station %s box not placement now(sensor).", getStation()->getName()), this);
		}

		////box door
		if (!station_cass->isBoxOpened()){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_CASS_CLOSE_EXCEPTION, Poco::format("Station %s box is closed now.", getStation()->getName()), this);
		}

		stationName = stationName == "ELP1" ? "LP1" : "LP2";
	}
	else if (stationName == "EALIGNER")
	{
		std::shared_ptr<EFEMAlignerSubsystem> aligner = std::dynamic_pointer_cast<EFEMAlignerSubsystem>(getStation());
		if (!aligner) {
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_CASS_CLOSE_EXCEPTION, Poco::format("%s is NULL ", getStation()->getName()), this);
		}
		if (aligner->getState() != IKernelSubSystem::State::SUB_NORMAL) {
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_CASS_CLOSE_EXCEPTION, Poco::format("%s state not SUB_NORMAL ", getStation()->getName()), this);
		}
		stationName = "ALIGNER";
	}
	else
	{
		std::shared_ptr<FortrendLoadLockSubsystem> lk = std::dynamic_pointer_cast<FortrendLoadLockSubsystem>(getStation());
		if (!lk){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_CASS_CLOSE_EXCEPTION, Poco::format("%s is NULL ", getStation()->getName()), this);
		}

		if (lk->getState() != IKernelSubSystem::State::SUB_NORMAL){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_CASS_CLOSE_EXCEPTION, Poco::format("%s state not SUB_NORMAL ", getStation()->getName()), this);
		}

		if (SIM_MODE == 1)
		{
			if (!lk->getCassetteDoorOpend()){
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_CASS_CLOSE_EXCEPTION, Poco::format("Station %s door is closed now.", getStation()->getName()), this);
			}
		}
		//调试注释
	

		//stationName = stationName == "LLA" ? "TOOLA1" : "TOOLA2"; //修改

		if (stationName == "LLA" && slotnum == 1)
		{
			stationName = d->TOOLA2;
		}
		else if (stationName == "LLA" && slotnum == 2)
		{
			stationName = d->TOOLA1;

		}
		else if (stationName == "LLB" && slotnum == 1)
		{
			stationName = d->TOOLB2;
		}
		else
		{
			stationName = d->TOOLB1;
		}
		slotnum = 1;
		std::cout << "stationName:"<< stationName << std::endl;
	}
	
	if (mapStat != Cassette::Mapping::Empty){
		throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_CASS_STATE_EXCEPTION, Poco::format("Station %s slot %d  not empty.", getStation()->getName(), (int)getSlot()), this);
	}

	////arm status
	if (!robot->hasObject(getArm()-1)){
		throw KernelCommandRejectException(__FILE__, KernelSysException::KR_ROBOT_OBJECT_EXCEPTION, Poco::format("Robot arm %d whitout wafer now.", (int)getArm()), this);
	}

	/*AutoStationWaferGetPutHelper helper(getStation(), getSlot(), false);
	if (!helper.isReady()){
		throw KernelCommandRejectException(__FILE__, KernelSysException::KR_STATION_CAN_NOT_GET_PUT, Poco::format("Station %s [slot:%d]not ready now.", getStation()->getName(), getSlot()), this);
	}*/



	//fill params
	int macroId = command_config->getInt("macroid", -1);
	int timeout = command_config->getInt("macrotimeout", -1);
	if (timeout > 0){
		robot->timeout = timeout;
	}
	robot->primaryMessageName = getName();
	std::string robotName = robot->getName();//模组名
	robotName = robotName.erase(0, 1);
	std::string str = Poco::format("MOV:UNLOAD/%s/%s/%d/0/%d", robotName,stationName, slotnum,getArm());
	str.push_back(';');
	logInform(robot->getName().c_str(), "command str:%s", str.c_str());
	//25-11-04  add
	robot->sendEvent(NEW_EVENT_ID_WITHNAME(EVENT_COMMAND_RUNNING), &parameter);

	bool result = robot->api->sendMessage(str.data(), str.size());
	RunResult ret = RunResult::RUN_OK;
	if (!result){
		AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_STATE_EXCEPTION, Poco::format("%s unload command failed to send, please check the communication!", robot->getName())));
		setAlarm(alarm);
		ret = RunResult::RUN_FAILD;
		logError(robot->getName().c_str(), "%s放料命令发送失败，请检查通讯！", robot->getName());
		return ret;
	}


	robot->setCommandState(EFEMAsciiApi::State::TRANS_WAIT_REPLY);
	robot->timestamp = std::chrono::system_clock::now();
	robot->wait();
	if (robot->getCommandState() == EFEMAsciiApi::State::TRANS_RESPONSE_TIMEOUT){
		AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_STATE_EXCEPTION, Poco::format("%s %s command timed out.", robot->getName(), getName())));
		setAlarm(alarm);
		ret = RunResult::RUN_FAILD;
	}
	else if (robot->getCommandState() == EFEMAsciiApi::State::TRANS_REQUEST_FAILD){
		AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_STATE_EXCEPTION, Poco::format("%s %s command failed", robot->getName(), getName())));
		setAlarm(alarm);
		ret = RunResult::RUN_FAILD;
	}
	if (robot->getCommandState() == EFEMAsciiApi::State::TRANS_FINISHED){
		robot_cass1->moveWafer(getArm(), station_cass.get(), getSlot());
		robot->setObject(getArm()-1, false);
	}
	robot->getKernel()->getKernelBlockManager()->releaseBlock(robot);
	return ret;

}

KERNEL_NS_END

