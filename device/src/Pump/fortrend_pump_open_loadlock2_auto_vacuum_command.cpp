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

#include "Pump/fortrend_pump_open_loadlock2_auto_vacuum_command.h"
#include "Pump/fortrend_pump_subsystem.h"
#include "Pump/fortrend_pump_mechanical_open_command.h"

#include "LoadLock/fortrend_loadlock_subsystem.h"
#include "LoadLock/fortrend_loadlock_close_angle_valve_command.h"
#include "LoadLock/fortrend_loadlock_close_diaphragm_valve_command.h"
#include "LoadLock/fortrend_loadlock_cavity_close_height_vacuum_baffle_valve_command.h"
#include "LoadLock/fortrend_loadlock_cavity_close_inserting_plate_valve_command.h"
#include "LoadLock/fortrend_loadlock_open_angle_valve_command.h"
#include "LoadLock/fortrend_loadlock_cavity_open_height_vacuum_baffle_valve_command.h"
#include "LoadLock/fortrend_loadlock_cavity_open_inserting_plate_valve_command.h"


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
#include <chrono>

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif


namespace FC{

	class PumpOpenLoadLock2AutoVacuumCommandPrivate {

	public:
		PumpOpenLoadLock2AutoVacuumCommandPrivate(PumpOpenLoadLock2AutoVacuumCommand* p);

		PumpOpenLoadLock2AutoVacuumCommandPrivate* p;
		std::shared_ptr<FortrendPumpSubsystem> pump;
		std::shared_ptr<FortrendTMCavitySubsystem> tm;
		std::shared_ptr<FortrendLoadLockSubsystem> lk1;
		std::shared_ptr<FortrendLoadLockSubsystem> lk2;
		IKernelCommand::RunResult ret = IKernelCommand::RunResult::RUN_FAILD;

		std::chrono::steady_clock::time_point start_time;//开始时间点
		const  std::chrono::hours timeout = std::chrono::hours(1); //超时时间

		bool isColseBaffleValvetm = false;//tm挡板阀
		bool isColseAngleValvetm = false; //tm角阀
		bool isColseBaffleValvellb = false;
		bool isColseAngleValvellb = false;
		bool loop = true;
		int llb_loop_count = 0;
	};
	PumpOpenLoadLock2AutoVacuumCommandPrivate::PumpOpenLoadLock2AutoVacuumCommandPrivate(PumpOpenLoadLock2AutoVacuumCommand* p)
	{

	}
	/**
	* PumpOpenLoadLock2AutoVacuumCommand
	*/
	PumpOpenLoadLock2AutoVacuumCommand::PumpOpenLoadLock2AutoVacuumCommand(KeyencePlcSubSystemHelper* helper)
		:KeyencePlcCommandExecuter(helper),
		d(new PumpOpenLoadLock2AutoVacuumCommandPrivate(this)) {
		//setMessageName("OpenLoadLock2AutoVacuum");
		//setDescription("open loadlock2 auto cauum for pump");

		initializeStateHandlers();
	};


	bool PumpOpenLoadLock2AutoVacuumCommand::executeCommand(std::shared_ptr<IKernelSubSystem> subsystem, std::shared_ptr<IKernelCommand> cmd, int currentStep, const std::string errorMessage)
	{
		subsystem->startCommand(cmd);
		cmd->wait();
		if (cmd->hasError())
		{
			addCommandExecutionAlarmMessage(currentStep, subsystem->getName(), errorMessage);
			return false;
		}

		return true;
	}

	void PumpOpenLoadLock2AutoVacuumCommand::initializeStateHandlers()
	{

		/*
		loadLockB 抽真空：
		1.关闭casste门
		2.关闭tM传输腔门
		3.关闭快慢隔膜阀
		4.loadlockA腔体的角阀  handleStep1050  -->去除
		5.TM角阀是否关闭       handleStep1060  -->去除
		6.打开干泵
		7.打开loadLockB腔体的角阀（先慢后快） handleStep1310
		8.判断是否达到粗抽真空设定值，超时时间1小时，超时报警
		9.关闭loadlockB腔体的角阀
		10.最后结束，打印结束日志
		*/

		stateHandlers = std::unordered_map<int, StateHandler>{
			{1045,[this]() {return handleStep1045(); }},
			{1040,[this]() {return handleStep1040(); }},
			{1030,[this]() {return handleStep1030(); }},
			{1050,[this]() {return handleStep1050(); }},
			{1060,[this]() {return handleStep1060(); }},
			{10, [this]() { return handleStep10(); }},
			{1310, [this]() { return handleStep1310(); }},
			{1100,[this]() {return handleStep1100(); }},
			{1320,[this]() {return handleStep1320(); }},
			{5210,[this]() {return handleStep5210(); }},
			{10000, [this]() { return handleStep10000(); }}
		};
	}

