// Library: Fortrend
// Package: CommandImp/Rnd/Aligner
//
// reset command for rnd aligner subsystem
//
// author xielonghua
//

#include "EFEM/efem_aligner_reset_command.h"
#include "EFEM/efem_aligner_subsystem.h"
#include "Kernel/kernel.h"
#include "Kernel/kernel_command_reject_exception.h"
#include "Kernel/kernel_command_runner.h"
#include "Kernel/kernel_subsystem_update_command.h"
#include "Kernel/Fortrend/fortrend_cassette_manager.h"

#include "Kernel/kernel_block.h"
#include "Kernel/kernel_block_manager.h"
#include <thread>
#include "Poco/Format.h"

#if _MSC_VER >1600
#pragma execution_character_set("utf-8")
#endif

KERNEL_NS_BEGIN

EFEMAlignerResetCommand::EFEMAlignerResetCommand(HexSubSystemHelper* hexHelper):HexResetCommand(hexHelper)
{
}


/**
* return true if success else false.
*/
IKernelCommand::RunResult EFEMAlignerResetCommand::onRun() throw(KernelException){
	std::string res;

	EFEMAlignerSubsystem* sub = dynamic_cast<EFEMAlignerSubsystem*>(getSubsystem());
	if (!sub) throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, "Subsystem error", this);

	for (auto robot : sub->getRobots()){
		if (robot->getState() != IKernelSubSystem::State::SUB_NORMAL){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_STATE_EXCEPTION, Poco::format("Robot: %s is not normal now.", robot->getName()), this);
		}
	}
	//get command configure
	std::shared_ptr<KernelConfiguration> command_config = sub->getConfigure()->createView(getName());
	logInform(sub->getName().c_str(), "复位命令开始执行.");

    /*ALIGN参数说明\n注意：参数可为空。若参数为空，
    边缘查找器将在完成边缘搜索后自动调用默认角度。
    */
    std::string stationName = sub->getName();
    stationName.erase(0,1);
    std::string sendStr = Poco::format("MOV:INIT/%s", stationName); //MOV:INIT/ALIGNER;
    sendStr.push_back(';');

    //std::string sendStr = "INIT/ALL;";
    bool result = sub->api->sendMessage(sendStr.data(), sendStr.size());
    RunResult ret = RunResult::RUN_OK;
	if (!result) {
		AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_STATE_EXCEPTION, Poco::format("%s Reset command failed to send, please check the communication!", sub->getName())));
		setAlarm(alarm);
		ret = RunResult::RUN_FAILD;
		logError(sub->getName().c_str(), "%s复位命令发送失败，请检查通讯！", sub->getName());
		return ret;
	}
	sub->setCommandState(EFEMAsciiApi::State::TRANS_WAIT_REPLY);
	sub->timestamp = std::chrono::system_clock::now();
	sub->wait();
	if (sub->getCommandState() == EFEMAsciiApi::State::TRANS_RESPONSE_TIMEOUT) {
		AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_STATE_EXCEPTION, Poco::format("%s %s command timed out.", sub->getName(), getName())));
		setAlarm(alarm);
		ret = RunResult::RUN_FAILD;
	}
	else if (sub->getCommandState() == EFEMAsciiApi::State::TRANS_REQUEST_FAILD) {
		AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_STATE_EXCEPTION, Poco::format("%s %s command failed", sub->getName(), getName())));
		setAlarm(alarm);
		ret = RunResult::RUN_FAILD;
	}
	
	sub->getKernel()->getKernelBlockManager()->releaseBlock(sub);
    logInform(sub->getName().c_str(), "复位命令执行结束.");

    return ret;

}

KERNEL_NS_END
