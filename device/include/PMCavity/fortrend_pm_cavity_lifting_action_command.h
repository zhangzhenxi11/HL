/**
* @file     fortrend_pm_cavity_lifting_action_command.h
* @brief    lifting action command for PMCavity
* @author   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/PMCavity

#ifndef _XLH_FORTREND_PM_CAVITY_LIFTING_ACTION__INCLUDE_
#define _XLH_FORTREND_PM_CAVITY_LIFTING_ACTION__INCLUDE_ 
#include "KeyencePLC/keyence_plc_command_executer.h"
#include "KeyencePLC/keyence_plc_subsystem_helper.h"
#include "PMCavity/fortrend_pm_cavity_defined.h"

namespace FC{

/**
* @brief lifting action command for PMCavity
*/
class  PMCavityLiftingActionCommand : public std::enable_shared_from_this<PMCavityLiftingActionCommand>, public  KernelSubsystemCommand, public KeyencePlcCommandExecuter
{
public:
    DECLARE_PTR(PMCavityLiftingActionCommand)
    PMCavityLiftingActionCommand(KeyencePlcSubSystemHelper* helper, double targetPos);
    virtual std::string getName()const override {return "LiftingAxisACtion";}
protected:
    virtual RunResult onRun() throw(KernelException);

private:
    DECLARE_PRIVATE(PMCavityLiftingActionCommand)
};

}
#endif

