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
	int _model;   //模式
    //int  _count;    //次数
};

/**
* PMCavityRotatingActionCommand
*/
PMCavityRotatingActionCommand::PMCavityRotatingActionCommand(KeyencePlcSubSystemHelper* helper, double degree, int model)
    :KeyencePlcCommandExecuter(helper)
    , d(new PMCavityRotatingActionCommandPrivate){
    d->_degree = degree;
	d->_model = model;
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
    std::string start_address;
	logInform(sub->getName().c_str(), "旋转轴动作命令模式:%d.", d->_model);

    if (d->_model == 1)
    {
        //相对移动模式
        start_address = command_config->getString("start_address", "");
    }
    else 
    {
		//绝对移动模式
        start_address = command_config->getString("start_address_2", "");
    }

    std::string finish_address = command_config->getString("finish_address", "");
    std::string failed_address = command_config->getString("failed_address", "");
    std::string relative_position_address = command_config->getString("relative_position_address", "");
    std::string warning_address = command_config->getString("warning_address", "");
    int timeout = command_config->getInt("timeout", -1);
    if (timeout < 10) {
        throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_DATA_OUTOF_RANGE, Poco::format("超时: 旋转轴动作命令超时参数设置错误.", sub->getName()), this);
    }
    //logInform(sub->getName().c_str(), "start_address:%s", start_address.c_str());
    //logInform(sub->getName().c_str(), "finish_address:%s", finish_address.c_str());
    //logInform(sub->getName().c_str(), "failed_address:%s", failed_address.c_str());
    //logInform(sub->getName().c_str(), "relative_position_address:%s", relative_position_address.c_str());
    //logInform(sub->getName().c_str(), "warning_address:%s", warning_address.c_str());
    if ((start_address == "") || (finish_address == "") || (failed_address == "") ||  (relative_position_address == "") ||(warning_address ==""))
    {
        throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, Poco::format("地址: 旋转轴命令地址未定义.", getName()), this);
    }
    //2026-4-21 R轴只能Z轴在旋转面才可以动作
    if (!sub->getRotatingimumPlaneLevelSignal())
    {
        throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_ABORT, Poco::format("Z轴不在旋转位停止旋转.", getName()), this);
    }
    if(d->_model ==1)
        logInform(sub->getName().c_str(), "旋转轴相对位置动作命令开始执行.");
    else
		logInform(sub->getName().c_str(), "旋转轴绝对位置命令开始执行.");

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
    float realRAxleLocation;
    bool warnRes;
    while (count <= loopCount)
    {
        Sleep(20);
        readBit(finish_address, readRes);
        readBit(failed_address, failedRes);
        readBit(warning_address, warnRes);
        realRAxleLocation = sub->getRealPMCavityRAxleLocation();

        if (readRes || failedRes || warnRes)
        {
            logInform(sub->getName().c_str(), "旋转轴实时位置：%.3f", realRAxleLocation);
            break;
        }
        count++;
    }
    IKernelCommand::RunResult ret = IKernelCommand::RunResult::RUN_FAILD;


    if (readRes)
    {
        sub->setPMCavitySafeSignal(true);
        realRAxleLocation = sub->getRealPMCavityRAxleLocation();
        logInform(sub->getName().c_str(), "定位完成，旋转轴实时位置：%.3f,定位位置:%.3f", realRAxleLocation, d->_degree);

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
    else if (warnRes)
    {
        AlarmMessage::Ptr alarm(new AlarmMessage(1, 2, "Z轴不在旋转位,R轴停止"));
        setAlarm(alarm);
    }
    else
    {
        AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_COMMUNICATION_TIMEOUT, "执行去定位开始命令信号超时"));
        setAlarm(alarm);
    }
    logInform(sub->getName().c_str(), "定位失败，旋转轴实时位置：%.3f,定位位置:%.3f", realRAxleLocation, d->_degree);
    return ret;

}

}
