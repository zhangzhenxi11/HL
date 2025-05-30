/**
 * @file            hex_loadport_opendoor_command.h
 * @brief           opendoor command for fortrend loadport
 * @author			xielonghua
 */

// Library: Fortrend
// Package: CommandImp/Hex/LoadPort


#ifndef _XLH_EFEM_OPENDOOR_COMMAND_INCLUDE_ 
#define _XLH_EFEM_OPENDOOR_COMMAND_INCLUDE_
#include "Kernel/Fortrend/loadport_abstract_command.h"
#include "Kernel/Fortrend/hex_command_executer.h"


KERNEL_NS_BEGIN
class HexSubSystemHelper;

/**
 *@brief  open door command for load port
*/
class  EFEMLoadPortOpenDoorCommand : public LoadPortAbstractOpenDoorCommand, public HexCommandExecuter
{
public:
	DECLARE_PTR(EFEMLoadPortOpenDoorCommand)
	EFEMLoadPortOpenDoorCommand(HexSubSystemHelper* hexHelper);
	virtual std::string getName() const override;

protected:
	virtual RunResult onRun() throw(KernelException);

private:
	DECLARE_PRIVATE(EFEMLoadPortOpenDoorCommand)
};

KERNEL_NS_END

#endif
