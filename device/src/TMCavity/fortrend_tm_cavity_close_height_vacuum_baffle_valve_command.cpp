/**
* @file            fortrend_tm_cavity_close_height_vacuum baffle_valve_command.h
* @brief           close height vacuum baffle valve command for tm cavity
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

#include "TMCavity/fortrend_tm_cavity_close_height_vacuum_baffle_valve_command.h"
#include "TMCavity/fortrend_tm_cavity_subsystem.h"


#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif


namespace FC{

	

	/**
	* TMCavityCloseHeightVacuumBaffleValveCommand
	*/
	TMCavityCloseHeightVacuumBaffleValveCommand::TMCavityCloseHeightVacuumBaffleValveCommand(KeyencePlcSubSystemHelper* hexHelper)
		:KeyencePlcCommandExecuter(hexHelper){
		//setMessageName("CloseHeightVacuumBaffleValve");
		//setDescription("close height vacuum baffle valve to tm cavity");
	};
	
	/**
	* return true if success else false.
	*/
	TMCavityCloseHeightVacuumBaffleValveCommand::RunResult TMCavityCloseHeightVacuumBaffleValveCommand::onRun() throw(KernelException){
		FortrendTMCavitySubsystem* sub = dynamic_cast<FortrendTMCavitySubsystem*>(getSubsystem());
		//
		if (!sub){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "子系统类型错误", this);
		}

		//get command configure
		std::shared_ptr<KernelConfiguration> command_config = sub->getConfigure()->createView(getName());
		//fill params
		std::string address = command_config->getString("address", "");

		if (address == "")
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, Poco::format("地址: 关闭高真空挡板阀地址未定义", getName()), this);
		}
		logInform(sub->getName().c_str(), "关闭高真空挡板阀命令开始.");
		IKernelCommand::RunResult ret = IKernelCommand::RunResult::RUN_FAILD;
		if (writeBit(address, true))
		{
			ret = IKernelCommand::RunResult::RUN_OK;
			sub->setHeightVacuumBaffleValveOpend(false);
			logInform(sub->getName().c_str(), "关闭高真空挡板阀命令执行结束.");

		}
		
		else
		{
			AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_COMMUNICATION_ERROR, "关闭高真空挡板阀命令通讯错误"));
			setAlarm(alarm);
			logError(sub->getName().c_str(), "关闭高真空挡板阀命令通讯错误.");
		}
		return ret;

	}



}