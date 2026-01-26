// Library: Fortrend
// Package: CommandImp/Hex/LoadPort
//
// closebox command for fortrend loadport
//
// author xielonghua
//


#include "EFEM/efem_loadport_closebox_command.h"
#include "EFEM/efem_loadport_subsystem.h"
#include "Kernel/Fortrend/fortrend_cassette_manager.h"
#include "Kernel/kernel.h"
#include "Kernel/kernel_command_reject_exception.h"
#include "Kernel/kernel_block.h"
#include "Kernel/kernel_block_manager.h"
#include "Poco/Format.h"

KERNEL_NS_BEGIN

class EFEMLoadPortCloseBoxCommandPrivate{
public:
	std::string name;
};


EFEMLoadPortCloseBoxCommand::EFEMLoadPortCloseBoxCommand(HexSubSystemHelper* hexHelper)
:HexCommandExecuter(hexHelper)
, d(new EFEMLoadPortCloseBoxCommandPrivate){
	d->name = "CloseBox";
	setMessageName(d->name); 
}


std::string EFEMLoadPortCloseBoxCommand::getName() const { 
	return d->name; 
};

/**
* return true if success else false.
*/
EFEMLoadPortCloseBoxCommand::RunResult EFEMLoadPortCloseBoxCommand::onRun() throw(KernelException){
	EFEMLPSubsystem* lp = dynamic_cast<EFEMLPSubsystem*>(getSubsystem());
	if (!lp) throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, "Subsystem error", this);

	for (auto robot : lp->getRobots()){
		if (robot->getState() != IKernelSubSystem::State::SUB_NORMAL){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_STATE_EXCEPTION, Poco::format("Robot: %s is not normal now.", robot->getName()), this);
		}
	}
	std::shared_ptr<KernelConfiguration> command_config = lp->getConfigure()->createView(getName());
	//if (SIM_MODE == 1)
	//{
	//	logInform("lp:", "EFEMLoadPortCloseBoxCommand 模拟测试.");
	//	return RunResult::RUN_OK;
	//}

	//fill params
	int macroId = command_config->getInt("macroid", -1);
	int timeout = command_config->getInt("macrotimeout", -1);
	if (timeout > 0){
		lp->timeout = timeout;
	}
	lp->timestamp = std::chrono::system_clock::now();//多线程下，有概率跳过判断，造成秒超时报警，待优化
	lp->primaryMessageName = this->getName();
	//std::string str = Poco::format("ACK:%s", );
	int lpn = lp->getName() == "ELP1" ? 1 : 2;
	std::string str = Poco::format("MOV:CLOSE/LP%d", lpn);
	str.push_back(';');
	bool result = lp->api->sendMessage(str.data(), str.size());
	RunResult ret = RunResult::RUN_OK;
	if (!result){
		AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_STATE_EXCEPTION, Poco::format("%s Open command failed to send, please check the communication!", lp->getName())));
		setAlarm(alarm);
		ret = RunResult::RUN_FAILD;
		logError(lp->getName().c_str(), "%s Open command failed to send, please check the communication!", lp->getName());
		return ret;
	}


	lp->setCommandState(EFEMAsciiApi::State::TRANS_WAIT_REPLY);
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
	else if (lp->getCommandState() == EFEMAsciiApi::State::TRANS_FINISHED){
		FortrendCassetteManager::Ptr cassManager = getSubsystem()->getKernel()->getKernelModule<FortrendCassetteManager>();
		auto cass1 = cassManager->getCassette(lp);
		if (cass1)cass1->setBoxOpened(false);
		lp->setDoorOpend(false);
	}
	lp->getKernel()->getKernelBlockManager()->releaseBlock(lp);
	return ret;
}

KERNEL_NS_END
