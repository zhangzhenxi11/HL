/**
* @file            fortrend_pump_open_tm_cavity_auto_break_vacuum_command.h
* @brief           open tm auto command for Pump
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

#include "Pump/fortrend_pump_open_tm_cavity_auto_break_vacuum_command.h"
#include "Pump/fortrend_pump_subsystem.h"
#include "Pump/fortrend_pump_mechanical_open_command.h"

#include "TMCavity/fortrend_tm_cavity_defined.h"
#include "TMCavity/fortrend_tm_cavity_subsystem.h"
#include "TMCavity/fortrend_tm_cavity_close_angle_valve_command.h"
#include "TMCavity/fortrend_tm_cavity_close_diaphragm_valve_command.h"
#include "TMCavity/fortrend_tm_cavity_open_angle_valve_command.h"

#include <windows.h>
#include <pipelinewidget.h>
#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif



namespace FC{

	class PumpOpenTMCavityAutoBreakVacuumCommandPrivate {

	public:
		PumpOpenTMCavityAutoBreakVacuumCommandPrivate(PumpOpenTMCavityAutoBreakVacuumCommand* p);

		PumpOpenTMCavityAutoBreakVacuumCommand* p;
		int loop_count = 0; //循环次数
		IKernelCommand::RunResult ret = IKernelCommand::RunResult::RUN_FAILD;
		FortrendLoadLockSubsystem*lk1 = nullptr;
		FortrendLoadLockSubsystem* lk2 = nullptr;
		FortrendTMCavitySubsystem* tm = nullptr;
		bool loop = true;
		std::chrono::steady_clock::time_point start_time;//开始时间点
		std::chrono::steady_clock::time_point start_time_1;//开始时间点
		const std::chrono::minutes timeout = std::chrono::minutes(30); //超时时间


	};



	/**
	* PumpOpenTMCavityAutoBreakVacuumCommand
	*/
	PumpOpenTMCavityAutoBreakVacuumCommand::PumpOpenTMCavityAutoBreakVacuumCommand(KeyencePlcSubSystemHelper* helper)
		:KeyencePlcCommandExecuter(helper), d(new PumpOpenTMCavityAutoBreakVacuumCommandPrivate(this)) {

		initializeHLStateHandlers();
	};

	PumpOpenTMCavityAutoBreakVacuumCommandPrivate::PumpOpenTMCavityAutoBreakVacuumCommandPrivate(PumpOpenTMCavityAutoBreakVacuumCommand* p)
	{

	}

	void PumpOpenTMCavityAutoBreakVacuumCommand::initializeHLStateHandlers()
	{
		/*
		1.排气压力达到设定值
		2.判断LL-TMCavityDoor在关闭状态
		3.关闭TM角阀,防止气体进入干泵内
		4.打开隔膜阀慢充
		5.检测是否达到大气压
		7.关闭隔膜阀
		8.结束
		*/

		stateHandlers = std::unordered_map<SystemState, StateHandler>{
				{SystemState::EXHAUST_VACUUM_VALUE_REACHES_SETVALUE,[this]() {return handleStepReachesSetValue(); }},
				{SystemState::CLOSE_TMCAVITY_DOOR,[this]() {return handleStepCloseTmCavityDoor(); }},
				{SystemState::CLOSE_ANGLE_VALVE,[this]() {return handleStepAngleValve(); }},
				{SystemState::OPEN_DIAPHRAGM_VALVE_SLOW,[this]() {return handleStepDiaphragmValveSlow(); }},
				{SystemState::JUDGE_CHARGING_ATMOSPHERE_CONDITION,[this]() {return handleStepAtmosphereCondition(); }},
				{SystemState::CLOSE_DIAPHRAGM_VALVE,[this]() {return handleStepCloseDiaphragmValve(); }},
				{SystemState::CREATE_END,[this]() {return handleStepEnd(); }}
		};
	}

	/**
	* return true if success else false.
	*/
	PumpOpenTMCavityAutoBreakVacuumCommand::RunResult PumpOpenTMCavityAutoBreakVacuumCommand::onRun() throw(KernelException){
		std::shared_ptr<FortrendPumpSubsystem> pump = getSubsystem()->getKernel()->getKernelModule<FortrendPumpSubsystem>("PUMP");
		//
		if (!pump){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "子系统类型错误.", this);
		}
		d->tm = getSubsystem()->getKernel()->getKernelModule<FortrendTMCavitySubsystem>("TM").get();

		//std::shared_ptr<FortrendTMCavitySubsystem> tm = getSubsystem()->getKernel()->getKernelModule<FortrendTMCavitySubsystem>("TM");
		//d->tm = tm.get();
		d->lk1 = getSubsystem()->getKernel()->getKernelModule<FortrendLoadLockSubsystem>("LLA").get();
		d->lk2 = getSubsystem()->getKernel()->getKernelModule<FortrendLoadLockSubsystem>("LLB").get();
		//std::shared_ptr<FortrendLoadLockSubsystem> lk1 = getSubsystem()->getKernel()->getKernelModule<FortrendLoadLockSubsystem>("LLA");
		//std::shared_ptr<FortrendLoadLockSubsystem> lk2 = getSubsystem()->getKernel()->getKernelModule<FortrendLoadLockSubsystem>("LLB");
		if (!d->tm)
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "TM子系统类型错误.", this);
		}
		if (d->tm->getState() != IKernelSubSystem::State::SUB_NORMAL)
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_DOOR_EXCEPTION, Poco::format("子系统: %s 不在正常状态.", d->tm->getName()), this);
		}
		if (!d->lk1)
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "LoadLock1子系统类型错误.", this);
		}
		if (d->lk1->getState() != IKernelSubSystem::State::SUB_NORMAL)
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_DOOR_EXCEPTION, Poco::format("子系统: %s 不在正常状态.", d->lk1->getName()), this);
		}
		if (!d->lk2)
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "LoadLock2子系统类型错误.", this);
		}
		if (d->lk2->getState() != IKernelSubSystem::State::SUB_NORMAL)
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_DOOR_EXCEPTION, Poco::format("子系统: %s 不在正常状态.", d->lk2->getName()), this);
		}

		std::shared_ptr<KernelConfiguration> command_config = pump->getConfigure()->createView(getName());
		//fill params
		int timeout = command_config->getInt("timeout_tm", 20 * 60 * 1000);
		if (timeout < 10){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_DATA_OUTOF_RANGE, Poco::format("超时: 打开%s 真空命令超时参数错误.", pump->getName()), this);
		}

		//IKernelCommand::RunResult ret = IKernelCommand::RunResult::RUN_FAILD;
		logInform(pump->getName().c_str(), Poco::format("破%s真空命令开始.", d->tm->getName()).c_str());

		SystemState currentState = SystemState::EXHAUST_VACUUM_VALUE_REACHES_SETVALUE;
		std::chrono::system_clock::time_point time_clock = std::chrono::system_clock::now();   //抽真空计时

		while (d->loop)
		{
			if (pump->getProcessAbort())
			{
				pump->setProcessAbort(false);
				logInform(pump->getName().c_str(), Poco::format("破%s真空命令终止.", pump->getName()).c_str());
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "命令终止", this);
				return IKernelCommand::RunResult::RUN_OK;
			};

			auto it = stateHandlers.find(currentState);
			if (it == stateHandlers.end()) {
				d->ret = IKernelCommand::RunResult::RUN_FAILD;
				logError(d->tm->getName().c_str(), Poco::format("未知的状态码：%d", int(currentState)).c_str());
				AlarmMessage::Ptr alarm(new AlarmMessage(1, 10000, Poco::format("破%s真空命令执行失败.", d->tm->getName())));
				setAlarm(alarm);
				break;
			}
			auto nextState = it->second();//执行回调函数

			if (nextState == SystemState::CREATE_END)
			{
				pipeLineWidget::getInstance().stop();
				d->ret == IKernelCommand::RunResult::RUN_OK;
				//结束
				logInform(d->tm->getName().c_str(), Poco::format("破%s真空命令执行完成.", d->tm->getName()).c_str());
				return IKernelCommand::RunResult::RUN_OK;
				//break;
			}
			currentState = nextState;
			Sleep(100);

			int pass = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - time_clock).count();
			if (pass >= timeout && d->loop)
			{
				AlarmMessage::Ptr alarm(new AlarmMessage(1, 10000, Poco::format("破%s真空命令执行超时.", d->tm->getName().c_str())));
				setAlarm(alarm);
				currentState = SystemState::CREATE_END;
				d->loop = false;
				break;
			}
		}
		AlarmMessage::Ptr alarm(new AlarmMessage(1, 10000, Poco::format("破%s真空命令执行超时.", d->tm->getName().c_str())));
		setAlarm(alarm);

		//NOTE:返回d->ret ,且一定要报警，否则virtual void onAttributeChange(const IKernelCommand* cmd)虚函数，找不到报警信息报错！！
		return d->ret;
	}


	PumpOpenTMCavityAutoBreakVacuumCommand::SystemState PumpOpenTMCavityAutoBreakVacuumCommand::handleStepReachesSetValue()
	{
		SystemState step = SystemState::EXHAUST_VACUUM_VALUE_REACHES_SETVALUE;

		//排气压力达到设定值
		if (d->tm->getTMCavityVacuumPressureGageState() !=1 )//大气值
		{
			step = SystemState::CLOSE_TMCAVITY_DOOR;
		}
		else
		{
			step = SystemState::CREATE_END;
		}
		return SystemState(step);
	}

	PumpOpenTMCavityAutoBreakVacuumCommand::SystemState PumpOpenTMCavityAutoBreakVacuumCommand::handleStepCloseTmCavityDoor()
	{
		SystemState step = SystemState::CLOSE_TMCAVITY_DOOR;

		std::string errorMessage = "关闭LL_TM传输腔门阀.";

		if (d->lk1->getState() == IKernelSubSystem::State::SUB_NORMAL && d->lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
		{
			if (d->lk1->getTMCavityDoorOpend())
			{
				//关闭传输腔门
				auto cmd = d->lk1->createCloseTMCavityDoorCommand();
				d->lk1->startCommand(cmd);
				cmd->wait();
				if (cmd->hasError())
				{
					addCommandExecutionAlarmMessage(int(step),d->lk1->getName(), errorMessage);
					step = SystemState::CREATE_END;
				}
				else
				{
					if (d->lk2->getTMCavityDoorOpend())
					{
						auto cmd = d->lk2->createCloseTMCavityDoorCommand();
						d->lk2->startCommand(cmd);
						cmd->wait();

						if (cmd->hasError())
						{
							addCommandExecutionAlarmMessage(int(step), d->lk2->getName(), errorMessage);
							step = SystemState::CREATE_END;
						}
					}
					else
					{
						step = SystemState::CLOSE_ANGLE_VALVE;
					}
				}
			}
			else {
				step = SystemState::CLOSE_ANGLE_VALVE;
			}
		}
		else
		{
			addSubsystemNotNormalAlarmMessage(int(step), d->lk1->getName());
			step = SystemState::CREATE_END;
		}
		return SystemState(step);
	}

	PumpOpenTMCavityAutoBreakVacuumCommand::SystemState
		PumpOpenTMCavityAutoBreakVacuumCommand::handleStepAngleValve()
	{
		SystemState step = SystemState::CLOSE_ANGLE_VALVE;
		std::string errorMessage = "关闭TM腔的角阀.";
		if (d->tm->getState() == IKernelSubSystem::State::SUB_NORMAL)
		{
			if (d->tm->getAngleValveOpend())
			{
				auto cmd = d->tm->createCloseAngleValveCommand();
				d->tm->startCommand(cmd);
				cmd->wait();
				if (cmd->hasError())
				{
					addCommandExecutionAlarmMessage(int(step),d->tm->getName(), errorMessage);
					step = SystemState::CREATE_END;
				}
				else {

					step = SystemState::OPEN_DIAPHRAGM_VALVE_SLOW;
				}
			}
			else {
				step = SystemState::OPEN_DIAPHRAGM_VALVE_SLOW;
			}
		}
		else
		{
			addSubsystemNotNormalAlarmMessage(int(step), d->tm->getName());
		}
		return SystemState(step);

	}

	PumpOpenTMCavityAutoBreakVacuumCommand::SystemState
		PumpOpenTMCavityAutoBreakVacuumCommand::handleStepDiaphragmValveSlow()
	{
		SystemState step = SystemState::OPEN_DIAPHRAGM_VALVE_SLOW;
		if (d->tm->getState() == IKernelSubSystem::State::SUB_NORMAL)
		{
			auto cmd = d->tm->createOpenDiaphragmValveCommand(TMCavityValveOpening::TMCavity_Slow);
			d->tm->startCommand(cmd);
			cmd->wait();
			if (cmd->hasError())
			{
				addCommandExecutionAlarmMessage(int(step),d->tm->getName(), "打开隔膜阀慢充.");
				step = SystemState::CREATE_END;
			}
			else {
				//开始计时
				d->start_time = std::chrono::steady_clock::now();
				step = SystemState::JUDGE_CHARGING_ATMOSPHERE_CONDITION;
			}
		}
		else {
			addSubsystemNotNormalAlarmMessage(int(step), d->tm->getName());
			step = SystemState::CREATE_END;
		}
		return SystemState(step);

	}
	PumpOpenTMCavityAutoBreakVacuumCommand::SystemState
		PumpOpenTMCavityAutoBreakVacuumCommand::handleStepCloseDiaphragmValve()
	{
		SystemState step = SystemState::CLOSE_DIAPHRAGM_VALVE;
		if (d->tm->getState() == IKernelSubSystem::State::SUB_NORMAL)
		{
			auto cmd = d->tm->createCloseDiaphragmValveCommand(TMCavityValveOpening::TMCavity_Slow);
			d->tm->startCommand(cmd);
			cmd->wait();
			if (cmd->hasError())
			{
				addCommandExecutionAlarmMessage(int(step),d->tm->getName(), "关闭隔膜阀.");
				step = SystemState::CREATE_END;;
			}
			else {
				step = SystemState::CREATE_END;;
			}
		}
		else {
			addSubsystemNotNormalAlarmMessage(int(step), d->tm->getName());
			step = SystemState::CREATE_END;;
		}
		return SystemState(step);

	}
	PumpOpenTMCavityAutoBreakVacuumCommand::SystemState
		PumpOpenTMCavityAutoBreakVacuumCommand::handleStepAtmosphereCondition()
	{
		SystemState step = SystemState::JUDGE_CHARGING_ATMOSPHERE_CONDITION;
		auto now_time = std::chrono::steady_clock::now();
		auto elapsed = now_time - d->start_time;

		std::this_thread::sleep_for(std::chrono::seconds(1));

		if (d->tm->getTMCavityVacuumPressureGageState() == 1)//大气值
		{
			step = SystemState::CLOSE_DIAPHRAGM_VALVE;
		}
		else if (elapsed >= d->timeout)
		{
			// 超时，报警并结束
			addCommandExecutionAlarmMessage(int(step), d->tm->getName(), "检测是否达到大气值超时.");
			step = SystemState::CREATE_END;
		}
		else
		{
			// 未达到大气压且未超时，继续检测
			step = SystemState::JUDGE_CHARGING_ATMOSPHERE_CONDITION;
		}
		return SystemState(step);

	}

	PumpOpenTMCavityAutoBreakVacuumCommand::SystemState
		PumpOpenTMCavityAutoBreakVacuumCommand::handleStepEnd()
	{
		d->ret = IKernelCommand::RunResult::RUN_OK;
		d->loop = false;
		return SystemState::CREATE_END;
	}

	void PumpOpenTMCavityAutoBreakVacuumCommand::addCommandExecutionAlarmMessage(const int code_id, const std::string subsytem_name, const std::string message){
		AlarmMessage::Ptr alarm(new AlarmMessage(1, code_id, Poco::format("子系统：%s %s执行失败.", subsytem_name, message)));
		setAlarm(alarm);
	}
	void PumpOpenTMCavityAutoBreakVacuumCommand::addSubsystemNotNormalAlarmMessage(const int code_id, const std::string subsytem_name)
	{
		AlarmMessage::Ptr alarm(new AlarmMessage(1, code_id, Poco::format("子系统：%s 状态异常.", subsytem_name)));
		setAlarm(alarm);
	}





}