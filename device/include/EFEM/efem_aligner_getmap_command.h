/**
 * @file            efem_aligner_getmap_command.h
 * @brief           getmapdt status command for hex  aligner subsystem
 * @author			xielonghua
 */

// Library: Fortrend
// Package: CommandImp/Rnd/Aligner



#ifndef _XLH_EFEM_ALIGNER_GETMAP_COMMAND_INCLUDE_
#define _XLH_EFEM_ALIGNER_GETMAP_COMMAND_INCLUDE_

#include "Kernel/Fortrend/hex_command_executer.h"
#include "Kernel/Fortrend/aligner_abstract_command.h"
#include "Kernel/kernel_exception.h" 

KERNEL_NS_BEGIN

class HexSubSystemHelper;

/**
* @brief   GetMap command for hex aligner subsystem
*/
class  EFEMAlignerGetMapCommand : public KernelSubsystemCommand, public HexCommandExecuter
{
public:
	DECLARE_PTR(EFEMAlignerGetMapCommand) 
	EFEMAlignerGetMapCommand(HexSubSystemHelper* hexHelper);
	virtual std::string getName() const override { return "GetMap"; }
protected:
	virtual RunResult onRun() throw(KernelException)override;
private:
	DECLARE_PRIVATE(EFEMAlignerGetMapCommand)
};

KERNEL_NS_END

#endif
