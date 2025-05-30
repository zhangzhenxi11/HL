/**
* @file            fortrend_pm_cavity_read_process_parameters_command.h
* @brief           read process parameters command for pm cavity
* @author		   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/PMCavity

#include <Windows.h>

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
	PMCavityReadProcessParametersCommand::PMCavityReadProcessParametersCommand(InovancePlcSubSystemHelper* helper)
		:InovancePlcCommandExecuter(helper){
		//setMessageName("ReadProcessParameters");
		//setDescription("read process parameters the pm cavity");
	};

	
	/**
	* return true if success else false.
	*/
	PMCavityReadProcessParametersCommand::RunResult PMCavityReadProcessParametersCommand::onRun() throw(KernelException){
		return IKernelCommand::RunResult::RUN_OK;
		FortrendPMCavitySubsystem* sub = dynamic_cast<FortrendPMCavitySubsystem*>(getSubsystem());
		//
		if (!sub){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "子系统类型错误", this);
		}
		//get command configure
		std::shared_ptr<KernelConfiguration> command_config = sub->getConfigure()->createView(getName());

		//fill params
		std::string heating_temperature_address = command_config->getString("heating_temperature_address", "");
		std::string initial_extraction_pressure_address = command_config->getString("initial_extraction_pressure_address", "");
		std::string purified_extraction_pressure_address = command_config->getString("purified_extraction_pressure_address", "");
		std::string sputtering_pressure_address = command_config->getString("sputtering_pressure_address", "");
		std::string sputtering_flow_rate1_address = command_config->getString("sputtering_flow_rate1_address", "");
		std::string sputtering_flow_rate2_address = command_config->getString("sputtering_flow_rate2_address", "");
		std::string sputtering_flow_rate3_address = command_config->getString("sputtering_flow_rate3_address", "");
		std::string sputtering_power1_address = command_config->getString("sputtering_power1_address", "");
		std::string sputtering_power_gear_up1_address = command_config->getString("sputtering_power_gear_up1_address", "");
		std::string sputtering_power2_address = command_config->getString("sputtering_power2_address", "");
		std::string sputtering_power_gear_up2_address = command_config->getString("sputtering_power_gear_up2_address", "");
		std::string sputtering_power3_address = command_config->getString("sputtering_power3_address", "");
		std::string sputtering_power_gear_up3_address = command_config->getString("sputtering_power_gear_up3_address", "");
		std::string pre_sputtering_time_address = command_config->getString("pre_sputtering_time_address", "");
		std::string substrate_speed_address = command_config->getString("substrate_speed_address", "");
		std::string process_sputtering_time_address = command_config->getString("process_sputtering_time_address", "");
		std::string cathode_power_selection_1_address = command_config->getString("cathode_power_selection_1_address", "");
		std::string cathode_power_selection_2_address = command_config->getString("cathode_power_selection_2_address", "");
		std::string cathode_power_selection_3_address = command_config->getString("cathode_power_selection_3_address", "");


		if ((heating_temperature_address == "") || (initial_extraction_pressure_address == "") || (purified_extraction_pressure_address == "") || (sputtering_pressure_address == "") ||
			(sputtering_flow_rate1_address == "") || (sputtering_flow_rate2_address == "") || (sputtering_flow_rate3_address == "") || (sputtering_power1_address == "") ||
			(sputtering_power_gear_up1_address == "") || (sputtering_power2_address == "") || (sputtering_power_gear_up2_address == "") || (sputtering_power3_address == "") ||
			(sputtering_power_gear_up3_address == "") || (pre_sputtering_time_address == "") || (substrate_speed_address == "") || (process_sputtering_time_address == "") ||
			(cathode_power_selection_1_address == "") || (cathode_power_selection_2_address == "") || (cathode_power_selection_3_address == ""))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, Poco::format("地址: 读取参数地址未定义", getName()), this);
		}
		PMCavityProcessParameters process_parameters;
		logInform(sub->getName().c_str(), "读取参数命令开始执行");
		if (!readFloat(heating_temperature_address, process_parameters.heating_temperature))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s读取温度设定错误", sub->getName()), this);
		}
		if (!readFloat(initial_extraction_pressure_address, process_parameters.initial_extraction_pressure))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s读取粗抽压力错误", sub->getName(), process_parameters.initial_extraction_pressure), this);
		}
		if (!readFloat(purified_extraction_pressure_address, process_parameters.purified_extraction_pressure))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s读取精抽压力错误", sub->getName(), process_parameters.purified_extraction_pressure), this);
		}
		if (!readFloat(sputtering_pressure_address, process_parameters.sputtering_pressure))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s读取溅射压力错误", sub->getName(), process_parameters.sputtering_pressure), this);
		}
		if (!readFloat(sputtering_flow_rate1_address, process_parameters.sputtering_flow_rate1))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s读取溅射流量1错误", sub->getName(), process_parameters.sputtering_flow_rate1), this);
		}
		if (!readFloat(sputtering_flow_rate2_address, process_parameters.sputtering_flow_rate2))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s读取溅射流量2错误", sub->getName(), process_parameters.sputtering_flow_rate2), this);
		}
		if (!readFloat(sputtering_flow_rate3_address, process_parameters.sputtering_flow_rate3))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s读取溅射流量3错误", sub->getName(), process_parameters.sputtering_flow_rate3), this);
		}
		if (!readFloat(sputtering_power1_address, process_parameters.sputtering_power1))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s读取溅射功率1错误", sub->getName(), process_parameters.sputtering_power1), this);
		}
		if (!readFloat(sputtering_power_gear_up1_address, process_parameters.sputtering_power_gear_up1))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s读取溅射功率1增速错误", sub->getName(), process_parameters.sputtering_power_gear_up1), this);
		}
		if (!readFloat(sputtering_power2_address, process_parameters.sputtering_power2))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s读取溅射功率2错误", sub->getName(), process_parameters.sputtering_power2), this);
		}
		if (!readFloat(sputtering_power_gear_up2_address, process_parameters.sputtering_power_gear_up2))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s读取溅射功率2增速错误", sub->getName(), process_parameters.sputtering_power_gear_up2), this);
		}
		if (!readFloat(sputtering_power3_address, process_parameters.sputtering_power3))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s读取溅射功率3错误", sub->getName(), process_parameters.sputtering_power3), this);
		}
		if (!readFloat(sputtering_power_gear_up3_address, process_parameters.sputtering_power_gear_up3))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s读取溅射功率3增速错误", sub->getName(), process_parameters.sputtering_power_gear_up3), this);
		}
		if (!readFloat(pre_sputtering_time_address, process_parameters.pre_sputtering_time))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s读取预溅射时间错误", sub->getName(), process_parameters.pre_sputtering_time), this);
		}
		if (!readFloat(substrate_speed_address, process_parameters.substrate_speed))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s读取工艺溅射旋转速度错误", sub->getName(), process_parameters.substrate_speed), this);
		}
		if (!readFloat(process_sputtering_time_address, process_parameters.process_sputtering_time))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s读取工艺溅射时间错误", sub->getName()), this);
		}
		if (!readInt(cathode_power_selection_1_address, process_parameters.cathode_power_selection_1))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s读取1#阴极电源旋转错误", sub->getName()), this);
		}
		if (!readInt(cathode_power_selection_2_address, process_parameters.cathode_power_selection_2))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s读取2#阴极电源旋转错误", sub->getName()), this);
		}
		if (!readInt(cathode_power_selection_3_address, process_parameters.cathode_power_selection_3))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s读取3#阴极电源旋转错误", sub->getName()), this);
		}
		sub->setPMCavityProcessParameters(process_parameters);
		logInform(sub->getName().c_str(), "读取参数命令执行结束");
		return IKernelCommand::RunResult::RUN_OK;

	}



}