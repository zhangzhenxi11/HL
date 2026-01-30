
/**
* @file            fortrend_pm_cavity_update_command.h
* @brief           update command for pm_cavity
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


#include "PMCavity/fortrend_pm_cavity_update_command.h"
#include "PMCavity/fortrend_pm_cavity_subsystem.h"

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif


namespace FC{

	

	/**
	* PMCavityUpdateCommand
	*/
	PMCavityUpdateCommand::PMCavityUpdateCommand(KeyencePlcSubSystemHelper* hexHelper)
		:KeyencePlcCommandExecuter(hexHelper){
		
	};
	
	/**
	* return true if success else false.
	*/
	PMCavityUpdateCommand::RunResult PMCavityUpdateCommand::onRun() throw(KernelException){
		FortrendPMCavitySubsystem* sub = dynamic_cast<FortrendPMCavitySubsystem*>(getSubsystem());
		//
		if (!sub){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "子系统类型错误", this);
		}
		
		//get command configure
		std::shared_ptr<KernelConfiguration> command_config = sub->getConfigure()->createView(getName());
		std::string open_tm_cavity_door_address = command_config->getString("open_tm_cavity_door_address", "");
		std::string close_tm_cavity_door_address = command_config->getString("close_tm_cavity_door_address", "");

		std::string lifting_axis_upper_enable_address = command_config->getString("lifting_axis_upper_enable_address", "");
		std::string rotating_axis_upper_enable_address = command_config->getString("rotating_axis_upper_enable_address", "");
		std::string lifting_axis_enable_address = command_config->getString("lifting_axis_enable_address", "");
		std::string rotating_axis_enable_address = command_config->getString("rotating_axis_enable_address", "");

		if (open_tm_cavity_door_address == "" || close_tm_cavity_door_address == "" ||(lifting_axis_upper_enable_address =="")||(rotating_axis_upper_enable_address =="")
			||(lifting_axis_enable_address =="")||(rotating_axis_enable_address ==""))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, Poco::format("地址: 获取状态地址未定义.", getName()), this);
		}
		IKernelCommand::RunResult ret = IKernelCommand::RunResult::RUN_OK;
		
		bool open_tm_cavity_door_value = false;
		bool close_tm_cavity_door_value = false;
		if (readBit(open_tm_cavity_door_address, open_tm_cavity_door_value) && readBit(close_tm_cavity_door_address, close_tm_cavity_door_value))
		{
			if (open_tm_cavity_door_value && !close_tm_cavity_door_value)
			{
				sub->setDoorOpen(true);
			}
			else if ((!open_tm_cavity_door_value && close_tm_cavity_door_value))
			{
				sub->setDoorOpen(false);
			}
			else
			{
			}
		}
		bool lifting_axis_enable = false;
		bool rotating_axis_enable = false;
		int count = 0;
		int loopCount = 10;

		while (count <= loopCount)
		{
			Sleep(20);
			readBit(lifting_axis_enable_address, lifting_axis_enable);
			readBit(rotating_axis_enable_address, rotating_axis_enable);

			if (lifting_axis_enable && rotating_axis_enable)
			{
				break;
			}
			count++;
		}

		if (!lifting_axis_enable)
		{
			if (!writeBit(lifting_axis_upper_enable_address, true))
			{
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s Z轴上使能命令地址错误，地址%s", sub->getName(), lifting_axis_upper_enable_address), this);
			}
		}

		if (!rotating_axis_enable)
		{
			if (!writeBit(rotating_axis_upper_enable_address, true))
			{
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s R轴上使能命令命令地址错误，地址%s", sub->getName(), rotating_axis_upper_enable_address), this);
			}
		}

		//2025-10-24 注释
		//if (sub->readPMCavityHasObjectState())
		//{
		//	//check modules
		//	auto cassManager = sub->getKernel()->getKernelModule<FortrendCassetteManager>();
		//	//get cass
		//	auto station_cass = cassManager->getCassette(sub);
		//	if (!station_cass){
		//		throw KernelCommandRejectException(__FILE__, KernelSysException::KR_STATION_WITHOUT_CASS_EXCEPTION, Poco::format("工位: %s 晶圆盒为空.", sub->getName()), this);
		//	}
		//	std::vector<int> all_slot;
		//	std::vector<Cassette::Mapping> all_mapping;
		//	all_slot.push_back(1);
		//	all_mapping.push_back(Cassette::Mapping::Present);

		//	//station_cass->setMapping(all_slot, all_mapping);
		//}

		return ret;

	}



}