/**
* @file            fortrend_loadlock_open_cassette_door_command.h
* @brief           open cassette door command for loadlock
* @author		   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/LoadLock

#include "Kernel/kernel.h"
#include "Kernel/kernel_command.h"
#include "Kernel/kernel_abstract_subsystem.h"
#include "kernel/kernel_event_id.h"
#include "kernel/kernel_event_paramters.h"
#include "kernel/kernel_command_reject_exception.h"
#include "Kernel/kernel_log.h"
#include "kernel/Fortrend/fortrend_cassette_manager.h"
#include "Poco/Format.h"

#include "LoadLock/fortrend_loadlock_open_cassette_door_command.h"
#include "LoadLock/fortrend_loadlock_subsystem.h"

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

namespace FC{

	

	/**
	* LoadLockOpenCassetteDoorCommand
	*/
	LoadLockOpenCassetteDoorCommand::LoadLockOpenCassetteDoorCommand(KeyencePlcSubSystemHelper* hexHelper)
		:KeyencePlcCommandExecuter(hexHelper){
		//setMessageName("OpenCassetteDoor");
		//setDescription("open cassette door the laodlock");
	};
	
	/**
	* return true if success else false.
	*/
	LoadLockOpenCassetteDoorCommand::RunResult LoadLockOpenCassetteDoorCommand::onRun() throw(KernelException){
		FortrendLoadLockSubsystem* sub = dynamic_cast<FortrendLoadLockSubsystem*>(getSubsystem());
		//
		if (!sub){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "子系统类型错误", this);
		}
		if (sub->getTMCavityDoorOpend())
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_DOOR_EXCEPTION, Poco::format("工位: %s 传输腔门阀已打开（逻辑错误）", sub->getName()), this);
		}
		if (sub->getSlowDiaphragmValveOpend())
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_STATION_CONFLICT_EXCEPTION, Poco::format("工位: %s 慢充隔膜阀已打开（逻辑错误）", sub->getName()), this);
		}
		if (sub->getFastDiaphragmValveOpend())
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_STATION_CONFLICT_EXCEPTION, Poco::format("工位: %s 快充隔膜阀已打开（逻辑错误）", sub->getName()), this);
		}
		/*if (sub->getAngleValveOpend())
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_DOOR_EXCEPTION, Poco::format("工位: %s 角阀已打开（逻辑错误）", sub->getName()), this);
		}*/
		/*if (sub->getExhaustValveOpend())
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_STATION_CONFLICT_EXCEPTION, Poco::format("工位: %s 排气阀已打开（逻辑错误）", sub->getName()), this);
		}*/
		if (sub->getVacuumValue()<=98800)
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_STATE_EXCEPTION, Poco::format("工位: %s 当前的真空值%d未达到设定值（逻辑错误）", sub->getName(), sub->getVacuumValue()), this);
		}
		/*if (sub->getVacuumPressureGageState() == 1 && sub->getVacuumEnable())
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_STATE_EXCEPTION, Poco::format("工位: %s 真空压力表有信号（逻辑错误）", sub->getName()), this);
		}*/
		//get command configure
		std::shared_ptr<KernelConfiguration> command_config = sub->getConfigure()->createView(getName());

		//fill params
		std::string open_address = command_config->getString("open_address", "");
		std::string close_address = command_config->getString("close_address", "");
		std::string finish_address = command_config->getString("finish_address", "");
		int timeout = command_config->getInt("timeout", -1);
		if (timeout < 10){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_DATA_OUTOF_RANGE, Poco::format("超时: %s 打开晶圆盒门阀超时参数设置错误", sub->getName()), this);
		}
		if ((open_address == "") || (close_address == "") || (finish_address == ""))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, Poco::format("地址: 打开晶圆盒门阀地址未定义", getName()), this);
		}
		logInform(sub->getName().c_str(), "打开晶圆盒门阀命令开始");
		if (!writeBit(close_address, false))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 写 0 到关闭晶圆盒门阀地址错误", sub->getName()), this);
		}
		Sleep(20);
		if (!writeBit(open_address, true))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 写 1 到打开晶圆盒门阀地址错误", sub->getName()), this);
		}
		Sleep(500);
		int loopCount = timeout / 20;
		int count = 0;
		bool readRes = false;
		bool readState = false;
		while (count <= loopCount)
		{
			Sleep(20);
			readState = readBit(finish_address,readRes);
			if (readRes)
			{
				break;
			}
			count++;
		}
		IKernelCommand::RunResult ret = IKernelCommand::RunResult::RUN_FAILD;
		if (readRes)
		{
			ret = IKernelCommand::RunResult::RUN_OK;
			sub->setCassetteDoorOpend(true);
			logInform(sub->getName().c_str(), "打开放晶圆盒门阀命令执行结束");

		}
		else if (readState)
		{
			short failed_code = 0;
			AlarmMessage::Ptr alarm(new AlarmMessage(1, 1, "打开放晶圆盒门阀执行失败，打开晶圆盒门阀到位信号异常"));
			setAlarm(alarm);
			logError(sub->getName().c_str(), "打开放晶圆盒门阀执行失败，打开晶圆盒门阀到位信号异常");
		}
		else
		{
			AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_COMMUNICATION_TIMEOUT, "打开晶圆盒门阀命令通讯超时"));
			setAlarm(alarm);
			logError(sub->getName().c_str(), "打开晶圆盒门阀命令通讯超时");
		}
		return ret;

	}



}