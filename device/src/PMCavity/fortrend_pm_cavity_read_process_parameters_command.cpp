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
	* PMCavityReadProcessParametersCommand
	*/
	PMCavityReadProcessParametersCommand::PMCavityReadProcessParametersCommand(KeyencePlcSubSystemHelper* helper)
		:KeyencePlcCommandExecuter(helper){

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

		std::vector<std::string> adddress_list;
		//fill params
		std::string lifting_axis_acce_address = command_config->getString("lifting_axis_acce_address", "");
		std::string lifting_axis_dece_address = command_config->getString("lifting_axis_dece_address", "");
		std::string lifting_axis_startup_speed_address = command_config->getString("lifting_axis_startup_speed_address", "");
		std::string lifting_axis_target_position_address = command_config->getString("lifting_axis_target_position_address", "");
		std::string lifting_axis_target_pressure_address = command_config->getString("lifting_axis_target_pressure_address", "");
		std::string lifting_axis_target1_position_address = command_config->getString("lifting_axis_target2_position_address", "");
		std::string lifting_axis_target2_position_address = command_config->getString("sputtering_flow_rate3_address", "");
		std::string lifting_axis_target3_position_address = command_config->getString("lifting_axis_target3_position_address", "");
		std::string lifting_axis_jog_speed_address = command_config->getString("lifting_axis_jog_speed_address", "");
		std::string lifting_axis_inch_movement_address = command_config->getString("lifting_axis_inch_movement_address", "");
		std::string rotating_axis_acce_address = command_config->getString("rotating_axis_acce_address", "");
		std::string rotating_axis_dece_address = command_config->getString("rotating_axis_dece_address", "");
		std::string rotating_axis_startup_speed_address = command_config->getString("rotating_axis_startup_speed_address", "");
		std::string rotating_axis_target_position_address = command_config->getString("rotating_axis_target_position_address", "");
		std::string rotating_axis_jog_speed_address = command_config->getString("rotating_axis_jog_speed_address", "");
		std::string rotating_axis_inch_movement_address = command_config->getString("rotating_axis_inch_movement_address", "");

		adddress_list.push_back(lifting_axis_acce_address);
		adddress_list.push_back(lifting_axis_dece_address);
		adddress_list.push_back(lifting_axis_startup_speed_address);
		adddress_list.push_back(lifting_axis_target_position_address);
		adddress_list.push_back(lifting_axis_target_pressure_address);
		adddress_list.push_back(lifting_axis_target1_position_address);
		adddress_list.push_back(lifting_axis_target2_position_address);
		adddress_list.push_back(lifting_axis_target3_position_address);
		adddress_list.push_back(lifting_axis_jog_speed_address);
		adddress_list.push_back(lifting_axis_inch_movement_address);
		adddress_list.push_back(rotating_axis_acce_address);
		adddress_list.push_back(rotating_axis_dece_address);
		adddress_list.push_back(rotating_axis_startup_speed_address);
		adddress_list.push_back(rotating_axis_target_position_address);
		adddress_list.push_back(rotating_axis_jog_speed_address);
		adddress_list.push_back(rotating_axis_inch_movement_address);
		for (auto _address : adddress_list)
		{
			if (_address.empty())
			{
				break;
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, 
					Poco::format("address: %s not defined", getName()), this);
			}
		}
		PMCavityAxisSettingParameters process_parameters;
		logInform(sub->getName().c_str(), "读取参数命令开始执行");
		
		if (!readDouble(lifting_axis_acce_address, process_parameters.lifting_axis_acce))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s读取升降轴加速度错误", sub->getName()), this);
		}
		if (!readDouble(lifting_axis_dece_address, process_parameters.lifting_axis_dece))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s读取升降轴减速度错误", sub->getName(), process_parameters.lifting_axis_dece), this);
		}
		if (!readDouble(lifting_axis_inch_movement_address, process_parameters.lifting_axis_inch_movement))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s读取升降轴启动速度错误", sub->getName(), process_parameters.lifting_axis_inch_movement), this);
		}
		if (!readDouble(lifting_axis_jog_speed_address, process_parameters.lifting_axis_jog_speed))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s读取升降轴目标坐标错误", sub->getName(), process_parameters.lifting_axis_jog_speed), this);
		}
		if (!readDouble(lifting_axis_startup_speed_address, process_parameters.lifting_axis_startup_speed))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s读取溅升降轴目标压力错误", sub->getName(), process_parameters.lifting_axis_startup_speed), this);
		}
		if (!readDouble(lifting_axis_target1_position_address, process_parameters.lifting_axis_target1_position))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s读取升降轴位置1错误", sub->getName(), process_parameters.lifting_axis_target1_position), this);
		}
		if (!readDouble(lifting_axis_target2_position_address, process_parameters.lifting_axis_target2_position))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s读取升降轴位置2错误", sub->getName(), process_parameters.lifting_axis_target2_position), this);
		}
		if (!readDouble(lifting_axis_target3_position_address, process_parameters.lifting_axis_target3_position))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s读取升降轴位置3错误", sub->getName(), process_parameters.lifting_axis_target3_position), this);
		}
		if (!readDouble(lifting_axis_target_position_address, process_parameters.lifting_axis_target_position))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s读取升降轴jog速度错误", sub->getName(), process_parameters.lifting_axis_target_position), this);
		}
		if (!readDouble(lifting_axis_target_pressure_address, process_parameters.lifting_axis_target_pressure))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s读取升降轴寸动移动量错误", sub->getName(), process_parameters.lifting_axis_target_pressure), this);
		}
		if (!readDouble(rotating_axis_acce_address, process_parameters.rotating_axis_acce))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s读取旋转轴加速度错误", sub->getName(), process_parameters.rotating_axis_acce), this);
		}
		if (!readDouble(rotating_axis_dece_address, process_parameters.rotating_axis_dece))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s读取旋转轴减速度错误", sub->getName(), process_parameters.rotating_axis_dece), this);
		}
		if (!readDouble(rotating_axis_startup_speed_address, process_parameters.rotating_axis_startup_speed))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s读取旋转轴定位速度错误", sub->getName(), process_parameters.rotating_axis_startup_speed), this);
		}
		if (!readDouble(rotating_axis_target_position_address, process_parameters.rotating_axis_target_position))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s读取旋转轴目标坐标错误", sub->getName(), process_parameters.rotating_axis_target_position), this);
		}
		if (!readDouble(rotating_axis_jog_speed_address, process_parameters.rotating_axis_jog_speed))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s读取旋转轴jog速度错误", sub->getName(), process_parameters.rotating_axis_jog_speed), this);
		}
		if (!readDouble(rotating_axis_inch_movement_address, process_parameters.rotating_axis_inch_movement))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s读取旋转轴寸动移动量错误", sub->getName()), this);
		}

		sub->setPMCavityAxisParameters(process_parameters);

		logInform(sub->getName().c_str(), "读取电机参数命令执行结束");
		return IKernelCommand::RunResult::RUN_OK;

	}



}