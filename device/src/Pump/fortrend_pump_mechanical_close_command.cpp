/**
* @file            fortrend_pump_mechanical_close_command.h
* @brief           mechanical close command for Pump
* @author		   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/Pump

#include "Poco/Format.h"

#include "Kernel/kernel.h"
#include "Kernel/kernel_command.h"
#include "kernel/kernel_command_reject_exception.h"
#include "Kernel/kernel_log.h"
#include "kernel/Fortrend/fortrend_cassette_manager.h"

#include "Pump/fortrend_pump_mechanical_close_command.h"
#include "Pump/fortrend_pump_subsystem.h"

#include <windows.h>


#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif


namespace FC{

	

	/**
	* PumpMechanicalCloseCommand
	*/
	PumpMechanicalCloseCommand::PumpMechanicalCloseCommand(KeyencePlcSubSystemHelper* hexHelper)
		:KeyencePlcCommandExecuter(hexHelper){
		//setMessageName("MechanicalClose");
		//setDescription("align the pump");
	};

	
	/**
	* return true if success else false.
	*/
	PumpMechanicalCloseCommand::RunResult PumpMechanicalCloseCommand::onRun() throw(KernelException){
		FortrendPumpSubsystem* sub = dynamic_cast<FortrendPumpSubsystem*>(getSubsystem());
		//
		if (!sub){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "子系统类型错误", this);
		}

		if (sub->getMechanicalPumpHasAlarm())
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, Poco::format("工位: %s 机械泵报警中！", sub->getName()), this);
		}
		//get command configure
		std::shared_ptr<KernelConfiguration> command_config = sub->getConfigure()->createView(getName());

		std::string address = command_config->getString("start_address", "");
		std::string finish_address = command_config->getString("finish_address", "");
		std::string failed_address = command_config->getString("failed_address", "");
	    int	timeout = command_config->getInt("timeout", 45000);
		if (address == "" || finish_address == "" || failed_address=="")
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, 
				Poco::format("地址: 关闭机械泵地址未定义", getName()), this);
		}
		if (timeout < 10){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_DATA_OUTOF_RANGE, 
				Poco::format("超时: %s 关闭机械泵超时参数错误", sub->getName()), this);
		}

		logInform(sub->getName().c_str(), "关闭机械泵命令开始");
		if (!writeBit(address, true))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, 
				Poco::format(" %s 写1到关闭机械泵地址错误", sub->getName()), this);
		}
		Sleep(500);
		int loopCount = timeout / 20;
		int count = 0;
		bool readRes = false;
		bool readRes2 = false;
		while (count <= loopCount)
		{
			Sleep(20);
			readBit(finish_address, readRes);
			readBit(failed_address, readRes2);
			if (readRes || readRes2)
			{
				break;
			}
			count++;
		}
		IKernelCommand::RunResult ret = IKernelCommand::RunResult::RUN_FAILD;
		if (readRes)
		{
			ret = IKernelCommand::RunResult::RUN_OK;
			sub->setMechanicalPumpOpened(false);
			logInform(sub->getName().c_str(), "关闭机械泵命令执行结束");

		}
		else if (readRes2)
		{
			AlarmMessage::Ptr alarm(new AlarmMessage(0, 2, "关闭机械泵命令执行错误"));
			setAlarm(alarm);
		}
		else
		{
			AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_COMMUNICATION_TIMEOUT, "关闭机械泵命令执行超时"));
			setAlarm(alarm);
		}

		return ret;
		

	}



}