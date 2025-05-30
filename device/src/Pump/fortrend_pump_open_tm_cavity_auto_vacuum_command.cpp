/**
* @file            fortrend_pump_open_loadlock1_auto_vacuum_command.h
* @brief           open loadlock1 auto command for Pump
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

#include "LoadLock/fortrend_loadlock_subsystem.h"
#include "LoadLock/fortrend_loadlock_close_angle_valve_command.h"
#include "LoadLock/fortrend_loadlock_cavity_close_height_vacuum_baffle_valve_command.h"

#include "Pump/fortrend_pump_open_tm_cavity_auto_vacuum_command.h"
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
	* PumpOpenTMCavityAutoVacuumCommand
	*/
	PumpOpenTMCavityAutoVacuumCommand::PumpOpenTMCavityAutoVacuumCommand(KeyencePlcSubSystemHelper* helper)
		:KeyencePlcCommandExecuter(helper){
		//setMessageName("OpenTMCavityAutoVacuum");
		//setDescription("open tm cavity auto cauum for pump");
	};


	/**
	* return true if success else false.
	*/
	PumpOpenTMCavityAutoVacuumCommand::RunResult PumpOpenTMCavityAutoVacuumCommand::onRun() throw(KernelException){
		std::shared_ptr<FortrendPumpSubsystem> pump = getSubsystem()->getKernel()->getKernelModule<FortrendPumpSubsystem>("PUMP");
		//
		if (!pump){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "子系统类型错误", this);
		}
		std::shared_ptr<FortrendTMCavitySubsystem> tm = getSubsystem()->getKernel()->getKernelModule<FortrendTMCavitySubsystem>("TM");
		std::shared_ptr<FortrendLoadLockSubsystem> lk1 = getSubsystem()->getKernel()->getKernelModule<FortrendLoadLockSubsystem>("LLA");
		std::shared_ptr<FortrendLoadLockSubsystem> lk2 = getSubsystem()->getKernel()->getKernelModule<FortrendLoadLockSubsystem>("LLB");
		if (!tm)
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "TM子系统类型错误", this);
		}
		if (tm->getState() != IKernelSubSystem::State::SUB_NORMAL)
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_DOOR_EXCEPTION, Poco::format("子系统: %s 不在正常状态", tm->getName()), this);
		}
		if (!lk1)
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "LoadLock1子系统类型错误", this);
		}
		if (lk1->getState() != IKernelSubSystem::State::SUB_NORMAL)
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_DOOR_EXCEPTION, Poco::format("子系统: %s 不在正常状态", lk1->getName()), this);
		}
		if (!lk2)
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "LoadLock2子系统类型错误", this);
		}
		if (lk2->getState() != IKernelSubSystem::State::SUB_NORMAL)
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_DOOR_EXCEPTION, Poco::format("子系统: %s 不在正常状态", lk2->getName()), this);
		}

		std::shared_ptr<KernelConfiguration> command_config = pump->getConfigure()->createView(getName());
		//fill params
		int timeout = command_config->getInt("timeout", 2 * 60 * 60);
		timeout = timeout * 1000;
		if (timeout < 10){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_DATA_OUTOF_RANGE, Poco::format("超时: 打开%s 真空命令超时参数错误", pump->getName()), this);
		}
		
		bool isColseBaffleValvella = false;
		bool isColseBaffleValvellb = false;
		bool isColseAngleValvella = false;
		bool isColseAngleValvellb = false;
		IKernelCommand::RunResult ret = IKernelCommand::RunResult::RUN_FAILD;
		logInform(pump->getName().c_str(), Poco::format("打开%s真空命令开始", tm->getName()).c_str());
		bool loop = true;
		int tm_loop_count = 0;
		int step = 10;
		std::chrono::system_clock::time_point time_clock = std::chrono::system_clock::now();   //抽真空计时
		while (loop)
		{
			switch (step)
			{
			case 10:
			{
				if (pump->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					if (pump->getMechanicalPumpOpened() == false)
					{
						auto cmd_open_mechanical = pump->createMechanicalOpenCommand();
						pump->startCommand(cmd_open_mechanical);
						cmd_open_mechanical->wait();
						if (cmd_open_mechanical->hasError())
						{
							addCommandExecutionAlarmMessage(step, pump->getName(), "打开机械泵");
							step = 10000;
						}
						else
						{
							step = 100;
						}
					}
					else
					{
						step = 100;
					}
				}
				else
				{
					addSubsystemNotNormalAlarmMessage(step, pump->getName());
					step = 10000;
				}
			}
			break;
			case 100:
			{
				//分子泵没有达到转速或TM腔没有达到真空设定值
				if (tm->getTMCavityRoughVacuumReachesTheSetValue() == false || pump->getMolecularPumpReachSpeedTM() == false)// || 
				{
					step = 1000;
				}
				else if (tm->getFastDiaphragmValveOpend() || tm->getSlowDiaphragmValveOpend())//隔膜阀打开或PID打开 tm->getPIDOpend() || 
				{
					step = 1010;
				}
				else if (tm->getInsertingPlateValveOpend() == false)//插板阀关闭
				{
					step = 1300;
				}
				else
				{
					step = 5000;
				}

			}
			break;

			/* ----------------- 打开分子泵流程 ------------------------*/
			case 1000:
			{
				if (tm->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					//关闭插板阀
					if (tm->getInsertingPlateValveOpend())
					{
						auto cmd = tm->createCloseInsertingPlateValveCommand();
						tm->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							addCommandExecutionAlarmMessage(step, tm->getName(), "关闭插板阀");
							step = 10000;
						}
						else
						{
							step = 1010;
						}
					}
					else{
						step = 1010;
					}
				}
				else
				{
					addSubsystemNotNormalAlarmMessage(step, tm->getName());
					step = 10000;
				}
			}
			break;
			case 1010:
			{
				if (tm->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					if (tm->getPIDOpend())
					{
						auto cmd = tm->createClosePIDCommand();
						tm->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							addCommandExecutionAlarmMessage(step, tm->getName(), "关闭PID");
							step = 10000;
						}
						else{
							step = 1030;
						}
					}
					else{
						step = 1030;
					}
				}
				else
				{
					addSubsystemNotNormalAlarmMessage(step, tm->getName());
					step = 10000;
				}
			}
			break;
			case 1030:
			{
				if (tm->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					if (tm->getFastDiaphragmValveOpend() || tm->getSlowDiaphragmValveOpend())
					{
						//关闭隔膜阀
						auto cmd = tm->createCloseDiaphragmValveCommand(TMCavityValveOpening::TMCavity_Both);
						tm->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							addCommandExecutionAlarmMessage(step, tm->getName(), "关闭隔膜阀");
							step = 10000;
						}
						else
						{
							step = 1040;
						}
					}
					else{
						step = 1040;
					}
				}
				else
				{
					addSubsystemNotNormalAlarmMessage(step, tm->getName());
					step = 10000;
				}

			}
			break;
			case 1040:
			{
				if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					if (lk1->getTMCavityDoorOpend())
					{
						//关闭传输腔门
						auto cmd = lk1->createCloseTMCavityDoorCommand();
						lk1->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							addCommandExecutionAlarmMessage(step, lk1->getName(), "关闭传输腔门阀");
							step = 10000;
						}
						else
						{
							step = 1045;
						}
					}
					else{
						step = 1045;
					}
				}
				else
				{
					addSubsystemNotNormalAlarmMessage(step, lk1->getName());
					step = 10000;
				}

			}
			break;
			case 1045:
			{
				if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					if (lk2->getTMCavityDoorOpend())
					{
						//关闭传输腔门
						auto cmd = lk2->createCloseTMCavityDoorCommand();
						lk2->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							addCommandExecutionAlarmMessage(step, lk2->getName(), "关闭传输腔门阀");
							step = 10000;
						}
						else
						{
							step = 1050;
						}
					}
					else{
						step = 1050;
					}
				}
				else
				{
					addSubsystemNotNormalAlarmMessage(step, lk2->getName());
					step = 10000;
				}

			}
			break;
			case 1050:
			{
				if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					if (lk1->getAngleValveOpend() && tm->getTMCavityVacuumValue()>300)
					{
						auto cmd = lk1->createCloseAngleValveCommand();
						lk1->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							addCommandExecutionAlarmMessage(step, lk1->getName(), "关闭角阀");
							step = 10000;
						}
						else{
							isColseAngleValvella = true;
							step = 1055;
						}
					}
					else{
						step = 1055;
					}
				}
				else
				{
					addSubsystemNotNormalAlarmMessage(step, lk1->getName());
					step = 10000;
				}
			}
			break;
			case 1055:
			{
				if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					if (lk1->getHeightVacuumBaffleValveOpend() && tm->getTMCavityVacuumValue()>300)
					{
						auto cmd = lk1->createCloseHeightVacuumBaffleValveCommand();
						lk1->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							addCommandExecutionAlarmMessage(step, lk1->getName(), "关闭高真空挡板阀");
							step = 10000;
						}
						else{
							isColseBaffleValvella = true;
							step = 1060;
						}
					}
					else{
						step = 1060;
					}
				}
				else
				{
					addSubsystemNotNormalAlarmMessage(step, lk1->getName());
					step = 10000;
				}
			}
			break;
			case 1060:
			{
				if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					if (lk2->getAngleValveOpend() && tm->getTMCavityVacuumValue()>300)
					{
						auto cmd = lk2->createCloseAngleValveCommand();
						lk2->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							addCommandExecutionAlarmMessage(step, lk2->getName(), "关闭角阀");
							step = 10000;
						}
						else
						{
							isColseAngleValvellb = true;
							step = 1065;
						}
					}
					else{
						step = 1065;
					}
				}
				else
				{
					addSubsystemNotNormalAlarmMessage(step, lk2->getName());
					step = 10000;
				}
			}
			break;
			case 1065:
			{
				if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					if (lk2->getHeightVacuumBaffleValveOpend() && tm->getTMCavityVacuumValue()>300)
					{
						auto cmd = lk2->createCloseHeightVacuumBaffleValveCommand();
						lk2->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							addCommandExecutionAlarmMessage(step, lk2->getName(), "关闭高真空挡板阀");
							step = 10000;
						}
						else
						{
							isColseBaffleValvellb = true;
							step = 1100;
						}
					}
					else{
						step = 1100;
					}
				}
				else
				{
					addSubsystemNotNormalAlarmMessage(step, lk2->getName());
					step = 10000;
				}
			}
			break;

			case 1100:
			{
				//是否达到粗抽压力
				if (tm->getTMCavityRoughVacuumReachesTheSetValue() == false)
				{
					step = 1120;
				}
				else
				{
					step = 1300;
				}
			}
			break;
			case 1120:
			{
				//分子泵打开或者运行状态
				if (pump->getMolecularPumpOpenedTM() || (pump->getMolecularPumpRunningStateTM() == 1))
				{
					step = 1140;
				}
				else
				{
					step = 1140;
				}
			}
			break;
			case 1140:
			{
				if (tm->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					if (tm->getAngleValveOpend())
					{
						//关闭角阀
						auto cmd = tm->createCloseAngleValveCommand();
						tm->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							addCommandExecutionAlarmMessage(step, tm->getName(), "关闭角阀");
							step = 10000;
						}
						else{
							step = 1200;
						}
					}
					else{
						step = 1200;
					}
				}
				else
				{
					addSubsystemNotNormalAlarmMessage(step, tm->getName());
					step = 10000;
				}
			}
			break;
			case 1200:
			{
				if (tm->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					//打开高真空挡板阀
					if (tm->getHeightVacuumBaffleValveOpend() == false)
					{
						auto cmd = tm->createOpenHeightVacuumBaffleValveCommand();
						tm->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							addCommandExecutionAlarmMessage(step, tm->getName(), "打开高真空挡板阀");
							step = 10000;
						}
						else
						{
							step = 1210;
						}
					}
					else
					{
						step = 1210;
					}
				}
				else
				{
					addSubsystemNotNormalAlarmMessage(step, tm->getName());
					step = 10000;
				}
			}
			break;
			case 1210:
			{
				//达到粗抽压力
				if (tm->getTMCavityRoughVacuumReachesTheSetValue())
				{
					Sleep(1000);
					step = 1300;
				}
				else
				{
					Sleep(100);
					
				}
			}
			break;
			case 1300:
			{
				if (tm->getTMCavityRoughVacuumReachesTheSetValue(10)){
					if (isColseAngleValvella){
						auto cmd = lk1->createOpenAngleValveCommand();
						lk1->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							addCommandExecutionAlarmMessage(step, lk1->getName(), "重新打开角阀");
						}
						else{
							isColseAngleValvella = false;
						}

					}
					if (isColseAngleValvellb){
						auto cmd = lk2->createOpenAngleValveCommand();
						lk2->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							addCommandExecutionAlarmMessage(step, lk2->getName(), "重新打开角阀");
						}
						else{
							isColseAngleValvellb = false;
						}
					}
				}

				if (tm->getMoleculePipelineVacuumValue() < 5.0)
				{
					tm_loop_count = 0;
					Sleep(1000);
					step = 1305;
				}
				else
				{
					Sleep(100);
					++tm_loop_count;
					if (tm_loop_count > 30)
					{
						tm_loop_count = 0;
						logInform(pump->getName().c_str(), Poco::format("%s: 等待前级管路压力小于5Pa,当前压力：%f", getName(), tm->getMoleculePipelineVacuumValue()).c_str());
					}
				}
			}
			break;
			case 1305:
			{
				if (tm->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					//关闭高真空挡板阀
					if (tm->getHeightVacuumBaffleValveOpend())
					{
						auto cmd = tm->createCloseHeightVacuumBaffleValveCommand();
						tm->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							addCommandExecutionAlarmMessage(step, tm->getName(), "关闭高真空挡板阀");
							step = 10000;
						}
						else
						{
							step = 1310;
						}
					}
					else{
						step = 1310;
					}
				}
				else
				{
					addSubsystemNotNormalAlarmMessage(step, tm->getName());
					step = 10000;
				}
			}
			break;
			case 1310:
			{
				if (tm->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					if (tm->getAngleValveOpend() == false)
					{
						auto cmd = tm->createOpenAngleValveCommand();
						tm->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							addCommandExecutionAlarmMessage(step, tm->getName(), "打开角阀");
							step = 10000;
						}
						else
						{
							step = 1320;
							tm_loop_count = 0;
						}
					}
					else
					{
						step = 1320;
						tm_loop_count = 0;
					}
				}
				else
				{
					addSubsystemNotNormalAlarmMessage(step, tm->getName());
					step = 10000;
				}
			}
			break;
			case 1320:
			{
				if (tm->getAngleValveOpend() && tm->getMoleculePipelineVacuumValue() < 10.0)
				{
					if (pump->getMolecularPumpOpenedTM() == false)
					{
						if (pump->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
							//打开分子泵
							auto cmd = pump->createMolecularOpenCommand("TM");
							pump->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError())
							{
								addCommandExecutionAlarmMessage(step, pump->getName(), "打开分子泵");
								step = 10000;
							}
							else
							{
								step = 1330;
							}
						}
						else
						{
							addSubsystemNotNormalAlarmMessage(step, pump->getName());
							step = 10000;
						}
					}
					else
					{
						step = 1330;
					}
				}
				else
				{
					step = 1000;
				}


			}
			break;
			case 1330:
			{
				if (pump->getMolecularPumpReachSpeedTM())
				{
					step = 1340;
				}
				else
				{
					Sleep(1000);
				}
			}
			break;
			case 1340:
			{
				if (tm->getMoleculePipelineVacuumValue() < 5.0 && tm->getAngleValveOpend())
				{
					if (pump->getMolecularPumpReachSpeedTM())
					{
						step = 1355;

					}
					else
					{
						step = 1100;
					}
				}
				else
				{
					Sleep(100);
				}

			}
			break;
			
			case 1355:
			{
				/*if (isColseAngleValvella){
					auto cmd = lk1->createOpenAngleValveCommand();
					lk1->startCommand(cmd);
					cmd->wait();
					if (cmd->hasError())
					{
						addCommandExecutionAlarmMessage(step, lk1->getName(), "重新打开角阀");
					}
					else{
						isColseAngleValvella = false;
					}
					
				}
				if (isColseAngleValvellb){
					auto cmd = lk2->createOpenAngleValveCommand();
					lk2->startCommand(cmd);
					cmd->wait();
					if (cmd->hasError())
					{
						addCommandExecutionAlarmMessage(step, lk2->getName(), "重新打开角阀");
					}
					else{
						isColseAngleValvellb = false;
					}
				}*/
				step = 1360;
			}
			break;
			case 1360:
			{
				if (tm->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					if (tm->getInsertingPlateValveOpend() == false)
					{
						if (tm->getTMCavityRoughVacuumReachesTheSetValue(10)){
							//打开插板阀
							auto cmd = tm->createOpenInsertingPlateValveCommand();
							tm->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError())
							{
								if (!tm->getTMCavityRoughVacuumReachesTheSetValue(10)){
									auto cmd2 = pump->createResetCommand();
									pump->startCommand(cmd2);
									cmd2->wait();
									step = 1000;
								}
								else{
									addCommandExecutionAlarmMessage(step, tm->getName(), "打开插板阀");
									step = 10000;
								}
							}
							else{
								step = 1380;
							}
						}
						else{//没有达到粗抽压力值
							step = 1000;
						}
						
					}
					else{
						step = 1380;
					}
				}
				else
				{
					addSubsystemNotNormalAlarmMessage(step, tm->getName());
					step = 10000;
				}

			}
			break;
			case 1380:
			{
				step = 10;
			}
			break;

			case 5000:
			{
				if (tm->getTMCavityVacuumValueReachesTheSetValue())
				{
					Sleep(1000);
					step = 5210;
				}
				else
				{
					Sleep(100);
				}
			}
			break;
			case 5100:
			{
				//if (tm->getState() == IKernelSubSystem::State::SUB_NORMAL)
				//{
				//	if (tm->getInsertingPlateValveOpend())
				//	{
				//		//关闭插板阀
				//		auto cmd = tm->createCloseInsertingPlateValveCommand();
				//		tm->startCommand(cmd);
				//		cmd->wait();
				//		if (cmd->hasError())
				//		{
				//			addCommandExecutionAlarmMessage(step, tm->getName(), "关闭插板阀");
				//			step = 10000;
				//		}
				//		else{
				//			step = 5200;
				//		}
				//	}
				//	else{
				//		step = 5200;
				//	}
				//}
				//else
				//{
				//	addSubsystemNotNormalAlarmMessage(step, tm->getName());
				//	step = 10000;
				//}

			}
			break;
			case 5200:
			{
				//if (tm->getState() == IKernelSubSystem::State::SUB_NORMAL)
				//{
				//	if (tm->createCloseAngleValveCommand())
				//	{
				//		//关闭角阀
				//		auto cmd = tm->createCloseInsertingPlateValveCommand();
				//		tm->startCommand(cmd);
				//		cmd->wait();
				//		if (cmd->hasError())
				//		{
				//			addCommandExecutionAlarmMessage(step, tm->getName(), "关闭角阀");
				//			step = 10000;
				//		}
				//		else{
				//			step = 5210;
				//		}
				//	}
				//	else{
				//		step = 5210;
				//	}
				//}
				//else
				//{
				//	addSubsystemNotNormalAlarmMessage(step, tm->getName());
				//	step = 10000;
				//}

			}
			break;
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
				AlarmMessage::Ptr alarm(new AlarmMessage(1, 10000, Poco::format("打开%s真空命令执行超时", tm->getName())));
				setAlarm(alarm);
				step = 10000;
				loop = false;
				break;
			}
		}
		if (ret == IKernelCommand::RunResult::RUN_OK)
		{
			logInform(pump->getName().c_str(), Poco::format("打开%s真空命令执行完成", tm->getName()).c_str());
		}
		return ret;
	}

	void PumpOpenTMCavityAutoVacuumCommand::addCommandExecutionAlarmMessage(const int code_id, const std::string subsytem_name, const std::string message){
		AlarmMessage::Ptr alarm(new AlarmMessage(1, code_id, Poco::format("子系统：%s %s执行失败", subsytem_name, message)));
		setAlarm(alarm);
	}
	void PumpOpenTMCavityAutoVacuumCommand::addSubsystemNotNormalAlarmMessage(const int code_id, const std::string subsytem_name)
	{
		AlarmMessage::Ptr alarm(new AlarmMessage(1, code_id, Poco::format("子系统：%s 状态异常", subsytem_name)));
		setAlarm(alarm);
	}



}