/**
 * @file            rnd_aligner_status_command.h
 * @brief           update status command for rnd  aligner subsystem
 * @author			xielonghua
 */

// Library: Fortrend
// Package: CommandImp/Rnd/Aligner



#ifndef _XLH_EFEM_ALIGNER_STATUS_COMMAND_INCLUDE_
#define _XLH_EFEM_ALIGNER_STATUS_COMMAND_INCLUDE_
#include "Kernel/Fortrend/rnd_command_executer.h"
#include  "Kernel/kernel_subsystem_update_command.h"
#include "Kernel/kernel_exception.h" 

KERNEL_NS_BEGIN

class HexSubSystemHelper;

/**
* @brief   update status command for rnd  aligner subsystem
*/
class  EFEMAlignerStatusCommand : public KernelSubsystemUpdateCommand
{
public:
	DECLARE_PTR(EFEMAlignerStatusCommand) 
	EFEMAlignerStatusCommand();
	virtual std::string getName() const override { return "Update"; }
protected:
	virtual bool isNeedResetwhenFaild()const override { return false; }
	virtual bool canBeStandby()const override { return true; }
	virtual RunResult onRun() throw(KernelException)override;
private:
	DECLARE_PRIVATE(EFEMAlignerStatusCommand)
};

KERNEL_NS_END

#endif
