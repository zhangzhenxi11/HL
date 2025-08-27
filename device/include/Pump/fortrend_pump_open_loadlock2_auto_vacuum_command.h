/**
* @file            fortrend_pump_open_loadlock2_auto_vacuum_command.h
* @brief           open loadlock2 auto vacuum command for pump
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Subsystem/Pump

#ifndef FORTREND_PUMP_OPEN_LOADLOCK2_AUTO_VACUUM_COMMAND_INCLUDE_
#define FORTREND_PUMP_OPEN_LOADLOCK2_AUTO_VACUUM_COMMAND_INCLUDE_

#include "kernel/kernel_subsystem_command.h"

#include "KeyencePLC/keyence_plc_command_executer.h"
#include "KeyencePLC/keyence_plc_subsystem_helper.h"

#include <unordered_map>
namespace FC{

	/**
	* @brief  open loadlock2 auto vacuum command for Pump
	*/
	class  PumpOpenLoadLock2AutoVacuumCommand : public  KernelSubsystemCommand, public KeyencePlcCommandExecuter{
	public:
		DECLARE_PTR(PumpOpenLoadLock2AutoVacuumCommand)
		PumpOpenLoadLock2AutoVacuumCommand(KeyencePlcSubSystemHelper* helper);
		virtual std::string getName()const override { return "OpenLoadLock2AutoVacuum"; }
		virtual void addCommandExecutionAlarmMessage(const int code_id, const std::string subsytem_name, const std::string message);
		virtual void addSubsystemNotNormalAlarmMessage(const int code_id, const std::string subsytem_name);
	public:
		using StateHandler = std::function<int()>;

		std::unordered_map<int, StateHandler> stateHandlers;

		bool executeCommand(std::shared_ptr<IKernelSubSystem> subsystem, std::shared_ptr<IKernelCommand> cmd, int currentStep, const std::string errorMessage);

		void initializeStateHandlers();

		//打开机械泵
		int handleStep10();

		//关闭隔膜阀
		int handleStep1030();

		//关闭传输腔门阀(LL-TM之间的门)
		int handleStep1040();

		//关闭晶圆盒门
		int handleStep1045();

		//关闭TM腔的角阀
		int handleStep1050();

		//关闭loadLock2腔体的角阀
		int handleStep1060();

		//loadLock1是否达到粗抽压力判断
		int handleStep1100();

		//打开loadlock1角阀
		int handleStep1310();

		//关闭loadlock1角阀
		int handleStep1320();

		int handleStep5210();

		//推出循环
		int handleStep10000();

	protected:
		virtual RunResult onRun() throw(KernelException);
	private:
		DECLARE_PRIVATE(PumpOpenLoadLock2AutoVacuumCommand)
	};




}
#endif
