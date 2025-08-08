/**
* @file     fortrend_stationmode_reset_command.h
* @brief    reset command for STATIONMODE
* @author   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/STATIONMODE

#ifndef _XLH_FORTREND_STATIONMODE_RESET_INCLUDE_
#define _XLH_FORTREND_STATIONMODE_RESET_INCLUDE_ 
#include "Kernel/Fortrend/hex_command_executer.h"
#include "kernel/kernel_subsystem_command.h"

namespace FC{

/**
* @brief reset command for STATIONMODE
*/
class  StationmodeResetCommand : public  KernelSubsystemCommand, public HexCommandExecuter
{
public:
    DECLARE_PTR(StationmodeResetCommand)
    StationmodeResetCommand(HexSubSystemHelper* hexHelper);
    virtual std::string getName()const override {return "Reset";}
protected:
    virtual RunResult onRun() throw(KernelException);

private:
    DECLARE_PRIVATE(StationmodeResetCommand)
};

}
#endif

