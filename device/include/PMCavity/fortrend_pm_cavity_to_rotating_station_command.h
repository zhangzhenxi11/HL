/**
* @file     fortrend_pm_cavity_to_rotating_station_command.h
* @brief    to_rotating_station command for PMCavity
* @author   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/PMCavity

#ifndef _XLH_FORTREND_PM_CAVITY_TO_ROTATING_STATION_INCLUDE_
#define _XLH_FORTREND_PM_CAVITY_TO_ROTATING_STATION_INCLUDE_ 
#include "KeyencePLC/keyence_plc_command_executer.h"
#include "KeyencePLC/keyence_plc_subsystem_helper.h"
#include "PMCavity/fortrend_pm_cavity_defined.h"

namespace FC{

/**
* @brief to_rotating_station command for PMCavity
*/
class  PMCavityToRotatingStationCommand : public std::enable_shared_from_this<PMCavityToRotatingStationCommand>, public  KernelSubsystemCommand, public KeyencePlcCommandExecuter
{
public:
    DECLARE_PTR(PMCavityToRotatingStationCommand)
    PMCavityToRotatingStationCommand(KeyencePlcSubSystemHelper* helper);
    virtual std::string getName()const override {return "ToRotatingStation";}
protected:
    virtual RunResult onRun() throw(KernelException);

private:
    DECLARE_PRIVATE(PMCavityToRotatingStationCommand)
};

}
#endif

