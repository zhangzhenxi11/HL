// Library: Fortrend
// Package: CommandImp/Rnd/Aligner
//
// status command for rnd subsystem
//
// author xielonghua
//


#include "EFEM/efem_robot_status_command.h"
#include "EFEM/efem_wafer_robot_subsystem.h"
#include "Kernel/Fortrend/fortrend_cassette_manager.h"
#include "Kernel/kernel.h"
#include "Kernel/kernel_command_reject_exception.h"
#include <thread>
#include "Kernel/kernel_block.h"
#include "Kernel/kernel_block_manager.h"
#include "Poco/Format.h"

KERNEL_NS_BEGIN


class EFEMRobotStatusCommandPrivate{
};


EFEMRobotStatusCommand::EFEMRobotStatusCommand()
: d(new EFEMRobotStatusCommandPrivate){
 
}
 

/**
* return true if success else false.
*/
IKernelCommand::RunResult EFEMRobotStatusCommand::onRun() throw(KernelException){
	EFEMWaferRobotSubsystem* ewtr = dynamic_cast<EFEMWaferRobotSubsystem*>(getSubsystem());
	if (!ewtr) throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, "Subsystem error", this);

	for (auto robot : ewtr->getRobots()){
		if (robot->getState() == IKernelSubSystem::State::SUB_UNKNOWN){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_STATE_EXCEPTION, Poco::format("Robot: %s is not normal now.", robot->getName()), this);
		}
	}
	std::shared_ptr<KernelConfiguration> command_config = ewtr->getConfigure()->createView(getName());

	//fill params
	int macroId = command_config->getInt("macroid", -1);
	int timeout = command_config->getInt("macrotimeout", -1);
	if (timeout > 0){
		ewtr->timeout = timeout;
	}
	ewtr->primaryMessageName = this->getName();
	std::string stationName = ewtr->getName().erase(0, 1);
	std::string str = Poco::format("GET:STATE/%s", stationName);
	str.push_back(';');
	bool result = ewtr->api->sendMessage(str.data(), str.size());
	RunResult ret = RunResult::RUN_OK;
	if (!result){
		AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_STATE_EXCEPTION, Poco::format("%s update command failed to send, please check the communication!", ewtr->getName())));
		setAlarm(alarm);
		ret = RunResult::RUN_FAILD;
		logError(ewtr->getName().c_str(), "%s»ñÈ¡×´Ì¬ÃüÁî·¢ËÍÊ§°Ü£¬Çë¼ì²éÍ¨Ñ¶£¡", ewtr->getName());
		return ret;
	}


	ewtr->setCommandState(EFEMAsciiApi::State::TRANS_WAIT_REPLY);
	ewtr->timestamp = std::chrono::system_clock::now();
	ewtr->wait();
	if (ewtr->getCommandState() == EFEMAsciiApi::State::TRANS_RESPONSE_TIMEOUT){
		AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_STATE_EXCEPTION, Poco::format("%s %s command timed out.", ewtr->getName(), getName())));
		setAlarm(alarm);
		ret = RunResult::RUN_FAILD;
	}
	else if (ewtr->getCommandState() == EFEMAsciiApi::State::TRANS_REQUEST_FAILD){
		AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_STATE_EXCEPTION, Poco::format("%s %s command failed", ewtr->getName(),getName())));
		setAlarm(alarm);
		ret = RunResult::RUN_FAILD;
	}

	std::string str2 = Poco::format("GET:MAPDT/%s", stationName);
	str2.push_back(';');
	bool result2 = ewtr->api->sendMessage(str2.data(), str2.size());

	if (!result2){
		AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_STATE_EXCEPTION, Poco::format("%s mapdt command failed to send, please check the communication!", ewtr->getName())));
		setAlarm(alarm);
		ret = RunResult::RUN_FAILD;
		logError(ewtr->getName().c_str(), "%sMAPDTÃüÁî·¢ËÍÊ§°Ü£¬Çë¼ì²éÍ¨Ñ¶£¡", ewtr->getName());
		return ret;
	}
	ewtr->getKernel()->getKernelBlockManager()->releaseBlock(ewtr);

	return ret;
	
}

KERNEL_NS_END
