/**
 * @file            rnd_aligner_align_command.h
 * @brief           align command for rnd  aligner subsystem
 * @author			xielonghua
 */

// Library: Fortrend
// Package: CommandImp/Rnd/Aligner



#ifndef _XLH_EFEM_ALIGN_COMMAND_INCLUDE_
#define _XLH_EFEM_ALIGN_COMMAND_INCLUDE_
//#include "Kernel/Fortrend/rnd_command_executer.h"
#include "Kernel/Fortrend/aligner_abstract_command.h"
#include "Kernel/kernel_exception.h" 
#include "Kernel/Fortrend/hex_command_executer.h"


KERNEL_NS_BEGIN
class HexSubSystemHelper;


/**
*@brief   align command for rnd  aligner subsystem
*/
class  EFEMAlignerAlignCommand :  public AlignerAbstractAlignCommand, public HexCommandExecuter
{
public:
	DECLARE_PTR(EFEMAlignerAlignCommand)
	EFEMAlignerAlignCommand(HexSubSystemHelper* hexHelper);

	virtual std::string getName() const override { return "Align"; }

protected:
	virtual RunResult onRun() throw(KernelException)override;
private:
	DECLARE_PRIVATE(EFEMAlignerAlignCommand)
};

KERNEL_NS_END

#endif
