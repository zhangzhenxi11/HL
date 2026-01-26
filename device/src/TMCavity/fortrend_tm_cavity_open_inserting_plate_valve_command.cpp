/**
* @file            fortrend_tm_cavity_open_inserting_plate_valve_command.h
* @brief           open inserting plate valve command for tm_cavity
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

#include "TMCavity/fortrend_tm_cavity_open_inserting_plate_valve_command.h"
#include "TMCavity/fortrend_tm_cavity_subsystem.h"

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif


namespace FC{

	

	/**
	* TMCavityOpenInsertingPlateValveCommand
	*/
	TMCavityOpenInsertingPlateValveCommand::TMCavityOpenInsertingPlateValveCommand(KeyencePlcSubSystemHelper* hexHelper)
		:KeyencePlcCommandExecuter(hexHelper){
		//setMessageName("OpenHeightVacuumBaffleValve");
		//setDescription("open ultrahigh vacuum baffle valve the tm cavity");
	};
	
	/**
	* return true if success else false.
	*/
	TMCavityOpenInsertingPlateValveCommand::RunResult TMCavityOpenInsertingPlateValveCommand::onRun() throw(KernelException){
		FortrendTMCavitySubsystem* sub = dynamic_cast<FortrendTMCavitySubsystem*>(getSubsystem());
		//
		if (!sub){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "子系统类型错误", this);
		}
		/*for (auto& pm : getSubsystem()->getKernel()->getKernelModules<FortrendPMCavitySubsystem>()){
			if (pm->hasDoorOpend())
			{
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, Poco::format("子系统: %s 传输腔门阀已打开（逻辑错误）", pm->getName()), this);
			}
		}*/
		for (auto& ll : getSubsystem()->getKernel()->getKernelModules<FortrendLoadLockSubsystem>()){
			if (ll->getTMCavityDoorOpend())
			{
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, Poco::format("子系统: %s 传输腔门阀已打开（逻辑错误）", ll->getName()), this);
			}
		}
		if (sub->getVacuumEnable())
		{
			if (sub->TMCavityCoverSafetyLock() == false)
			{
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, Poco::format("子系统: %s 未检测到传输腔盖门锁信号", sub->getName()), this);
			}
		}
		if (sub->getSlowDiaphragmValveOpend())
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, Poco::format("子系统: %s 隔膜阀未关闭（逻辑错误）", sub->getName()), this);
		}
		if (sub->getFastDiaphragmValveOpend())
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, Poco::format("子系统: %s 隔膜阀未关闭（逻辑错误）", sub->getName()), this);
		}
		if (sub->getHeightVacuumBaffleValveOpend())
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, Poco::format("子系统: %s 高真空挡板阀未关闭（逻辑错误）", sub->getName()), this);
		}
		if (!sub->getTMCavityRoughVacuumReachesTheSetValue(10))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, Poco::format("子系统: %s 传输腔当前真空压力未达到粗抽压力设定值无法打开插板阀（逻辑错误）", getName()), this);
		}
		//get command configure
		std::shared_ptr<KernelConfiguration> command_config = sub->getConfigure()->createView(getName());
		//fill params
		std::string open_address = command_config->getString("open_address", "");
		std::string close_address = command_config->getString("close_address", "");
		std::string finish_address = command_config->getString("finish_address", "");
		int timeout = command_config->getInt("timeout", -1);
		if (timeout < 10){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_DATA_OUTOF_RANGE, Poco::format("超时: %s 打开插板阀超时设置错误", sub->getName()), this);
		}

		if ((open_address == "") || (close_address == "") || (finish_address == ""))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, Poco::format("地址: 打开插板阀地址未定义", getName()), this);
		}
		logInform(sub->getName().c_str(), "打开插板阀命令开始.");
		if (!writeBit(close_address, false))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s写0到关闭插板阀地址错误", sub->getName()), this);
		}
		Sleep(20);
		if (!writeBit(open_address, true))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s写1到打开插板阀地址错误", sub->getName()), this);
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
			sub->setInsertingPlateValveOpend(true);
			ret = IKernelCommand::RunResult::RUN_OK;
			logInform(sub->getName().c_str(), "打开插板阀命令执行结束.");
		}
		else if (readState)
		{

			AlarmMessage::Ptr alarm(new AlarmMessage(1, 2, "打开插板阀命令执行失败，打开插板阀到位信号异常"));
			setAlarm(alarm);
			logError(sub->getName().c_str(), "打开插板阀命令执行失败，打开插板阀到位信号异常.");
		}
		else
		{
			AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_COMMUNICATION_TIMEOUT, "打开插板阀命令通讯超时"));
			setAlarm(alarm);
			logError(sub->getName().c_str(), "打开插板阀命令通讯超时.");
		}
		return ret;
	}



}