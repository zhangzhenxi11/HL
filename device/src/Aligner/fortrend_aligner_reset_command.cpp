/**
* @file            fortrend_aligner_reset_command.h
* @brief           reset command for sub
* @author		   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/Aligner

#include "Poco/Format.h"

#include "kernel/kernel.h"
#include "kernel/kernel_api.h"
#include "kernel/kernel_log.h"
#include "kernel/kernel_command_reject_exception.h"
#include "kernel/Fortrend/fortrend_cassette_manager.h"
#include "Kernel/Fortrend/robot_abstract_subsystem.h"
#include "Kernel/kernel_command_runner.h"
#include "Kernel/kernel_subsystem_update_command.h"

#include "Aligner/fortrend_aligner_reset_command.h"
#include "Aligner/fortrend_aligner_subsystem.h"

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif


namespace FC{

	

	/**
	* AlignerResetCommand
	*/
	AlignerResetCommand::AlignerResetCommand(AlignerSubSystemHelper* helper)
		:AlignerCommandExecuter(helper){
		

		
		
	};


	/**
	* return true if success else false.
	*/
	AlignerResetCommand::RunResult AlignerResetCommand::onRun() throw(KernelException){
		FortrendAlignerSubsystem* sub = dynamic_cast<FortrendAlignerSubsystem*>(getSubsystem());
		if (!sub) throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "子系统类型错误", this);
		for (auto robot : sub->getRobots()){
			if (robot->getState() != IKernelSubSystem::State::SUB_NORMAL){
				//throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_STATE_EXCEPTION, Poco::format("机械手: %s 状态不在正常状态.", robot->getName()), this);
			}
		}
		//get command configure
		std::shared_ptr<KernelConfiguration> command_config = sub->getConfigure()->createView(getName());

		//fill params
		
		int timeout = command_config->getInt("timeout", 30000);
		if (timeout < 10){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_DATA_OUTOF_RANGE, Poco::format("超时: 复位指令超时时间设置错误.", sub->getName()), this);
		}
		logInform(sub->getName().c_str(), "复位指令开始.");
		
		IKernelCommand::RunResult ret = IKernelCommand::RunResult::RUN_FAILD;
		if (sendRequest("ALGN RESET "))
		{
			std::string recv_data = recvResponse(timeout);
			if (recv_data == "_RDY ")
			{
				ret = IKernelCommand::RunResult::RUN_OK;
				logInform(sub->getName().c_str(), "复位指令执行结束.");
			}
			else{
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_DATA_OUTOF_RANGE, Poco::format("子系统: 接收命令错误,返回数据：%s", recv_data), this);

			}

		}
		
		else
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_DATA_OUTOF_RANGE, Poco::format("子系统: 发送复位命令错误.", sub->getName()), this);

		}
		return ret;
	}



}