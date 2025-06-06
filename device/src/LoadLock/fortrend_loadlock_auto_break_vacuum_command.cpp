/**
* @file            fortrend_pump_open_loadlock2_auto_vacuum_command.h
* @brief           open loadlock2 auto command for Pump
* @author		   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/Pump

#include "Poco/Format.h"

#include "Kernel/kernel.h"
#include "Kernel/kernel_action_subsystem.h"
#include "Kernel/kernel_command.h"
#include "kernel/kernel_command_reject_exception.h"
#include "Kernel/kernel_log.h"
#include "kernel/kernel_subsystem.h"

#include "kernel/Fortrend/fortrend_cassette_manager.h"

#include "LoadLock/fortrend_loadlock_auto_break_vacuum_command.h"

#include "LoadLock/fortrend_loadlock_subsystem.h"
#include "LoadLock/fortrend_loadlock_close_angle_valve_command.h"
#include "LoadLock/fortrend_loadlock_close_diaphragm_valve_command.h"
#include "LoadLock/fortrend_loadlock_cavity_close_height_vacuum_baffle_valve_command.h"
#include "LoadLock/fortrend_loadlock_cavity_close_inserting_plate_valve_command.h"
#include "LoadLock/fortrend_loadlock_open_angle_valve_command.h"
#include "LoadLock/fortrend_loadlock_cavity_open_height_vacuum_baffle_valve_command.h"
#include "LoadLock/fortrend_loadlock_cavity_open_inserting_plate_valve_command.h"


//#include "Pump/fortrend_pump_open_tm_cavity_auto_break_vacuum_command.h"
#include "Pump/fortrend_pump_subsystem.h"
#include "Pump/fortrend_pump_mechanical_open_command.h"
#include "Pump/fortrend_pump_molecular_open_command.h"

#include "TMCavity/fortrend_tm_cavity_defined.h"
#include "TMCavity/fortrend_tm_cavity_subsystem.h"
#include "TMCavity/fortrend_tm_cavity_close_angle_valve_command.h"
#include "TMCavity/fortrend_tm_cavity_close_diaphragm_valve_command.h"
#include "TMCavity/fortrend_tm_cavity_close_height_vacuum_baffle_valve_command.h"
#include "TMCavity/fortrend_tm_cavity_close_inserting_plate_valve_command.h"
#include "TMCavity/fortrend_tm_cavity_open_angle_valve_command.h"
#include "TMCavity/fortrend_tm_cavity_open_height_vacuum_baffle_valve_command.h"
#include "TMCavity/fortrend_tm_cavity_open_inserting_plate_valve_command.h"

#include <windows.h>


#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif


namespace FC{



	/**
	* LoadLockAutoBreakVacuumCommand
	*/
	LoadLockAutoBreakVacuumCommand::LoadLockAutoBreakVacuumCommand(KeyencePlcSubSystemHelper* helper)
		:KeyencePlcCommandExecuter(helper){
		//setMessageName("OpenLoadLock2AutoBreakVacuum");
		//setDescription("open loadlock2 auto cauum for pump");

	};


	void LoadLockAutoBreakVacuumCommand::initializeHLStateHandlers()
	{
	}

	/**
	* return true if success else false.
	*/
	LoadLockAutoBreakVacuumCommand::RunResult LoadLockAutoBreakVacuumCommand::onRun() throw(KernelException){
		
		 FortrendLoadLockSubsystem* sub = dynamic_cast<FortrendLoadLockSubsystem*>(getSubsystem());

		if (!sub)
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "LoadLock子系统类型错误", this);
		}
		if (sub->getState() != IKernelSubSystem::State::SUB_NORMAL)
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_DOOR_EXCEPTION, Poco::format("子系统: %s 不在正常状态", sub->getName()), this);
		}

		if (sub->getCassetteDoorOpend())
		{
		throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_DOOR_EXCEPTION, Poco::format("子系统: %s 放晶圆盒的门已打开（逻辑错误）", sub->getName()), this);
		}
		if (sub->getTMCavityDoorOpend())
		{
		throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_DOOR_EXCEPTION, Poco::format("子系统: %s 传输腔门阀已打开（逻辑错误）", sub->getName()), this);
		}

		//get command configure
		std::shared_ptr<KernelConfiguration> command_config = sub->getConfigure()->createView(getName());
		//fill params
		int timeout = command_config->getInt("timeout", 50 * 60 * 1000);
		if (timeout < 10){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_DATA_OUTOF_RANGE, Poco::format("超时: 破%s 真空命令超时参数错误", sub->getName()), this);
		}

		IKernelCommand::RunResult ret = IKernelCommand::RunResult::RUN_FAILD;
		logInform(sub->getName().c_str(), Poco::format("破%s真空命令开始", sub->getName()).c_str());
		bool loop = true;
		int llb_loop_count = 0;
		int step = 10;
		std::chrono::system_clock::time_point time_clock = std::chrono::system_clock::now();   //抽真空计时
		while (loop)
		{
			switch (step)
			{
#pragma region 破真空流程
			case 10://没有晶圆盒
			{
				//排气压力达到设定值9W9
				if (!sub->getExhaustVacuumValueReachesTheSetValue())
				{
					step = 100;
				}
				else
				{
					step = 5210;
				}

			}
			break;
			case 100:
			{
				if (sub->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					if (sub->getHeightVacuumBaffleValveOpend()){
						auto cmd = sub->createCloseHeightVacuumBaffleValveCommand();
						sub->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							addCommandExecutionAlarmMessage(sub->getName(), "关闭挡板阀", step);
						}
						else{
							step = 105;
						}
					}
					else{
						step = 105;
					}
				}
				else{
					addSubsystemNotNormalAlarmMessage(step, sub->getName());
				}
			}
			break;
			case 105:
			{
				if (sub->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					if (sub->getInsertingPlateValveOpend()){
						auto cmd = sub->createCloseInsertingPlateValveCommand();
						sub->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							addCommandExecutionAlarmMessage(sub->getName(), "关闭插板阀", step);
						}
						else{
							step = 110;
						}
					}
					else{
						step = 110;
					}


				}
				else{
					addSubsystemNotNormalAlarmMessage(step, sub->getName());
				}
			}
			break;
			//打开隔膜阀2 慢充
			case 110:
			{
				if (sub->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					auto cmd = sub->createOpenDiaphragmValveCommand(LoadLockValveOpening::LoadLock_Slow);
					sub->startCommand(cmd);
					cmd->wait();
					if (cmd->hasError())
					{
						addCommandExecutionAlarmMessage(sub->getName(), "打开隔膜阀慢充", step);
					}
					else{
						step = 120;
					}
				}
				else{
					addSubsystemNotNormalAlarmMessage(step, sub->getName());
				}
			}
			break;
			case 120:
			{
				if (sub->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					step = 130;
				}
				else{
					addSubsystemNotNormalAlarmMessage(step, sub->getName());
				}
			}
			break;
			case 130:
			{
				if (sub->getQuickInflationValueReachesTheSetValue())//真空值大于5000pa
				{

					step = 140;
				}
				else{
					Sleep(100);
				}
			}
			break;
			case 140:
			{
				/*if (sub->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					auto cmd = sub->createCloseDiaphragmValveCommand(LoadLockValveOpening::LoadLock_Slow);
					sub->startCommand(cmd);
					cmd->wait();
					if (cmd->hasError())
					{
						addCommandExecutionAlarmMessage(sub->getName(), "关闭隔膜阀慢充", step);
					}
					else{
						step = 150;
					}
				}
				else{
					addSubsystemNotNormalAlarmMessage(step, sub->getName());
				}*/
				step = 150;
			}
			break;
			//打开隔膜阀1 快充
			case 150:
			{
				if (sub->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					auto cmd = sub->createOpenDiaphragmValveCommand(LoadLockValveOpening::LoadLock_Fast);
					sub->startCommand(cmd);
					cmd->wait();
					if (cmd->hasError())
					{
						addCommandExecutionAlarmMessage(sub->getName(), "打开隔膜阀快充", step);
					}
					else{
						step = 160;
					}
				}
				else{
					addSubsystemNotNormalAlarmMessage(step, sub->getName());
				}
			}
			break;
			case 160:
			{
				if (sub->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					if (sub->getExhaustVacuumValueReachesTheSetValue())//真空值达到大气值9W9
					{
						step = 161;
					}
					else{
						Sleep(20);
					}
				}
				else{
					addSubsystemNotNormalAlarmMessage(step, sub->getName());
				}
			}
			break;
			case 161:
			{
				if (sub->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					if (sub->getExhaustVacuumValueReachesTheSetValue())//真空值达到大气值9W9
					{
						step = 170;
					}
					else{
						Sleep(20);
					}
				}
				else{
					addSubsystemNotNormalAlarmMessage(step, sub->getName());
				}
			}
			break;
			case 170:
			{
				if (sub->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					auto cmd = sub->createCloseDiaphragmValveCommand(LoadLockValveOpening::LoadLock_Both);
					sub->startCommand(cmd);
					cmd->wait();
					if (cmd->hasError())
					{
						addCommandExecutionAlarmMessage(sub->getName(), "关闭隔膜阀", step);
					}
					else{
						step = 180;
					}
				}
				else{
					addSubsystemNotNormalAlarmMessage(step, sub->getName());
				}
			}
			break;
			case 180:
			{
				if (sub->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					step = 190;
				}
				else{
					addSubsystemNotNormalAlarmMessage(step, sub->getName());
				}
			}
			break;
			case 190:
			{
				if (sub->getExhaustVacuumValueReachesTheSetValue()-500)
				{
					step = 5210;
				}
				else{
					step = 150;
				}
			}
			break;
