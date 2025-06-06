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
		virtual void addCommandExecutionAlarmMessage(const int code_id, const std::string subsytem_name, const std::string message);
		virtual void addSubsystemNotNormalAlarmMessage(const int code_id, const std::string subsytem_name);

		using StateHandler = std::function<int()>;

		std::unordered_map<int, StateHandler> stateHandlers;

		bool executeCommand(std::shared_ptr<IKernelSubSystem> subsystem, std::shared_ptr<IKernelCommand> cmd, int currentStep, const std::string errorMessage);

		void initializeStateHandlers();

		//打开干泵
		int handleStep10();

		//打开TM腔的角阀
		int handleStep1050();

		//关闭隔膜阀
		int handleStep1030();

		//关闭传输腔门阀(LL1-TM之间的门)
		int handleStep1040();

		//关闭传输腔门阀(LL2-TM之间的门)
		int handleStep1041();

		//关闭PM腔门阀(PM1~PM4)
		int handleStep1042();

		//关闭loadLock2的角阀
		int handleStep1060();

		//关闭loadlock1角阀
		int handleStep1310();

		//TM是否达到粗抽压力判断
		int handleStep1100();

		int handleStep5210();

		//退出循环
		int handleStep10000();

	protected:
		virtual RunResult onRun() throw(KernelException);
	private:
		DECLARE_PRIVATE(PumpOpenTMCavityAutoVacuumCommand)
	};




}
#endif
