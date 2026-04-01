/**
* @file            fortrend_loadlock_close_tm_cavity_door_command.h
* @brief           close tm cavity door command for loadlock
* @author		   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/LoadLock

#include <Windows.h>

#include "Poco/Format.h"

#include "Kernel/kernel.h"
#include "Kernel/kernel_command.h"
#include "kernel/kernel_command_reject_exception.h"
#include "Kernel/kernel_log.h"
#include "kernel/Fortrend/fortrend_cassette_manager.h"


#include "LoadLock/fortrend_loadlock_close_tm_cavity_door_command.h"
#include "LoadLock/fortrend_loadlock_subsystem.h"

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

namespace FC{

	

	/**
	* LoadLockCloseTMCavityDoorCommand
	*/
	LoadLockCloseTMCavityDoorCommand::LoadLockCloseTMCavityDoorCommand(KeyencePlcSubSystemHelper* hexHelper)
		:KeyencePlcCommandExecuter(hexHelper){
		//setMessageName("CloseTMCavityDoor");
		//setDescription("close tm cavity door the laodlock");
	}

	//底层WTR已经锁住了目标工位，所以这里不需要重写resources，2026-2-2
	// 若重写就是互锁，wtr那边会解锁，可是LL这里是加锁，但没去解锁，releaseResource().导致持有WTR资源.
 
	//std::vector<IKernelResources*> LoadLockCloseTMCavityDoorCommand::resources() const
	//{
		//std::vector<IKernelResources* > ret = KernelSubsystemCommand::resources();
		//FortrendCassetteManager::Ptr cassManager = getSubsystem()->getKernel()->getKernelModule<FortrendCassetteManager>();
		//auto wtr = cassManager->getStation("WTR");
		//if (wtr) {
		//	ret.push_back(wtr);
		//}
		//else
		//{
		//	return std::vector<IKernelResources*>();
		//}
		//return ret;
	//	return std::vector<IKernelResources*>();
	//};

	
	/**
	* return true if success else false.
	*/
	LoadLockCloseTMCavityDoorCommand::RunResult LoadLockCloseTMCavityDoorCommand::onRun() throw(KernelException){
		FortrendLoadLockSubsystem* sub = dynamic_cast<FortrendLoadLockSubsystem*>(getSubsystem());
		//
		if (!sub){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "子类型错误", this);
		}
		//get command configure
		std::shared_ptr<KernelConfiguration> command_config = sub->getConfigure()->createView(getName());

		//fill params
		std::string open_address = command_config->getString("open_address", "");
		std::string close_address = command_config->getString("close_address", "");
		std::string finish_address = command_config->getString("finish_address", "");
		std::string failed_address = command_config->getString("failed_address", "");
		std::string safe_Interlock_signal_address = command_config->getString("safe_Interlock_signal_address","");

		int timeout = command_config->getInt("timeout", -1);
		if (timeout < 10){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_DATA_OUTOF_RANGE, Poco::format("超时: 关闭传输腔门阀超时参数设置失败.", sub->getName()), this);
		}

		if ((open_address == "") || (close_address == "") || (finish_address == "")||(failed_address == "")||(safe_Interlock_signal_address == ""))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, Poco::format("地址:关闭传输腔门阀地址未定义.", getName()), this);
		}

		bool is_safe = false;
		if (!readBit(safe_Interlock_signal_address, is_safe))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 读LLA-机械手抓放料完成信号命令地址错误.", sub->getName()), this);
		}
		if (!is_safe)
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s LLA-机械手抓放料完成信号未到位.", sub->getName()), this);
		}

		logInform(sub->getName().c_str(), "关闭传输腔门阀命令开始.");
		if (!writeBit(open_address, false))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 写0到打开传输腔门阀命令地址错误.", sub->getName()), this);
		}
		Sleep(20);



		if (!writeBit(close_address, true))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 写1到关闭传输腔门阀命令地址错误.", sub->getName()), this);
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
			sub->setTMCavityDoorOpend(false);
			logInform(sub->getName().c_str(), "关闭传输门阀命令执行完成.");
		}
		else if (readFailedState && failedRes)
		{
			AlarmMessage::Ptr alarm(new AlarmMessage(1, 3, "关闭传输腔门阀命令执行失败，关闭传输腔门阀到位信号异常"));
			setAlarm(alarm);
			logError(sub->getName().c_str(), "关闭传输腔门阀命令执行失败，关闭传输腔门阀到位信号异常.");
		}
		else
		{
			AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_COMMUNICATION_TIMEOUT, "关闭传输腔门阀命令通讯超时"));
			setAlarm(alarm);
			logError(sub->getName().c_str(), "关闭传输腔门阀命令通讯超时.");
		}
		return ret;

	}



}