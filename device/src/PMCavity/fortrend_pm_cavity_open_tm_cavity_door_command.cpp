/**
* @file            fortrend_pm_cavity_open_tm_cavity_door_command.h
* @brief           open tm cavity door command for pm cavity
* @author		   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/PMCavity

#include "Kernel/kernel.h"
#include "Kernel/kernel_command.h"
#include "Kernel/kernel_abstract_subsystem.h"
#include "kernel/kernel_event_id.h"
#include "kernel/kernel_event_paramters.h"
#include "kernel/kernel_command_reject_exception.h"
#include "Kernel/kernel_log.h"
#include "kernel/Fortrend/fortrend_cassette_manager.h"
#include "Poco/Format.h"

#include "TMCavity/fortrend_tm_cavity_subsystem.h"
#include "PMCavity/fortrend_pm_cavity_open_tm_cavity_door_command.h"
#include "PMCavity/fortrend_pm_cavity_subsystem.h"

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif


namespace FC{

	

	/**
	* PMCavityOpenTMCavityDoorCommand
	*/
	PMCavityOpenTMCavityDoorCommand::PMCavityOpenTMCavityDoorCommand(KeyencePlcSubSystemHelper* keyence_helper, InovancePlcSubSystemHelper* inovance_helper)
		:KeyencePlcCommandExecuter(keyence_helper) , InovancePlcCommandExecuter(inovance_helper){
		//setMessageName("OpenTMCavityDoor");
		//setDescription("open tm cavity door the pm cavity");
	};
	
	/**
	* return true if success else false.
	*/
	PMCavityOpenTMCavityDoorCommand::RunResult PMCavityOpenTMCavityDoorCommand::onRun() throw(KernelException){
		return IKernelCommand::RunResult::RUN_OK;
		FortrendPMCavitySubsystem* sub = dynamic_cast<FortrendPMCavitySubsystem*>(getSubsystem());
		//
		if (!sub){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "子系统类型错误", this);
		}
		FortrendTMCavitySubsystem * tm = dynamic_cast<FortrendTMCavitySubsystem*>((sub->getKernel()->getKernelModule<FortrendTMCavitySubsystem>("TM")).get());
		//
		if (!tm){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "传输腔类型错误", this);
		}
		if (!tm->getTMCavityVacuumValueUpperLimitReachesTheSetValue())
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, "TM传输腔当前的的真空值未达到上限设定值", this);
		}
		if (!sub->getVacuumValueReachesTheSetValue())
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, "PM腔当前的的真空值未达到设定值", this);
		}
		if (!sub->getTemperatureValueReachesTheSetValue())
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, "PM腔当前的的温度值未达到设定值", this);
		}
		if (!sub->getPMCavityRemoteMode())
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, "PM腔未处于远程模式状态", this);
		}
		if (!sub->getPMCavitySafeSignal())
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, "PM腔未发出安全信号", this);
		}
		if (sub->getPMCavityHasAlarm())
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, "PM腔内部报警中", this);
		}
		if (tm->getVacuumEnable())
		{
			if ((tm->getTMCavityVacuumValue() * sub->getPMCavityMagnitude() - sub->getVacuumValue()) < 0.0)
			{
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, "传输腔和PM腔的压差未达到设定值", this);
			}
		}
		else
		{
			if (!sub->getExhaustVacuumValueReachesTheSetValue())
			{
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, "传输腔当前的的真空值未达到正常气压设定值", this);
			}
			/*if (tm->getTMCavityVacuumPressureGageState())
			{
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_STATE_EXCEPTION, Poco::format("工位: %s 真空压力表有信号（逻辑错误），请检查压力是否正常", tm->getName()), this);
			}*/
		}
		
		//check modules
		auto cassManager = sub->getKernel()->getKernelModule<FortrendCassetteManager>();
		//get cass
		auto station_cass = cassManager->getCassette(sub);
		if (!station_cass){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_STATION_WITHOUT_CASS_EXCEPTION, Poco::format("工位: %s 晶圆盒为空.", sub->getName()), this);
		}
		if (station_cass->getMapping(1) == Cassette::Mapping::Present)
		{
			if (sub->getPMCavityGetRequest() == false)
			{
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, "PM腔存在晶圆，没有取片请求！", this);
			}
		}
		else if (station_cass->getMapping(1) == Cassette::Mapping::Empty)
		{
			if (sub->getPMCavityUploadRequest() == false)
			{
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, "PM腔不存在晶圆，没有上片请求！", this);
			}
		}
		else{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, "PM腔晶圆状态异常！", this);
		}
		//读取PM腔的PLC报警
		if (sub->getPMCavityHasAlarm())
		{
			std::string message = sub->getPMCavityAlarmMessage();
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, message, this);
		}

		//get command configure
		std::shared_ptr<KernelConfiguration> command_config = sub->getConfigure()->createView(getName());

		//fill params
		std::string open_address = command_config->getString("open_address", "");
		std::string close_address = command_config->getString("close_address", "");
		std::string finish_address = command_config->getString("finish_address", "");
		int timeout = command_config->getInt("timeout", -1);
		if (timeout < 10){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_DATA_OUTOF_RANGE, Poco::format("超时: %s 打开传输腔插板门阀超时参数设置异常", sub->getName()), this);
		}
		if ((open_address == "") || (close_address == "") || (finish_address == ""))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, Poco::format("地址: 打开传输腔插板门阀地址未定义", getName()), this);
		}
		
		logInform(sub->getName().c_str(), "打开传输腔插板门阀命令开始");
		if (!KeyencePlcCommandExecuter::writeBit(close_address, false))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 写0到关闭传输腔门阀命令地址错误", sub->getName()), this);
		}
		Sleep(20);
		if (!KeyencePlcCommandExecuter::writeBit(open_address, true))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 写1到打开传输腔门阀命令地址错误", sub->getName()), this);
		}
		Sleep(500);
		int loopCount = timeout / 20;
		int count = 0;
		bool readRes = false;
		bool readState = false;
		while (count <= loopCount)
		{
			Sleep(20);
			readState = KeyencePlcCommandExecuter::readBit(finish_address, readRes);
			if (readRes)
			{
				break;
			}
			count++;
		}
		IKernelCommand::RunResult ret = IKernelCommand::RunResult::RUN_FAILD;
		if (readRes)
		{
			sub->setDoorOpen(true);
			ret = IKernelCommand::RunResult::RUN_OK;
			logInform(sub->getName().c_str(), "打开传输腔插板门阀命令完成");

		}
		else if (readState)
		{
			AlarmMessage::Ptr alarm(new AlarmMessage(1, 1, "打开传输腔门阀命令执行失败，打开传输腔门阀到位信号异常"));
			setAlarm(alarm);
			logError(sub->getName().c_str(), "打开传输腔门阀命令执行失败，打开传输腔门阀到位信号异常");
		}
		else
		{
			AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_COMMUNICATION_TIMEOUT, "打开传输腔插板阀超时"));
			setAlarm(alarm);
			logError(sub->getName().c_str(), "打开传输腔插板阀超时");
		}
		return ret;

	}



}