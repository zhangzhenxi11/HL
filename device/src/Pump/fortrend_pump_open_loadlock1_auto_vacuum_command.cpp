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
#include "LoadLock/fortrend_loadlock_close_diaphragm_valve_command.h"
#include "LoadLock/fortrend_loadlock_cavity_close_height_vacuum_baffle_valve_command.h"
#include "LoadLock/fortrend_loadlock_cavity_close_inserting_plate_valve_command.h"
#include "LoadLock/fortrend_loadlock_open_angle_valve_command.h"
#include "LoadLock/fortrend_loadlock_cavity_open_height_vacuum_baffle_valve_command.h"
#include "LoadLock/fortrend_loadlock_cavity_open_inserting_plate_valve_command.h"

#include "Pump/fortrend_pump_subsystem.h"
#include "Pump/fortrend_pump_molecular_open_command.h"
#include "Pump/fortrend_pump_open_loadlock2_auto_vacuum_command.h"
#include "Pump/fortrend_pump_open_tm_cavity_auto_vacuum_command.h"
#include "Pump/fortrend_pump_mechanical_open_command.h"

#include "TMCavity/fortrend_tm_cavity_subsystem.h"
#include "TMCavity/fortrend_tm_cavity_defined.h"
#include "TMCavity/fortrend_tm_cavity_close_angle_valve_command.h"
#include "TMCavity/fortrend_tm_cavity_close_diaphragm_valve_command.h"
#include "TMCavity/fortrend_tm_cavity_close_height_vacuum_baffle_valve_command.h"
#include "TMCavity/fortrend_tm_cavity_close_inserting_plate_valve_command.h"
#include "TMCavity/fortrend_tm_cavity_open_angle_valve_command.h"
#include "TMCavity/fortrend_tm_cavity_open_height_vacuum_baffle_valve_command.h"
#include "TMCavity/fortrend_tm_cavity_open_inserting_plate_valve_command.h"

#include  "SunwayRobot/fortrend_sunwayrobot_subsystem.h"
#include <windows.h>

#include <chrono>

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif



namespace FC{

	class PumpOpenLoadLock1AutoVacuumCommandPrivate {
	public:

		PumpOpenLoadLock1AutoVacuumCommandPrivate(PumpOpenLoadLock1AutoVacuumCommand* p);

	public:
		PumpOpenLoadLock1AutoVacuumCommandPrivate* p;
		std::shared_ptr<FortrendPumpSubsystem> pump;
		std::shared_ptr<FortrendTMCavitySubsystem> tm;
		std::shared_ptr<FortrendLoadLockSubsystem> lk1;
		std::shared_ptr<FortrendLoadLockSubsystem> lk2;
		std::shared_ptr<FortrendSunwayRobotSubsystem> wtr;
		IKernelCommand::RunResult ret = IKernelCommand::RunResult::RUN_FAILD;

		std::chrono::steady_clock::time_point start_time;//开始时间点
		const  std::chrono::hours timeout = std::chrono::hours(1); //超时时间
	};
	PumpOpenLoadLock1AutoVacuumCommandPrivate::
	PumpOpenLoadLock1AutoVacuumCommandPrivate(PumpOpenLoadLock1AutoVacuumCommand* p)
	{
	}
	/**
	* PumpOpenLoadLock1AutoVacuumCommand
	*/
	PumpOpenLoadLock1AutoVacuumCommand::PumpOpenLoadLock1AutoVacuumCommand(KeyencePlcSubSystemHelper* helper)
		:KeyencePlcCommandExecuter(helper),
		d(new PumpOpenLoadLock1AutoVacuumCommandPrivate(this))
		{
			initializeHLStateHandlers();		
		};

	bool PumpOpenLoadLock1AutoVacuumCommand::executeCommand(std::shared_ptr<IKernelSubSystem> subsystem, 
		std::shared_ptr<IKernelCommand> cmd, int currentStep,const std::string errorMessage)
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

