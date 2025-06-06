/**
* @file            fortrend_loadlock_open_angle_valve_command.h
* @brief           open angle valve command for loadlock
* @author		   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/LoadLock

#include "Kernel/kernel.h"
#include "Kernel/kernel_command.h"
#include "Kernel/kernel_abstract_subsystem.h"
#include "kernel/kernel_event_id.h"
#include "kernel/kernel_event_paramters.h"
#include "kernel/kernel_command_reject_exception.h"
#include "Kernel/kernel_log.h"
#include "kernel/Fortrend/fortrend_cassette_manager.h"
#include "Poco/Format.h"

#include "LoadLock/fortrend_loadlock_open_angle_valve_command.h"
#include "LoadLock/fortrend_loadlock_subsystem.h"
#include "TMCavity/fortrend_tm_cavity_subsystem.h"
#include "Pump/fortrend_pump_subsystem.h"

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

namespace FC{



	/**
	* LoadLockOpenAngleValveCommand
	*/
	LoadLockOpenAngleValveCommand::LoadLockOpenAngleValveCommand(KeyencePlcSubSystemHelper* hexHelper)
		:KeyencePlcCommandExecuter(hexHelper){
		//setMessageName("OpenAngleValve");
		//setDescription("open angle valve the laodlock");
	};
	std::vector<IKernelResources* > LoadLockOpenAngleValveCommand::resources() const{
		return std::vector<IKernelResources* >();
	}
	/**
	* return true if success else false.
	*/
	LoadLockOpenAngleValveCommand::RunResult LoadLockOpenAngleValveCommand::onRun() throw(KernelException){
		FortrendLoadLockSubsystem* sub = dynamic_cast<FortrendLoadLockSubsystem*>(getSubsystem());
		//
		if (!sub){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "子系统类型错误", this);
		}
		//check modules		

		std::shared_ptr<FortrendTMCavitySubsystem> tm = getSubsystem()->getKernel()->getKernelModule<FortrendTMCavitySubsystem>("TM");
		if (!tm)
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "子系统类型错误", this);
		}
		//std::shared_ptr<FortrendPumpSubsystem> pump = sub->getKernel()->getKernelModule<FortrendPumpSubsystem>("PUMP");
		FortrendPumpSubsystem * pump = dynamic_cast<FortrendPumpSubsystem*>((sub->getKernel()->getKernelModule<FortrendPumpSubsystem>("PUMP")).get());


		//if (sub->getHeightVacuumBaffleValveOpend())
		//{
		//	throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, Poco::format("%s高真空挡板阀未关闭(逻辑错误)", sub->getName()), this);
		//}

		if (SIMULATION_TEST == 1)
		{
			logInform(sub->getName().c_str(), "模拟执行打开角阀命令...");
			return RunResult::RUN_OK;
		}

		// TODO:2025-5-26 如果只开角阀，那条件变化是什么？
		if (sub->getName() == "LLA"){
			if (tm->getMoleculePipelineVacuumValue() > 30 && pump->getMolecularPumpOpenedLLA()){
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_STATION_CONFLICT_EXCEPTION, Poco::format("工位: %s 分子泵阀已打开，%s 前级管道当前压力未小于30Pa（逻辑错误）", sub->getName(), sub->getName()), this);
			}
		}
		else{
			if (tm->getMoleculePipelineVacuumValue() > 30 && pump->getMolecularPumpOpenedLLB()){
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_STATION_CONFLICT_EXCEPTION, Poco::format("工位: %s 分子泵阀已打开，%s 前级管道当前压力未小于30Pa（逻辑错误）", sub->getName(), sub->getName()), this);
			}
		}


		//get command configure
		std::shared_ptr<KernelConfiguration> command_config = sub->getConfigure()->createView(getName());

		//fill params
		std::string address_1 = command_config->getString("address_1", "");
		std::string address_2 = command_config->getString("address_2", "");
		std::string finish_address = command_config->getString("finish_address", "");
		int timeout = command_config->getInt("timeout", -1);
		if (timeout < 10){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_DATA_OUTOF_RANGE, Poco::format("超时: %s 打开角阀命令设置超时参数错误", sub->getName()), this);
		}

		if ((address_1 == "") || (address_2 == "") || (finish_address == ""))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, Poco::format("地址: 打开角阀命令地址未定义", getName()), this);
		}
		logInform(sub->getName().c_str(), "打开角阀命令开始");
		if (!writeBit(address_1, true))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 写 1 到打开角阀命令地址1错误", sub->getName()), this);
		}
		if (!writeBit(address_2, true))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 写 1 到打开角阀命令地址2错误", sub->getName()), this);
		}
		Sleep(500);
		int loopCount = timeout / 20;
		int count = 0;
		bool readRes = false;
		bool readState = false;
		while (count <= loopCount)
		{
			Sleep(20);
			readState = readBit(finish_address, readRes);

			if (readRes)
			{
				break;
			}
			count++;
		}
		IKernelCommand::RunResult ret = IKernelCommand::RunResult::RUN_FAILD;
		if (readRes)
		{
			sub->setAngleValveOpend(true);
			ret = IKernelCommand::RunResult::RUN_OK;
			logInform(sub->getName().c_str(), "打开角阀命令执行结束");

		}
		else if (readState)
		{

			AlarmMessage::Ptr alarm(new AlarmMessage(1, 1, "打开角阀命令执行失败，打开角阀到位信号异常"));
			setAlarm(alarm);
			logError(sub->getName().c_str(), "打开角阀命令执行失败，打开角阀到位信号异常");
		}
		else
		{
			AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_COMMUNICATION_TIMEOUT, "打开角阀命令通讯超时"));
			setAlarm(alarm);
			logError(sub->getName().c_str(), "打开角阀命令通讯超时");
		}
		return ret;

	}



}