	int PumpOpenLoadLock2AutoVacuumCommand::handleStep10()
	{
		int step = 10;
		std::string errorMessage = "打开机械泵";
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
					step = 1310;
				}
			}
			else
			{
				//开始计时
				d->start_time = std::chrono::steady_clock::now();
				step = 1310;
			}
		}
		else
		{
			addSubsystemNotNormalAlarmMessage(step, d->pump->getName());
			step = 10000;
		}
		return step;
	}


	int PumpOpenLoadLock2AutoVacuumCommand::handleStep1030()
	{
		int step = 1030;
		std::string errorMessage = "关闭隔膜阀";
		if (d->lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
		{
			if (d->lk2->getFastDiaphragmValveOpend() || d->lk2->getSlowDiaphragmValveOpend())
			{
				//关闭隔膜阀
				auto cmd = d->lk2->createCloseDiaphragmValveCommand(LoadLockValveOpening::LoadLock_Both);
				if (!executeCommand(d->lk2, cmd, step, errorMessage)) {
					addCommandExecutionAlarmMessage(step, d->lk2->getName(), errorMessage);
					step = 10000;
				}
				else
				{
					step = 10;
				}
			}
			else
			{
				step = 10;
			}
		}
		else
		{
			addSubsystemNotNormalAlarmMessage(step, d->lk2->getName());
			step = 10000;
		}
		return step;
	}

	int PumpOpenLoadLock2AutoVacuumCommand::handleStep1040()
	{
		int step = 1040;
		std::string errorMessage = "关闭传输腔门阀";
		if (d->lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
		{
			if (d->lk2->getTMCavityDoorOpend())
			{
				//关闭传输腔门
				auto cmd = d->lk2->createCloseTMCavityDoorCommand();
				if (!executeCommand(d->lk2, cmd, step, errorMessage)) {
					addCommandExecutionAlarmMessage(step, d->lk2->getName(), errorMessage);
					step = 10000;
				}
				else
				{
					step = 1030;
				}
			}
			else {
				step = 1030;
			}
		}
		else
		{
			addSubsystemNotNormalAlarmMessage(step, d->lk1->getName());
			step = 10000;
		}
		return step;
	}

	int PumpOpenLoadLock2AutoVacuumCommand::handleStep1045()
	{
		int step = 1045;
		std::string errorMessage = "关闭晶圆盒门";
		if (d->lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
		{
			if (d->lk2->getCassetteDoorOpend())
			{
				//关闭晶圆盒门
				auto cmd = d->lk2->createCloseCassetteDoorCommand();
				if (!executeCommand(d->lk2, cmd, step, errorMessage))
				{
					addCommandExecutionAlarmMessage(step, d->lk2->getName(), errorMessage);
					step = 10000;
				}
				else
				{
					step = 1040;
				}
			}
			else {
				step = 1040;
			}
		}
		else
		{
			addSubsystemNotNormalAlarmMessage(step, d->lk2->getName());
			step = 10000;
		}
		return step;
	}

	int PumpOpenLoadLock2AutoVacuumCommand::handleStep1050()
	{
		int step = 1050;
		std::string errorMessage = "关闭TM腔的角阀";
		if (d->tm->getState() == IKernelSubSystem::State::SUB_NORMAL)
		{
			if (d->tm->getAngleValveOpend())
			{
				auto cmd = d->tm->createCloseAngleValveCommand();

				if (!executeCommand(d->tm, cmd, step, errorMessage))
				{
					addCommandExecutionAlarmMessage(step, d->tm->getName(), errorMessage);
					step = 10000;
				}
				else {

					d->isColseAngleValvetm = true;
					step = 1060;
				}
			}
			else {
				step = 1060;
			}
		}
		else
		{
			addSubsystemNotNormalAlarmMessage(step, d->tm->getName());
			step = 10000;
		}
		return step;
	}

	int PumpOpenLoadLock2AutoVacuumCommand::handleStep1060()
	{
		int step = 1060;
		std::string errorMessage = "关闭loadLock1腔体的角阀";

		if (d->lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
		{
			if (d->lk1->getAngleValveOpend())
			{
				auto cmd = d->lk1->createCloseAngleValveCommand();
				if (!executeCommand(d->lk1, cmd, step, errorMessage))
				{
					addCommandExecutionAlarmMessage(step, d->lk1->getName(), errorMessage);
					step = 10000;
				}
				else
				{
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
			addSubsystemNotNormalAlarmMessage(step, d->lk1->getName());
			step = 10000;
		}

		return step;
	}


	int PumpOpenLoadLock2AutoVacuumCommand::handleStep1100()
	{
		int step = 1100;
		std::string errorMessage = "干泵粗抽超时";

		auto now_time = std::chrono::steady_clock::now();
		auto elapsed = now_time - d->start_time;

		std::this_thread::sleep_for(std::chrono::seconds(1));
		auto  delay_time = d->lk2->getVacuumPumpingDelayTime();
		//是否达到真空上限值
		if (d->lk2->getVacuumValueUpperLimitReachesTheSetValue())
		{
			std::this_thread::sleep_for(std::chrono::seconds(int(delay_time)));
			step = 1320;
		}
		else
		{
			Sleep(100);

			if (elapsed >= d->timeout)
			{
				//超时
				d->llb_loop_count = 0;
				logInform(d->pump->getName().c_str(), Poco::format("%s:  等待loadlockB腔体压力小于1Pa,当前压力：%f",
					getName(), d->lk2->getVacuumValue()).c_str());

				addCommandExecutionAlarmMessage(step, d->lk2->getName(), errorMessage);
				step = 1320;//退出
			}
			else
			{
				auto remaining = d->timeout - elapsed;
				auto sec = std::chrono::duration_cast<std::chrono::seconds>(remaining).count();
				//继续当前函数
				step = 1100;
			}
		}
		return step;
	}

	int PumpOpenLoadLock2AutoVacuumCommand::handleStep1310()
	{
		int step = 1310;
		std::string errorMessage = "打开角阀";
		if (d->lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
		{
			if (!d->lk2->getAngleValveOpend())
			{
				auto cmd = d->lk2->createOpenAngleValveCommand();
				d->lk2->startCommand(cmd);
				cmd->wait();
				if (cmd->hasError())
				{
					addCommandExecutionAlarmMessage(step, d->lk2->getName(), "打开角阀");
					step = 10000;
				}
				else
				{
					step = 1100;
				}
			}
			else
			{
				step = 1100;
			}
		}
		else
		{
			addSubsystemNotNormalAlarmMessage(step, d->lk2->getName());
			step = 10000;
		}
		return step;
	}

	int PumpOpenLoadLock2AutoVacuumCommand::handleStep1320()
	{

		int step = 1320;
		std::string errorMessage = "关闭角阀";
		if (d->lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
		{
			if (d->lk2->getAngleValveOpend())
			{
				auto cmd = d->lk2->createCloseAngleValveCommand();
				d->lk2->startCommand(cmd);
				cmd->wait();
				if (cmd->hasError())
				{
					addCommandExecutionAlarmMessage(step, d->lk2->getName(), "关闭角阀");
					step = 10000;
				}
				else
				{
					step = 5210;
				}
			}
			else
			{
				step = 5210; //关闭状态
			}
		}
		else
		{
			addSubsystemNotNormalAlarmMessage(step, d->lk2->getName());
			step = 10000;
		}
		return step;
	}

	int PumpOpenLoadLock2AutoVacuumCommand::handleStep5210()
	{
		d->ret = IKernelCommand::RunResult::RUN_OK;
		int step = 10000;
		return step;
	}

	int PumpOpenLoadLock2AutoVacuumCommand::handleStep10000()
	{
		logInform(d->lk2->getName().c_str(), "抽真空循环结束");
		d->loop = false;
		return -1;
	}

	/**
	* return true if success else false.
	*/
	PumpOpenLoadLock2AutoVacuumCommand::RunResult PumpOpenLoadLock2AutoVacuumCommand::onRun() throw(KernelException){
		d->pump = getSubsystem()->getKernel()->getKernelModule<FortrendPumpSubsystem>("PUMP");
		//
		if (!d->pump){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "子系统类型错误", this);
		}
		d->tm = getSubsystem()->getKernel()->getKernelModule<FortrendTMCavitySubsystem>("TM");
		d->lk1 = getSubsystem()->getKernel()->getKernelModule<FortrendLoadLockSubsystem>("LLA");
		d->lk2 = getSubsystem()->getKernel()->getKernelModule<FortrendLoadLockSubsystem>("LLB");
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

		/*if (lk2->getCassetteDoorOpend())
		{
		throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_DOOR_EXCEPTION, Poco::format("子系统: %s 放晶圆盒的门已打开（逻辑错误）", lk2->getName()), this);
		}
		if (lk2->getTMCavityDoorOpend())
		{
		throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_DOOR_EXCEPTION, Poco::format("子系统: %s 传输腔门阀已打开（逻辑错误）", lk2->getName()), this);
		}*/

		//get command configure
		std::shared_ptr<KernelConfiguration> command_config = d->pump->getConfigure()->createView(getName());
		//fill params
		int timeout = command_config->getInt("timeout", 2 * 60 * 60);
		timeout = timeout * 1000;
		if (timeout < 10){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_DATA_OUTOF_RANGE, Poco::format("超时: 打开%s 真空命令超时参数错误", d->pump->getName()), this);
		}
		logInform(d->pump->getName().c_str(), Poco::format("打开%s真空命令开始", d->lk2->getName()).c_str());

		int robot_auto_step = 1045;
		std::chrono::system_clock::time_point time_clock = std::chrono::system_clock::now();   //抽真空计时

		while (d->loop)
		{

			if (d->pump->getProcessAbort()) {
				d->pump->setProcessAbort(false);
				logInform(d->pump->getName().c_str(), Poco::format("打开%s真空命令执行终止", d->lk2->getName()).c_str());
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "命令终止", this);
				return IKernelCommand::RunResult::RUN_OK;
			};

			auto it = stateHandlers.find(robot_auto_step);
			if (it == stateHandlers.end())
			{
				d->ret = IKernelCommand::RunResult::RUN_FAILD;
				logError(d->pump->getName().c_str(), Poco::format("未知的状态码：%s", std::to_string(robot_auto_step)).c_str());
				AlarmMessage::Ptr alarm(new AlarmMessage(1, 10001, Poco::format("打开%s真空命令,执行到未知的状态码,逻辑错误", d->lk1->getName())));
				setAlarm(alarm);
				break;
			}

			int nextState = it->second();//执行回调函数
			if (nextState == -1)
			{
				//结束
				logInform(d->pump->getName().c_str(), Poco::format("打开%s真空命令执行完成", d->lk2->getName()).c_str());
				break;
			}

			robot_auto_step = nextState;
			Sleep(100);
			int pass = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - time_clock).count();
			if (pass >= timeout && d->loop)
			{
				AlarmMessage::Ptr alarm(new AlarmMessage(1, 10000, Poco::format("打开%s真空命令执行超时", d->lk2->getName())));
				setAlarm(alarm);
				robot_auto_step = 10000;
				d->loop = false;
				break;
			}
		}
			
		if (d->ret == IKernelCommand::RunResult::RUN_OK)
		{
			logInform(d->pump->getName().c_str(), Poco::format("打开%s真空命令执行完成", d->lk2->getName()).c_str());
		}
		return d->ret;
	}

		void PumpOpenLoadLock2AutoVacuumCommand::addCommandExecutionAlarmMessage(const int code_id, const std::string subsytem_name, const std::string message){
			AlarmMessage::Ptr alarm(new AlarmMessage(1, code_id, Poco::format("子系统：%s %s执行失败", subsytem_name, message)));
			setAlarm(alarm);
		}
		void PumpOpenLoadLock2AutoVacuumCommand::addSubsystemNotNormalAlarmMessage(const int code_id, const std::string subsytem_name)
		{
			AlarmMessage::Ptr alarm(new AlarmMessage(1, code_id, Poco::format("子系统：%s 状态异常", subsytem_name)));
			setAlarm(alarm);
		}






}