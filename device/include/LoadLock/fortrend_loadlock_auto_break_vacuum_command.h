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
	* @brief  open loadlock2 auto vacuum command for Pump
	*/
	class  LoadLockAutoBreakVacuumCommand : public  KernelSubsystemCommand, public KeyencePlcCommandExecuter{
	public:
		DECLARE_PTR(LoadLockAutoBreakVacuumCommand)
		LoadLockAutoBreakVacuumCommand(KeyencePlcSubSystemHelper* helper);
		virtual std::string getName()const override { return "LoadLockAutoBreakVacuum"; }
		virtual void addCommandExecutionAlarmMessage(const std::string subsytem_name, const std::string message, const int code_id);
		virtual void addSubsystemNotNormalAlarmMessage(const int code_id, const std::string subsytem_name);

	public:
		using StateHandler = std::function<int()>;

		std::unordered_map<int, StateHandler> stateHandlers;

		void initializeHLStateHandlers();

	protected:
		virtual RunResult onRun() throw(KernelException);
	private:
		DECLARE_PRIVATE(LoadLockAutoBreakVacuumCommand)
	};




}
#endif
