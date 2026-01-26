/**
* @file     fortrend_pm_cavity_lifting_axis_home_command.h
* @brief    lifting axis home command for PMCavity
* @author   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/PMCavity
#include "Poco/Format.h"

#include "Kernel/kernel.h"
#include "Kernel/kernel_command.h"
#include "kernel/kernel_command_reject_exception.h"
#include "Kernel/kernel_log.h"
#include "kernel/Fortrend/fortrend_cassette_manager.h"

#include "PMCavity/fortrend_pm_cavity_lifting_axis_home_command.h"
#include "PMCavity/fortrend_pm_cavity_subsystem.h"
#include "Kernel/kernel_event_paramters.h"
#include "fortrend_device_kernel.h"

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

namespace FC{

/**
* PMCavityLiftingAxisHomeCommandPrivate
*/
class PMCavityLiftingAxisHomeCommandPrivate{
public:
    
};

/**
* PMCavityLiftingAxisHomeCommand
*/
PMCavityLiftingAxisHomeCommand::PMCavityLiftingAxisHomeCommand(KeyencePlcSubSystemHelper* helper)
    :KeyencePlcCommandExecuter(helper)
    , d(new PMCavityLiftingAxisHomeCommandPrivate){
    
};



PMCavityLiftingAxisHomeCommand::RunResult PMCavityLiftingAxisHomeCommand::onRun() throw(KernelException){
    FortrendPMCavitySubsystem* sub = dynamic_cast<FortrendPMCavitySubsystem*>(getSubsystem());
    
    if (!sub){
        throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, "subsystem type error.", this);
    }
    //check modules
    auto cassManager = sub->getKernel()->getKernelModule<FortrendCassetteManager>();
    //get cass
    auto station_cass = cassManager->getCassette(sub);
    if (!station_cass) {
        throw KernelCommandRejectException(__FILE__, KernelSysException::KR_STATION_WITHOUT_CASS_EXCEPTION, Poco::format("工位: %s 晶圆盒为空.", sub->getName()), this);
    }
    std::shared_ptr<KernelConfiguration> command_config = sub->getConfigure()->createView(getName());

    //fill params
    std::string start_address = command_config->getString("start_address", "");
    std::string finish_address = command_config->getString("finish_address", "");
    std::string running_address = command_config->getString("running_address","");
    int timeout = command_config->getInt("timeout", -1);
    if (timeout < 10) {
        throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_DATA_OUTOF_RANGE, Poco::format("超时: 升降轴回原命令超时参数设置错误", sub->getName()), this);
    }

    if ((start_address == "") || (finish_address == "")||(running_address == ""))
    {
        throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, Poco::format("地址: 升降轴回原命令地址未定义", getName()), this);
    }
    logInform(sub->getName().c_str(), "升降轴回原命令开始执行.");

    if (!writeBit(start_address, true))
    {
        throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 回原开始命令地址错误，地址%s", sub->getName(), start_address), this);
    }

    Sleep(100);
    int loopCount = timeout / 20;
    int count = 0;
    bool readRes;
    bool readRes_1;
    while (count <= loopCount)
    {
        Sleep(20);
        readBit(finish_address,readRes);
        readBit(running_address, readRes_1);
        if (readRes)
        {
            break;
        }
        count++;
    }

	IKernelCommand::RunResult ret = IKernelCommand::RunResult::RUN_FAILD;
    if (!writeBit(start_address, false))
    {
        throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 置位定位开始命令地址错误，地址%s", sub->getName(), start_address), this);
    }
    if (readRes)
    {
        ret = IKernelCommand::RunResult::RUN_OK;
        logInform(sub->getName().c_str(), "升降轴回原命令执行完成.");
    }
    else if(!readRes)
    {
        AlarmMessage::Ptr alarm(new AlarmMessage(1, 1, "升降轴回原命令执行失败"));
        setAlarm(alarm);
    }
    else if(readRes_1)
    {
        AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_COMMUNICATION_TIMEOUT, "执行升降轴回原命令信号超时"));
        setAlarm(alarm);
    }

    return ret;

}

}
