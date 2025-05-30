/**
* @file     fortrend_sunwayrobot_output_command.h
* @brief    output command for sunway robot
* @author   kai
*/

// Library: Fortrend
// Package: SubSystem/SunwayRobot

#ifndef _XLH_FORTREND_SUNWAYROBOT_OUTPUT_INCLUDE_
#define _XLH_FORTREND_SUNWAYROBOT_OUTPUT_INCLUDE_ 
#include "kernel/kernel_subsystem_command.h"
#include "kernel/Fortrend/abstract_output_command.h"

#include "SunwayRobot/sunway_subsystem_helper.h"
#include "SunwayRobot/sunway_command_executer.h"

namespace FC{

	/**
	* @brief output command for SunwayRobot
	*/
	class  SunwayRobotOutputCommand : public  AbstractOutPutCommand, public SunwayCommandExecuter
	{
	public:
		DECLARE_PTR(SunwayRobotOutputCommand)
		SunwayRobotOutputCommand(int chanel,bool state,SunwaySubSystemHelper* helper);
		virtual std::string getName()const override { return "Output"; }
	protected:
		virtual RunResult onRun() throw(KernelException);

	private:
		DECLARE_PRIVATE(SunwayRobotOutputCommand)
	};

}
#endif

