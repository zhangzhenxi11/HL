/**
* @file            fortrend_loadlock_reset_command.h
* @brief           reset command for loadlock
* @author		   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/LoadLock

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

#include "LoadLock/fortrend_loadlock_defined.h"
#include "LoadLock/fortrend_loadlock_reset_command.h"
#include "LoadLock/fortrend_loadlock_subsystem.h"

#include "EFEM/efem_wafer_robot_subsystem.h" 


#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

namespace FC{



	/**
	* LoadLockResetCommand
	*/
	LoadLockResetCommand::LoadLockResetCommand(KeyencePlcSubSystemHelper* helper)
		:KeyencePlcCommandExecuter(helper){




	};


	/**
	* return true if success else false.
	*/
	LoadLockResetCommand::RunResult LoadLockResetCommand::onRun() throw(KernelException){
		FortrendLoadLockSubsystem* sub = dynamic_cast<FortrendLoadLockSubsystem*>(getSubsystem());
		if (!sub) throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "子系统类型错误", this);
		//if (!sub->getProtrudingSensorState())
		//{
		//	throw KernelCommandRejectException(__FILE__, KernelSysException::KR_STATION_CONFLICT_EXCEPTION, Poco::format("工位： %s 检测到凸片", sub->getName()), this);
		//}
		for (auto robot : sub->getRobots()){
			if (robot->getState() != IKernelSubSystem::State::SUB_NORMAL){
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_STATE_EXCEPTION, Poco::format("机械手: %s 状态不在正常状态", robot->getName()), this);
			}
		}
		/*if (getSubsystem()->getName()=="LLA"){
			auto smif = getSubsystem()->getKernel()->getKernelModule<EFEMWaferRobotSubsystem>("EWTR");
			if (smif&&smif->getState() != IKernelSubSystem::State::SUB_NORMAL){
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_STATE_EXCEPTION, Poco::format("%s 状态不在正常状态", smif->getName()), this);
			}
		}
		else{
			auto smif = getSubsystem()->getKernel()->getKernelModule<EFEMWaferRobotSubsystem>("EWTR");
			if (smif&&smif->getState() != IKernelSubSystem::State::SUB_NORMAL){
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_STATE_EXCEPTION, Poco::format("%s 状态不在正常状态", smif->getName()), this);
			}
		}*/
		
		//get command configure
		std::shared_ptr<KernelConfiguration> command_config = sub->getConfigure()->createView(getName());

		//fill params
		std::string start_address = command_config->getString("start_address", "");
		std::string finish_address = command_config->getString("finish_address", "");
		std::string failed_address = command_config->getString("failed_address", "");
		std::string error_code_address = command_config->getString("error_code_address", "");
		int timeout = command_config->getInt("timeout", -1);
		if (timeout < 10){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_DATA_OUTOF_RANGE, Poco::format("超时: 复位命令超时参数设置错误", sub->getName()), this);
		}

		if ((start_address == "") || (finish_address == "") || (failed_address == "") || (error_code_address == ""))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, Poco::format("地址: 复位命令地址未定义", getName()), this);
		}
		sub->setBoxPlacement(false);
		logInform(sub->getName().c_str(), "复位命令开始");
		if (!writeBit(start_address, true))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 写1到复位命令地址错误", sub->getName()), this);
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
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 写0到复位命令地址错误", sub->getName()), this);
		}
		IKernelCommand::RunResult ret = IKernelCommand::RunResult::RUN_FAILD;
		if (readRes[0])
		{
			//sub->setCassetteDoorOpend(false);
			//sub->setTMCavityDoorOpend(false);
			sub->setHasResetFlag(true);
			ret = IKernelCommand::RunResult::RUN_OK;
			logInform(sub->getName().c_str(), "复位命令执行结束");

		}
		else if (readRes[1])
		{
			short code = 0;
			readShort(error_code_address, code);
			auto code_message = getErrorCode(LoadLockErrorCommand::Reset, code);
			AlarmMessage::Ptr alarm(new AlarmMessage(code_message->type, code_message->code, code_message->message));
			setAlarm(alarm);
			logError(sub->getName().c_str(), "复位命令执行错误");
		}
		else
		{
			AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_COMMUNICATION_TIMEOUT, "复位命令通讯超时"));
			setAlarm(alarm);
			logError(sub->getName().c_str(), "复位命令通讯超时");
		}

		return ret;

	}



}