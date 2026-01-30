/**
* @file            fortrend_tm_cavity_open_pid_command.h
* @brief           open pid command for tm cavity
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


#include "TMCavity/fortrend_tm_cavity_open_pid_command.h"
#include "TMCavity/fortrend_tm_cavity_subsystem.h"

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif


namespace FC{



	/**
	* TMCavityOpenPIDCommand
	*/
	TMCavityOpenPIDCommand::TMCavityOpenPIDCommand(KeyencePlcSubSystemHelper* hexHelper)
		:KeyencePlcCommandExecuter(hexHelper){
		//setMessageName("OpenPID");
		//setDescription("open pid the tm cavity");
	};


	/**
	* return true if success else false.
	*/
	TMCavityOpenPIDCommand::RunResult TMCavityOpenPIDCommand::onRun() throw(KernelException){
		FortrendTMCavitySubsystem* sub = dynamic_cast<FortrendTMCavitySubsystem*>(getSubsystem());
		//
		if (!sub){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "子系统类型错误.", this);
		}
		
		if (sub->getSlowDiaphragmValveOpend() == false)
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, Poco::format("子系统: %s 传输腔慢充隔膜阀未打开，无法打开PID（逻辑错误）", getName()), this);
		}
		if (sub->getTMCavityVacuumValue() > 3.0)
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, Poco::format("子系统: %s 传输腔当前真空压力大于3Pa无法打开PID（逻辑错误）", getName()), this);
		}
		//get command configure
		std::shared_ptr<KernelConfiguration> command_config = sub->getConfigure()->createView(getName());

		//fill params
		std::string open_address = command_config->getString("open_address", "");
		std::string close_address = command_config->getString("close_address", "");
		

		if ((open_address == "") || (close_address == ""))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, Poco::format("地址:打开PID命令地址未定义.", getName()), this);
		}
		if (!writeBit("MR507", false))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 写0到关闭慢充地址错误.", sub->getName()), this);
		}
		Sleep(100);
		if (!writeBit("MR508", false))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 写1到关闭快充地址错误.", sub->getName()), this);
		}
		Sleep(100);
		if (!writeBit("MR509", false))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 写1到关闭充气地址错误.", sub->getName()), this);
		}
		logInform(sub->getName().c_str(), "打开PID命令开始.");
		if (!writeBit(close_address, false))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 写0到关闭PID地址错误.", sub->getName()), this);
		}
		Sleep(100);
		if (!writeBit(open_address, true))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 写1到打开PID地址错误.", sub->getName()), this);
		}
		sub->setPIDOpend(true);
		logInform(sub->getName().c_str(), "打开PID命令执行完成.");
		return IKernelCommand::RunResult::RUN_OK;;

	}



}