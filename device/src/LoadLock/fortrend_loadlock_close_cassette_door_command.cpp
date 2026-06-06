/**
* @file            fortrend_loadlock_opendoor_cassette_door_command.h
* @brief           close cassette door command for loadlock
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


#include "LoadLock/fortrend_loadlock_close_cassette_door_command.h"
#include "LoadLock/fortrend_loadlock_subsystem.h"

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif


namespace FC{

	

	/**
	* LoadLockCloseCassetteDoorCommand
	*/
	LoadLockCloseCassetteDoorCommand::LoadLockCloseCassetteDoorCommand(KeyencePlcSubSystemHelper* hexHelper)
		:KeyencePlcCommandExecuter(hexHelper){
		//setMessageName("CloseCassetteDoor");
		//setDescription("close cassette door the laodlock");
	};

	//2026-6-6 注释
	//std::vector<IKernelResources* > LoadLockCloseCassetteDoorCommand::resources() const{
	//	std::vector<IKernelResources* > ret = KernelSubsystemCommand::resources();
	//	FortrendCassetteManager::Ptr cassManager = getSubsystem()->getKernel()->getKernelModule<FortrendCassetteManager>();
	//	auto smif = cassManager->getStation("EWTR");
	//	if (smif){
	//		ret.push_back(smif);
	//	}
	//	else{
	//		return KernelSubsystemCommand::resources();
	//	}
	//	
	//	return ret;
	//	//return std::vector<IKernelResources* >();
	//}
	
	/**
	* return true if success else false.
	*/
	LoadLockCloseCassetteDoorCommand::RunResult LoadLockCloseCassetteDoorCommand::onRun() throw(KernelException){
		FortrendLoadLockSubsystem* sub = dynamic_cast<FortrendLoadLockSubsystem*>(getSubsystem());
		//
		if (!sub){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "子系统类型错误", this);
		}
		
		//get command configure
		std::shared_ptr<KernelConfiguration> command_config = sub->getConfigure()->createView(getName());

		//fill params
		std::string open_address = command_config->getString("open_address", "");
		std::string close_address = command_config->getString("close_address", "");
		std::string finish_address = command_config->getString("finish_address", "");
		std::string failed_address = command_config->getString("failed_address","");

		//测试强制注释
		if (!sub->getLoadLockCassetteCloseSafeSignal())
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_DATA_OUTOF_RANGE, Poco::format("地址: %s EFEM允许关闭cassette门信号未到位.", sub->getName()), this);
		}

		int timeout = command_config->getInt("timeout", -1);
		if (timeout < 10){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_DATA_OUTOF_RANGE, Poco::format("超时: %s 关闭放晶圆盒门阀超时参数设置错误.", sub->getName()), this);
		}

		if ((open_address == "") || (close_address == "") || (finish_address == ""))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, Poco::format("地址: 关闭放晶圆盒门阀地址未定义.", getName()), this);
		}
		logInform(sub->getName().c_str(), "关闭放晶圆盒门阀命令开始.");


		//if (SIMULATION_TEST == 1)
		//{
		//	logInform(sub->getName().c_str(), "模拟关闭放晶圆盒门阀命令...");
		//	return RunResult::RUN_OK;
		//}

		if (!writeBit(open_address, false))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 写0到打开晶圆盒门阀地址错误.", sub->getName()), this);
		}
		Sleep(20);
		if (!writeBit(close_address, true))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 写1到关闭晶圆盒门阀地址错误.", sub->getName()), this);
		}
		//Sleep(500);
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
			sub->setCassetteDoorOpend(false);
			logInform(sub->getName().c_str(), "关闭放晶圆盒门阀命令执行完成.");
			
		}
		else if (readFailedState && failedRes)
		{
			AlarmMessage::Ptr alarm(new AlarmMessage(0, 2, "关闭放晶圆盒门阀执行失败，关闭晶圆盒门阀到位信号异常"));
			setAlarm(alarm);
			//logError(sub->getName().c_str(), "关闭放晶圆盒门阀执行失败.");
		}
		else
		{
			AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_COMMUNICATION_TIMEOUT, "关闭放晶圆盒门阀命令通讯超时"));
			setAlarm(alarm);
			logError(sub->getName().c_str(), "关闭放晶圆盒门阀命令通讯超时.");
		}
		
		return ret;

	}



}