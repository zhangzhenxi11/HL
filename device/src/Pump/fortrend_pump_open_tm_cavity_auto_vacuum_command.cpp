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

#include "PMCavity/fortrend_pm_cavity_subsystem.h"
#include "PMCavity/fortrend_pm_cavity_open_tm_cavity_door_command.h"
#include "PMCavity/fortrend_pm_cavity_close_tm_cavity_door_command.h"
#include "PMCavity/fortrend_pm_cavity_defined.h"

#include <windows.h>
#include <vector>

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif



namespace FC{

	class PumpOpenTMCavityAutoVacuumCommandPrivate
	{
	public:
		PumpOpenTMCavityAutoVacuumCommandPrivate(PumpOpenTMCavityAutoVacuumCommand* p);

		PumpOpenTMCavityAutoVacuumCommandPrivate* p;
		
		std::shared_ptr<FortrendPumpSubsystem> pump;
		std::shared_ptr<FortrendTMCavitySubsystem> tm;
		std::shared_ptr<FortrendLoadLockSubsystem> lk1;
		std::shared_ptr<FortrendLoadLockSubsystem> lk2;
		std::shared_ptr<FortrendPMCavitySubsystem> pm1;
		std::shared_ptr<FortrendPMCavitySubsystem> pm2;
		std::shared_ptr<FortrendPMCavitySubsystem> pm3;
		std::shared_ptr<FortrendPMCavitySubsystem> pm4;

		std::vector<std::shared_ptr<FortrendPMCavitySubsystem>> Pm_list;

		IKernelCommand::RunResult ret = IKernelCommand::RunResult::RUN_FAILD;

		std::chrono::steady_clock::time_point start_time;//开始时间点
		const  std::chrono::hours timeout = std::chrono::hours(1); //超时时间

		bool isColseAngleValvetm = false; //tm角阀
		bool isColseAngleValvellb = false;
		bool isColseAngleValvella = false;
		bool loop = true;
		int tm_loop_count = 0;
	};


	PumpOpenTMCavityAutoVacuumCommandPrivate::PumpOpenTMCavityAutoVacuumCommandPrivate(PumpOpenTMCavityAutoVacuumCommand* p)
	{

	}


	/**
	* PumpOpenTMCavityAutoVacuumCommand
	*/
	PumpOpenTMCavityAutoVacuumCommand::PumpOpenTMCavityAutoVacuumCommand(KeyencePlcSubSystemHelper* helper)
		:KeyencePlcCommandExecuter(helper),d(new PumpOpenTMCavityAutoVacuumCommandPrivate(this)){
		initializeStateHandlers();
	};


	/**
	* return true if success else false.
	*/
	PumpOpenTMCavityAutoVacuumCommand::RunResult PumpOpenTMCavityAutoVacuumCommand::onRun() throw(KernelException){
		d->pump = getSubsystem()->getKernel()->getKernelModule<FortrendPumpSubsystem>("PUMP");
		//
		if (!d->pump){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "子系统类型错误", this);
		}
		d->tm = getSubsystem()->getKernel()->getKernelModule<FortrendTMCavitySubsystem>("TM");
		d->lk1 = getSubsystem()->getKernel()->getKernelModule<FortrendLoadLockSubsystem>("LLA");
		d->lk2 = getSubsystem()->getKernel()->getKernelModule<FortrendLoadLockSubsystem>("LLB");

