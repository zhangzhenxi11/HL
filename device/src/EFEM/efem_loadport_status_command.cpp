// Library: Fortrend
// Package: CommandImp/Rnd/Aligner
//
// status command for rnd subsystem
//
// author xielonghua
//


#include "EFEM/efem_loadport_status_command.h"
#include "EFEM/efem_loadport_getmap_command.h"
#include "EFEM/efem_loadport_subsystem.h"
#include "Kernel/Fortrend/fortrend_cassette_manager.h"
#include "Kernel/kernel.h"
#include "Kernel/kernel_command_reject_exception.h"
#include "Kernel/kernel_command_runner.h"
#include <thread>
#include "Kernel/kernel_block.h"
#include "Kernel/kernel_block_manager.h"
#include "Poco/Format.h"

KERNEL_NS_BEGIN


class EFEMLoadPortStatusCommandPrivate{
public: 
	RndSubSystemHelper* rndHelper;
};


EFEMLoadPortStatusCommand::EFEMLoadPortStatusCommand()
:d(new EFEMLoadPortStatusCommandPrivate){
 
}
 

/**
* return true if success else false.
*/
IKernelCommand::RunResult EFEMLoadPortStatusCommand::onRun() throw(KernelException){
	EFEMLPSubsystem* sub = dynamic_cast<EFEMLPSubsystem*>(getSubsystem());
	if (!sub) throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, "Subsystem error", this);

	if (sub->getState() == IKernelSubSystem::State::SUB_UNKNOWN){
		throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_STATE_EXCEPTION, Poco::format("Loadport: %s is not normal now.", sub->getName()), this);
	}
	std::shared_ptr<KernelConfiguration> command_config = sub->getConfigure()->createView(getName());

	//fill params
	int macroId = command_config->getInt("macroid", -1);
	int timeout = command_config->getInt("macrotimeout", -1);
	if (timeout > 0){
		sub->timeout = timeout;
	}
	sub->primaryMessageName = this->getName();
	std::string stationName = sub->getName().erase(0, 1);
	std::string str = Poco::format("GET:STATE/%s", stationName);
	str.push_back(';');
	bool result = sub->api->sendMessage(str.data(), str.size());
	RunResult ret = RunResult::RUN_OK;
	if (!result){
		AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_STATE_EXCEPTION, Poco::format("%s update command failed to send, please check the communication!", sub->getName())));
		setAlarm(alarm);
		ret = RunResult::RUN_FAILD;
		logError(sub->getName().c_str(), "%s获取状态命令发送失败，请检查通讯！", sub->getName());
		return ret;
	}


	sub->setCommandState(EFEMAsciiApi::State::TRANS_WAIT_REPLY);
	sub->timestamp = std::chrono::system_clock::now();
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
	/*auto cmd = sub->createGetMapCommand();
	KernelCommandRunner runner(sub->getKernel(), cmd, sub);
	runner.runCmd();*/

	std::string str2 = Poco::format("GET:MAPDT/%s", stationName);
	str2.push_back(';');
	bool result2 = sub->api->sendMessage(str2.data(), str2.size());
	if (!result2){
		AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_STATE_EXCEPTION, Poco::format("%s mapdt command failed to send, please check the communication!", sub->getName())));
		setAlarm(alarm);
		ret = RunResult::RUN_FAILD;
		logError(sub->getName().c_str(), "%s获取MAP命令发送失败，请检查通讯！", sub->getName());
		return ret;
	}

	sub->getKernel()->getKernelBlockManager()->releaseBlock(sub);
	return ret;
}

KERNEL_NS_END
