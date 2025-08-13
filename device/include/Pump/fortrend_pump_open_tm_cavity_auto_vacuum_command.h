/**
* @file            fortrend_pump_open_tm_cavity_auto_vacuum_command.h
* @brief           open tm cavity auto vacuum command for pump
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Subsystem/Pump

#ifndef FORTREND_PUMP_OPEN_TM_CAVITY_AUTO_VACUUM_COMMAND_INCLUDE_
#define FORTREND_PUMP_OPEN_TM_CAVITY_AUTO_VACUUM_COMMAND_INCLUDE_

#include "kernel/kernel_subsystem_command.h"

#include "KeyencePLC/keyence_plc_command_executer.h"
#include "KeyencePLC/keyence_plc_subsystem_helper.h"
#include <unordered_map>

namespace FC{

	/**
	* @brief  open tm cavity auto vacuum command for Pump
	*/
	class  PumpOpenTMCavityAutoVacuumCommand : public  KernelSubsystemCommand, public KeyencePlcCommandExecuter{
	public:
		DECLARE_PTR(PumpOpenTMCavityAutoVacuumCommand)
		PumpOpenTMCavityAutoVacuumCommand(KeyencePlcSubSystemHelper* helper);
		virtual std::string getName()const override { return "OpenTMCavityAutoVacuum"; }
		void addCommandExecutionAlarmMessage(const int code_id, const std::string subsytem_name, const std::string message);
		virtual void addSubsystemNotNormalAlarmMessage(const int code_id, const std::string subsytem_name);


		enum class SystemState {
			OPEN_MECHANICAL_PUMP = 10,
			OPEN_TM_ANGLE_VALVE = 20,
			CLOSE_TM_DIAPHRAGM_VALVE = 30,
			CLOSE_LLA_TM_CAVITY_DOOR = 40,
			CLOSE_LLB_TM_CAVITY_DOOR = 50,
			CLOSE_LLA_ANGLE_VALVE = 60,
			CLOSE_LLB_ANGLE_VALVE = 70,
			CLOSE_PM_CAVITY_DOOR = 80,
			JUDGE_COARSE_SUCTION_PRESSURE =90,
			CREATE_END = 10000
		};

		using StateHandler = std::function<SystemState()>;
		std::unordered_map<SystemState, StateHandler> stateHandlers;

		void initializeStateHandlers();

		//10 打开干泵
		SystemState handleStepOpenMechanicalPump();

		//20 打开TM腔的角阀
		SystemState handleStepOpenAngleValve();

		//30 关闭隔膜阀
		SystemState handleStepCLoseDiaphragmValve();

		//40 关闭传输腔门阀(LL1-TM之间的门)
		SystemState handleStepCloseLlaTmDoor();

		//50 关闭传输腔门阀(LL2-TM之间的门)
		SystemState handleStepCloseLlbTmDoor();

		//60 关闭PM腔门阀(PM1~PM4)
		SystemState handleStepClosePmDoor();

		//70 关闭loadLock2的角阀
		SystemState handleStepCloseLlbAngleValve();

		//80 关闭loadlock1角阀
		SystemState handleStepCloseLlaAngleValve();

		//90 TM是否达到粗抽压力判断
		SystemState handleStepCoarseSuctionPressure();

		//退出循环
		SystemState handleStepEND();


	protected:
		virtual RunResult onRun() throw(KernelException);
	private:
		DECLARE_PRIVATE(PumpOpenTMCavityAutoVacuumCommand)
	};




}
#endif