		d->pm1 = getSubsystem()->getKernel()->getKernelModule<FortrendPMCavitySubsystem>("PM1");
		d->pm2 = getSubsystem()->getKernel()->getKernelModule<FortrendPMCavitySubsystem>("PM2");
		d->pm3 = getSubsystem()->getKernel()->getKernelModule<FortrendPMCavitySubsystem>("PM3");
		d->pm4 = getSubsystem()->getKernel()->getKernelModule<FortrendPMCavitySubsystem>("PM4");
		d->Pm_list.push_back(d->pm1);
		d->Pm_list.push_back(d->pm2);
		d->Pm_list.push_back(d->pm3);
		d->Pm_list.push_back(d->pm4);
		if (!d->tm)
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "TM子系统类型错误", this);
		}
		if (d->tm->getState() != IKernelSubSystem::State::SUB_NORMAL)
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_DOOR_EXCEPTION, Poco::format("子系统: %s 不在正常状态", d->tm->getName()), this);
		}
		if (!d->lk1)
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "LoadLock1子系统类型错误", this);
		}
		if (d->lk1->getState() != IKernelSubSystem::State::SUB_NORMAL)
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_DOOR_EXCEPTION, Poco::format("子系统: %s 不在正常状态", d->lk1->getName()), this);
		}
		if (!d->lk2)
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "LoadLock2子系统类型错误", this);
		}
		if (d->lk2->getState() != IKernelSubSystem::State::SUB_NORMAL)
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_DOOR_EXCEPTION, Poco::format("子系统: %s 不在正常状态", d->lk2->getName()), this);
		}

		std::shared_ptr<KernelConfiguration> command_config = d->pump->getConfigure()->createView(getName());
		//fill params
		int timeout = command_config->getInt("timeout", 2 * 60 * 60);
		timeout = timeout * 1000;
		if (timeout < 10){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_DATA_OUTOF_RANGE, Poco::format("超时: 打开%s 真空命令超时参数错误", d->pump->getName()), this);
		}
		
		logInform(d->pump->getName().c_str(), Poco::format("打开%s真空命令开始", d->tm->getName()).c_str());

		std::chrono::system_clock::time_point time_clock = std::chrono::system_clock::now();   //抽真空计时
		SystemState currentState = SystemState::CLOSE_PM_CAVITY_DOOR;
		while (d->loop)
		{
			if (d->pump->getProcessAbort()) {
				d->pump->setProcessAbort(false);

				logInform(d->pump->getName().c_str(), Poco::format("打开%s真空命令执行终止", d->pump->getName()).c_str());
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "命令终止", this);
				return IKernelCommand::RunResult::RUN_OK;
			};

			auto it = stateHandlers.find(currentState);
			if (it == stateHandlers.end())
			{
				logError(d->pump->getName().c_str(), Poco::format("未知的状态码：%d", int(currentState)).c_str());
				AlarmMessage::Ptr alarm(new AlarmMessage(1, 10000, Poco::format("破%s真空命令执行失败", d->pump->getName())));
				setAlarm(alarm);
				d->ret = IKernelCommand::RunResult::RUN_FAILD;
				break;
			}
			auto nextState = it->second();//执行回调函数
			currentState = nextState;
			Sleep(100);
			
			int pass = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - time_clock).count();
			if (pass >= timeout && d->loop)
			{
				AlarmMessage::Ptr alarm(new AlarmMessage(1, 10000, Poco::format("打开%s真空命令执行超时", d->tm->getName())));
				setAlarm(alarm);
				currentState = SystemState::CREATE_END;
				d->loop = false;
				break;
			}
		}
		if (d->ret == IKernelCommand::RunResult::RUN_OK)
		{
			logInform(d->pump->getName().c_str(), Poco::format("打开%s真空命令执行完成", d->tm->getName()).c_str());
		}
		return d->ret;
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

	void PumpOpenTMCavityAutoVacuumCommand::initializeStateHandlers()
	{
		//TM 抽真空：
		//1.pM工艺腔门（备用）
		//2.tm快慢隔膜阀，
		//3.loadlockb,loadlocka传输腔阀门
		//4.loadlockb,loadlocka角阀是否关闭
		//5.打开干泵
		//6.打开TM腔体的角阀（先慢后快）
		//7.判断是否达到粗抽真空设定值，超时时间1小时，超时报警
		//8.最后结束，打印结束日志

		stateHandlers = std::unordered_map<SystemState, StateHandler>{
			{SystemState::CLOSE_PM_CAVITY_DOOR,[this]() {return handleStepClosePmDoor(); }},
			{SystemState::CLOSE_TM_DIAPHRAGM_VALVE,[this]() {return handleStepCLoseDiaphragmValve(); }},
			{SystemState::CLOSE_LLA_TM_CAVITY_DOOR,[this]() {return handleStepCloseLlaTmDoor(); }},
			{SystemState::CLOSE_LLB_TM_CAVITY_DOOR,[this]() {return handleStepCloseLlbTmDoor(); }},
			{SystemState::CLOSE_LLA_ANGLE_VALVE,[this]() {return handleStepCloseLlaAngleValve(); }},
			{SystemState::CLOSE_LLB_ANGLE_VALVE,[this]() {return handleStepCloseLlbAngleValve(); }},
			{SystemState::OPEN_MECHANICAL_PUMP,[this]() {return handleStepOpenMechanicalPump(); }},
			{SystemState::OPEN_TM_ANGLE_VALVE,[this]() {return handleStepOpenAngleValve(); }},
			{SystemState::JUDGE_COARSE_SUCTION_PRESSURE,[this]() {return handleStepCoarseSuctionPressure(); }},
			{SystemState::CREATE_END,[this]() {return handleStepEND(); }}
		};

	}
	PumpOpenTMCavityAutoVacuumCommand::SystemState PumpOpenTMCavityAutoVacuumCommand::handleStepOpenMechanicalPump()
	{
		int step = 10;
		if (d->pump->getState() == IKernelSubSystem::State::SUB_NORMAL)
		{
			if (!d->pump->getMechanicalPumpOpened())
			{
				auto cmd_open_mechanical = d->pump->createMechanicalOpenCommand();
				d->pump->startCommand(cmd_open_mechanical);
				cmd_open_mechanical->wait();
				if (cmd_open_mechanical->hasError())
				{
					
					addCommandExecutionAlarmMessage(step, d->pump->getName(), "打开机械泵");
					step = 10000;
				}
				else
				{
					//开始计时
					d->start_time = std::chrono::steady_clock::now();
					step = 20;
				}
			}
			else
			{
				//开始计时
				d->start_time = std::chrono::steady_clock::now();
				step = 20;
			}
		}
		else
		{
			addSubsystemNotNormalAlarmMessage(step, d->pump->getName());
			step = 10000;
		}
		return SystemState(step);
	}

	PumpOpenTMCavityAutoVacuumCommand::SystemState PumpOpenTMCavityAutoVacuumCommand::handleStepOpenAngleValve()
	{
		int step = 20;
		std::string errorMessage = "打开TM腔的角阀";
		if (d->tm->getState() == IKernelSubSystem::State::SUB_NORMAL)
		{
			if (!d->tm->getAngleValveOpend())
			{
				auto cmd = d->tm->createOpenAngleValveCommand();
				d->tm->startCommand(cmd);
				cmd->wait();
				if (cmd->hasError())
				{
					addCommandExecutionAlarmMessage(step, d->tm->getName(), errorMessage);
					step = 10000;
				}
				else {
					step = 90;
				}
			}
			else {
				step = 90;
			}
		}
		else
		{
			addSubsystemNotNormalAlarmMessage(step, d->tm->getName());
			step = 10000;
		}
		return SystemState(step);
	}

	PumpOpenTMCavityAutoVacuumCommand::SystemState PumpOpenTMCavityAutoVacuumCommand::handleStepCLoseDiaphragmValve()
	{

		int step = 30;
		std::string errorMessage = "关闭TM腔的隔膜阀";
		if (d->tm->getState() == IKernelSubSystem::State::SUB_NORMAL)
		{
			if (d->tm->getFastDiaphragmValveOpend() || d->tm->getSlowDiaphragmValveOpend())
			{
				//关闭隔膜阀
				auto cmd = d->tm->createCloseDiaphragmValveCommand(TMCavityValveOpening::TMCavity_Both);
				d->tm->startCommand(cmd);
				cmd->wait();
				if (cmd->hasError())
				{
					addCommandExecutionAlarmMessage(step, d->tm->getName(), errorMessage);
					step = 10000;
				}
				else
				{
					step = 40; 
					//SystemState::CLOSE_LLA_TM_CAVITY_DOOR;
				}
			}
			else
			{
				step = 40;
			}
		}
		else
		{
			addSubsystemNotNormalAlarmMessage(step, d->tm->getName());
			step = 10000;
		}
		return SystemState(step);
	}

	PumpOpenTMCavityAutoVacuumCommand::SystemState PumpOpenTMCavityAutoVacuumCommand::handleStepCloseLlaTmDoor()
	{
		int step = 40;
		std::string errorMessage = "关闭LLA_TM传输腔门阀";
		if (d->lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
		{
			if (d->lk1->getTMCavityDoorOpend())
			{
				//关闭传输腔门
				auto cmd = d->lk1->createCloseTMCavityDoorCommand();
				d->lk1->startCommand(cmd);
				cmd->wait();
				if (cmd->hasError())
				{
					addCommandExecutionAlarmMessage(step, d->lk1->getName(), errorMessage);
					step = 10000;
				}
				else
				{
					step = 50;
				}
			}
			else {
				step = 50;
			}
		}
		else
		{
			addSubsystemNotNormalAlarmMessage(step, d->lk1->getName());
			step = 10000;
		}
		return SystemState(step);
	}

	PumpOpenTMCavityAutoVacuumCommand::SystemState PumpOpenTMCavityAutoVacuumCommand::handleStepCloseLlbTmDoor()
	{
		int step = 50;
		std::string errorMessage = "关闭LLB_TM传输腔门阀";
		if (d->lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
		{
			if (d->lk2->getTMCavityDoorOpend())
			{
				//关闭传输腔门
				auto cmd = d->lk2->createCloseTMCavityDoorCommand();
				d->lk2->startCommand(cmd);
				cmd->wait();
				if (cmd->hasError())
				{
					addCommandExecutionAlarmMessage(step, d->lk2->getName(), errorMessage);
					step = 10000;
				}
				else
				{
					step = 60;
				}
			}
			else {
				step = 60;
			}
		}
		else
		{
			addSubsystemNotNormalAlarmMessage(step, d->lk2->getName());
			step = 10000;
		}
		return SystemState(step);
	}
	PumpOpenTMCavityAutoVacuumCommand::SystemState PumpOpenTMCavityAutoVacuumCommand::handleStepCloseLlaAngleValve()
	{
		int step = 60;
		std::string errorMessage = "关闭LLA腔的角阀";
		if (d->lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
		{
			if (d->lk1->getAngleValveOpend())
			{
				auto cmd = d->lk1->createCloseAngleValveCommand();
				d->lk1->startCommand(cmd);
				cmd->wait();
				if (cmd->hasError())
				{
					addCommandExecutionAlarmMessage(step, d->lk1->getName(), errorMessage);
					step = 10000;
				}
				else {
					d->isColseAngleValvella = true;
					step = 70;
				}
			}
			else {
				step = 70;
			}
		}
		else
		{
			addSubsystemNotNormalAlarmMessage(step, d->tm->getName());
			step = 10000;
		}
		return SystemState(step);
	}
	PumpOpenTMCavityAutoVacuumCommand::SystemState PumpOpenTMCavityAutoVacuumCommand::handleStepCloseLlbAngleValve()
	{

		int step = 70;
		std::string errorMessage = "关闭LLB腔的角阀";
		if (d->lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
		{
			if (d->lk2->getAngleValveOpend())
			{
				auto cmd = d->lk2->createCloseAngleValveCommand();
				d->lk2->startCommand(cmd);
				cmd->wait();
				if (cmd->hasError())
				{
					addCommandExecutionAlarmMessage(step, d->lk2->getName(), errorMessage);
					step = 10000;
				}
				else {
					//d->isColseAngleValvellb = true;
					step = 10;
				}
			}
			else {
				step = 10;
			}
		}
		else
		{
			addSubsystemNotNormalAlarmMessage(step, d->tm->getName());
			step = 10000;
		}
		return SystemState(step);
	}

	
	PumpOpenTMCavityAutoVacuumCommand::SystemState PumpOpenTMCavityAutoVacuumCommand::handleStepClosePmDoor()
	{
		//PM1~PM4
		int step = 80;
		std::string errorMessage = "关闭PM1~PM4腔的门";
		//d->pm1
		for (int i = 0; i < 4; i++)
		{
			auto pmPtr = d->Pm_list.at(i);
			if (pmPtr != nullptr)
			{
				if (!(pmPtr->getState() == IKernelSubSystem::State::SUB_NORMAL))
				{
					logError(pmPtr.get()->getName().c_str(), "当前PM状态异常");
					addSubsystemNotNormalAlarmMessage(step, pmPtr.get()->getName());
					step = 10000;
					break;
				}
				if (d->tm->getPMCavityDoorOpend(i))
				{
					//关闭传输腔门
					auto cmd = pmPtr->createCloseTMCavityDoorCommand();
					pmPtr->startCommand(cmd);
					cmd->wait();
					if (cmd->hasError())
					{
						addCommandExecutionAlarmMessage(step, pmPtr->getName(), errorMessage);
						step = 10000;
						break;
					}
					else
					{
						step = 30; 
					}
				}
				else {
					step = 30;
				}
			}
			else
			{
				logError(pmPtr.get()->getName().c_str(), "获取当前PM不存在,请检查PM的是否实例化");
				step = 10000;
				break;
			}
		}
		return SystemState(step);
	}
	PumpOpenTMCavityAutoVacuumCommand::SystemState PumpOpenTMCavityAutoVacuumCommand::handleStepCoarseSuctionPressure()
	{
		int step = 90;
		std::string errorMessage = "干泵粗抽超时";
		//是否达到粗抽压力
		auto now_time = std::chrono::steady_clock::now();
		auto elapsed = now_time - d->start_time;
		std::this_thread::sleep_for(std::chrono::seconds(1));
		
		//int successCount = 0; //达到极限值次数

		if (d->tm->getTMCavityVacuumValueUpperLimitReachesTheSetValue()) 
		{
			step = 10000;
		}
		else
		{
			if (elapsed >= d->timeout)
			{
				//超时
				d->tm_loop_count = 0;
				logInform(d->pump->getName().c_str(), Poco::format("%s: 等待TM腔体压力达到1pa超时,当前压力：%f",getName(), d->tm->getTMCavityVacuumValue()).c_str());
				addCommandExecutionAlarmMessage(step, d->tm->getName(), errorMessage);
				step = 10000;
			}
			else
			{
				step = 90;//继续当前步骤
			}
		}
		return SystemState(step);
	}

	PumpOpenTMCavityAutoVacuumCommand::SystemState PumpOpenTMCavityAutoVacuumCommand::handleStepEND()
	{
		d->ret = IKernelCommand::RunResult::RUN_OK;
		logInform(d->lk1->getName().c_str(), "循环结束");
		d->loop = false;

		int step = 10;
		return SystemState(step);
	}

	

}