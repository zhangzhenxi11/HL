/**
* @file            fortrend_pm_cavity_reset_command.h
* @brief           reset command for pm_cavity
* @author		   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/PMCavity

#include <Windows.h>

#include "Poco/Format.h"

#include "kernel/kernel.h"
#include "kernel/kernel_api.h"
#include "kernel/kernel_log.h"
#include "kernel/kernel_command_reject_exception.h"
#include "kernel/Fortrend/fortrend_cassette_manager.h"
#include "Kernel/Fortrend/robot_abstract_subsystem.h"
#include "Kernel/kernel_command_runner.h"
#include "Kernel/kernel_subsystem_update_command.h"

#include "PMCavity/fortrend_pm_cavity_reset_command.h"
#include "PMCavity/fortrend_pm_cavity_subsystem.h"

#include "Kernel/kernel_event_paramters.h"
#include "fortrend_device_kernel.h"

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif


namespace FC{

	

	/**
	* PMCavityResetCommand
	*/
	PMCavityResetCommand::PMCavityResetCommand(KeyencePlcSubSystemHelper* helper)
		:KeyencePlcCommandExecuter(helper){
		

		
		
	};


	/**
	* return true if success else false.
	*/
	PMCavityResetCommand::RunResult PMCavityResetCommand::onRun() throw(KernelException){
		FortrendPMCavitySubsystem* sub = dynamic_cast<FortrendPMCavitySubsystem*>(getSubsystem());
		KernelCommandParameter parameter(shared_from_this());

		if (!sub) throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "子系统类型错误", this);
		
		return IKernelCommand::RunResult::RUN_OK;

		for (auto robot : sub->getRobots()){
			if (robot->getState() != IKernelSubSystem::State::SUB_NORMAL){
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_STATE_EXCEPTION, Poco::format("机械手: %s 状态未处于正常状态.", robot->getName()), this);
			}
		}
		//get command configure
		std::shared_ptr<KernelConfiguration> command_config = sub->getConfigure()->createView(getName());

		//fill params
		std::string lifting_axis_start_address = command_config->getString("lifting_axis_start_address", "");
		std::string lifting_axis_finish_address = command_config->getString("lifting_axis_finish_address", "");
		std::string rotating_axis_start_address = command_config->getString("rotating_axis_start_address", "");
		std::string rotating_axis_finish_address = command_config->getString("rotating_axis_finish_address", "");
		std::string lifting_axis_alarm_address = command_config->getString("lifting_axis_alarm_address", "");
		std::string rotating_axis_alarm_address = command_config->getString("rotating_axis_alarm_address", "");
		int timeout = command_config->getInt("timeout", -1);
		if (timeout < 10){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_DATA_OUTOF_RANGE, Poco::format("超时: %s 复位命令超时参数设置错误.", sub->getName()), this);
		}

		if ((lifting_axis_start_address == "") || (lifting_axis_finish_address == "") || (rotating_axis_start_address == "") || (rotating_axis_finish_address == "")
			||(lifting_axis_alarm_address =="") ||(rotating_axis_alarm_address == ""))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, Poco::format("地址: 复位命令地址未定义.", getName()), this);
		}
		logInform(sub->getName().c_str(), "复位命令开始.");
		sub->sendEvent(NEW_EVENT_ID_WITHNAME(EVENT_COMMAND_RUNNING), &parameter);
		int32_t lifting_axis_alarm_code = 0;
		int32_t rotating_axis_alarm_code = 0;
		int loopCount = timeout / 20;
		int count = 0;
		bool readZaxisRes =false;
		bool readRaxisRes = false;
		IKernelCommand::RunResult ret = IKernelCommand::RunResult::RUN_FAILD;

		if (!readInt(lifting_axis_alarm_address, lifting_axis_alarm_code))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s读升降轴报警地址错误.", sub->getName()), this);

		}
		if (!readInt(rotating_axis_alarm_address, rotating_axis_alarm_code))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s读旋转轴报警地址错误.", sub->getName()), this);
		}

		if (lifting_axis_alarm_code != 0)
		{
			if (!writeBit(lifting_axis_start_address, true))
			{
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s写1到清除升降轴错误命令地址错误.", sub->getName()), this);
			}
			Sleep(100);

			while (count <= loopCount)
			{
				Sleep(20);
				readBit(lifting_axis_finish_address, readZaxisRes);
				if (readZaxisRes)
				{
					break;
				}
				count++;
			}

			if (readZaxisRes)
			{
				ret = IKernelCommand::RunResult::RUN_OK;
				logInform(sub->getName().c_str(), "清除升降轴错误命令执行结束.");

			}
			else
			{
				logError(sub->getName().c_str(), "清除升降轴错误命令执行失败.");
				auto code_message = getErrorCode(3, lifting_axis_alarm_code); 
				AlarmMessage::Ptr alarm(new AlarmMessage(code_message->type, code_message->code, code_message->message));
				setAlarm(alarm);
				return ret;

			}
		}

		else if (rotating_axis_alarm_code != 0)
		{
			int loopCount = timeout / 20;
			int count = 0;

			if (!writeBit(rotating_axis_start_address, true))
			{
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s写1到清除旋转轴错误命令地址错误.", sub->getName()), this);
			}

			while (count <= loopCount)
			{
				Sleep(20);
				readBit(rotating_axis_finish_address, readRaxisRes);
				if (readRaxisRes)
				{
					break;
				}
				count++;
			}

			if (readRaxisRes)
			{
				ret = IKernelCommand::RunResult::RUN_OK;
				logInform(sub->getName().c_str(), "清除旋转轴错误命令执行结束.");
			}
			else
			{
				logError(sub->getName().c_str(), "清除旋转轴错误命令执行失败.");
				auto code_message = getErrorCode(2, rotating_axis_alarm_code); //待定
				AlarmMessage::Ptr alarm(new AlarmMessage(code_message->type, code_message->code, code_message->message));
				setAlarm(alarm);
				return ret;
			}
		}

		else
		{
			ret = IKernelCommand::RunResult::RUN_OK;
			logInform(sub->getName().c_str(), "复位命令执行结束.");
		}

		return ret;
	}



}