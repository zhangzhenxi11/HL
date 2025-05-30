/**
 * @file            hex_loadport_lockbox_command.h
 * @brief           lockbox command for fortrend loadport
 * @author			xielonghua
 */

// Library: Fortrend
// Package: CommandImp/Hex/LoadPort


#ifndef _XLH_EFEM_LOCKBOX_COMMAND_INCLUDE_ 
#define _XLH_EFEM_LOCKBOX_COMMAND_INCLUDE_
#include "Kernel/Fortrend/loadport_abstract_command.h"
#include "Kernel/Fortrend/hex_command_executer.h"

KERNEL_NS_BEGIN
class HexSubSystemHelper;

/**
 *@brief  lockbox command for fortrend loadport
*/
class  EFEMLoadPortLockBoxCommand : public  LoadPortAbstractLockBoxCommand,  public HexCommandExecuter
{
public:
	DECLARE_PTR(EFEMLoadPortLockBoxCommand)
	EFEMLoadPortLockBoxCommand(HexSubSystemHelper* hexHelper);
	virtual std::string getName() const override;

protected:
	virtual RunResult onRun() throw(KernelException);

private:
	DECLARE_PRIVATE(EFEMLoadPortLockBoxCommand)
};

KERNEL_NS_END

#endif
