/**
* @file            fortrend_tm_cavity_open_ultrahigh_vacuum baffle_valve_command.h
* @brief           open ultrahigh vacuum baffle valve command for tm cavity
* @author		   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/TMCavity

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

#include "TMCavity/fortrend_tm_cavity_open_height_vacuum_baffle_valve_command.h"
#include "TMCavity/fortrend_tm_cavity_subsystem.h"

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif


namespace FC{



	/**
	* TMCavityOpenHeightVacuumBaffleValveCommand
	*/
	TMCavityOpenHeightVacuumBaffleValveCommand::TMCavityOpenHeightVacuumBaffleValveCommand(KeyencePlcSubSystemHelper* hexHelper)
		:KeyencePlcCommandExecuter(hexHelper){
		//setMessageName("OpenHeightVacuumBaffleValve");
		//setDescription("open height vacuum baffle valve to tm cavity");
	};

	/**
	* return true if success else false.
	*/
	TMCavityOpenHeightVacuumBaffleValveCommand::RunResult TMCavityOpenHeightVacuumBaffleValveCommand::onRun() throw(KernelException){
		FortrendTMCavitySubsystem* sub = dynamic_cast<FortrendTMCavitySubsystem*>(getSubsystem());
		//
		if (!sub){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "子系统类型错误", this);
		}

		/*for (auto& pm : getSubsystem()->getKernel()->getKernelModules<FortrendPMCavitySubsystem>()){
			if (pm->hasDoorOpend())
			{
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, Poco::format("子系统: %s 传输腔门阀未关闭（逻辑错误）", pm->getName()), this);
			}
		}*/
		for (auto& ll : getSubsystem()->getKernel()->getKernelModules<FortrendLoadLockSubsystem>()){
			if (ll->getTMCavityDoorOpend())
			{
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, Poco::format("子系统: %s 传输腔门阀未关闭（逻辑错误）", ll->getName()), this);
			}
		}

		if (sub->getVacuumEnable())
		{
			if (sub->TMCavityCoverSafetyLock() == false)
			{
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, Poco::format("子系统: %s 传输腔未检测到传输腔盖门锁信号", sub->getName()), this);
			}
		}
		if (sub->getSlowDiaphragmValveOpend())
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, Poco::format("子系统: %s 传输腔慢充隔膜阀未关闭（逻辑错误）", sub->getName()), this);
		}
		if (sub->getFastDiaphragmValveOpend())
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, Poco::format("子系统: %s 传输腔快充隔膜阀未关闭（逻辑错误）", sub->getName()), this);
		}
		if (sub->getInsertingPlateValveOpend())
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, Poco::format("子系统: %s 传输腔插板阀未关闭（逻辑错误）", sub->getName()), this);
		}

		if (sub->getAngleValveOpend())
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, Poco::format("子系统: %s 角阀未关闭（逻辑错误）", sub->getName()), this);
		}

		if (sub->getTMCavityRoughVacuumReachesTheSetValue(6))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, Poco::format("%s : 传输腔当前真空压力大于粗抽压力设定值无法打开高真空挡板阀（逻辑错误）", getName()), this);
		}

		//get command configure
		std::shared_ptr<KernelConfiguration> command_config = sub->getConfigure()->createView(getName());

		std::string address = command_config->getString("address", "");
		std::string finish_address = command_config->getString("finish_address", "");
		int timeout = command_config->getInt("timeout", -1);
		if (timeout < 10){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_DATA_OUTOF_RANGE, Poco::format("超时: %s 打开高真空挡板阀超时设置错误", sub->getName()), this);
		}
		if (address == "" || finish_address=="")
		{
		   throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, Poco::format("地址: 打开高真空挡板阀地址未定义", getName()), this);
		}
		
		
		logInform(sub->getName().c_str(), "打开高真空挡板阀命令开始");
		if (!writeBit(address, true))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s写1到打开高真空挡板阀地址错误", sub->getName()), this);
		}
		Sleep(500);
		int loopCount = timeout / 20;
		int count = 0;
		bool readRes = false;
		bool readState = false;
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
		IKernelCommand::RunResult ret = IKernelCommand::RunResult::RUN_FAILD;
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
		else
		{
			AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_COMMUNICATION_TIMEOUT, "打开高真空挡板阀命令通讯超时"));
			setAlarm(alarm);
			logError(sub->getName().c_str(), "打开高真空挡板阀命令通讯超时");
		}
		return ret;

		//fill params
		/*std::string address = command_config->getString("address", "");

		if (address == "")
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, Poco::format("地址: 打开高真空挡板阀地址未定义", getName()), this);
		}
		if (sub->getTMCavityRoughVacuumReachesTheSetValue())
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, Poco::format("%s : 传输腔当前真空压力大于粗抽压力设定值无法打开高真空挡板阀（逻辑错误）", getName()), this);
		}
		logInform(sub->getName().c_str(), "打开高真空挡板阀命令开始");
		IKernelCommand::RunResult ret = IKernelCommand::RunResult::RUN_FAILD;
		if (writeBit(address, true))
		{
			ret = IKernelCommand::RunResult::RUN_OK;
			sub->setHeightVacuumBaffleValveOpend(true);
			logInform(sub->getName().c_str(), "打开高真空挡板阀命令执行结束");

		}

		else
		{
			AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_COMMUNICATION_ERROR, "打开高真空挡板阀命令通讯错误"));
			setAlarm(alarm);
			logError(sub->getName().c_str(), "打开高真空挡板阀命令通讯错误");
		}
		return ret;*/

	}



}