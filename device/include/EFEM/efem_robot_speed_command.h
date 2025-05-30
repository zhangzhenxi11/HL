/**
 * @file            hex_loadport_reset_command.h
 * @brief           reset command for fortrend loadport subsystem
 * @author			xielonghua
 */

// Library: Fortrend
// Package: CommandImp/Hex/LoadPort



#ifndef _XLH_EFEM_LOADPORT_SPEED_COMMAND_INCLUDE_
#define _XLH_EFEM_LOADPORT_SPEED_COMMAND_INCLUDE_
#include "Kernel/Fortrend/hex_robot_setspeed_command.h"
#include "kernel/kernel_exception.h" 

KERNEL_NS_BEGIN


/**
*@brief   reset command for loadport
*/
class  EFEMRobotSpeedCommand : public  HexRobotSetSpeedCommand
{
public:
	DECLARE_PTR(EFEMRobotSpeedCommand)
	EFEMRobotSpeedCommand(HexSubSystemHelper* hexHelper,int speed);

protected:
	virtual RunResult onRun() throw(KernelException);
private:
	int speed;
};

KERNEL_NS_END

#endif
