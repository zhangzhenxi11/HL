/**
* @file     fortrend_pm_cavity_rotating_axis_home_command.h
* @brief    rotating axis home command for PMCavity
* @author   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/PMCavity

#ifndef  FORTREND_PM_CAVITY_ROTATING_AXIS_HOME_COMMAND_INCLUDE_
#define  FORTREND_PM_CAVITY_ROTATING_AXIS_HOME_COMMAND_INCLUDE_ 
#include "KeyencePLC/keyence_plc_command_executer.h"
#include "KeyencePLC/keyence_plc_subsystem_helper.h"
#include "PMCavity/fortrend_pm_cavity_defined.h"

namespace FC{

/**
* @brief lifting action command for PMCavity
*/
class  PMCavityRotatingAxisHomeCommand :  public  KernelSubsystemCommand, public KeyencePlcCommandExecuter
{
public:
    DECLARE_PTR(PMCavityRotatingAxisHomeCommand)
    PMCavityRotatingAxisHomeCommand(KeyencePlcSubSystemHelper* helper);
    virtual std::string getName()const override {return "RotatingAxisHome";}
protected:
    virtual RunResult onRun() throw(KernelException);

private:
    DECLARE_PRIVATE(PMCavityRotatingAxisHomeCommand)
};

}
#endif

