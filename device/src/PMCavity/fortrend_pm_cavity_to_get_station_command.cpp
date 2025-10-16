/**
* @file            fortrend_pm_cavity_upload_request_command.h
* @brief           upload request command for pm cavity
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


#include "PMCavity/fortrend_pm_cavity_to_get_station_command.h"
#include "PMCavity/fortrend_pm_cavity_subsystem.h"
#include "Kernel/kernel_event_paramters.h"
#include "fortrend_device_kernel.h"
#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif


namespace FC{



	/**
	* PMCavityToGetStationCommand
	*/
	PMCavityToGetStationCommand::PMCavityToGetStationCommand(KeyencePlcSubSystemHelper* helper)
		:KeyencePlcCommandExecuter(helper){
		//setMessageName("GetFinished");
		//setDescription("get request the pm cavity");
	};


	/**
	* return true if success else false.
	*/
	PMCavityToGetStationCommand::RunResult PMCavityToGetStationCommand::onRun() throw(KernelException){
		FortrendPMCavitySubsystem* sub = dynamic_cast<FortrendPMCavitySubsystem*>(getSubsystem());
		KernelCommandParameter parameter(shared_from_this());
		//
		if (!sub){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "子系统类型错误", this);
		}
		/*if (sub->hasDoorOpend())
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_DOOR_EXCEPTION, Poco::format("工位: %s 传输腔门阀未关闭（逻辑错误）", sub->getName()), this);
		}*/
		//check modules
		auto cassManager = sub->getKernel()->getKernelModule<FortrendCassetteManager>();
		//get cass
		auto station_cass = cassManager->getCassette(sub);
		if (!station_cass){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_STATION_WITHOUT_CASS_EXCEPTION, Poco::format("工位: %s 晶圆盒为空.", sub->getName()), this);
		}
		/*if (station_cass->getMapping(1) == Cassette::Mapping::Present)
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, "PM腔目前存在晶圆！", this);
		}*/
		std::shared_ptr<KernelConfiguration> command_config = sub->getConfigure()->createView(getName());

		//fill params
		std::string start_address = command_config->getString("start_address", "");
		std::string finish_address = command_config->getString("finish_address", "");
		std::string failed_address = command_config->getString("failed_address", "");
		std::string abs_position_address = command_config->getString("abs_position_address", "");
		std::string axis_target1_position_address = command_config->getString("axis_target1_position_address", "");
	
		int timeout = command_config->getInt("timeout", -1);
		if (timeout < 10){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_DATA_OUTOF_RANGE, Poco::format("超时: 去取料位命令超时参数设置错误", sub->getName()), this);
		}

		if ((start_address == "") || (finish_address == "") || (failed_address == "") || (abs_position_address == "")||(axis_target1_position_address == ""))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, Poco::format("地址: 去取料位命令地址未定义", getName()), this);
		}

		logInform(sub->getName().c_str(), "去取料位命令开始执行");
		sub->sendEvent(NEW_EVENT_ID_WITHNAME(EVENT_COMMAND_RUNNING), &parameter);

		float axis_target1_pos = 0.0F;
		if (!readFloat(axis_target1_position_address, axis_target1_pos))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 读设置的取料位命令地址错误，地址%s", sub->getName(), axis_target1_position_address), this);
		}

		if (!writeFloat(abs_position_address, axis_target1_pos))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 写去取料位位置命令地址错误，地址%s", sub->getName(), abs_position_address), this);
		}

		if (!writeBit(start_address, true))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 写1到去取料位命令地址错误，地址%s", sub->getName(), start_address), this);
		}
		Sleep(100);
		int loopCount = timeout / 20;
		int count = 0;
		bool readRes;
		bool failedRes;
		while (count <= loopCount)
		{
			Sleep(20);
			readBit(finish_address, readRes);
			readBit(failed_address, failedRes);
			if (readRes || failedRes)
			{
				break;
			}
			count++;
		}

		if (!writeBit(start_address, false))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 写0到去取料位命令地址错误", sub->getName()), this);
		}
		IKernelCommand::RunResult ret = IKernelCommand::RunResult::RUN_FAILD;
		if (readRes)
		{
			sub->setPMCavitySafeSignal(true);
			ret = IKernelCommand::RunResult::RUN_OK;
			logInform(sub->getName().c_str(), "去取料位命令执行完成");

		}
		else if (!readRes || failedRes)
		{
			AlarmMessage::Ptr alarm(new AlarmMessage(1, 1, "去取料位命令执行失败"));
			setAlarm(alarm);
		}
		else
		{
			AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_COMMUNICATION_TIMEOUT, "执行去取料位命令信号超时"));
			setAlarm(alarm);
		}

		return ret;
	}



}