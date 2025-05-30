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


namespace FC{

	/**
	* @brief  open tm cavity auto vacuum command for Pump
	*/
	class  PumpOpenTMCavityAutoVacuumCommand : public  KernelSubsystemCommand, public KeyencePlcCommandExecuter{
	public:
		DECLARE_PTR(PumpOpenTMCavityAutoVacuumCommand)
		PumpOpenTMCavityAutoVacuumCommand(KeyencePlcSubSystemHelper* helper);
		virtual std::string getName()const override { return "OpenTMCavityAutoVacuum"; }
		virtual void addCommandExecutionAlarmMessage(const int code_id, const std::string subsytem_name, const std::string message);
		virtual void addSubsystemNotNormalAlarmMessage(const int code_id, const std::string subsytem_name);

	protected:
		virtual RunResult onRun() throw(KernelException);
	private:
		DECLARE_PRIVATE(PumpOpenTMCavityAutoVacuumCommand)
	};




}
#endif
