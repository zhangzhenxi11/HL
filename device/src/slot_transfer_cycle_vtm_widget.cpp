/**
* @file            slot_transfer_cycle_test_widget.h
* @brief           slot transfer cycle test widget
* @author			xielonghua
*/

// Library: FortrendUI
// Package: VTM

#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <iostream>
#include <Windows.h>

#include <array>
#include "Poco/Format.h"

#include  "slot_transfer_cycle_vtm_widget.h" 
#include  "Kernel/FortrendUI/slot_transfer_editor_widget.h"
#include  "device/ui_slot_transfer_cycle_vtm_widget.h"
#include  "Kernel/kernel.h"
#include  "kernel/kernel_log.h"
#include  "Kernel/kernel_action_subsystem.h"
#include  "kernel/kernel_parallel_action.h"
#include  "kernel/kernel_subsystem.h"
#include  "Kernel/Fortrend/fortrend_cassette_manager.h"
#include  "Kernel/Fortrend/cassette.h"

#include "EFEM/efem_wafer_robot_subsystem.h" 
#include "EFEM/efem_loadport_subsystem.h" 
#include "EFEM/efem_aligner_subsystem.h"

#include  "Kernel/Fortrend/fortrend_station.h"
#include  "Kernel/Fortrend/aligner_abstract_subsystem.h"
#include  "Kernel/kernel_subsystem_update_command.h"
#include  "kernel/kernel_block_manager.h"

#include  "Aligner/fortrend_aligner_subsystem.h"
#include  "CoolingCavity/fortrend_cooling_cavity_subsystem.h"
#include  "LoadLock/fortrend_loadlock_subsystem.h"
#include  "PMCavity/fortrend_pm_cavity_subsystem.h"
#include  "TMCavity/fortrend_tm_cavity_subsystem.h"
#include  "SunwayRobot/fortrend_sunwayrobot_subsystem.h"
#include  "Pump/fortrend_pump_subsystem.h"

#include  "Aligner/fortrend_aligner_align_command.h"

#include  "CoolingCavity/fortrend_cooling_cavity_cooling_command.h"
#include  "CoolingCavity/fortrend_cooling_cavity_close_diaphragm_valve_command.h"
#include  "CoolingCavity/fortrend_cooling_cavity_open_diaphragm_valve_command.h"

#include  "LoadLock/fortrend_loadlock_close_angle_valve_command.h"
#include  "LoadLock/fortrend_loadlock_close_cassette_door_command.h"
#include  "LoadLock/fortrend_loadlock_close_diaphragm_valve_command.h"
#include  "LoadLock/fortrend_loadlock_close_exhaust_valve_command.h"
#include  "LoadLock/fortrend_loadlock_close_tm_cavity_door_command.h"
#include  "LoadLock/fortrend_loadlock_mapping_command.h"
#include  "LoadLock/fortrend_loadlock_move_to_slot_command.h"
#include  "LoadLock/fortrend_loadlock_open_angle_valve_command.h"
#include  "LoadLock/fortrend_loadlock_open_cassette_door_command.h"
#include  "LoadLock/fortrend_loadlock_open_diaphragm_valve_command.h"
#include  "LoadLock/fortrend_loadlock_open_exhaust_valve_command.h"
#include  "LoadLock/fortrend_loadlock_open_tm_cavity_door_command.h"

#include  "PMCavity/fortrend_pm_cavity_close_tm_cavity_door_command.h"
#include  "PMCavity/fortrend_pm_cavity_open_tm_cavity_door_command.h"
#include  "PMCavity/fortrend_pm_cavity_reset_command.h"

#include "EFEM/efem_loadport_reset_command.h"
#include "EFEM/efem_loadport_openbox_command.h"
#include "EFEM/efem_loadport_closebox_command.h"
#include "EFEM/efem_loadport_opendoor_command.h"
#include "EFEM/efem_loadport_closedoor_command.h"
#include "EFEM/efem_loadport_getmap_command.h"
#include "EFEM/efem_loadport_lockbox_command.h"
#include "EFEM/efem_loadport_unlockbox_command.h"

#include "EFEM/efem_robot_speed_command.h"
#include "EFEM/efem_robot_reset_command.h"
#include "EFEM/efem_robot_getwafer_command.h"
#include "EFEM/efem_robot_putwafer_command.h"
#include "EFEM/efem_robot_status_command.h"
#include "EFEM/efem_aligner_align_command.h"
#include  "Pump/fortrend_pump_mechanical_open_command.h"
#include  "Pump/fortrend_pump_mechanical_close_command.h"
#include  "Pump/fortrend_pump_molecular_open_command.h"
#include  "Pump/fortrend_pump_molecular_close_command.h"
#include  "VTMSignalTower/fortrend_vtm_signal_tower.h"

#include  <QVBoxLayout>
#include  <QHBoxLayout>
#include  <QTableWidget>
#include  <QMessageBox>
#include  <QDebug>
#include  <QDoubleSpinBox>
#include  <QFileDialog>
#include  <QSettings>
#include  <QComboBox>
#include  <QCheckBox>
#include  <QCoreApplication>
#include  <QDir>

#include "./device/UnifiedWaferTask.h"
#include "./device/TaskManager.h"
#include "./device/ThreadSafeStateMachine.h"

#if _MSC_VER >1600
#pragma execution_character_set("utf-8")
#endif

#define CYCLE_SIM_MODE 1
// 全局任务管理器
TaskManager& taskManager = TaskManager::getInstance();

namespace FC{


	//LK 内部传输任务 (机械手搬运)
	struct LoadLockTransferWafer{
		int slot = 0;
		std::string transfer = "lk1_to_lk2";
		bool is_finish = false;
		int selected_arm = 0;
	};
	//LP -> LK 传输任务 (EFEM 搬运)
	struct LPTransferWafer{
		int slot_lp = 0;
		int slot_lk = 0;
		std::string transfer = "lk1_to_lk2";
		std::string source_lp = "ELP1";
		bool is_finish_putlk = false; //是否完成放入Load Lock
		int selected_arm = 0;
	};
	//机械手跨腔传输任务 (TM 机械手)
	struct RobotTransferWafer{
		std::string source_loadlock = "";
		std::string target_loadlock = "";
		std::string source_lp = "ELP1";
		int slot_lp = 0;
		int slot_lk = 0;
		std::string transfer = "lk1_to_lk2";
		int selected_arm = 0;
		bool pm1_is_enable = true;
		bool pm2_is_enable = true;
		bool pm3_is_enable = true;
		bool pm4_is_enable = true;//add
		bool is_finish = false;
	};

	// 定义传输任务结构体
	struct TransferTask {
		enum PortType { LP1, LP2 };
		enum LockType { LLA, LLB };

		PortType sourcePort;
		LockType targetLock;
		int sourceSlot;   // LoadPort中的槽位
		int targetSlot;   // LoadLock中的槽位
		int arm;          // 机械臂选择 (0=A, 1=B)
		bool pm1Enabled;  // 处理模块1使能
		bool pm2Enabled;  // 处理模块2使能
		bool pm3Enabled;  // 处理模块3使能
		bool pm4Enabled;  // 处理模块4使能
	};

	UnifiedWaferTask UnifiedTask; //统一结构体

	/**
	* QSlotTransforEditor
	*/
	class QSlotTransforEditor : public QSlotTransferEditorWidget{
	public:
		QSlotTransforEditor(const std::shared_ptr<IKernel>& kernel, QWidget* parent = NULL);

	};

	QSlotTransforEditor::QSlotTransforEditor(const std::shared_ptr<IKernel>& kernel, QWidget* parent)
		:QSlotTransferEditorWidget(kernel, parent){

	}

	/**
	* QSlotTransferCycleWTMWidgetPrivate
	*/
	class QSlotTransferCycleVTMWidgetPrivate : public KernelListener < IKernelCommand > {
	public:
		Q_DECLARE_PUBLIC(QSlotTransferCycleVTMWidget)
		QSlotTransferCycleVTMWidgetPrivate(QSlotTransferCycleVTMWidget*p);
	public:
		void startLoadLock1Action(); //pass
		void startLoadLock2Action(); //pass
		void startEFEMAction();      //pass
		void startPMAction();        
		void startUpdateStatusAction(); 
		void startRobotAction();    //pass
		void startVacuumAction();   //pass
		void resetAction();
		void setRunning(const bool value);
		void setLoadLock1PutCassetteFinished();
		void setLoadLock2PutCassetteFinished();
		
		//获取UI流程队列
		bool setTransferSequence();

		//衡流获取UI流程队列
		/*bool setHLTransferSequence();*/

		bool setHLTransferSequence();

		bool setPMCavityParameter();

		void onUpdateCycleInfo();

		void onUpdateProcessControlEnabled(const bool value);

		void onUpdateControlEnabled(const QString control, const bool enabled);

		virtual void onAttributeChange(const IKernelCommand* cmd);

		//找此片晶圆的pm工艺腔
		std::string getSelectPmProcessName(UnifiedWaferTask task);

		UnifiedWaferTask::Location getSelectPmLocation(UnifiedWaferTask task);

		void UpdateLLASubTransferDatas();

		void UpdateLLBSubTransferDatas();

		void UpdateEfemSubTransferDatas();

		void UpdatePmSubTransferDatas(std::string pmName);

		/************************zzx  add*********************************/
		//处理多任务
		void executeEFEMTransfer();

		void executeLLATransfer();

		void executeLLBTransfer();

		void executePM1Transfer();

		void executePM2Transfer();

		void executePM3Transfer();

		void executePM4Transfer();

		void executeTMTransfer();

		void executeUpdateTransferStatus();

		/************************zzx  add*********************************/
	private:
		QSlotTransferCycleVTMWidget * q_ptr;
		Ui::SlotTransferCycleVTMWidget *ui;
		std::shared_ptr<IKernel> kernel = 0;
		std::shared_ptr<FortrendVTMSignalTower> tower = 0;
		std::shared_ptr<FortrendTMCavitySubsystem> tmplc;
		bool plcauto = false;

		/************************zzx  add*********************************/
		std::shared_ptr<EFEMWaferRobotSubsystem> ewtr = nullptr;
		std::shared_ptr<EFEMLPSubsystem> elp1 = nullptr;
		std::shared_ptr<EFEMLPSubsystem> elp2 = nullptr;
		std::shared_ptr<FortrendLoadLockSubsystem> lk1 = nullptr;
		std::shared_ptr<FortrendLoadLockSubsystem> lk2 = nullptr;
		std::shared_ptr<FortrendSunwayRobotSubsystem> wtr = nullptr;
		std::shared_ptr<EFEMAlignerSubsystem> ealigner = nullptr;
		std::shared_ptr<FortrendTMCavitySubsystem> tm = nullptr;
		std::shared_ptr<FortrendPMCavitySubsystem> pm1 = nullptr;
		std::shared_ptr<FortrendPMCavitySubsystem> pm2 = nullptr;
		std::shared_ptr<FortrendPMCavitySubsystem> pm3 = nullptr;
		std::shared_ptr<FortrendPMCavitySubsystem> pm4 = nullptr;
		std::shared_ptr<Cassette> cass1;
		std::shared_ptr<Cassette> cass2;
		std::shared_ptr<Cassette> lk1_cass;
		std::shared_ptr<Cassette> lk2_cass;

		bool task_right_exchange = false; //双片交换 
		bool tool_allow_get_wafer = false;// true呼叫LP上料，  false LP上料完成
		bool tool_allow_put_wafer = false;// true呼叫LP下料，  false LP下料完成


		bool tool_allow_pm1_get_wafer = false;//true呼叫pm1上料，  false pm1上料完成
		bool tool_allow_pm1_put_wafer = false; //true呼叫pm1下料，  false pm1下料完成

		bool tool_allow_pm2_get_wafer = false;
		bool tool_allow_pm2_put_wafer = false;

		bool tool_allow_pm3_get_wafer = false;
		bool tool_allow_pm3_put_wafer = false;

		bool tool_allow_pm4_get_wafer = false;
		bool tool_allow_pm4_put_wafer = false;


		//int selectPmEnableList[4] = { 0, 0, 0, 0 }; // 工艺腔使能数组
		std::array<int, 4> selectPmEnableList;

		bool CheckTMVacuumMeetsStandard(int preStep);
		//CSR 取放晶圆到LL时检测条件
		bool CheckLLVacuumMeetsStandard(std::string llName, int preStep);

		/************************zzx  add*********************************/
		//2025-7-16 新增
		std::vector<UnifiedWaferTask> tasks; //总任务队列

		std::vector<UnifiedWaferTask> worktasks; //实际工作队列

		/************************EFEM*********************************/

		std::vector<UnifiedWaferTask> efemUnkownStatusTasks; //未知状态的数组

		std::vector<UnifiedWaferTask> efemPendingTasks;  //工艺前,待上料数组
									  
		std::vector<UnifiedWaferTask> efemCompletedTasks; //工艺前，放料完成的数组

		std::vector<UnifiedWaferTask> efemReturnPendingTasks;  //工艺后,待下料数组

		std::vector<UnifiedWaferTask> efemReturnCompletedTasks; //工艺后，下料完成的数组

		/************************loadLock*********************************/

		std::vector<UnifiedWaferTask> loadLockPendingTasks;

		std::vector<UnifiedWaferTask> loadLockCompletedTasks;

		std::vector<UnifiedWaferTask> loadLockReturnPendingTasks;

		std::vector<UnifiedWaferTask> loadLockReturnCompletedTasks;


		std::vector<UnifiedWaferTask> loadLockAPendingTasks;
									  
		std::vector<UnifiedWaferTask> loadLockACompletedTasks;

		std::vector<UnifiedWaferTask> loadLockAReturnPendingTasks;

		std::vector<UnifiedWaferTask> loadLockAReturnCompletedTasks;
							  
		std::vector<UnifiedWaferTask> loadLockBPendingTasks;
									  
		std::vector<UnifiedWaferTask> loadLockBCompletedTasks;

		std::vector<UnifiedWaferTask> loadLockBReturnPendingTasks;

		std::vector<UnifiedWaferTask> loadLockBReturnCompletedTasks;

		/************************PM*********************************/
		std::vector<UnifiedWaferTask> pmPendingTasks;

		std::vector<UnifiedWaferTask> pmCompletedTasks;
		// IN_PROGRESS
		std::vector<UnifiedWaferTask> pmProgressingTasks;

		/******************************************1.LP->LK 任务池（按 LK 分类）******************************************************/
		// 1.LP->LK 任务池（按 LK 分类）
		std::vector<LPTransferWafer> sequence_tolk1_wafer;//cycle配置数组 上料时给sequence_lp1_get_wafer赋值的
		std::vector<LPTransferWafer> sequence_tolk2_wafer;//cycle配置数组 上料时给sequence_lp2_get_wafer赋值的

		std::vector<LPTransferWafer> sequence_tolk1_wafer_copy;//cycle配置数组
		std::vector<LPTransferWafer> sequence_tolk2_wafer_copy;//cycle配置数组

		std::vector<LPTransferWafer> sequence_lp1_transfer_wafer;//cycle配置 lp1待工艺总数组
		std::vector<LPTransferWafer> sequence_lp1_put_wafer;//工艺完后 LP1待放料数组
		std::vector<LPTransferWafer> sequence_lp1_get_wafer;//工艺完前 LP1待上料数组
		
		std::vector<LPTransferWafer> sequence_lp2_transfer_wafer; //cycle配置 lp2待工艺总数组
		std::vector<LPTransferWafer> sequence_lp2_put_wafer;//工艺完后 LP2待放料数组
		std::vector<LPTransferWafer> sequence_lp2_get_wafer;//工艺完前 LP2待上料数组

		//LLB的复制任务队列  做完一次循环后重新添加到sequence_loadlock2_transfer_wafer数组
		std::vector<LPTransferWafer> sequence_lp2_transfer_wafer_copy;

		//LLA的复制任务队列，做完一次循环后重新添加到sequence_loadlock1_transfer_wafer数组
		std::vector<LPTransferWafer> sequence_lp1_transfer_wafer_copy;


		/****************************************2. 机械手任务池********************************************************/

		//2. 机械手任务池
		/*
		机械手任务队列里面的LP1部分。LLA全部做完工艺 需再循环时 重新添加到sequence_robot_transfer_wafer的末尾
		*/
		std::vector<RobotTransferWafer> sequence_robot_transfer_wafer_lp1;

		/*
		机械手任务队列里面的LP2部分，每一次读取cycle配置时添加
		*/
		std::vector<RobotTransferWafer> sequence_robot_transfer_wafer_lp2;

		/*
		cycle上面配置的所有LLA和LLB晶圆集合,做完工艺放回对应LL腔后从集合删除
		*/
		std::vector<RobotTransferWafer> sequence_robot_transfer_wafer;  // 总任务队列

		/*
		机械手放PM上的晶圆数组,从PM上取走后删除
		*/
		std::vector<RobotTransferWafer> sequence_robot_get_wafer;


		/*****************************************互斥锁*******************************************************/

		/*
		sequence_robot_transfer_wafer数组的互斥锁，数组在多个线程里面操作的情况下必须添加
		*/
		std::mutex vec_mutex_robot;

		/*
		sequence_loadlock1_transfer_wafer数组的互斥锁，数组在多个线程里面操作的情况下必须添加
		*/
		std::mutex vec_mutex_lla;

		/*
		sequence_loadlock2_transfer_wafer数组的互斥锁，数组在多个线程里面操作的情况下必须添加
		*/
		std::mutex vec_mutex_llb;

		/**************************************3.LK 内部任务池**********************************************************/

		/*
		  Cycle配置LLA腔上待做工艺的晶圆数组，从LLA取料完成后删除
		*/
		std::vector<LoadLockTransferWafer> sequence_loadlock1_transfer_wafer;
		/*
		 从LLA取走后的晶圆数组。放回LLA后从数组删除
		*/
		std::vector<LoadLockTransferWafer> sequence_loadlock1_put_wafer;
		/*
		Cycle配置LLB腔上待做工艺的晶圆数组，从LLB腔取走后删除
		*/
		std::vector<LoadLockTransferWafer> sequence_loadlock2_transfer_wafer;
		/*
		从LLB取走后的晶圆数组。放回LLB后从数组删除
		*/
		std::vector<LoadLockTransferWafer> sequence_loadlock2_put_wafer;

		/************************************************************************************************/


		PMCavityProcessParameters pm_process_param[3];
		std::string armWferTarget = "";
		std::string target_loadlock = "";

		bool cycleFinished_lla = false;
		bool cycleFinished_llb = false;

		bool robot_step_once_finished = true;
		bool vacumm_step_once_finished = true;
		bool update_step_once_finished = true;
		bool loadlock1_step_once_finished = true;
		bool loadlock2_step_once_finished = true;

		bool abortCycle = false;//true：中止Cycle
		bool efem_step_once_finished = true;
		bool abortCycleFinsshed_robot = false; // 中止Cycle已完成
		bool abortCycleFinsshed_lla = false; // 中止Cycle已完成
		bool abortCycleFinsshed_llb = false; // 中止Cycle已完成

		bool pm_step_once_finished = true;

		bool robot_step_wafer_finished = false;
		int PMMessagecount = 2;
		

		bool running = false;
		bool ispause = false;
		int robot_auto_step = 0;
		int loadlock1_auto_step = 10;
		int loadlock2_auto_step = 10;
		int vacuum_auto_step = 0;
		int pm_auto_step = 10;
		int efem_auto_step = 10;
		int update_auto_step = 10;

		/**
		是否开始抽LoadLock1真空
		*/
		bool loadlock1_get_vacuum = false;
		/**
		是否开始抽LoadLock2真空
		*/
		bool loadlock2_get_vacuum = false;
		/**
		  是否开始抽TM真空
		*/
		bool tm_get_vacuum = false;

		bool tm_angle_valve_recard_state = false;

		bool tm_air_inflation = false;

		bool tm_vacuum_in_range = false;

		//int cycle_times = 1;

		int cycle_times_lla = 1;
		int cycle_times_llb = 1;
		int internal_times = 1;

		//int finished_time = 0;

		int finished_time_lla = 0;

		int finished_time_llb = 0;

		int cycle_strategy = 0;

		int loadlock1_move_slot_index = 1;
		int loadlock2_move_slot_index = 1;

		bool loadlock1_put_cassette_finished = false; //放料到loadlock1完成
		bool loadlock2_put_cassette_finished = false;

		bool loadlock1_process_finished = false;
		bool loadlock2_process_finished = false;


		bool pm_allow_get_put_wafer = false;//PM允许取放  
		bool pm_allow_goto_craft = false; //PM允许转到工艺

		bool loadlock1_allow_get_wafer = false; //true呼叫真空机械手取料
		bool loadlock2_allow_get_wafer = false;
		bool loadlock1_allow_put_wafer = false; //true呼叫真空机械手放料
		bool loadlock2_allow_put_wafer = false;



		bool tool1_allow_get_wafer = false;// true呼叫LP1上料，  false LP1上料完成

		bool tool2_allow_get_wafer = false;// true呼叫LP2上料，  false LP2上料完成

		bool tool1_allow_put_wafer = false;// true呼叫下料到LP1，false 下料到LP1完成

		bool tool2_allow_put_wafer = false;// true呼叫下料到LP2，false 下料到LP2完成

		/**
		true：LP1的当前晶圆已做完工艺
		*/
		bool lp1_cycle_one_time_finished = false;
		/**
		true：LP2的当前晶圆已做完工艺
		*/
		bool lp2_cycle_one_time_finished = false;

		bool is_lp2_cycle = false;//lp2循环

		bool is_lp1_cycle = false;//lp1循环

		bool hasUPS = false;

		std::string loadlock_process_name;
		const std::string loadlock1_process_name = "LLA流程步骤";
		const std::string loadlock2_process_name = "LLB流程步骤";
		const std::string robot_process_name = "机械手流程步骤";
		const std::string vacuum_process_name = "真空控制流程步骤";
		const std::string pm_process_name = "PM流程步骤";
		const std::string reset_process_name = "reset";
		const std::string update_process_name = "update";
		const std::string efem_process_name = "EFEM流程步骤";
		const int sleep_time = 5000;
		int robot_selected_arm = 0;  //机械手臂选择

	public:
		const std::string module_name = "Cycle";

	private:
		void logFailed(const std::string station_name, const std::string log);
		void logFailedNotNormal(const std::string station_name, const std::string process_name, const int step);
		
		void logFailedExcuteCommandHasError(const std::string station_name, const std::string command_name, const std::string process_name, const int step);
	};

	QSlotTransferCycleVTMWidgetPrivate::QSlotTransferCycleVTMWidgetPrivate(QSlotTransferCycleVTMWidget*p)
		:q_ptr(p){

		std::array<int, 4> selectPmEnableList = { 0, 0, 0, 0 }; // 合法
		//tower = kernel->getKernelModule<FortrendVTMSignalTower>();
	}

	void QSlotTransferCycleVTMWidgetPrivate::onUpdateCycleInfo(){
		QMetaObject::invokeMethod(q_ptr, "update_cycle_data", Qt::AutoConnection);
	}

	void QSlotTransferCycleVTMWidgetPrivate::onUpdateProcessControlEnabled(const bool enabled){
		QMetaObject::invokeMethod(q_ptr, "updateProcessControlEnabled", Qt::AutoConnection,
			Q_ARG(bool, enabled));
	}

	void QSlotTransferCycleVTMWidgetPrivate::onUpdateControlEnabled(const QString control, const bool enabled){
		QMetaObject::invokeMethod(q_ptr, "updateControlEnabled", Qt::AutoConnection,
			Q_ARG(QString, control), Q_ARG(bool, enabled));
	}

	void QSlotTransferCycleVTMWidgetPrivate::onAttributeChange(const IKernelCommand* cmd){
		if (cmd->getState() == IKernelCommand::CMD_IDEL || cmd->getState() == IKernelCommand::CMD_BUSY){
			return;
		}


		QMetaObject::invokeMethod(q_ptr, "update_cycle_data", Qt::AutoConnection);
	}

	std::string QSlotTransferCycleVTMWidgetPrivate::getSelectPmProcessName(UnifiedWaferTask task)
	{
		//task.pm1Enabled;
		int pmIndex = -1;
		for (int i = 0; i < 4; i++)
		{
			if (selectPmEnableList[i] == 1)
			{
				pmIndex = i;
				break;
			}
		}
		if (pmIndex == 0)
			return "PM1";
		else if (pmIndex == 1)
			return "PM2";
		else if (pmIndex == 2)
			return "PM3";
		else
			return "PM4";
	}

	UnifiedWaferTask::Location QSlotTransferCycleVTMWidgetPrivate::getSelectPmLocation(UnifiedWaferTask task)
	{
		int pmIndex = -1;
		for (int i = 0; i < 4; i++)
		{
			if (selectPmEnableList[i] == 1)
			{
				pmIndex = i;
				break;
			}
		}
		if (pmIndex == 0)
			return UnifiedWaferTask::Location::PM1;
		else if (pmIndex == 1)
			return UnifiedWaferTask::Location::PM2;
		else if (pmIndex == 2)
			return UnifiedWaferTask::Location::PM3;
		else
			return UnifiedWaferTask::Location::PM4;

		return UnifiedWaferTask::Location();
	}

	void QSlotTransferCycleVTMWidgetPrivate::UpdateLLASubTransferDatas()
	{
		efemUnkownStatusTasks = taskManager.getEfemUnkownStatusTasks(); //LP中初始状态的晶圆
		loadLockAPendingTasks = taskManager.getLoadLockPendingTasks("LLA");     // 5上料 破过真空，但没抽真空的
		loadLockACompletedTasks = taskManager.getLoadLockCompletedTasks("LLA"); // 6 上料 抽完真空，并取走的
		loadLockAReturnPendingTasks = taskManager.getLoadLockReturnPendingTasks("LLA");// 7下料 ，待放到LL的晶圆数量
		loadLockAReturnCompletedTasks = taskManager.getLoadLockReturnCompletedTasks("LLA");// 8下料 ，放到LL的晶圆数量
	}

	void QSlotTransferCycleVTMWidgetPrivate::UpdateLLBSubTransferDatas()
	{
		efemUnkownStatusTasks = taskManager.getEfemUnkownStatusTasks(); //LP中初始状态的晶圆
		loadLockBPendingTasks = taskManager.getLoadLockPendingTasks("LLB");     // 5上料 破过真空，但没抽真空的
		loadLockBCompletedTasks = taskManager.getLoadLockCompletedTasks("LLB"); // 6 上料 抽完真空，并取走的
		loadLockBReturnPendingTasks = taskManager.getLoadLockReturnPendingTasks("LLB");// 7下料 ，待放到LL的晶圆数量
		loadLockBReturnCompletedTasks = taskManager.getLoadLockReturnCompletedTasks("LLB");// 8下料 ，放到LL的晶圆数量
	}

	void QSlotTransferCycleVTMWidgetPrivate::UpdateEfemSubTransferDatas()
	{
		efemUnkownStatusTasks = taskManager.getEfemUnkownStatusTasks(); //未知
		efemPendingTasks = taskManager.getEfemPendingTasks(); //要上料的晶圆
		efemReturnPendingTasks = taskManager.getEfemRuturnPendingTasks(); //已经到LL上的晶圆
		efemReturnCompletedTasks = taskManager.getEfemRuturnCompletedTasks(); //EFEM下料完成的任务

	}

	void QSlotTransferCycleVTMWidgetPrivate::UpdatePmSubTransferDatas(std::string pmName)
	{
		pmPendingTasks = taskManager.getPMPendingTasks(pmName);
		pmCompletedTasks = taskManager.getPMPendingTasks(pmName);
		pmProgressingTasks = taskManager.getPMPendingTasks(pmName);
	}

	/*
	*EFEM大气流程
	*每一片的来源位置，目标位置，在UI中都明确的，分上下料，那么处理时候达到此状态下的晶圆
	* lp上下料
	* 上料lp
	* 1.判断门是否开， 开直接那mapping result,否则，开盒指令
	* 2.ewtr 从下到上取片，默认双臂，下手臂取下，上手臂取上
	* 3.先后寻边
	* 4.再到aliger片子取到手臂上
	* 5.判断casste是否开着，判断有无晶圆，再去放对应LLA的槽
	*
	* 下料到lp
	* 6.当LLa中放回了wafer，再ewtr取片
	* 7.ewtr取到放回lp1的槽上，流程步骤 +1
	*/
	
