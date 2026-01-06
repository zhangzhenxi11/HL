/**
* @file            fortrend_loadlock_close_angle_valve_command.h
* @brief           close angle valve command for loadlock
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


#include "LoadLock/fortrend_loadlock_close_angle_valve_command.h"
#include "LoadLock/fortrend_loadlock_subsystem.h"


#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif


namespace FC{

	

	/**
	* LoadLockCloseAngleValveCommand
	*/
	LoadLockCloseAngleValveCommand::LoadLockCloseAngleValveCommand(KeyencePlcSubSystemHelper* hexHelper)
		:KeyencePlcCommandExecuter(hexHelper){
		//setMessageName("CloseAngleValve");
		//setDescription("close angle valve the laodlock");
	};

	std::vector<IKernelResources* > LoadLockCloseAngleValveCommand::resources() const{
		return std::vector<IKernelResources* >();
	}
	
	/**
	* return true if success else false.
	*/
	LoadLockCloseAngleValveCommand::RunResult LoadLockCloseAngleValveCommand::onRun() throw(KernelException){
		FortrendLoadLockSubsystem* sub = dynamic_cast<FortrendLoadLockSubsystem*>(getSubsystem());
		//
		if (!sub){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "子系统类型错误", this);
		}
		//get command configure
		std::shared_ptr<KernelConfiguration> command_config = sub->getConfigure()->createView(getName());

		//fill params
		std::string address_1 = command_config->getString("address_1", "");
		std::string address_2 = command_config->getString("address_2", "");
		std::string finish_address = command_config->getString("finish_address", "");
		int timeout = command_config->getInt("timeout", -1);
		if (timeout < 10){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_DATA_OUTOF_RANGE, Poco::format("超时: %s 关闭角阀设置超时参数错误", sub->getName()), this);
		}

		if ((address_1 == "") || (address_2 == "") || (finish_address == ""))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, Poco::format("地址: 关闭角阀地址未定义", getName()), this);
		}

		//if (SIMULATION_TEST == 1)
		//{
		//	Sleep(200);
		//	logInform(sub->getName().c_str(), "模拟关闭角阀命令执行完成...");
		//	return RunResult::RUN_OK;
		//}

		logInform(sub->getName().c_str(), "关闭角阀命令开始执行");

		if (!writeBit(address_1, true))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 写0到打开角阀地址1错误", sub->getName()), this);
		}
		if (!writeBit(address_2, true))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 写0到关闭角阀地址2错误", sub->getName()), this);
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
			sub->setAngleValveOpend(false);
			ret = IKernelCommand::RunResult::RUN_OK;
			logInform(sub->getName().c_str(), "关闭角阀命令执行完成");
			
		}
		else if (readState)
		{
			AlarmMessage::Ptr alarm(new AlarmMessage(1, 2, "关闭角阀命令执行失败，关闭角阀到位信号异常"));
			setAlarm(alarm);
			//logError(sub->getName().c_str(), "关闭角阀命令执行失败，关闭角阀到位信号异常");
		}
		else
		{
			AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_COMMUNICATION_TIMEOUT, "关闭角阀命令通讯超时"));
			setAlarm(alarm);
			//logError(sub->getName().c_str(), "关闭角阀命令通讯超时");
		}
		return ret;
	}



}