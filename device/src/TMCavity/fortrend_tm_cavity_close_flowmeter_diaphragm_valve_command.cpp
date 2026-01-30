/**
* @file            fortrend_tm_cavity_close_flowmeter_diaphragm_valve_command.h
* @brief           close flowmeter diaphragm valve command for tm cavity
* @author		   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/TMCavity

#include <Windows.h>
#include "Poco/Format.h"

#include "Kernel/kernel.h"
#include "Kernel/kernel_command.h"
#include "kernel/kernel_command_reject_exception.h"
#include "Kernel/kernel_log.h"
#include "kernel/Fortrend/fortrend_cassette_manager.h"


#include "TMCavity/fortrend_tm_cavity_close_flowmeter_diaphragm_valve_command.h"
#include "TMCavity/fortrend_tm_cavity_subsystem.h"

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif


namespace FC{

	

	/**
	* TMCavityCloseFlowmeterDiaphragmValveCommand
	*/
	TMCavityCloseFlowmeterDiaphragmValveCommand::TMCavityCloseFlowmeterDiaphragmValveCommand(KeyencePlcSubSystemHelper* hexHelper)
		:KeyencePlcCommandExecuter(hexHelper){
		//setMessageName("CloseFlowmeterDiaphragmValve");
		//setDescription("close flowmeter diaphragm valve the tm cavity");
	};

	
	/**
	* return true if success else false.
	*/
	TMCavityCloseFlowmeterDiaphragmValveCommand::RunResult TMCavityCloseFlowmeterDiaphragmValveCommand::onRun() throw(KernelException){
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
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, Poco::format("地址:关闭流量计隔膜阀地址未定义.", getName()), this);
		}

		logInform(sub->getName().c_str(), "关闭流量计隔膜阀命令开始.");
		IKernelCommand::RunResult ret = IKernelCommand::RunResult::RUN_FAILD;
		if (writeBit(address1, false) && writeBit(address2, false) && writeBit(address3, false))
		{
			ret = IKernelCommand::RunResult::RUN_OK;
			logInform(sub->getName().c_str(), "关闭流量计隔膜阀命令执行结束.");
		}
		
		else
		{
			AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_COMMUNICATION_ERROR, "关闭流量计隔膜阀命令通讯错误"));
			setAlarm(alarm);
			logError(sub->getName().c_str(), "关闭流量计隔膜阀命令通讯错误.");
		}
		return ret;

	}



}