/**
* @file            fortrend_pm_cavity_close_tm_cavity_door_command.h
* @brief           close tm cavity door command for pm cavity
* @author		   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/PMCavity
#include <Windows.h>

#include "Poco/Format.h"

#include "Kernel/kernel.h"
#include "Kernel/kernel_command.h"
#include "kernel/kernel_command_reject_exception.h"
#include "Kernel/kernel_log.h"
#include "kernel/Fortrend/fortrend_cassette_manager.h"


#include "PMCavity/fortrend_pm_cavity_close_tm_cavity_door_command.h"
#include "PMCavity/fortrend_pm_cavity_subsystem.h"

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif


namespace FC{

	

	/**
	* PMCavityCloseTMCavityDoorCommand
	*/
	PMCavityCloseTMCavityDoorCommand::PMCavityCloseTMCavityDoorCommand(KeyencePlcSubSystemHelper* hexHelper)
		:KeyencePlcCommandExecuter(hexHelper){
		//setMessageName("CloseTMCavityDoor");
		//setDescription("close tm cavity door the laodlock");
	};

	
	/**
	* return true if success else false.
	*/
	PMCavityCloseTMCavityDoorCommand::RunResult PMCavityCloseTMCavityDoorCommand::onRun() throw(KernelException){
		return IKernelCommand::RunResult::RUN_OK;
		FortrendPMCavitySubsystem* sub = dynamic_cast<FortrendPMCavitySubsystem*>(getSubsystem());
		//
		if (!sub){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "子系统类型错误", this);
		}
		//get command configure
		std::shared_ptr<KernelConfiguration> command_config = sub->getConfigure()->createView(getName());

		//fill params
		std::string open_address = command_config->getString("open_address", "");
		std::string close_address = command_config->getString("close_address", "");
		std::string finish_address = command_config->getString("finish_address", "");
		int timeout = command_config->getInt("timeout", -1);
		if (timeout < 10){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_DATA_OUTOF_RANGE, Poco::format("超时: 关闭传输腔门阀超时参数设置失败", sub->getName()), this);
		}

		if ((open_address == "") || (close_address == "") || (finish_address == ""))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, Poco::format("地址: 关闭传输腔门阀命令地址未定义", getName()), this);
		}
		logInform(sub->getName().c_str(), "关闭传输腔门阀命令开始");
		if (!writeBit(open_address, false))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 写 0 到打开传输腔门阀命令地址错误", sub->getName()), this);
		}
		Sleep(20);
		if (!writeBit(close_address, true))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 写 1 到关闭传输腔门阀命令地址错误", sub->getName()), this);
		}
		Sleep(500);
		int loopCount = timeout / 20;
		int count = 0;
		bool readRes = false;
		bool readState = false;
		while (count <= loopCount)
		{
			Sleep(20);
			readState = readBit(finish_address, readRes);
			if (readRes)
			{
				break;
			}
			count++;
		}
		
		IKernelCommand::RunResult ret = IKernelCommand::RunResult::RUN_FAILD;
		if (readRes)
		{
			sub->setDoorOpen(false);
			ret = IKernelCommand::RunResult::RUN_OK;
			logInform(sub->getName().c_str(), "关闭传输腔门阀命令执行结束");
			
		}
		else if (readState)
		{
			AlarmMessage::Ptr alarm(new AlarmMessage(1, 1, "关闭传输腔门阀命令执行失败，关闭传输腔门阀到位信号异常"));
			setAlarm(alarm);
			logError(sub->getName().c_str(), "关闭传输腔门阀命令执行失败，关闭传输腔门阀到位信号异常");
		}
		else
		{
			AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_COMMUNICATION_TIMEOUT, "关闭传输腔门阀命令超时"));
			setAlarm(alarm);
			logError(sub->getName().c_str(), "关闭传输腔门阀命令超时");
		}
		return ret;

	}



}