	void QSlotTransferCycleVTMWidgetPrivate::executeEFEMTransfer()
	{
		std::shared_ptr<EFEMLPSubsystem> elp;
		std::shared_ptr<FortrendLoadLockSubsystem> lk; 
		if (!ewtr || !elp1 || !elp2 || !lk1 || !lk2)
		{
			ewtr = kernel->getKernelModule<EFEMWaferRobotSubsystem>("EWTR");
			elp1 = kernel->getKernelModule<EFEMLPSubsystem>("ELP1");
			elp2 = kernel->getKernelModule<EFEMLPSubsystem>("ELP2");
			lk1 = kernel->getKernelModule<FortrendLoadLockSubsystem>("LLA");
			lk2 = kernel->getKernelModule<FortrendLoadLockSubsystem>("LLB");
			ealigner = kernel->getKernelModule<EFEMAlignerSubsystem>("EALIGNER");
		}
		while (!taskManager.isStopped())
		{
			Sleep(500);
			efem_step_once_finished = false;

			if (taskManager.waitForTasks(1000))
			{
				switch (efem_auto_step)
				{
				case 10:
				{
					efemUnkownStatusTasks = taskManager.getEfemUnkownStatusTasks();
					efemPendingTasks = taskManager.getEfemPendingTasks(); //要上料的晶圆
					efemReturnPendingTasks = taskManager.getEfemRuturnPendingTasks(); //已经到LL上的晶圆
					efemReturnCompletedTasks = taskManager.getEfemRuturnCompletedTasks(); //EFEM下料完成的任务

					if (tool_allow_get_wafer || efemPendingTasks.size() >0 )//给tool上料
					{
						efem_auto_step = 100;
						if (ui->disabledefem->checkState() == Qt::CheckState::Checked)
							efem_auto_step = 3000;
					}
					else if (tool_allow_put_wafer || efemReturnPendingTasks.size() >0 )//给tool下料
					{
						efem_auto_step = 200;
						if (ui->disabledefem->checkState() == Qt::CheckState::Checked)
							efem_auto_step = 4000;
					}
					else if (efemUnkownStatusTasks.size()>0 && (efemUnkownStatusTasks.size() == efemReturnCompletedTasks.size()))
					{
						if(efemUnkownStatusTasks.at(0).source == UnifiedWaferTask::Location::LP1)
						{
							lp1_cycle_one_time_finished = true;//一次lp1循环完成
						}
						else
						{
							lp2_cycle_one_time_finished = true;//一次lp2循环完成
						}
						Sleep(10);
					}
					else
					{
						Sleep(10);//没任务需求，cpu切到其他线程
					}
				}
				break;
				#pragma region 给TOOL上料
				case 100:
				{
					if (efemUnkownStatusTasks.size() > 0)
					{
						elp = efemUnkownStatusTasks.at(0).source == UnifiedWaferTask::Location::LP1 ? elp1 : elp2;//UI中确定的数据，elp就是确定的
						lk = efemUnkownStatusTasks.at(0).target == UnifiedWaferTask::Location::LLA ? lk1 : lk2;
						if (elp->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
							if (!elp->hasDoorOpend()) {
								auto cmd = elp->createOpenBoxCommand();
								elp->startCommand(cmd);
								cmd->wait();
								if (cmd->hasError())
								{
									logFailedExcuteCommandHasError(elp->getName(), "打开晶圆盒", efem_process_name, efem_auto_step);
								}
								else {
									efem_auto_step = 101;
								}
							}
							else {
								auto cmd = elp->createGetMapCommand();
								elp->startCommand(cmd);
								cmd->wait();
								if (cmd->hasError())
								{
									logFailedExcuteCommandHasError(elp->getName(), "获取MAP", efem_process_name, efem_auto_step);
								}
								else {
									efem_auto_step = 101;
								}
							}

						}
						else {
							logFailedNotNormal(elp->getName(), efem_process_name, efem_auto_step);
						}
					}

				}
				break;
				case 101:
				{
					auto cassManager = lk->getKernel()->getKernelModule<FortrendCassetteManager>();
					auto station_cass_lk = cassManager->getCassette(lk.get());
					auto lkmaps = station_cass_lk->getAllMapping();//查看

					int count = 0;
					if (efemUnkownStatusTasks.size() == 1)
					{
						count = 1;
					}
					else if(efemUnkownStatusTasks.size() > 1)
					{
						count = lkmaps.size();
					}
					for (int i = 0; i < count; i++)
					{
						if (lkmaps[i] == Cassette::Empty) //空片的槽号
						{
							if (efemUnkownStatusTasks.size() > 0)
							{
								//更新为成LL待上料任务类型
								taskManager.updateTaskStatus(efemUnkownStatusTasks[i].taskId, UnifiedWaferTask::TaskType::EFEM_TRANSFER,UnifiedWaferTask::Status::QUEUED);
							}
						}
					}
					efem_auto_step = 110;
				}
				break;
				
				case 110:
				{
					efemPendingTasks = taskManager.getEfemPendingTasks();
					if (efemPendingTasks.size() == 1)
					{
						efem_auto_step = 115;//单取LP单放LK
					}
					else if (efemPendingTasks.size() >= 2)
					{
						efem_auto_step = 150;//双取LP双放LK
					}
					else
					{//上料完成
						if (elp && elp->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
							if (elp->hasDoorOpend())
							{
								auto cmd = elp->createCloseBoxCommand();
								elp->startCommand(cmd);
								cmd->wait();
								if (cmd->hasError())
								{
									logFailedExcuteCommandHasError(elp->getName(), "关闭晶圆盒", efem_process_name, efem_auto_step);
								}
								else
								{
									efem_auto_step = 10; //跳转到开始步骤
								}
							}
						}

						if (!elp->hasDoorOpend())
						{
							tool_allow_get_wafer = false; //LP1上料完成
							efem_auto_step = 10; //跳转到开始步骤
						}
					}
				}
				break;
				#pragma region 单取LP单放LK
				case 115:
				{
					//打开晶圆盒
					if (elp && elp->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						if (!elp->hasDoorOpend()) {
							auto cmd = elp->createOpenBoxCommand();
							elp->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError())
							{
								logFailedExcuteCommandHasError(elp->getName(), "打开晶圆盒", efem_process_name, efem_auto_step);
							}
							else {
								efem_auto_step = 120;
							}
						}
						else {
							efem_auto_step = 120;
						}
					}
					else {
						logFailedNotNormal(elp->getName(), efem_process_name, efem_auto_step);
					}
					logInform(elp->getName().c_str(), "打开晶圆盒,step:%d", efem_auto_step);
				}
				break;
				case 120://LP1上料
				{
					logInform(elp->getName().c_str(), "取LP晶圆,step:%d", efem_auto_step);

					if (elp && elp->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto cmd = ewtr->createGetCommand(elp, 1, efemPendingTasks[0].sourceSlot);//1:1手，2：2手
						ewtr->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(ewtr->getName(), "取LP晶圆", efem_process_name, efem_auto_step);
						}
						else
						{
							efem_auto_step = 130;
						}
					}
					else {
						logFailedNotNormal(elp->getName(), efem_process_name, efem_auto_step);
					}

				}
				break;
				case 130://LP1上料
				{
					logInform(elp->getName().c_str(), "LL打开晶圆盒门,step:%d", efem_auto_step);

					if (lk && lk->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						if (!lk->getCassetteDoorOpend())
						{
							auto cmdopendoor = lk->createOpenCassetteDoorCommand();

							lk->startCommand(cmdopendoor);
							cmdopendoor->wait();
							if (cmdopendoor->hasError())
							{
								logFailedExcuteCommandHasError(lk->getName(), "LL打开晶圆盒门", efem_process_name, efem_auto_step);
							}
						}
						else {
							Sleep(2000);
							efem_auto_step = 135;
						}

					}
					else {
						logFailedNotNormal(lk1->getName(), efem_process_name, efem_auto_step);
					}
				}
				break;
				case 135://LP1上料
				{
					logInform(elp->getName().c_str(), "放LL晶圆,step:%d", efem_auto_step);
					if (ewtr && ewtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto cmd2 = ewtr->createPutCommand(lk, 1, efemPendingTasks[0].targetSlot);
						ewtr->startCommand(cmd2);
						cmd2->wait();
						if (cmd2->hasError())
						{
							logFailedExcuteCommandHasError(ewtr->getName(), "放LL晶圆", efem_process_name, efem_auto_step);
						}
						else
						{
							//更新指定ID task状态改变，转移到其他状态下的队列中
							taskManager.updateTaskStatus(efemPendingTasks[0].taskId, UnifiedWaferTask::TaskType::EFEM_TRANSFER, UnifiedWaferTask::Status::COMPLETED);
							taskManager.updateTaskStatus(efemPendingTasks[0].taskId, UnifiedWaferTask::TaskType::LOADLOCK_TRANSFER, UnifiedWaferTask::Status::QUEUED);
							efem_auto_step = 110; //跳转到110
						}
					}
					else
					{
						logFailedNotNormal(ewtr->getName(), efem_process_name, efem_auto_step);
					}
				}
				break;
				#pragma endregion

				#pragma region 双取LP双放LK
				case 150:
				{
					elp = efemPendingTasks[0].source == UnifiedWaferTask::Location::LP1 ? elp1 : elp2;

					if (elp && elp->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						if (!elp->hasDoorOpend())
						{
							auto cmd = elp->createOpenBoxCommand();
							elp->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError())
							{
								logFailedExcuteCommandHasError(elp->getName(), "打开晶圆盒", efem_process_name, efem_auto_step);
							}
							else {
								efem_auto_step = 151;
							}
						}
						else {
							efem_auto_step = 151;
						}
					}
					else {
						logFailedNotNormal(elp->getName(), efem_process_name, efem_auto_step);
					}
				}
				break;
				case 151:
				{
					elp = efemPendingTasks[1].source == UnifiedWaferTask::Location::LP1 ? elp1 : elp2;

					if (elp && elp->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						if (!elp->hasDoorOpend())
						{
							auto cmd = elp->createOpenBoxCommand();
							elp->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError())
							{
								logFailedExcuteCommandHasError(elp->getName(), "打开晶圆盒", efem_process_name, efem_auto_step);
							}
							else {
								efem_auto_step = 152;
							}
						}
						else {
							efem_auto_step = 152;
						}
					}
					else {
						logFailedNotNormal(elp->getName(), efem_process_name, efem_auto_step);
					}
				}
				break;
				case 152:
				{
					elp = efemPendingTasks[0].source == UnifiedWaferTask::Location::LP1 ? elp1 : elp2;
					if (elp && elp->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto cmd = ewtr->createGetCommand(elp, 1, efemPendingTasks[0].sourceSlot);
						ewtr->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(ewtr->getName(), "A手臂取LP晶圆", efem_process_name, efem_auto_step);
						}
						else
						{
							efem_auto_step = 154;
						}
					}
					else {
						logFailedNotNormal(elp->getName(), efem_process_name, efem_auto_step);
					}
				}
				break;
				case 154:
				{
					elp = efemPendingTasks[1].source == UnifiedWaferTask::Location::LP1 ? elp1 : elp2;
					if (elp && elp->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto cmd = ewtr->createGetCommand(elp, 2, efemPendingTasks[1].sourceSlot);
						ewtr->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(ewtr->getName(), "B手臂取LP晶圆", efem_process_name, efem_auto_step);
						}
						else
						{
							efem_auto_step = 155;
						}
					}
					else {
						logFailedNotNormal(elp->getName(), efem_process_name, efem_auto_step);
					}

				}
				break;
				case 155:
				{
					std::shared_ptr<FortrendLoadLockSubsystem>get_lk = efemPendingTasks.at(0).target == UnifiedWaferTask::Location::LLA ? lk1 : lk2;

					if (get_lk && get_lk->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						if (!get_lk->getCassetteDoorOpend())
						{
							auto cmdopendoor = get_lk->createOpenCassetteDoorCommand();
							get_lk->startCommand(cmdopendoor);
							cmdopendoor->wait();
							if (cmdopendoor->hasError())
							{
								logFailedExcuteCommandHasError(get_lk->getName(), "LL打开晶圆盒门", efem_process_name, efem_auto_step);
							}
						}
						else {
							Sleep(2000);
							efem_auto_step = 156;
						}
					}
					else {
						logFailedNotNormal(get_lk->getName(), efem_process_name, efem_auto_step);
					}
				}
				break;
				case 156:
				{
					std::shared_ptr<FortrendLoadLockSubsystem>get_lk = efemPendingTasks.at(1).target == UnifiedWaferTask::Location::LLA ? lk1 : lk2;

					if (get_lk && get_lk->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						if (!get_lk->getCassetteDoorOpend())
						{
							auto cmdopendoor = get_lk->createOpenCassetteDoorCommand();
							get_lk->startCommand(cmdopendoor);
							cmdopendoor->wait();
							if (cmdopendoor->hasError())
							{
								logFailedExcuteCommandHasError(get_lk->getName(), "LL打开晶圆盒门", efem_process_name, efem_auto_step);
							}
						}
						else {
							Sleep(2000);
							efem_auto_step = 157;
						}
					}
					else {
						logFailedNotNormal(get_lk->getName(), efem_process_name, efem_auto_step);
					}
				}
				break;
				case 157:
				{
					std::shared_ptr<FortrendLoadLockSubsystem>get_lk = efemPendingTasks.at(0).target == UnifiedWaferTask::Location::LLA ? lk1 : lk2;
					if (ewtr && ewtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto cmd2 = ewtr->createPutCommand(get_lk, 1, efemPendingTasks.at(0).targetSlot);
						ewtr->startCommand(cmd2);
						cmd2->wait();
						if (cmd2->hasError())
						{
							logFailedExcuteCommandHasError(ewtr->getName(), "A手臂放LL晶圆", efem_process_name, efem_auto_step);
						}
						else {//放料完成
							efem_auto_step = 158;
							taskManager.updateTaskStatus(efemPendingTasks.at(0).taskId, UnifiedWaferTask::TaskType::EFEM_TRANSFER, UnifiedWaferTask::Status::COMPLETED);
						}
					}
					else {
						logFailedNotNormal(ewtr->getName(), efem_process_name, efem_auto_step);
					}
				}
				break;
				case 158:
				{
					std::shared_ptr<FortrendLoadLockSubsystem>get_lk = efemPendingTasks.at(1).target == UnifiedWaferTask::Location::LLA ? lk1 : lk2;
					if (ewtr && ewtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto cmd2 = ewtr->createPutCommand(get_lk, 2, efemPendingTasks.at(1).targetSlot);
						ewtr->startCommand(cmd2);
						cmd2->wait();
						if (cmd2->hasError())
						{
							logFailedExcuteCommandHasError(ewtr->getName(), "B手臂放LL晶圆", efem_process_name, efem_auto_step);
						}
						else {//放料完成
							taskManager.updateTaskStatus(efemPendingTasks.at(1).taskId, UnifiedWaferTask::TaskType::EFEM_TRANSFER, UnifiedWaferTask::Status::COMPLETED);
							efem_auto_step = 110;
						}
					}
					else {
						logFailedNotNormal(ewtr->getName(), efem_process_name, efem_auto_step);
					}
				}
				break;
				#pragma endregion

				#pragma endregion

				#pragma region 给TOOL下料
				case 200:
				{
					//efemReturnPendingTasks 这个数组数据来源是 LL腔  8->3

					
					loadLockAReturnCompletedTasks = taskManager.getLoadLockReturnCompletedTasks("LLA");

					loadLockBReturnCompletedTasks = taskManager.getLoadLockReturnCompletedTasks("LLB");

					for (auto& task : loadLockAReturnCompletedTasks)
					{
						taskManager.updateTaskStatus(task.taskId, UnifiedWaferTask::EFEM_RETURN, UnifiedWaferTask::QUEUED);
					}

					for (auto& task : loadLockBReturnCompletedTasks)
					{
						taskManager.updateTaskStatus(task.taskId, UnifiedWaferTask::EFEM_RETURN, UnifiedWaferTask::QUEUED);
					}

					UpdateEfemSubTransferDatas();

					elp = efemReturnPendingTasks.at(0).source == UnifiedWaferTask::Location::LP1 ? elp1 : elp2;//下料，实际源头和目标互换
					lk = efemReturnPendingTasks.at(0).target == UnifiedWaferTask::Location::LLA ? lk1 : lk2;

					if (elp && elp->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						if (!elp->hasDoorOpend()) {
							auto cmd = elp->createOpenBoxCommand();
							elp->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError())
							{
								logFailedExcuteCommandHasError(elp->getName(), "打开晶圆盒", efem_process_name, efem_auto_step);
							}
							else {
								efem_auto_step = 201;
							}
						}
						else {
							auto cmd = elp->createGetMapCommand();
							elp->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError())
							{
								logFailedExcuteCommandHasError(elp->getName(), "获取MAP", efem_process_name, efem_auto_step);
							}
							else {
								efem_auto_step = 201;
							}
						}
					}
				}
				break;
				case 201:
				{
					UpdateEfemSubTransferDatas();
					if (efemReturnPendingTasks.size() == 1)
					{
						efem_auto_step = 240;
					}
					else if (efemReturnPendingTasks.size() >= 2)
					{
						efem_auto_step = 250;
					}
					else
					{
						//下料完成
						if (elp1 && elp1->getState() == IKernelSubSystem::State::SUB_NORMAL) {
							if (elp1->hasDoorOpend()) {
								auto cmd = elp1->createCloseBoxCommand();
								elp1->startCommand(cmd);
								cmd->wait();
								if (cmd->hasError())
								{
									logFailedExcuteCommandHasError(elp1->getName(), "关闭晶圆盒", efem_process_name, efem_auto_step);
								}
							}
						}
						else {
							logFailedNotNormal(elp1->getName(), efem_process_name, efem_auto_step);
						}

						if (elp2 && elp2->getState() == IKernelSubSystem::State::SUB_NORMAL) {
							if (elp2->hasDoorOpend()) {
								auto cmd = elp2->createCloseBoxCommand();
								elp2->startCommand(cmd);
								cmd->wait();
								if (cmd->hasError())
								{
									logFailedExcuteCommandHasError(elp2->getName(), "关闭晶圆盒", efem_process_name, efem_auto_step);
								}
							}
						}
						else {
							logFailedNotNormal(elp1->getName(), efem_process_name, efem_auto_step);
						}
						if (!elp2->hasDoorOpend() && !elp1->hasDoorOpend())
						{
							tool_allow_put_wafer = false;//下料到LP完成
							efem_auto_step = 10;
						}
					}
				}
				break;
#pragma region 单取LK单放LP
				case 240:
				{
					if (elp && elp->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						if (!elp->hasDoorOpend()) {
							auto cmd = elp->createOpenBoxCommand();
							elp->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError())
							{
								logFailedExcuteCommandHasError(elp->getName(), "打开晶圆盒", efem_process_name, efem_auto_step);
							}
							else {
								efem_auto_step = 241;
							}
						}
						else {
							efem_auto_step = 241;
						}
					}
					else {
						logFailedNotNormal(elp->getName(), efem_process_name, efem_auto_step);
					}
				}
				break;
				case 242:
				{
					if (lk && lk->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						if (!lk->getCassetteDoorOpend()) {
							auto cmdopendoor = lk->createOpenCassetteDoorCommand();
							lk->startCommand(cmdopendoor);
							cmdopendoor->wait();
							if (cmdopendoor->hasError())
							{
								logFailedExcuteCommandHasError(lk->getName(), "LL打开晶圆盒门", efem_process_name, efem_auto_step);
							}
							else {
								efem_auto_step = 243;
								Sleep(2000);
							}
						}
						else {
							efem_auto_step = 243;
							Sleep(2000);
						}
					}
					else {
						logFailedNotNormal(lk->getName(), efem_process_name, efem_auto_step);
					}

				}
				break;
				case 243:
				{
					if (ewtr && ewtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto cmd1 = ewtr->createGetCommand(lk, 1, efemReturnPendingTasks.at(0).targetSlot);
						ewtr->startCommand(cmd1);
						cmd1->wait();
						if (cmd1->hasError())
						{
							logFailedExcuteCommandHasError(ewtr->getName(), "取LLA晶圆", efem_process_name, efem_auto_step);
						}
						else
						{
							efem_auto_step = 244;
						}
					}
					else {
						logFailedNotNormal(ewtr->getName(), efem_process_name, efem_auto_step);
					}
				}
				break;
				case 244:
				{
					if (ewtr && ewtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto cmd2 = ewtr->createPutCommand(elp, 1, efemReturnPendingTasks[0].sourceSlot);
						ewtr->startCommand(cmd2);
						cmd2->wait();
						if (cmd2->hasError())
						{
							logFailedExcuteCommandHasError(ewtr->getName(), "放LP晶圆", efem_process_name, efem_auto_step);
						}

						else
						{
							taskManager.updateTaskStatus(efemReturnPendingTasks[0].taskId, UnifiedWaferTask::TaskType::EFEM_RETURN, UnifiedWaferTask::Status::COMPLETED);
							
							UpdateEfemSubTransferDatas();
							
							if (efemUnkownStatusTasks.size() == 0
								&& efemReturnPendingTasks.size() ==1)
							{
								if (efemReturnPendingTasks.at(0).source == UnifiedWaferTask::Location::LP1)
								{
									logInform(ewtr->getName().c_str(), "cycle end efemUnkownStatusTasks=%d efemReturnPendingTasks=%d",
										efemUnkownStatusTasks.size(), efemReturnPendingTasks.size());
									is_lp1_cycle = true;
								}
								else if (efemReturnPendingTasks.at(0).source == UnifiedWaferTask::Location::LP2)
								{
									logInform(ewtr->getName().c_str(), "cycle end efemUnkownStatusTasks=%d efemReturnPendingTasks=%d",
										efemUnkownStatusTasks.size(), efemReturnPendingTasks.size());
									is_lp2_cycle = true;
								}
							}
							
							efem_auto_step = 201;
						}

					}
					else
					{
						logFailedNotNormal(ewtr->getName(), efem_process_name, efem_auto_step);
					}
				}
				break;

#pragma endregion

#pragma region 双取LK双放LP
				case 250:
				{
					UpdateEfemSubTransferDatas();
					std::shared_ptr<EFEMLPSubsystem> elp = efemReturnPendingTasks.at(0).source == UnifiedWaferTask::Location::LP1 ? elp1 : elp2;
					std::shared_ptr<EFEMLPSubsystem> elp2put = efemReturnPendingTasks.at(1).source == UnifiedWaferTask::Location::LP1 ? elp1 : elp2;

					bool elpopen = false;
					bool elp2open = false;

					if (elp && elp->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						if (!elp->hasDoorOpend()) {
							auto cmd = elp->createOpenBoxCommand();
							elp->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError())
							{
								logFailedExcuteCommandHasError(elp->getName(), "打开晶圆盒", efem_process_name, efem_auto_step);
							}
							else {
								elpopen = true;
							}
						}
						else {
							elpopen = true;
						}
					}
					else {
						logFailedNotNormal(elp->getName(), efem_process_name, efem_auto_step);
					}

					if (elp2put && elp2put->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						if (!elp2put->hasDoorOpend()) {
							auto cmd = elp2put->createOpenBoxCommand();
							elp2put->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError())
							{
								logFailedExcuteCommandHasError(elp2put->getName(), "打开晶圆盒", efem_process_name, efem_auto_step);
							}
							else {
								elp2open = true;
							}
						}
						else {
							elp2open = true;
						}
					}
					else {
						logFailedNotNormal(elp2put->getName(), efem_process_name, efem_auto_step);
					}

					if (elpopen && elp2open) {
						efem_auto_step = 251;
					}
				}
				break;
				case 251:
				{
					std::shared_ptr<FortrendLoadLockSubsystem> lk1 = efemReturnPendingTasks.at(0).target == UnifiedWaferTask::Location::LLA ? lk1 : lk2;
					std::shared_ptr<FortrendLoadLockSubsystem> lk2 = efemReturnPendingTasks.at(1).target == UnifiedWaferTask::Location::LLA ? lk1 : lk2;

					bool lkopen = false;
					bool lk2open = false;
					if (lk1 && lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						if (!lk1->getCassetteDoorOpend()) {
							auto cmdopendoor = lk1->createOpenCassetteDoorCommand();
							lk1->startCommand(cmdopendoor);
							cmdopendoor->wait();
							if (cmdopendoor->hasError())
							{
								logFailedExcuteCommandHasError(lk1->getName(), "LL打开晶圆盒门", efem_process_name, efem_auto_step);
							}
							else {
								lkopen = true;

								Sleep(2000);
							}
						}
						else {
							lkopen = true;

							Sleep(2000);
						}
					}
					else {
						logFailedNotNormal(lk1->getName(), efem_process_name, efem_auto_step);
					}

					if (lk2 && lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						if (!lk2->getCassetteDoorOpend()) {
							auto cmdopendoor = lk2->createOpenCassetteDoorCommand();
							lk2->startCommand(cmdopendoor);
							cmdopendoor->wait();
							if (cmdopendoor->hasError())
							{
								logFailedExcuteCommandHasError(lk2->getName(), "LL打开晶圆盒门", efem_process_name, efem_auto_step);
							}
							else {
								lk2open = true;
								Sleep(2000);
							}
						}
						else {
							lk2open = true;
							Sleep(2000);
						}
					}
					else {
						logFailedNotNormal(lk2->getName(), efem_process_name, efem_auto_step);
					}

					if (lk2open && lkopen)
					{
						efem_auto_step = 253;
					}
				}
				break;
				case 253:
				{
					std::shared_ptr<FortrendLoadLockSubsystem> lk1 = efemReturnPendingTasks.at(0).target == UnifiedWaferTask::Location::LLA ? lk1 : lk2;

					if (ewtr && ewtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto cmd1 = ewtr->createGetCommand(lk1, 1, efemReturnPendingTasks[0].targetSlot);
						ewtr->startCommand(cmd1);
						cmd1->wait();
						if (cmd1->hasError())
						{
							logFailedExcuteCommandHasError(ewtr->getName(), "A手臂取LL晶圆", efem_process_name, efem_auto_step);
						}
						else
						{
							efem_auto_step = 254;
						}
					}
					else {
						logFailedNotNormal(ewtr->getName(), efem_process_name, efem_auto_step);
					}
				}
				break;
				case 254:
				{
					std::shared_ptr<FortrendLoadLockSubsystem> lk2 = efemReturnPendingTasks.at(1).target == UnifiedWaferTask::Location::LLA ? lk1 : lk2;

					if (ewtr && ewtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto cmd1 = ewtr->createGetCommand(lk2, 2, efemReturnPendingTasks[1].targetSlot);
						ewtr->startCommand(cmd1);
						cmd1->wait();
						if (cmd1->hasError())
						{
							logFailedExcuteCommandHasError(ewtr->getName(), "B手臂取LL晶圆", efem_process_name, efem_auto_step);
						}
						else
						{
							efem_auto_step = 255;
						}
					}
					else {
						logFailedNotNormal(ewtr->getName(), efem_process_name, efem_auto_step);
					}
				}
				break;
				case 256:
				{

					if (ewtr && ewtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						std::shared_ptr<EFEMLPSubsystem> elp = efemReturnPendingTasks.at(0).source == UnifiedWaferTask::Location::LP1 ? elp1 : elp2;
						std::shared_ptr<EFEMLPSubsystem> elp2put = efemReturnPendingTasks.at(1).source == UnifiedWaferTask::Location::LP1 ? elp1 : elp2;

						auto cmd3 = ewtr->createPutCommand(elp, 1, efemReturnPendingTasks[0].sourceSlot);
						ewtr->startCommand(cmd3);

						auto cmd4 = ewtr->createPutCommand(elp2put, 2, efemReturnPendingTasks[1].sourceSlot);
						ewtr->startCommand(cmd4);

						cmd3->wait();
						cmd4->wait();

						if (cmd3->hasError())
						{
							logFailedExcuteCommandHasError(ewtr->getName(), "A手臂放LP晶圆", efem_process_name, efem_auto_step);
						}
						else
						{
							efem_auto_step = 257;
						}

						if (cmd4->hasError())
						{
							logFailedExcuteCommandHasError(ewtr->getName(), "B手臂放LP晶圆", efem_process_name, efem_auto_step);
						}
						else
						{
							efem_auto_step = 257;
						}
					}
					else {
						logFailedNotNormal(ewtr->getName(), efem_process_name, efem_auto_step);
					}
				}
				case 257:
				{
					taskManager.updateTaskStatus(efemReturnPendingTasks.at(0).taskId, UnifiedWaferTask::TaskType::EFEM_RETURN, UnifiedWaferTask::Status::COMPLETED);
					taskManager.updateTaskStatus(efemReturnPendingTasks.at(1).taskId, UnifiedWaferTask::TaskType::EFEM_RETURN, UnifiedWaferTask::Status::COMPLETED);

					UpdateEfemSubTransferDatas();

					if (efemUnkownStatusTasks.size() == 0
						&& efemReturnPendingTasks.size() == 2)
					{
						if (efemReturnPendingTasks.at(0).source == UnifiedWaferTask::Location::LP1 && efemReturnPendingTasks.at(0).source == efemReturnPendingTasks.at(1).source)
						{
							logInform(ewtr->getName().c_str(), "cycle end efemUnkownStatusTasks=%d efemReturnPendingTasks=%d",
								efemUnkownStatusTasks.size(), efemReturnPendingTasks.size());
							is_lp1_cycle = true;
						}
						else if (efemReturnPendingTasks.at(0).source == UnifiedWaferTask::Location::LP2 && efemReturnPendingTasks.at(0).source == efemReturnPendingTasks.at(1).source)
						{
							logInform(ewtr->getName().c_str(), "cycle end efemUnkownStatusTasks=%d efemReturnPendingTasks=%d",
								efemUnkownStatusTasks.size(), efemReturnPendingTasks.size());
							is_lp2_cycle = true;
						}

						else if (efemReturnPendingTasks.at(0).source == UnifiedWaferTask::Location::LP1 && efemReturnPendingTasks.at(0).source != efemReturnPendingTasks.at(1).source)
						{
							is_lp1_cycle = true;
						}

						else if (efemReturnPendingTasks.at(0).source == UnifiedWaferTask::Location::LP2 && efemReturnPendingTasks.at(0).source != efemReturnPendingTasks.at(1).source)
						{
							is_lp2_cycle = true;
						}
					}

					efem_auto_step = 201;
				}
				break;

