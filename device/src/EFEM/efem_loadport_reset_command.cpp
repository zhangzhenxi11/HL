// Library: Fortrend
// Package: CommandImp/Hex/LoadPort
//
// reset command for fortrend loadport subsystem
//
// author xielonghua
//

#include "EFEM/efem_loadport_reset_command.h"
#include "EFEM/efem_loadport_subsystem.h"
#include "Kernel/Fortrend/robot_abstract_subsystem.h"
#include "Kernel/kernel_command_reject_exception.h"
#include "Kernel/kernel_command_runner.h"
#include "Kernel/kernel_subsystem_update_command.h"
#include "Kernel/kernel_block.h"
#include "Kernel/kernel_block_manager.h"
#include "Kernel/kernel.h"
#include <Poco/Format.h>

#if _MSC_VER >1600
#pragma execution_character_set("utf-8")
#endif
KERNEL_NS_BEGIN

EFEMLoadPortResetCommand::EFEMLoadPortResetCommand(HexSubSystemHelper* hexHelper)
	:HexResetCommand(hexHelper){

}


EFEMLoadPortResetCommand::RunResult EFEMLoadPortResetCommand::onRun() throw(KernelException){
	EFEMLPSubsystem* sub = dynamic_cast<EFEMLPSubsystem*>(getSubsystem());
	if (!sub) throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, "Subsystem error", this);

	for (auto robot : sub->getRobots()){
		if (robot->getState() != IKernelSubSystem::State::SUB_NORMAL){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_STATE_EXCEPTION, Poco::format("Robot: %s is not normal now.", robot->getName()),this);
		}
	}
	std::shared_ptr<KernelConfiguration> command_config = sub->getConfigure()->createView(getName());

	//fill params
	int macroId = command_config->getInt("macroid", -1);
	int timeout = command_config->getInt("macrotimeout", -1);
	if (timeout > 0){
		sub->timeout = timeout;
	}
	else{
		sub->timeout = 60000;
	}
	
	sub->primaryMessageName = this->getName();
	std::string stationName = sub->getName().erase(0, 1);
	std::string str = Poco::format("MOV:INIT/%s", stationName);
	str.push_back(';');
     bool result= sub->api->sendMessage(str.data(), str.size());
	 RunResult ret = RunResult::RUN_OK;
	 if (!result){
		// AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_STATE_EXCEPTION, Poco::format("%s¸´Î»ÃüÁî·¢ËÍÊ§°Ü£¬Çë¼ì²éÍ¨Ñ¶£¡", sub->getName())));
		 AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_STATE_EXCEPTION, Poco::format("%s Reset command failed to send, please check the communication!", sub->getName())));
		 setAlarm(alarm);
		 ret = RunResult::RUN_FAILD;
		 logError(sub->getName().c_str(), "%s¸´Î»ÃüÁî·¢ËÍÊ§°Ü£¬Çë¼ì²éÍ¨Ñ¶£¡", sub->getName());
		 //throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_STATE_EXCEPTION, Poco::format("%s¸´Î»ÃüÁî·¢ËÍÊ§°Ü£¬Çë¼ì²éÍ¨Ñ¶£¡", sub->getName()), this);
		return ret;
	 }


	 sub->setCommandState(EFEMAsciiApi::State::TRANS_WAIT_REPLY);
	 sub->timestamp=std::chrono::system_clock::now();
	 sub->wait();
	 if (sub->getCommandState() == EFEMAsciiApi::State::TRANS_RESPONSE_TIMEOUT){
		 AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_STATE_EXCEPTION, Poco::format("%s %s command timed out.", sub->getName(), getName())));
		 setAlarm(alarm);
		 ret = RunResult::RUN_FAILD;
	 }
	 else if (sub->getCommandState() == EFEMAsciiApi::State::TRANS_REQUEST_FAILD){
		 AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_STATE_EXCEPTION, Poco::format("%s %s command failed", sub->getName(), getName())));
		 setAlarm(alarm);
		 ret = RunResult::RUN_FAILD;
	 }
	 sub->getKernel()->getKernelBlockManager()->releaseBlock(sub);
	return ret;
}

KERNEL_NS_END