	bool PumpOpenLoadLock1AutoVacuumCommand::checkSubsystemsNormal(std::initializer_list<std::shared_ptr<IKernelSubSystem>> subsystems)
	{
		for (auto& sub : subsystems)
		{
			if (sub->getState() != IKernelSubSystem::State::SUB_NORMAL)
			{
				return false;
			}
		}
		return true;
	}

	void PumpOpenLoadLock1AutoVacuumCommand::initializeStateHandlers()
	{

		//Note:这里必须显式指定 std::unordered_map 的模板参数，否则报operate= 不明确错误！！！
		stateHandlers = std::unordered_map<int, StateHandler>{
		{10, [this]() { return handleStep10(); }},
		{100, [this]() { return handleStep100(); }},
		{1000, [this]() { return handleStep1000(); }},
		{1010, [this]() { return handleStep1010(); }},
		{1030 , [this]() { return handleStep1030(); }},
		{1040 , [this]() { return handleStep1040(); }},
		{1045 , [this]() { return handleStep1045(); }},
		{1050 , [this]() { return handleStep1050(); }},
		{1055 , [this]() { return handleStep1055(); }},
		{1060 , [this]() { return handleStep1060(); }},
		{1065 , [this]() { return handleStep1065(); }},
		{1100 , [this]() { return handleStep1100(); }},
		{1120 , [this]() { return handleStep1120(); }},
		{1140 , [this]() { return handleStep1140(); }},
		{1200 , [this]() { return handleStep1200(); }},
		{1210 , [this]() { return handleStep1210(); }},
		{1300 , [this]() { return handleStep1300(); }},
		{1305 , [this]() { return handleStep1305(); }},
		{1310 , [this]() { return handleStep1310(); }},
		{1320 , [this]() { return handleStep1320(); }},
		{1340 , [this]() { return handleStep1340(); }},
		{1345 , [this]() { return handleStep1345(); }},
		{1355 , [this]() { return handleStep1355(); }},
		{1360 , [this]() { return handleStep1360(); }},
		{1380 , [this]() { return handleStep1380(); }},
		{5000 , [this]() { return handleStep5000(); }},
		{5210 , [this]() { return handleStep5210(); }},
		{10000, [this]() { return handleStep10000(); }}
		};
	}

	void PumpOpenLoadLock1AutoVacuumCommand::initializeHLStateHandlers()
	{
		/*
		* 前提：例如loadLockA 抽真空：casste门，tM传输腔门，快慢隔膜阀，其他腔体的角阀是否关闭
		1.打开干泵							 handleStep10  
		2.打开对应的角阀						 handleStep1310：判断是否达到粗抽真空设定值，超时时间1小时，超时报警。最后结束
		3.关闭当前腔体的隔膜阀				 handleStep1030
		4.检查casste门和LL-TM腔体门是否关闭    handleStep1045 ,handleStep1040
		5.关闭其他腔体的角阀					 handleStep1050, handleStep1060
		6.抽到达标真空值结束                   handleStep1100  handleStep5210
		*/

		/*
		loadLockA 抽真空：
		1.关闭casste门，   handleStep1045
		2.关闭tm传输腔门， handleStep1040
		3.关闭快慢隔膜阀， handleStep1030
		4.TM角阀关闭,        handleStep1050 -->去除
		5.关闭loadlockb腔体的角阀 handleStep1060 -->去除
		6.打开干泵		 handleStep10
		7.打开loadLockA腔体的角阀（先慢后快）  handleStep1310
		8.(判断是否达到粗抽真空设定值)，抽到真空上限值（1pa）,超时时间1小时，超时报警  handleStep1100
		9. 关闭loadLockA角阀
		10.最后结束，打印结束日志  handleStep5210,handleStep10000
		*/

		stateHandlers = std::unordered_map<int, StateHandler>{
			{1045,  [this]() {return handleStep1045(); }},
			{1040,  [this]() {return handleStep1040(); }},
			{1030,  [this]() {return handleStep1030(); }},
			{1050,  [this]() {return handleStep1050(); }},
			{1060,  [this]() {return handleStep1060(); }},
			{10,    [this]() { return handleStep10(); }},
			{1310,  [this]() { return handleStep1310(); }},
			{1100,  [this]() {return handleStep1100(); }},
			{1350,  [this]() { return handleStep1350(); }},
			{5210,  [this]() {return handleStep5210(); }},
			{10000, [this]() { return handleStep10000(); }}
		};
	}

