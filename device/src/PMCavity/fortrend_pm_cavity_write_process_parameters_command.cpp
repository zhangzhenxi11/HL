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
		PMCavityProcessParameters process_parameters;
	};

	/**
	* PMCavityWriteProcessParametersCommand
	*/
	PMCavityWriteProcessParametersCommand::PMCavityWriteProcessParametersCommand(const PMCavityProcessParameters process_parameters, InovancePlcSubSystemHelper* hexHelper)
		:InovancePlcCommandExecuter(hexHelper)
		, d(new PMCavityWriteProcessParametersCommandPrivate){
		d->process_parameters = process_parameters;
		//setMessageName("WriteProcessParameters");
		//setDescription("write process parameters the pm cavity");
	};

	
	/**
	* return true if success else false.
	*/
	PMCavityWriteProcessParametersCommand::RunResult PMCavityWriteProcessParametersCommand::onRun() throw(KernelException){
		return IKernelCommand::RunResult::RUN_OK;
		FortrendPMCavitySubsystem* sub = dynamic_cast<FortrendPMCavitySubsystem*>(getSubsystem());
		//
		if (!sub){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "子系统类型错误", this);
		}
		if (!sub->getPMCavityRemoteMode())
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, "PM腔未处于远程模式状态", this);
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
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, Poco::format("address: %s not defined", getName()), this);
		}
		logInform(sub->getName().c_str(), "写入工艺参数命令开始执行");
		if (!writeFloat(heating_temperature_address, d->process_parameters.heating_temperature))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 写 %f 到温度设定地址错误", sub->getName(), d->process_parameters.heating_temperature), this);
		}
		if (!writeFloat(initial_extraction_pressure_address, d->process_parameters.initial_extraction_pressure))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 写 %f 到粗抽压力地址错误", sub->getName(), d->process_parameters.initial_extraction_pressure), this);
		}
		if (!writeFloat(purified_extraction_pressure_address, d->process_parameters.purified_extraction_pressure))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 写 %f 到精抽压力地址错误", sub->getName(), d->process_parameters.purified_extraction_pressure), this);
		}
		if (!writeFloat(sputtering_pressure_address, d->process_parameters.sputtering_pressure))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 写 %f 到溅射压力地址错误", sub->getName(), d->process_parameters.sputtering_pressure), this);
		}
		if (!writeFloat(sputtering_flow_rate1_address, d->process_parameters.sputtering_flow_rate1))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 写 %f 到溅射流量1地址错误", sub->getName(), d->process_parameters.sputtering_flow_rate1), this);
		}
		if (!writeFloat(sputtering_flow_rate2_address, d->process_parameters.sputtering_flow_rate2))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 写 %f 到溅射流量2地址错误", sub->getName(), d->process_parameters.sputtering_flow_rate2), this);
		}
		if (!writeFloat(sputtering_flow_rate3_address, d->process_parameters.sputtering_flow_rate3))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 写 %f 到溅射流量3地址错误", sub->getName(), d->process_parameters.sputtering_flow_rate3), this);
		}
		if (!writeFloat(sputtering_power1_address, d->process_parameters.sputtering_power1))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 写 %f 到溅射功率1地址错误", sub->getName(), d->process_parameters.sputtering_power1), this);
		}
		if (!writeFloat(sputtering_power_gear_up1_address, d->process_parameters.sputtering_power_gear_up1))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 写 %f 到溅射功率1增速地址错误", sub->getName(), d->process_parameters.sputtering_power_gear_up1), this);
		}
		if (!writeFloat(sputtering_power2_address, d->process_parameters.sputtering_power2))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 写 %f 到溅射功率2地址错误", sub->getName(), d->process_parameters.sputtering_power2), this);
		}
		if (!writeFloat(sputtering_power_gear_up2_address, d->process_parameters.sputtering_power_gear_up2))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 写 %f 到溅射功率2增速地址错误", sub->getName(), d->process_parameters.sputtering_power_gear_up2), this);
		}
		if (!writeFloat(sputtering_power3_address, d->process_parameters.sputtering_power3))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 写 %f 到溅射功率3地址错误", sub->getName(), d->process_parameters.sputtering_power3), this);
		}
		if (!writeFloat(sputtering_power_gear_up3_address, d->process_parameters.sputtering_power_gear_up3))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 写 %f 到溅射功率3增速地址错误", sub->getName(), d->process_parameters.sputtering_power_gear_up3), this);
		}
		if (!writeFloat(pre_sputtering_time_address, d->process_parameters.pre_sputtering_time))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 写 %f 到预溅射时间地址错误", sub->getName(), d->process_parameters.pre_sputtering_time), this);
		}
		if (!writeFloat(substrate_speed_address, d->process_parameters.substrate_speed))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 写 %f 到工艺溅射旋转速度地址错误", sub->getName(), d->process_parameters.substrate_speed), this);
		}
		if (!writeFloat(process_sputtering_time_address, d->process_parameters.process_sputtering_time))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 写 %f 到工艺溅射时间地址错误", sub->getName(), d->process_parameters.process_sputtering_time), this);
		}
		if (!writeInt(cathode_power_selection_1_address, d->process_parameters.cathode_power_selection_1))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 写 %d 到1#阴极电源选择地址错误", sub->getName(), d->process_parameters.cathode_power_selection_1), this);
		}
		if (!writeInt(cathode_power_selection_2_address, d->process_parameters.cathode_power_selection_2))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 写 %d 到2#阴极电源选择地址错误", sub->getName(), d->process_parameters.cathode_power_selection_2), this);
		}
		if (!writeInt(cathode_power_selection_3_address, d->process_parameters.cathode_power_selection_3))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 写 %d 到3#阴极电源选择地址错误", sub->getName(), d->process_parameters.cathode_power_selection_3), this);
		}
		logInform(sub->getName().c_str(), "写工艺参数命令执行结束");
		return IKernelCommand::RunResult::RUN_OK;

	}



}