/**
 * @file            hex_loadport_closebox_command.h
 * @brief           closebox command for fortrend loadport
 * @author			xielonghua
 */

// Library: Fortrend
// Package: CommandImp/Hex/LoadPort



#ifndef _XLH_EFEM_CLOSEBOX_COMMAND_INCLUDE_ 
#define _XLH_EFEM_CLOSEBOX_COMMAND_INCLUDE_
#include "Kernel/Fortrend/loadport_abstract_command.h"
#include "Kernel/Fortrend/hex_command_executer.h"

KERNEL_NS_BEGIN
class HexSubSystemHelper;

/**
 *@brief   closebox command for fortrend loadport
*/
class  EFEMLoadPortCloseBoxCommand : public  LoadPortAbstractCloseBoxCommand,  public HexCommandExecuter
{
public:
	DECLARE_PTR(EFEMLoadPortCloseBoxCommand)
	EFEMLoadPortCloseBoxCommand(HexSubSystemHelper* hexHelper);
	virtual std::string getName() const override;

protected:
	virtual RunResult onRun() throw(KernelException);

private:
	DECLARE_PRIVATE(EFEMLoadPortCloseBoxCommand)
};

KERNEL_NS_END

#endif
