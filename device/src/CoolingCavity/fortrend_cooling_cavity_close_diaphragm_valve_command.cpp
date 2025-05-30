/**
* @file            fortrend_cooling_cavity_close_diaphragm_valve_command.h
* @brief           close diaphragm valve command for cooling cavity
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

#include "CoolingCavity/fortrend_cooling_cavity_close_diaphragm_valve_command.h"
#include "CoolingCavity/fortrend_cooling_cavity_subsystem.h"

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif


namespace FC{

	

	/**
	* CoolingCavityCloseDiaphragmValveCommand
	*/
	CoolingCavityCloseDiaphragmValveCommand::CoolingCavityCloseDiaphragmValveCommand(KeyencePlcSubSystemHelper* hexHelper)
		:KeyencePlcCommandExecuter(hexHelper){
		//setMessageName("CloseDiaphragmValve");
		//setDescription("close diaphragm valve the cooling cavity");
	};
	
	/**
	* return true if success else false.
	*/
	CoolingCavityCloseDiaphragmValveCommand::RunResult CoolingCavityCloseDiaphragmValveCommand::onRun() throw(KernelException){
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
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, Poco::format("地址: 关闭隔膜阀命令地址未定义", getName()), this);
		}
		logInform(sub->getName().c_str(), "关闭隔膜阀命令开始");
		
		IKernelCommand::RunResult ret = IKernelCommand::RunResult::RUN_FAILD;
		if (writeBit(address1, false) && writeBit(address2, false))
		{
			ret = IKernelCommand::RunResult::RUN_OK;
			logInform(sub->getName().c_str(), "关闭隔膜阀命令执行结束");

		}
		else
		{
			AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_COMMUNICATION_ERROR, "关闭隔膜阀命令通讯错误"));
			setAlarm(alarm);
			logError(sub->getName().c_str(), "关闭隔膜阀命令通讯错误");
		}
		return ret;

	}



}