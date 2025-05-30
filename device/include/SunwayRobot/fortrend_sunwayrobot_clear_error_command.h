/**
* @file     fortrend_sunwayrobot_clear_error_command.h
* @brief    clear error command for sunway robot
* @author   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/SunwayRobot

#ifndef _XLH_FORTREND_SUNWAYROBOT_CLEAR_ERROR_INCLUDE_
#define _XLH_FORTREND_SUNWAYROBOT_CLEAR_ERROR_INCLUDE_ 
#include "Kernel/Fortrend/hex_command_executer.h"
#include "kernel/kernel_subsystem_command.h"
#include "kernel/kernel_subsystem_reset_command.h"

#include "SunwayRobot/sunway_subsystem_helper.h"
#include "SunwayRobot/sunway_command_executer.h"

namespace FC{

	/**
	* @brief clear error command for SunwayRobot
	*/
	class  SunwayRobotClearErrorCommand : public  KernelSubsystemResetCommand, public SunwayCommandExecuter
	{
	public:
		DECLARE_PTR(SunwayRobotClearErrorCommand)
		SunwayRobotClearErrorCommand(SunwaySubSystemHelper* helper);
		virtual std::string getName()const override { return "ClearError"; }
	protected:
		virtual RunResult onRun() throw(KernelException);

	private:
		DECLARE_PRIVATE(SunwayRobotClearErrorCommand)
	};

}
#endif

