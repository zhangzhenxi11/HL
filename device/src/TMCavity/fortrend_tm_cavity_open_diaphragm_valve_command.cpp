/**
* @file            fortrend_tm_cavity_open_diaphragm_valve_command.h
* @brief           open diaphragm valve command for tm_cavity
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

#include "TMCavity/fortrend_tm_cavity_open_diaphragm_valve_command.h"
#include "TMCavity/fortrend_tm_cavity_subsystem.h"

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif


namespace FC{

	class TMCavityOpenDiaphragmValveCommandPrivate{
	public:
		TMCavityValveOpening opening = TMCavityValveOpening::TMCavity_Both;
	};

	/**
	* TMCavityOpenDiaphragmValveCommand
	*/
	TMCavityOpenDiaphragmValveCommand::TMCavityOpenDiaphragmValveCommand(const TMCavityValveOpening opening, KeyencePlcSubSystemHelper* hexHelper)
		:KeyencePlcCommandExecuter(hexHelper)
		, d(new TMCavityOpenDiaphragmValveCommandPrivate){
		d->opening = opening;
		//setMessageName("OpenDiaphragmValve");
		//setDescription("open diaphragm valve the tm cavity");
	};
	
	/**
	* return true if success else false.
	*/
	TMCavityOpenDiaphragmValveCommand::RunResult TMCavityOpenDiaphragmValveCommand::onRun() throw(KernelException){
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
			if (d->opening != TMCavityValveOpening::TMCavity_Slow)
			{
				if (ll->getTMCavityDoorOpend())
				{
					throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, Poco::format("子系统: %s 传输腔门阀已打开（逻辑错误）", ll->getName()), this);
				}
			}
		}

		//if (sub->getHeightVacuumBaffleValveOpend())
		//{
		//	throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR,
		// Poco::format("子系统: %s 腔高真空档板阀未关闭（逻辑错误）", sub->getName()), this);
		//}

		if (d->opening != TMCavityValveOpening::TMCavity_Slow)
		{
			if (sub->getAngleValveOpend())
			{
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, 
					Poco::format("子系统: %s 角阀阀未关闭（逻辑错误", sub->getName()), this);
			}
		}

		/*if (sub->getPIDOpend())
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, 
			Poco::format("子系统: %s PID已打开（逻辑错误）", sub->getName()), this);
		}*/

		//get command configure
		std::shared_ptr<KernelConfiguration> command_config = sub->getConfigure()->createView(getName());

		//fill params
		std::string address1 = command_config->getString("Slow_inflation_address", "");
		std::string address2 = command_config->getString("Fast_inflation_address", "");
		if (address1 == "" || address2 == "")
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, 
				Poco::format("地址: 打开隔膜阀命令地址未定义", getName()), this);
		}
		logInform(sub->getName().c_str(), "打开隔膜阀命令开始.");
		IKernelCommand::RunResult ret = IKernelCommand::RunResult::RUN_FAILD;
		bool write_result = false;
		if (d->opening == TMCavityValveOpening::TMCavity_Slow)
		{
			write_result = writeBit(address1, true);
		}
		else if (d->opening == TMCavityValveOpening::TMCavity_Fast)
		{
			write_result = writeBit(address2, true);
		}
		else{
			write_result = (writeBit(address1, true) && writeBit(address2, true));
		}
		if (write_result)
		{
			ret = IKernelCommand::RunResult::RUN_OK;
			//if (d->opening == TMCavityValveOpening::TMCavity_Slow || d->opening == TMCavityValveOpening::TMCavity_Both)
			//{
			//	sub->setSlowDiaphragmValveOpend(true);
			//}
			//if (d->opening == TMCavityValveOpening::TMCavity_Fast || d->opening == TMCavityValveOpening::TMCavity_Both)
			//{
			//	sub->setFastDiaphragmValveOpend(true);
			//}
			logInform(sub->getName().c_str(), "打开隔膜阀命令执行结束.");

		}
		else
		{
			AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_COMMUNICATION_ERROR, "打开隔膜阀命令通讯错误"));
			setAlarm(alarm);
			logError(sub->getName().c_str(), "打开隔膜阀命令通讯错误.");
		}
		return ret;

	}



}