// Library: Fortrend
// Package: CommandImp/Rnd/Aligner
//
// rotate command for rnd subsystem
//
// author xielonghua
//


#include "EFEM/efem_aligner_rotate_command.h"
#include "EFEM/efem_aligner_subsystem.h"
#include "Kernel/Fortrend/fortrend_cassette_manager.h"
#include "Kernel/kernel.h"
#include "Kernel/kernel_command_reject_exception.h"
#include "Kernel/kernel_block.h"
#include "Kernel/kernel_block_manager.h"
#include "Poco/Format.h"

#include <thread>


KERNEL_NS_BEGIN

#if _MSC_VER >1600
#pragma execution_character_set("utf-8")
#endif
class EFEMAlignerRotateCommandPrivate{
public:
	float target_angle = 0.0f;
};

EFEMAlignerRotateCommand::EFEMAlignerRotateCommand(HexSubSystemHelper* hexHelper, float angle)
	:HexCommandExecuter(hexHelper)
	, d(new EFEMAlignerRotateCommandPrivate)
{
	d->target_angle = angle;
}

/**
* return true if success else false.
*/
IKernelCommand::RunResult EFEMAlignerRotateCommand::onRun() throw(KernelException){
	std::string res;

	auto aligner = dynamic_cast<EFEMAlignerSubsystem*>(getSubsystem());
	if (!aligner){
		throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, 
			"aligner status command must belong to Aligner.",this);
	}
	auto cassManager = aligner->getKernel()->getKernelModule<FortrendCassetteManager>();
	//get cass
	Cassette::Ptr cass = cassManager->getCassette(aligner);
	if (!cass){
		throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, 
			Poco::format("Aliger %s must has logic cassette.", aligner->getName()), this);
	}
	std::shared_ptr<KernelConfiguration> command_config = aligner->getConfigure()->createView(getName());

	//fill params
	int timeout = command_config->getInt("timeout", -1);//20000ms
	if (timeout < 10) {
		throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_DATA_OUTOF_RANGE,
			Poco::format("超时: 寻边超时参数设置失败.", aligner->getName()), this);
	}
	//寻边指令发参数，指定寻边之后旋转的具体角度,参数范围	(A-359.00-A359.99).
	//MOV:ALIGNER/A090.00;
	logInform(aligner->getName().c_str(), "aligner Rotate angle：%lf", d->target_angle);

	std::string command = "MOV:";

	command.append(aligner->getName().erase(0,1)).append("/A");
	command.append(std::to_string(d->target_angle));
	command.push_back(';');
	logInform(aligner->getName().c_str(), "aligner Rotate command：%s", command.c_str());

	bool result = aligner->api->sendMessage(command.data(), command.size());
	RunResult ret = RunResult::RUN_OK;
	if (!result) {
		AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_STATE_EXCEPTION,
		Poco::format("%s Aligner command failed to send, please check the communication!", aligner->getName())));
		setAlarm(alarm);
		ret = RunResult::RUN_FAILD;
		logError(aligner->getName().c_str(), "%s Aligner command failed to send, please check the communication!", aligner->getName());
		return ret;
	}

	aligner->setCommandState(EFEMAsciiApi::State::TRANS_WAIT_REPLY);
	aligner->timestamp = std::chrono::system_clock::now();
	aligner->wait();
	if (aligner->getCommandState() == EFEMAsciiApi::State::TRANS_RESPONSE_TIMEOUT) {
		AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_STATE_EXCEPTION, Poco::format("%s %s command timed out.", aligner->getName(), getName())));
		setAlarm(alarm);
		ret = RunResult::RUN_FAILD;
	}
	else if (aligner->getCommandState() == EFEMAsciiApi::State::TRANS_REQUEST_FAILD) {
		AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_STATE_EXCEPTION, Poco::format("%s %s command failed", aligner->getName(), getName())));
		setAlarm(alarm);
		ret = RunResult::RUN_FAILD;

	}
	aligner->getKernel()->getKernelBlockManager()->releaseBlock(aligner);
	logInform(aligner->getName().c_str(), "寻边命令执行结束.");
	return ret;
}

KERNEL_NS_END
