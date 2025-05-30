/**
* @file            fortrend_tm_cavity_open_ultrahigh_vacuum baffle_valve_command.h
* @brief           open ultrahigh vacuum baffle valve command for tm cavity
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

#include "LoadLock/fortrend_loadlock_subsystem.h"
#include "PMCavity/fortrend_pm_cavity_subsystem.h"

#include "LoadLock/fortrend_loadlock_cavity_open_height_vacuum_baffle_valve_command.h"
#include "LoadLock/fortrend_loadlock_subsystem.h"

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif


namespace FC{



	/**
	* LoadLockOpenHeightVacuumBaffleValveCommand
	*/
	LoadLockOpenHeightVacuumBaffleValveCommand::LoadLockOpenHeightVacuumBaffleValveCommand(KeyencePlcSubSystemHelper* hexHelper)
		:KeyencePlcCommandExecuter(hexHelper){
		//setMessageName("OpenHeightVacuumBaffleValve");
		//setDescription("open height vacuum baffle valve to tm cavity");
	};
	std::vector<IKernelResources* > LoadLockOpenHeightVacuumBaffleValveCommand::resources() const{
		return std::vector<IKernelResources* >();
	}
	/**
	* return true if success else false.
	*/
	LoadLockOpenHeightVacuumBaffleValveCommand::RunResult LoadLockOpenHeightVacuumBaffleValveCommand::onRun() throw(KernelException){
		FortrendLoadLockSubsystem* sub = dynamic_cast<FortrendLoadLockSubsystem*>(getSubsystem());
		//
		if (!sub){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "子系统类型错误", this);
		}
		if (sub->getTMCavityDoorOpend())
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, Poco::format("子系统: %s 传输腔门阀未关闭（逻辑错误）", sub->getName()), this);
		}

		if (sub->getCassetteDoorOpend())
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, Poco::format("子系统: %s 晶圆盒门未关闭（逻辑错误）", sub->getName()), this);
		}

		if (sub->getSlowDiaphragmValveOpend())//
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, Poco::format("子系统: %s 慢充隔膜阀未关闭（逻辑错误）", sub->getName()), this);
		}
		if (sub->getFastDiaphragmValveOpend())//
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, Poco::format("子系统: %s 快充隔膜阀未关闭（逻辑错误）", sub->getName()), this);
		}
		if (sub->getInsertingPlateValveOpend())//
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, Poco::format("子系统: %s 插板阀未关闭（逻辑错误）", sub->getName()), this);
		}

		if (sub->getAngleValveOpend())
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, Poco::format("子系统: %s 角阀未关闭（逻辑错误）", sub->getName()), this);
		}

		std::shared_ptr<KernelConfiguration> command_config = sub->getConfigure()->createView(getName());
		//get command configure
		std::string open_address = command_config->getString("open_address", "");
		std::string open_address_slowdraw = command_config->getString("open_address2", "");
		std::string close_address = command_config->getString("close_address", "");
		std::string finish_address = command_config->getString("finish_address", "");
		std::string finish_address_slowdraw = command_config->getString("finish_address2", "");

		int timeout = command_config->getInt("timeout", -1);
		if (timeout < 10){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_DATA_OUTOF_RANGE, Poco::format("超时: %s 打开高真空挡板阀超时设置错误", sub->getName()), this);
		}
		if (open_address == "" || close_address == "" || finish_address == "" || finish_address_slowdraw == "" || open_address_slowdraw=="")
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, Poco::format("地址: 打开高真空挡板阀地址未定义", getName()), this);
		}
		/*if (sub->getLoadLockRoughVacuumReachesTheSetValue())
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, Poco::format("%s : 当前真空压力大于粗抽压力设定值无法打开高真空挡板阀（逻辑错误）", getName()), this);
		}*/
		logInform(sub->getName().c_str(), "打开高真空慢抽挡板阀命令开始");
		if (!writeBit(close_address, false))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s写0到关闭高真空挡板阀地址错误", sub->getName()), this);
		}
		Sleep(20);
		if (!writeBit(open_address_slowdraw, true))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s写1到打开高真空慢抽挡板阀地址错误", sub->getName()), this);
		}

		Sleep(500);
		int loopCount = timeout / 2000;
		int count = 0;
		bool readRes = false;
		bool readState = false;
		while (count <= loopCount)
		{
			Sleep(20);
			readState = readBit(finish_address_slowdraw, readRes);
			if (readRes)
			{
				break;
			}
			count++;
		}

		IKernelCommand::RunResult ret = IKernelCommand::RunResult::RUN_FAILD;
		if (readRes)
		{
			sub->setHeightVacuumBaffleValveOpend(true);
			//ret = IKernelCommand::RunResult::RUN_OK;
			//logInform(sub->getName().c_str(), "打开高真空挡板阀命令执行结束");
		}
		else if (readState)
		{
			AlarmMessage::Ptr alarm(new AlarmMessage(1, 2, "打开高真空慢抽挡板阀命令执行失败，打开高真空慢抽挡板阀到位信号异常"));
			setAlarm(alarm);
			logError(sub->getName().c_str(), "打开高真空慢抽挡板阀命令执行失败，打开高真空慢抽挡板阀到位信号异常");
			return ret;
		}
		else
		{
			AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_COMMUNICATION_TIMEOUT, "打开高真空慢抽挡板阀命令通讯超时"));
			setAlarm(alarm);
			logError(sub->getName().c_str(), "打开高真空慢抽挡板阀命令通讯超时");
			return ret;
		}

		loopCount = timeout / 20;
		count = 0;
		readRes = false;
		readState = false;
		while (count <= loopCount)
		{
			Sleep(20);
			readRes = sub->getVacuumValue() <= 50000;
			if (readRes)
			{
				break;
			}
			count++;
		}

		if (readRes){
			logInform(sub->getName().c_str(), "打开高真空快抽挡板阀命令开始");

			Sleep(20);
			if (!writeBit(open_address, true))
			{
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s写1到打开高真空快抽挡板阀地址错误", sub->getName()), this);
			}

			Sleep(500);
			loopCount = timeout / 2000;
			count = 0;
			readRes = false;
			readState = false;
			while (count <= loopCount)
			{
				Sleep(20);
				readState = readBit(finish_address, readRes);
				if (readRes)
				{
					break;
				}
				count++;
			}

			if (readRes)
			{
				sub->setHeightVacuumBaffleValveOpend(true);
				ret = IKernelCommand::RunResult::RUN_OK;
				logInform(sub->getName().c_str(), "打开高真空挡板阀命令执行结束");
			}
			else if (readState)
			{
				AlarmMessage::Ptr alarm(new AlarmMessage(1, 2, "打开高真空挡板阀命令执行失败，打开高真空挡板阀到位信号异常"));
				setAlarm(alarm);
				logError(sub->getName().c_str(), "打开高真空挡板阀命令执行失败，打开高真空挡板阀到位信号异常");
			}
			
			
		}
		else{
			AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_COMMUNICATION_TIMEOUT, "打开高真空挡板阀命令通讯超时"));
			setAlarm(alarm);
			logError(sub->getName().c_str(), "打开高真空挡板阀命令通讯超时");
		}

		return ret;
		

	}



}