	/**
	* return true if success else false.
	*/
	PumpOpenLoadLock1AutoVacuumCommand::RunResult PumpOpenLoadLock1AutoVacuumCommand::onRun() throw(KernelException){
		d->pump = getSubsystem()->getKernel()->getKernelModule<FortrendPumpSubsystem>("PUMP");
		//
		if (!d->pump){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "子系统类型错误", this);
		}
		d->tm = getSubsystem()->getKernel()->getKernelModule<FortrendTMCavitySubsystem>("TM");
		d->lk1 = getSubsystem()->getKernel()->getKernelModule<FortrendLoadLockSubsystem>("LLA");
		d->lk2 = getSubsystem()->getKernel()->getKernelModule<FortrendLoadLockSubsystem>("LLB");
		d->wtr = getSubsystem()->getKernel()->getKernelModule<FortrendSunwayRobotSubsystem>("WTR");
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
		if (!d->wtr)
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "wtr子系统类型错误", this);
		}
		if (d->wtr->getState() != IKernelSubSystem::State::SUB_NORMAL)
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_DOOR_EXCEPTION, Poco::format("子系统: %s 不在正常状态", d->wtr->getName()), this);
		}

		/*if (d->lk1->getCassetteDoorOpend())
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_DOOR_EXCEPTION, Poco::format("子系统: %s 放晶圆盒的门已打开（逻辑错误）", d->lk1->getName()), this);
		}
		if (d->lk1->getTMCavityDoorOpend())
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_DOOR_EXCEPTION, Poco::format("子系统: %s 传输腔门阀已打开（逻辑错误）", d->lk1->getName()), this);
		}*/

		//get command configure
		std::shared_ptr<KernelConfiguration> command_config = d->pump->getConfigure()->createView(getName());
		//fill params
		int timeout = command_config->getInt("timeout", 2 * 60 * 60);
		timeout = timeout * 1000;

		if (timeout < 10){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_DATA_OUTOF_RANGE, Poco::format("超时: 打开%s 真空命令超时参数错误", d->pump->getName()), this);
		}

		logInform(d->pump->getName().c_str(), Poco::format("打开%s真空命令开始", d->lk1->getName()).c_str());
		//bool loop = true;
		////int lla_loop_count = 0;
		//int step = 10;
		std::chrono::system_clock::time_point time_clock = std::chrono::system_clock::now();   //抽真空计时
		
		int robot_auto_step = 1045;
		//查表调用状态处理函数
		while (loop)
		{
			if (d->pump->getProcessAbort()) {d->pump->setProcessAbort(false);
				
				logInform(d->pump->getName().c_str(), Poco::format("打开%s真空命令执行终止", d->lk1->getName()).c_str());
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
				logInform(d->pump->getName().c_str(), Poco::format("打开%s真空命令执行完成", d->lk1->getName()).c_str());
				break;
			}
			
			robot_auto_step = nextState;
			Sleep(100);

			int pass = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - time_clock).count();
			if (pass >= timeout && loop)
			{
				AlarmMessage::Ptr alarm(new AlarmMessage(1, 10000, Poco::format("打开%s真空命令执行超时", d->lk1->getName())));
				setAlarm(alarm);
				robot_auto_step = 10000;
				loop = false;
				break;
			}
		}

		if (d->ret == IKernelCommand::RunResult::RUN_OK)
		{
			logInform(d->pump->getName().c_str(), Poco::format("打开%s真空命令执行完成", d->lk1->getName()).c_str());
		}
		return d->ret;
	}

	void PumpOpenLoadLock1AutoVacuumCommand::addCommandExecutionAlarmMessage(const int code_id, const std::string subsytem_name, const std::string message){
		AlarmMessage::Ptr alarm(new AlarmMessage(1, code_id, Poco::format("子系统：%s %s执行失败", subsytem_name, message)));
		setAlarm(alarm);
	}
	void PumpOpenLoadLock1AutoVacuumCommand::addSubsystemNotNormalAlarmMessage(const int code_id, const std::string subsytem_name)
	{
		AlarmMessage::Ptr alarm(new AlarmMessage(1, code_id, Poco::format("子系统：%s 状态异常", subsytem_name)));
		setAlarm(alarm);
	}


	/*
		auto lp = kernel->getKernelModule<EFEMLPSubsystem>("ELP" + result.str(1));
		lp->handle(command);
	*/
	int PumpOpenLoadLock1AutoVacuumCommand::handleStep10()
	{
		int step = 10;
		std::string errorMessage = "打开机械泵";
		if (d->pump->getState() == IKernelSubSystem::State::SUB_NORMAL)
		{
			if (d->pump->getMechanicalPumpOpened() == false)
			{
				auto cmd_open_mechanical = d->pump->createMechanicalOpenCommand();
				if (!executeCommand(d->pump, cmd_open_mechanical, step, errorMessage))
				{
					addCommandExecutionAlarmMessage(step, d->pump->getName(), errorMessage);
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

	int PumpOpenLoadLock1AutoVacuumCommand::handleStep100()
	{
		int step = 100;
		//分子泵没有达到转速或腔没有达到粗抽设定值
		if (d->lk1->getLoadLockRoughVacuumReachesTheSetValue() == false || d->pump->getMolecularPumpReachSpeedLLA() == false)// || 
		{
			step = 1000;
		}
		else if (d->lk1->getFastDiaphragmValveOpend() || d->lk1->getSlowDiaphragmValveOpend())//隔膜阀打开或PID打开 d->lk1->getPIDOpend() || 
		{
			step = 1010;
		}
		else if (d->lk1->getInsertingPlateValveOpend() == false)//插板阀关闭
		{
			step = 1300;
		}
		else
		{
			step = 5000;
		}

		return step;
	}

	int PumpOpenLoadLock1AutoVacuumCommand::handleStep1000()
	{
		int step = 1000;
		std::string errorMessage = "关闭插板阀";
		if (d->lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
		{
			//关闭插板阀
			if (d->lk1->getInsertingPlateValveOpend())
			{
				auto cmd = d->lk1->createCloseInsertingPlateValveCommand();
				if(!executeCommand(d->lk1, cmd, step, errorMessage))
				{
					addCommandExecutionAlarmMessage(step, d->lk1->getName(), errorMessage);
					step = 10000;
				}
				else
				{
					step = 1010;
				}
			}
			else {
				step = 1010;
			}
		}
		else
		{
			addSubsystemNotNormalAlarmMessage(step, d->lk1->getName());
			step = 10000;
		}
		return step;
	}

	int PumpOpenLoadLock1AutoVacuumCommand::handleStep1010()
	{
		int step = 1030;
		return step;
	}

	int PumpOpenLoadLock1AutoVacuumCommand::handleStep1030()
	{
		int step = 1030;
		std::string errorMessage = "关闭隔膜阀";
		if (d->lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
		{
			if (d->lk1->getFastDiaphragmValveOpend() || d->lk1->getSlowDiaphragmValveOpend())
			{
				//关闭隔膜阀
				auto cmd = d->lk1->createCloseDiaphragmValveCommand(LoadLockValveOpening::LoadLock_Both);
				if (!executeCommand(d->lk1, cmd, step, errorMessage))
				{
					addCommandExecutionAlarmMessage(step, d->lk1->getName(), errorMessage);
					step = 10000;
				}
				else
				{
					step = 10; //1050-->10
				}
			}
			else
			{
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

	int PumpOpenLoadLock1AutoVacuumCommand::handleStep1040()
	{
		int step = 1040;
		std::string errorMessage = "关闭传输腔门阀";
		if (d->lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
		{
			if (d->lk1->getWtrOriginSafeSignal())
			{
				//得到安全信号后，最低延迟2s，才能关门阀，否则报Resources : LLA   has be lock by WTR.
				Sleep(3000);

				if (d->wtr->getState() == IKernelSubSystem::State::SUB_NORMAL ||
					d->wtr->getState() == IKernelSubSystem::State::SUB_IDEL)
				{
					if (d->lk1->getTMCavityDoorOpend())
					{
						//关闭传输腔门
						auto cmd = d->lk1->createCloseTMCavityDoorCommand();
						if (!executeCommand(d->lk1, cmd, step, errorMessage)) {
							addCommandExecutionAlarmMessage(step, d->lk1->getName(), errorMessage);
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
					step = 1040;
				}
			}
			else
			{
				logInform(d->lk1->getName().c_str(), Poco::format("等待%s中的机械手动作执行完成", d->lk1->getName()).c_str());
				step = 1040;
			}
		}
		else
		{
			addSubsystemNotNormalAlarmMessage(step, d->lk1->getName());
			step = 10000;
		}
		return step;
	}

	int PumpOpenLoadLock1AutoVacuumCommand::handleStep1045()
	{
		int step = 1045;
		std::string errorMessage = "关闭晶圆盒门";
		if (d->lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
		{
			if (d->lk1->getCassetteDoorOpend())
			{
				//关闭晶圆盒门
				auto cmd = d->lk1->createCloseCassetteDoorCommand();
				if (!executeCommand(d->lk1, cmd, step, errorMessage))
				{
					addCommandExecutionAlarmMessage(step, d->lk1->getName(), errorMessage);
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

	int PumpOpenLoadLock1AutoVacuumCommand::handleStep1050()
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

					isColseAngleValvetm = true;
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

	int PumpOpenLoadLock1AutoVacuumCommand::handleStep1055()
	{
		int step = 1055;
		std::string errorMessage = "关闭高真空挡板阀";

		if (d->tm->getState() == IKernelSubSystem::State::SUB_NORMAL)
		{
			if (d->tm->getHeightVacuumBaffleValveOpend() && d->lk1->getVacuumValue() > 300)
			{
				auto cmd = d->tm->createCloseHeightVacuumBaffleValveCommand();
				if (!executeCommand(d->tm, cmd, step, errorMessage))
				{
					addCommandExecutionAlarmMessage(step, d->tm->getName(), "关闭高真空挡板阀");
					step = 10000;
				}
				else {

					isColseBaffleValvetm = true;
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
		return 0;
	}

	int PumpOpenLoadLock1AutoVacuumCommand::handleStep1060()
	{
		int step = 1060;
		std::string errorMessage = "关闭loadLock2腔体的角阀";

		if (d->lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
		{
			if (d->lk2->getAngleValveOpend())
			{
				auto cmd = d->lk2->createCloseAngleValveCommand();
				if (!executeCommand(d->lk2, cmd, step, errorMessage)) 
				{
					addCommandExecutionAlarmMessage(step, d->lk2->getName(), errorMessage);
					step = 10000;
				}
				else
				{
					isColseAngleValvellb = true;
					step = 10;
				}
			}
			else {
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

	int PumpOpenLoadLock1AutoVacuumCommand::handleStep1065()
	{
		int step = 1065;
		std::string errorMessage = "关闭loadLock2高真空挡板阀";

		if (d->lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
		{
			if (d->lk2->getHeightVacuumBaffleValveOpend() && d->lk1->getVacuumValue() > 300)
			{
				auto cmd = d->lk2->createCloseHeightVacuumBaffleValveCommand();
				if (!executeCommand(d->lk2, cmd, step, errorMessage))
				{
					addCommandExecutionAlarmMessage(step, d->lk2->getName(), "关闭loadLock2高真空挡板阀");
					step = 10000;
				}
				else
				{
					isColseBaffleValvellb = true;
					step = 1100;
				}
			}
			else {
				step = 1100;
			}
		}
		else
		{
			addSubsystemNotNormalAlarmMessage(step, d->lk2->getName());
			step = 10000;
		}
		return 0;
	}

	int PumpOpenLoadLock1AutoVacuumCommand::handleStep1100()
	{
		int step = 1100;
		std::string errorMessage = "干泵粗抽超时";
		auto now_time = std::chrono::steady_clock::now();
		auto elapsed = now_time - d->start_time;

		std::this_thread::sleep_for(std::chrono::seconds(1));
		auto  delay_time = d->lk1->getVacuumPumpingDelayTime();
		//int successCount = 0; //达到极限值次数

		//是否达到真空上限值
		if (d->lk1->getVacuumValueUpperLimitReachesTheSetValue())
		{
			std::this_thread::sleep_for(std::chrono::seconds(int(delay_time)));
			step = 1350;//
		}
		else
		{
			Sleep(100);

			if (elapsed >= d->timeout) {
			
				//超时
				lla_loop_count = 0;
				logInform(d->pump->getName().c_str(), Poco::format("%s: 等待loadlockA腔体压力小于1Pa,当前压力：%f",getName(), d->lk1->getVacuumValue()).c_str());

				addCommandExecutionAlarmMessage(step, d->lk1->getName(), errorMessage);
				step = 1350;//退出
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

	int PumpOpenLoadLock1AutoVacuumCommand::handleStep1350()
	{
		int step = 1350;
		std::string errorMessage = "关闭LLA角阀";
		if (d->lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
		{
			if (d->lk1->getAngleValveOpend())
			{
				auto cmd = d->lk1->createCloseAngleValveCommand();
				d->lk1->startCommand(cmd);
				cmd->wait();
				if (cmd->hasError())
				{
					addCommandExecutionAlarmMessage(step, d->lk1->getName(), "关闭LLA角阀");
					step = 10000;
				}
				else
				{
					step = 5210;
				}
			}
			else
			{
				step = 5210;
			}
		}
		else
		{
			addSubsystemNotNormalAlarmMessage(step, d->lk1->getName());
			step = 10000;
		}
		return step;
	}

	int PumpOpenLoadLock1AutoVacuumCommand::handleStep1120()
	{
		int step = 10;
		//分子泵打开或者运行状态
		if (d->pump->getMolecularPumpOpenedLLA() || (d->pump->getMolecularPumpRunningStateLLA() == 1))
		{
			step = 1140;
		}
		else
		{
			step = 1140;
		}
		return step;
	}

	int PumpOpenLoadLock1AutoVacuumCommand::handleStep1140()
	{
		int step = 1140;
		std::string errorMessage = "关闭loadlock1的角阀";
		if (d->lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
		{
			if (d->lk1->getAngleValveOpend())
			{
				//关闭角阀
				auto cmd = d->lk1->createCloseAngleValveCommand();
				if (!executeCommand(d->lk1, cmd, step, errorMessage))
				{
					addCommandExecutionAlarmMessage(step, d->lk1->getName(), errorMessage);
					step = 10000;
				}
				else {
					step = 1200;
				}
			}
			else {
				step = 1200;
			}
		}
		else
		{
			addSubsystemNotNormalAlarmMessage(step, d->lk1->getName());
			step = 10000;
		}
		return step;
	}

	int PumpOpenLoadLock1AutoVacuumCommand::handleStep1200()
	{
		int step = 1200;
		std::string errorMessage = "打开loadlock1的高真空挡板阀";
		if (d->lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
		{
			//打开高真空挡板阀
			if (d->lk1->getHeightVacuumBaffleValveOpend() == false)
			{
				auto cmd = d->lk1->createOpenHeightVacuumBaffleValveCommand();
				if (!executeCommand(d->lk1, cmd, step, errorMessage))
				{
					addCommandExecutionAlarmMessage(step, d->lk1->getName(), errorMessage);
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
			addSubsystemNotNormalAlarmMessage(step, d->lk1->getName());
			step = 10000;
		}
		return step;
	}

	int PumpOpenLoadLock1AutoVacuumCommand::handleStep1210()
	{
		int step = 10000;
		//达到粗抽压力
		if (d->lk1->getLoadLockRoughVacuumReachesTheSetValue())
		{
			Sleep(1000);
			step = 1300;
		}
		else
		{
			Sleep(100);
		}
		return step;
	}

	int PumpOpenLoadLock1AutoVacuumCommand::handleStep1300()
	{
		int step = 1300;
		std::string errorMessage = "重新打开角阀";
		if (d->lk1->getLoadLockRoughVacuumReachesTheSetValue(10)) {
			if (isColseAngleValvetm) {
				auto cmd = d->tm->createOpenAngleValveCommand();
				if (!executeCommand(d->tm, cmd, step, errorMessage))
				{
					addCommandExecutionAlarmMessage(step, d->tm->getName(), "重新打开角阀");
				}
				else {
					isColseAngleValvetm = false;
				}
			}
			if (isColseAngleValvellb) {
				auto cmd = d->lk2->createOpenAngleValveCommand();
				if (!executeCommand(d->lk2, cmd, step, errorMessage))
				{
					addCommandExecutionAlarmMessage(step, d->lk2->getName(), "重新打开角阀");
				}
				else {
					isColseAngleValvellb = false;
				}
			}
		}

		if (d->tm->getMoleculePipelineVacuumValue() < 5.0)//三个腔室共用一个皮拉尼规
		{
			lla_loop_count = 0;
			Sleep(1000);
			step = 1305;
		}
		else
		{
			Sleep(100);
			++lla_loop_count;
			if (lla_loop_count > 30)
			{
				lla_loop_count = 0;
				logInform(d->pump->getName().c_str(), Poco::format("%s: 等待前级管路压力小于5Pa,当前压力：%f", 
					getName(), d->tm->getMoleculePipelineVacuumValue()).c_str());
			}
		}
		return step;
	}

	int PumpOpenLoadLock1AutoVacuumCommand::handleStep1305()
	{
		int step = 1305;
		std::string errorMessage = "关闭高真空挡板阀";
		if (d->lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
		{
			//关闭高真空挡板阀
			if (d->lk1->getHeightVacuumBaffleValveOpend())
			{
				auto cmd = d->lk1->createCloseHeightVacuumBaffleValveCommand();
				if (!executeCommand(d->lk1, cmd, step, errorMessage))
				{
					addCommandExecutionAlarmMessage(step, d->lk1->getName(), errorMessage);
					step = 10000;
				}
				else
				{
					step = 1310;
				}
			}
			else {
				step = 1310;
			}
		}
		else
		{
			addSubsystemNotNormalAlarmMessage(step, d->lk1->getName());
			step = 10000;
		}

		return step;
	}

	int PumpOpenLoadLock1AutoVacuumCommand::handleStep1310()
	{
		int step = 1310;
		std::string errorMessage = "打开LLA角阀";
		if (d->lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
		{
			if (d->lk1->getAngleValveOpend() == false)
			{
				auto cmd = d->lk1->createOpenAngleValveCommand();
				d->lk1->startCommand(cmd);
				cmd->wait();
				if (cmd->hasError())
				{
					addCommandExecutionAlarmMessage(step, d->lk1->getName(), "打开LLA角阀");
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
			addSubsystemNotNormalAlarmMessage(step, d->lk1->getName());
			step = 10000;
		}
		return step;
	}

	int PumpOpenLoadLock1AutoVacuumCommand::handleStep1320()
	{

		int step = 1320;
		std::string errorMessage = "打开分子泵";
		if (d->tm->getMoleculePipelineVacuumValue() < 10.0)
		{
			if (d->pump->getMolecularPumpOpenedLLA() == false)
			{
				if (d->pump->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					//打开分子泵
					auto cmd = d->pump->createMolecularOpenCommand("LLA");
					d->pump->startCommand(cmd);
					cmd->wait();
					if (cmd->hasError())
					{
						addCommandExecutionAlarmMessage(step, d->pump->getName(), "打开分子泵");
						step = 10000;
					}
					else
					{
						step = 1330;
					}
				}
				else
				{
					addSubsystemNotNormalAlarmMessage(step, d->pump->getName());
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
		return step;
	}

	int PumpOpenLoadLock1AutoVacuumCommand::handleStep1340()
	{
		int step = 10;
		if (d->tm->getMoleculePipelineVacuumValue() < 5.0 && d->lk1->getAngleValveOpend())
		{
			if (d->pump->getMolecularPumpReachSpeedLLA())
			{
				step = 1345;
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
		return step;
	}

	int PumpOpenLoadLock1AutoVacuumCommand::handleStep1345()
	{
		int step = 1345;
		Sleep(2000);
		if (d->lk1->getLoadLockRoughVacuumReachesTheSetValue(6))
		{
			step = 1355;
		}
		else
		{
			step = 1100;
		}
		return step;
	}

	int PumpOpenLoadLock1AutoVacuumCommand::handleStep1355()
	{
		return 1360;
	}

	int PumpOpenLoadLock1AutoVacuumCommand::handleStep1360()
	{
		int step = 1360;
		if (d->lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
		{
			if (d->lk1->getInsertingPlateValveOpend() == false)
			{
				if (d->lk1->getLoadLockRoughVacuumReachesTheSetValue(10)) {
					//打开插板阀
					auto cmd = d->lk1->createOpenInsertingPlateValveCommand();
					d->lk1->startCommand(cmd);
					cmd->wait();
					if (cmd->hasError())
					{
						if (!d->lk1->getLoadLockRoughVacuumReachesTheSetValue(10)) {
							auto cmd2 = d->pump->createResetCommand();
							d->pump->startCommand(cmd2);
							cmd2->wait();
							step = 1000;
						}
						else {
							addCommandExecutionAlarmMessage(step, d->lk1->getName(), "打开插板阀");
							step = 10000;
						}
					}
					else {
						step = 1380;
					}
				}
				else {
					step = 1000;
				}

			}
			else {
				step = 1380;
			}
		}
		else
		{
			addSubsystemNotNormalAlarmMessage(step, d->lk1->getName());
			step = 10000;
		}
		return step;
	}

	int PumpOpenLoadLock1AutoVacuumCommand::handleStep1380()
	{
		return 10;
	}

	int PumpOpenLoadLock1AutoVacuumCommand::handleStep5000()
	{
		int step = 5000;
		if (d->lk1->getVacuumValueReachesTheSetValue())
		{
			Sleep(1000);
			step = 5210;
		}
		else
		{
			Sleep(100);
		}
		return step;
	}

	int PumpOpenLoadLock1AutoVacuumCommand::handleStep5210()
	{
		d->ret = IKernelCommand::RunResult::RUN_OK;
		int step = 10000;
		return step;
	}

	int PumpOpenLoadLock1AutoVacuumCommand::handleStep10000()
	{
		logInform(d->lk1->getName().c_str(), "抽真空循环结束");
		loop = false;
		return -1;
	}



}