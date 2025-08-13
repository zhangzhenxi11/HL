/**
* @file            fortrend_pump_open_loadlock2_auto_vacuum_command.h
* @brief           open loadlock2 auto vacuum command for pump
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Subsystem/Pump

#ifndef FORTREND_LOADLOCK_AUTO_BREAK_VACUUM_COMMAND_INCLUDE_
#define FORTREND_LOADLOCK_AUTO_BREAK_VACUUM_COMMAND_INCLUDE_

#include "kernel/kernel_subsystem_command.h"

#include "KeyencePLC/keyence_plc_command_executer.h"
#include "KeyencePLC/keyence_plc_subsystem_helper.h"

#include <unordered_map>
namespace FC{

	/**
	* @brief  open loadlock2/loadlock1 auto vacuum command for Pump
	*/
	class  LoadLockAutoBreakVacuumCommand : public  KernelSubsystemCommand, public KeyencePlcCommandExecuter{
	public:
		DECLARE_PTR(LoadLockAutoBreakVacuumCommand)
		LoadLockAutoBreakVacuumCommand(KeyencePlcSubSystemHelper* helper);
		virtual std::string getName()const override { return "LoadLockAutoBreakVacuum"; }
		virtual void addCommandExecutionAlarmMessage(const std::string subsytem_name, const std::string message, const int code_id);
		virtual void addSubsystemNotNormalAlarmMessage(const int code_id, const std::string subsytem_name);

	public:
		enum class SystemState {
			EXHAUST_VACUUM_VALUE_REACHES_SETVALUE = 10,
			CLOSE_TMCAVITY_DOOR = 20,
			CLOSE_ANGLE_VALVE = 30,
			OPEN_DIAPHRAGM_VALVE_SLOW = 40,
			OPEN_DIAPHRAGM_VALVE_FAST = 50,
			JUDGE_FAST_CHARGING_CONDITION = 60,
			CLOSE_DIAPHRAGM_VALVE = 70,
			JUDGE_CHARGING_ATMOSPHERE_CONDITION = 80,
			CREATE_END = 10000
		};
		using StateHandler = std::function<SystemState()>;
		std::unordered_map<SystemState, StateHandler> stateHandlers;

		SystemState handleStepReachesSetValue();

		//关闭传输腔门阀(LL-TM之间的门)
		SystemState handleStepCloseTmCavityDoor();

		SystemState handleStepAngleValve();

		SystemState handleStepDiaphragmValveSlow();

		SystemState handleStepDiaphragmValveFast();

		SystemState handleStepFastChargingCondition();//快充条件

		SystemState handleStepCloseDiaphragmValve();

		SystemState handleStepAtmosphereCondition();
		
		SystemState handleStepEnd();
			

		void initializeHLStateHandlers();

	protected:
		virtual RunResult onRun() throw(KernelException);
	private:
		DECLARE_PRIVATE(LoadLockAutoBreakVacuumCommand)
	};




}
#endif
