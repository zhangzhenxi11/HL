/**
* @file     fortrend_stationmode_reset_command.h
* @brief    reset command for STATIONMODE
* @author   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/STATIONMODE

#include "STATIONMODE/fortrend_stationmode_reset_command.h"
#include "STATIONMODE/fortrend_stationmode_subsystem.h"
#include "kernel/kernel_command_reject_exception.h"

#include "Poco/Format.h"


namespace FC{

/**
* StationmodeResetCommandPrivate
*/
class StationmodeResetCommandPrivate{
public:
    
};

/**
* StationmodeResetCommand
*/
StationmodeResetCommand::StationmodeResetCommand(HexSubSystemHelper* hexHelper)
    :HexCommandExecuter(hexHelper)
    , d(new StationmodeResetCommandPrivate){
    setMessageName("Reset");
    setDescription("Reset on STATIONMODE");

};



StationmodeResetCommand::RunResult StationmodeResetCommand::onRun() throw(KernelException){
    FortrendSTATIONMODESubsystem* fliper = dynamic_cast<FortrendSTATIONMODESubsystem*>(getSubsystem());
    //
    if (!fliper){
        throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, "subsystem type error.", this);
    }

    //get command configure
    std::shared_ptr<KernelConfiguration> command_config = fliper->getConfigure()->createView(getName());

    //fill params
    int macroId = command_config->getInt("macroid", -1);
    int timeout = command_config->getInt("macrotimeout", -1);
    /*setParams({ (char)macroId });*/
    if (timeout > 0){
        this->setReplyTimeout(timeout);
    }

    if (macroId == -1){
        throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, Poco::format("macro: %s not defined", getName()), this);
    }

    //star hex transaction
    HexCommandExecuter::ExeResult ret = startTransaction();
    //set alarm data
    AlarmMessage::Ptr alarm(new AlarmMessage(ret.errorType, ret.errorCode, ret.errString));
    setAlarm(alarm);

    return ret.result;

}

}
