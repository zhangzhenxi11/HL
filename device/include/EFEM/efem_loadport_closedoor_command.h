/**
 * @file            hex_loadport_closedoor_command.h
 * @brief           close door command for load port
 * @author			xielonghua
 */

// Library: Fortrend
// Package: CommandImp/Hex/LoadPort

#ifndef _XLH_EFEM_CLOSEDOOR_COMMAND_INCLUDE_ 
#define _XLH_EFEM_CLOSEDOOR_COMMAND_INCLUDE_
#include "Kernel/Fortrend/loadport_abstract_command.h"
#include "Kernel/Fortrend/hex_command_executer.h"


KERNEL_NS_BEGIN
class HexSubSystemHelper;

/**
 *@brief close door command for fims
*/
class   EFEMLoadPortCloseDoorCommand : public LoadPortAbstractCloseDoorCommand, public HexCommandExecuter
{
public:
	DECLARE_PTR(EFEMLoadPortCloseDoorCommand)
	EFEMLoadPortCloseDoorCommand(HexSubSystemHelper* hexHelper);
	virtual std::string getName() const override;

protected:
	virtual RunResult onRun() throw(KernelException);

private:
	DECLARE_PRIVATE(EFEMLoadPortCloseDoorCommand)
};

KERNEL_NS_END

#endif
