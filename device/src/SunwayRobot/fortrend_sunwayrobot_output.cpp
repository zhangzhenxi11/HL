/**
* @file     fortrend_sunwayrobot_output_command.h
* @brief    get_wafer command for sunway robot
* @author   kai
*/

// Library: Fortrend
// Package: SubSystem/SunwayRobot

#include "SunwayRobot/fortrend_sunwayrobot_output_command.h"
#include "SunwayRobot/fortrend_sunwayrobot_subsystem.h"
#include "kernel/kernel_command_reject_exception.h"

#include "Poco/Format.h"


namespace FC{

	/**
	* SunwayRobotOutputCommandPrivate
	*/
	class SunwayRobotOutputCommandPrivate{
	public:
		int chanel = 0;
		bool state = false;
	};

	/**
	* SunwayRobotOutputCommand
	*/
	SunwayRobotOutputCommand::SunwayRobotOutputCommand(int chanel, bool state, SunwaySubSystemHelper* helper)
		:SunwayCommandExecuter(helper)
		, d(new SunwayRobotOutputCommandPrivate){
		d->chanel = chanel;
		d->state = state;
		//setMessageName("Output");
		//setDescription("Output on SunwayRobot");

	};



	SunwayRobotOutputCommand::RunResult SunwayRobotOutputCommand::onRun() throw(KernelException){
		FortrendSunwayRobotSubsystem* fliper = dynamic_cast<FortrendSunwayRobotSubsystem*>(getSubsystem());
		//
		if (!fliper){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "子系统类型错误", this);
		}

		//get command configure
		std::shared_ptr<KernelConfiguration> command_config = fliper->getConfigure()->createView(getName());

		//fill params
		std::string macroId = command_config->getString("macroid", "ORG");
		int timeout = command_config->getInt("macrotimeout", 200000);

		std::string res;
		//ALG
		sendRequest("ORG");
		res = recvResponse(timeout);
		if (res != std::string("ORG")){
			//set alarm data
			AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_RESPONSE_ERROR, "Robot Reset command faild."));
			setAlarm(alarm);
			return RunResult::RUN_FAILD;
		}

		return RunResult::RUN_OK;

	}

}
