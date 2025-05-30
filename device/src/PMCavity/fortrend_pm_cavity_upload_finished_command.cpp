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


#include "PMCavity/fortrend_pm_cavity_upload_finished_command.h"
#include "PMCavity/fortrend_pm_cavity_subsystem.h"

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif


namespace FC{



	/**
	* PMCavityUploadFinishedCommand
	*/
	PMCavityUploadFinishedCommand::PMCavityUploadFinishedCommand(InovancePlcSubSystemHelper* helper)
		:InovancePlcCommandExecuter(helper){
		//setMessageName("UploadFinished");
		//setDescription("upload request the pm cavity");
	};


	/**
	* return true if success else false.
	*/
	PMCavityUploadFinishedCommand::RunResult PMCavityUploadFinishedCommand::onRun() throw(KernelException){
		return IKernelCommand::RunResult::RUN_OK;
		FortrendPMCavitySubsystem* sub = dynamic_cast<FortrendPMCavitySubsystem*>(getSubsystem());
		//
		if (!sub){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "子系统类型错误", this);
		}
		if (sub->hasDoorOpend())
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_DOOR_EXCEPTION, Poco::format("工位: %s 传输腔门阀未关闭（逻辑错误）", sub->getName()), this);
		}
		//check modules
		auto cassManager = sub->getKernel()->getKernelModule<FortrendCassetteManager>();
		//get cass
		auto station_cass = cassManager->getCassette(sub);
		if (!station_cass){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_STATION_WITHOUT_CASS_EXCEPTION, Poco::format("工位: %s 晶圆盒为空.", sub->getName()), this);
		}
		if (station_cass->getMapping(1) == Cassette::Mapping::Empty)
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, Poco::format("工位: %s 腔目前不存在晶圆（逻辑错误）", sub->getName()), this);
		}
		if (!sub->getPMCavityUploadRequest())
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, Poco::format("工位: %s 腔未发出上片请求（逻辑错误）", sub->getName()), this);
		}
		//get command configure
		std::shared_ptr<KernelConfiguration> command_config = sub->getConfigure()->createView(getName());
		//fill params
		std::string write_address = command_config->getString("write_address", "");
		std::string finish_reset_address = command_config->getString("finish_reset_address", "");
		
		int timeout = command_config->getInt("timeout", -1);
		if (timeout < 10){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_DATA_OUTOF_RANGE, Poco::format("超时: 上片完成超时参数设置失败", sub->getName()), this);
		}

		if ((write_address == "") || (finish_reset_address == ""))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, Poco::format("地址: 上片完成地址未定义", getName()), this);
		}

		logInform(sub->getName().c_str(), "上片完成命令开始执行");
		if (!writeBit(write_address, true))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s写1到上片完成(%s)地址失败", sub->getName(), write_address), this);
		}
		
		/*bool write_state = false;
		bool allow_reset = false;
		Sleep(4000);
		if (writeBit(write_address, false))
		{
			write_state = true;
		}*/
		Sleep(20);
		int loopCount = timeout / 50;
		int count = 0;
		short readRes = -1;
		bool write_state = false;
		bool allow_reset = false;
		while (count <= loopCount && (!write_state))
		{
			if (allow_reset == false)
			{
				readShort(finish_reset_address, readRes);
				if (readRes >= 130)
				{
					allow_reset = true;
				}
			}
			else
			{
				Sleep(100);
				if (writeBit(write_address, false))
				{
					write_state = true;
					break;
				}
			}
			Sleep(50);
			count++;
		}
		IKernelCommand::RunResult ret = IKernelCommand::RunResult::RUN_FAILD;
		if (write_state)
		{
			
			ret = IKernelCommand::RunResult::RUN_OK;
			logInform(sub->getName().c_str(), "上片完成命令执行完成");

		}
		else if (allow_reset)
		{
			AlarmMessage::Ptr alarm(new AlarmMessage(1, 1, "复位上片完成命令执行失败"));
			setAlarm(alarm);
			//logError(sub->getName().c_str(), "复位上片完成命令执行失败");
		}
		else
		{
			AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_COMMUNICATION_TIMEOUT, "复位上片完成信号超时"));
			setAlarm(alarm);
			//logError(sub->getName().c_str(), "复位上片完成信号超时");
		}
		return ret;
	}



}