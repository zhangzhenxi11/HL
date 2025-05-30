/**
 * @file            hex_loadport_getmap_command.h
 * @brief           return map command for fortrend loadport
 * @author			xielonghua
 */

// Library: Fortrend
// Package: CommandImp/Hex/LoadPort


#ifndef _XLH_EFEM_GETMAP_COMMAND_INCLUDE_ 
#define _XLH_EFEM_GETMAP_COMMAND_INCLUDE_
#include "Kernel/Fortrend/loadport_abstract_command.h"
#include "Kernel/Fortrend/hex_command_executer.h"

KERNEL_NS_BEGIN

class HexSubSystemHelper;

/**
 *@brief  return map command for fortrend loadport
*/
class  HexLoadPorGetMapCommand : public  LoadPortAbstractGetMapCommand,  public HexCommandExecuter
{
public:
	DECLARE_PTR(HexLoadPorGetMapCommand)
	HexLoadPorGetMapCommand(HexSubSystemHelper* hexHelper);
	virtual std::string getName() const override;

protected:
	virtual RunResult onRun() throw(KernelException);

private:
	DECLARE_PRIVATE(HexLoadPorGetMapCommand)
};

KERNEL_NS_END

#endif
