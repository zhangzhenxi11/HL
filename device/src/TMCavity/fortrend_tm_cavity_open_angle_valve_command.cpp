/**
* @file            fortrend_tm_cavity_open_angle_valve_command.h
* @brief           open angle valve command for tm cavity
* @author		   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/TMCavity

#include "Kernel/kernel.h"
#include "Kernel/kernel_command.h"
#include "Kernel/kernel_abstract_subsystem.h"
#include "kernel/kernel_event_id.h"
#include "kernel/kernel_event_paramters.h"
#include "kernel/kernel_command_reject_exception.h"
#include "Kernel/kernel_log.h"
#include "kernel/Fortrend/fortrend_cassette_manager.h"
#include "Poco/Format.h"

#include "TMCavity/fortrend_tm_cavity_open_angle_valve_command.h"
#include "TMCavity/fortrend_tm_cavity_subsystem.h"

#include "LoadLock/fortrend_loadlock_subsystem.h"
#include "Pump/fortrend_pump_subsystem.h"

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif


namespace FC{

	

	/**
	* TMCavityOpenAngleValveCommand
	*/
	TMCavityOpenAngleValveCommand::TMCavityOpenAngleValveCommand(KeyencePlcSubSystemHelper* hexHelper)
		:KeyencePlcCommandExecuter(hexHelper){
		//setMessageName("OpenAngleValve");
		//setDescription("open angle valve the tm cavity");
	};
	
	/**
	* return true if success else false.
	*/
	TMCavityOpenAngleValveCommand::RunResult TMCavityOpenAngleValveCommand::onRun() throw(KernelException){
		FortrendTMCavitySubsystem* sub = dynamic_cast<FortrendTMCavitySubsystem*>(getSubsystem());
		std::shared_ptr<FortrendPumpSubsystem> pump = getSubsystem()->getKernel()->getKernelModule<FortrendPumpSubsystem>("PUMP");
		std::shared_ptr<FortrendLoadLockSubsystem> lk2 = getSubsystem()->getKernel()->getKernelModule<FortrendLoadLockSubsystem>("LLB");
		std::shared_ptr<FortrendLoadLockSubsystem> lk1 = getSubsystem()->getKernel()->getKernelModule<FortrendLoadLockSubsystem>("LLA");
		//
		if (!sub || !pump || !lk1 || !lk2){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "子系统类型错误", this);
		}
		//1.全部的PM门阀关闭，2.TM腔的隔膜阀关闭，3.LLK-TM侧门阀关闭 4.TM腔盖已关闭 5.确保干泵处于打开状态

		//真空模式
		if (sub->getVacuumEnable())
		{

			for (auto& ll : getSubsystem()->getKernel()->getKernelModules<FortrendLoadLockSubsystem>()) {
				if (ll->getTMCavityDoorOpend())
				{
					throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR,
						Poco::format("子系统: %s TM腔门阀未关闭（逻辑错误）", ll->getName()), this);
				}
			}

			for (int i = 0; i <= 3; i++)
			{
				if (sub->getPMCavityDoorOpend(i))
				{
					throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR,
						Poco::format("子系统: %s PM%d腔门阀未关闭（逻辑错误）", sub->getName(), i), this);
				}
			}
			if (sub->getSlowDiaphragmValveOpend())
			{
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR,
					Poco::format("子系统: %s TM腔慢充隔膜阀未关闭（逻辑错误）", sub->getName()), this);
			}
			if (sub->getFastDiaphragmValveOpend())
			{
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR,
					Poco::format("子系统: %s TM腔快充隔膜阀未关闭（逻辑错误）", sub->getName()), this);
			}

			//if (sub->getHeightVacuumBaffleValveOpend())
			//{
			//	throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, 
			//		Poco::format("%s高真空挡板阀未关闭(逻辑错误)", sub->getName()), this);
			//}

			/*if (lk2->getAngleValveOpend() && pump->getMolecularPumpOpenedLLB() && sub->getTMCavityVacuumValue() > 15.0)
			{
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_STATION_CONFLICT_EXCEPTION,
				Poco::format("工位: %s 角阀和分子泵阀已打开，%s 腔室当前压力未小于15Pa（逻辑错误）", lk2->getName(), sub->getName()), this);
			}

			if (lk1->getAngleValveOpend() && pump->getMolecularPumpOpenedLLA() && sub->getTMCavityVacuumValue() > 15.0)
			{
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_STATION_CONFLICT_EXCEPTION,
				Poco::format("工位: %s 角阀和分子泵阀已打开，%s 腔室当前压力未小于15Pa（逻辑错误）", lk1->getName(), sub->getName()), this);
			}*/

			if (!pump->getMechanicalPumpOpened())
			{
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_STATION_CONFLICT_EXCEPTION,
					Poco::format("前级泵未打开（逻辑错误）", sub->getName()), this);
			}
			if (sub->getMoleculePipelineVacuumValue() > 30) {
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_STATION_CONFLICT_EXCEPTION,
					Poco::format("前级管道当前压力未小于30Pa（逻辑错误）", sub->getName()), this);
			}

		}
		
		
		//get command configure
		std::shared_ptr<KernelConfiguration> command_config = sub->getConfigure()->createView(getName());

		//fill params
		std::string address_1 = command_config->getString("address_1", "");
		std::string address_2 = command_config->getString("address_2", "");
		std::string finish_address = command_config->getString("finish_address", "");
		std::string failed_address = command_config->getString("failed_address","");
		int timeout = command_config->getInt("timeout", -1);
		if (timeout < 10){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_DATA_OUTOF_RANGE,
				Poco::format("超时: 打开角阀命令超时参数设置错误", sub->getName()), this);
		}

		if ((address_1 == "") || (address_2 == "") || (finish_address == "")|| (failed_address == ""))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT,
				Poco::format("地址: 打开角阀命令定义未定义", getName()), this);
		}

		logInform(sub->getName().c_str(), "打开角阀命令执行开始");
		if (!writeBit(address_1, true))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR,
				Poco::format(" %s 写1到打开角阀命令地址1错误", sub->getName()), this);
		}
		if (!writeBit(address_2, true))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, 
				Poco::format(" %s 写1到打开角阀命令地址2错误", sub->getName()), this);
		}

		Sleep(500);
		logInform(sub->getName().c_str(),"打开角阀命令开始执行");
		int loopCount = timeout / 20;
		int count = 0;
		bool readRes = false;
		bool readState = false;
		bool readRes_failed = false;
		bool readState_failed = false;
		while (count <= loopCount)
		{
			Sleep(20);
			readState = readBit(finish_address, readRes);
			readState_failed = readBit(failed_address, readRes_failed);
			if (readRes || readRes_failed)
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
		else if (readRes_failed)
		{
			AlarmMessage::Ptr alarm(new AlarmMessage(1, 1, "打开角阀命令执行失败，打开角阀到位信号异常"));
			setAlarm(alarm);
			logError(sub->getName().c_str(), "打开角阀命令执行失败，打开角阀到位信号异常");
		}
		else
		{
			AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE,
				KernelSysException::KR_MODULE_COMMUNICATION_TIMEOUT, "打开角阀命令通讯超时"));
			setAlarm(alarm);
			logError(sub->getName().c_str(), "打开角阀命令通讯超时");
		}
		return ret;
	}

}