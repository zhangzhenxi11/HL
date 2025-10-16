/**
* @file     fortrend_pm_cavity_rotating_action_command.h
* @brief    rotating action command for PMCavity
* @author   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/PMCavity

#ifndef _XLH_FORTREND_PM_CAVITY_ROTATING_ACTION__INCLUDE_
#define _XLH_FORTREND_PM_CAVITY_ROTATING_ACTION__INCLUDE_ 
#include "KeyencePLC/keyence_plc_command_executer.h"
#include "KeyencePLC/keyence_plc_subsystem_helper.h"
#include "PMCavity/fortrend_pm_cavity_defined.h"

namespace FC{

/**
* @brief rotating action command for PMCavity
*/
class  PMCavityRotatingActionCommand : public std::enable_shared_from_this<PMCavityRotatingActionCommand>, public  KernelSubsystemCommand, public KeyencePlcCommandExecuter
{
public:
    DECLARE_PTR(PMCavityRotatingActionCommand)
    PMCavityRotatingActionCommand(KeyencePlcSubSystemHelper* helper,double degree);
    virtual std::string getName()const override {return "RotatingAxisAction";}
protected:
    virtual RunResult onRun() throw(KernelException);

private:
    DECLARE_PRIVATE(PMCavityRotatingActionCommand)
};

}
#endif

