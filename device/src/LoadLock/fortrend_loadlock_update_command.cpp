
/**
* @file            fortrend_loadlock_update_command.h
* @brief           update command for loadlock
* @author		   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/LoadLock

#include "Poco/Format.h"

#include "Kernel/kernel.h"
#include "Kernel/kernel_abstract_subsystem.h"
#include "Kernel/kernel_command.h"
#include "kernel/kernel_command_reject_exception.h"
#include "Kernel/kernel_log.h"
#include "kernel/Fortrend/fortrend_cassette_manager.h"


#include "LoadLock/fortrend_loadlock_update_command.h"
#include "LoadLock/fortrend_loadlock_subsystem.h"


#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

namespace FC{

	

	/**
	* LoadLockUpdateCommand
	*/
	LoadLockUpdateCommand::LoadLockUpdateCommand(KeyencePlcSubSystemHelper* hexHelper)
		:KeyencePlcCommandExecuter(hexHelper){
		
	};
	
	/**
	* return true if success else false.
	*/
	LoadLockUpdateCommand::RunResult LoadLockUpdateCommand::onRun() throw(KernelException){
		FortrendLoadLockSubsystem* sub = dynamic_cast<FortrendLoadLockSubsystem*>(getSubsystem());
		//
		if (!sub){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "subsystem type error.", this);
		}
		//get command configure
		std::shared_ptr<KernelConfiguration> command_config = sub->getConfigure()->createView(getName());
		std::string open_cassette_door_address = command_config->getString("open_cassette_door_address", "");
		std::string close_cassette_door_address = command_config->getString("close_cassette_door_address", "");
		std::string open_tm_cavity_door_address = command_config->getString("open_tm_cavity_door_address", "");
		std::string close_tm_cavity_door_address = command_config->getString("close_tm_cavity_door_address", "");
		std::string diaphragm_valve_address1 = command_config->getString("diaphragm_valve_address1", "");
		std::string diaphragm_valve_address2 = command_config->getString("diaphragm_valve_address2", "");
		std::string open_angle_valve_address = command_config->getString("open_angle_valve_address", "");
		std::string close_angle_valve_address = command_config->getString("close_angle_valve_address", "");
		if (open_cassette_door_address == "" || close_cassette_door_address == "" ||
			open_tm_cavity_door_address == "" || close_tm_cavity_door_address == "" ||
			diaphragm_valve_address1 == "" || diaphragm_valve_address2 == "" ||
			open_angle_valve_address == "" || close_angle_valve_address == ""
			)
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, Poco::format("地址: 获取状态地址未定义", getName()), this);
		}
		IKernelCommand::RunResult ret = IKernelCommand::RunResult::RUN_OK;
		bool open_cassette_door_value = false;
		bool close_cassette_door_value = false;
		if (readBit(open_cassette_door_address, open_cassette_door_value) && readBit(close_cassette_door_address, close_cassette_door_value))
		{
			if (open_cassette_door_value && !close_cassette_door_value)
			{
				sub->setCassetteDoorOpend(true);
			}
			else if ((!open_cassette_door_value && close_cassette_door_value))
			{
				sub->setCassetteDoorOpend(false);
			}
			else
			{
			}
		}
		bool open_tm_cavity_door_value = false;
		bool close_tm_cavity_door_value = false;
		if (readBit(open_tm_cavity_door_address, open_tm_cavity_door_value) && readBit(close_tm_cavity_door_address, close_tm_cavity_door_value))
		{
			if (open_tm_cavity_door_value && !close_tm_cavity_door_value)
			{
				sub->setTMCavityDoorOpend(true);
			}
			else if (!open_tm_cavity_door_value && close_tm_cavity_door_value)
			{
				sub->setTMCavityDoorOpend(false);
			}
			else
			{
			}
		}
		bool diaphragm_valve_1_value = false;
		bool diaphragm_valve_2_value = false;
		if (readBit(diaphragm_valve_address1, diaphragm_valve_1_value))
		{
			if (diaphragm_valve_1_value )
			{
				sub->setSlowDiaphragmValveOpend(true);
			}
			
		}
		if (readBit(diaphragm_valve_address2, diaphragm_valve_2_value))
		{
			if (diaphragm_valve_2_value)
			{
				sub->setFastDiaphragmValveOpend(true);
			}
		}
		bool open_angle_valve_value = false;
		bool close_angle_valve_value = false;
		if (readBit(open_angle_valve_address, open_angle_valve_value) && readBit(close_angle_valve_address, close_angle_valve_value))
		{
			if (open_angle_valve_value && !close_angle_valve_value)
			{
				sub->setAngleValveOpend(true);
			}
			else if (!open_angle_valve_value && close_angle_valve_value)
			{
				sub->setAngleValveOpend(false);
			}
			else
			{
			}
		}
		return ret;

	}



}