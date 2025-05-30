// Library: Fortrend
// Package: CommandImp/Rnd/Aligner
//
// align command for rnd aligner subsystem
//
// author xielonghua
//


#include "EFEM/efem_aligner_align_command.h"
#include "EFEM/efem_aligner_subsystem.h"
#include "Kernel/Fortrend/fortrend_cassette_manager.h"
#include "Kernel/kernel.h"
#include <thread>
#include "Kernel/kernel_command_reject_exception.h"
#include "Poco/Format.h"

KERNEL_NS_BEGIN
#if _MSC_VER >1600
#pragma execution_character_set("utf-8")
#endif

class EFEMAlignerAlignCommandPrivate{
public:
	//float target_angle = 0.0f;
};


EFEMAlignerAlignCommand::EFEMAlignerAlignCommand(TcpEfemSubSystemHelper* hexHelper)
:TcpEfemCommandExecuter(hexHelper)
, d(new EFEMAlignerAlignCommandPrivate){
	

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
	// MOV:ALIGNER;
	std::string command = "MOV:";
	command.append(sub->getName()).append(";");
	
	bool result = sendRequest(command);

	RunResult ret = RunResult::RUN_OK;
	if (!result) {
		AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_STATE_EXCEPTION, 
			Poco::format("%s Aligner command failed to send, please check the communication!", sub->getName())));
		setAlarm(alarm);
		ret = RunResult::RUN_FAILD;
		logError(sub->getName().c_str(), "%s Aligner command failed to send, please check the communication!", sub->getName());
		return ret;
	}

	auto startTime = std::chrono::high_resolution_clock::now();
	//auto timeout2 = std::chrono::seconds(30);
	std::string strifInf = "MOV";
	while (true)
	{
		auto currentTime = std::chrono::high_resolution_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(currentTime- startTime);

		strifInf = recvResponse(timeout);

		if (strifInf.find("INF:") != std::string::npos)
		{
			//’“µΩ¡À
			break;
		}
		if (elapsed.count() >= timeout/1000)
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, "—∞±ﬂ√¸¡Ó÷¥––≥¨ ±", this);
		}
	}
	logInform(sub->getName().c_str(), "—∞±ﬂ√¸¡Ó÷¥––Ω· ¯");

	return ret;

}

KERNEL_NS_END