#pragma endregion
#pragma endregion

				#pragma region 模拟EFEM给TOOL上料

				taskManager.updateTaskStatus(loadLockPendingTasks.at(0).taskId, UnifiedWaferTask::TaskType::EFEM_TRANSFER, UnifiedWaferTask::Status::COMPLETED);
				taskManager.updateTaskStatus(loadLockPendingTasks.at(1).taskId, UnifiedWaferTask::TaskType::EFEM_TRANSFER, UnifiedWaferTask::Status::COMPLETED);

				if (!lk1->getCassetteDoorOpend())
				{
					auto cmdopendoor = lk1->createOpenCassetteDoorCommand();
					lk1->startCommand(cmdopendoor);
					cmdopendoor->wait();
					if (cmdopendoor->hasError())
					{
						logFailedExcuteCommandHasError(lk1->getName(), "LLA打开晶圆盒门", efem_process_name, efem_auto_step);
					}
				}
				if (!lk2->getCassetteDoorOpend())
				{
					auto cmdopendoor = lk2->createOpenCassetteDoorCommand();
					lk2->startCommand(cmdopendoor);
					cmdopendoor->wait();
					if (cmdopendoor->hasError())
					{
						logFailedExcuteCommandHasError(lk2->getName(), "LLB打开晶圆盒门", efem_process_name, efem_auto_step);
					}
				}

				Sleep(5000);
				logInform("LLA", "EFEM给LLA上料完成");
				logInform("LLB", "EFEM给LLB上料完成");

				tool_allow_get_wafer = false;
				efem_auto_step = 10;


				#pragma endregion

				#pragma region 模拟EFEM给TOOL下料
				taskManager.updateTaskStatus(efemReturnPendingTasks.at(0).taskId, UnifiedWaferTask::TaskType::EFEM_RETURN, UnifiedWaferTask::Status::COMPLETED);
				taskManager.updateTaskStatus(efemReturnPendingTasks.at(1).taskId, UnifiedWaferTask::TaskType::EFEM_RETURN, UnifiedWaferTask::Status::COMPLETED);

				if (!lk1->getCassetteDoorOpend())
				{
					auto cmdopendoor = lk1->createOpenCassetteDoorCommand();
					lk1->startCommand(cmdopendoor);
					cmdopendoor->wait();
					if (cmdopendoor->hasError())
					{
						logFailedExcuteCommandHasError(lk1->getName(), "LLA打开晶圆盒门", efem_process_name, efem_auto_step);
					}
				}

				if (!lk2->getCassetteDoorOpend())
				{
					auto cmdopendoor = lk2->createOpenCassetteDoorCommand();
					lk2->startCommand(cmdopendoor);
					cmdopendoor->wait();
					if (cmdopendoor->hasError())
					{
						logFailedExcuteCommandHasError(lk2->getName(), "LLB打开晶圆盒门", efem_process_name, efem_auto_step);
					}
				}
				logInform("LLB", "EFEM给LLB下料完成 ");
				tool_allow_put_wafer = false;
				efem_auto_step = 10;

				#pragma endregion
				default:
					break;
				}
			}
			efem_step_once_finished = true;
			Sleep(10);	
		}
	
	}
	/*
	* 大气/真空流程，分为取晶圆，放晶圆，下料到EFEM, 破真空，抽真空
	* 
	*/
	void QSlotTransferCycleVTMWidgetPrivate::executeLLATransfer()
	{

		if (!ewtr || !elp1 || !elp2 || !lk1 || !lk2 || wtr)
		{
			ewtr = kernel->getKernelModule<EFEMWaferRobotSubsystem>("EWTR");
			lk1 = kernel->getKernelModule<FortrendLoadLockSubsystem>("LLA");
			lk2 = kernel->getKernelModule<FortrendLoadLockSubsystem>("LLB");
			wtr = kernel->getKernelModule<FortrendSunwayRobotSubsystem>("WTR");
		}
		while (!taskManager.isStopped())
		{
			Sleep(500);
			if (taskManager.waitForTasks(1000))
			{
				loadlock1_step_once_finished = false;
				switch (loadlock1_auto_step)
				{
				case 10:
				{
					int taskSize = taskManager.getAllTasksSize();
					UpdateLLASubTransferDatas();

					//LLA 服务于LP1
					if (loadLockAPendingTasks.size() == 0 && !is_lp2_cycle && is_lp1_cycle)
					{
						loadlock1_auto_step = 6000;
					}

					if (loadLockAPendingTasks.size() > 0 || loadLockAReturnCompletedTasks.size() > 0 )
					{
						//有wafer，直接抽真空，走取放晶圆流程，下料流程，
						loadlock1_auto_step = 400;
					}
					else if (loadLockACompletedTasks.size() == 2 || loadLockAReturnPendingTasks.size() == 2 || efemUnkownStatusTasks.size() <= taskSize)
					{
						//无wafer,破真空，让efem上料, 此时lp中的wafer状态是unkown
		
						if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
							Sleep(2000);
							if (ui->enableAtmosphere->checkState() == Qt::CheckState::Checked)
							{ //大气
								loadlock1_auto_step = 300;
							}
							else {
								loadlock1_auto_step = 20;
							}
						}
						else
						{
							logFailedNotNormal(lk1->getName(), loadlock1_process_name, loadlock1_auto_step);
						}
					}
					else
					{
						Sleep(500);
					}
				}
				break;
				case 20://需要上晶圆,先破真空
				{
					if (ui->enableAtmosphere->checkState() == Qt::CheckState::Checked)
					{
						logInform("Cycle", "step 20:大气cycle，跳过真空检测");
						loadlock1_auto_step = 300;
					}
					else
					{	
						//排气压力达到设定值9W9  
						if (!lk1->getExhaustVacuumValueReachesTheSetValue())
						{
							loadlock1_auto_step = 100;
						}
						else
						{
							loadlock1_auto_step = 300;//真空值达到大气设定值
						}

					}

				}
				break;
#pragma region 没有晶圆盒的破真空流程
				case 100:
				{
					if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto cmd = lk1->createAutoBreakVacuumCommand();
						lk1->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(lk1->getName(), "自动破真空", loadlock1_process_name, loadlock1_auto_step);
						}
						else {
							loadlock1_auto_step = 300;
						}
					}
					else {
						logFailedNotNormal(lk1->getName(), loadlock1_process_name, loadlock1_auto_step);
					}
				}
				break;
				case 300:
				{
					loadlock1_put_cassette_finished = false;//放料到loadlock1未完成
					loadlock1_auto_step = 301;
				}
			break;
				case 301:
				{
					tool_allow_get_wafer = true;//呼叫LP上料
					loadlock1_auto_step = 302;
				}
				break;
				case 302:
				{
					if (!tool_allow_get_wafer)
					{	//EFEM上料完成
						loadlock1_auto_step = 350;
						loadlock1_put_cassette_finished = true; //放料到loadlock1完成
					}
					else {
						Sleep(50);
					}
				}
				break;
				case 350:
				{
					if (loadlock1_put_cassette_finished && !tool_allow_get_wafer)//上料完成
					{
						loadlock1_auto_step = 400;

						//此时task status 改成LockPendingTasks 
						efemCompletedTasks = taskManager.getEfemCompletedTasks();
						for (auto& task : efemCompletedTasks)
						{
							taskManager.updateTaskStatus(task.taskId, UnifiedWaferTask::TaskType::LOADLOCK_TRANSFER, UnifiedWaferTask::Status::QUEUED);
						}
					}
					else
					{
						Sleep(50);
					}
				}
				break;
#pragma endregion

				case 400://有晶圆盒
				{
					if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
					/*					auto cmd = lk1->createCloseCassetteDoorCommand();
						lk1->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(lk1->getName(), "关闭放晶圆盒门阀", loadlock1_process_name, loadlock1_auto_step);
						}
						else {
							if (ui->enableAtmosphere->checkState() == Qt::CheckState::Checked)
							{
								logInform("Cycle","step:400, 大气cycle，跳过真空检测");
								loadlock1_auto_step = 800; 
							}
							else
							{
								loadlock1_auto_step = 410;
							}
							
						}*/
						if (ui->enableAtmosphere->checkState() == Qt::CheckState::Checked)
						{
							logInform("Cycle", "step:400, 大气cycle，跳过真空检测");
							loadlock1_auto_step = 800;
						}
						else
						{
							loadlock1_auto_step = 410;
						}
					}
					else
					{
						logFailedNotNormal(lk1->getName(), loadlock1_process_name, loadlock1_auto_step);
					}
				}
				break;
				case 410:
				{
					if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						//LoadLock1有真空信号，未达到设定值，角阀未打开
						if (lk1->getVacuumEnable() && (!lk1->getVacuumValueReachesTheSetValue() || !lk1->getAngleValveOpend()))
						{
							loadlock1_auto_step = 500;
						}
						else
						{
							loadlock1_auto_step = 800;
						}
					}
					else {
						logFailedNotNormal(lk1->getName(), loadlock1_process_name, loadlock1_auto_step);
					}
				}
				break;
				case 500:
				{
					if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						loadlock1_get_vacuum = true; //开始抽LoadLock1真空
						loadlock1_auto_step = 510;
					}
					else {
						logFailedNotNormal(lk1->getName(), loadlock1_process_name, loadlock1_auto_step);
					}
				}
				break;
				case 510:
				{
					if (loadlock1_get_vacuum == false)//LoadLock1抽真空完成
					{
						if (lk1->getVacuumValueUpperLimitReachesTheSetValue())
						{
							//Update status
							auto loadlockPendingTasks = taskManager.getLoadLockPendingTasks("LLA");
							for (auto& task : loadlockPendingTasks)
							{
								taskManager.updateTaskStatus(task.taskId, UnifiedWaferTask::TaskType::LOADLOCK_TRANSFER, UnifiedWaferTask::Status::COMPLETED);
							}
							loadlock1_auto_step = 800;
						}
						else
						{
							loadlock1_auto_step = 500;//继续抽
						}
					}
					else
					{
						if (lk1->getVacuumValueReachesTheSetValue() && loadlock1_get_vacuum && (loadlock2_get_vacuum || tm_get_vacuum))
						{
							loadlock1_get_vacuum = false;//真空值已经达到，泵在抽其他腔室
						}
						Sleep(100);
					}
				}
				break;
#pragma region Mapping流程
				case 800:
				{
					if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						if (ui->enableAtmosphere->checkState() == Qt::CheckState::Checked)
						{
							logInform("Cycle","step:800,大气cycle，跳过真空检测");
							loadlock1_auto_step = 810;
						}
						else
						{
							if (lk1->getVacuumEnable() && (!lk1->getVacuumValueReachesTheSetValue() || !lk1->getAngleValveOpend()))
							{
								loadlock1_auto_step = 500;
							}
						}
					}
					else
					{
						logFailedNotNormal(lk1->getName(), loadlock1_process_name, loadlock1_auto_step);
					}
				}
				break;
				case 810:
				{
					if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						loadlock1_auto_step = 900;
					}
					else
					{
						logFailedNotNormal(lk1->getName(), loadlock1_process_name, loadlock1_auto_step);
					}
				}
				break;
#pragma endregion

#pragma region 判断是否取晶圆、放晶圆流程、efem下料的流程
				case 900:
				{
					logInform("Cycle", "step 900:判断是否取晶圆、放晶圆流程、efem下料的流程");
					UpdateLLASubTransferDatas();

					if (loadLockAPendingTasks.size() > 0 || loadLockAReturnPendingTasks.size() > 0)
					{
						loadlock1_auto_step = 901;//取晶圆、放晶圆流程
					}
					else if (loadLockAReturnCompletedTasks.size() > 0) //要兼顾到：若LLa有一片待工艺片，还有一片待efem下料的片，怎么解决？
					{
						loadlock1_auto_step = 5000;//出空casstte的流程
					}
				}
				break;
#pragma endregion
				case 901:
				{
					if (ui->enableAtmosphere->checkState() == Qt::CheckState::Checked)
					{
						logInform("Cycle", "step: 901,大气cycle，跳过真空检测");
						loadlock1_auto_step = 950;
					}
					else
					{
						if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
							if (lk1->getVacuumEnable() && (!lk1->getVacuumValueReachesTheSetValue() || !lk1->getAngleValveOpend()))
							{
								loadlock1_get_vacuum = true;
								loadlock1_auto_step = 920;
							}
							else {
								loadlock1_auto_step = 950;
							}
						}
						else {
							logFailedNotNormal(lk1->getName(), loadlock1_process_name, loadlock1_auto_step);
						}
					}
				}
				break;
				case 920:
				{
					if (loadlock1_get_vacuum == false)
					{
						if (lk1->getVacuumValueUpperLimitReachesTheSetValue())
						{
							loadlock1_auto_step = 950;
						}
						else
						{
							loadlock1_auto_step = 910;
						}
					}
					else {
						if (lk1->getVacuumValueReachesTheSetValue() && loadlock1_get_vacuum && (loadlock2_get_vacuum || tm_get_vacuum))
						{
							loadlock1_get_vacuum = false;//真空值已经达到，泵在抽其他腔室
						}
						Sleep(100);
					}
				}
				break;
				case 950:
				{
					UpdateLLASubTransferDatas();
					if (loadLockAPendingTasks.size() > 0 && !abortCycle)
					{
						//取晶圆
						loadlock1_auto_step = 1000;//允许取晶圆流程
					}
					else if (loadLockAReturnPendingTasks.size() > 0 && !abortCycle)
					{
						//放晶圆
						loadlock1_auto_step = 2000;//允许放晶圆流程
					}
					else if (loadLockAReturnCompletedTasks.size() > 0)
					{
						loadlock1_auto_step = 5000;//出空Cassette流程
					}
					Sleep(500);
				}
				break;

#pragma region 允许取晶圆流程
				case 1000:
				{
					//取片原则，1.是上进下出， 只能取上层的， 2.是上下两层都可取，原则从下到上取，下层：1，上层：2 ，默认第2种
					loadlock1_move_slot_index = loadLockAPendingTasks.at(0).targetSlot; //拿第一个task

					//当取完结束，更新状态
					logInform("Test", "loadlock1_move_slot_index %d loadLockACompletedTasks=%d", loadlock1_move_slot_index, loadLockACompletedTasks.size());
					loadlock1_auto_step = 1010;
				}
				break;
				case 1010:
				{
					if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto cassManager = lk1->getKernel()->getKernelModule<FortrendCassetteManager>();
						auto station_cass = cassManager->getCassette(lk1.get());

						if (station_cass->getMapping(loadlock1_move_slot_index) == Cassette::Mapping::Present)
						{
							loadlock1_auto_step = 1040;
						}
						else
						{
							logFailed(lk1->getName(), Poco::format("%s 第%d槽不是正常片", lk1->getName(), loadlock1_move_slot_index));
						}

					}
					else
					{
						logFailedNotNormal(lk1->getName(), loadlock1_process_name, loadlock1_auto_step);
					}
				}
				break;
				case 1040:
				{
					if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						// 选择不加真空机械手线程，直接操作，真空检测和门阀，阀动作，csr取放动作
						if (CheckLLVacuumMeetsStandard("LLA", 2010))
						{
							loadlock1_auto_step = 1050;
						}
						else
						{
							logFailedNotNormal(lk1->getName(), loadlock1_process_name, loadlock1_auto_step);
						}
					}
					else
					{
						logFailedNotNormal(lk1->getName(), loadlock1_process_name, loadlock1_auto_step);
					}
				}
				break;
				case 1050:
				{
					if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						//LoadLock1 打开传输腔门阀
						auto cmd = lk1->createOpenTMCavityDoorCommand();
						lk1->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(lk1->getName(), "打开传输腔门阀", loadlock1_process_name, loadlock1_auto_step);
						}
						else
						{
							loadlock1_auto_step = 1051;
						}

					}
					else
					{
						logFailedNotNormal(lk1->getName(), loadlock1_process_name, loadlock1_auto_step);
					}

				}
				break;
				case 1051:
				{
					if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL && lk1->hasDoorOpend())
					{
						//robot 用手臂A 从loadlock1 取wafer ，
						auto cmd = wtr->createGetCommand(lk1, loadLockAPendingTasks.at(0).arm, loadlock1_move_slot_index);
						wtr->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(wtr->getName(), "取晶圆", loadlock1_process_name, loadlock1_auto_step);
						}
						else
						{
							loadlock1_auto_step = 1052;
						}
					}
					else
					{
						logFailedNotNormal(wtr->getName(), loadlock1_process_name, loadlock1_auto_step);
					}
				}
				case 1052:
				{
					//真空机械手取料完成
					taskManager.updateTaskStatus(loadLockAPendingTasks.at(0).taskId, UnifiedWaferTask::TaskType::LOADLOCK_TRANSFER, UnifiedWaferTask::Status::COMPLETED);

					//这里要呼叫PM来取片吗？，等待Pm取片成功？
					pm_allow_get_put_wafer = true;

					auto cmd = lk1->createCloseTMCavityDoorCommand();
					lk1->startCommand(cmd);
					cmd->wait();
					if (cmd->hasError())
					{
						logFailedExcuteCommandHasError(lk1->getName(), "关闭传输腔门阀", loadlock1_process_name, loadlock1_auto_step);
					}
					else {
						loadlock1_auto_step = 950;
					}
				}
				break;
#pragma endregion

#pragma region 允许放晶圆流程
				case 2000:
				{
					//放片原则，1.是上进下出， 只能放下层的， 2.是上下两层都可放，原则从下到上放，默认第2种
					//放回到指定ll,对应的槽
					if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						loadlock1_move_slot_index = loadLockReturnPendingTasks.at(0).targetSlot;
						loadlock1_auto_step = 2010;
					}
					else
					{
						logFailedNotNormal(lk1->getName(), loadlock1_process_name, loadlock1_auto_step);
					}
				}
				break;
				case 2010:
				{
					if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto cassManager = lk1->getKernel()->getKernelModule<FortrendCassetteManager>();
						auto station_cass = cassManager->getCassette(lk1.get());
						if (station_cass->getMapping(loadlock1_move_slot_index) == Cassette::Mapping::Empty)
						{
							loadlock1_auto_step = 2030;
						}
						else
						{
							logFailed(lk1->getName(), Poco::format("%s 第%d槽不是空片", lk1->getName(), loadlock1_move_slot_index));
						}

					}
					else
					{
						logFailedNotNormal(lk1->getName(), loadlock1_process_name, loadlock1_auto_step);
					}
				}
				break;
				case 2030:
				{
					if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						// 真空检测和门阀，阀动作，csr取放动作
						if (CheckLLVacuumMeetsStandard("LLA", 2010))
						{
							loadlock1_auto_step = 2040;
						}
						else
						{
							logFailedNotNormal(lk1->getName(), loadlock1_process_name, loadlock1_auto_step);
						}
					}
					else
					{
						logFailedNotNormal(lk1->getName(), loadlock1_process_name, loadlock1_auto_step);
					}

				}
				break;
				case 2040:
				{
					if (tm->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto cmd = tm->createCloseDiaphragmValveCommand(TMCavityValveOpening::TMCavity_Both);
						tm->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(tm->getName(), "关闭隔膜阀失败！", loadlock1_process_name, loadlock1_auto_step);
						}
						else
						{
							loadlock1_auto_step = 2050;
						}
					}
					else
					{
						logFailedNotNormal(tm->getName(), loadlock1_process_name, loadlock1_auto_step);
					}
				}
				break;
				case 2050:
				{
					if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto cmd = lk1->createOpenTMCavityDoorCommand();
						lk1->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(lk1->getName(), "打开传输腔门阀", loadlock1_process_name, loadlock1_auto_step);
						}
						else
						{
							loadlock1_auto_step = 2060;
						}

					}
					else
					{
						logFailedNotNormal(lk1->getName(), loadlock1_process_name, loadlock1_auto_step);
					}
				}
				break;
				case 2060:
				{
					if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL && lk1->hasDoorOpend())
					{
						//robot 把手臂A的放到LoadLock1
						auto cmd = wtr->createPutCommand(lk1, loadLockReturnPendingTasks.at(0).arm, loadlock1_move_slot_index);
						wtr->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(wtr->getName(), "放晶圆", loadlock1_process_name, loadlock1_auto_step);
						}
						else {
							loadlock1_auto_step = 2070;
						}
					}
					else if (lk1->hasDoorOpend() == false)
					{
						logFailed(lk1->getName(), Poco::format("%s 传输腔门阀未打开， %s：%d", lk1->getName(), loadlock1_process_name, loadlock1_auto_step));
					}
					else
					{
						logFailedNotNormal(wtr->getName(), loadlock1_process_name, loadlock1_auto_step);
					}
				}
				break;
				case 2070:
				{
					//下料到LL的晶圆 8->3
					taskManager.updateTaskStatus(loadLockReturnPendingTasks.at(0).taskId, UnifiedWaferTask::LOADLOCK_RETURN, UnifiedWaferTask::COMPLETED);
					taskManager.updateTaskStatus(loadLockReturnPendingTasks.at(0).taskId, UnifiedWaferTask::EFEM_RETURN, UnifiedWaferTask::QUEUED);

					if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto cmd = lk1->createCloseTMCavityDoorCommand();
						lk1->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(lk1->getName(), "关闭传输腔门阀", loadlock1_process_name, loadlock1_auto_step);
						}
						else
						{
							loadlock1_auto_step = 2080;
						}
					}
					else
					{
						Sleep(500);
					}
				}
				break;
				case 2080:
				{

					if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						loadlock1_auto_step = 950;
					}
					else
					{
						logFailedNotNormal(lk1->getName(), loadlock1_process_name, loadlock1_auto_step);
					}

				}
				break;
#pragma endregion

#pragma region 出空casstte的流程
				case 5000:
				{
					if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						if (ui->enableAtmosphere->checkState() == Qt::CheckState::Checked)
						{
							logInform("Cycle", "step:5000 ,大气cycle，跳过破真空");
							loadlock1_auto_step = 5022;
						}
						else
							loadlock1_auto_step = 5021;
					}
					else
					{
						logFailedNotNormal(lk1->getName(), loadlock1_process_name, loadlock1_auto_step);
					}

				}
				break;
				case 5021:
				{
					if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto cmd = lk1->createAutoBreakVacuumCommand();
						lk1->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(lk1->getName(), "自动破真空", loadlock1_process_name, loadlock1_auto_step);
						}
						else {
							loadlock1_auto_step = 5022;
						}
					}
					else {
						logFailedNotNormal(lk1->getName(), loadlock1_process_name, loadlock1_auto_step);
					}
				}
				break;
				case 5022:
				{
					if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						if (ui->enableAtmosphere->checkState() == Qt::CheckState::Checked)
						{
							logInform("Cycle", "大气cycle，跳过破真空");
							loadlock1_auto_step = 5023;
						}
						else
						{
							if (lk1->getFastDiaphragmValveOpend() || lk1->getSlowDiaphragmValveOpend() || lk1->getVacuumValue() <= 99600)
							{
								loadlock1_auto_step = 5021;
							}
						}
						auto cmd = lk1->createOpenCassetteDoorCommand();
						lk1->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(lk1->getName(), "打开放晶圆盒门阀", loadlock1_process_name, loadlock1_auto_step);
						}
						else {
							loadlock1_auto_step = 5023;
						}
					}
					else
					{
						logFailedNotNormal(lk1->getName(), loadlock1_process_name, loadlock1_auto_step);
					}
				}
				break;
				case 5023:
				{
					tool_allow_put_wafer = true;//呼叫EFEM下料
					loadlock1_auto_step = 5024;
				}
				break;
				case 5024:
				{
					if (!tool_allow_put_wafer) {//下料完成
						loadlock1_auto_step = 5025;
					}
					else {
						Sleep(50);
					}
				}
				break;
				case 5025:
				{
					if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto cmd = lk1->createCloseCassetteDoorCommand();
						lk1->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(lk1->getName(), "关闭放晶圆盒门阀", loadlock1_process_name, loadlock1_auto_step);
						}
						else {
							loadlock1_auto_step = 6000;
						}
					}
					else
					{
						logFailedNotNormal(lk1->getName(), loadlock1_process_name, loadlock1_auto_step);
					}
				}
				break;
				case 6000:
				{
					loadLockAPendingTasks = taskManager.getLoadLockPendingTasks("LLA");

					if (loadLockAPendingTasks.size() == 0 && !is_lp2_cycle && is_lp1_cycle)
					{
						lp1_cycle_one_time_finished = true;
						//loadlock1_process_finished = true;
					}
					else
					{
						loadlock1_auto_step = 10;
					}
				}
				break;
