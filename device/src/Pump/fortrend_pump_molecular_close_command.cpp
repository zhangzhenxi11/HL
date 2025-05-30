/**
* @file            fortrend_pump_molecular_close_command.h
* @brief           molecular close command for Pump
* @author		   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/Pump

#include "Poco/Format.h"

#include "Kernel/kernel.h"
#include "Kernel/kernel_command.h"
#include "kernel/kernel_command_reject_exception.h"
#include "Kernel/kernel_log.h"
#include "kernel/Fortrend/fortrend_cassette_manager.h"

#include "Pump/fortrend_pump_molecular_close_command.h"
#include "Pump/fortrend_pump_subsystem.h"

#include <windows.h>

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif



namespace FC{

	

	/**
	* PumpMolecularCloseCommand
	*/
	PumpMolecularCloseCommand::PumpMolecularCloseCommand(KeyencePlcSubSystemHelper* helper, std::string name)
		:KeyencePlcCommandExecuter(helper){
		moduleName = name;
		//setMessageName("MolecularClose");
		//setDescription("align the pump");
	};

	std::string PumpMolecularCloseCommand::getmoduleName(){
		return moduleName;
	}
	/**
	* return true if success else false.
	*/
	PumpMolecularCloseCommand::RunResult PumpMolecularCloseCommand::onRun() throw(KernelException)
	{
		FortrendPumpSubsystem* sub = dynamic_cast<FortrendPumpSubsystem*>(getSubsystem());
		//
		if (!sub){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "子系统类型错误", this);
		}
		//get command configure
		std::shared_ptr<KernelConfiguration> command_config = sub->getConfigure()->createView(getName());
		std::string llaAddress = command_config->getString("start_address_lla", "");
		std::string llbAddress = command_config->getString("start_address_llb", "");
		std::string tmAddress = command_config->getString("start_address_tm", "");

		std::string llafinish_address = command_config->getString("finish_address_lla", "");
		std::string llbfinish_address = command_config->getString("finish_address_llb", "");
		std::string tmfinish_address = command_config->getString("finish_address_tm", "");

		std::string llafailed_address = command_config->getString("failed_address_lla", "");
		std::string llbfailed_address = command_config->getString("failed_address_llb", "");
		std::string tmfailed_address = command_config->getString("failed_address_tm", "");

		std::string address = "";
		std::string finish_address = "";
		std::string failed_address = "";
		int timeout = 0;
		if (moduleName == "LLA"){
			address = llaAddress;
			finish_address = llafinish_address;
			failed_address = llafailed_address;
			timeout = command_config->getInt("timeout_lla", 450000);
		}
		else if (moduleName == "LLB"){
			address = llbAddress;
			finish_address = llbfinish_address;
			failed_address = llbfailed_address;
			timeout = command_config->getInt("timeout_llb", 450000);
		}
		else if (moduleName == "TM"){
			address = tmAddress;
			finish_address = tmfinish_address;
			failed_address = tmfailed_address;
			timeout = command_config->getInt("timeout_tm", 450000);
		}

		if (timeout < 10){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_DATA_OUTOF_RANGE, Poco::format("超时: %s 关闭分子泵超时参数错误", sub->getName()), this);
		}

		if (address == "" || finish_address == "" || failed_address == "")
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, Poco::format("地址: 关闭分子泵地址未定义", getName()), this);
		}
		logInform(sub->getName().c_str(), "关闭分子泵命令执行开始 %s", moduleName);

		if (!writeBit(address, true))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 写1到关闭分子泵地址错误", sub->getName()), this);
		}
		Sleep(500);
		int loopCount = timeout / 20;
		int count = 0;
		bool readRes = false;
		bool readRes2 = false;
		while (count <= loopCount)
		{
			Sleep(20);
			readBit(finish_address, readRes);
			readBit(failed_address, readRes2);
			if (readRes || readRes2)
			{
				break;
			}
			count++;
		}
		IKernelCommand::RunResult ret = IKernelCommand::RunResult::RUN_FAILD;
		if (readRes)
		{
			ret = IKernelCommand::RunResult::RUN_OK;
			if (moduleName == "LLA"){
				sub->setMolecularPumpOpenedLLA(false);
			}
			else if (moduleName == "LLB"){
				sub->setMolecularPumpOpenedLLB(false);
			}
			else if (moduleName == "TM"){
				sub->setMolecularPumpOpenedTM(false);
			}
			logInform(sub->getName().c_str(), "关闭分子泵命令执行完成");

		}
		else if (readRes2)
		{
			AlarmMessage::Ptr alarm(new AlarmMessage(0, 2, "关闭分子泵命令执行失败"));
			setAlarm(alarm);
		}
		else
		{
			AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_COMMUNICATION_TIMEOUT, "关闭分子泵命令通讯超时"));
			setAlarm(alarm);
		}

		return ret;
	}



}