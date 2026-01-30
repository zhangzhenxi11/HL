/**
* @file            fortrend_loadlock_open_diaphragm_valve_command.h
* @brief           open diaphragm valve command for loadlock
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

#include "LoadLock/fortrend_loadlock_open_diaphragm_valve_command.h"
#include "LoadLock/fortrend_loadlock_subsystem.h"


#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

namespace FC{

	
	/**
	* LoadLockOpenDiaphragmValveCommandPrivate
	*/
	class LoadLockOpenDiaphragmValveCommandPrivate{
	public:
		LoadLockValveOpening opening = LoadLockValveOpening::LoadLock_Both;
	};

	std::vector<IKernelResources* > LoadLockOpenDiaphragmValveCommand::resources() const{
		return std::vector<IKernelResources* >();
	}

	/**
	* LoadLockOpenDiaphragmValveCommand
	*/
	LoadLockOpenDiaphragmValveCommand::LoadLockOpenDiaphragmValveCommand(const LoadLockValveOpening opening, KeyencePlcSubSystemHelper* hexHelper)
		:KeyencePlcCommandExecuter(hexHelper)
		, d(new LoadLockOpenDiaphragmValveCommandPrivate){
		d->opening = opening;
		//setMessageName("OpenDiaphragmValve");
		//setDescription("open diaphragm valve the laodlock");
	};
	
	/**
	* return true if success else false.
	*/
	LoadLockOpenDiaphragmValveCommand::RunResult LoadLockOpenDiaphragmValveCommand::onRun() throw(KernelException){
		FortrendLoadLockSubsystem* sub = dynamic_cast<FortrendLoadLockSubsystem*>(getSubsystem());
		//
		if (!sub){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "子系统类型错误.", this);
		}
		if (sub->getCassetteDoorOpend())
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_DOOR_EXCEPTION, Poco::format("工位: %s 放晶圆盒门阀已打开（逻辑错误）", sub->getName()), this);
		}
		if (sub->getTMCavityDoorOpend())
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_DOOR_EXCEPTION, Poco::format("工位: %s 传输腔门阀已打开（逻辑错误）", sub->getName()), this);
		}
		if (sub->getAngleValveOpend())
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_DOOR_EXCEPTION, Poco::format("工位: %s 角阀已打开（逻辑错误）", sub->getName()), this);
		}


		if (d->opening == LoadLockValveOpening::LoadLock_Fast || d->opening == LoadLockValveOpening::LoadLock_Both)
		{
			if (sub->getQuickInflationValueReachesTheSetValue() == false)
			{
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_DOOR_EXCEPTION, Poco::format("工位: %s 腔室当前压力未达到快充设定条件（逻辑错误）", sub->getName()), this);
			}
		}
		//get command configure
		std::shared_ptr<KernelConfiguration> command_config = sub->getConfigure()->createView(getName());

		//fill params
		std::string address1 = command_config->getString("address1", "");//隔膜阀慢充
		std::string address2 = command_config->getString("address2", "");//隔膜阀快充
		if (address1 == "" || address2 == "")
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, Poco::format("地址: 打开隔膜阀地址未定义.", getName()), this);
		}
		//if (SIMULATION_TEST == 1)
		//{
		//	Sleep(200);
		//	logInform(sub->getName().c_str(), "模拟打开隔膜阀命令执行完成...");
		//	return RunResult::RUN_OK;
		//}
		logInform(sub->getName().c_str(), "打开隔膜阀命令开始.");
		IKernelCommand::RunResult ret = IKernelCommand::RunResult::RUN_FAILD;
		bool write_result = false;
		
		if (d->opening == LoadLockValveOpening::LoadLock_Slow)
		{
			logInform(sub->getName().c_str(), "打开隔膜阀慢充 %s", address1);
			write_result = writeBit(address1, true);
			write_result = writeBit(address2, false);
		}
		else if (d->opening == LoadLockValveOpening::LoadLock_Fast)
		{
			logInform(sub->getName().c_str(), "打开隔膜阀快充 %s", address1);
			write_result = writeBit(address2, true);
			write_result = writeBit(address1, false);
		}
		else
		{
			//logInform(sub->getName().c_str(), "打开隔膜阀快慢充 %s %s", address1,address2);
			write_result = (writeBit(address1, true) && writeBit(address2, true));
		}
		if (write_result)
		{
			ret = IKernelCommand::RunResult::RUN_OK;
			//if (d->opening == LoadLockValveOpening::LoadLock_Slow || d->opening == LoadLockValveOpening::LoadLock_Both)
			//{
			//	sub->setSlowDiaphragmValveOpend(true);
			//}
			//if (d->opening == LoadLockValveOpening::LoadLock_Fast || d->opening == LoadLockValveOpening::LoadLock_Both)
			//{
			//	sub->setFastDiaphragmValveOpend(true);
			//}
			
			logInform(sub->getName().c_str(), "打开隔膜阀命令执行结束.");
		}
		else
		{
			AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_COMMUNICATION_ERROR, "打开隔膜阀命令执行错误"));
			setAlarm(alarm);
			logError(sub->getName().c_str(), "打开隔膜阀命令执行错误.");
		}
		return ret;

	}



}