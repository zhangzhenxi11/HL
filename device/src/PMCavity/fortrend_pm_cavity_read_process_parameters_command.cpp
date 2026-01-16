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
#include <stdint.h>

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
		// 2025-10-28: 删除_mapping_table成员变量，直接使用getMappingTable()返回的static引用
		// std::vector<ParameterMapping> _mapping_table;  // 已删除
	};

	/**
	* PMCavityReadProcessParametersCommand
	*/
	PMCavityReadProcessParametersCommand::PMCavityReadProcessParametersCommand(KeyencePlcSubSystemHelper* helper)
		:KeyencePlcCommandExecuter(helper)
		, d(std::make_shared<PMCavityReadProcessParametersCommandPrivate>())  // 2025-10-28: 初始化d指针，避免空指针访问
	{
		// 2025-10-28: 不需要赋值，直接使用getMappingTable()返回的static引用，避免不必要的拷贝
		// d->_mapping_table = getMappingTable(); // 删除此行，因为不再需要存储在成员变量中
	}

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

		// 2025-10-28: 直接使用getMappingTable()返回的static const引用，避免从成员变量读取
		const auto& mapping_table = getMappingTable();
		
		// 使用循环读取所有参数
		for (const auto& mapping : mapping_table)
		{
			std::string address = command_config->getString(mapping.config_key, "");

			if (address.empty())
			{
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT,
					Poco::format("address: %s not defined", sub->getName()), this);
			}
			//readUnsignedInt 
			if (mapping.config_key == "lifting_axis_jerk_address" || mapping.config_key == "rotating_axis_jerk_address")
			{
				float& value = axis_parames.*(mapping.member_ptr);
				uint32_t _value = uint32_t(value);
				if (!readUnsignedInt(address, _value))
				{
					throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR,
						Poco::format(" %s读取%s错误", sub->getName(), mapping.description), this);
				}
				value = float(_value);
			}
			else
			{
				float& value = axis_parames.*(mapping.member_ptr);

				if (!readFloat(address, value))
				{
					throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR,
						Poco::format(" %s读取%s错误", sub->getName(), mapping.description), this);
				}
			}
		}
	
		sub->setPMCavityAxisParameters(axis_parames);
		logInform(sub->getName().c_str(), "读取电机参数命令执行结束");
		return IKernelCommand::RunResult::RUN_OK;

	}



}