#pragma endregion
				default:
					break;
				}
			}
			loadlock1_step_once_finished = true;
		}
	}

	void QSlotTransferCycleVTMWidgetPrivate::executeLLBTransfer()
	{

		if (!ewtr || !elp1 || !elp2 || !lk1 || !lk2 || wtr)
		{
			ewtr = kernel->getKernelModule<EFEMWaferRobotSubsystem>("EWTR");
			lk1 = kernel->getKernelModule<FortrendLoadLockSubsystem>("LLA");
			lk2 = kernel->getKernelModule<FortrendLoadLockSubsystem>("LLB");
			wtr = kernel->getKernelModule<FortrendSunwayRobotSubsystem>("WTR");
		}
		while (!taskManager.isStopped())
		{
			Sleep(500);
			if (taskManager.waitForTasks(1000))
			{
				loadlock2_step_once_finished = false;
				switch (loadlock2_auto_step)
				{
				case 10:
				{
					int taskSize = taskManager.getAllTasksSize();

					UpdateLLBSubTransferDatas();

					if (loadLockBPendingTasks.size() == 0 && !is_lp1_cycle && is_lp2_cycle)
					{
						loadlock2_auto_step = 6000;
					}
					//有无wafer
					if (loadLockBPendingTasks.size() > 0 || loadLockBReturnCompletedTasks.size() > 0)
					{
						//有wafer，直接抽真空，走取放晶圆流程，下料流程，
						loadlock2_auto_step = 400;
					}
					else if (loadLockBCompletedTasks.size() == 2 || loadLockBReturnPendingTasks.size() == 2 || efemUnkownStatusTasks.size() <= taskSize)
					{
						//无wafer,破真空，让efem上料, 此时lp中的wafer状态是unkown

						if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
							Sleep(2000);
							if (ui->enableAtmosphere->checkState() == Qt::CheckState::Checked)
							{ //大气
								loadlock2_auto_step = 300;
							}
							else {
								loadlock2_auto_step = 20;
							}
						}
						else
						{
							logFailedNotNormal(lk2->getName(), loadlock2_process_name, loadlock2_auto_step);
						}
					}

				}
				break;
				case 20://需要上晶圆,先破真空
				{
					if (ui->enableAtmosphere->checkState() == Qt::CheckState::Checked)
					{
						logInform("Cycle", "step 20:大气cycle，跳过真空检测");
						loadlock2_auto_step = 300;
					}
					else
					{
						//排气压力达到设定值9W9  
						if (!lk2->getExhaustVacuumValueReachesTheSetValue())
						{
							loadlock2_auto_step = 100;
						}
						else
						{
							loadlock2_auto_step = 300;//真空值达到大气设定值
						}

					}

				}
				break;
#pragma region 没有晶圆盒的破真空流程
				case 100:
				{
					if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto cmd = lk2->createAutoBreakVacuumCommand();
						lk2->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(lk2->getName(), "自动破真空", loadlock2_process_name, loadlock2_auto_step);
						}
						else {
							loadlock2_auto_step = 300;
						}
					}
					else {
						logFailedNotNormal(lk2->getName(), loadlock2_process_name, loadlock2_auto_step);
					}
				}
				break;
				case 300:
				{
					loadlock2_put_cassette_finished = false;//放料到loadlock2未完成
					loadlock2_auto_step = 301;
				}
				break;
				case 301:
				{
					tool_allow_get_wafer = true;//呼叫LP上料
					loadlock2_auto_step = 302;
				}
				break;
				case 302:
				{
					if (!tool_allow_get_wafer)
					{	//EFEM上料完成
						loadlock2_auto_step = 350;
						loadlock2_put_cassette_finished = true; //放料到loadlock2完成
					}
					else {
						Sleep(50);
					}
				}
				break;
				case 350:
				{
					if (loadlock2_put_cassette_finished && !tool_allow_get_wafer)//上料完成
					{
						//此时task status 改成LockPendingTasks 
						efemCompletedTasks = taskManager.getEfemCompletedTasks();
						for (auto& task : efemCompletedTasks)
						{
							taskManager.updateTaskStatus(task.taskId, UnifiedWaferTask::TaskType::LOADLOCK_TRANSFER, UnifiedWaferTask::Status::QUEUED);
						}
						loadlock2_auto_step = 400;
					}
					else
					{
						Sleep(50);
					}
				}
				break;
#pragma endregion

				case 400://有晶圆盒
				{
					if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						//auto cmd = lk2->createCloseCassetteDoorCommand();
						//lk2->startCommand(cmd);
						//cmd->wait();
						//if (cmd->hasError())
						//{
						//	logFailedExcuteCommandHasError(lk2->getName(), "关闭放晶圆盒门阀", loadlock2_process_name, loadlock2_auto_step);
						//}
						//else {
						//	if (ui->enableAtmosphere->checkState() == Qt::CheckState::Checked)
						//	{
						//		logInform("Cycle", "step:400, 大气cycle，跳过真空检测");
						//		loadlock2_auto_step = 800;
						//	}
						//	else
						//	{
						//		loadlock2_auto_step = 410;
						//	}

						//}
						if (ui->enableAtmosphere->checkState() == Qt::CheckState::Checked)
						{
							logInform("Cycle", "step:400, 大气cycle，跳过真空检测");
							loadlock2_auto_step = 800;
						}
						else
						{
							loadlock2_auto_step = 410;
						}
					}
					else
					{
						logFailedNotNormal(lk2->getName(), loadlock2_process_name, loadlock2_auto_step);
					}
				}
				break;
				case 410:
				{
					if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						//Loadlock2有真空信号，未达到设定值，角阀未打开
						if (lk2->getVacuumEnable() && (!lk2->getVacuumValueReachesTheSetValue() || !lk2->getAngleValveOpend()))
						{
							loadlock2_auto_step = 500;
						}
						else
						{
							loadlock2_auto_step = 800;
						}
					}
					else {
						logFailedNotNormal(lk2->getName(), loadlock2_process_name, loadlock2_auto_step);
					}
				}
				break;
				case 500:
				{
					if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						loadlock2_get_vacuum = true; //开始抽Loadlock2真空
						loadlock2_auto_step = 510;
					}
					else {
						logFailedNotNormal(lk2->getName(), loadlock2_process_name, loadlock2_auto_step);
					}
				}
				break;
				case 510:
				{
					if (loadlock2_get_vacuum == false)//Loadlock2抽真空完成
					{
						if (lk2->getVacuumValueUpperLimitReachesTheSetValue())
						{
							//Update status
							auto loadlockPendingTasks = taskManager.getLoadLockPendingTasks("LLA");
							for (auto& task : loadlockPendingTasks)
							{
								taskManager.updateTaskStatus(task.taskId, UnifiedWaferTask::TaskType::LOADLOCK_TRANSFER, UnifiedWaferTask::Status::COMPLETED);
							}
							loadlock2_auto_step = 800;
						}
						else
						{
							loadlock2_auto_step = 500;//继续抽
						}
					}
					else
					{
						if (lk2->getVacuumValueReachesTheSetValue() && loadlock2_get_vacuum && (loadlock2_get_vacuum || tm_get_vacuum))
						{
							loadlock2_get_vacuum = false;//真空值已经达到，泵在抽其他腔室
						}
						Sleep(100);
					}
				}
				break;
#pragma region Mapping流程
				case 800:
				{
					if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						if (ui->enableAtmosphere->checkState() == Qt::CheckState::Checked)
						{
							logInform("Cycle", "step:800,大气cycle，跳过真空检测");
							loadlock2_auto_step = 810;
						}
						else
						{
							if (lk2->getVacuumEnable() && (!lk2->getVacuumValueReachesTheSetValue() || !lk2->getAngleValveOpend()))
							{
								loadlock2_auto_step = 500;
							}
						}
					}
					else
					{
						logFailedNotNormal(lk2->getName(), loadlock2_process_name, loadlock2_auto_step);
					}
				}
				break;
				case 810:
				{
					if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						loadlock2_auto_step = 900;
					}
					else
					{
						logFailedNotNormal(lk2->getName(), loadlock2_process_name, loadlock2_auto_step);
					}
				}
				break;
#pragma endregion

#pragma region 判断是否取晶圆、放晶圆流程、efem下料的流程
				case 900:
				{
					logInform("Cycle", "step 900:判断是否取晶圆、放晶圆流程、efem下料的流程");
					UpdateLLBSubTransferDatas();

					if (loadLockBPendingTasks.size() > 0 || loadLockBReturnPendingTasks.size() > 0)
					{
						loadlock2_auto_step = 901;//取晶圆、放晶圆流程
					}
					else if (loadLockBReturnCompletedTasks.size() > 0) //要兼顾到：若LLa有一片待工艺片，还有一片待efem下料的片，怎么解决？
					{
						loadlock2_auto_step = 5000;//出空casstte的流程
					}
				}
				break;
#pragma endregion
				case 901:
				{
					if (ui->enableAtmosphere->checkState() == Qt::CheckState::Checked)
					{
						logInform("Cycle", "step: 901,大气cycle，跳过真空检测");
						loadlock2_auto_step = 950;
					}
					else
					{
						if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
							if (lk2->getVacuumEnable() && (!lk2->getVacuumValueReachesTheSetValue() || !lk2->getAngleValveOpend()))
							{
								loadlock2_get_vacuum = true;
								loadlock2_auto_step = 920;
							}
							else {
								loadlock2_auto_step = 950;
							}
						}
						else {
							logFailedNotNormal(lk2->getName(), loadlock2_process_name, loadlock2_auto_step);
						}
					}
				}
				break;
				case 920:
				{
					if (loadlock2_get_vacuum == false)
					{
						if (lk2->getVacuumValueUpperLimitReachesTheSetValue())
						{
							loadlock2_auto_step = 950;
						}
						else
						{
							loadlock2_auto_step = 910;
						}
					}
					else {
						if (lk2->getVacuumValueReachesTheSetValue() && loadlock2_get_vacuum && (loadlock2_get_vacuum || tm_get_vacuum))
						{
							loadlock2_get_vacuum = false;//真空值已经达到，泵在抽其他腔室
						}
						Sleep(100);
					}
				}
				break;
				case 950:
				{
					UpdateLLBSubTransferDatas();
					if (loadLockBPendingTasks.size() > 0 && !abortCycle)
					{
						//取晶圆
						loadlock2_auto_step = 1000;//允许取晶圆流程
					}
					else if (loadLockBReturnPendingTasks.size() > 0 && !abortCycle)
					{
						//放晶圆
						loadlock2_auto_step = 2000;//允许放晶圆流程
					}
					else if (loadLockBReturnCompletedTasks.size() > 0)
					{
						loadlock2_auto_step = 5000;//出空Cassette流程
					}
					Sleep(500);
				}
				break;
#pragma region 允许取晶圆流程
				case 1000:
				{
					//取片原则，1.是上进下出， 只能取上层的， 2.是上下两层都可取，原则从下到上取，下层：1，上层：2 ，默认第2种
					loadlock2_move_slot_index = loadLockBPendingTasks.at(0).targetSlot; //拿第一个task

					//当取完结束，更新状态
					logInform("Test", "loadlock2_move_slot_index %d loadLockBCompletedTasks=%d", loadlock2_move_slot_index, loadLockBCompletedTasks.size());
					loadlock2_auto_step = 1010;
				}
				break;
				case 1010:
				{
					if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto cassManager = lk2->getKernel()->getKernelModule<FortrendCassetteManager>();
						auto station_cass = cassManager->getCassette(lk2.get());

						if (station_cass->getMapping(loadlock2_move_slot_index) == Cassette::Mapping::Present)
						{
							loadlock2_auto_step = 1040;
						}
						else
						{
							logFailed(lk2->getName(), Poco::format("%s 第%d槽不是正常片", lk2->getName(), loadlock2_move_slot_index));
						}

					}
					else
					{
						logFailedNotNormal(lk2->getName(), loadlock2_process_name, loadlock2_auto_step);
					}
				}
				break;
				case 1040:
				{
					if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						// 选择不加真空机械手线程，直接操作，真空检测和门阀，阀动作，csr取放动作
						if (CheckLLVacuumMeetsStandard("LLA", 2010))
						{
							loadlock2_auto_step = 1050;
						}
						else
						{
							logFailedNotNormal(lk2->getName(), loadlock2_process_name, loadlock2_auto_step);
						}
					}
					else
					{
						logFailedNotNormal(lk2->getName(), loadlock2_process_name, loadlock2_auto_step);
					}
				}
				break;
				case 1050:
				{
					if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						//Loadlock2 打开传输腔门阀
						/*	auto cmd = lk2->createOpenTMCavityDoorCommand();
						lk2->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(lk2->getName(), "打开传输腔门阀", robot_process_name, robot_auto_step);
						}
						else
						{
							robot_auto_step = 1051;
						}*/
						loadlock2_auto_step = 1051;

					}
					else
					{
						logFailedNotNormal(lk2->getName(), robot_process_name, loadlock2_auto_step);
					}

				}
				break;
				case 1051:
				{
					if (wtr == nullptr)
					{
						wtr = kernel->getKernelModule<FortrendSunwayRobotSubsystem>("WTR");
					}
					if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						//robot 用手臂A 从loadlock2 取wafer ，
						auto cmd = wtr->createGetCommand(lk2, loadLockBPendingTasks.at(0).arm, loadlock2_move_slot_index);
						wtr->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(wtr->getName(), "取晶圆", robot_process_name, loadlock2_auto_step);
						}
						else
						{
							loadlock2_auto_step = 1052;
						}
					}
					else
					{
						logFailedNotNormal(wtr->getName(), robot_process_name, loadlock2_auto_step);
					}
				}
				case 1052:
				{
					//真空机械手取料完成
					taskManager.updateTaskStatus(loadLockBPendingTasks.at(0).taskId, UnifiedWaferTask::TaskType::LOADLOCK_TRANSFER, UnifiedWaferTask::Status::COMPLETED);

					//这里要呼叫PM来取片吗？
					pm_allow_get_put_wafer = true;

					loadlock2_auto_step = 950;
					//auto cmd = lk2->createCloseTMCavityDoorCommand();
					//lk2->startCommand(cmd);
					//cmd->wait();
					//if (cmd->hasError())
					//{
					//	logFailedExcuteCommandHasError(lk2->getName(), "关闭传输腔门阀", loadlock2_process_name, loadlock2_auto_step);
					//}
					//else {
					//	loadlock2_auto_step = 950;
					//}
				}
				break;
#pragma endregion

#pragma region 允许放晶圆流程
				case 2000:
				{
					//放片原则，1.是上进下出， 只能放下层的， 2.是上下两层都可放，原则从下到上放，默认第2种
					//放回到指定ll,对应的槽
					if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						loadlock2_move_slot_index = loadLockReturnPendingTasks.at(0).targetSlot;
						loadlock2_auto_step = 2010;
					}
					else
					{
						logFailedNotNormal(lk2->getName(), loadlock2_process_name, loadlock2_auto_step);
					}
				}
				break;
				case 2010:
				{
					if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto cassManager = lk2->getKernel()->getKernelModule<FortrendCassetteManager>();
						auto station_cass = cassManager->getCassette(lk2.get());
						if (station_cass->getMapping(loadlock2_move_slot_index) == Cassette::Mapping::Empty)
						{
							loadlock2_auto_step = 2030;
						}
						else
						{
							logFailed(lk2->getName(), Poco::format("%s 第%d槽不是空片", lk2->getName(), loadlock2_move_slot_index));
						}

					}
					else
					{
						logFailedNotNormal(lk2->getName(), loadlock2_process_name, loadlock2_auto_step);
					}
				}
				break;
				case 2030:
				{
					if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						// 真空检测和门阀，阀动作，csr取放动作
						if (CheckLLVacuumMeetsStandard("LLA", 2010))
						{
							loadlock2_auto_step = 2040;
						}
						else
						{
							logFailedNotNormal(lk2->getName(), loadlock2_process_name, loadlock2_auto_step);
						}
					}
					else
					{
						logFailedNotNormal(lk2->getName(), loadlock2_process_name, loadlock2_auto_step);
					}

				}
				break;
				case 2040:
				{
					if (tm->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						//auto cmd = tm->createCloseDiaphragmValveCommand(TMCavityValveOpening::TMCavity_Both);
						//tm->startCommand(cmd);
						//cmd->wait();
						//if (cmd->hasError())
						//{
						//	logFailedExcuteCommandHasError(tm->getName(), "关闭隔膜阀失败！", loadlock2_process_name, loadlock2_auto_step);
						//}
						//else
						//{
						//	loadlock2_auto_step = 2050;
						//}
						loadlock2_auto_step = 2050;
					}
					else
					{
						logFailedNotNormal(tm->getName(), loadlock2_process_name, loadlock2_auto_step);
					}
				}
				break;
				case 2050:
				{
					if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto cmd = lk2->createOpenTMCavityDoorCommand();
						lk2->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(lk2->getName(), "打开传输腔门阀", loadlock2_process_name, loadlock2_auto_step);
						}
						else
						{
							loadlock2_auto_step = 2060;
						}

					}
					else
					{
						logFailedNotNormal(lk2->getName(), loadlock2_process_name, loadlock2_auto_step);
					}
				}
				break;
				case 2060:
				{
					if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL && lk2->hasDoorOpend())
					{
						//robot 把手臂A的放到Loadlock2
						auto cmd = wtr->createPutCommand(lk2, loadLockReturnPendingTasks.at(0).arm, loadlock2_move_slot_index);
						wtr->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(wtr->getName(), "放晶圆", loadlock2_process_name, loadlock2_auto_step);
						}
						else {
							loadlock2_auto_step = 2070;
						}
					}
					else if (lk2->hasDoorOpend() == false)
					{
						logFailed(lk2->getName(), Poco::format("%s 传输腔门阀未打开， %s：%d", lk2->getName(), loadlock2_process_name, loadlock2_auto_step));
					}
					else
					{
						logFailedNotNormal(wtr->getName(), loadlock2_process_name, loadlock2_auto_step);
					}
				}
				break;
				case 2070:
				{
					//下料到LL的晶圆 8->3
					taskManager.updateTaskStatus(loadLockReturnPendingTasks.at(0).taskId, UnifiedWaferTask::LOADLOCK_RETURN, UnifiedWaferTask::COMPLETED);
					
					//taskManager.updateTaskStatus(loadLockReturnPendingTasks.at(0).taskId, UnifiedWaferTask::EFEM_RETURN, UnifiedWaferTask::QUEUED);

					if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						//auto cmd = lk2->createCloseTMCavityDoorCommand();
						//lk2->startCommand(cmd);
						//cmd->wait();
						//if (cmd->hasError())
						//{
						//	logFailedExcuteCommandHasError(lk2->getName(), "关闭传输腔门阀", loadlock2_process_name, loadlock2_auto_step);
						//}
						//else
						//{
						//	loadlock2_auto_step = 2080;
						//}
						loadlock2_auto_step = 2080;
					}
					else
					{
						Sleep(500);
					}
				}
				break;
				case 2080:
				{

					if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						loadlock2_auto_step = 950;
					}
					else
					{
						logFailedNotNormal(lk2->getName(), loadlock2_process_name, loadlock2_auto_step);
					}

				}
				break;
#pragma endregion

#pragma region 出空casstte的流程
				case 5000:
				{
					UpdateLLBSubTransferDatas();
					if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						if (ui->enableAtmosphere->checkState() == Qt::CheckState::Checked)
						{
							logInform("Cycle", "step:5000 ,大气cycle，跳过破真空");
							loadlock2_auto_step = 5022;
						}
						else
							loadlock2_auto_step = 5021;
					}
					else
					{
						logFailedNotNormal(lk2->getName(), loadlock2_process_name, loadlock2_auto_step);
					}

				}
				break;
				case 5021:
				{
					if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto cmd = lk2->createAutoBreakVacuumCommand();
						lk2->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(lk2->getName(), "自动破真空", loadlock2_process_name, loadlock2_auto_step);
						}
						else {
							loadlock2_auto_step = 5022;
						}
					}
					else {
						logFailedNotNormal(lk2->getName(), loadlock2_process_name, loadlock2_auto_step);
					}
				}
				break;
				case 5022:
				{
					if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						if (ui->enableAtmosphere->checkState() == Qt::CheckState::Checked)
						{
							logInform("Cycle", "大气cycle，跳过破真空");
							loadlock2_auto_step = 5023;
						}
						else
						{
							if (lk2->getFastDiaphragmValveOpend() || lk2->getSlowDiaphragmValveOpend() || lk2->getVacuumValue() <= 99600)
							{
								loadlock2_auto_step = 5021;
							}
						}
						auto cmd = lk2->createOpenCassetteDoorCommand();
						lk2->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(lk2->getName(), "打开放晶圆盒门阀", loadlock2_process_name, loadlock2_auto_step);
						}
						else {
							loadlock2_auto_step = 5023;
						}
					}
					else
					{
						logFailedNotNormal(lk2->getName(), loadlock2_process_name, loadlock2_auto_step);
					}
				}
				break;
				case 5023:
				{
					tool_allow_put_wafer = true;//呼叫EFEM下料
					loadlock2_auto_step = 5024;
				}
				break;
				case 5024:
				{
					if (!tool_allow_put_wafer) {//下料完成
						loadlock2_auto_step = 5025;
					}
					else {
						Sleep(50);
					}
				}
				break;
				case 5025:
				{
					if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto cmd = lk2->createCloseCassetteDoorCommand();
						lk2->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(lk2->getName(), "关闭放晶圆盒门阀", loadlock2_process_name, loadlock2_auto_step);
						}
						else {
							loadlock2_auto_step = 6000;
						}
					}
					else
					{
						logFailedNotNormal(lk2->getName(), loadlock2_process_name, loadlock2_auto_step);
					}
				}
				break;
				case 6000:
				{
					if (loadLockBPendingTasks.size() == 0 && !is_lp1_cycle && is_lp2_cycle)
					{
						lp2_cycle_one_time_finished = true;
						//loadlock2_process_finished = true;
					}
					else
					{
						loadlock2_auto_step = 10;
					}
				}
				break;
