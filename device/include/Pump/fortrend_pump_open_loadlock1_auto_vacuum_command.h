/**
* @file            fortrend_pump_open_loadlock1_auto_vacuum_command.h
* @brief           open loadlock1 auto vacuum command for pump
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Subsystem/Pump

#ifndef FORTREND_PUMP_OPEN_LOADLOCK1_AUTO_VACUUM_COMMAND_INCLUDE_
#define FORTREND_PUMP_OPEN_LOADLOCK1_AUTO_VACUUM_COMMAND_INCLUDE_

#include "kernel/kernel_subsystem_command.h"

#include "KeyencePLC/keyence_plc_command_executer.h"
#include "KeyencePLC/keyence_plc_subsystem_helper.h"
#include <unordered_map>

namespace FC{

	/**
	* @brief  open loadlock1 auto vacuum command for Pump
	*/
	class  PumpOpenLoadLock1AutoVacuumCommand : public  KernelSubsystemCommand, public KeyencePlcCommandExecuter{
	public:
		DECLARE_PTR(PumpOpenLoadLock1AutoVacuumCommand)
		PumpOpenLoadLock1AutoVacuumCommand(KeyencePlcSubSystemHelper* helper);
		virtual std::string getName()const override { return "OpenLoadLock1AutoVacuum"; }
		virtual void addCommandExecutionAlarmMessage(const int code_id, const std::string subsytem_name, const std::string message);
		virtual void addSubsystemNotNormalAlarmMessage(const int code_id, const std::string subsytem_name);

		//状态处理函数
	
		//1.关闭晶圆盒门
		int handleStep1045();

		//2.关闭传输腔门阀(LL-TM之间的门)
		int handleStep1040();

		//3.关闭llA隔膜阀
		int handleStep1030();

		//4.关闭TM腔的角阀
		int handleStep1050();

		//5.关闭loadLock2腔体的角阀
		int handleStep1060();

		//6.打开机械泵
		int handleStep10();

		//7.打开loadlock1角阀
		int handleStep1310();

		//8.loadLock1是否达到粗抽压力判断
		int handleStep1100();

		//9.转10000
		int handleStep5210();

		//10.退出循环
		int handleStep10000();


		// 打开分子泵流程判断--->后面大于100的
		int handleStep100();
		//关闭插板阀
		int handleStep1000();
		//关闭隔膜阀
		int handleStep1010();

		//关闭TM腔高真空挡板阀
		int handleStep1055();

		//关闭loadLock2高真空挡板阀
		int handleStep1065();

		//分子泵打开或者运行状态
		int handleStep1120();

		//关闭loadlock1的角阀
		int handleStep1140();

		//打开loadlock1的高真空挡板阀
		int handleStep1200();

		//loadlock1达到粗抽压力
		int handleStep1210();

		//重新打开TM角阀
		int handleStep1300();

		//关闭loadlock1高真空挡板阀
		int handleStep1305();

		//打开lk1分子泵
		int handleStep1320();
		//分子管道真空值<5pa ||角阀打开?
	
		// 转速达到
		int handleStep1340();
		int handleStep1345();
		int handleStep1355();
		int handleStep1360();
		int handleStep1380();
		int handleStep5000();



	public:
		using StateHandler = std::function<int()>;

		std::unordered_map<int, StateHandler> stateHandlers;

		bool executeCommand(std::shared_ptr<IKernelSubSystem> subsystem,std::shared_ptr<IKernelCommand> cmd,int currentStep, const std::string errorMessage);

		bool checkSubsystemsNormal(std::initializer_list<std::shared_ptr<IKernelSubSystem>> subsystems);

		void initializeStateHandlers();

		void initializeHLStateHandlers();

	protected:

		bool isColseBaffleValvetm = false;//tm挡板阀
		bool isColseAngleValvetm = false; //tm角阀
		bool isColseBaffleValvellb = false;
		bool isColseAngleValvellb = false;
		int lla_loop_count = 0;
		bool loop = true;
		int step = 10;
		virtual RunResult onRun() throw(KernelException);
	private:
		DECLARE_PRIVATE(PumpOpenLoadLock1AutoVacuumCommand)
	};
}

#endif