/**
 * @file            rnd_aligner_rotate_command.h
 * @brief           rotate command for rnd  aligner subsystem
 * @author			xielonghua
 */

// Library: Fortrend
// Package: CommandImp/Rnd/Aligner

#ifndef _XLH_EFEM_ALIGNER_ROTATE_COMMAND_INCLUDE_
#define _XLH_EFEM_ALIGNER_ROTATE_COMMAND_INCLUDE_

#include "Kernel/Fortrend/hex_command_executer.h"
#include "Kernel/Fortrend/aligner_abstract_command.h"
#include "Kernel/kernel_exception.h" 

KERNEL_NS_BEGIN

class HexSubSystemHelper;

/**
*@brief  rotate command for rnd  aligner subsystem
*/
class  EFEMAlignerRotateCommand : public AlignerAbstractRotateCommand, public  HexCommandExecuter
{
public:
	DECLARE_PTR(EFEMAlignerRotateCommand) 
	EFEMAlignerRotateCommand(HexSubSystemHelper* hexHelper, float angle);

	virtual std::string getName() const override { return "Rotate"; }
protected:
	virtual RunResult onRun() throw(KernelException)override;
private:
	DECLARE_PRIVATE(EFEMAlignerRotateCommand)
};

KERNEL_NS_END

#endif
