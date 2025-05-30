
/**
* @file            fortrend_tm_cavity_update_command.h
* @brief           update command for tm_cavity
* @author		   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/TMCavity

#include "Poco/Format.h"

#include "Kernel/kernel.h"
#include "Kernel/kernel_abstract_subsystem.h"
#include "Kernel/kernel_command.h"
#include "kernel/kernel_command_reject_exception.h"
#include "Kernel/kernel_log.h"
#include "kernel/Fortrend/fortrend_cassette_manager.h"


#include "TMCavity/fortrend_tm_cavity_update_command.h"
#include "TMCavity/fortrend_tm_cavity_subsystem.h"

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif


namespace FC{

	

	/**
	* UpdateCommand
	*/
	TMCavityUpdateCommand::TMCavityUpdateCommand(KeyencePlcSubSystemHelper* hexHelper)
		:KeyencePlcCommandExecuter(hexHelper){
		
	};
	
	/**
	* return true if success else false.
	*/
	TMCavityUpdateCommand::RunResult TMCavityUpdateCommand::onRun() throw(KernelException){
		FortrendTMCavitySubsystem* sub = dynamic_cast<FortrendTMCavitySubsystem*>(getSubsystem());
		//
		if (!sub){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "subsystem type error.", this);
		}
		//get command configure
		std::shared_ptr<KernelConfiguration> command_config = sub->getConfigure()->createView(getName());
		std::string diaphragm_valve_address1 = command_config->getString("diaphragm_valve_address1", "");
		std::string diaphragm_valve_address2 = command_config->getString("diaphragm_valve_address2", "");
		std::string high_vacuum_baffle_value_address = command_config->getString("high_vacuum_baffle_value_address", "");
		std::string open_angle_valve_address = command_config->getString("open_angle_valve_address", "");
		std::string close_angle_valve_address = command_config->getString("close_angle_valve_address", "");
		std::string open_inserting_plate_valve_address = command_config->getString("open_inserting_plate_valve_address", "");
		std::string close_inserting_plate_valve_address = command_config->getString("close_inserting_plate_valve_address", "");
		std::string open_pid_address = command_config->getString("open_pid_address", "");
		std::string close_pid_address = command_config->getString("close_pid_address", "");
		if (diaphragm_valve_address1 == "" || diaphragm_valve_address2 == "" ||
			high_vacuum_baffle_value_address == "" ||
			open_angle_valve_address == "" || close_angle_valve_address == "" ||
			open_inserting_plate_valve_address == "" || close_inserting_plate_valve_address == "" ||
			open_pid_address == "" || close_pid_address == "")
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, Poco::format("地址: 获取状态地址未定义", getName()), this);
		}
		IKernelCommand::RunResult ret = IKernelCommand::RunResult::RUN_OK;

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
		bool high_vacuum_baffle_value_value = false;
		if (readBit(high_vacuum_baffle_value_address, high_vacuum_baffle_value_value))
		{
			if (high_vacuum_baffle_value_value)
			{
				sub->setHeightVacuumBaffleValveOpend(true);
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
			else if ((!open_angle_valve_value && close_angle_valve_value))
			{
				sub->setAngleValveOpend(false);
			}
			else
			{
			}
		}
		bool open_inserting_plate_valve_value = false;
		bool close_inserting_plate_valve_value = false;
		if (readBit(open_inserting_plate_valve_address, open_inserting_plate_valve_value) && readBit(close_inserting_plate_valve_address, close_inserting_plate_valve_value))
		{
			if (open_inserting_plate_valve_value && !close_inserting_plate_valve_value)
			{
				sub->setInsertingPlateValveOpend(true);
			}
			else if ((!open_inserting_plate_valve_value && close_inserting_plate_valve_value))
			{
				sub->setInsertingPlateValveOpend(false);
			}
			else
			{
			}
		}
		bool pid_opend_value = false;
		bool pid_close_value = false;

		if (readBit(open_pid_address, pid_opend_value) && readBit(close_pid_address, pid_close_value))
		{
			if (pid_opend_value && !pid_close_value)
			{
				sub->setPIDOpend(true);
			}
			else if ((!pid_opend_value && pid_close_value))
			{
				sub->setPIDOpend(false);
			}
			else
			{
			}
		}
		return ret;

	}



}