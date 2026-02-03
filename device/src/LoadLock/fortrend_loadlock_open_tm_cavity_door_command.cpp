/**
* @file            fortrend_loadlock_open_tm_cavity_door_command.h
* @brief           open tm cavity door command for loadlock
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

#include "LoadLock/fortrend_loadlock_open_tm_cavity_door_command.h"
#include "LoadLock/fortrend_loadlock_subsystem.h"

#include "TMCavity/fortrend_tm_cavity_subsystem.h"


#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

namespace FC{



	/**
	* LoadLockOpenTMCavityDoorCommand
	*/
	LoadLockOpenTMCavityDoorCommand::LoadLockOpenTMCavityDoorCommand(KeyencePlcSubSystemHelper* hexHelper)
		:KeyencePlcCommandExecuter(hexHelper){
		//setMessageName("OpenTMCavityDoor");
		//setDescription("open tm cavity door the laodlock");
	}
	std::vector<IKernelResources*> LoadLockOpenTMCavityDoorCommand::resources() const
	{
		return std::vector<IKernelResources*>();
	};

	/**
	* return true if success else false.
	*/
	LoadLockOpenTMCavityDoorCommand::RunResult LoadLockOpenTMCavityDoorCommand::onRun() throw(KernelException){
		FortrendLoadLockSubsystem* sub = dynamic_cast<FortrendLoadLockSubsystem*>(getSubsystem());
		//
		if (!sub){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "子系统类型错误", this);
		}

		//if (sub->getCassetteDoorOpend())
		//{
		//	throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_DOOR_EXCEPTION, Poco::format("工位: %s 晶圆盒门阀已打开（逻辑错误）", sub->getName()), this);
		//}
		if (sub->getVacuumEnable())
		{
			
			if (sub->getSlowDiaphragmValveOpend())
			{
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_STATION_CONFLICT_EXCEPTION, Poco::format("工位: %s 慢充隔膜阀已打开（逻辑错误）", sub->getName()), this);
			}
			if (sub->getFastDiaphragmValveOpend())
			{
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_STATION_CONFLICT_EXCEPTION, Poco::format("工位: %s 快充隔膜阀已打开（逻辑错误）", sub->getName()), this);
			}
			/*if (sub->getExhaustValveOpend())
			{
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_STATION_CONFLICT_EXCEPTION, Poco::format("工位: %s 排气阀已打开（逻辑错误）", sub->getName()), this);
			}*/
			if (!sub->getVacuumValueUpperLimitReachesTheSetValue())
			{
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_STATE_EXCEPTION, Poco::format("工位: %s 真空值未达到上限设定值（逻辑错误）", sub->getName()), this);
			}

			std::shared_ptr<FortrendTMCavitySubsystem> tm = getSubsystem()->getKernel()->getKernelModule<FortrendTMCavitySubsystem>("TM");
			if (!tm) {
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "TM传输腔子系统类型错误", this);
			}
			if (tm->getState() != IKernelSubSystem::State::SUB_NORMAL)
			{
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_DOOR_EXCEPTION, Poco::format("子系统: %s 不在正常状态.", tm->getName()), this);
			}

			double pressureA = tm->getTMCavityVacuumValue(); //TM 
			double pressureB = sub->getVacuumValue(); //LOADLOCK
			double maxSafeDiff;
			double avgPressure = (pressureA + pressureB) / 2.0;

			if (avgPressure >= 50000) {
				maxSafeDiff = 0.1 * avgPressure; // 高压范围
			}
			else if (avgPressure >= 10000) {
				maxSafeDiff = 0.2 * avgPressure; // 中压范围
			}
			else {
				maxSafeDiff = 10; // 低压范围
			}

			if (fabs(pressureA - pressureB) >= maxSafeDiff) {
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_STATE_EXCEPTION, Poco::format("工位: %s和传输腔压强差大于10（逻辑错误）", tm->getName()), this);
			}
		}

		//get command configure
		std::shared_ptr<KernelConfiguration> command_config = sub->getConfigure()->createView(getName());

		//fill params
		std::string open_address = command_config->getString("open_address", "");
		std::string close_address = command_config->getString("close_address", "");
		std::string finish_address = command_config->getString("finish_address", "");
		std::string failed_address = command_config->getString("failed_address", "");
		int timeout = command_config->getInt("timeout", -1);
		if (timeout < 10){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_DATA_OUTOF_RANGE, Poco::format("超时: %s 打开传输腔门阀参数超时设置错误.", sub->getName()), this);
		}

		if ((open_address == "") || (close_address == "") || (finish_address == "") ||(failed_address == ""))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, Poco::format("地址: 传输腔门阀地址未定义.", getName()), this);
		}
		logInform(sub->getName().c_str(), "打开传输腔门阀命令开始.");
		if (!writeBit(close_address, false))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 写0到关闭传输腔门阀命令地址错误.", sub->getName()), this);
		}
		Sleep(20);
		if (!writeBit(open_address, true))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 写1到打开传输腔门阀命令地址错误.", sub->getName()), this);
		}
		Sleep(500);
		int loopCount = timeout / 20;
		int count = 0;
		bool readRes = false;
		bool failedRes = false;
		bool readState = false;
		bool readFailedState = false;
		while (count <= loopCount)
		{
			Sleep(20);
			readState = readBit(finish_address, readRes);
			readFailedState = readBit(failed_address, failedRes);
			if (readRes || failedRes)
			{
				break;
			}
			count++;
		}
		IKernelCommand::RunResult ret = IKernelCommand::RunResult::RUN_FAILD;
		if (readRes)
		{
			ret = IKernelCommand::RunResult::RUN_OK;
			sub->setTMCavityDoorOpend(true);
			logInform(sub->getName().c_str(), "打开传输腔门阀命令执行完成.");

		}
		else if (readFailedState && failedRes)
		{
			AlarmMessage::Ptr alarm(new AlarmMessage(1, 2, "打开传输腔门阀命令执行失败，打开传输腔门阀到位信号异常"));
			setAlarm(alarm);
			logError(sub->getName().c_str(), "打开传输腔门阀命令执行失败，打开传输腔门阀到位信号异常.");
		}
		else
		{
			AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_COMMUNICATION_TIMEOUT, "打开传输腔门阀命令通讯超时"));
			setAlarm(alarm);
			logError(sub->getName().c_str(), "打开传输腔门阀命令通讯超时.");
		}
		return ret;

	}



}