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

void  TestDumpGeneration()
{
	int* p = NULL;
	*p = 5;
}

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

		////测试dump
		//TestDumpGeneration();
		for (auto robot : sub->getRobots()){
			if (robot->getState() != IKernelSubSystem::State::SUB_NORMAL){
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_STATE_EXCEPTION, Poco::format("机械手: %s 状态不在正常状态", robot->getName()), this);
			}
		}
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
		logInform(sub->getName().c_str(), "复位命令开始.");
		if (!writeBit(start_address, true))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 写1到复位命令地址错误", sub->getName()), this);
		}
	
		if (!writeBit(start_address, false))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 写0到复位命令地址错误", sub->getName()), this);
		}
	
		return IKernelCommand::RunResult::RUN_OK;
	}

}