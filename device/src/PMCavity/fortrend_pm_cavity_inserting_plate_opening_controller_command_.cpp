/**
* @file            fortrend_pm_cavity_upload_request_command.h
* @brief           upload request command for pm cavity
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


#include "PMCavity/fortrend_pm_cavity_inserting_plate_opening_controller_command.h"
#include "PMCavity/fortrend_pm_cavity_subsystem.h"

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif


namespace FC{

	/**
	*PMCavityInsertingPlateOpeningControllerCommandPrivate
	*/
	class PMCavityInsertingPlateOpeningControllerCommandPrivate{
	public:
		float percentage;
	};

	/**
	* PMCavityInsertingPlateOpeningControllerCommand
	*/
	PMCavityInsertingPlateOpeningControllerCommand::PMCavityInsertingPlateOpeningControllerCommand(float percentage, KeyencePlcSubSystemHelper* helper)
		:KeyencePlcCommandExecuter(helper)
		, d(new PMCavityInsertingPlateOpeningControllerCommandPrivate){
		//setMessageName("InsertingPlateOpeningController");
		//setDescription("get request the pm cavity");
		d->percentage = percentage;
	};


	/**
	* return true if success else false.
	*/
	PMCavityInsertingPlateOpeningControllerCommand::RunResult PMCavityInsertingPlateOpeningControllerCommand::onRun() throw(KernelException){
		return IKernelCommand::RunResult::RUN_OK;
		FortrendPMCavitySubsystem* sub = dynamic_cast<FortrendPMCavitySubsystem*>(getSubsystem());
		//
		if (!sub){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "子系统类型错误", this);
		}
		//get command configure
		std::shared_ptr<KernelConfiguration> command_config = sub->getConfigure()->createView(getName());
		//fill params
		std::string start_address = command_config->getString("start_address", "");
		std::string valve_opening_value_address = command_config->getString("valve_opening_value", "");

		int timeout = command_config->getInt("timeout", -1);
		if (timeout < 10){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_DATA_OUTOF_RANGE, Poco::format("超时: %s分子泵插板阀开度地址未定义超时参数设置失败.", sub->getName()), this);
		}

		if ((start_address == "") || (valve_opening_value_address == ""))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, Poco::format("地址: %s分子泵插板阀开度地址未定义.", getName()), this);
		}

		logInform(sub->getName().c_str(), Poco::format("控制%s分子泵插板阀开度命令开始执行.",sub->getName()).c_str());
		if (!writeFloat(valve_opening_value_address, d->percentage))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s写%d到分子泵插板阀开度(%s)地址失败.", sub->getName(), d->percentage, valve_opening_value_address), this);
		}

		IKernelCommand::RunResult ret = IKernelCommand::RunResult::RUN_FAILD;
		if (writeBit(start_address, true))
		{
			ret = IKernelCommand::RunResult::RUN_OK;
			logInform(sub->getName().c_str(), Poco::format("控制%s分子泵插板阀开度命令执行完成.", sub->getName()).c_str());

		}
		else
		{
			AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_COMMUNICATION_TIMEOUT, "控制分子泵插板阀开度命令失败"));
			setAlarm(alarm);
			logError(sub->getName().c_str(), Poco::format("控制%s分子泵插板阀开度命令执行失败.", sub->getName()).c_str());
		}
		return ret;
	}



}