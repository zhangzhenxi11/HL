/**
 * @file            hex_loadport_openbox_command.h
 * @brief           openbox command for fortrend loadport
 * @author			xielonghua
 */

// Library: Fortrend
// Package: CommandImp/Hex/LoadPort




#ifndef _XLH_EFEM_OPENBOX_COMMAND_INCLUDE_ 
#define _XLH_EFEM_OPENBOX_COMMAND_INCLUDE_
#include "Kernel/Fortrend/loadport_abstract_command.h"
#include "Kernel/Fortrend/hex_command_executer.h"

KERNEL_NS_BEGIN
class HexSubSystemHelper;

/**
 *@brief   openbox command for fortrend loadport
*/
class  EFEMLoadPortOpenBoxCommand : public LoadPortAbstractOpenBoxCommand, public HexCommandExecuter
{
public:
	DECLARE_PTR(EFEMLoadPortOpenBoxCommand)
	EFEMLoadPortOpenBoxCommand(HexSubSystemHelper* hexHelper);
	virtual std::string getName() const override;
	virtual bool isNeedResetwhenFaild()const  override;

protected:
	virtual RunResult onRun() throw(KernelException);

private:
	DECLARE_PRIVATE(EFEMLoadPortOpenBoxCommand)
};

KERNEL_NS_END

#endif
