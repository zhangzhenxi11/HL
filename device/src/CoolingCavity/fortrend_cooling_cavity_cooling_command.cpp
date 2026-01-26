/**
* @file            fortrend_cooling_cavity_cooling_command.h
* @brief           cooling command for cooling cavity
* @author		   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/CoolingCavity

#include <Windows.h>

#include "Poco/Format.h"

#include "Kernel/kernel.h"
#include "Kernel/kernel_command.h"
#include "kernel/kernel_command_reject_exception.h"
#include "Kernel/kernel_log.h"
#include "kernel/Fortrend/fortrend_cassette_manager.h"


#include "CoolingCavity/fortrend_cooling_cavity_cooling_command.h"
#include "CoolingCavity/fortrend_cooling_cavity_subsystem.h"

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif


namespace FC{

	

	/**
	* CoolingCavityCoolingCommand
	*/
	CoolingCavityCoolingCommand::CoolingCavityCoolingCommand(KeyencePlcSubSystemHelper* hexHelper)
		:KeyencePlcCommandExecuter(hexHelper){
		//setMessageName("Cooling");
		//setDescription("cooling the laodlock");
	};

	
	/**
	* return true if success else false.
	*/
	CoolingCavityCoolingCommand::RunResult CoolingCavityCoolingCommand::onRun() throw(KernelException){
		FortrendCoolingCavitySubsystem* sub = dynamic_cast<FortrendCoolingCavitySubsystem*>(getSubsystem());
		//
		if (!sub){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "子系统类型错误", this);
		}
		//get command configure
		std::shared_ptr<KernelConfiguration> command_config = sub->getConfigure()->createView(getName());

		//fisub params
		std::string start_address = command_config->getString("start_address", "");
		std::string finish_address = command_config->getString("finish_address", "");
		std::string failed_address = command_config->getString("failed_address", "");
		std::string error_code_address = command_config->getString("error_code_address", "");
		int timeout = command_config->getInt("timeout", -1);
		if (timeout < 10){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_DATA_OUTOF_RANGE, Poco::format("超时: 冷却命令超时时间设置错误", sub->getName()), this);
		}

		if ((start_address == "") || (finish_address == "") || (failed_address == "") || (error_code_address == ""))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, Poco::format("地址: 冷却命令地址未定义", getName()), this);
		}
		logInform(sub->getName().c_str(), "冷却命令执行开始.");
		if (!writeBit(start_address, true))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 写1到冷却命令地址失败", sub->getName()), this);
		}
		Sleep(100);
		int loopCount = timeout / 20;
		int count = 0;
		bool readRes[2];
		while (count <= loopCount)
		{
			Sleep(20);
			readBits(finish_address, 2, readRes);
			if (readRes[0] || readRes[1])
			{
				break;
			}
			count++;
		}
		if (!writeBit(start_address, false))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 写0到冷却命令地址失败", sub->getName()), this);
		}
		IKernelCommand::RunResult ret = IKernelCommand::RunResult::RUN_FAILD;
		if (readRes[0])
		{
			ret = IKernelCommand::RunResult::RUN_OK;
			logInform(sub->getName().c_str(), "冷却命令执行结束.");
			
		}
		else if (readRes[1])
		{
			int code = 0;
			readInt(error_code_address, code);
			auto code_message = getErrorCode(1, code);
			AlarmMessage::Ptr alarm(new AlarmMessage(code_message->type, code_message->code, code_message->message));
			setAlarm(alarm);
			logError(sub->getName().c_str(), "冷却命令执行失败.");
		}
		else
		{
			AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_COMMUNICATION_TIMEOUT, "冷却命令通讯超时"));
			setAlarm(alarm);
			logError(sub->getName().c_str(), "冷却命令通讯超时.");
		}
		return ret;

	}



}