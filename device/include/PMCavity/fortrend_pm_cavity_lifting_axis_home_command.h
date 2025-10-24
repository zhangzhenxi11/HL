/**
* @file     fortrend_pm_cavity_lifting_axis_home_command.h
* @brief    lifting axis home command for PMCavity
* @author   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/PMCavity

#ifndef   FORTREND_PM_CAVITY_LIFTING_AXIS_HOME_COMMAND_INCLUDE_
#define   FORTREND_PM_CAVITY_LIFTING_AXIS_HOME_COMMAND_INCLUDE_ 
#include "KeyencePLC/keyence_plc_command_executer.h"
#include "KeyencePLC/keyence_plc_subsystem_helper.h"
#include "PMCavity/fortrend_pm_cavity_defined.h"

namespace FC{

/**
* @brief lifting action command for PMCavity
*/
class  PMCavityLiftingAxisHomeCommand :public  KernelSubsystemCommand, public KeyencePlcCommandExecuter
{
public:
    DECLARE_PTR(PMCavityLiftingAxisHomeCommand)
    PMCavityLiftingAxisHomeCommand(KeyencePlcSubSystemHelper* helper);
    virtual std::string getName()const override {return "LiftingAxisHome";}
protected:
    virtual RunResult onRun() throw(KernelException);

private:
    DECLARE_PRIVATE(PMCavityLiftingAxisHomeCommand)
};

}
#endif