#pragma endregion
				default:
					break;
				}
			}
			loadlock2_step_once_finished = true;
		}
	}


	/*
		PM具体工艺先不写，处理LL腔体取出的wafer，放到PM1腔内，并延迟10s，并让指定手指取出，改变task状态，放回到原先的LL腔体
	*/
	void QSlotTransferCycleVTMWidgetPrivate::executePM1Transfer()
	{
		std::shared_ptr<FortrendSunwayRobotSubsystem> wtr = kernel->getKernelModule<FortrendSunwayRobotSubsystem>("WTR");
		std::shared_ptr<FortrendPMCavitySubsystem> pm1 = kernel->getKernelModule<FortrendPMCavitySubsystem>("PM1");
		auto cassManager = wtr->getKernel()->getKernelModule<FortrendCassetteManager>();

		while (!taskManager.isStopped())
		{
			Sleep(500);
			pm_step_once_finished = false;

			if (taskManager.waitForTasks(1000))
			{
				switch (pm_auto_step)
				{
					case 10:
					{
						//获取待放片晶圆
						if (pm_allow_get_put_wafer)
						{//允许取放
							loadLockACompletedTasks = taskManager.getLoadLockCompletedTasks("LLA");
							if(loadLockACompletedTasks.size()> 0)
							{
								for (auto& task : loadLockACompletedTasks)
								{
									taskManager.updateTaskStatus(task.taskId, UnifiedWaferTask::TaskType::PM_PROCESS, UnifiedWaferTask::Status::QUEUED);
								}
							}

							loadLockBCompletedTasks = taskManager.getLoadLockCompletedTasks("LLB");
							if(loadLockBCompletedTasks.size()>0 )
							{
								for (auto& task : loadLockBCompletedTasks)
								{
									taskManager.updateTaskStatus(task.taskId, UnifiedWaferTask::TaskType::PM_PROCESS, UnifiedWaferTask::Status::QUEUED);
								}
							}

							UpdatePmSubTransferDatas("PM1");

							pm_auto_step = 100;
						}
						else
						{
							Sleep(200);
						}

						if (pm_allow_goto_craft)
						{//转工艺
							pm_auto_step = 2000;
						}
						else {
							Sleep(200);
						}
					}
					break;

					case 100:
					{
						UpdatePmSubTransferDatas("PM1");
						//判断是放、取,不考虑交互手
						if (pmPendingTasks.size() > 0 || pmCompletedTasks.size() > 0)
						{
							pm_auto_step = 200; //取放片
						}
						else
						{
							Sleep(10);
						}
					}
					break;

					case 200:
					{
						//暂不考虑交互手
						if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
							bool haswaferpm = cassManager->getCassette(pm1.get())->getMapping(1) == Cassette::Present;   //pm中有片
							bool haswaferarm1 = cassManager->getCassette(wtr.get())->getMapping(1) == Cassette::Present; //arm1有片
							bool haswaferarm2 = cassManager->getCassette(wtr.get())->getMapping(2) == Cassette::Present; //arm2有片

							if (!haswaferpm && haswaferarm1) {//手臂1放料
								pm_auto_step = 1010;
							}
							else if (!haswaferpm && haswaferarm2) {//手臂2放料
								pm_auto_step = 1030;
							}
							else if (haswaferpm && !haswaferarm1) //手臂1取料
							{
								pm_auto_step = 1040;
							}
							else if (haswaferpm && !haswaferarm2) //手臂1取料
							{
								pm_auto_step = 1050;
							}
							//else if (haswaferpm && !haswaferarm1 && haswaferarm2) {//手1先取，手2后放
							//	pm_auto_step = 1050;
							//}
							//else if (haswaferpm && haswaferarm1 && !haswaferarm2) {//手2先取，手1后放
							//	pm_auto_step = 1070;
							//}
							//else if (haswaferpm && !haswaferarm2 && !haswaferarm1) {//两个手臂没料，A手取
							//	pm_auto_step = 1090;
							//}
							else {
								logFailedExcuteCommandHasError(wtr->getName(), "机械手晶圆状态不对", pm_process_name, pm_auto_step);
							}
						}
						else
						{
							logFailedNotNormal(wtr->getName(), pm_process_name, pm_auto_step);
						}	
					}
					break;
					case 1010:
					{//放片
						if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
							robot_selected_arm = 0;
							auto cmd = wtr->createPutCommand(pm1, robot_selected_arm, 1);
							wtr->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError())
							{
								logFailedExcuteCommandHasError(wtr->getName(), "放晶圆", pm_process_name, pm_auto_step);
							}
							else
							{
								//下层的
								//taskManager.updateTaskStatus(pmPendingTasks.at(0).taskId,UnifiedWaferTask::TaskType::PM_PROCESS, UnifiedWaferTask::Status::IN_PROGRESS);
								//taskManager.updateTaskStatus(pmPendingTasks.at(0).taskId, UnifiedWaferTask::TaskType::PM_PROCESS, UnifiedWaferTask::Status::COMPLETED);
								//taskManager.updateTaskStatus(pmPendingTasks.at(0).taskId, UnifiedWaferTask::TaskType::LOADLOCK_RETURN, UnifiedWaferTask::Status::QUEUED);

								pm_allow_get_put_wafer = false;
								pm_auto_step = 2000;
							}
						}
						else
						{
							logFailedNotNormal(wtr->getName(), pm_process_name, pm_auto_step);
						}	
					}
					break;
					case 1030:
					{//放片
						if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
							robot_selected_arm = 1;
							auto cmd = wtr->createPutCommand(pm1, robot_selected_arm, 1);
							wtr->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError())
							{
								logFailedExcuteCommandHasError(wtr->getName(), "放晶圆", pm_process_name, pm_auto_step);
							}
							else
							{
								//下层的
								//taskManager.updateTaskStatus(pmPendingTasks.at(0).taskId, UnifiedWaferTask::TaskType::PM_PROCESS, UnifiedWaferTask::Status::IN_PROGRESS);
							
								//taskManager.updateTaskStatus(pmPendingTasks.at(0).taskId, UnifiedWaferTask::TaskType::PM_PROCESS, UnifiedWaferTask::Status::COMPLETED);
								//taskManager.updateTaskStatus(pmPendingTasks.at(0).taskId, UnifiedWaferTask::TaskType::LOADLOCK_RETURN, UnifiedWaferTask::Status::QUEUED);

								pm_allow_get_put_wafer = false;
								pm_auto_step = 2000;
							}
						}
						else
						{
							logFailedNotNormal(wtr->getName(), pm_process_name, pm_auto_step);
						}
					}
					break;
					case 1040:
					{
						//取片
						if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
							robot_selected_arm = 0;
							auto cmd = wtr->createGetCommand(pm1, robot_selected_arm, 1);
							wtr->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError())
							{
								logFailedExcuteCommandHasError(wtr->getName(), "取晶圆", pm_process_name, pm_auto_step);
							}
							else
							{
								//下层的
								taskManager.updateTaskStatus(pmPendingTasks.at(0).taskId, UnifiedWaferTask::TaskType::PM_PROCESS, UnifiedWaferTask::Status::COMPLETED);
								taskManager.updateTaskStatus(pmPendingTasks.at(0).taskId, UnifiedWaferTask::TaskType::LOADLOCK_RETURN, UnifiedWaferTask::Status::QUEUED);

								pm_allow_get_put_wafer = false;
								pm_auto_step = 10;
							}
						}
						else
						{
							logFailedNotNormal(wtr->getName(), pm_process_name, pm_auto_step);
						}
					}
					break;
					case 1050:
					{
						if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
							robot_selected_arm = 1;
							auto cmd = wtr->createGetCommand(pm1, robot_selected_arm, 1);
							wtr->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError())
							{
								logFailedExcuteCommandHasError(wtr->getName(), "取晶圆", pm_process_name, robot_auto_step);
							}
							else
							{
								//下层的
								taskManager.updateTaskStatus(pmPendingTasks.at(0).taskId, UnifiedWaferTask::TaskType::PM_PROCESS, UnifiedWaferTask::Status::COMPLETED);
								taskManager.updateTaskStatus(pmPendingTasks.at(0).taskId, UnifiedWaferTask::TaskType::LOADLOCK_RETURN, UnifiedWaferTask::Status::QUEUED);
								pm_allow_get_put_wafer = false;
								pm_auto_step = 10;
							}
						}
						else
						{
							logFailedNotNormal(wtr->getName(), pm_process_name, robot_auto_step);
						}
					}
					break;
					case 2000:
					{
						Sleep(500);
						logInform("PM1","模拟做工艺流程.....");
						taskManager.updateTaskStatus(pmPendingTasks.at(0).taskId, UnifiedWaferTask::TaskType::PM_PROCESS, UnifiedWaferTask::Status::COMPLETED);

						pmCompletedTasks = taskManager.getPMCompletedTasks("PM1"); //10


						taskManager.updateTaskStatus(pmPendingTasks.at(0).taskId, UnifiedWaferTask::TaskType::LOADLOCK_RETURN, UnifiedWaferTask::Status::QUEUED);//7

						Sleep(10);
						pm_auto_step = 10;
					}
					break;
					default:
						break;
				}

			}
			pm_step_once_finished = true;
		}
	}

	void QSlotTransferCycleVTMWidgetPrivate::executePM2Transfer()
	{
		pmPendingTasks = taskManager.getPMPendingTasks("PM2");
	}

	void QSlotTransferCycleVTMWidgetPrivate::executePM3Transfer()
	{
		pmPendingTasks = taskManager.getPMPendingTasks("PM3");
	}

	void QSlotTransferCycleVTMWidgetPrivate::executePM4Transfer()
	{
		pmPendingTasks = taskManager.getPMPendingTasks("PM4");
	}

	void QSlotTransferCycleVTMWidgetPrivate::executeTMTransfer()
	{

	}

	void QSlotTransferCycleVTMWidgetPrivate::executeUpdateTransferStatus()
	{
		while (!taskManager.isStopped())
		{
			Sleep(500);
			update_step_once_finished = false;

			switch (update_auto_step)
			{
			case 10:
			{
				if (lp1_cycle_one_time_finished && !cycleFinished_lla) {//Lp1的一次Cycle已做完
					update_auto_step = 1030;
				}
				else if (lp2_cycle_one_time_finished && !cycleFinished_llb) {//Lp2的一次Cycle已做完
					update_auto_step = 1040;
				}
				else {
					Sleep(1000);
				}
			}
			break;
			case 1030:
			{
				finished_time_lla++;

				if (is_lp1_cycle)
				{
					is_lp1_cycle = false;
				}
				if (finished_time_lla >= cycle_times_lla)
				{
					//cycle 完成
					logInform("Cycle", Poco::format("LP1 Cycle次数%d已完成", cycle_times_lla).c_str());
					finished_time_lla = 0;
					cycleFinished_lla = true;
				}
				else
				{
					// 4---> 0
					efemReturnCompletedTasks = taskManager.getEfemRuturnCompletedTasks();//4
					for (auto& task : efemReturnCompletedTasks)
					{
						taskManager.updateTaskStatus(task.taskId,UnifiedWaferTask::TaskType::UNKNOWN, UnifiedWaferTask::Status::UNKNOWN_PROGRESS);
					}
				}
				onUpdateCycleInfo();
				lp1_cycle_one_time_finished = false;
				update_auto_step = 10;
			}
			break;
			case 1040:
			{
				finished_time_llb++;
				if (is_lp2_cycle) {
					is_lp2_cycle = false;
				}
				if (finished_time_llb >= cycle_times_llb)
				{
					logInform("Cycle", Poco::format("LP2 Cycle次数%d已完成", cycle_times_llb).c_str());
					finished_time_llb = 0;
					cycleFinished_llb = true;

				}
				else
				{
					efemReturnCompletedTasks = taskManager.getEfemRuturnCompletedTasks();//4
					for (auto& task : efemReturnCompletedTasks)
					{
						taskManager.updateTaskStatus(task.taskId, UnifiedWaferTask::TaskType::UNKNOWN, UnifiedWaferTask::Status::UNKNOWN_PROGRESS);
					}
				}
				onUpdateCycleInfo();
				lp2_cycle_one_time_finished = false;
				update_auto_step = 10;
			
			}
			break;
			default:
				break;
			update_step_once_finished = true;
			Sleep(10);
			}

		}
	}

	bool QSlotTransferCycleVTMWidgetPrivate::CheckTMVacuumMeetsStandard(int preStep)
	{
		
		return false;
	}


	bool QSlotTransferCycleVTMWidgetPrivate::CheckLLVacuumMeetsStandard(std::string llName, int preStep)
	{
		tm = kernel->getKernelModule<FortrendTMCavitySubsystem>("TM");
		lk1 = kernel->getKernelModule<FortrendLoadLockSubsystem>("LLA");
		lk2 = kernel->getKernelModule<FortrendLoadLockSubsystem>("LLB");
		std::shared_ptr<FortrendLoadLockSubsystem> lk;

		bool loadlock_get_vacuum = false;
		if (llName == "PM1")
		{
			lk = lk1;
			loadlock_get_vacuum = loadlock1_get_vacuum;
		}
		else if (llName == "PM2")
		{
			lk = lk2;
			loadlock_get_vacuum = loadlock2_get_vacuum;
		}
		if (ui->enableAtmosphere->checkState() == Qt::CheckState::Checked)
		{
			logInform("cycle", "大气模式，lk和tm腔室不检测极限真空值!");
			return true;
		}
		switch (preStep)
		{
			case 2010:
			{
				if (lk->getVacuumValueUpperLimitReachesTheSetValue())
				{
					robot_auto_step = 2050;
				}
				else
				{
					loadlock_get_vacuum = true;
					robot_auto_step = 2020;
				}
			}
			break;
			case 2020:
			{
				if (loadlock_get_vacuum == false)
				{
					robot_auto_step = 2010;
				}
				else {
					Sleep(100);
				}
			}
			break;
			case 2050:
			{
				if (tm->getTMCavityVacuumValueUpperLimitReachesTheSetValue())
				{
					robot_auto_step = 2100;
				}
				else
				{
					tm_get_vacuum = true;
					robot_auto_step = 2060;
				}
			}
			break;
			case 2060:
			{
				if (tm_get_vacuum == false)
				{
					robot_auto_step = 2050;
				}
				else {
					Sleep(100);
				}
			}
			break;
			case 2100:
			{
				if (lk->getVacuumValueUpperLimitReachesTheSetValue() &&
					tm->getTMCavityVacuumValueUpperLimitReachesTheSetValue())
				{
					logInform("cycle","lk和tm腔室都达到了极限真空值!");
					return true;
				}
				else
				{
					robot_auto_step = 2010;
				}
			}
			break;
			default:
				break;
		}
		return false;
	}


	void QSlotTransferCycleVTMWidgetPrivate::resetAction(){

		if (!taskManager.isStopped())
		{
			return;
		}
		while (!vacumm_step_once_finished || !loadlock1_step_once_finished || !loadlock2_step_once_finished)
		{
			Sleep(100);
		}
		onUpdateControlEnabled("reset_pbt", false);
		onUpdateControlEnabled("execute_pbt", false);
		onUpdateControlEnabled("pause_pbt", false);

		std::shared_ptr<FortrendLoadLockSubsystem> lk1 = kernel->getKernelModule<FortrendLoadLockSubsystem>("LLA");
		std::shared_ptr<FortrendSunwayRobotSubsystem> wtr = kernel->getKernelModule<FortrendSunwayRobotSubsystem>("WTR");
		std::shared_ptr<FortrendTMCavitySubsystem> tm = kernel->getKernelModule<FortrendTMCavitySubsystem>("TM");
		std::shared_ptr<FortrendPMCavitySubsystem> pm1 = kernel->getKernelModule<FortrendPMCavitySubsystem>("PM1");
		std::shared_ptr<FortrendPMCavitySubsystem> pm2 = kernel->getKernelModule<FortrendPMCavitySubsystem>("PM2");
		std::shared_ptr<FortrendPMCavitySubsystem> pm3 = kernel->getKernelModule<FortrendPMCavitySubsystem>("PM3");
		std::shared_ptr<FortrendPMCavitySubsystem> pm4 = kernel->getKernelModule<FortrendPMCavitySubsystem>("PM4");

		std::shared_ptr<FortrendLoadLockSubsystem> lk2 = kernel->getKernelModule<FortrendLoadLockSubsystem>("LLB");
		std::shared_ptr<EFEMWaferRobotSubsystem> ewtr = kernel->getKernelModule<EFEMWaferRobotSubsystem>("EWTR");

		std::shared_ptr<EFEMLPSubsystem> elp1 = kernel->getKernelModule<EFEMLPSubsystem>("ELP1");

		std::shared_ptr<EFEMLPSubsystem> elp2 = kernel->getKernelModule<EFEMLPSubsystem>("ELP2");

		//check modules
		auto cassManager = wtr->getKernel()->getKernelModule<FortrendCassetteManager>();

		bool reset_loop = true;
		bool reset_finish = false;
		int rest_step = 0;
		tower->setOutput(FortrendVTMSignalTower::Output::GREEN_LIGHT, false);
		while (reset_loop)
		{
			switch (rest_step)
			{
			case 0:
			{
				if (wtr->getState() != IKernelSubSystem::State::SUB_UNKNOWN&&
					(ewtr->getState() != IKernelSubSystem::State::SUB_UNKNOWN ||
					 ui->disabledefem->checkState() == Qt::CheckState::Checked)
					)
				{
					logInform(reset_process_name.c_str(), "整机复位开始");
					auto cmd = wtr->createResetCommand();
					wtr->startCommand(cmd);
					
					if (ui->disabledefem->checkState() == Qt::CheckState::Unchecked)
					{
						auto cmd_ewtr = ewtr->createResetCommand();
						ewtr->startCommand(cmd_ewtr);
						cmd_ewtr->wait();

						if (cmd_ewtr->hasError())
						{
							rest_step = 15000;
							logError(reset_process_name.c_str(), Poco::format("%s复位指令执行失败！", cmd_ewtr->getName()).c_str());
						}
					}

					cmd->wait();

					if (cmd->hasError())
					{
						rest_step = 15000;
						logError(reset_process_name.c_str(), Poco::format("%s复位指令执行失败！", wtr->getName()).c_str());
					}
					else
					{
						rest_step = 100;
					}
				}
				else{
					rest_step = 15000;
					logError(reset_process_name.c_str(), Poco::format("%s处于未知状态", wtr->getName()).c_str());
				}

			}
			break;

			case 100:
			{
				if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL &&
					(ewtr->getState() == IKernelSubSystem::State::SUB_NORMAL || ui->disabledefem->checkState() == Qt::CheckState::Checked) &&
					lk1->getState() != IKernelSubSystem::State::SUB_UNKNOWN &&
					lk2->getState() != IKernelSubSystem::State::SUB_UNKNOWN &&
					pm2->getState() != IKernelSubSystem::State::SUB_UNKNOWN &&
					tm->getState() != IKernelSubSystem::State::SUB_UNKNOWN &&
					(elp1->getState() != IKernelSubSystem::State::SUB_UNKNOWN || ui->disabledefem->checkState() == Qt::CheckState::Checked) &&
					elp2->getState() != IKernelSubSystem::State::SUB_UNKNOWN || ui->disabledefem->checkState() == Qt::CheckState::Checked)
				{
					auto cmd_lk1 = lk1->createResetCommand();
					lk1->startCommand(cmd_lk1);

					auto cmd_lk2 = lk2->createResetCommand();
					lk2->startCommand(cmd_lk2);

					auto cmd_pm1 = pm1->createResetCommand();
					pm1->startCommand(cmd_pm1);

					auto cmd_pm2 = pm2->createResetCommand();
					pm2->startCommand(cmd_pm2);

					auto cmd_pm3 = pm3->createResetCommand();
					pm3->startCommand(cmd_pm3);

					auto cmd_pm4 = pm4->createResetCommand();
					pm3->startCommand(cmd_pm4);

					auto cmd_tm = tm->createResetCommand();
					tm->startCommand(cmd_tm);

					if (ui->disabledefem->checkState() == Qt::CheckState::Unchecked){
						auto cmd_elp1 = elp1->createResetCommand();
						elp1->startCommand(cmd_elp1);

						auto cmd_elp2 = elp2->createResetCommand();
						elp2->startCommand(cmd_elp2);

						cmd_elp1->wait();
						cmd_elp2->wait();

						if (cmd_elp1->hasError())
						{
							rest_step = 15000;
							logError(reset_process_name.c_str(), Poco::format("%s复位指令执行失败！", cmd_elp1->getName()).c_str());
						}
						else if (cmd_elp2->hasError())
						{
							rest_step = 15000;
							logError(reset_process_name.c_str(), Poco::format("%s复位指令执行失败！", cmd_elp2->getName()).c_str());
						}
						elp1->setDoorOpend(false);
						elp2->setDoorOpend(false);
					}
					cmd_lk1->wait();
					cmd_lk2->wait();
					cmd_pm1->wait();
					cmd_pm2->wait();
					cmd_pm3->wait();
					cmd_pm4->wait();
					cmd_tm->wait();
					
					if (cmd_lk1->hasError())
					{
						rest_step = 15000;
						logError(reset_process_name.c_str(), Poco::format("%s复位指令执行失败！", lk1->getName()).c_str());
					}
					else if (cmd_lk2->hasError())
					{
						rest_step = 15000;
						logError(reset_process_name.c_str(), Poco::format("%s复位指令执行失败！", lk2->getName()).c_str());
					}
					else if (cmd_pm1->hasError())
					{
					rest_step = 15000;
					logError(reset_process_name.c_str(), Poco::format("%s复位指令执行失败！", pm1->getName()).c_str());
					}
					else if (cmd_pm2->hasError())
					{
						rest_step = 15000;
						logError(reset_process_name.c_str(), Poco::format("%s复位指令执行失败！", pm2->getName()).c_str());
					}
					else if (cmd_pm3->hasError())
					{
					rest_step = 15000;
					logError(reset_process_name.c_str(), Poco::format("%s复位指令执行失败！", pm3->getName()).c_str());
					}
					else if (cmd_pm4->hasError())
					{
						rest_step = 15000;
						logError(reset_process_name.c_str(), Poco::format("%s复位指令执行失败！", pm4->getName()).c_str());
					}
					else if (cmd_tm->hasError())
					{
						rest_step = 15000;
						logError(reset_process_name.c_str(), Poco::format("%s复位指令执行失败！", tm->getName()).c_str());
					}
					else{
						rest_step = 120;
					}
					
				}
				else
				{
					rest_step = 15000;
					if (wtr->getState() != IKernelSubSystem::State::SUB_NORMAL)
					{
						logError(reset_process_name.c_str(), Poco::format("%s不在正常状态", wtr->getName()).c_str());
					}
					if (lk1->getState() == IKernelSubSystem::State::SUB_UNKNOWN)
					{
						logError(reset_process_name.c_str(), Poco::format("%s不在正常状态", lk1->getName()).c_str());
					}
					if (lk2->getState() == IKernelSubSystem::State::SUB_UNKNOWN)
					{
						logError(reset_process_name.c_str(), Poco::format("%s不在正常状态", lk2->getName()).c_str());
					}
					if (pm1->getState() == IKernelSubSystem::State::SUB_UNKNOWN)
					{
					logError(reset_process_name.c_str(), Poco::format("%s不在正常状态", pm1->getName()).c_str());
					}
					if (pm2->getState() == IKernelSubSystem::State::SUB_UNKNOWN)
					{
						logError(reset_process_name.c_str(), Poco::format("%s不在正常状态", pm2->getName()).c_str());
					}
					if (pm3->getState() == IKernelSubSystem::State::SUB_UNKNOWN)
					{
					logError(reset_process_name.c_str(), Poco::format("%s不在正常状态", pm3->getName()).c_str());
					}
					if (pm4->getState() == IKernelSubSystem::State::SUB_UNKNOWN)
					{
						logError(reset_process_name.c_str(), Poco::format("%s不在正常状态", pm4->getName()).c_str());
					}
					if (tm->getState() == IKernelSubSystem::State::SUB_UNKNOWN)
					{
						logError(reset_process_name.c_str(), Poco::format("%s不在正常状态", tm->getName()).c_str());
					}
					if (elp1->getState() == IKernelSubSystem::State::SUB_UNKNOWN)
					{
						logError(reset_process_name.c_str(), Poco::format("%s不在正常状态", elp1->getName()).c_str());
					}
					if (elp2->getState() == IKernelSubSystem::State::SUB_UNKNOWN)
					{
						logError(reset_process_name.c_str(), Poco::format("%s不在正常状态", elp2->getName()).c_str());
					}
				}
			}
			break;
			case 120:
			{
				if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL &&
					lk2->getState() == IKernelSubSystem::State::SUB_NORMAL &&
					pm1->getState() == IKernelSubSystem::State::SUB_NORMAL &&
					pm2->getState() == IKernelSubSystem::State::SUB_NORMAL &&
					pm3->getState() == IKernelSubSystem::State::SUB_NORMAL &&
					pm4->getState() == IKernelSubSystem::State::SUB_NORMAL &&
					tm->getState() == IKernelSubSystem::State::SUB_NORMAL&&
					(ewtr->getState() == IKernelSubSystem::State::SUB_NORMAL || ui->disabledefem->checkState() == Qt::CheckState::Checked) &&
					(elp1->getState() == IKernelSubSystem::State::SUB_NORMAL || ui->disabledefem->checkState() == Qt::CheckState::Checked) &&
					(elp2->getState() == IKernelSubSystem::State::SUB_NORMAL || ui->disabledefem->checkState() == Qt::CheckState::Checked)
					)
				{
					rest_step = 200;

				}
				else
				{
					if (lk1->getState() != IKernelSubSystem::State::SUB_NORMAL)
					{
						logError(reset_process_name.c_str(), Poco::format("%s不在正常状态", lk1->getName()).c_str());
					}

					if (lk2->getState() != IKernelSubSystem::State::SUB_NORMAL)
					{
						logError(reset_process_name.c_str(), Poco::format("%s不在正常状态", lk2->getName()).c_str());
					}
					if (pm1->getState() != IKernelSubSystem::State::SUB_NORMAL)
					{
					logError(reset_process_name.c_str(), Poco::format("%s不在正常状态", pm1->getName()).c_str());
					}
					if (pm2->getState() != IKernelSubSystem::State::SUB_NORMAL)
					{
						logError(reset_process_name.c_str(), Poco::format("%s不在正常状态", pm2->getName()).c_str());
					}
					if (pm3->getState() != IKernelSubSystem::State::SUB_NORMAL)
					{
					logError(reset_process_name.c_str(), Poco::format("%s不在正常状态", pm3->getName()).c_str());
					}
					if (pm4->getState() != IKernelSubSystem::State::SUB_NORMAL)
					{
						logError(reset_process_name.c_str(), Poco::format("%s不在正常状态", pm4->getName()).c_str());
					}
					if (tm->getState() != IKernelSubSystem::State::SUB_NORMAL)
					{
						logError(reset_process_name.c_str(), Poco::format("%s不在正常状态", tm->getName()).c_str());
					}
					if (ewtr->getState() != IKernelSubSystem::State::SUB_NORMAL)
					{
						logError(reset_process_name.c_str(), Poco::format("%s不在正常状态", ewtr->getName()).c_str());
					}
					if (elp1->getState() != IKernelSubSystem::State::SUB_NORMAL)
					{
						logError(reset_process_name.c_str(), Poco::format("%s不在正常状态", elp1->getName()).c_str());
					}
					if (elp2->getState() != IKernelSubSystem::State::SUB_NORMAL)
					{
						logError(reset_process_name.c_str(), Poco::format("%s不在正常状态", elp2->getName()).c_str());
					}
					rest_step = 15000;
				}
			}
			break;

			case 200:
			{
				auto cmd_lk1 = lk1->createCloseTMCavityDoorCommand();
				auto cmd_lk2 = lk1->createCloseTMCavityDoorCommand();
				auto cmd_pm1 = pm1->createCloseTMCavityDoorCommand();
				auto cmd_pm2 = pm2->createCloseTMCavityDoorCommand();
				auto cmd_pm3 = pm3->createCloseTMCavityDoorCommand();
				auto cmd_pm4 = pm4->createCloseTMCavityDoorCommand();

				lk1->startCommand(cmd_lk1);
				lk2->startCommand(cmd_lk2);
				pm1->startCommand(cmd_pm1);
				pm2->startCommand(cmd_pm2);
				pm3->startCommand(cmd_pm3);
				pm4->startCommand(cmd_pm4);

				cmd_lk1->wait();
				cmd_lk2->wait();
				cmd_pm1->wait();
				cmd_pm2->wait();
				cmd_pm3->wait();
				cmd_pm4->wait();

				if (cmd_lk1->hasError())
				{
					rest_step = 15000;
					logError(reset_process_name.c_str(), Poco::format("%s关闭传输腔门阀命令执行失败！", lk1->getName()).c_str());
				}
				else if (cmd_lk2->hasError())
				{
					rest_step = 15000;
					logError(reset_process_name.c_str(), Poco::format("%s关闭传输腔门阀命令执行失败！", lk2->getName()).c_str());
				}
				else if (cmd_pm1->hasError())
				{
				rest_step = 15000;
				logError(reset_process_name.c_str(), Poco::format("%s关闭传输腔门阀命令执行失败！", pm1->getName()).c_str());
				}
				else if (cmd_pm2->hasError())
				{
					rest_step = 15000;
					logError(reset_process_name.c_str(), Poco::format("%s关闭传输腔门阀命令执行失败！", pm2->getName()).c_str());
				}
				else if (cmd_pm3->hasError())
				{
				rest_step = 15000;
				logError(reset_process_name.c_str(), Poco::format("%s关闭传输腔门阀命令执行失败！", pm3->getName()).c_str());
				}
				else if (cmd_pm4->hasError())
				{
					rest_step = 15000;
					logError(reset_process_name.c_str(), Poco::format("%s关闭传输腔门阀命令执行失败！", pm4->getName()).c_str());
				}
				else{
					rest_step = 10000;
				}
			}
			break;

			case 10000:
			{
				logInform(reset_process_name.c_str(), "整机复位完成");
				robot_auto_step = 10;
				loadlock1_auto_step = 10;
				loadlock2_auto_step = 10;
				vacuum_auto_step = 10;
				efem_auto_step = 10;
				pm_auto_step = 10;
				update_auto_step = 10;
				reset_loop = false;
				reset_finish = true;
				rest_step = -1;

			}
			break;
			case 15000:
			{
				logInform(reset_process_name.c_str(), "整机复位失败");
				reset_loop = false;
				reset_finish = false;
				rest_step = -1;

			}
			break;
			default:
				break;
			}
		}
		if (reset_finish)
		{
			tower->setOutput(FortrendVTMSignalTower::Output::YELLOW_LIGHT, true);
			onUpdateControlEnabled("execute_pbt", true);
			onUpdateControlEnabled("pause_pbt", true);

			onUpdateControlEnabled("loadlock1_put_cassette_finished_pbt", false);
			onUpdateControlEnabled("loadlock2_put_cassette_finished_pbt", false);
		}
		onUpdateControlEnabled("reset_pbt", true);

	}

	void QSlotTransferCycleVTMWidgetPrivate::startVacuumAction(){
		std::shared_ptr<FortrendTMCavitySubsystem> tm = kernel->getKernelModule<FortrendTMCavitySubsystem>("TM");
		std::shared_ptr<FortrendPumpSubsystem> pump = kernel->getKernelModule<FortrendPumpSubsystem>("PUMP");
		std::shared_ptr<FortrendLoadLockSubsystem> lk1 = kernel->getKernelModule<FortrendLoadLockSubsystem>("LLA");
		std::shared_ptr<FortrendLoadLockSubsystem> lk2 = kernel->getKernelModule<FortrendLoadLockSubsystem>("LLB");
		while (running)
		{
			vacumm_step_once_finished = false;
			if (tm->getVacuumEnable())
			{
				switch (vacuum_auto_step)
				{
					case 10:
					{
						if (tm_get_vacuum)
						{
							vacuum_auto_step = 5000;
						}
						else if (loadlock1_get_vacuum&&!loadlock2_get_vacuum)//新增LLA、LLB的抽真空顺序判断
						{
							vacuum_auto_step = 10000;
						}
						else if (loadlock2_get_vacuum&&!loadlock1_get_vacuum)
						{
							vacuum_auto_step = 20000;
						}
						else if (loadlock1_get_vacuum&&loadlock2_get_vacuum&&lk1->getVacuumValue()<lk2->getVacuumValue()){
							vacuum_auto_step = 10000;
						}
						else if (loadlock1_get_vacuum&&loadlock2_get_vacuum&&lk1->getVacuumValue()>lk2->getVacuumValue()){
							vacuum_auto_step = 20000;
						}
						else
						{
							Sleep(10);
						}	
						/*if (tm_get_vacuum)
						{
							vacuum_auto_step = 5000;
						}
						else if (loadlock2_get_vacuum)
						{
							vacuum_auto_step = 20000;
						}
						else if (loadlock1_get_vacuum)
						{
							vacuum_auto_step = 10000;
						}
						else
						{
							Sleep(10);
						}*/
					}
					break;
					case 5000:
					{
						auto cmd = pump->createOpenTMCavityAutoVacuumCommand();
						pump->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(pump->getName(), "抽TM真空", vacuum_process_name, 10);
							break;
						}
						else
						{
							tm_get_vacuum = false;//TM抽真空完成标志
							vacuum_auto_step = 10;
						}
					}
				    break;
					case 10000:
					{
						auto cmd = pump->createOpenLoadLock1AutoVacuumCommand();
						pump->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(pump->getName(), "抽LoadLockA真空", vacuum_process_name, 1000);
							break;
						}
						else
						{
							loadlock1_get_vacuum = false;//LoadLock1抽真空完成标志
							vacuum_auto_step = 10;
						}
					}
					break;
					case 20000:
					{
						auto cmd = pump->createOpenLoadLock2AutoVacuumCommand();
						pump->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(pump->getName(), "抽LoadLockB真空", vacuum_process_name, 1010);
							break;
						}
						else
						{
							loadlock2_get_vacuum = false;//LoadLock1抽真空完成标志
							vacuum_auto_step = 10;
						}

					}
					break;
					default:
						break;
				}

			}
			//真空屏蔽
			else
			{
				if (loadlock1_get_vacuum)
				{
					loadlock1_get_vacuum = false;
				}
				if (loadlock2_get_vacuum)
				{
					loadlock2_get_vacuum = false;
				}
				if (tm_get_vacuum)
				{
					tm_get_vacuum = false;
				}
			}

			vacumm_step_once_finished = true;
			Sleep(10);
		}
	}

	void QSlotTransferCycleVTMWidgetPrivate::startEFEMAction(){
		std::shared_ptr<EFEMWaferRobotSubsystem> ewtr = kernel->getKernelModule<EFEMWaferRobotSubsystem>("EWTR");
		std::shared_ptr<EFEMLPSubsystem> elp1 = kernel->getKernelModule<EFEMLPSubsystem>("ELP1");
		std::shared_ptr<EFEMLPSubsystem> elp2 = kernel->getKernelModule<EFEMLPSubsystem>("ELP2");
		std::shared_ptr<FortrendLoadLockSubsystem> lk1 = kernel->getKernelModule<FortrendLoadLockSubsystem>("LLA");
		std::shared_ptr<FortrendLoadLockSubsystem> lk2 = kernel->getKernelModule<FortrendLoadLockSubsystem>("LLB");
		

		}

	void QSlotTransferCycleVTMWidgetPrivate::startUpdateStatusAction(){
		//std::shared_ptr<FortrendTMCavitySubsystem> tm = kernel->getKernelModule<FortrendTMCavitySubsystem>("TM");
		//std::shared_ptr<FortrendPumpSubsystem> pump = kernel->getKernelModule<FortrendPumpSubsystem>("PUMP");
		//std::shared_ptr<FortrendLoadLockSubsystem> lk1 = kernel->getKernelModule<FortrendLoadLockSubsystem>("LLA");
		//std::shared_ptr<FortrendLoadLockSubsystem> lk2 = kernel->getKernelModule<FortrendLoadLockSubsystem>("LLB");

		while (running)
		{
			update_step_once_finished = false;
			switch (update_auto_step)
			{
			case 10:
			{
				if (lp1_cycle_one_time_finished && !cycleFinished_lla){//LLA的一次Cycle已做完
					update_auto_step = 1030;
				}
				else if (lp2_cycle_one_time_finished &&! cycleFinished_llb){//LLB的一次Cycle已做完
					update_auto_step = 1040;
				}
				/*else if (sequence_robot_transfer_wafer.size() == 0 && sequence_robot_get_wafer.size() == 0){
				update_auto_step = 1050;
				}*/
				else{
					Sleep(1000);
				}
			}
			break;
			case 1030:
			{
				finished_time_lla ++;

				if (is_lp1_cycle)
				{
					is_lp1_cycle = false;
				}

				if (finished_time_lla >= cycle_times_lla)
				{
					//onUpdateProcessControlEnabled(true);
					//cycle 完成
					logInform("Cycle", Poco::format("LP1 Cycle次数%d已完成", cycle_times_lla).c_str());
					
					/*running = false;*/
					finished_time_lla = 0;
					//loadlock1_auto_step = 10;

					cycleFinished_lla = true;

					sequence_lp1_transfer_wafer_copy.clear();

					sequence_robot_transfer_wafer_lp1.clear();

				}
				else
				{
					logInform("Cycle", "LP1第%d次Cycle完成 copysize=%d size=%d robot_lla=%d robot=%d", 
						finished_time_lla, 
						sequence_lp1_transfer_wafer_copy.size(),
						sequence_lp1_transfer_wafer.size(),
						sequence_robot_transfer_wafer_lp1.size(),
						sequence_robot_transfer_wafer.size());

					// 将sequence_robot_transfer_wafer_lp1中的所有元素添加到sequence_robot_transfer_wafer的末尾

					if (sequence_robot_transfer_wafer_lp1.size()>0)
					{
						std::lock_guard<std::mutex> lock1(vec_mutex_robot);
						sequence_robot_transfer_wafer.insert(sequence_robot_transfer_wafer.end(), sequence_robot_transfer_wafer_lp1.begin(), sequence_robot_transfer_wafer_lp1.end());
					}

					if (sequence_lp1_transfer_wafer_copy.size() > 0){
						std::lock_guard<std::mutex> lock(vec_mutex_lla);
						std::copy(sequence_lp1_transfer_wafer_copy.begin(), sequence_lp1_transfer_wafer_copy.end(), std::back_inserter(sequence_lp1_transfer_wafer));
					}

					if (sequence_tolk1_wafer_copy.size()>0 && sequence_tolk1_wafer.size()==0)
					{
						std::copy(sequence_tolk1_wafer_copy.begin(), sequence_tolk1_wafer_copy.end(), std::back_inserter(sequence_tolk1_wafer));
					}

					if (sequence_tolk2_wafer_copy.size()>0 && sequence_tolk2_wafer.size() == 0)
					{
						std::copy(sequence_tolk2_wafer_copy.begin(), sequence_tolk2_wafer_copy.end(), std::back_inserter(sequence_tolk2_wafer));
					}

				}
				onUpdateCycleInfo();
				lp1_cycle_one_time_finished = false;
				update_auto_step = 10;
			}
			break;
			case 1040:
			{
				finished_time_llb++;
				if (is_lp2_cycle){
					is_lp2_cycle = false;
				}
				if (finished_time_llb >= cycle_times_llb)
				{
					//onUpdateProcessControlEnabled(true);
					//cycle 完成,删除集合做标识位
					logInform("Cycle", Poco::format("LP2 Cycle次数%d已完成", cycle_times_llb).c_str());
					finished_time_llb = 0;
					//loadlock2_auto_step = 10;
					cycleFinished_llb = true;
					sequence_lp2_transfer_wafer_copy.clear();
					sequence_robot_transfer_wafer_lp2.clear();

				}
				else
				{
					logInform("Cycle", "LP2第%d次Cycle完成 copysize=%d size=%d robot_llb=%d robot=%d", finished_time_llb, sequence_lp2_transfer_wafer_copy.size(), sequence_lp2_transfer_wafer.size(), sequence_robot_transfer_wafer_lp2.size(), sequence_robot_transfer_wafer.size());
					//setTransferSequence(); 只初始化时使用，单独更新cycle流程
					
					// 将sequence_robot_transfer_wafer_lp2中的所有元素添加到sequence_robot_transfer_wafer的末尾

					if (sequence_robot_transfer_wafer_lp2.size()>0)
					{
						std::lock_guard<std::mutex> lock1(vec_mutex_robot);
						sequence_robot_transfer_wafer.insert(sequence_robot_transfer_wafer.end(), sequence_robot_transfer_wafer_lp2.begin(), sequence_robot_transfer_wafer_lp2.end());
					}

					if (sequence_lp2_transfer_wafer_copy.size() > 0)
					{
						std::lock_guard<std::mutex> lock(vec_mutex_llb);
						std::copy(sequence_lp2_transfer_wafer_copy.begin(), sequence_lp2_transfer_wafer_copy.end(), std::back_inserter(sequence_lp2_transfer_wafer));
					}

					if (sequence_tolk1_wafer_copy.size()>0 && sequence_tolk1_wafer.size() == 0)
					{
						std::copy(sequence_tolk1_wafer_copy.begin(), sequence_tolk1_wafer_copy.end(), std::back_inserter(sequence_tolk1_wafer));
					}
					if (sequence_tolk2_wafer_copy.size()>0 && sequence_tolk2_wafer.size() == 0)
					{
						std::copy(sequence_tolk2_wafer_copy.begin(), sequence_tolk2_wafer_copy.end(), std::back_inserter(sequence_tolk2_wafer));
					}

				}
				onUpdateCycleInfo();
				lp2_cycle_one_time_finished = false;
				update_auto_step = 10;
			}
			break;
			/*case 1050:
			{
			logInform("Cycle", "WTR Cycle已完成");
			robot_auto_step = 10;
			onUpdateProcessControlEnabled(true);
			running = false;
			}
			break;*/
			default:
				break;
			}
			update_step_once_finished = true;
			Sleep(10);
		}
	}

	void QSlotTransferCycleVTMWidgetPrivate::startRobotAction(){

		std::shared_ptr<FortrendLoadLockSubsystem> lk1 = kernel->getKernelModule<FortrendLoadLockSubsystem>("LLA");
		std::shared_ptr<FortrendSunwayRobotSubsystem> wtr = kernel->getKernelModule<FortrendSunwayRobotSubsystem>("WTR");
		std::shared_ptr<FortrendTMCavitySubsystem> tm = kernel->getKernelModule<FortrendTMCavitySubsystem>("TM");
		std::shared_ptr<FortrendLoadLockSubsystem> lk2 = kernel->getKernelModule<FortrendLoadLockSubsystem>("LLB");
		//std::shared_ptr<FortrendPMCavitySubsystem> pm1 = kernel->getKernelModule<FortrendPMCavitySubsystem>("PM1");
		std::shared_ptr<FortrendPMCavitySubsystem> pm2 = kernel->getKernelModule<FortrendPMCavitySubsystem>("PM2");
		//std::shared_ptr<FortrendPMCavitySubsystem> pm3 = kernel->getKernelModule<FortrendPMCavitySubsystem>("PM3");

		std::shared_ptr<FortrendAlignerSubsystem> aligner = kernel->getKernelModule<FortrendAlignerSubsystem>("Aligner");

		//check modules
		auto cassManager = wtr->getKernel()->getKernelModule<FortrendCassetteManager>();


	}

	void QSlotTransferCycleVTMWidgetPrivate::startLoadLock1Action(){
		Q_Q(QSlotTransferCycleVTMWidget);
		std::shared_ptr<FortrendLoadLockSubsystem> lk1 = kernel->getKernelModule<FortrendLoadLockSubsystem>("LLA");
	}

	void QSlotTransferCycleVTMWidgetPrivate::startLoadLock2Action(){

		std::shared_ptr<FortrendLoadLockSubsystem> lk2 = kernel->getKernelModule<FortrendLoadLockSubsystem>("LLB");

	}

	void QSlotTransferCycleVTMWidgetPrivate::startPMAction(){
		std::shared_ptr<FortrendPMCavitySubsystem> pm2 = kernel->getKernelModule<FortrendPMCavitySubsystem>("PM2");
		while (running)
		{
			pm_step_once_finished = false;

			switch (pm_auto_step)
			{
			case 10:
			{
				if (!pm_allow_get_put_wafer){
					pm_auto_step = 100;
				}
				else{
					Sleep(200);
				}
				
				if (pm_allow_goto_craft){
					pm_auto_step = 2000;
				}
				else{
					Sleep(200);
				}
			}
			break;
			case 100:
			{
				if (pm2->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					auto cmd = pm2->createToGetStationCommand();
					pm2->startCommand(cmd);
					cmd->wait();
					if (cmd->hasError())
					{
						logFailedExcuteCommandHasError(pm2->getName(), "去上下料位命令执行失败", pm_process_name, pm_auto_step);
					}
					else
					{
						pm_allow_get_put_wafer = true;
						pm_auto_step = 200;
					}

				}
				else
				{
					logFailedNotNormal(pm2->getName(), pm_process_name, pm_auto_step);
				}
			}
			case 200:
			{
				if (!pm_allow_get_put_wafer){
					pm_auto_step = 10;
				}
				else{
					Sleep(200);
				}
			}

			break;
			case 2000:
			{
				if (pm2->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					auto cmd = pm2->createToPutStationCommand(2);
					pm2->startCommand(cmd);
					cmd->wait();
					if (cmd->hasError())
					{
						logFailedExcuteCommandHasError(pm2->getName(), "去工艺位命令执行失败", pm_process_name, pm_auto_step);
					}
					else
					{
						pm_auto_step = 2100;
					}

				}
				else
				{
					logFailedNotNormal(pm2->getName(), pm_process_name, pm_auto_step);
				}
			}
			break;
			case 2100:
			{
				auto cassManager = pm2->getKernel()->getKernelModule<FortrendCassetteManager>();
				auto station_cass = cassManager->getCassette(pm2.get());
				int count = station_cass->getPodSize();
				bool finish = true;
				//count = count == 0 ? 1 : count;
				logInform(pm2->getName().c_str(), "开始工艺次数 %d", count);
				for (int i = 0; i < count; i++)
				{
					if (pm2->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto cmd = pm2->createToPutStationCommand(2);//去工艺位2
						pm2->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							finish = false;
							logFailedExcuteCommandHasError(pm2->getName(), "去工艺位命令执行失败", pm_process_name, pm_auto_step);
							logError(pm2->getName().c_str(), "第%d次工艺失败", i);
							pm_auto_step = 2110;
							break;
						}
						else
						{
							auto cmd = pm2->createToPutStationCommand();//去工艺位1
							pm2->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError())
							{
								finish = false;
								logFailedExcuteCommandHasError(pm2->getName(), "去上下料位命令执行失败", pm_process_name, pm_auto_step);
								logError(pm2->getName().c_str(), "第%d次工艺失败", i);
								pm_auto_step = 2110;
								break;
							}
						}

					}
					else
					{
						finish = false;
						logFailedNotNormal(pm2->getName(), pm_process_name, pm_auto_step);
						pm_auto_step = 2110;
						break;
					}
				}
				if (finish){
					pm_auto_step = 2200;
				}
				else{
					pm_auto_step = 2110;
				}
			}
			break;

			case 2200:
			{
				if (pm2->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					auto cmd = pm2->createToGetStationCommand();
					pm2->startCommand(cmd);
					cmd->wait();
					if (cmd->hasError())
					{
						logFailedExcuteCommandHasError(pm2->getName(), "去上下料位命令执行失败", pm_process_name, pm_auto_step);
					}
					else
					{
						pm_allow_get_put_wafer = true;
						pm_allow_goto_craft = false;
						pm_auto_step = 10;
					}

				}
				else
				{
					logFailedNotNormal(pm2->getName(), pm_process_name, pm_auto_step);
				}
			}
			break;
			case 2110:
			{
				if (PMMessagecount > 0){
					//报错后弹窗人工确认
					if (QMessageBox::Yes == QMessageBox::question(q_ptr, "警告", "工艺位移动指令错误，请人工检查！点击确认将会尝试重新开始工艺", QMessageBox::Yes | QMessageBox::No)){
						pm_auto_step = 2100;
						PMMessagecount = 2;
					}
					else{
						PMMessagecount--;
					}
				}
				Sleep(5000);
			}
			break;
			
			default:
				break;
			}

			pm_step_once_finished = true;
			Sleep(10);
		}
	}

	void QSlotTransferCycleVTMWidgetPrivate::setLoadLock1PutCassetteFinished()
	{
		loadlock1_put_cassette_finished = true;
	}

	void QSlotTransferCycleVTMWidgetPrivate::setLoadLock2PutCassetteFinished(){
		loadlock2_put_cassette_finished = true;
	}

	bool QSlotTransferCycleVTMWidgetPrivate::setTransferSequence(){
		sequence_robot_transfer_wafer.clear();
		sequence_loadlock1_transfer_wafer.clear();
		sequence_loadlock2_transfer_wafer.clear();
		sequence_loadlock1_put_wafer.clear();
		sequence_loadlock2_put_wafer.clear();
		sequence_lp1_transfer_wafer.clear();
		sequence_lp2_transfer_wafer.clear();
		sequence_lp1_transfer_wafer_copy.clear();
		sequence_lp2_transfer_wafer_copy.clear();
		sequence_tolk1_wafer.clear();
		sequence_tolk2_wafer.clear();
		sequence_tolk1_wafer_copy.clear();
		sequence_tolk2_wafer_copy.clear();

		//计算LoadLock的理论槽位（未直接使用）
		int lk1slot = 1;
		int lk2slot = 1;

		//lpnum跟踪每个LoadPort（LP）的晶圆传输顺序编号,1-8循环
		//LL总共能放8片
		int lp1num = 1;
		int lp2num = 1;

		//记录上一次传输的序号，用于方向切换时的槽位计算
		int lp1num2 = 1;
		int lp2num2 = 1;

		std::string lastlp = "";
		std::string lasttransfer = "";
		bool lk1_goto_lk2 = false;
		bool lk2_goto_lk1 = false;
		//新增不规则晶圆数计算LL腔槽号
		int currentSlotNumber = 1; //每4片晶圆切换LoadLock
		bool isRobotLLA = true;//机器人当前服务的LoadLock（True=LLA, False=LLB）

		for (int i = 0; i < ui->sequence_edit_tbw->rowCount(); ++i) {
			QWidget *widget_direction = ui->sequence_edit_tbw->cellWidget(i, 1);
			QComboBox *direction = (QComboBox*)widget_direction;
			QWidget *widget_arm = ui->sequence_edit_tbw->cellWidget(i, 3);
			QComboBox *arm = (QComboBox*)widget_arm;

			QWidget *widget_loadlock1 = ui->sequence_edit_tbw->cellWidget(i, 0);
			QComboBox *loadlock1_slot = (QComboBox*)widget_loadlock1;

			QWidget *widget_loadlock2 = ui->sequence_edit_tbw->cellWidget(i, 2);
			QComboBox *loadlock2_slot = (QComboBox*)widget_loadlock2;

			QWidget *widget_pm1 = ui->sequence_edit_tbw->cellWidget(i, 4);
			QCheckBox *pm1 = (QCheckBox*)widget_pm1;

			QWidget *widget_pm2 = ui->sequence_edit_tbw->cellWidget(i, 5);
			QCheckBox *pm2 = (QCheckBox*)widget_pm2;

			QWidget *widget_pm3 = ui->sequence_edit_tbw->cellWidget(i, 6);
			QCheckBox *pm3 = (QCheckBox*)widget_pm3;

			QWidget* widget_pm4 = ui->sequence_edit_tbw->cellWidget(i, 7);
			QCheckBox* pm4 = (QCheckBox*)widget_pm4;

			int lp1slot = loadlock1_slot->currentText().toInt();//1
			int lp2slot = loadlock2_slot->currentText().toInt();//1

			if (ui->disabledefem->checkState() == Qt::CheckState::Checked)
			{//禁用efem
				lk1slot = lp1slot % 4 == 0 ? 4 : lp1slot % 4;// 1，2，3，4层 ，loadlock共4层的晶圆盒，所以取4的余数
				lk2slot = lp2slot % 4 == 0 ? 4 : lp2slot % 4;//
			}
			else{
				lk1slot = lk1slot > 4 ? 1 : lk1slot;
				lk2slot = lk2slot > 4 ? 1 : lk2slot;
			}
			
			//1-8循环
			lp1num = lp1num > 8 ? 1 : lp1num;
			lp2num = lp2num > 8 ? 1 : lp2num;

			//logInform("Cycle", "i=%d lk1slot=%d lk2slot=%d lp1num1=%d lp1num2=%d", i, lk1slot, lk2slot, lp1num, lp2num);

			LPTransferWafer lp1;
			lp1.slot_lp = lp1slot;
			lp1.is_finish_putlk = false;
			//lp1.selected_arm = robot.selected_arm;

			LPTransferWafer lp2;
			lp2.slot_lp = lp2slot;
			lp2.is_finish_putlk = false;
			//lp2.selected_arm = robot.selected_arm;

			RobotTransferWafer robot;
			robot.is_finish = false;
           if (direction->currentText() == "LP1<----->LP1")
		   {
			   if (lastlp != "ELP1" && i != 0)
			   {
				   if (lp2num2 >= 1 && lp2num2 < 4){//lk2_to_lk2
					   lp1num = lp2num2 + 5;
					   lk2slot = lk1slot + 1;
					   lk1_goto_lk2 = true;
					   logInform("Cycle", "i=%d lp1num=%d lk2slot=%d lp2num=%d", i, lp1num, lk2slot, lp2num2);
				   }
				   else if (lp2num2 >= 5 && lp2num2 < 8)
				   {
					   lp1num = lp2num2 - 3;
					   lk2slot = lk1slot + 1;
					   lk1_goto_lk2 = true;
					   logInform("Cycle", "i=%d lp1num=%d lk2slot=%d lp2num2=%d ", i, lp1num, lk2slot, lp2num2);
				   }
				   else if (lp2num2 == 8)
				   {
					   lp1num = 5;
				   }
			   }
			   lastlp = "ELP1";
			   lp1.source_lp = "ELP1";
			  
				if (isRobotLLA)
				{//lp1num <= 4
					robot.source_loadlock = "LLA";
					robot.target_loadlock = "LLA";
					robot.transfer = "lk1_to_lk1";
					//robot.slot_lk = lk1slot;
					//lp1.slot_lk = lk1slot;

					robot.slot_lk = currentSlotNumber;

					lp1.slot_lk = currentSlotNumber;
					lp1.transfer = "lk1_to_lk1";

					sequence_lp1_transfer_wafer.push_back(lp1);
					sequence_lp1_transfer_wafer_copy.push_back(lp1);

					sequence_tolk1_wafer.push_back(lp1);
					sequence_tolk1_wafer_copy.push_back(lp1);

					lasttransfer = "lk1_to_lk1";
					lk1slot++;
					if (lk1_goto_lk2 && lp1num == 4 )
					{
						lk1_goto_lk2 = false;
						lk2slot = 1;
						logInform("Cycle", "i=%d updatelk2slot=%d", i, lk2slot);
					}
				}
				else
				{
					robot.source_loadlock = "LLB";
					robot.target_loadlock = "LLB";
					robot.transfer = "lk2_to_lk2";
					//robot.slot_lk = lk2slot;
					//lp1.slot_lk = lk2slot;
					robot.slot_lk = currentSlotNumber;
					lp1.slot_lk = currentSlotNumber;

					lp1.transfer = "lk2_to_lk2";
					sequence_lp1_transfer_wafer.push_back(lp1);
					sequence_lp1_transfer_wafer_copy.push_back(lp1);
					sequence_tolk2_wafer.push_back(lp1);
					sequence_tolk2_wafer_copy.push_back(lp1);
					lasttransfer = "lk2_to_lk2";
					lk2slot++;
				}
				robot.source_lp = "ELP1";
				robot.slot_lp = lp1slot;
				logInform("Cycle", "i=%d lp2=%s lk=%d robot.slot_lp=%d lastlp=%s lp1num=%d robotsource_loadlock=%s",
					i, lp1.transfer, lp1.slot_lk, robot.slot_lp, lastlp, lp1num, robot.source_loadlock);
				lp1num2 = lp1num;
				lp1num++;
			}
			else if (direction->currentText() == "LP2<----->LP2")
		    {
				if (lastlp != "ELP2"&&i!=0)
				{
					if (lp1num2 >= 1 && lp1num2 < 4){
						lp2num = lp1num2 + 5;
						lk1slot = lk2slot + 1;
						lk2_goto_lk1 = true;
						logInform("Cycle", "i=%d lp2num=%d lk1slot=%d lp1num2=%d", i, lp2num, lk1slot, lp1num2);
					}
					else if (lp1num2 >= 5 && lp1num2 < 8){
						lp2num = lp1num2 - 3;
						lk1slot = lk2slot + 1;
						lk2_goto_lk1 = true;
						logInform("Cycle", "i=%d lp2num=%d lk1slot=%d lp1num2=%d", i, lp2num, lk1slot, lp1num2);
					}
					else if (lp1num2==8){
						lp2num = 5;
					}
					
				}
				lastlp = "ELP2";
				lp2.source_lp = "ELP2";
				if (!isRobotLLA)
				{//lp2num <= 4
					robot.source_loadlock = "LLB";
					robot.target_loadlock = "LLB";
					robot.transfer = "lk2_to_lk2";
					//robot.slot_lk = lk2slot;
					//lp2.slot_lk = lk2slot;
					robot.slot_lk = currentSlotNumber;
					lp2.slot_lk = currentSlotNumber;

					lp2.transfer = "lk2_to_lk2";
					sequence_lp2_transfer_wafer.push_back(lp2);
					sequence_lp2_transfer_wafer_copy.push_back(lp2);
					sequence_tolk2_wafer.push_back(lp2);
					sequence_tolk2_wafer_copy.push_back(lp2);
					lasttransfer = "lk2_to_lk2";
					lk2slot++;
					if (lk2_goto_lk1&&lp2num == 4){
						lk2_goto_lk1 = false;
						lk1slot=1;
						logInform("Cycle", "i=%d updatelk1slot=%d", i, lk1slot);
					}
				}
				else{
					robot.source_loadlock = "LLA";
					robot.target_loadlock = "LLA";
					robot.transfer = "lk1_to_lk1";
					/*robot.slot_lk = lk1slot;
					lp2.slot_lk = lk1slot;*/
					robot.slot_lk = currentSlotNumber;
					lp2.slot_lk = currentSlotNumber;

					lp2.transfer = "lk1_to_lk1";
					sequence_lp2_transfer_wafer.push_back(lp2);
					sequence_lp2_transfer_wafer_copy.push_back(lp2);
					sequence_tolk1_wafer.push_back(lp2);
					sequence_tolk1_wafer_copy.push_back(lp2);
					lasttransfer = "lk1_to_lk1";
					lk1slot++;
				}
			
				robot.source_lp = "ELP2";
				robot.slot_lp = lp2slot;
				logInform("Cycle", "i=%d lp2=%s lk=%d robot.slot_lp=%d lastlp=%s lp2num=%d robotsource_loadlock=%s", i, lp2.transfer, lp2.slot_lk, robot.slot_lp, lastlp, lp2num, robot.source_loadlock);
				lp2num2 = lp2num;
				lp2num++;
			}
			else
			{
				return false;
			}

			++currentSlotNumber;
			if (currentSlotNumber > 4)
			{
				currentSlotNumber = 1;
				isRobotLLA = !isRobotLLA;
			}

			if (arm->currentText() == "A")
			{
				robot.selected_arm = 0;
			}
			else if (arm->currentText() == "B")
			{
				robot.selected_arm = 1;
			}
			else
			{
				return false;
			}
			//robot.pm1_is_enable = pm1->isChecked();
			robot.pm2_is_enable = pm2->isChecked();
			//robot.pm3_is_enable = pm3->isChecked();
				/*if (robot.target_loadlock == "LLB"){
					sequence_robot_transfer_wafer_lp2.push_back(robot);
				}
				else if (robot.target_loadlock == "LLA"){
					sequence_robot_transfer_wafer_lp1.push_back(robot);
				}*/
			if (robot.source_lp == "ELP2"){
				sequence_robot_transfer_wafer_lp2.push_back(robot);
			}
			else if (robot.source_lp == "ELP1"){
				sequence_robot_transfer_wafer_lp1.push_back(robot);
			}
			sequence_robot_transfer_wafer.push_back(robot);
		}

		if (ui->sequence_edit_tbw->rowCount() > 0)
		{
			onUpdateProcessControlEnabled(false);
			logInform("Cycle", "Cycle开始");
			return true;
		}
		return false;
	}


	bool QSlotTransferCycleVTMWidgetPrivate::setHLTransferSequence()
	{
		taskManager.clearTasks();

		int llaSlot = 1;  // LLA当前槽位
		int llbSlot = 1;  // LLB当前槽位

		elp1 = kernel->getKernelModule<EFEMLPSubsystem>("ELP1");
		elp2 = kernel->getKernelModule<EFEMLPSubsystem>("ELP2");

		lk1 = kernel->getKernelModule<FortrendLoadLockSubsystem>("LLA");
		lk2 = kernel->getKernelModule<FortrendLoadLockSubsystem>("LLB");

		FortrendCassetteManager::Ptr cass1Manager = elp1->getKernel()->getKernelModule<FortrendCassetteManager>();
		cass1 = cass1Manager->getCassette(elp1.get());

		FortrendCassetteManager::Ptr cass2Manager = elp2->getKernel()->getKernelModule<FortrendCassetteManager>();
		cass2 = cass1Manager->getCassette(elp2.get());

		FortrendCassetteManager::Ptr llaManager = lk1->getKernel()->getKernelModule<FortrendCassetteManager>();
		lk1_cass = llaManager->getCassette(lk1.get());

		FortrendCassetteManager::Ptr llbManager = lk2->getKernel()->getKernelModule<FortrendCassetteManager>();
		lk2_cass = llbManager->getCassette(lk2.get());

		// 处理UI中的每一行
		for (int i = 0; i < ui->sequence_edit_tbw->rowCount(); ++i)
		{
			// 解析UI数据
			QComboBox* direction = static_cast<QComboBox*>(ui->sequence_edit_tbw->cellWidget(i, 1));
			QComboBox* arm = static_cast<QComboBox*>(ui->sequence_edit_tbw->cellWidget(i, 3));
			QComboBox* loadlock1_slot = static_cast<QComboBox*>(ui->sequence_edit_tbw->cellWidget(i, 0));
			QComboBox* loadlock2_slot = static_cast<QComboBox*>(ui->sequence_edit_tbw->cellWidget(i, 2));

			QCheckBox* pm1 = static_cast<QCheckBox*>(ui->sequence_edit_tbw->cellWidget(i, 4));
			QCheckBox* pm2 = static_cast<QCheckBox*>(ui->sequence_edit_tbw->cellWidget(i, 5));
			QCheckBox* pm3 = static_cast<QCheckBox*>(ui->sequence_edit_tbw->cellWidget(i, 6));
			QCheckBox* pm4 = static_cast<QCheckBox*>(ui->sequence_edit_tbw->cellWidget(i, 7));

			
			UnifiedWaferTask task;
			task.taskId = i; //ID
			task.status = UnifiedWaferTask::Status::UNKNOWN_PROGRESS;
			task.taskType = UnifiedWaferTask::TaskType::UNKNOWN;
			task.Aligner_status = UnifiedWaferTask::AlignerStatus::ALIGNER_READY;


			if (direction->currentText() == "LP1<----->LP1") {
				const std::string& id = std::to_string(task.taskId);
				cass1->setWaferId(i, id);

				task.source = UnifiedWaferTask::Location::LP1;
				task.sourceSlot = loadlock1_slot->currentText().toInt();
			}
			else if (direction->currentText() == "LP2<----->LP2") {

				const std::string& id = std::to_string(task.taskId);
				cass1->setWaferId(i, id);

				task.source = UnifiedWaferTask::Location::LP2;
				task.sourceSlot = loadlock2_slot->currentText().toInt();
			}
			else {
				// 无效方向
				return false;
			}

			// 确定机械臂
			if (arm->currentText() == "A") {
				task.arm = 0;
			}
			else if (arm->currentText() == "B") {
				task.arm = 1;
			}
			else {
				// 无效手臂选择
				return false;
			}

			// 处理模块状态
			task.pm1Enabled = pm1->isChecked();
			task.pm2Enabled = pm2->isChecked();
			task.pm3Enabled = pm3->isChecked();
			task.pm4Enabled = pm4->isChecked();

			selectPmEnableList[0] = task.pm1Enabled; // 隐式转换：true->1, false->0
			selectPmEnableList[1] = task.pm2Enabled;
			selectPmEnableList[2] = task.pm3Enabled;
			selectPmEnableList[3] = task.pm4Enabled;

			// 轮换式LoadLock分配
			const int GROUP_SIZE = 2;
			int groupIndex = i / GROUP_SIZE;

			//2. 确定目标LoadLock：偶数组->LLA，奇数组->LLB :-->2025/8/2 硬件原因调换,优先分配给LLB
			if (groupIndex % 2 == 0)
			{
				task.target = UnifiedWaferTask::Location::LLB;
				task.target_pm = getSelectPmLocation(task);
				task.targetSlot = llbSlot++;
				
				//setWaferId
				const std::string& id = std::to_string(task.taskId);
				lk2_cass->setWaferId(task.targetSlot, id);
				if (llbSlot > GROUP_SIZE) llbSlot = 1;// 组内循环
			}
			else
			{
				task.target = UnifiedWaferTask::Location::LLA;
				task.target_pm = getSelectPmLocation(task);
				task.targetSlot = llaSlot++;
				//setWaferId
				const std::string& id = std::to_string(task.taskId);
				lk1_cass->setWaferId(task.targetSlot, id);

				if (llaSlot > GROUP_SIZE) llaSlot = 1;// 组内循环
			}
			taskManager.addTask(task);//加入到管理者

			// 调试日志
			logInform("TransferSetup", "Row %d: Source %s Group %d -> %s Slot %d  taskID %d  Status %s  %s ",
				i, task.locationToString(task.source), 
				groupIndex,
				(task.target == UnifiedWaferTask::Location::LLA) ? "LLA" : "LLB",
				task.targetSlot,
				task.taskId,
				task.statusToString(task.status),
				getSelectPmProcessName(task));
		}

		if (ui->sequence_edit_tbw->rowCount() > 0)
		{
			onUpdateProcessControlEnabled(false);
			logInform("Cycle", "传输序列生成完成");
			return true;
		}
		return false;
	}

	
	bool QSlotTransferCycleVTMWidgetPrivate::setPMCavityParameter(){
		for (int i = 0; i < ui->pm_cavity_param_edit_tbw->rowCount(); i++)
		{
			QWidget *widget_temp = ui->pm_cavity_param_edit_tbw->cellWidget(i, 1);
			QDoubleSpinBox *temperature_dsb = (QDoubleSpinBox*)widget_temp;
			pm_process_param[i].heating_temperature = temperature_dsb->value();
		}

		for (int i = 0; i < ui->pm_cavity_param_edit_tbw->rowCount(); i++)
		{
			QWidget *widget_extraction_pressure = ui->pm_cavity_param_edit_tbw->cellWidget(i, 2);
			QDoubleSpinBox *extraction_pressure_dsb = (QDoubleSpinBox*)widget_extraction_pressure;
			pm_process_param[i].initial_extraction_pressure = extraction_pressure_dsb->value();

		}



		for (int i = 0; i < ui->pm_cavity_param_edit_tbw->rowCount(); i++)
		{
			QWidget *widget_purified_pressure = ui->pm_cavity_param_edit_tbw->cellWidget(i, 3);
			QDoubleSpinBox *purified_pressure_dsb = (QDoubleSpinBox*)widget_purified_pressure;
			pm_process_param[i].purified_extraction_pressure = purified_pressure_dsb->value();
		}



		for (int i = 0; i < ui->pm_cavity_param_edit_tbw->rowCount(); i++)
		{
			QWidget *widget_sputtering_pressure = ui->pm_cavity_param_edit_tbw->cellWidget(i, 4);
			QDoubleSpinBox *sputtering_pressure_dsb = (QDoubleSpinBox*)widget_sputtering_pressure;
			pm_process_param[i].sputtering_pressure = sputtering_pressure_dsb->value();
		}




		for (int i = 0; i < ui->pm_cavity_param_edit_tbw->rowCount(); i++)
		{
			QWidget *widget_flow_rate1 = ui->pm_cavity_param_edit_tbw->cellWidget(i, 5);
			QDoubleSpinBox *flow_rate1_dsb = (QDoubleSpinBox*)widget_flow_rate1;
			pm_process_param[i].sputtering_flow_rate1 = flow_rate1_dsb->value();
		}


		for (int i = 0; i < ui->pm_cavity_param_edit_tbw->rowCount(); i++)
		{
			QWidget *widget_flow_rate2 = ui->pm_cavity_param_edit_tbw->cellWidget(i, 6);
			QDoubleSpinBox *flow_rate2_dsb = (QDoubleSpinBox*)widget_flow_rate2;
			pm_process_param[i].sputtering_flow_rate2 = flow_rate2_dsb->value();
		}

		for (int i = 0; i < ui->pm_cavity_param_edit_tbw->rowCount(); i++)
		{
			QWidget *widget_flow_rate3 = ui->pm_cavity_param_edit_tbw->cellWidget(i, 7);
			QDoubleSpinBox *flow_rate3_dsb = (QDoubleSpinBox*)widget_flow_rate3;
			pm_process_param[i].sputtering_flow_rate3 = flow_rate3_dsb->value();
		}

		for (int i = 0; i < ui->pm_cavity_param_edit_tbw->rowCount(); i++)
		{
			QWidget *widget_power1 = ui->pm_cavity_param_edit_tbw->cellWidget(i, 8);
			QDoubleSpinBox *flow_power1_dsb = (QDoubleSpinBox*)widget_power1;
			pm_process_param[i].sputtering_power1 = flow_power1_dsb->value();
		}

		for (int i = 0; i < ui->pm_cavity_param_edit_tbw->rowCount(); i++)
		{
			QWidget *widget_power_gear_up1 = ui->pm_cavity_param_edit_tbw->cellWidget(i, 9);
			QDoubleSpinBox *flow_power_gear_up1_dsb = (QDoubleSpinBox*)widget_power_gear_up1;
			pm_process_param[i].sputtering_power_gear_up1 = flow_power_gear_up1_dsb->value();
		}

		for (int i = 0; i <ui->pm_cavity_param_edit_tbw->rowCount(); i++)
		{
			QWidget *widget_power2 = ui->pm_cavity_param_edit_tbw->cellWidget(i, 10);
			QDoubleSpinBox *flow_power2_dsb = (QDoubleSpinBox*)widget_power2;
			pm_process_param[i].sputtering_power2 = flow_power2_dsb->value();
		}

		for (int i = 0; i < ui->pm_cavity_param_edit_tbw->rowCount(); i++)
		{
			QWidget *widget_power_gear_up2 = ui->pm_cavity_param_edit_tbw->cellWidget(i, 11);
			QDoubleSpinBox *flow_power_gear_up2_dsb = (QDoubleSpinBox*)widget_power_gear_up2;
			pm_process_param[i].sputtering_power_gear_up2 = flow_power_gear_up2_dsb->value();
		}

		for (int i = 0; i < ui->pm_cavity_param_edit_tbw->rowCount(); i++)
		{
			QWidget *widget_power3 = ui->pm_cavity_param_edit_tbw->cellWidget(i, 12);
			QDoubleSpinBox *flow_power3_dsb = (QDoubleSpinBox*)widget_power3;
			pm_process_param[i].sputtering_power3 = flow_power3_dsb->value();
		}

		for (int i = 0; i < ui->pm_cavity_param_edit_tbw->rowCount(); i++)
		{
			QWidget *widget_power_gear_up3 = ui->pm_cavity_param_edit_tbw->cellWidget(i, 13);
			QDoubleSpinBox *flow_power_gear_up3_dsb = (QDoubleSpinBox*)widget_power_gear_up3;
			pm_process_param[i].sputtering_power_gear_up3 = flow_power_gear_up3_dsb->value();
		}

		for (int i = 0; i < ui->pm_cavity_param_edit_tbw->rowCount(); i++)
		{
			QWidget *widget_sputtering_time = ui->pm_cavity_param_edit_tbw->cellWidget(i, 14);
			QDoubleSpinBox *sputtering_time_dsb = (QDoubleSpinBox*)widget_sputtering_time;
			pm_process_param[i].pre_sputtering_time = sputtering_time_dsb->value();
		}

		for (int i = 0; i < ui->pm_cavity_param_edit_tbw->rowCount(); i++)
		{
			QWidget *widget_substrate_speed = ui->pm_cavity_param_edit_tbw->cellWidget(i, 15);
			QDoubleSpinBox *substrate_speed_dsb = (QDoubleSpinBox*)widget_substrate_speed;
			pm_process_param[i].substrate_speed = substrate_speed_dsb->value();
		}

		for (int i = 0; i < ui->pm_cavity_param_edit_tbw->rowCount(); i++)
		{
			QWidget *widget_process_sputtering_time = ui->pm_cavity_param_edit_tbw->cellWidget(i, 16);
			QDoubleSpinBox *process_sputtering_time_dsb = (QDoubleSpinBox*)widget_process_sputtering_time;
			pm_process_param[i].process_sputtering_time = process_sputtering_time_dsb->value();
		}

		for (int i = 0; i < ui->pm_cavity_param_edit_tbw->rowCount(); i++)
		{
			QWidget *widget_cathode_power_selection_1 = ui->pm_cavity_param_edit_tbw->cellWidget(i, 17);
			QComboBox *cathode_power_selection_1_cbx = (QComboBox*)widget_cathode_power_selection_1;
			pm_process_param[i].cathode_power_selection_1 = cathode_power_selection_1_cbx->currentIndex();
		}

		for (int i = 0; i < ui->pm_cavity_param_edit_tbw->rowCount(); i++)
		{
			QWidget *widget_cathode_power_selection_2 = ui->pm_cavity_param_edit_tbw->cellWidget(i, 18);
			QComboBox *cathode_power_selection_2_cbx = (QComboBox*)widget_cathode_power_selection_2;
			pm_process_param[i].cathode_power_selection_2 = cathode_power_selection_2_cbx->currentIndex();
		}

		for (int i = 0; i < ui->pm_cavity_param_edit_tbw->rowCount(); i++)
		{
			QWidget *widget_cathode_power_selection_3 = ui->pm_cavity_param_edit_tbw->cellWidget(i, 19);
			QComboBox *cathode_power_selection_3_cbx = (QComboBox*)widget_cathode_power_selection_3;
			pm_process_param[i].cathode_power_selection_3 = cathode_power_selection_3_cbx->currentIndex();
		}

		return true;

	}

	void QSlotTransferCycleVTMWidgetPrivate::logFailed(const std::string station_name, const std::string log){
		Q_Q(QSlotTransferCycleVTMWidget);
		running = false;
		ispause = true;
		logError(station_name.c_str(), log.c_str());
		onUpdateControlEnabled("execute_pbt", true);
	}

	void QSlotTransferCycleVTMWidgetPrivate::logFailedNotNormal(const std::string station_name, const std::string process_name, const int step){
		Q_Q(QSlotTransferCycleVTMWidget);
		logFailed(station_name, Poco::format("%s 未处于正常状态， %s：%d", station_name, process_name, step));
	}

	void QSlotTransferCycleVTMWidgetPrivate::logFailedExcuteCommandHasError(const std::string station_name, const std::string command_name, const std::string process_name, const int step){
		Q_Q(QSlotTransferCycleVTMWidget);
		logFailed(station_name, Poco::format("%s %s命令执行失败， %s：%d", station_name, command_name, process_name, step));
	}

	/**
	 * QSlotTransferCycleVTMWidget
	 */
	QSlotTransferCycleVTMWidget::QSlotTransferCycleVTMWidget(const std::shared_ptr<IKernel>& kernel, QFortrendStationStatusVTMWidget* ptr, QWidget* parent)
		:QKernelModuleWidget(parent)
		, d_ptr(new QSlotTransferCycleVTMWidgetPrivate(this)){
		Q_D(QSlotTransferCycleVTMWidget);
		s_ptr = ptr;
		d->kernel = kernel;
		d->tower = kernel->getKernelModule<FortrendVTMSignalTower>();
		d->tmplc = kernel->getKernelModule<FortrendTMCavitySubsystem>("TM");
		QHBoxLayout* main_layout = new QHBoxLayout(this);
		d->ui = new Ui::SlotTransferCycleVTMWidget;

		QWidget* opt_panel = new QWidget;
		d->ui->setupUi(opt_panel);
		main_layout->addWidget(opt_panel);
		for (size_t i = 0; i < d->ui->sequence_edit_tbw->columnCount(); i++)
		{
			d->ui->sequence_edit_tbw->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Stretch);
		}
		for (size_t i = 0; i < d->ui->pm_cavity_param_edit_tbw->columnCount(); i++)
		{
			d->ui->pm_cavity_param_edit_tbw->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Stretch);
		}
		//d->ui->sequence_edit_tbw->horizontalHeader()->sectionResizeMode(QHeaderView::Stretch);
		d->ui->sequence_edit_tbw->setSelectionBehavior(QAbstractItemView::SelectRows);
		d->ui->pm_cavity_param_edit_tbw->setSelectionBehavior(QAbstractItemView::SelectRows);
		//d->ui->sequence_edit_tbw->horizontalHeader()->setStyleSheet("QHeaderView::section{background:#0000FF;}");
		//d->ui->sequence_edit_tbw->verticalHeader()->setStyleSheet("QHeaderView::section{background:#a7fffa;}");
		//d->ui->sequence_edit_tbw->setStyleSheet("QTableCornerButton::section{background:#0000FF;}");
		//d->ui->execute_btn->setEnabled(false);

		//connect
		connect(d->ui->add_an_item_pbt, &QPushButton::clicked, this, &QSlotTransferCycleVTMWidget::onAddAnItem);
		connect(d->ui->delete_the_selected_item_pbt, &QPushButton::clicked, this, &QSlotTransferCycleVTMWidget::onDeleteTheSelectedItem);
		connect(d->ui->clear_sequence_pbt, &QPushButton::clicked, this, &QSlotTransferCycleVTMWidget::onClearSequence);
		connect(d->ui->save_sequence_pbt, &QPushButton::clicked, this, &QSlotTransferCycleVTMWidget::onSaveSequence);
		connect(d->ui->load_sequence_pbt, &QPushButton::clicked, this, &QSlotTransferCycleVTMWidget::onLoadSequence);


		connect(d->ui->execute_pbt, &QPushButton::clicked, this, &QSlotTransferCycleVTMWidget::onStart);
		connect(d->ui->pause_pbt, &QPushButton::clicked, this, &QSlotTransferCycleVTMWidget::onPause);
		connect(d->ui->reset_pbt, &QPushButton::clicked, this, &QSlotTransferCycleVTMWidget::onReset);

		connect(d->ui->abort_pbt, &QPushButton::clicked, this, &QSlotTransferCycleVTMWidget::onAbort);
		//connect(d->ui->loadlock1_put_cassette_finished_pbt, &QPushButton::clicked, this, &QSlotTransferCycleVTMWidget::onLoadLock1PutCassetteFinished);
		//connect(d->ui->loadlock2_put_cassette_finished_pbt, &QPushButton::clicked, this, &QSlotTransferCycleVTMWidget::onLoadLock2PutCassetteFinished);

		connect(d->ui->get_step_pbt, &QPushButton::clicked, this, &QSlotTransferCycleVTMWidget::onGetStep);

		connect(s_ptr, &QFortrendStationStatusVTMWidget::signalUpdateRecipe, this, &QSlotTransferCycleVTMWidget::onUpdateRecipe);


		d->ui->enableAtmosphere->setEnabled(true);
		d->ui->execute_pbt->setEnabled(true);
		initPMCavityParamEdieTableWidget();

		//d->ui->gbx_pm_parameter->hide();

		//d->ui->loadlock1_put_cassette_finished_pbt->hide();
		//d->ui->loadlock2_put_cassette_finished_pbt->hide();

		d->ui->spb_min->hide();
		d->ui->spb_max->hide();
		d->ui->label_3->hide();
		d->ui->label_4->hide();
		//d->ui->smif_feed_btn->hide();
		//d->ui->smif_blanking_btn->hide();
		d->ui->abort_pbt->hide();
		//d->ui->enablesmif1->hide();
		//d->ui->enablesmif2->hide();

		std::thread thd_vacuum(&QSlotTransferCycleVTMWidget::onProcess, this);
		thd_vacuum.detach();

	}

	QSlotTransferCycleVTMWidget::~QSlotTransferCycleVTMWidget(){
		Q_D(QSlotTransferCycleVTMWidget);
		delete d->ui;
		delete d;
	}

	void QSlotTransferCycleVTMWidget::onAddAnItem(){
		Q_D(QSlotTransferCycleVTMWidget);

		int row_count = d->ui->sequence_edit_tbw->rowCount();
		d->ui->sequence_edit_tbw->insertRow(row_count);

		/*std::shared_ptr<FortrendLoadLockSubsystem> lk1 = d->kernel->getKernelModule<FortrendLoadLockSubsystem>("LLA");
		std::shared_ptr<FortrendLoadLockSubsystem> lk2 = d->kernel->getKernelModule<FortrendLoadLockSubsystem>("LLB");*/


		QComboBox *loadlock1_slot_selected_cbx = new QComboBox();
		for (size_t i = 1; i <= 25; i++)
		{
			loadlock1_slot_selected_cbx->addItem(std::to_string(i).c_str());
		}
		d->ui->sequence_edit_tbw->setCellWidget(row_count, 0, loadlock1_slot_selected_cbx);

		QComboBox *loadlock2_slot_selected_cbx = new QComboBox();
		for (size_t i = 1; i <= 25; i++)
		{
			loadlock2_slot_selected_cbx->addItem(std::to_string(i).c_str());
		}
		d->ui->sequence_edit_tbw->setCellWidget(row_count, 2, loadlock2_slot_selected_cbx);

		QComboBox *transfer_direction_selected_cbx = new QComboBox();
		transfer_direction_selected_cbx->addItem("LP1<----->LP1");
		transfer_direction_selected_cbx->addItem("LP2<----->LP2");
		/*transfer_direction_selected_cbx->addItem("LoadLock1----->LoadLock2");
		transfer_direction_selected_cbx->addItem("LoadLock1<-----LoadLock2");*/
		d->ui->sequence_edit_tbw->setCellWidget(row_count, 1, transfer_direction_selected_cbx);

		QComboBox *transfer_robot_arm_selected_cbx = new QComboBox();
		transfer_robot_arm_selected_cbx->addItem("A");
		transfer_robot_arm_selected_cbx->addItem("B");
		d->ui->sequence_edit_tbw->setCellWidget(row_count, 3, transfer_robot_arm_selected_cbx);

		QCheckBox *pm1_ckb = new QCheckBox();
		pm1_ckb->setText("启用");
		d->ui->sequence_edit_tbw->setCellWidget(row_count, 4, pm1_ckb);

		QCheckBox *pm2_ckb = new QCheckBox();
		pm2_ckb->setText("启用");
		d->ui->sequence_edit_tbw->setCellWidget(row_count, 5, pm2_ckb);

		QCheckBox *pm3_ckb = new QCheckBox();
		pm3_ckb->setText("启用");
		d->ui->sequence_edit_tbw->setCellWidget(row_count, 6, pm3_ckb);

		QCheckBox* pm4_ckb = new QCheckBox();
		pm4_ckb->setText("启用");
		d->ui->sequence_edit_tbw->setCellWidget(row_count, 7, pm4_ckb);

	}

	void QSlotTransferCycleVTMWidget::onUpdateRecipe(int model){
		Q_D(QSlotTransferCycleVTMWidget);
		//0=A模式 1=B模式 2=先A后B模式 3=先B后A模式 
		if (d->running)return;
		try{
			QString fileName = "";
			switch (model)
			{
			case 0:
				fileName = QCoreApplication::applicationDirPath() + "/CycleIni/A.ini";
				break;
			case 1:
				fileName = QCoreApplication::applicationDirPath() + "/CycleIni/B.ini";
				break;
			case 2:
				fileName = QCoreApplication::applicationDirPath() + "/CycleIni/AB.ini";
				break;
			case 3:
				fileName = QCoreApplication::applicationDirPath() + "/CycleIni/BA.ini";
				break;
			default:
				break;
			}
			if (fileName == "")return;
			onClearSequence();
			QSettings settings(fileName, QSettings::IniFormat);
			int rowCount = settings.value("rowCount", 0).toInt();
			int columnCount = settings.value("columnCount", 0).toInt();
			for (int i = 0; i < rowCount; ++i) {
				onAddAnItem();
				for (int j = 0; j < 4; ++j) {
					QString key = QString("row%1col%2").arg(i).arg(j);
					QString value = settings.value(key, "").toString();
					QWidget *widget = d->ui->sequence_edit_tbw->cellWidget(i, j);
					QComboBox *combox = (QComboBox*)widget;
					combox->setCurrentText(value);
				}
				for (int j = 4; j < columnCount; j++)
				{
					QString key = QString("row%1col%2").arg(i).arg(j);
					QString value = settings.value(key, "").toString();
					QWidget *widget = d->ui->sequence_edit_tbw->cellWidget(i, j);
					QCheckBox *combox = (QCheckBox*)widget;
					combox->setChecked(value.toLower() == "true");
				}
			}

			int pm_rowCount = settings.value("pm_rowCount", 0).toInt();
			int pm_columnCount = settings.value("pm_columnCount", 0).toInt();
			for (int i = 0; i < pm_rowCount; ++i) {
				for (int j = 1; j < pm_columnCount; ++j) {
					QString key = QString("pm_row%1pm_col%2").arg(i).arg(j);
					QString value = settings.value(key, "").toString();
					QWidget *widget = d->ui->pm_cavity_param_edit_tbw->cellWidget(i, j);
					if (j > 16)
					{
						QComboBox *combox = (QComboBox*)widget;
						combox->setCurrentText(value);
					}
					else{
						QDoubleSpinBox *dsb = (QDoubleSpinBox*)widget;
						dsb->setValue(value.toDouble());
					}
				}
			}
		}
		catch (KernelException& e){
			logError(d->module_name.c_str(), e.what());
			//throw e;
		}
	}

	void QSlotTransferCycleVTMWidget::onDeleteTheSelectedItem(){
		Q_D(QSlotTransferCycleVTMWidget);
		int row_count = d->ui->sequence_edit_tbw->rowCount();
		if (row_count > 0)
		{
			int selected_row = d->ui->sequence_edit_tbw->currentRow();
			d->ui->sequence_edit_tbw->removeRow(selected_row);
		}
	}

	void QSlotTransferCycleVTMWidget::onClearSequence(){
		Q_D(QSlotTransferCycleVTMWidget);
		int row_count = d->ui->sequence_edit_tbw->rowCount();
		if (row_count > 0)
		{
			d->ui->sequence_edit_tbw->setRowCount(0);
		}

	}

	void QSlotTransferCycleVTMWidget::onLoadSequence()  throw (KernelException){
		Q_D(QSlotTransferCycleVTMWidget);
		try
		{
		QString cycle_dir = QCoreApplication::applicationDirPath() + "/CycleIni";
		QDir dir(cycle_dir);
		if (!dir.exists())
		{
			dir.mkpath(cycle_dir); // 使用mkpath创建文件夹，如果文件夹的父目录不存在也会一并创建
		}
		QString fileName = QFileDialog::getOpenFileName(this, "加载顺序", cycle_dir, "配置文件 (*.ini)");
		if (fileName.isEmpty())
		{
			QMessageBox::warning(this, tr("警告信息"), tr("加载路径不能为空！"));
			return;
		}
			

		QSettings settings(fileName, QSettings::IniFormat);
		onClearSequence();


		int rowCount = settings.value("rowCount", 0).toInt();
		int columnCount = settings.value("columnCount", 0).toInt();
		for (int i = 0; i < rowCount; ++i) {
			onAddAnItem();
			for (int j = 0; j < 4; ++j) {
				QString key = QString("row%1col%2").arg(i).arg(j);
				QString value = settings.value(key, "").toString();
				QWidget *widget = d->ui->sequence_edit_tbw->cellWidget(i, j);
				QComboBox *combox = (QComboBox*)widget;
				combox->setCurrentText(value);
			}
			for (int j = 4; j < columnCount; j++)
			{
				QString key = QString("row%1col%2").arg(i).arg(j);
				QString value = settings.value(key, "").toString();
				QWidget *widget = d->ui->sequence_edit_tbw->cellWidget(i, j);
				QCheckBox *combox = (QCheckBox*)widget;

				combox->setChecked(value.toLower() == "true");
			}
		}

		int pm_rowCount = settings.value("pm_rowCount", 0).toInt();
		int pm_columnCount = settings.value("pm_columnCount", 0).toInt();
		for (int i = 0; i < pm_rowCount; ++i) {
			for (int j = 1; j < pm_columnCount; ++j) {
				QString key = QString("pm_row%1pm_col%2").arg(i).arg(j);
				QString value = settings.value(key, "").toString();
				QWidget *widget = d->ui->pm_cavity_param_edit_tbw->cellWidget(i, j);

				//if (j > 16)
				//{
				//	QComboBox *combox = (QComboBox*)widget;
				//	combox->setCurrentText(value);

				//}
				//else{
				//	QDoubleSpinBox *dsb = (QDoubleSpinBox*)widget;
				//	dsb->setValue(value.toDouble());
				//}
				if (j == 2)
				{
					QComboBox* combox = (QComboBox*)widget;
					combox->setCurrentText(value);
				}
				else if (j == 3)
				{
					QSpinBox* spb = (QSpinBox*)widget;
					spb->setValue(value.toDouble());
				}
				else 
				{
					QDoubleSpinBox *dsb = (QDoubleSpinBox*)widget;
					dsb->setValue(value.toDouble());
				}
			}

		}
	}
		catch (KernelException& e){
			logError(d->module_name.c_str(), e.what());
			//throw e;
		}

	}
	void QSlotTransferCycleVTMWidget::onSaveSequence() throw (KernelException){
		Q_D(QSlotTransferCycleVTMWidget);
		try
		{
		if (d->ui->sequence_edit_tbw->rowCount()<1)
		{
			QMessageBox::warning(this, tr("警告信息"), tr("请先添加顺序！"));
			return;
		}
		QString cycle_dir = QCoreApplication::applicationDirPath() + "/CycleIni";
		QDir dir(cycle_dir);
		if (!dir.exists())
		{
			dir.mkpath(cycle_dir); // 使用mkpath创建文件夹，如果文件夹的父目录不存在也会一并创建
		}
		
		QString fileName = QFileDialog::getSaveFileName(this, "保存顺序", cycle_dir, "配置文件(*.ini)");
		if (fileName.isEmpty())
		{
			QMessageBox::warning(this, tr("警告信息"), tr("保存路径不能为空！"));
			return;
		}
		QSettings settings(fileName, QSettings::IniFormat);

		int rowCount = d->ui->sequence_edit_tbw->rowCount();
		int columnCount = d->ui->sequence_edit_tbw->columnCount();//8
		settings.setValue("rowCount", rowCount);
		settings.setValue("columnCount", columnCount);

		for (int i = 0; i < rowCount; ++i) {
			for (int j = 0; j < 4; ++j) {
				QString key = QString("row%1col%2").arg(i).arg(j);
				QWidget *widget = d->ui->sequence_edit_tbw->cellWidget(i, j);
				QComboBox *combox = (QComboBox*)widget;
				settings.setValue(key, combox->currentText());
			}
			for (int j = 4; j < columnCount; j++)
			{
				QString key = QString("row%1col%2").arg(i).arg(j);
				QWidget *widget = d->ui->sequence_edit_tbw->cellWidget(i, j);
				QCheckBox *combox = (QCheckBox*)widget;
				settings.setValue(key, combox->isChecked());
			}
		}

		int pm_rowCount = d->ui->pm_cavity_param_edit_tbw->rowCount();
		int pm_columnCount = d->ui->pm_cavity_param_edit_tbw->columnCount();
		settings.setValue("pm_rowCount", pm_rowCount);
		settings.setValue("pm_columnCount", pm_columnCount);

		for (int i = 0; i < pm_rowCount; ++i) {
			for (int j = 1; j < pm_columnCount; ++j) {
				QString key = QString("pm_row%1pm_col%2").arg(i).arg(j);
				QWidget *widget = d->ui->pm_cavity_param_edit_tbw->cellWidget(i, j);

				//auto className = widget->metaObject()->className();
				//qDebug() << "className:" << className << endl;

				if (j == 2)
				{
					QComboBox* combox = (QComboBox*)widget;
					settings.setValue(key, combox->currentText());
				}
				else if (j == 3)
				{
					QSpinBox* spb = (QSpinBox*)widget;
					settings.setValue(key, spb->value());
				}
				else
				{
					QDoubleSpinBox *dsb = (QDoubleSpinBox*)widget;
					settings.setValue(key, dsb->value());
				}

				//if (j > 16)
				//{
				//	QComboBox *combox = (QComboBox*)widget;
				//	settings.setValue(key, combox->currentText());
				//}
				//else{
				//	QDoubleSpinBox *dsb = (QDoubleSpinBox*)widget;
				//	settings.setValue(key, dsb->value());
				//}

			}

		}
	}
		catch (KernelException& e){
			logError(d->module_name.c_str(), e.what());
			//throw e;
		}

	}

	void QSlotTransferCycleVTMWidget::onLoadLock1PutCassetteFinished(){
		Q_D(QSlotTransferCycleVTMWidget);

		d->setLoadLock1PutCassetteFinished();
	}

	void QSlotTransferCycleVTMWidget::onLoadLock2PutCassetteFinished(){
		Q_D(QSlotTransferCycleVTMWidget);
		d->setLoadLock2PutCassetteFinished();

	}

	void QSlotTransferCycleVTMWidget::startVacuumAction(){
		Q_D(QSlotTransferCycleVTMWidget);
		d->startVacuumAction();
	}

	void QSlotTransferCycleVTMWidget::startVacuumMoniterAction(){
		Q_D(QSlotTransferCycleVTMWidget);

	}

	void QSlotTransferCycleVTMWidget::startRobotAction(){
		Q_D(QSlotTransferCycleVTMWidget);
		d->startRobotAction();
	}

	void QSlotTransferCycleVTMWidget::startLoadLock1Action(){
		Q_D(QSlotTransferCycleVTMWidget);
		d->startLoadLock1Action();
	}

	void QSlotTransferCycleVTMWidget::startLoadLock2Action(){
		Q_D(QSlotTransferCycleVTMWidget);
		d->startLoadLock2Action();
	}

	void QSlotTransferCycleVTMWidget::startPMAction(){
		Q_D(QSlotTransferCycleVTMWidget);
		d->startPMAction();
	}
	void QSlotTransferCycleVTMWidget::startEFEMAction(){
		Q_D(QSlotTransferCycleVTMWidget);
		d->startEFEMAction();
	}

	void QSlotTransferCycleVTMWidget::startUpdateStatusAction(){
		Q_D(QSlotTransferCycleVTMWidget);
		d->startUpdateStatusAction();
	}
	void QSlotTransferCycleVTMWidget::onPause(){
		Q_D(QSlotTransferCycleVTMWidget);
		std::shared_ptr<FortrendPMCavitySubsystem> pm2 = d->kernel->getKernelModule<FortrendPMCavitySubsystem>("PM2");
		pm2->setIsRunning(false);
		d->running = false;
		d->ispause = true;
		//ZZX
		taskManager.stop();

		d->ui->execute_pbt->setEnabled(true);
		d->ui->reset_pbt->setEnabled(true);
		d->ui->pause_pbt->setEnabled(false);
		
	}

	void QSlotTransferCycleVTMWidget::onAbort(){
		Q_D(QSlotTransferCycleVTMWidget);
		d->setTransferSequence();
		d->onUpdateProcessControlEnabled(true);
		/*if (!d->abortCycle){
			d->abortCycle = true;
			d->ui->reset_pbt->setEnabled(true);
			d->ui->pause_pbt->setEnabled(true);
			updateProcessControlEnabled(true);
		}
		else{
			d->abortCycle = false;
		}*/
		
	}

	void QSlotTransferCycleVTMWidget::onReset(){
		Q_D(QSlotTransferCycleVTMWidget);
		/*if (d->setTransferSequence()) 测试
		{
			d->ui->cycle_finished_times_spx->setValue(0);
			d->ui->cycle_finished_times_spx_2->setValue(0);
			logInform("Cycle", "传送流程配置成功。");
		}*/
		if (!isEnabledplcAuto()){
			QMessageBox::warning(this, "警告", "PLC不在自动模式.");
			return;
		}
		//ZZX
		if (!taskManager.isStopped())
		{
			taskManager.stop();
		}

		d->running = false;
		d->ispause = false;
		std::thread thread(&QSlotTransferCycleVTMWidget::resetAction, this);
		thread.detach();
	}

	void QSlotTransferCycleVTMWidget::onStart(){
		Q_D(QSlotTransferCycleVTMWidget);

		//start action & store param
		std::shared_ptr<FortrendPMCavitySubsystem> pm2 = d->kernel->getKernelModule<FortrendPMCavitySubsystem>("PM2");

		//调试注释
		if (SIM_CYCLE_MODE == 0)
		{
			if (!pm2->getPMCavityMotorHomeSignal()){
				QMessageBox::warning(this, "警告", "PM腔步进电机未后退到原位.");
				return;
			}
		}
		d->cycle_times_lla = d->ui->cycle_setting_times_sbx->value(); //LP1循环次数
		d->cycle_times_llb = d->ui->cycle_setting_times_sbx_2->value();//LP2循环次数
		if (d->sequence_lp1_transfer_wafer.size() == 0 && d->sequence_lp2_transfer_wafer.size() == 0 &&
			d->sequence_robot_transfer_wafer.size() == 0 )// && !d->ispause暂停重新启动的情况不需要重新配置
		{

			//测试
			if (d->setHLTransferSequence())
			{
				d->ui->cycle_finished_times_spx->setValue(0);
				d->ui->cycle_finished_times_spx_2->setValue(0);
				logInform("Cycle", "传送HL流程配置成功。");
			}
			else
			{
				QMessageBox::warning(this, "警告", "传送流程配置错误.");
				return;
			}

			//if (d->setTransferSequence())
			//{
			//	d->ui->cycle_finished_times_spx->setValue(0);
			//	d->ui->cycle_finished_times_spx_2->setValue(0);
			//	logInform("Cycle", "传送流程配置成功。");
			//}
			//else
			//{
			//	QMessageBox::warning(this, "警告", "传送流程配置错误.");
			//	return;
			//}
		}
		if (SIM_CYCLE_MODE == 0)
		{
			if (!isEnabledplcAuto()) {
				QMessageBox::warning(this, "警告", "PLC不在自动模式.");
				return;
			}
		}

		d->running = true;
		pm2->setIsRunning(d->running);
		d->ispause = false;

		d->efem_auto_step = 10;
		//新流程线程启动
		startProcessingThreads();
		taskManager.start();


#if 0
		std::thread thd_vacuum(&QSlotTransferCycleVTMWidget::startVacuumAction, this);
		thd_vacuum.detach();

		std::thread thd_robot(&QSlotTransferCycleVTMWidget::startRobotAction, this);
		thd_robot.detach();

		std::thread thd_loadlock1(&QSlotTransferCycleVTMWidget::startLoadLock1Action, this);
		thd_loadlock1.detach();

		std::thread thd_loadlock2(&QSlotTransferCycleVTMWidget::startLoadLock2Action, this);
		thd_loadlock2.detach();

		std::thread thd_pm(&QSlotTransferCycleVTMWidget::startPMAction, this);
		thd_pm.detach();

		std::thread thd_update(&QSlotTransferCycleVTMWidget::startUpdateStatusAction, this);
		thd_update.detach();

		std::thread thd_efem(&QSlotTransferCycleVTMWidget::startEFEMAction, this);
		thd_efem.detach();
#endif

		d->tower->setOutput(FortrendVTMSignalTower::Output::YELLOW_LIGHT, false);
		d->tower->setOutput(FortrendVTMSignalTower::Output::GREEN_LIGHT, true);
		d->ui->execute_pbt->setEnabled(false);
		d->ui->reset_pbt->setEnabled(false);
		d->ui->pause_pbt->setEnabled(true);
	}

	void QSlotTransferCycleVTMWidget::clickStart(){
		onStart();
	}
	void QSlotTransferCycleVTMWidget::clickPause(){
		onPause();
	}
	void QSlotTransferCycleVTMWidget::clickReset(){
		onReset();
	}
	void QSlotTransferCycleVTMWidget::clickAbort(){
		onAbort();
	}
	void QSlotTransferCycleVTMWidget::clickplcAuto(bool isauto){
		Q_D(QSlotTransferCycleVTMWidget);
		if (!d->running){
			d->tmplc->writeBit("MR50001", isauto);
		}
		else{
			logError("Cycle", "执行cycle期间不能修改PLC模式%d", isauto);
		}
		
	}
	
	bool QSlotTransferCycleVTMWidget::isAuto(){
		Q_D(QSlotTransferCycleVTMWidget);
		bool isauto = false;
		d->tmplc->readBit("MR50001", isauto);
		return isauto;

	}
	bool QSlotTransferCycleVTMWidget::isEnabledplcAuto(){
		Q_D(QSlotTransferCycleVTMWidget);
		return d->plcauto;
	}

	bool QSlotTransferCycleVTMWidget::isEnabledStart(){
		Q_D(QSlotTransferCycleVTMWidget);
		return d->ui->execute_pbt->isEnabled();
	}
	bool QSlotTransferCycleVTMWidget::isEnabledPause(){
		Q_D(QSlotTransferCycleVTMWidget);
		return d->ui->pause_pbt->isEnabled();
	}
	bool QSlotTransferCycleVTMWidget::isEnabledReset(){
		Q_D(QSlotTransferCycleVTMWidget);
		return d->ui->reset_pbt->isEnabled();
	}
	bool QSlotTransferCycleVTMWidget::isEnabledAbort(){
		Q_D(QSlotTransferCycleVTMWidget);
		return d->ui->abort_pbt->isEnabled();
	}

	void QSlotTransferCycleVTMWidget::onProcess(){
		Q_D(QSlotTransferCycleVTMWidget);
		std::shared_ptr<FortrendTMCavitySubsystem> tm = d->kernel->getKernelModule<FortrendTMCavitySubsystem>("TM");
		std::shared_ptr<FortrendLoadLockSubsystem> lk1 = d->kernel->getKernelModule<FortrendLoadLockSubsystem>("LLA");
		std::shared_ptr<FortrendLoadLockSubsystem> lk2 = d->kernel->getKernelModule<FortrendLoadLockSubsystem>("LLB");
		std::shared_ptr<FortrendPMCavitySubsystem> pm2 = d->kernel->getKernelModule<FortrendPMCavitySubsystem>("PM2");
		bool  ups = false;
		bool isCloseInsertingPlateTM = false;
		bool isCloseInsertingPlateLLA = false;
		bool isCloseInsertingPlateLLB = false;
		bool changed = false;
		bool cmdstate = true;
		bool isCloseAngleValveTM = false;
		bool isCloseAngleValveLLA = false;
		bool isCloseAngleValveLLB = false;
#if 0
		while (true)
		{
			pm2->setIsRunning(d->running);
			tm->readBit("MR35507", ups);
			tm->readBit("MR50001", d->plcauto);
			if (ups!=d->hasUPS)
			{
				d->hasUPS = ups;
				changed = true;
				if (d->hasUPS){
					logError("Cycle", "检测到断电，等待动作执行完后停止cycle");
				}
				else{
					logError("Cycle", "检测到断电恢复，重新开始cycle");
				}
			}


			if (d->hasUPS){
				if (d->robot_step_wafer_finished){
					d->robot_step_wafer_finished = false;
					logError("Cycle", "动作已完成，暂停cycle");
					onPause();
					/*d->running = false;
					d->onUpdateControlEnabled("reset_pbt", true);
					d->onUpdateControlEnabled("execute_pbt", true);
					d->onUpdateControlEnabled("pause_pbt", false);*/
				}
				if (!d->running){
					if (tm&&tm->getState() == IKernelSubSystem::State::SUB_NORMAL){
						if (tm->getInsertingPlateValveOpend() && cmdstate){
							auto cmd = tm->createCloseInsertingPlateValveCommand();
							tm->startCommand(cmd);
							cmd->wait();
							if (!cmd->hasError()){
								isCloseInsertingPlateTM = true;
							}
							else{
								cmdstate = false;
							}
						}
						if (tm->getAngleValveOpend() && cmdstate){
							auto cmd = tm->createCloseAngleValveCommand();
							tm->startCommand(cmd);
							cmd->wait();
							if (!cmd->hasError()){
								isCloseAngleValveTM = true;
							}
							else{
								cmdstate = false;
							}
						}
					}
					if (lk1&&lk1->getState() == IKernelSubSystem::State::SUB_NORMAL){
						if (lk1->getInsertingPlateValveOpend() && cmdstate){
							auto cmd = lk1->createCloseInsertingPlateValveCommand();
							lk1->startCommand(cmd);
							cmd->wait();
							if (!cmd->hasError()){
								isCloseInsertingPlateLLA = true;
							}
							else{
								cmdstate = false;
							}
						}
						if (lk1->getAngleValveOpend() && cmdstate){
							auto cmd = lk1->createCloseAngleValveCommand();
							lk1->startCommand(cmd);
							cmd->wait();
							if (!cmd->hasError()){
								isCloseAngleValveLLA = true;
							}
							else{
								cmdstate = false;
							}
						}
					}
					if (lk2&&lk2->getState() == IKernelSubSystem::State::SUB_NORMAL){
						if (lk2->getInsertingPlateValveOpend() && cmdstate){
							auto cmd = lk2->createCloseInsertingPlateValveCommand();
							lk2->startCommand(cmd);
							cmd->wait();
							if (!cmd->hasError()){
								isCloseInsertingPlateLLB = true;
							}
							else{
								cmdstate = false;
							}
						}
						if (lk2->getAngleValveOpend() && cmdstate){
							auto cmd = lk2->createCloseAngleValveCommand();
							lk2->startCommand(cmd);
							cmd->wait();
							if (!cmd->hasError()){
								isCloseAngleValveLLB = true;
							}
							else{
								cmdstate = false;
							}
						}
					}
				}
				
				
			}
			else{
				if (!d->running&&changed){
					if (tm&&tm->getState() == IKernelSubSystem::State::SUB_NORMAL){
						if (!tm->getAngleValveOpend() && isCloseAngleValveTM&& cmdstate){
							auto cmd = tm->createOpenAngleValveCommand();
							tm->startCommand(cmd);
							cmd->wait();
							if (!cmd->hasError()){
								isCloseAngleValveTM = false;
							}
							else{
								cmdstate = false;
							}
						}
						if (!tm->getInsertingPlateValveOpend() && isCloseInsertingPlateTM&& cmdstate){
							auto cmd = tm->createOpenInsertingPlateValveCommand();
							tm->startCommand(cmd);
							cmd->wait();
							if (!cmd->hasError()){
								isCloseInsertingPlateTM = false;
							}
							else{
								cmdstate = false;
							}
						}
						
					}
					if (lk1&&lk1->getState() == IKernelSubSystem::State::SUB_NORMAL){
						if (!lk1->getAngleValveOpend() && isCloseAngleValveLLA&& cmdstate){
							auto cmd = lk1->createOpenAngleValveCommand();
							lk1->startCommand(cmd);
							cmd->wait();
							if (!cmd->hasError()){
								isCloseAngleValveLLA = false;
							}
							else{
								cmdstate = false;
							}
						}
						if (!lk1->getInsertingPlateValveOpend() && isCloseInsertingPlateLLA&& cmdstate){
							auto cmd = lk1->createOpenInsertingPlateValveCommand();
							lk1->startCommand(cmd);
							cmd->wait();
							if (!cmd->hasError()){
								isCloseInsertingPlateLLA = false;
							}
							else{
								cmdstate = false;
							}
						}
					}
					if (lk2&&lk2->getState() == IKernelSubSystem::State::SUB_NORMAL){
						if (!lk2->getAngleValveOpend() && isCloseAngleValveLLB&& cmdstate){
							auto cmd = lk2->createOpenAngleValveCommand();
							lk2->startCommand(cmd);
							cmd->wait();
							if (!cmd->hasError()){
								isCloseAngleValveLLB = false;
							}
							else{
								cmdstate = false;
							}
						}
						if (!lk2->getInsertingPlateValveOpend() && isCloseInsertingPlateLLB&& cmdstate){
							auto cmd = lk2->createOpenInsertingPlateValveCommand();
							lk2->startCommand(cmd);
							cmd->wait();
							if (!cmd->hasError()){
								isCloseInsertingPlateLLB = false;
							}
							else{
								cmdstate = false;
							}
						}
						
					}
					changed = false;
					onStart();
				}

				
			}
			Sleep(50);
		}

#endif
	}

	void QSlotTransferCycleVTMWidget::resetAction(){
		Q_D(QSlotTransferCycleVTMWidget);
		d->resetAction();
	}

	void QSlotTransferCycleVTMWidget::initPMCavityParamEdieTableWidget(){
		Q_D(QSlotTransferCycleVTMWidget);


		addAnPMItem("PM1");
		addAnPMItem("PM2");
		addAnPMItem("PM3");
		addAnPMItem("PM4");
	}

	void QSlotTransferCycleVTMWidget::addAnPMItem(const QString name){
		Q_D(QSlotTransferCycleVTMWidget);
		
		int row_count = d->ui->pm_cavity_param_edit_tbw->rowCount();
		d->ui->pm_cavity_param_edit_tbw->insertRow(row_count);

		QTableWidgetItem* item = new QTableWidgetItem;
		item->setText(name);
		item->setFlags(item->flags() & ~Qt::ItemIsEditable);
		d->ui->pm_cavity_param_edit_tbw->setItem(row_count, 0, item);

		//int row, int column, double min_value, double max_value, double single_step, double value, int decimals_value
		addEditTableWidgetItemDoubleSpinBox(row_count, 1, 60.0, 100.0, 1, 100);//电机升降开始位置
		addEditTableWidgetItemComboBox(row_count, 2,1);//电机旋转角度/°
		//addEditTableWidgetItemDoubleSpinBox(row_count, 3,0,6,1,1);
		addEditTableWidgetItemDoubleSpinBox(row_count, 4, 60.0, 100.0, 1, 100);//电机旋转位置
		addEditTableWidgetItemDoubleSpinBox(row_count, 5, 60.0, 120.0, 1, 120);//电机升降结束位置/mm

		//旋转次数
		QSpinBox* Rotation_count_spx = new QSpinBox();
		Rotation_count_spx->setMinimum(0);
		Rotation_count_spx->setMaximum(6);
		Rotation_count_spx->setSingleStep(1);
		d->ui->pm_cavity_param_edit_tbw->setCellWidget(row_count, 3, Rotation_count_spx);

		addEditTableWidgetItemDoubleSpinBox(row_count, 6, 0, 15.0, 1, 15.0);//工艺时间
	}

	void QSlotTransferCycleVTMWidget::executeEFEMTransfer()
	{
		Q_D(QSlotTransferCycleVTMWidget);
		d->executeEFEMTransfer();
	}

	void QSlotTransferCycleVTMWidget::executeLLATransfer()
	{
		Q_D(QSlotTransferCycleVTMWidget);
		d->executeLLATransfer();
	}

	void QSlotTransferCycleVTMWidget::executeLLBTransfer()
	{
		Q_D(QSlotTransferCycleVTMWidget);
		d->executeLLBTransfer();
	}

	void QSlotTransferCycleVTMWidget::executePM1Transfer()
	{
		Q_D(QSlotTransferCycleVTMWidget);
		d->executePM1Transfer();
	}

	void QSlotTransferCycleVTMWidget::executePM2Transfer()
	{
		Q_D(QSlotTransferCycleVTMWidget);
		d->executePM2Transfer();
	}

	void QSlotTransferCycleVTMWidget::executePM3Transfer()
	{
		Q_D(QSlotTransferCycleVTMWidget);
		d->executePM3Transfer();
	}

	void QSlotTransferCycleVTMWidget::executePM4Transfer()
	{
		Q_D(QSlotTransferCycleVTMWidget);
		d->executePM4Transfer();
	}

	void QSlotTransferCycleVTMWidget::executeTMTransfer()
	{
		Q_D(QSlotTransferCycleVTMWidget);
		d->executeTMTransfer();
	}


	void QSlotTransferCycleVTMWidget::onGetStep(){
		Q_D(QSlotTransferCycleVTMWidget);
		logInform("Cycle", Poco::format("%s = %d", d->loadlock1_process_name, d->loadlock1_auto_step).c_str());
		logInform("Cycle", Poco::format("%s = %d", d->loadlock2_process_name, d->loadlock2_auto_step).c_str());
		logInform("Cycle", Poco::format("%s = %d", d->robot_process_name, d->robot_auto_step).c_str());
		logInform("Cycle", Poco::format("%s = %d", d->vacuum_process_name, d->vacuum_auto_step).c_str());
		logInform("Cycle", Poco::format("%s = %d", d->efem_process_name, d->efem_auto_step).c_str());
		logInform("Cycle", Poco::format("%s = %d", d->pm_process_name, d->pm_auto_step).c_str());
		logInform("Cycle", Poco::format("%s = %d", d->update_process_name, d->update_auto_step).c_str());
	}

	void QSlotTransferCycleVTMWidget::update_cycle_data(){
		Q_D(QSlotTransferCycleVTMWidget);
		d->ui->cycle_finished_times_spx->setValue(d->finished_time_lla);
		d->ui->cycle_finished_times_spx_2->setValue(d->finished_time_llb);
	}

	void QSlotTransferCycleVTMWidget::updateProcessControlEnabled(const bool enabled){
		Q_D(QSlotTransferCycleVTMWidget);
		
		d->ui->cycle_setting_times_sbx->setEnabled(enabled);
		d->ui->cycle_setting_times_sbx_2->setEnabled(enabled);
		d->ui->sequence_edit_tbw->setEnabled(enabled);
		d->ui->pm_cavity_param_edit_tbw->setEnabled(enabled);
		d->ui->horizontalLayout->setEnabled(enabled);
		d->ui->add_an_item_pbt->setEnabled(enabled);
		d->ui->delete_the_selected_item_pbt->setEnabled(enabled);
		d->ui->clear_sequence_pbt->setEnabled(enabled);
		d->ui->load_sequence_pbt->setEnabled(enabled);
		d->ui->execute_pbt->setEnabled(enabled);
	}

	void QSlotTransferCycleVTMWidget::updateControlEnabled(const QString control, const bool enabled){
		Q_D(QSlotTransferCycleVTMWidget);
		if (control == "execute_pbt")
		{
			d->ui->execute_pbt->setEnabled(enabled);
		}
		else if (control == "pause_pbt")
		{
			d->ui->pause_pbt->setEnabled(enabled);
		}
		else if (control == "reset_pbt")
		{
			d->ui->reset_pbt->setEnabled(enabled);
		}
		else if (control == "loadlock1_put_cassette_finished_pbt")
		{
			d->ui->loadlock1_put_cassette_finished_pbt->setEnabled(enabled);
		}
		else if (control == "loadlock2_put_cassette_finished_pbt")
		{
			d->ui->loadlock2_put_cassette_finished_pbt->setEnabled(enabled);
		}
		else{

		}
		
	}

	void QSlotTransferCycleVTMWidget::addEditTableWidgetItemDoubleSpinBox(int row, int column, double min_value, double max_value, double single_step, double value, int decimals_value){
		Q_D(QSlotTransferCycleVTMWidget);
		QDoubleSpinBox *dsb = new QDoubleSpinBox();
		dsb->setMaximum(max_value);
		dsb->setMinimum(min_value);
		dsb->setDecimals(decimals_value);
		dsb->setSingleStep(single_step);
		dsb->setValue(value);
		d->ui->pm_cavity_param_edit_tbw->setCellWidget(row, column, dsb);
	}
	
	void QSlotTransferCycleVTMWidget::addEditTableWidgetItemComboBox(int row, int column, int value)
	{
		Q_D(QSlotTransferCycleVTMWidget);
		QComboBox* dcb = new QComboBox();
		QMap<QString, int> AngleMap;
		AngleMap.insert("0",0);
		AngleMap.insert("60", 60);
		AngleMap.insert("120", 120);
		AngleMap.insert("180", 180);
		AngleMap.insert("240", 240);
		AngleMap.insert("300", 300);
		AngleMap.insert("360", 360);
		foreach(const QString & str, AngleMap.keys())
			dcb->addItem(str, AngleMap.value(str));

		dcb->setCurrentIndex(value);

		d->ui->pm_cavity_param_edit_tbw->setCellWidget(row,column,dcb);
	}


	void QSlotTransferCycleVTMWidget::startProcessingThreads()
	{
		Q_D(QSlotTransferCycleVTMWidget);

		//d->efemUnkownStatusTasks = taskManager.getEfemUnkownStatusTasks();
		//taskManager.updateTaskStatus(d->efemUnkownStatusTasks.at(0).taskId, UnifiedWaferTask::TaskType::EFEM_TRANSFER, UnifiedWaferTask::Status::QUEUED);
		//Sleep(500);

		//d->efemPendingTasks = taskManager.getEfemPendingTasks();
		//taskManager.updateTaskStatus(d->efemPendingTasks.at(0).taskId, UnifiedWaferTask::TaskType::LOADLOCK_TRANSFER,UnifiedWaferTask::Status::QUEUED);
		//Sleep(500);
		//

		//d->loadLockBPendingTasks = taskManager.getLoadLockPendingTasks("LLB");
		//taskManager.updateTaskStatus(d->loadLockBPendingTasks.at(0).taskId, UnifiedWaferTask::TaskType::LOADLOCK_TRANSFER, UnifiedWaferTask::Status::COMPLETED);
	
		//Sleep(500);

		//d->loadLockACompletedTasks = taskManager.getLoadLockCompletedTasks("LLA");
		//taskManager.updateTaskStatus(d->loadLockACompletedTasks.at(0).taskId, UnifiedWaferTask::Status::COMPLETED,UnifiedWaferTask::TaskType::PM_PROCESS);
		//
		//Sleep(500);

		std::thread thd_efem(&QSlotTransferCycleVTMWidget::executeEFEMTransfer, this);
		thd_efem.detach();
		
		//std::thread thd_LoadLockA(&QSlotTransferCycleVTMWidget::executeLLATransfer, this);
		//thd_LoadLockA.detach();

		std::thread thd_LoadLockB(&QSlotTransferCycleVTMWidget::executeLLBTransfer, this);
		thd_LoadLockB.detach();

		std::thread thd_vacumn(&QSlotTransferCycleVTMWidget::startVacuumAction, this);
		thd_vacumn.detach();

		std::thread thread_pm1(&QSlotTransferCycleVTMWidget::executePM1Transfer,this);
		thread_pm1.detach();

	}

}