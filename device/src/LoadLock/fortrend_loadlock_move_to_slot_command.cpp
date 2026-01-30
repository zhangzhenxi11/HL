/**
* @file            fortrend_loadlock_move_to_slot_command.h
* @brief           move to slot command for loadlock
* @author		   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/LoadLock

#include "Poco/Format.h"

#include "Kernel/kernel.h"
#include "Kernel/kernel_command.h"
#include "kernel/kernel_command_reject_exception.h"
#include "Kernel/kernel_log.h"
#include "kernel/Fortrend/fortrend_cassette_manager.h"

#include "LoadLock/fortrend_loadlock_defined.h"
#include "LoadLock/fortrend_loadlock_move_to_slot_command.h"
#include "LoadLock/fortrend_loadlock_subsystem.h"
#include "EFEM/efem_wafer_robot_subsystem.h"

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif


namespace FC{


	/**
	* LoadLockMoveToSlotCommandPrivate
	*/
	class LoadLockMoveToSlotCommandPrivate{
	public:
		int slot = 0;
	};

	

	/**
	* LoadLockMoveToSlotCommand
	*/
	LoadLockMoveToSlotCommand::LoadLockMoveToSlotCommand(const int slot, KeyencePlcSubSystemHelper* hexHelper)
		:KeyencePlcCommandExecuter(hexHelper)
		, d(new LoadLockMoveToSlotCommandPrivate){
		d->slot = slot;
	};
	/**
	* return true if success else false.
	*/
	LoadLockMoveToSlotCommand::RunResult LoadLockMoveToSlotCommand::onRun() throw(KernelException){
		FortrendLoadLockSubsystem* sub = dynamic_cast<FortrendLoadLockSubsystem*>(getSubsystem());
		//
		if (!sub){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "子系统类型错误.", this);
		}
		/*auto ewtr=sub->getKernel()->getKernelModule<EFEMWaferRobotSubsystem>("EWTR");

		if (!ewtr){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "EWTR系统类型错误.", this);
		}
		if (ewtr->getState() != IKernelSubSystem::State::SUB_NORMAL){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "EWTR系统类型错误.", this);
		}*/
		/*if (!sub->hasBoxPresent())
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_STATION_WITHOUT_CASS_EXCEPTION, Poco::format("工位： %s 没有晶圆盒.", sub->getName()), this);
		}*/
		/*if (sub->getCassetteDoorOpend())
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_DOOR_EXCEPTION, Poco::format("工位： %s放晶圆盒的门已打开.", sub->getName()), this);
		}*/
		/*if (sub->getTMCavityDoorOpend())
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_DOOR_EXCEPTION, Poco::format("工位： %s传输腔门阀已打开.", sub->getName()), this);
		}*/
		/*if (!sub->getProtrudingSensorState())
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_STATION_CONFLICT_EXCEPTION, Poco::format("工位： %s 检测到凸片.", sub->getName()), this);
		}*/
		//get command configure
		std::shared_ptr<KernelConfiguration> command_config = sub->getConfigure()->createView(getName());
		//fill params
		std::string start_address = command_config->getString("start_address", "");
		std::string finish_address = command_config->getString("finish_address", "");
		std::string failed_address = command_config->getString("failed_address", "");
		std::string error_code_address = command_config->getString("error_code_address", "");
		std::string slot_address = command_config->getString("slot_address", "");
		int timeout = command_config->getInt("timeout", -1);
		if (timeout < 10){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_DATA_OUTOF_RANGE, Poco::format("超时： %s 移动到指定槽号超时时间设置错误.", sub->getName()), this);
		}

		if ((start_address == "") || (finish_address == "") || (failed_address == "") || (error_code_address == "") || (slot_address == ""))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, Poco::format("地址：移动到指定槽号地址未定义.", getName()), this);
		}
		/*if (d->slot < 1 || (d->slot >25 && d->slot != 28))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_PARAMER_ERROR, Poco::format(" 参数： 移动到指定槽号(%d)参数超出范围(1~25 and 28 )", d->slot), this);
		}*/
		sub->setBoxPlacement(false);
		logInform(sub->getName().c_str(), Poco::format("移动到第%d槽号命令开始.", d->slot).c_str());
		short s_slot = d->slot;
		if (!writeShort(slot_address, s_slot))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 写 %d 到槽号地址错误.", sub->getName(), s_slot), this);
		}
		if (!writeBit(start_address, true))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 写 1 到移动到指定槽号命令地址错误.", sub->getName()), this);
		}
		sub->setLastMoveSlot(0);
		Sleep(500);
		int loopCount = timeout / 20;
		int count = 0;
		bool readRes[2];
		while (count <= loopCount)
		{
			Sleep(20);
			readBits(finish_address, 2, readRes);
			if (readRes[0] || readRes[1])
			{
				break;
			}
			count++;
		}
		if (!writeBit(start_address, false))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 写0到移动到指定槽号命令地址错误.", getName()), this);
		}
		IKernelCommand::RunResult ret = IKernelCommand::RunResult::RUN_FAILD;
		if (readRes[0])
		{
			sub->setBoxPlacement(true);
			sub->setLastMoveSlot(d->slot);
			ret = IKernelCommand::RunResult::RUN_OK;
			logInform(sub->getName().c_str(), Poco::format("移动到第%d槽号命令执行结束.", d->slot).c_str());

		}
		else if (readRes[1])
		{
			sub->setBoxPlacement(false);
			short code = 0;
			readShort(error_code_address, code);
			auto code_message = getErrorCode(LoadLockErrorCommand::MoveToSlot, code);
			AlarmMessage::Ptr alarm(new AlarmMessage(code_message->type, code_message->code, code_message->message));
			setAlarm(alarm);
			logError(sub->getName().c_str(), Poco::format("移动到第%d槽号命令执行失败.", d->slot).c_str());
		}
		else
		{
			sub->setBoxPlacement(false);
			AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_COMMUNICATION_TIMEOUT, "移动到指定槽号命令通讯超时"));
			setAlarm(alarm);
			logError(sub->getName().c_str(), "移动到指定槽号命令通讯超时.");
		}
		return ret;

	}


	int LoadLockMoveToSlotCommand::getSlot(){
		return d->slot;
	}

}