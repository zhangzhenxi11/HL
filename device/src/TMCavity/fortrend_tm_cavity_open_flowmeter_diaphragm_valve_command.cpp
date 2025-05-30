/**
* @file            fortrend_tm_cavity_open_flowmeter_diaphragm_valve_command.h
* @brief           open flowmeter diaphragm valve command for tm cavity
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

#include "TMCavity/fortrend_tm_cavity_open_flowmeter_diaphragm_valve_command.h"
#include "TMCavity/fortrend_tm_cavity_subsystem.h"

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif


namespace FC{

	

	/**
	* TMCavityOpenFlowmeterDiaphragmValveCommand
	*/
	TMCavityOpenFlowmeterDiaphragmValveCommand::TMCavityOpenFlowmeterDiaphragmValveCommand(KeyencePlcSubSystemHelper* hexHelper)
		:KeyencePlcCommandExecuter(hexHelper){
		//setMessageName("OpenFlowmeterDiaphragmValveTheTMCavity");
		//setDescription("open flowmeter diaphragm valve the tm cavity");
	};
	
	/**
	* return true if success else false.
	*/
	TMCavityOpenFlowmeterDiaphragmValveCommand::RunResult TMCavityOpenFlowmeterDiaphragmValveCommand::onRun() throw(KernelException){
		FortrendTMCavitySubsystem* sub = dynamic_cast<FortrendTMCavitySubsystem*>(getSubsystem());
		//
		if (!sub){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "子系统类型错误", this);
		}
		
		//get command configure
		std::shared_ptr<KernelConfiguration> command_config = sub->getConfigure()->createView(getName());

		//fill params
		std::string address1 = command_config->getString("address1", "");
		std::string address2 = command_config->getString("address2", "");
		std::string address3 = command_config->getString("address3", "");

		if ((address1 == "") || (address2 == "") || (address3 == ""))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, Poco::format("地址: 打开流量计隔膜阀命令地址未定义", getName()), this);
		}

		logInform(sub->getName().c_str(), "打开流量计隔膜阀命令开始");
		IKernelCommand::RunResult ret = IKernelCommand::RunResult::RUN_FAILD;
		if (writeBit(address1, true) && writeBit(address2, true) && writeBit(address3, true))
		{
			ret = IKernelCommand::RunResult::RUN_OK;
			logInform(sub->getName().c_str(), "打开流量计隔膜阀命令执行结束");
		}

		else
		{
			AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_COMMUNICATION_ERROR, "打开流量计隔膜阀命令通讯超时"));
			setAlarm(alarm);
			logError(sub->getName().c_str(), "打开流量计隔膜阀命令通讯超时");
		}
		return ret;

	}



}