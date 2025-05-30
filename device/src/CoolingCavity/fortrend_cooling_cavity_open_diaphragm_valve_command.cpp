/**
* @file            fortrend_cooling_cavity_open_diaphragm_valve_command.h
* @brief           open diaphragm valve command for cooling cavity
* @author		   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/CoolingCavity

#include "Kernel/kernel.h"
#include "Kernel/kernel_command.h"
#include "Kernel/kernel_abstract_subsystem.h"
#include "kernel/kernel_event_id.h"
#include "kernel/kernel_event_paramters.h"
#include "kernel/kernel_command_reject_exception.h"
#include "Kernel/kernel_log.h"
#include "kernel/Fortrend/fortrend_cassette_manager.h"
#include "Poco/Format.h"

#include "CoolingCavity/fortrend_cooling_cavity_open_diaphragm_valve_command.h"
#include "CoolingCavity/fortrend_cooling_cavity_subsystem.h"

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif


namespace FC{

	

	/**
	* CoolingCavityOpenDiaphragmValveCommand
	*/
	CoolingCavityOpenDiaphragmValveCommand::CoolingCavityOpenDiaphragmValveCommand(KeyencePlcSubSystemHelper* hexHelper)
		:KeyencePlcCommandExecuter(hexHelper){
		//setMessageName("OpenDiaphragmValve");
		//setDescription("open diaphragm valve the cooling cavity");
	};
	
	/**
	* return true if success else false.
	*/
	CoolingCavityOpenDiaphragmValveCommand::RunResult CoolingCavityOpenDiaphragmValveCommand::onRun() throw(KernelException){
		FortrendCoolingCavitySubsystem* sub = dynamic_cast<FortrendCoolingCavitySubsystem*>(getSubsystem());
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
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, Poco::format("地址: 打开隔膜阀地址未定义", getName()), this);
		}
		logInform(sub->getName().c_str(), "打开隔膜阀命令开始");
		IKernelCommand::RunResult ret = IKernelCommand::RunResult::RUN_FAILD;
		if (writeShort(address1, 1) && writeShort(address2, 1))
		{
			ret = IKernelCommand::RunResult::RUN_OK;
			logInform(sub->getName().c_str(), "打开隔膜阀命令执行开始");
		}
		else
		{
			AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_COMMUNICATION_ERROR, "打开隔膜阀命令通讯超时"));
			setAlarm(alarm);
			logError(sub->getName().c_str(), "打开隔膜阀命令通讯超时");
		}
		return ret;

	}



}