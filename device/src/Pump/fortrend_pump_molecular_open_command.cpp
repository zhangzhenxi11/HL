 /**
* @file            fortrend_pump_molecular_open_command.h
* @brief           molecular open command for Pump
* @author		   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/Pump

#include "Poco/Format.h"

#include "Kernel/kernel.h"
#include "Kernel/kernel_command.h"
#include "kernel/kernel_command_reject_exception.h"
#include "Kernel/kernel_log.h"
#include "kernel/Fortrend/fortrend_cassette_manager.h"

#include "Pump/fortrend_pump_molecular_open_command.h"
#include "Pump/fortrend_pump_subsystem.h"

#include "TMCavity/fortrend_tm_cavity_subsystem.h"
#include "LoadLock/fortrend_loadlock_subsystem.h"

#include <windows.h>

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif



namespace FC{

	

	/**
	* PumpMolecularOpenCommand
	*/
	PumpMolecularOpenCommand::PumpMolecularOpenCommand(KeyencePlcSubSystemHelper* helper, std::string name)
		:KeyencePlcCommandExecuter(helper){
		moduleName = name;
		//setMessageName("MolecularOpen");
		//setDescription("align the pump");
	};

	std::string PumpMolecularOpenCommand::getmoduleName(){
		return moduleName;
	}
	/**
	* return true if success else false.
	*/
	PumpMolecularOpenCommand::RunResult PumpMolecularOpenCommand::onRun() throw(KernelException){
		FortrendPumpSubsystem* sub = dynamic_cast<FortrendPumpSubsystem*>(getSubsystem());
		//
		if (!sub){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "子系统类型错误", this);
		}
		//get command configure
		std::shared_ptr<KernelConfiguration> command_config = sub->getConfigure()->createView(getName());

		std::string llaAddress = command_config->getString("start_address_lla", "");
		std::string llbAddress = command_config->getString("start_address_llb", "");
		std::string tmAddress = command_config->getString("start_address_tm", "");

		std::string llafinish_address = command_config->getString("finish_address_lla", "");
		std::string llbfinish_address = command_config->getString("finish_address_llb", "");
		std::string tmfinish_address = command_config->getString("finish_address_tm", "");

		std::string llafailed_address = command_config->getString("failed_address_lla", "");
		std::string llbfailed_address = command_config->getString("failed_address_llb", "");
		std::string tmfailed_address = command_config->getString("failed_address_tm", "");

		std::string address = "";
		std::string finish_address = "";
		std::string failed_address = "";
		int timeout = 0;
		std::shared_ptr<FortrendTMCavitySubsystem> tm = sub->getKernel()->getKernelModule<FortrendTMCavitySubsystem>("TM");
		std::shared_ptr<FortrendLoadLockSubsystem> lla = sub->getKernel()->getKernelModule<FortrendLoadLockSubsystem>("LLA");
		std::shared_ptr<FortrendLoadLockSubsystem> llb = sub->getKernel()->getKernelModule<FortrendLoadLockSubsystem>("LLB");
		if (!tm){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "传输腔子系统类型错误", this);
		}
		if (!lla){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "LLA子系统类型错误", this);
		}
		if (!llb){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "LLB子系统类型错误", this);
		}

		if (moduleName == "LLA"){
			address = llaAddress;
			finish_address = llafinish_address;
			failed_address = llafailed_address;
			timeout = command_config->getInt("timeout_lla", 450000);

			if (tm->getMoleculePipelineVacuumValue() > 30.0&&lla->getAngleValveOpend())
			{
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, "分子泵前级管路当前真空未达到20Pa以下，无法打开分子泵(逻辑错误)", this);
			}

			if (!lla->getLoadLockRoughVacuumReachesTheSetValue(10) && lla->getInsertingPlateValveOpend())
			{
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, "LLA当前真空未达到设定值，无法打开分子泵(逻辑错误)", this);
			}

			if (lla->getInsertingPlateValveOpend())
			{
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, "LLA腔插板阀未关闭(逻辑错误)", this);
			}

			/*if (lla->getHeightVacuumBaffleValveOpend() && lla->getInsertingPlateValveOpend())
			{
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, "LLA高真空挡板阀未关闭(逻辑错误)", this);
			}*/

			//if (!lla->getAngleValveOpend())
			//{
			//	throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, "LLA角阀未打开(逻辑错误)", this);
			//}

			/*if (lla->getVacuumValue()>1000){

				if (llb->getAngleValveOpend() && llb->getVacuumValue()<100)
				{
					throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, "LLB角阀未关闭(逻辑错误)", this);
				}
				if (tm->getAngleValveOpend() && tm->getTMCavityVacuumValue()<100)
				{
					throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, "TM角阀未关闭(逻辑错误)", this);
				}
			}*/
			

			
		}
		else if (moduleName == "LLB"){
			address = llbAddress;
			finish_address = llbfinish_address;
			failed_address = llbfailed_address;
			timeout = command_config->getInt("timeout_llb", 450000);


			if (tm->getMoleculePipelineVacuumValue() > 30.0&&llb->getAngleValveOpend())
			{
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, "分子泵前级管路当前真空未达到30Pa以下，无法打开分子泵(逻辑错误)", this);
			}

			if (!llb->getLoadLockRoughVacuumReachesTheSetValue(10) && llb->getInsertingPlateValveOpend())
			{
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, "LLB当前真空未达到设定值，无法打开分子泵(逻辑错误)", this);
			}

			if (llb->getInsertingPlateValveOpend())
			{
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, "LLB腔插板阀未关闭(逻辑错误)", this);
			}
			if (llb->getHeightVacuumBaffleValveOpend() && llb->getInsertingPlateValveOpend())
			{
		    	throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, "LLB高真空挡板阀未关闭(逻辑错误)", this);
			}

			/*if (!llb->getAngleValveOpend())
			{
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, "LLB角阀未打开(逻辑错误)", this);
			}*/

			/*if (llb->getVacuumValue()>1000){

				if (lla->getAngleValveOpend() && lla->getVacuumValue()<100)
				{
					throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, "LLA角阀未关闭(逻辑错误)", this);
				}
				if (tm->getAngleValveOpend() && tm->getTMCavityVacuumValue()<100)
				{
					throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, "TM角阀未关闭(逻辑错误)", this);
				}
			}*/

		}
		else if (moduleName == "TM"){
			address = tmAddress;
			finish_address = tmfinish_address;
			failed_address = tmfailed_address;
			timeout = command_config->getInt("timeout_tm", 450000);
			if (tm->getMoleculePipelineVacuumValue() > 30.0&&tm->getAngleValveOpend())
			{
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, "分子泵前级管路当前真空未达到20Pa以下，无法打开分子泵(逻辑错误)", this);
			}
			if (!tm->getTMCavityRoughVacuumReachesTheSetValue(10) && tm->getInsertingPlateValveOpend())
			{
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, "传输腔当前真空未达到设定值，无法打开分子泵(逻辑错误)", this);
			}
			
			if (tm->getInsertingPlateValveOpend())
			{
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, "传输腔插板阀未关闭(逻辑错误)", this);
			}
			if (tm->getHeightVacuumBaffleValveOpend() && tm->getInsertingPlateValveOpend())
			{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, "TM高真空挡板阀未关闭(逻辑错误)", this);
			}

			/*if (!tm->getAngleValveOpend())
			{
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, "传输腔角阀未打开(逻辑错误)", this);
			}*/

			/*if (tm->getTMCavityVacuumValue()>1000){

				if (lla->getAngleValveOpend() && lla->getVacuumValue()<100)
				{
					throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, "LLA角阀未关闭(逻辑错误)", this);
				}
				if (llb->getAngleValveOpend() && llb->getVacuumValue()<100)
				{
					throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, "LLB角阀未关闭(逻辑错误)", this);
				}
			}*/

		}
		
		if (timeout < 10){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_DATA_OUTOF_RANGE, Poco::format("超时: %s 打开分子泵超时参数错误", sub->getName()), this);
		}

		if (address == "" || finish_address == "" || failed_address=="")
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, Poco::format("地址: 打开分子泵地址未定义", getName()), this);
		}
		logInform(sub->getName().c_str(), "打开分子泵命令执行开始 %s", moduleName);

		if (!writeBit(address, true))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 写1到打开分子泵地址错误", sub->getName()), this);
		}
		Sleep(500);
		int loopCount = timeout / 20;
		int count = 0;
		bool readRes = false;
		bool readRes2 = false;
		while (count <= loopCount)
		{
			Sleep(20);
			readBit(finish_address, readRes);
			readBit(failed_address, readRes2);
			if (readRes || readRes2)
			{
				break;
			}
			count++;
		}
		IKernelCommand::RunResult ret = IKernelCommand::RunResult::RUN_FAILD;
		if (readRes)
		{
			ret = IKernelCommand::RunResult::RUN_OK;
			if (moduleName == "LLA"){
				sub->setMolecularPumpOpenedLLA(true);
			}
			else if (moduleName == "LLB"){
				sub->setMolecularPumpOpenedLLB(true);
			}
			else if (moduleName == "TM"){
				sub->setMolecularPumpOpenedTM(true);
			}
			logInform(sub->getName().c_str(), "打开%s分子泵命令执行完成", moduleName);

		}
		else if (readRes2)
		{
			AlarmMessage::Ptr alarm(new AlarmMessage(0, 2, Poco::format("打开%s分子泵命令执行失败", moduleName)));
			setAlarm(alarm);
		}
		else
		{
			AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_COMMUNICATION_TIMEOUT, Poco::format("打开%s分子泵命令通讯超时", moduleName)));
			setAlarm(alarm);
		}

		return ret;
	}



}