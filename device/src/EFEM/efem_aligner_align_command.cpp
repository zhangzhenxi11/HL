// Library: Fortrend
// Package: CommandImp/Rnd/Aligner
//
// align command for rnd aligner subsystem
//
// author xielonghua
//


#include "EFEM/efem_aligner_align_command.h"
#include "EFEM/efem_aligner_subsystem.h"
#include "Kernel/kernel.h"
#include "Kernel/kernel_block.h"
#include "Kernel/kernel_block_manager.h"
#include "Kernel/kernel_command_reject_exception.h"
#include "Kernel/Fortrend/fortrend_cassette_manager.h"
#include "Poco/Format.h"
#include <thread>
KERNEL_NS_BEGIN
#if _MSC_VER >1600
#pragma execution_character_set("utf-8")
#endif

class EFEMAlignerAlignCommandPrivate{
public:
	
};



EFEMAlignerAlignCommand::EFEMAlignerAlignCommand(HexSubSystemHelper* hexHelper):
	HexCommandExecuter(hexHelper),
	d(new EFEMAlignerAlignCommandPrivate)
{
}

/**
* return true if success else false.
*/
IKernelCommand::RunResult EFEMAlignerAlignCommand::onRun() throw(KernelException){
	std::string res;

	EFEMAlignerSubsystem* sub = dynamic_cast<EFEMAlignerSubsystem*>(getSubsystem());
	if (!sub) throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, "Subsystem error", this);

	for (auto robot : sub->getRobots()) {
		if (robot->getState() != IKernelSubSystem::State::SUB_NORMAL) {
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_STATE_EXCEPTION, 
				Poco::format("Robot: %s is not normal now.", robot->getName()), this);
		}
	}
	auto cassManager = sub->getKernel()->getKernelModule<FortrendCassetteManager>();
	//get cass
	Cassette::Ptr cass = cassManager->getCassette(sub);
	if (!cass) {
		throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, 
			Poco::format("Aliger %s must has logic cassette.", sub->getName()), this);
	}
	std::shared_ptr<KernelConfiguration> command_config = sub->getConfigure()->createView(getName());

	//fill params
	int timeout = command_config->getInt("timeout", -1);//20000ms
	if (timeout < 10) {
		throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_DATA_OUTOF_RANGE, 
			Poco::format("≥¨ ±: —∞±ﬂ≥¨ ±≤Œ ˝…Ë÷√ ß∞‹", sub->getName()), this);
	}

	logInform(sub->getName().c_str(), "—∞±ﬂ√¸¡Óø™ º÷¥––");
	// MOV:ALIGN;
	std::string command = "MOV:";
	//EALIGNER
	std::string str = sub->getName().erase(0,1);
	str.erase(5,3);
	command.append(str).append(";");
	bool result = sub->api->sendMessage(command.data(), command.size());
	RunResult ret = RunResult::RUN_OK;
	if (!result) {
		AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_STATE_EXCEPTION, 
			Poco::format("%s Aligner command failed to send, please check the communication!", sub->getName())));
		setAlarm(alarm);
		ret = RunResult::RUN_FAILD;
		logError(sub->getName().c_str(), "%s Aligner command failed to send, please check the communication!", sub->getName());
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
	else if (sub->getCommandState() == EFEMAsciiApi::State::TRANS_REQUEST_FAILD)
	{

		AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_STATE_EXCEPTION, Poco::format("%s %s command failed", sub->getName(), getName())));
		setAlarm(alarm);
		ret = RunResult::RUN_FAILD;
		
	}
	sub->getKernel()->getKernelBlockManager()->releaseBlock(sub);
	logInform(sub->getName().c_str(), "—∞±ﬂ√¸¡Ó÷¥––Ω· ¯");
	return ret;

}

KERNEL_NS_END
