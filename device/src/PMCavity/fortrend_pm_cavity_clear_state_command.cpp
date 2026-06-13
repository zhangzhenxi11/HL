/**
* @file            fortrend_pm_cavity_clear_state_command.h
* @brief           clear state command for pm_cavity
* @author		   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/PMCavity

#include "Poco/Format.h"

#include "Kernel/kernel.h"
#include "Kernel/kernel_abstract_subsystem.h"
#include "Kernel/kernel_command.h"
#include "kernel/kernel_command_reject_exception.h"
#include "Kernel/kernel_log.h"
#include "kernel/Fortrend/fortrend_cassette_manager.h"


#include "PMCavity/fortrend_pm_cavity_clear_state_command.h"
#include "PMCavity/fortrend_pm_cavity_subsystem.h"

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif


namespace FC{



	/**
	* PMCavityClearStateCommand
	*/
	PMCavityClearStateCommand::PMCavityClearStateCommand(KeyencePlcSubSystemHelper* hexHelper)
		:KeyencePlcCommandExecuter(hexHelper){

	};

	/**
	* return true if success else false.
	*/
	PMCavityClearStateCommand::RunResult PMCavityClearStateCommand::onRun() throw(KernelException){
		FortrendPMCavitySubsystem* sub = dynamic_cast<FortrendPMCavitySubsystem*>(getSubsystem());
		//
		if (!sub){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "子系统类型错误", this);
		}

		//get command configure
		std::shared_ptr<KernelConfiguration> command_config = sub->getConfigure()->createView(getName());
		std::string clear_state_address = command_config->getString("clear_state_address", "");
		if (clear_state_address == "")
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, Poco::format("地址: 清除状态地址未定义.", getName()), this);
		}
		IKernelCommand::RunResult ret = IKernelCommand::RunResult::RUN_OK;
		logInform(sub->getName().c_str(), "清除状态命令执行开始.");

		//2026-6-13 注释
		//if (!writeBit(clear_state_address, false))
		//{
		//	throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 写0到清除状态地址错误.", sub->getName()), this);
		//}
		//check modules
		auto cassManager = sub->getKernel()->getKernelModule<FortrendCassetteManager>();
		//get cass
		auto station_cass = cassManager->getCassette(sub);
		if (!station_cass) {
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_STATION_WITHOUT_CASS_EXCEPTION, Poco::format("工位: %s 晶圆盒为空.", sub->getName()), this);
		}
		std::vector<int> all_slot;
		std::vector<Cassette::Mapping> all_mapping;
		all_slot.push_back(1);
		all_mapping.push_back(Cassette::Mapping::Empty);
		station_cass->setMapping(all_slot, all_mapping);
		logInform(sub->getName().c_str(), "清除状态命令执行结束.");
		return ret;


	}



}
