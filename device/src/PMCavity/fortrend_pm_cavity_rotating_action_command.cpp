/**
* @file     fortrend_pm_cavity_rotating_action_command.h
* @brief     rotating action command for PMCavity
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

#include "PMCavity/fortrend_pm_cavity_rotating_action_command.h"
#include "PMCavity/fortrend_pm_cavity_subsystem.h"
#include "Kernel/kernel_event_paramters.h"
#include "fortrend_device_kernel.h"

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

namespace FC{

/**
* PMCavityRotatingActionCommandPrivate
*/
class PMCavityRotatingActionCommandPrivate{
public:
    double _degree; //角度
    //int  _count;    //次数
};

/**
* PMCavityRotatingActionCommand
*/
PMCavityRotatingActionCommand::PMCavityRotatingActionCommand(KeyencePlcSubSystemHelper* helper, double degree)
    :KeyencePlcCommandExecuter(helper)
    , d(new PMCavityRotatingActionCommandPrivate){
    d->_degree = degree;
    //d->_count = count;

};

PMCavityRotatingActionCommand::RunResult PMCavityRotatingActionCommand::onRun() throw(KernelException){
    FortrendPMCavitySubsystem* sub = dynamic_cast<FortrendPMCavitySubsystem*>(getSubsystem());
    KernelCommandParameter parameter(shared_from_this());
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
    std::string failed_address = command_config->getString("failed_address", "");
    std::string relative_position_address = command_config->getString("relative_position_address", "");

    int timeout = command_config->getInt("timeout", -1);
    if (timeout < 10) {
        throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_DATA_OUTOF_RANGE, Poco::format("超时: 旋转轴动作命令超时参数设置错误.", sub->getName()), this);
    }

    if ((start_address == "") || (finish_address == "") || (failed_address == "") ||  (relative_position_address == ""))
    {
        throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, Poco::format("地址: 旋转轴命令地址未定义.", getName()), this);
    }
    logInform(sub->getName().c_str(), "旋转轴动作命令开始执行.");
    sub->sendEvent(NEW_EVENT_ID_WITHNAME(EVENT_COMMAND_RUNNING), &parameter);

    if (!writeFloat(relative_position_address, d->_degree))
    {
        throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 写目标坐标:%f 命令地址错误，地址%s", sub->getName(), d->_degree, relative_position_address), this);
    }
    if (!writeBit(start_address, true))
    {
        throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 定位开始命令地址错误，地址%s", sub->getName(), start_address), this);
    }
    Sleep(100);
    int loopCount = timeout / 20;
    int count = 0;
    bool readRes;
    bool failedRes;
    while (count <= loopCount)
    {
        Sleep(20);
        readBit(finish_address, readRes);
        readBit(failed_address, failedRes);
        if (readRes || failedRes)
        {
            break;
        }
        count++;
    }
    IKernelCommand::RunResult ret = IKernelCommand::RunResult::RUN_FAILD;

    if (readRes)
    {
        sub->setPMCavitySafeSignal(true);

        if (!writeBit(start_address, false))
        {
            throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 置位定位开始命令地址错误，地址%s", sub->getName(), start_address), this);
        }

        ret = IKernelCommand::RunResult::RUN_OK;
        logInform(sub->getName().c_str(), "旋转轴定位开始命令执行完成.");
    }
    else if (!readRes || failedRes)
    {
        AlarmMessage::Ptr alarm(new AlarmMessage(1, 1, "旋转轴定位开始命令执行失败"));
        setAlarm(alarm);
    }
    else
    {
        AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_COMMUNICATION_TIMEOUT, "执行去定位开始命令信号超时"));
        setAlarm(alarm);
    }
    return ret;

}

}
