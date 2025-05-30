/**
* @file            fortrend_pump_reset_command.h
* @brief           reset command for sub
* @author		   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/Pump

#include "Poco/Format.h"

#include "kernel/kernel.h"
#include "kernel/kernel_api.h"
#include "kernel/kernel_log.h"
#include "kernel/kernel_command_reject_exception.h"
#include "kernel/Fortrend/fortrend_cassette_manager.h"
#include "Kernel/Fortrend/robot_abstract_subsystem.h"
#include "Kernel/kernel_command_runner.h"
#include "Kernel/kernel_subsystem_update_command.h"

#include "Pump/fortrend_pump_reset_command.h"
#include "Pump/fortrend_pump_subsystem.h"

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif


namespace FC{

	

	/**
	* PumpResetCommand
	*/
	PumpResetCommand::PumpResetCommand(KeyencePlcSubSystemHelper* helper)
		:KeyencePlcCommandExecuter(helper){
		

		
		
	};


	/**
	* return true if success else false.
	*/
	PumpResetCommand::RunResult PumpResetCommand::onRun() throw(KernelException){
		FortrendPumpSubsystem* sub = dynamic_cast<FortrendPumpSubsystem*>(getSubsystem());
		if (!sub) throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "子系统类型错误", this);
		Sleep(3000);
		//get command configure
		return IKernelCommand::RunResult::RUN_OK;
		std::shared_ptr<KernelConfiguration> command_config = sub->getConfigure()->createView(getName());
		//fill params
		std::string mechanical_reset_address = command_config->getString("mechanical_reset_address", "");
		if (mechanical_reset_address == "")
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, Poco::format("地址: 机械泵报警复位地址未定义", getName()), this);
		}
		logInform(sub->getName().c_str(), "复位命令开始执行");
		IKernelCommand::RunResult ret = IKernelCommand::RunResult::RUN_FAILD;
		if (writeBit(mechanical_reset_address, true))
		{
			Sleep(500);
			if (writeBit(mechanical_reset_address, false))
			{
				ret = IKernelCommand::RunResult::RUN_OK;
				logInform(sub->getName().c_str(), "复位命令执行结束");
			}
			else{
				AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_COMMUNICATION_ERROR, "复位命令执行结束错误,原因：写0到机械泵报警复位地址失败。"));
				setAlarm(alarm);
			}
		}
		else
		{
			AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_COMMUNICATION_ERROR, "复位命令执行结束错误,原因：写1到机械泵报警复位地址失败。"));
			setAlarm(alarm);
		}
		return ret;
	}



}