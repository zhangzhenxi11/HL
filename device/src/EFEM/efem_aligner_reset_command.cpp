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
#include "Kernel/Fortrend/robot_abstract_subsystem.h"
#include <thread>

#include "Poco/Format.h"

#if _MSC_VER >1600
#pragma execution_character_set("utf-8")
#endif

KERNEL_NS_BEGIN


class EFEMAlignerResetCommandPrivate{
public:
};


EFEMAlignerResetCommand::EFEMAlignerResetCommand(TcpEfemSubSystemHelper* hexHelper)
    :TcpEfemCommandExecuter(hexHelper)
, d(new EFEMAlignerResetCommandPrivate){

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
	logInform(sub->getName().c_str(), "复位命令开始执行");

    std::string stationName = sub->getName();
    std::string sendStr = Poco::format("MOV:INIT/%s", stationName);
    sendStr.push_back(';');

    //std::string sendStr = "INIT/ALL;";
    if (!sendRequest(sendStr))
    {
        throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT,
            Poco::format("指令发送失败:", sendStr).c_str(), this);
    }

    int time_mapping = 0;
    std::string strifInf = "MOV";
    while (strifInf.find("INF:INIT") == std::string::npos)
    {
        strifInf = recvResponse(0);
        Sleep(1);
        if (time_mapping >= 120 * 1000)
        {
            throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, "复位命令执行超时", this);
        }
        time_mapping++;
    }

    logInform(sub->getName().c_str(), "复位命令执行结束");

    return IKernelCommand::RunResult::RUN_OK;

}

KERNEL_NS_END
