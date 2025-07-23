// Library: Fortrend
// Package: CommandImp/Hex/LoadPort
//
// return map command for fortrend loadport
//
// author xielonghua
//


#include "EFEM/efem_loadport_getmap_command.h"
#include "EFEM/efem_loadport_subsystem.h"
#include "Kernel/Fortrend/fortrend_cassette_manager.h"
#include "Kernel/kernel.h"
#include "Kernel/kernel_command_reject_exception.h"
#include "Kernel/kernel_block.h"
#include "Kernel/kernel_block_manager.h"
#include "Poco/Format.h"

KERNEL_NS_BEGIN


class HexLoadPorGetMapCommandPrivate{
public:
	std::string name;
};


HexLoadPorGetMapCommand::HexLoadPorGetMapCommand(HexSubSystemHelper* hexHelper)
:HexCommandExecuter(hexHelper)
, d(new HexLoadPorGetMapCommandPrivate){
	d->name = "GetMap";
	setMessageName(d->name);
}

std::string HexLoadPorGetMapCommand::getName() const { 
	return d->name; 
};

/**
* return true if success else false.
*/
HexLoadPorGetMapCommand::RunResult HexLoadPorGetMapCommand::onRun() throw(KernelException){
	EFEMLPSubsystem* lp = dynamic_cast<EFEMLPSubsystem*>(getSubsystem());
	//
	if (!lp){
		throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, "subsystem type error.",this);
	}

	//get command configure
	std::shared_ptr<KernelConfiguration> command_config = lp->getConfigure()->createView(getName());

	//fill params
	int macroId = command_config->getInt("macroid", -1);
	int timeout = command_config->getInt("macrotimeout", -1);

	if (timeout > 0){
		lp->timeout = timeout;
	}

	lp->primaryMessageName = this->getName();
	std::string lpn = lp->getName() == "ELP1" ? "LP1" : "LP2";
	std::string str = Poco::format("GET:MAPDT/%s", lpn);
	str.push_back(';');
	bool result = lp->api->sendMessage(str.data(), str.size());
	RunResult ret = RunResult::RUN_OK;
	if (!result){
		AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_STATE_EXCEPTION, Poco::format("%s MAPDT command failed to send, please check the communication!", lp->getName())));
		setAlarm(alarm);
		ret = RunResult::RUN_FAILD;
		logError(lp->getName().c_str(), "%s MAPDT command failed to send, please check the communication!", lp->getName());
		return ret;
	}

	lp->setCommandState(EFEMAsciiApi::State::TRANS_WAIT_REPLY);
	lp->timestamp = std::chrono::system_clock::now();
	lp->wait();
	if (lp->getCommandState() == EFEMAsciiApi::State::TRANS_RESPONSE_TIMEOUT){
		AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_STATE_EXCEPTION, Poco::format("%s %s command timed out.", lp->getName(), getName())));
		setAlarm(alarm);
		ret = RunResult::RUN_FAILD;
	}
	else if (lp->getCommandState() == EFEMAsciiApi::State::TRANS_REQUEST_FAILD){
		AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_STATE_EXCEPTION, Poco::format("%s %s command failed", lp->getName(), getName())));
		setAlarm(alarm);
		ret = RunResult::RUN_FAILD;
	}
	lp->getKernel()->getKernelBlockManager()->releaseBlock(lp);
	return ret;
}

KERNEL_NS_END
