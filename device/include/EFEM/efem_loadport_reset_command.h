/**
 * @file            hex_loadport_reset_command.h
 * @brief           reset command for fortrend loadport subsystem
 * @author			xielonghua
 */

// Library: Fortrend
// Package: CommandImp/Hex/LoadPort



#ifndef _XLH_EFEM_LOADPORT_RESET_COMMAND_INCLUDE_
#define _XLH_EFEM_LOADPORT_RESET_COMMAND_INCLUDE_
#include "Kernel/Fortrend/hex_reset_command.h"
#include "kernel/kernel_exception.h" 

KERNEL_NS_BEGIN


/**
*@brief   reset command for loadport
*/
class  EFEMLoadPortResetCommand : public  HexResetCommand
{
public:
	DECLARE_PTR(EFEMLoadPortResetCommand) 
	EFEMLoadPortResetCommand(HexSubSystemHelper* hexHelper);

protected:
	virtual RunResult onRun() throw(KernelException);

};

KERNEL_NS_END

#endif
