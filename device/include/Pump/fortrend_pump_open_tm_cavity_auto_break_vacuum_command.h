/**
* @file            fortrend_pump_open_tm_cavity_auto_break_vacuum_command.h
* @brief           open tm cavity auto break vacuum command for pump
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Subsystem/Pump

#ifndef FORTREND_PUMP_OPEN_TM_CAVITY_AUTO_BREAK_VACUUM_COMMAND_INCLUDE_
#define FORTREND_PUMP_OPEN_TM_CAVITY_AUTO_BREAK_VACUUM_COMMAND_INCLUDE_

#include "kernel/kernel_subsystem_command.h"

#include "KeyencePLC/keyence_plc_command_executer.h"
#include "KeyencePLC/keyence_plc_subsystem_helper.h"

#include <unordered_map>
namespace FC{

	/**
	* @brief  open tm cavity auto break vacuum command for Pump
	*/
	class  PumpOpenTMCavityAutoBreakVacuumCommand : public  KernelSubsystemCommand, public KeyencePlcCommandExecuter{
	public:
		DECLARE_PTR(PumpOpenTMCavityAutoBreakVacuumCommand)
		PumpOpenTMCavityAutoBreakVacuumCommand(KeyencePlcSubSystemHelper* helper);
		virtual std::string getName()const override { return "PumpOpenTMCavityAutoBreakVacuum"; }
		virtual void addCommandExecutionAlarmMessage(const int code_id, const std::string subsytem_name, const std::string message);
		virtual void addSubsystemNotNormalAlarmMessage(const int code_id, const std::string subsytem_name);
	public:
			enum class SystemState {
			EXHAUST_VACUUM_VALUE_REACHES_SETVALUE = 10,
			CLOSE_TMCAVITY_DOOR = 20,
			CLOSE_ANGLE_VALVE = 30,
			OPEN_DIAPHRAGM_VALVE_SLOW = 40,
			JUDGE_CHARGING_ATMOSPHERE_CONDITION = 60,
			CLOSE_DIAPHRAGM_VALVE = 70,
			CREATE_END = 10000
		};

		using StateHandler = std::function<SystemState()>;
		std::unordered_map<SystemState, StateHandler> stateHandlers;
		
		//排气压力是否达到设定值
		SystemState handleStepReachesSetValue();

		//关闭传输腔门阀(LL-TM之间的门)
		SystemState handleStepCloseTmCavityDoor();
		
		//关闭角阀
		SystemState handleStepAngleValve();

		//打开慢充隔膜阀
		SystemState handleStepDiaphragmValveSlow();
		
		//关隔膜阀
		SystemState handleStepCloseDiaphragmValve();
		
		//检测是否大气条件
		SystemState handleStepAtmosphereCondition();
		
		//结束
		SystemState handleStepEnd();
		
		void initializeHLStateHandlers();
		
	protected:
		virtual RunResult onRun() throw(KernelException);
	private:
		DECLARE_PRIVATE(PumpOpenTMCavityAutoBreakVacuumCommand)
	};




}
#endif
