/**
 * @file            rnd_aligner_reset_command.h
 * @brief           reset command for rnd  aligner subsystem
 * @author			xielonghua
 */

// Library: Fortrend
// Package: CommandImp/Rnd/Aligner



#ifndef _XLH_EFEM_RESET_COMMAND_INCLUDE_
#define _XLH_EFEM_RESET_COMMAND_INCLUDE_

#include "Kernel/Fortrend/hex_reset_command.h"
#include "Kernel/Fortrend/aligner_abstract_command.h"
#include "Kernel/kernel_subsystem_reset_command.h"
#include "Kernel/kernel_exception.h" 

KERNEL_NS_BEGIN

/**
* @brief   reset command for rnd  aligner subsystem
*/
class  EFEMAlignerResetCommand : public HexResetCommand
{
public:
	DECLARE_PTR(EFEMAlignerResetCommand)
	EFEMAlignerResetCommand(HexSubSystemHelper* hexHelper);
	virtual std::string getName() const override { return "Reset"; }

protected:
	virtual RunResult onRun() throw(KernelException)override;
private:
	DECLARE_PRIVATE(EFEMAlignerResetCommand)
};

KERNEL_NS_END

#endif
