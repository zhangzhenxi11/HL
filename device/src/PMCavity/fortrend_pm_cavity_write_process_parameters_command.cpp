/**
* @file            fortrend_pm_cavity_write_process_parameters_command.h
* @brief           write process parameters command for pm cavity
* @author		   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/PMCavity



#include "Poco/Format.h"

#include "Kernel/kernel.h"
#include "Kernel/kernel_command.h"
#include "kernel/kernel_command_reject_exception.h"
#include "Kernel/kernel_log.h"
#include "kernel/Fortrend/fortrend_cassette_manager.h"


#include "PMCavity/fortrend_pm_cavity_write_process_parameters_command.h"
#include "PMCavity/fortrend_pm_cavity_subsystem.h"

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif


namespace FC{

	/**
	* PMCavityWriteProcessParametersCommandPrivate
	*/
	class PMCavityWriteProcessParametersCommandPrivate{
	public:
		PMCavityAxisSettingParameters _axis_parames;
		std::vector<ParameterMapping> _mapping_table;

	};

	/**
	* PMCavityWriteProcessParametersCommand
	*/
	PMCavityWriteProcessParametersCommand::PMCavityWriteProcessParametersCommand(const PMCavityAxisSettingParameters axis_parames, KeyencePlcSubSystemHelper* hexHelper)
		:KeyencePlcCommandExecuter(hexHelper)
		, d(new PMCavityWriteProcessParametersCommandPrivate){
		d->_axis_parames = axis_parames;
		d->_mapping_table = getMappingTable();
	}

	/**
	* return true if success else false.
	*/
	PMCavityWriteProcessParametersCommand::RunResult PMCavityWriteProcessParametersCommand::onRun() throw(KernelException){
		//return IKernelCommand::RunResult::RUN_OK;
		FortrendPMCavitySubsystem* sub = dynamic_cast<FortrendPMCavitySubsystem*>(getSubsystem());
		//
		if (!sub){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "子系统类型错误", this);
		}
		//if (!sub->getPMCavityRemoteMode())
		//{
		//	throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, "PM腔未处于远程模式状态", this);
		//}
		
		//get command configure
		std::shared_ptr<KernelConfiguration> command_config = sub->getConfigure()->createView(getName());
		logInform(sub->getName().c_str(), "写参数命令开始执行.");

		// 使用循环读取所有参数
		for (const auto& mapping : d->_mapping_table)
		{
			std::string address = command_config->getString(mapping.config_key, "");

			if (address.empty())
			{
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT,
					Poco::format("address: %s not defined", sub->getName()), this);
			}
			if (mapping.config_key == "lifting_axis_jerk_address" || mapping.config_key == "rotating_axis_jerk_address")
			{
				auto& value = d->_axis_parames.*(mapping.member_ptr);
				uint32_t _value = uint32_t(value);
				if (!readUnsignedInt(address, _value))
				{
					throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR,
						Poco::format(" %s写%s错误.", sub->getName(), mapping.description), this);
				}
			}

			float& value = d->_axis_parames.*(mapping.member_ptr);

			if (!writeFloat(address, value)) 
			{
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR,
					Poco::format(" %s写%s错误.", sub->getName(), mapping.description), this);
			}	
		}

		logInform(sub->getName().c_str(), "写工艺参数命令执行结束.");
		return IKernelCommand::RunResult::RUN_OK;

	}
}