// Library: Fortrend
// Package: CommandImp/Hex/Aligner
//
// status command for Hex subsystem
//
// author xielonghua
//


#include "EFEM/efem_aligner_getmap_command.h"
#include "EFEM/efem_aligner_subsystem.h"
#include "Kernel/Fortrend/fortrend_cassette_manager.h"
#include "Kernel/kernel.h"
#include "Kernel/kernel_command_reject_exception.h"
#include "Kernel/kernel_block.h"
#include "Kernel/kernel_block_manager.h"
#include <thread>

#include "Poco/Format.h"

KERNEL_NS_BEGIN

#if _MSC_VER >1600
#pragma execution_character_set("utf-8")
#endif

class EFEMAlignerGetMapCommandPrivate{
public: 
	
};


EFEMAlignerGetMapCommand::EFEMAlignerGetMapCommand(HexSubSystemHelper* hexHelper):
	 HexCommandExecuter(hexHelper),
	 d(new EFEMAlignerGetMapCommandPrivate)
{
 
}
 
/**
* return true if success else false.
*/
IKernelCommand::RunResult EFEMAlignerGetMapCommand::onRun() throw(KernelException){
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
			Poco::format("Aliger %s must has logic cassette.", aligner->getName()),this);
	}
	std::shared_ptr<KernelConfiguration> command_config = aligner->getConfigure()->createView(getName());

	//fill params
	int timeout = command_config->getInt("timeout", -1);//20000ms
	if (timeout < 10) {
		throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_DATA_OUTOF_RANGE,
			Poco::format("超时: 获取状态超时参数设置失败", aligner->getName()), this);
	}

	//GET:MAPDT/ALIGNER;
	aligner->primaryMessageName = this->getName();

	std::string stationName = aligner->getName().erase(0,1); //ALIGNER
	RunResult ret = RunResult::RUN_OK;
	std::string str2 = Poco::format("GET:MAPDT/%s", stationName);
	str2.push_back(';');

	bool result2 = aligner->api->sendMessage(str2.data(), str2.size());
	if (!result2) {
		AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_STATE_EXCEPTION, Poco::format("%s mapdt command failed to send, please check the communication!", aligner->getName())));
		setAlarm(alarm);
		ret = RunResult::RUN_FAILD;
		logError(aligner->getName().c_str(), "%s获取MAP命令发送失败，请检查通讯！", aligner->getName());
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
	logInform(aligner->getName().c_str(), Poco::format("获取寻边器有无晶圆状态 %s 命令执行结束", aligner->getName()).c_str());

	return RunResult::RUN_OK;
}

KERNEL_NS_END
