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

	class LoadLockAutoBreakVacuumCommandPrivate {
	
	public:
		LoadLockAutoBreakVacuumCommandPrivate(LoadLockAutoBreakVacuumCommand* p);

		LoadLockAutoBreakVacuumCommandPrivate* p;
		int loop_count = 0; //循环次数
		IKernelCommand::RunResult ret = IKernelCommand::RunResult::RUN_FAILD;
		FortrendLoadLockSubsystem* sub;
		//std::shared_ptr<FortrendLoadLockSubsystem> sub;
		bool loop = true;
		std::chrono::steady_clock::time_point start_time;//开始时间点
		std::chrono::steady_clock::time_point start_time_1;//开始时间点
		const  std::chrono::hours timeout = std::chrono::hours(1); //超时时间
	};


	/**
	* LoadLockAutoBreakVacuumCommand
	*/
	LoadLockAutoBreakVacuumCommand::LoadLockAutoBreakVacuumCommand(KeyencePlcSubSystemHelper* helper)
		:KeyencePlcCommandExecuter(helper),d(new LoadLockAutoBreakVacuumCommandPrivate(this)){

		initializeHLStateHandlers();
	};

	LoadLockAutoBreakVacuumCommandPrivate::LoadLockAutoBreakVacuumCommandPrivate(LoadLockAutoBreakVacuumCommand* p)
	{

	}

	void LoadLockAutoBreakVacuumCommand::initializeHLStateHandlers()
	{

		/*
			1.排气压力达到设定值
			2.判断LL-TMCavityDoor在关闭状态，cassete门也要关闭
			3.关闭loadlock角阀,防止气体进入干泵内
			4.打开隔膜阀慢充
			5.达到快充条件 (5000pa)
			6.打开隔膜阀快充 ,再次检测是否达到大气压
			7.关闭隔膜阀
			8.结束
		*/
		stateHandlers = std::unordered_map<SystemState, StateHandler>{
			{SystemState::EXHAUST_VACUUM_VALUE_REACHES_SETVALUE,[this]() {return handleStepReachesSetValue(); }},
			{SystemState::CLOSE_TMCAVITY_DOOR,[this]() {return handleStepCloseTmCavityDoor(); }},
			{SystemState::CLOSE_ANGLE_VALVE,[this]() {return handleStepAngleValve(); }},
			{SystemState::OPEN_DIAPHRAGM_VALVE_SLOW,[this]() {return handleStepDiaphragmValveSlow(); }},
			{SystemState::OPEN_DIAPHRAGM_VALVE_FAST,[this]() {return handleStepDiaphragmValveFast(); }},
			{SystemState::JUDGE_FAST_CHARGING_CONDITION,[this]() {return handleStepFastChargingCondition(); }},
			{SystemState::JUDGE_CHARGING_ATMOSPHERE_CONDITION,[this]() {return handleStepAtmosphereCondition(); }},
			{SystemState::CLOSE_DIAPHRAGM_VALVE,[this]() {return handleStepCloseDiaphragmValve(); }},

			{SystemState::CREATE_END,[this]() {return handleStepEnd(); }}
		};
	}

	/**
	* return true if success else false.
	*/
	LoadLockAutoBreakVacuumCommand::RunResult LoadLockAutoBreakVacuumCommand::onRun() throw(KernelException)
	{
		auto pump = getSubsystem()->getKernel()->getKernelModule<FortrendPumpSubsystem>("PUMP");
		if (!pump)
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "pump子系统类型错误", this);
		}

		d->sub = dynamic_cast<FortrendLoadLockSubsystem*>(getSubsystem());
		if (!d->sub)
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "LoadLock子系统类型错误", this);
		}
		if (d->sub->getState() != IKernelSubSystem::State::SUB_NORMAL)
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_DOOR_EXCEPTION, Poco::format("子系统: %s 不在正常状态", d->sub->getName()), this);
		}

		if (d->sub->getCassetteDoorOpend())
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_DOOR_EXCEPTION, Poco::format("子系统: %s 放晶圆盒的门已打开（逻辑错误）", d->sub->getName()), this);
		}
		if (d->sub->getTMCavityDoorOpend())
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_DOOR_EXCEPTION, Poco::format("子系统: %s 传输腔门阀已打开（逻辑错误）", d->sub->getName()), this);
		}

		//get command configure
		std::shared_ptr<KernelConfiguration> command_config = d->sub->getConfigure()->createView(getName());
		//fill params
		int timeout = command_config->getInt("timeout", 50 * 60 * 1000);
		if (timeout < 10) {
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_DATA_OUTOF_RANGE, Poco::format("超时: 破%s 真空命令超时参数错误", d->sub->getName()), this);
		}

		//IKernelCommand::RunResult ret = IKernelCommand::RunResult::RUN_FAILD;
		logInform(d->sub->getName().c_str(), Poco::format("破%s真空命令开始", d->sub->getName()).c_str());

		SystemState currentState = SystemState::EXHAUST_VACUUM_VALUE_REACHES_SETVALUE;
		std::chrono::system_clock::time_point time_clock = std::chrono::system_clock::now();   //抽真空计时
		while (d->loop)
		{
			if (pump->getProcessAbort()) 
			{
				pump->setProcessAbort(false);
				logInform(pump->getName().c_str(), Poco::format("破%s真空命令终止", pump->getName()).c_str());
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "命令终止", this);
				return IKernelCommand::RunResult::RUN_OK;
			};

			auto it = stateHandlers.find(currentState);
			if (it == stateHandlers.end()) {
				d->ret = IKernelCommand::RunResult::RUN_FAILD;
				logError(d->sub->getName().c_str(), Poco::format("未知的状态码：%d", int(currentState)).c_str());
				AlarmMessage::Ptr alarm(new AlarmMessage(1, 10000, Poco::format("破%s真空命令执行失败", d->sub->getName())));
				setAlarm(alarm);
				break;
			}
			auto nextState = it->second();//执行回调函数
			if (nextState == SystemState::CREATE_END)
			{
				d->ret == IKernelCommand::RunResult::RUN_OK;
				//结束
				logInform(d->sub->getName().c_str(), Poco::format("破%s真空命令执行完成", d->sub->getName()).c_str());
				return IKernelCommand::RunResult::RUN_OK;
				//break;
			}
			currentState = nextState;
			Sleep(100);


			int pass = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - time_clock).count();
			if (pass >= timeout && d->loop)
			{
				AlarmMessage::Ptr alarm(new AlarmMessage(1, 10000, Poco::format("破%s真空命令执行超时", d->sub->getName().c_str())));
				setAlarm(alarm);
				currentState = SystemState::CREATE_END;
				d->loop = false;
				break;
			}
		}

		AlarmMessage::Ptr alarm(new AlarmMessage(1, 10000, Poco::format("破%s真空命令执行超时", d->sub->getName().c_str())));
		setAlarm(alarm);
	
		//NOTE:返回d->ret ,且一定要报警，否则virtual void onAttributeChange(const IKernelCommand* cmd)虚函数，找不到报警信息报错！！
		return d->ret;
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

	LoadLockAutoBreakVacuumCommand::SystemState LoadLockAutoBreakVacuumCommand::handleStepReachesSetValue()
	{
		//int step = 10;
		SystemState step = SystemState::CREATE_END;

		//排气压力达到设定值
		if (!d->sub->getExhaustVacuumValueReachesTheSetValue())
		{
			step = SystemState::CLOSE_TMCAVITY_DOOR;
		}
		else
		{
			step = SystemState::CREATE_END;
		}
		return step;
	}

	LoadLockAutoBreakVacuumCommand::SystemState LoadLockAutoBreakVacuumCommand::handleStepCloseTmCavityDoor()
	{
		int step = 20;
		std::string errorMessage = "关闭LL_TM传输腔门阀";
		if (d->sub->getState() == IKernelSubSystem::State::SUB_NORMAL)
		{
			if (d->sub->getTMCavityDoorOpend())
			{
				//关闭传输腔门
				auto cmd = d->sub->createCloseTMCavityDoorCommand();
				d->sub->startCommand(cmd);
				cmd->wait();
				if (cmd->hasError())
				{
					addCommandExecutionAlarmMessage(d->sub->getName(), errorMessage ,step);
					step = 10000;
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
			addSubsystemNotNormalAlarmMessage(step, d->sub->getName());
			step = 10000;
		}
		return SystemState(step);
	}

	LoadLockAutoBreakVacuumCommand::SystemState LoadLockAutoBreakVacuumCommand::handleStepAngleValve()
	{
		int step = 30;
		std::string errorMessage = "关闭LL腔的角阀";
		if (d->sub->getState() == IKernelSubSystem::State::SUB_NORMAL)
		{
			if (d->sub->getAngleValveOpend())
			{
				auto cmd = d->sub->createCloseAngleValveCommand();
				d->sub->startCommand(cmd);
				cmd->wait();
				if (cmd->hasError())
				{
					addCommandExecutionAlarmMessage(d->sub->getName(), errorMessage, step);
					step = 10000;
				}
				else {
				
					step = 40;
				}
			}
			else {
				step = 40;
			}
		}
		else
		{
			addSubsystemNotNormalAlarmMessage(step, d->sub->getName());
			step = 10000;
		}
		return SystemState(step);
	}

	LoadLockAutoBreakVacuumCommand::SystemState LoadLockAutoBreakVacuumCommand::handleStepDiaphragmValveSlow()
	{
		int step = 40;
		if (d->sub->getState() == IKernelSubSystem::State::SUB_NORMAL)
		{
			auto cmd = d->sub->createOpenDiaphragmValveCommand(LoadLockValveOpening::LoadLock_Slow);
			d->sub->startCommand(cmd);
			cmd->wait();
			if (cmd->hasError())
			{
				addCommandExecutionAlarmMessage(d->sub->getName(), "打开隔膜阀慢充", step);
				step = 10000;
			}
			else {
				//开始计时
				d->start_time = std::chrono::steady_clock::now();
				step = 60;
			}
		}
		else {
			addSubsystemNotNormalAlarmMessage(step, d->sub->getName());
			step = 10000;
		}
		return SystemState(step);
	}

	LoadLockAutoBreakVacuumCommand::SystemState LoadLockAutoBreakVacuumCommand::handleStepDiaphragmValveFast()
	{
		int step = 50;
		if (d->sub->getState() == IKernelSubSystem::State::SUB_NORMAL)
		{
			auto cmd = d->sub->createOpenDiaphragmValveCommand(LoadLockValveOpening::LoadLock_Fast);
			d->sub->startCommand(cmd);
			cmd->wait();
			if (cmd->hasError())
			{
				addCommandExecutionAlarmMessage(d->sub->getName(), "打开隔膜阀快充", step);
				step = 10000;
			}
			else 
			{
				d->start_time_1 = std::chrono::steady_clock::now();
				step = 80;
			}
		}
		else {
			addSubsystemNotNormalAlarmMessage(step, d->sub->getName());
			step = 10000;
		}
		return SystemState(step);
	}

	LoadLockAutoBreakVacuumCommand::SystemState LoadLockAutoBreakVacuumCommand::handleStepFastChargingCondition()
	{
		int step = 60;

		auto now_time = std::chrono::steady_clock::now();
		auto elapsed = now_time - d->start_time;

		std::this_thread::sleep_for(std::chrono::seconds(1));

		//加上计数条件，否则会死循环
		if (d->sub->getQuickInflationValueReachesTheSetValue())//真空值大于99000pa
		{
			step = 50;
		}
		else {
			Sleep(100); //1s

			if (elapsed >= d->timeout)
			{
				d->loop_count = 0;
				addCommandExecutionAlarmMessage(d->sub->getName(), "检测是否达到快充隔膜阀真空值超时", step);
				step = 10000;
			}
			else
			{
				auto remaining = d->timeout - elapsed;
				auto sec = std::chrono::duration_cast<std::chrono::seconds>(remaining).count();
				step = 60;//继续当前函数
			}

		}
		return SystemState(step);
	}

	LoadLockAutoBreakVacuumCommand::SystemState LoadLockAutoBreakVacuumCommand::handleStepCloseDiaphragmValve()
	{
		int step = 70;
		if (d->sub->getState() == IKernelSubSystem::State::SUB_NORMAL)
		{
			auto cmd = d->sub->createCloseDiaphragmValveCommand(LoadLockValveOpening::LoadLock_Both);
			d->sub->startCommand(cmd);
			cmd->wait();
			if (cmd->hasError())
			{
				addCommandExecutionAlarmMessage(d->sub->getName(), "关闭隔膜阀", step);
				step = 10000;
			}
			else {
				step = 10000;
			}
		}
		else {
			addSubsystemNotNormalAlarmMessage(step, d->sub->getName());
			step = 10000;
		}
		return SystemState(step);
	}

	LoadLockAutoBreakVacuumCommand::SystemState LoadLockAutoBreakVacuumCommand::handleStepAtmosphereCondition()
	{
		int step = 80;
		auto now_time = std::chrono::steady_clock::now();
		auto elapsed = now_time - d->start_time_1;

		std::this_thread::sleep_for(std::chrono::seconds(1));

		//2025-8-13： getExhaustVacuumValueReachesTheSetValue()---> 改为getVacuumPressureGageState() == 1

		if (d->sub->getVacuumPressureGageState() == 1)//大气值
		{
			step = 70;
		}
		else {
			Sleep(100); //1s

			if (elapsed >= d->timeout)
			{
				addCommandExecutionAlarmMessage(d->sub->getName(), "检测是否达到大气值超时", step);
				step = 10000;
			}
			else
			{
				auto remaining = d->timeout - elapsed;
				auto sec = std::chrono::duration_cast<std::chrono::seconds>(remaining).count();
				step = 80;//继续当前函数
			}

		}
		return SystemState(step);
	}

	LoadLockAutoBreakVacuumCommand::SystemState LoadLockAutoBreakVacuumCommand::handleStepEnd()
	{
		d->ret = IKernelCommand::RunResult::RUN_OK;
		d->loop = false;
		//int step = 10000;
		return SystemState::CREATE_END;
	}
}