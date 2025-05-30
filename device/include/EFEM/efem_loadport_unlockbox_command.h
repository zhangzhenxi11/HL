/**
 * @file            hex_loadport_unlockbox_command.h
 * @brief           unlockbox command for fortrend loadport
 * @author			xielonghua
 */

// Library: Fortrend
// Package: CommandImp/Hex/LoadPort



#ifndef _XLH_EFEM_UNLOCKBOX_COMMAND_INCLUDE_ 
#define _XLH_EFEM_UNLOCKBOX_COMMAND_INCLUDE_
#include "Kernel/Fortrend/loadport_abstract_command.h"
#include "Kernel/Fortrend/hex_command_executer.h"

KERNEL_NS_BEGIN
class HexSubSystemHelper;

/**
 *@brief  unlockbox command for fortrend loadport
*/
class  EFEMLoadPortUnlockBoxCommand : public  LoadPortAbstractUnlockBoxCommand,  public HexCommandExecuter
{
public:
	DECLARE_PTR(EFEMLoadPortUnlockBoxCommand)
	EFEMLoadPortUnlockBoxCommand(HexSubSystemHelper* hexHelper);
	virtual std::string getName() const override;

protected:
	virtual RunResult onRun() throw(KernelException);

private:
	DECLARE_PRIVATE(EFEMLoadPortUnlockBoxCommand)
};

KERNEL_NS_END

#endif
