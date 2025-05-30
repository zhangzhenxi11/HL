// Library: Fortrend
// Package: CommandImp/Hex/LoadPort
//
// unlockbox command for fortrend loadport
//
// author xielonghua
//


#include "EFEM/efem_loadport_unlockbox_command.h"
#include "EFEM/efem_loadport_subsystem.h"
#include "Kernel/Fortrend/fortrend_cassette_manager.h"
#include "Kernel/kernel.h"
#include "Kernel/kernel_command_reject_exception.h"
#include "Kernel/kernel_block.h"
#include "Kernel/kernel_block_manager.h"
#include "Poco/Format.h"

KERNEL_NS_BEGIN

class EFEMLoadPortUnlockBoxCommandPrivate{
public:
	std::string name;
};


EFEMLoadPortUnlockBoxCommand::EFEMLoadPortUnlockBoxCommand(HexSubSystemHelper* hexHelper)
:HexCommandExecuter(hexHelper)
, d(new EFEMLoadPortUnlockBoxCommandPrivate){
	d->name = "UnLockBox";
	setMessageName(d->name); 
}


std::string EFEMLoadPortUnlockBoxCommand::getName() const { 
	return d->name; 
};

/**
* return true if success else false.
*/
EFEMLoadPortUnlockBoxCommand::RunResult EFEMLoadPortUnlockBoxCommand::onRun() throw(KernelException){
	EFEMLPSubsystem* lp = dynamic_cast<EFEMLPSubsystem*>(getSubsystem());
	//
	if (!lp){
		throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, "subsystem type error.", this);
	}

	std::shared_ptr<KernelConfiguration> command_config = lp->getConfigure()->createView(getName());

	//fill params
	int macroId = command_config->getInt("macroid", -1);
	int timeout = command_config->getInt("macrotimeout", -1);
	if (timeout > 0){
		lp->timeout = timeout;
	}
	lp->primaryMessageName = this->getName();
	//std::string str = Poco::format("ACK:%s", );
	int lpn = lp->getName() == "ELP1" ? 1 : 2;
	std::string str = Poco::format("MOV:UNLOCK/LP%d", lpn);
	str.push_back(';');
	bool result = lp->api->sendMessage(str.data(), str.size());
	RunResult ret = RunResult::RUN_OK;
	if (!result){
		AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_STATE_EXCEPTION, Poco::format("%s UNLOCK command failed to send, please check the communication!", lp->getName())));
		setAlarm(alarm);
		ret = RunResult::RUN_FAILD;
		logError(lp->getName().c_str(), "%s UNLOCK command failed to send, please check the communication!", lp->getName());
		return ret;
	}

	lp->setCommandState(EFEMAsciiApi::State::TRANS_WAIT_REPLY);
	lp->timestamp = std::chrono::system_clock::now();
	lp->wait();
	if (lp->getCommandState() == EFEMAsciiApi::State::TRANS_RESPONSE_TIMEOUT || lp->getCommandState() == EFEMAsciiApi::State::TRANS_REQUEST_FAILD){
		AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_STATE_EXCEPTION, Poco::format("%s UNLOCK command failed or timed out.", lp->getName())));
		setAlarm(alarm);
		logError(lp->getName().c_str(), "%s UNLOCK command failed or timed out.", lp->getName());
		ret = RunResult::RUN_FAILD;
	}
	lp->getKernel()->getKernelBlockManager()->releaseBlock(lp);
	return ret;
}

KERNEL_NS_END
