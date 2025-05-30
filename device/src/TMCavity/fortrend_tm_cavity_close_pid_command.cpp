/**
* @file            fortrend_tm_cavity_close_pid_command.h
* @brief           close pid command for tm cavity
* @author		   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/TMCavity
#include <Windows.h>

#include "Poco/Format.h"

#include "Kernel/kernel.h"
#include "Kernel/kernel_command.h"
#include "kernel/kernel_command_reject_exception.h"
#include "Kernel/kernel_log.h"
#include "kernel/Fortrend/fortrend_cassette_manager.h"


#include "TMCavity/fortrend_tm_cavity_close_pid_command.h"
#include "TMCavity/fortrend_tm_cavity_subsystem.h"

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif


namespace FC{



	/**
	* TMCavityClosePIDCommand
	*/
	TMCavityClosePIDCommand::TMCavityClosePIDCommand(KeyencePlcSubSystemHelper* hexHelper)
		:KeyencePlcCommandExecuter(hexHelper){
		//setMessageName("ClosePID");
		//setDescription("close angle valve the tm cavity");
	};


	/**
	* return true if success else false.
	*/
	TMCavityClosePIDCommand::RunResult TMCavityClosePIDCommand::onRun() throw(KernelException){
		FortrendTMCavitySubsystem* sub = dynamic_cast<FortrendTMCavitySubsystem*>(getSubsystem());
		//
		if (!sub){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "子系统类型错误.", this);
		}
		//get command configure
		std::shared_ptr<KernelConfiguration> command_config = sub->getConfigure()->createView(getName());

		//fill params
		std::string open_address = command_config->getString("open_address", "");
		std::string close_address = command_config->getString("close_address", "");
		

		if ((open_address == "") || (close_address == ""))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, Poco::format("地址:关闭角阀命令地址未定义", getName()), this);
		}
		logInform(sub->getName().c_str(), "关闭PID命令开始");
		if (!writeBit(open_address, false))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 写0到打开PID地址错误", sub->getName()), this);
		}
		if (!writeBit(close_address, true))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 写1到关闭PID地址错误", sub->getName()), this);
		}
		sub->setPIDOpend(false);
		logInform(sub->getName().c_str(), "关闭PID命令执行完成");
		return  IKernelCommand::RunResult::RUN_OK;;

	}



}