#pragma endregion
			case 5210:
			{
				ret = IKernelCommand::RunResult::RUN_OK;
				step = 10000;

			}
			break;
			case 10000:
			{
				loop = false;
			}
			break;
			default:
				break;
			}
			Sleep(10);
			int pass = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - time_clock).count();
			if (pass >= timeout && loop)
			{
				AlarmMessage::Ptr alarm(new AlarmMessage(1, 10000, Poco::format("破%s真空命令执行超时", sub->getName())));
				setAlarm(alarm);
				step = 10000;
				loop = false;
				break;
			}
			}
			if (ret == IKernelCommand::RunResult::RUN_OK)
			{
				logInform(sub->getName().c_str(), Poco::format("破%s真空命令执行完成", sub->getName()).c_str());
			}
			return ret;
		}

		void LoadLockAutoBreakVacuumCommand::addCommandExecutionAlarmMessage(const std::string subsytem_name, const std::string message, const int code_id){
			AlarmMessage::Ptr alarm(new AlarmMessage(1, code_id, Poco::format("子系统：%s %s执行失败", subsytem_name, message)));
			setAlarm(alarm);
		}
		void LoadLockAutoBreakVacuumCommand::addSubsystemNotNormalAlarmMessage(const int code_id, const std::string subsytem_name)
		{
			AlarmMessage::Ptr alarm(new AlarmMessage(1, code_id, Poco::format("子系统：%s 状态异常", subsytem_name)));
			setAlarm(alarm);
		}




}