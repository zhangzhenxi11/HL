
/**
* @file            fortrend_pump_update_command.h
* @brief           update command for pump
* @author		   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/Pump

#include "Poco/Format.h"

#include "Kernel/kernel.h"
#include "kernel/kernel_subsystem.h"
#include "Kernel/kernel_abstract_subsystem.h"
#include "Kernel/kernel_command.h"
#include "kernel/kernel_command_reject_exception.h"
#include "Kernel/kernel_log.h"
#include "kernel/Fortrend/fortrend_cassette_manager.h"


#include "Pump/fortrend_pump_update_command.h"
#include "Pump/fortrend_pump_subsystem.h"

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

namespace FC{

	

	/**
	* PumpUpdateCommand
	*/
	PumpUpdateCommand::PumpUpdateCommand(KeyencePlcSubSystemHelper* helper)
		:KeyencePlcCommandExecuter(helper){
		
	};
	
	/**
	* return true if success else false.
	*/
	PumpUpdateCommand::RunResult PumpUpdateCommand::onRun() throw(KernelException){
		FortrendPumpSubsystem* sub = dynamic_cast<FortrendPumpSubsystem*>(getSubsystem());
		//
		if (!sub){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "子系统类型错误", this);
		}
		//get command configure
		std::shared_ptr<KernelConfiguration> command_config = sub->getConfigure()->createView(getName());
		//std::string mechanical_pump_address = command_config->getString("mechanical_pump_address", "");

		//std::string llaAddress = command_config->getString("address_lla", "");
		//std::string llbAddress = command_config->getString("address_llb", "");
		//std::string tmAddress = command_config->getString("address_tm", "");
		//std::string llafinish_address = command_config->getString("finish_address_lla", "");
		//std::string llbfinish_address = command_config->getString("finish_address_llb", "");
		//std::string tmfinish_address = command_config->getString("finish_address_tm", "");

		//fill params
		IKernelCommand::RunResult ret = IKernelCommand::RunResult::RUN_OK;


		//if (sub->getState() != IKernelSubSystem::State::SUB_UNKNOWN)
		//{
		//	//机械泵
		//	bool readRes = false;
		//	if (readBit(mechanical_pump_address, readRes))
		//	{
		//		sub->setMechanicalPumpOpened(readRes);
		//	}
		//}
		return ret;

	}



}