/**
* @file            fortrend_loadlock_close_diaphragm_valve_command.h
* @brief           close diaphragm valve command for loadlock
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

#include "LoadLock/fortrend_loadlock_close_diaphragm_valve_command.h"
#include "LoadLock/fortrend_loadlock_subsystem.h"

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

namespace FC{

	class LoadLockCloseDiaphragmValveCommandPrivate{
	public:
		LoadLockValveOpening opening = LoadLockValveOpening::LoadLock_Both;
	};

	/**
	* LoadLockCloseDiaphragmValveCommand
	*/
	LoadLockCloseDiaphragmValveCommand::LoadLockCloseDiaphragmValveCommand(const LoadLockValveOpening opening, KeyencePlcSubSystemHelper* hexHelper)
		:KeyencePlcCommandExecuter(hexHelper)
		, d(new LoadLockCloseDiaphragmValveCommandPrivate){
		d->opening = opening;
		//setMessageName("CloseDiaphragmValve");
		//setDescription("close diaphragm valve the laodlock");
	};
	std::vector<IKernelResources* > LoadLockCloseDiaphragmValveCommand::resources() const{
		return std::vector<IKernelResources* >();
	}
	/**
	* return true if success else false.
	*/
	LoadLockCloseDiaphragmValveCommand::RunResult LoadLockCloseDiaphragmValveCommand::onRun() throw(KernelException){
		FortrendLoadLockSubsystem* sub = dynamic_cast<FortrendLoadLockSubsystem*>(getSubsystem());
		//
		if (!sub){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "子系统类型错误", this);
		}
		
		
		
		//get command configure
		std::shared_ptr<KernelConfiguration> command_config = sub->getConfigure()->createView(getName());

		//fill params
		std::string address1 = command_config->getString("address1", "");
		std::string address2 = command_config->getString("address2", "");
		if (address1 == "" || address2 == "")
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, Poco::format("地址: 关闭隔膜阀地址未定义", getName()), this);
		}
		logInform(sub->getName().c_str(), "关闭隔膜阀命令开始");
		
		IKernelCommand::RunResult ret = IKernelCommand::RunResult::RUN_FAILD;
		bool write_result = false;
		if (d->opening == LoadLockValveOpening::LoadLock_Slow)
		{
			write_result = writeBit(address1, true);
		}
		else if (d->opening == LoadLockValveOpening::LoadLock_Fast)
		{
			write_result = writeBit(address2, true);
		}
		else{
			write_result = (writeBit(address1, true) && writeBit(address2, true));
		}
		if (write_result)
		{
			ret = IKernelCommand::RunResult::RUN_OK;
			if (d->opening == LoadLockValveOpening::LoadLock_Slow || d->opening == LoadLockValveOpening::LoadLock_Both)
			{
				sub->setSlowDiaphragmValveOpend(false);
			}
			if (d->opening == LoadLockValveOpening::LoadLock_Fast || d->opening == LoadLockValveOpening::LoadLock_Both)
			{
				sub->setFastDiaphragmValveOpend(false);
			}
			logInform(sub->getName().c_str(), "关闭隔膜阀命令结束");

		}
		else
		{
			AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_COMMUNICATION_ERROR, "关闭隔膜阀命令通讯错误"));
			setAlarm(alarm);
			//logError(sub->getName().c_str(), "关闭隔膜阀命令通讯错误");
		}
		return ret;

	}



}