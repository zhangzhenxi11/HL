/**
* @file            fortrend_pm_cavity_read_process_parameters_command.h
* @brief           read process parameters command for pm cavity
* @author		   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/PMCavity

#include <Windows.h>
#include <vector>
#include "Poco/Format.h"

#include "Kernel/kernel.h"
#include "Kernel/kernel_command.h"
#include "kernel/kernel_command_reject_exception.h"
#include "Kernel/kernel_log.h"
#include "kernel/Fortrend/fortrend_cassette_manager.h"


#include "PMCavity/fortrend_pm_cavity_read_process_parameters_command.h"
#include "PMCavity/fortrend_pm_cavity_subsystem.h"

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif


namespace FC{

	/**
* PMCavityReadProcessParametersCommandPrivate
*/
	class PMCavityReadProcessParametersCommandPrivate {
	public:
		
		std::vector<ParameterMapping> _mapping_table;

	};

	/**
	* PMCavityReadProcessParametersCommand
	*/
	PMCavityReadProcessParametersCommand::PMCavityReadProcessParametersCommand(KeyencePlcSubSystemHelper* helper)
		:KeyencePlcCommandExecuter(helper){

		d->_mapping_table = getMappingTable();

	};

	/**
	* return true if success else false.
	*/
	PMCavityReadProcessParametersCommand::RunResult PMCavityReadProcessParametersCommand::onRun() throw(KernelException){
		//return IKernelCommand::RunResult::RUN_OK;
		FortrendPMCavitySubsystem* sub = dynamic_cast<FortrendPMCavitySubsystem*>(getSubsystem());
		//
		if (!sub){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "子系统类型错误", this);
		}
		//get command configure
		std::shared_ptr<KernelConfiguration> command_config = sub->getConfigure()->createView(getName());
		logInform(sub->getName().c_str(), "读参数命令开始执行");
		PMCavityAxisSettingParameters axis_parames;

		// 使用循环读取所有参数
		for (const auto& mapping : d->_mapping_table)
		{
			std::string address = command_config->getString(mapping.config_key, "");

			if (address.empty())
			{
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT,
					Poco::format("address: %s not defined", sub->getName()), this);
			}

			float& value = axis_parames.*(mapping.member_ptr);

			if (!readFloat(address, value))
			{
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR,
					Poco::format(" %s读取%s错误", sub->getName(), mapping.description), this);
			}

		}
	
		sub->setPMCavityAxisParameters(axis_parames);
		logInform(sub->getName().c_str(), "读取电机参数命令执行结束");
		return IKernelCommand::RunResult::RUN_OK;

	}



}