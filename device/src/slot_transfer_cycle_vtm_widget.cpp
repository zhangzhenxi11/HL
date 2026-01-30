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
#include <condition_variable>
#include <atomic>
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
#include "STATIONMODE/fortrend_StationMode_subsystem.h" 
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
#include  "pm_recipe_widget.h"

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

#include "UnifiedWaferTask.h"
#include "TaskManager.h"
#include "CycleStateSnapshot.h"  // 2025-10-28: 状态快照保存与恢复

#include "ThreadSafeStateMachine.h"

#if _MSC_VER >1600
#pragma execution_character_set("utf-8")
#endif

#define CYCLE_SIM_MODE1

#define DEBUG_LOAD_SNAPSHOT1

#define DEBUG_TEST_PM1

// 全局任务管理器
TaskManager& taskManager = TaskManager::getInstance();

namespace FC{
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

	class CheckableMutex {
	private:
		std::mutex m_mutex;
		std::atomic<bool> m_locked{ false };

	public:
		void lock() {
			m_mutex.lock();
			m_locked = true;
		}

		void unlock() {
			m_locked = false;
			m_mutex.unlock();
		}

		bool try_lock() {
			if (m_mutex.try_lock()) {
				m_locked = true;
				return true;
			}
			return false;
		}

		bool is_locked() const {
			return m_locked;
		}
	};



	/**
	* QSlotTransferCycleWTMWidgetPrivate
	*/
	class QSlotTransferCycleVTMWidgetPrivate : public KernelListener < IKernelCommand > {
	public:
		Q_DECLARE_PUBLIC(QSlotTransferCycleVTMWidget)
		QSlotTransferCycleVTMWidgetPrivate(QSlotTransferCycleVTMWidget*p);
	public:
		void startVacuumAction();
		void resetAction();
		void setRunning(const bool value);
		void setLoadLock1PutCassetteFinished();
		void setLoadLock2PutCassetteFinished();
		void onGetStep();
		//获取UI流程队列
		bool setTransferSequence();

		//衡流获取UI流程队列
		bool setHLTransferSequence();

		//bool setPMCavityParameter();

		void onUpdateCycleInfo();

		void onStatusChanged();

		void onUpdateProcessControlEnabled(const bool value);

		void onUpdateControlEnabled(const QString control, const bool enabled);

		void onUpdateLightButtonStatus(const QString control, const int color);

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

		void RerunTheFilmCycle();

		void startAllThreads();

		void pauseAllThreads();

		void stopAllThreads();

		//上料互锁条件
		bool llA_condition = false; //true 满足互锁 ， false不满足没锁

		bool llB_condition = false;//true 满足互锁 ，  false不满足没锁


		//情景：当一个LL的料下到位，另一个没下料时，为了不让另一个LL继续发送上料请求的互锁条件
		bool isLoadingInterlock(const std::string& LLName);

	private:
		QSlotTransferCycleVTMWidget * q_ptr;
		Ui::SlotTransferCycleVTMWidget *ui;
		std::shared_ptr<IKernel> kernel = 0;
		std::shared_ptr<FortrendVTMSignalTower> tower = 0;
		std::shared_ptr<FortrendTMCavitySubsystem> tmplc;
		bool plcauto = false;
		bool reset_finish = false;
		std::chrono::steady_clock::time_point LLA_start_time; //检测真空
		std::chrono::steady_clock::time_point LLB_start_time; //检测真空

		std::mutex updateLPData_mtx;
		
		typedef enum FilmTransferMode
		{
			Formula_Go_Up_And_Down = 0,
			Formula_Double_Up_And_Down =1
		};

		FilmTransferMode currentTransferMode = FilmTransferMode::Formula_Go_Up_And_Down; //传片模式
		std::string  filmTransferMode;//模式名

		std::shared_ptr<EFEMWaferRobotSubsystem> ewtr = nullptr;
		std::shared_ptr<EFEMLPSubsystem> elp1 = nullptr;
		std::shared_ptr<EFEMLPSubsystem> elp2 = nullptr;
		std::shared_ptr<FortrendLoadLockSubsystem> lk1 = nullptr;
		std::shared_ptr<FortrendLoadLockSubsystem> lk2 = nullptr;

		std::shared_ptr<EFEMLPSubsystem> elp = nullptr;
		std::shared_ptr<FortrendLoadLockSubsystem> lk = nullptr;

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

		bool tool_allow_get_wafer_LLA = false; // LLA上料请求   true呼叫LP上料， false LP上料完成

		bool tool_allow_put_wafer_LLA = false; // LLA下料请求   true呼叫LP下料， false LP下料完成

		bool tool_allow_get_wafer_LLB = false; // LLB上料请求

		bool tool_allow_put_wafer_LLB = false; // LLB下料请求

	
		std::mutex current_loadlock_mux;
		// 添加当前处理的LoadLock标识
		std::string current_loadlock; // "LLA" 或 "LLB"

		bool tool_allow_pm1_get_wafer = false;//true呼叫pm1上料，  false pm1上料完成
		bool tool_allow_pm1_put_wafer = false; //true呼叫pm1下料，  false pm1下料完成

		bool tool_allow_pm2_get_wafer = false;
		bool tool_allow_pm2_put_wafer = false;

		bool tool_allow_pm3_get_wafer = false;
		bool tool_allow_pm3_put_wafer = false;

		bool tool_allow_pm4_get_wafer = false;
		bool tool_allow_pm4_put_wafer = false;

		bool tool_allow_lla = false; //LLA呼叫的pm 上下料

		bool tool_allow_llb = false;//LLb呼叫的pm  上下料


		//int selectPmEnableList[4] = { 0, 0, 0, 0 }; // 工艺腔使能数组
		//std::array<int, 4> selectPmEnableList;

		bool CheckTMVacuumMeetsStandard(int preStep);
		//CSR 取放晶圆到LL时检测条件
		bool CheckLLVacuumMeetsStandard(std::string llName, int preStep);

		int originTaskSize;  //待传输任务的大小

		int lp1TaskSize;

		int lp2TaskSize;

		//2025-7-16 新增
		std::vector<UnifiedWaferTask> tasks; //总任务队列

		std::vector<UnifiedWaferTask> worktasks; //实际工作队列

		CheckableMutex efem_robot_mutex; //efem 上下料锁

		std::mutex feeding_mutex; //上料锁

		std::mutex blanking_mutex;//下料锁
		

		/************************EFEM*********************************/

		std::vector<UnifiedWaferTask> efemUnkownStatusTasks; //未知状态的数组

		std::vector<UnifiedWaferTask> efemPendingTasks;  //工艺前,待上料数组
									  
		std::vector<UnifiedWaferTask> efemCompletedTasks; //工艺前，放料完成的数组

		std::vector<UnifiedWaferTask> efemReturnPendingTasks;  //工艺后,待下料数组

		std::vector<UnifiedWaferTask> efemReturnCompletedTasks; //工艺后，下料完成的数组

		std::vector<UnifiedWaferTask> efemAllocationLLAInitTasks;
		
		std::vector<UnifiedWaferTask> efemAllocationLLBInitTasks;
			
		//2026-1-23 双上料时缓存两个任务的完整信息，避免case 157更新状态后数组变化
		UnifiedWaferTask cached_task_first;  // 第一片料缓存
		UnifiedWaferTask cached_task_second; // 第二片料缓存
			
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

		std::vector<UnifiedWaferTask> pm2PendingTasks;

		std::vector<UnifiedWaferTask> pm2CompletedTasks;
		// IN_PROGRESS
		std::vector<UnifiedWaferTask> pm2ProgressingTasks;

		std::vector<UnifiedWaferTask> pm3PendingTasks;

		std::vector<UnifiedWaferTask> pm3CompletedTasks;
		// IN_PROGRESS
		std::vector<UnifiedWaferTask> pm3ProgressingTasks;

		std::vector<UnifiedWaferTask> pm4PendingTasks;

		std::vector<UnifiedWaferTask> pm4CompletedTasks;
		// IN_PROGRESS
		std::vector<UnifiedWaferTask> pm4ProgressingTasks;


		PMCavityProcessParameters pm_process_param[3];
		std::string armWferTarget = "";
		std::string target_loadlock = "";



		bool cycleFinished_lla = false; //lp1完成所有循环
		bool cycleFinished_llb = false; //lp2完成所有循环

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
		bool pm1_step_once_finished = true;
		bool pm3_step_once_finished = true;
		bool pm4_step_once_finished = true;

		bool robot_step_wafer_finished = false;
		int PMMessagecount = 2;
		

		//bool running = false;
		//bool ispause = false;

		std::vector<std::thread> threads;
		std::mutex mtx;
		std::condition_variable cv;
		std::atomic<bool> running{ false };       //控制线程是否应该执行工作（true=执行，false=暂停）
		std::atomic<bool> ispause{ false };       //running = false 暂停
		std::atomic<bool> stopRequested{ false }; //指示线程是否应该完全退出（true=线程应该结束）

		int robot_auto_step=0;
		int loadlock1_auto_step = 0;
		int loadlock2_auto_step = 0;
		int vacuum_auto_step = 0;
		int pm_auto_step;
		std::atomic<int> pm1_auto_step;
		std::atomic<int> pm2_auto_step;
		std::atomic<int> pm3_auto_step;
		std::atomic<int> pm4_auto_step;
		int efem_auto_step= 0;
		int update_auto_step= 0;
		std::atomic<bool> needReset_EFEM{ false };
		std::atomic<bool> needReset_LLA{ false };
		std::atomic<bool> needReset_LLB{ false };
		std::atomic<bool> needReset_PM1{ false };
		std::atomic<bool> needReset_PM2{ false };
		std::atomic<bool> needReset_PM3{ false };
		std::atomic<bool> needReset_PM4{ false };
		std::atomic<bool> needReset_Update{ false };

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

		/*4个pm分开*/
		bool pm1_allow_get_put_wafer = false;//PM1允许取放  
		bool pm1_allow_goto_craft = false;   //PM1允许转到工艺

		bool pm2_allow_get_put_wafer = false;//PM2允许取放  
		bool pm2_allow_goto_craft = false;   //PM2允许转到工艺

		bool pm3_allow_get_put_wafer = false;//PM3允许取放  
		bool pm3_allow_goto_craft = false;   //PM3允许转到工艺

		bool pm4_allow_get_put_wafer = false;//PM4允许取放  
		bool pm4_allow_goto_craft = false;   //PM4允许转到工艺

		//int pm_allow_get_put_wafer_list[4] = {0,0,0,0};


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

		bool current_lp_cycle = false; 

		bool hasUPS = false;

		std::string loadlock_process_name;
		const std::string loadlock1_process_name = "LLA流程步骤";
		const std::string loadlock2_process_name = "LLB流程步骤";
		const std::string robot_process_name = "机械手流程步骤";
		const std::string vacuum_process_name = "真空控制流程步骤";
		const std::string pm_process_name = "PM流程步骤";
		const std::string pm1_process_name = "PM1流程步骤";
		const std::string pm2_process_name = "PM2流程步骤";
		const std::string pm3_process_name = "PM3流程步骤";
		const std::string pm4_process_name = "PM4流程步骤";

		const std::string reset_process_name = "reset";
		const std::string update_process_name = "update";
		const std::string efem_process_name = "EFEM流程步骤";
		const int sleep_time = 5000;
		int robot_selected_arm = 0;  //机械手臂选择

	public:
		const std::string module_name = "Cycle";

		// WPH测试相关变量
		std::atomic<bool> wph_test_running{ false };
		std::atomic<bool> wph_test_stop_requested{ false };
		int wph_cycle_count = 10;
		int wph_completed_count = 0;
		std::thread wph_test_thread;

		// WPH测试执行函数
		void executeWphTest();

	private:
		void logFailed(const std::string station_name, const std::string log);

		void logFailedNotNormal(const std::string station_name, const std::string process_name, const int step);
		
		void logFailedExcuteCommandHasError(const std::string station_name, const std::string command_name, const std::string process_name, const int step);
		
		// 2025-10-28: 状态快照保存与恢复功能
		void saveCurrentStateSnapshot(const std::string& errorMsg, const std::string& errorLocation);

		bool loadStateSnapshot(const std::string& filePath);

		CycleStateSnapshot::Snapshot captureCurrentState(const std::string& errorMsg, const std::string& errorLocation);

		void restoreStateFromSnapshot(const CycleStateSnapshot::Snapshot& snapshot);
	};

	QSlotTransferCycleVTMWidgetPrivate::QSlotTransferCycleVTMWidgetPrivate(QSlotTransferCycleVTMWidget*p)
		:q_ptr(p), pm1_auto_step(10), pm2_auto_step(10), pm3_auto_step(10), pm4_auto_step(10){
	
	}

	void QSlotTransferCycleVTMWidgetPrivate::onUpdateCycleInfo(){
		QMetaObject::invokeMethod(q_ptr, "update_cycle_data", Qt::AutoConnection);
	}

	void QSlotTransferCycleVTMWidgetPrivate::onStatusChanged()
	{
		QMetaObject::invokeMethod(q_ptr, "status_Changed", Qt::AutoConnection);
	}

	void QSlotTransferCycleVTMWidgetPrivate::onUpdateProcessControlEnabled(const bool enabled){
		QMetaObject::invokeMethod(q_ptr, "updateProcessControlEnabled", Qt::AutoConnection,
			Q_ARG(bool, enabled));
	}

	void QSlotTransferCycleVTMWidgetPrivate::onUpdateControlEnabled(const QString control, const bool enabled){
		QMetaObject::invokeMethod(q_ptr, "updateControlEnabled", Qt::AutoConnection,
			Q_ARG(QString, control), Q_ARG(bool, enabled));
	}

	void QSlotTransferCycleVTMWidgetPrivate::onUpdateLightButtonStatus(const QString control, const int color)
	{
		QMetaObject::invokeMethod(q_ptr, "updateLightButtonStatus", Qt::AutoConnection,
			Q_ARG(QString, control), Q_ARG(int, color));
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
			if (task.selectPmEnableList[i] == 1)
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
			if (task.selectPmEnableList[i] == 1)
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
		efemUnkownStatusTasks = taskManager.getEfemUnkownStatusTasks();						// LP中初始状态的晶圆
		loadLockAPendingTasks = taskManager.getLoadLockPendingTasks("LLA");				    // 5上料 破过真空，但没抽真空的
		loadLockACompletedTasks = taskManager.getLoadLockCompletedTasks("LLA");			    // 6上料 抽完真空，并取走的
		loadLockAReturnPendingTasks = taskManager.getLoadLockReturnPendingTasks("LLA");     // 7下料 待放到LL的晶圆数量
		loadLockAReturnCompletedTasks = taskManager.getLoadLockReturnCompletedTasks("LLA"); // 8下料 放到LL的晶圆数量
	}

	void QSlotTransferCycleVTMWidgetPrivate::UpdateLLBSubTransferDatas()
	{
		efemUnkownStatusTasks = taskManager.getEfemUnkownStatusTasks();						// LP中初始状态的晶圆
		loadLockBPendingTasks = taskManager.getLoadLockPendingTasks("LLB");					// 5上料 破过真空，但没抽真空的
		loadLockBCompletedTasks = taskManager.getLoadLockCompletedTasks("LLB");				// 6上料 抽完真空，并取走的
		loadLockBReturnPendingTasks = taskManager.getLoadLockReturnPendingTasks("LLB");		// 7下料 待放到LL的晶圆数量
		loadLockBReturnCompletedTasks = taskManager.getLoadLockReturnCompletedTasks("LLB"); // 8下料 放到LL的晶圆数量
	}

	void QSlotTransferCycleVTMWidgetPrivate::UpdateEfemSubTransferDatas()
	{
		efemUnkownStatusTasks = taskManager.getEfemUnkownStatusTasks(); //未知
		efemPendingTasks = taskManager.getEfemPendingTasks(); //要上料的晶圆
		efemReturnPendingTasks = taskManager.getEfemRuturnPendingTasks(); //已经到LL上的晶圆,待下料到efem
		efemReturnCompletedTasks = taskManager.getEfemRuturnCompletedTasks(); //EFEM下料完成的任务

	}

	void QSlotTransferCycleVTMWidgetPrivate::UpdatePmSubTransferDatas(std::string pmName)
	{
		if(pmName == "PM1")
		{
			pmPendingTasks = taskManager.getPMPendingTasks(pmName);
			pmCompletedTasks = taskManager.getPMPendingTasks(pmName);
			pmProgressingTasks = taskManager.getPMPendingTasks(pmName);
		}
		else if (pmName == "PM2")
		{
			pm2PendingTasks = taskManager.getPMPendingTasks(pmName);
			pm2CompletedTasks = taskManager.getPMPendingTasks(pmName);
			pm2ProgressingTasks = taskManager.getPMPendingTasks(pmName);
		}
		else if (pmName == "PM3")
		{
			pm3PendingTasks = taskManager.getPMPendingTasks(pmName);
			pm3CompletedTasks = taskManager.getPMPendingTasks(pmName);
			pm3ProgressingTasks = taskManager.getPMPendingTasks(pmName);
		}
		else
		{
			pm4PendingTasks = taskManager.getPMPendingTasks(pmName);
			pm4CompletedTasks = taskManager.getPMPendingTasks(pmName);
			pm4ProgressingTasks = taskManager.getPMPendingTasks(pmName);
		}
	}

	/*
	*EFEM大气流程
	*每一片的来源位置，目标位置，在UI中都明确的，分上下料，那么处理时候达到此状态下的晶圆
	*/
	
	void QSlotTransferCycleVTMWidgetPrivate::executeEFEMTransfer()
	{
		try {

		if (ewtr == nullptr || elp1 == nullptr || elp2 == nullptr || lk1 == nullptr || lk2 == nullptr|| ealigner == nullptr)
		{
			ewtr = kernel->getKernelModule<EFEMWaferRobotSubsystem>("EWTR");
			elp1 = kernel->getKernelModule<EFEMLPSubsystem>("ELP1");
			elp2 = kernel->getKernelModule<EFEMLPSubsystem>("ELP2");
			lk1 = kernel->getKernelModule<FortrendLoadLockSubsystem>("LLA");
			lk2 = kernel->getKernelModule<FortrendLoadLockSubsystem>("LLB");
			ealigner = kernel->getKernelModule<EFEMAlignerSubsystem>("EALIGNER");
		}

			while (!stopRequested)
			{
				Sleep(500);
				// Wait until running is true
				{
					std::unique_lock<std::mutex> lock(mtx);
					cv.wait(lock, [this] { return running.load() || stopRequested.load(); });//也判断stopRequested，是为了退出线程时，也能唤醒，退出当前函数
				}

				if (stopRequested) break;

				// 检查是否需要重置
				if (needReset_EFEM.load()) {
					efem_auto_step = 10;
					needReset_EFEM = false;
					logWarn("EFEM", "重置，跳过本次循环剩余部分.");
					continue; // 跳过本次循环剩余部分
				}

				efem_step_once_finished = false;
				switch (efem_auto_step)
				{
				case 10:
				{
					UpdateEfemSubTransferDatas();
					Sleep(500);
					// 检查循环完成状态
					if (efemReturnCompletedTasks.size() == originTaskSize && (originTaskSize != 0))
					{
						if(efemReturnCompletedTasks.size() > 0)
						{
							if (efemReturnCompletedTasks.at(0).source == UnifiedWaferTask::Location::LP1)
							{
								if (!lp1_cycle_one_time_finished)
								{
									logWarn("cycle", "当前一次lp1循环完成.");
									lp1_cycle_one_time_finished = true;//一次lp1循环完成
								}
							}
							else if (efemReturnCompletedTasks.at(0).source == UnifiedWaferTask::Location::LP2)
							{
								if (!lp2_cycle_one_time_finished)
								{
									logWarn("cycle", "当前一次lp2循环完成.");
									lp2_cycle_one_time_finished = true;//一次lp2循环完成
								}
							}
						}

						Sleep(100);

					}
					else
					{
						efem_auto_step = 11;
					}
				}
				break;

				case 11:
				{
					// 处理下料请求（优先）
					if (tool_allow_put_wafer_LLA || efemReturnPendingTasks.size() > 0) {

						if (!efem_robot_mutex.is_locked())
						{
							efem_robot_mutex.lock();
						}
						current_loadlock = "LLA";
						efem_auto_step = 200;  // 下料流程
					}
					else if (tool_allow_put_wafer_LLB || efemReturnPendingTasks.size() > 0) {
						if (!efem_robot_mutex.is_locked())
						{
							efem_robot_mutex.lock();
						}
						current_loadlock = "LLB";
						efem_auto_step = 200;  // 下料流程
					}
					// 处理上料请求
					else if (tool_allow_get_wafer_LLA || efemPendingTasks.size() > 0) {
						if (!efem_robot_mutex.is_locked())
						{
							efem_robot_mutex.lock();
						}
						current_loadlock = "LLA";
						efem_auto_step = 100;  // 上料流程
					}
					else if (tool_allow_get_wafer_LLB || efemPendingTasks.size() > 0) {
						if (!efem_robot_mutex.is_locked())
						{
							efem_robot_mutex.lock();
						}
						current_loadlock = "LLB";
						efem_auto_step = 100;  // 上料流程
					}
					else {
						Sleep(10);
					}
				}
				break;
				#pragma region 给TOOL上料
				case 100:
				{
					logWarn("EFEM", "efem_auto_step:100,给%s上料 Lock thread...", current_loadlock.c_str());

					if (efemUnkownStatusTasks.size() > 0)
					{
						elp = efemUnkownStatusTasks.at(0).source == UnifiedWaferTask::Location::LP1 ? elp1 : elp2;
						//找到当前loadLock的elp，lk
						if (current_loadlock == "LLA")
						{
							lk = lk1;
						}
						else
						{
							lk = lk2;
						}
						logWarn("EFEM", "step:100,当前lk:%s", lk->getName().c_str());

						//UI中确定的数据，elp就是确定的
						//lk = efemUnkownStatusTasks.at(0).target == UnifiedWaferTask::Location::LLA ? lk1 : lk2;

						if (elp == nullptr && lk == nullptr)
						{
							logFailedExcuteCommandHasError(elp->getName(), "elp or lk is nullptr", efem_process_name, efem_auto_step);
						}

						if (elp->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
							if (ui->simulation_cbx->checkState() == Qt::CheckState::Checked)
							{
								logInform(elp->getName().c_str(), "step:100,模拟EFEM程序....");
								efem_auto_step = 101;
							}
							else
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

						}
						else {
							logFailedNotNormal(elp->getName(), efem_process_name, efem_auto_step);
						}
					}
					else
					{
						Sleep(500);
					}
				}
				break;
				case 101:
				{
					logWarn("EFEM", "efem_auto_step:%d", efem_auto_step);
					auto cassManager = lk->getKernel()->getKernelModule<FortrendCassetteManager>();
					auto station_cass_lk = cassManager->getCassette(lk.get());
					std::vector<Cassette::Mapping> lkmaps;
					int count = 0;

					if (currentTransferMode == FilmTransferMode::Formula_Go_Up_And_Down)
					{
						lkmaps = station_cass_lk->getAllMapping();
						for (int i = 0; i < lkmaps.size(); i++)
						{
							if (lkmaps[i] == Cassette::Empty) //空片的槽号
							{
								count = 1; 
								logWarn(lk->getName().c_str(),"当前传片模式是:Go_Up_And_Down.");
								break;
							}
						}	
					}
					else if (currentTransferMode == FilmTransferMode::Formula_Double_Up_And_Down)
					{
						logWarn(lk->getName().c_str(), "当前传片模式是:Double_Up_And_Down.");
						lkmaps = station_cass_lk->getAllMapping();

						//2026-1-19  lp中有多片下，按loadlock空槽数，去上料

						for (int i = 0; i < lkmaps.size(); i++)
						{
							if (lkmaps[i] == Cassette::Empty) //空片的槽号
							{
								count++; //<=2
							}
						}
						//count = lkmaps.size(); 
					}
					else
					{
						logFailed(lk->getName().c_str(),"当前是未知的传片模式!");
					}

					//要区分LLA,LLB 分别给LLA,LLB 分配task,只改变选中的料
					//getEfemUnkownStatusLLATasks

					if (elp != nullptr && lk != nullptr)
					{
						if (lk->getName() == "LLA") {
							efemAllocationLLAInitTasks = taskManager.getEfemUnkownStatusLLATasks();
							if(efemAllocationLLAInitTasks.size() > 0)
							{
								for (int i = 0; i < count; i++)
								{
									if (lkmaps[i] == Cassette::Empty) //空片的槽号
									{
										taskManager.updateTaskStatus(efemAllocationLLAInitTasks[i].taskId, UnifiedWaferTask::TaskType::EFEM_TRANSFER,
											UnifiedWaferTask::Status::QUEUED);
									}
								}
							}
						}
						else if(lk->getName() == "LLB")
						{
							efemAllocationLLBInitTasks = taskManager.getEfemUnkownStatusLLBTasks();

							if(efemAllocationLLBInitTasks.size() > 0)
							{
								for (int i = 0; i < count; i++)
								{
									if (lkmaps[i] == Cassette::Empty) //空片的槽号
									{
										taskManager.updateTaskStatus(efemAllocationLLBInitTasks[i].taskId, UnifiedWaferTask::TaskType::EFEM_TRANSFER,
											UnifiedWaferTask::Status::QUEUED);
									}
								}
							}
						}
						else
						{
							logFailedExcuteCommandHasError(elp->getName(), "lk Name is error", efem_process_name, efem_auto_step);
							Sleep(200);
						}
					}
					efem_auto_step = 110;
				}
				break;
				
				case 110:
				{
					logWarn("EFEM", "efem_auto_step:%d", efem_auto_step);
					efemPendingTasks = taskManager.getEfemPendingTasks();
					
					if (efemPendingTasks.size() == 1)
					{
						if (ui->simulation_cbx->checkState() == Qt::CheckState::Checked)
						{
							efem_auto_step = 3000;
						}
						else
						{
							efem_auto_step = 115;//单取LP单放LK
						}				
					}
					else if (efemPendingTasks.size() >= 2)
					{
						if (ui->simulation_cbx->checkState() == Qt::CheckState::Checked)
						{
							efem_auto_step = 3001;
						}
						else
						{
							efem_auto_step = 150;//双取LP双放LK
						}
					}
					else
					{//上料完成
						//上面对task设置状态，数量不是1就是2,那么只要上料动作完成，efemPendingTasks数量只会是0
						//不管上进下出模式，还是双上双下，这里 efemPendingTasks.count == 0 
						if (elp == nullptr)
						{
							logFailedExcuteCommandHasError(elp->getName(), "elp  is nullptr", efem_process_name, efem_auto_step);
						}

						if (elp && elp->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
							// 2026-1-09  下料完成步骤去执行关lp！！
#if 0

							if (ui->simulation_cbx->checkState() == Qt::CheckState::Checked)
							{
								//tool_allow_get_wafer = false; //LP上料完成						
								if (current_loadlock == "LLA") {
									logWarn("cycle", "step:110,给TOOL上料 LLA unLock  thread...");
									efem_robot_mutex.unlock();

									tool_allow_get_wafer_LLA = false;
								}
								else if (current_loadlock == "LLB") {
									logWarn("cycle", "step:110,给TOOL上料 LLB unLock  thread...");
									efem_robot_mutex.unlock();
									tool_allow_get_wafer_LLB = false;
								}
								efem_auto_step = 10; //跳转到开始步骤
							}
							else
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

#endif // 0
							// 清除当前处理的LoadLock请求标志
							if (current_loadlock == "LLA") {
								logWarn("EFEM", "step:110,给TOOL上料 LLA unLock  thread...");
								efem_robot_mutex.unlock();
								tool_allow_get_wafer_LLA = false;
							}
							else if (current_loadlock == "LLB") {
								efem_robot_mutex.unlock();
								logWarn("EFEM", "step:110,给TOOL上料 LLB unLock  thread...");
								tool_allow_get_wafer_LLB = false;
							}
							efem_auto_step = 10; //跳转到开始步骤
							
						}
					}
				}
				break;
				#pragma region 单取LP单放LK
				case 115:
				{
					logWarn("EFEM", "efem_auto_step:%d", efem_auto_step);
					if (elp == nullptr)
					{
						logFailedExcuteCommandHasError(elp->getName(), "elp  is nullptr", efem_process_name, efem_auto_step);
					}
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
							//Sleep(200);
							efem_auto_step = 131;
						}

					}
					else {
						logFailedNotNormal(lk1->getName(), efem_process_name, efem_auto_step);
					}
				}
				break;
				case 131:
				{
					logInform(elp->getName().c_str(), "放晶圆到寻边器,step:%d", efem_auto_step);

					auto cassManager = ealigner->getKernel()->getKernelModule<FortrendCassetteManager>();
					auto station_cass = cassManager->getCassette(ealigner.get());
					if (ealigner == nullptr)
					{
						ealigner = kernel->getKernelModule<EFEMAlignerSubsystem>("EALIGNER");
					}
					if (ewtr && ewtr->getState() == IKernelSubSystem::State::SUB_NORMAL && ealigner->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						if (station_cass->getMapping(1) == Cassette::Mapping::Empty)
						{
							auto cmd2 = ewtr->createPutCommand(ealigner, 1, 1);
							ewtr->startCommand(cmd2);
							cmd2->wait();
							if (cmd2->hasError())
							{
								logFailedExcuteCommandHasError(ewtr->getName(), "放晶圆到寻边器", efem_process_name, efem_auto_step);
							}
							else
							{
								efem_auto_step = 132; //跳转到132
							}
						}
						else
						{
							Sleep(200);
						}
					}
					else
					{
						logFailedNotNormal(ewtr->getName(), efem_process_name, efem_auto_step);
					}
				}
				break;
				//加寻边并读码
				case 132:
				{
					logInform(elp->getName().c_str(), "晶圆寻边,step:%d", efem_auto_step);
					auto cassManager = ealigner->getKernel()->getKernelModule<FortrendCassetteManager>();
					auto station_cass = cassManager->getCassette(ealigner.get());

					if(ealigner->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						if (station_cass->getMapping(1) == Cassette::Mapping::Present)
						{
							auto alignerCmd = ealigner->createAlignCommand();
							ealigner->startCommand(alignerCmd);
							alignerCmd->wait();
							if (alignerCmd->hasError())
							{
								logFailedExcuteCommandHasError(ealigner->getName(), "晶圆寻边", efem_process_name, efem_auto_step);
							}
							else {
								Sleep(200);
								efem_auto_step = 133;
							}
						}
						else {
							logFailedNotNormal(ealigner->getName(), efem_process_name, efem_auto_step);
						}
					}
				}
				break;
				case 133:
				{
					logInform(elp->getName().c_str(), "取寻边器晶圆,step:%d", efem_auto_step);
					if (elp && elp->getState() == IKernelSubSystem::State::SUB_NORMAL && ealigner->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto cmd = ewtr->createGetCommand(ealigner, 1, 1);//1:1手，2：2手
						ewtr->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(ewtr->getName(), "取寻边器晶圆", efem_process_name, efem_auto_step);
						}
						else
						{
							efem_auto_step = 135;
						}
					}
					else {
						logFailedNotNormal(elp->getName(), efem_process_name, efem_auto_step);
					}
				}
				break;
				case 135://LP1上料
				{
					logInform(lk->getName().c_str(), "放LL晶圆,step:%d", efem_auto_step);

					if (ewtr && ewtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto cmd2 = ewtr->createPutCommand(lk, 1, efemPendingTasks[0].targetFeedingSlot);
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
					logWarn("EFEM", " 双取LP双放LK efem_auto_step:%d", efem_auto_step);
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
					logWarn("EFEM", " efem_auto_step:%d", efem_auto_step);
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
					logWarn("EFEM", " efem_auto_step:%d", efem_auto_step);
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
					logWarn("EFEM", " efem_auto_step:%d", efem_auto_step);
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
							//efem_auto_step = 1540;
							
							//2026-1-23 跳过寻边
							efem_auto_step = 155;
						}
					}
					else {
						logFailedNotNormal(elp->getName(), efem_process_name, efem_auto_step);
					}

				}
				break;
				//加寻边
				case 1540:
				{
					logInform(elp->getName().c_str(), "放晶圆到寻边器,step:%d", efem_auto_step);
					if (ealigner == nullptr)
					{
						ealigner = kernel->getKernelModule<EFEMAlignerSubsystem>("EALIGNER");
					}
					if (ewtr && ewtr->getState() == IKernelSubSystem::State::SUB_NORMAL && ealigner->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{

						auto maptCmd = ealigner->createGetMapCommand();
						ealigner->startCommand(maptCmd);
						maptCmd->wait();

						if (!maptCmd->hasError())
						{
							if (!ealigner->getPresentWafer())
							{
								auto cmd2 = ewtr->createPutCommand(ealigner, 1, 1);//A手放
								ewtr->startCommand(cmd2);
								cmd2->wait();
								if (cmd2->hasError())
								{
									logFailedExcuteCommandHasError(ewtr->getName(), "A手放晶圆到寻边器", efem_process_name, efem_auto_step);
								}
								else
								{
									efem_auto_step = 1541;
								}
							}
							else
							{
								Sleep(200);
							}
						}
						else
						{
							logFailedNotNormal(ealigner->getName(), efem_process_name, efem_auto_step);
						}
					}
					else
					{
						logFailedNotNormal(ealigner->getName(), efem_process_name, efem_auto_step);
					}
				}
				break;
				case 1541:
				{
					//寻边
					if (elp == nullptr) {
						logError("EFEM", "step:%d, elp指针为空，重新获取失败.", efem_auto_step);
						break;
					}

					// 确保ealigner指针不为空
					if (ealigner == nullptr) {
						ealigner = kernel->getKernelModule<EFEMAlignerSubsystem>("EALIGNER");
					}

					logInform(elp->getName().c_str(), "晶圆寻边,step:%d", efem_auto_step);

					if (ealigner && ealigner->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						if (ealigner->getPresentWafer())
						{
							auto alignerCmd = ealigner->createAlignCommand();
							ealigner->startCommand(alignerCmd);
							alignerCmd->wait();
							if (alignerCmd->hasError())
							{
								logFailedExcuteCommandHasError(ealigner->getName(), "晶圆寻边", efem_process_name, efem_auto_step);
							}
							else {
								Sleep(200);
								efem_auto_step = 1542;
							}
						}
						else {
							
							logFailedNotNormal(ealigner->getName(), efem_process_name, efem_auto_step);
						}
		
					}
					else
					{
						logFailedNotNormal(ealigner->getName(), efem_process_name, efem_auto_step);
					}
				}
				break;
				case 1542:
				{
					logInform(elp->getName().c_str(), "取寻边器晶圆,step:%d", efem_auto_step);
					if (elp && elp->getState() == IKernelSubSystem::State::SUB_NORMAL && ealigner->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto cmd = ewtr->createGetCommand(ealigner, 1, 1);//1:1手，2：2手
						ewtr->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(ewtr->getName(), "取寻边器晶圆", efem_process_name, efem_auto_step);
						}
						else
						{
							efem_auto_step = 1544;
						}
					}
					else {
						logFailedNotNormal(elp->getName(), efem_process_name, efem_auto_step);
					}
				}
				break;
				case 1544:
				{
				
					logInform(elp->getName().c_str(), "放晶圆到寻边器,step:%d", efem_auto_step);
					if (ealigner == nullptr)
					{
						ealigner = kernel->getKernelModule<EFEMAlignerSubsystem>("EALIGNER");
					}
					if (ewtr && ewtr->getState() == IKernelSubSystem::State::SUB_NORMAL && ealigner->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto maptCmd = ealigner->createGetMapCommand();
						ealigner->startCommand(maptCmd);
						maptCmd->wait();
						if (!maptCmd->hasError())
						{

							if (!ealigner->getPresentWafer())
							{
								auto cmd2 = ewtr->createPutCommand(ealigner, 2, 1);//B手放
								ewtr->startCommand(cmd2);
								cmd2->wait();
								if (cmd2->hasError())
								{
									logFailedExcuteCommandHasError(ewtr->getName(), "B手放晶圆到寻边器", efem_process_name, efem_auto_step);
								}
								else
								{
									efem_auto_step = 1545;
								}
							}
							else
							{
								Sleep(200);
							}
						}
						else
						{

						}
					}
					else
					{
						logFailedNotNormal(ewtr->getName(), efem_process_name, efem_auto_step);
					}
				}
				break;
				case 1545:
				{
					logInform(elp->getName().c_str(), "晶圆寻边,step:%d", efem_auto_step);

					if (ealigner->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto maptCmd = ealigner->createGetMapCommand();
						ealigner->startCommand(maptCmd);
						maptCmd->wait();
						if (!maptCmd->hasError())
						{
							if (ealigner->getPresentWafer())
							{
								auto alignerCmd = ealigner->createAlignCommand();
								ealigner->startCommand(alignerCmd);
								alignerCmd->wait();
								if (alignerCmd->hasError())
								{
									logFailedExcuteCommandHasError(ealigner->getName(), "晶圆寻边", efem_process_name, efem_auto_step);
								}
								else {
									//Sleep(200);
									efem_auto_step = 1546;
								}
							}
							else {
								logFailedNotNormal(ealigner->getName(), efem_process_name, efem_auto_step);
							}
						}
						else
						{

						}
					}
				}
				break;
				case 1546:
				{
					logInform(elp->getName().c_str(), "取寻边器晶圆,step:%d", efem_auto_step);
					if (elp && elp->getState() == IKernelSubSystem::State::SUB_NORMAL && ealigner->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto cmd = ewtr->createGetCommand(ealigner, 2, 1);//1:1手，2：2手
						ewtr->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(ewtr->getName(), "取寻边器晶圆", efem_process_name, efem_auto_step);
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
					logWarn("EFEM", " efem_auto_step:%d", efem_auto_step);
					//2026-1-23 双上料必须先缓存两个任务的信息，因为case 157更新状态后数组会变化
					if (efemPendingTasks.size() < 2)
					{
						logError("EFEM", "efem_auto_step:155, efemPendingTasks size < 2, size=%d", efemPendingTasks.size());
						efem_auto_step = 110;
						break;
					}
					// 缓存两个任务的完整信息，避免case 157更新状态后数组变化导致访问错误
					cached_task_first = efemPendingTasks.at(0);  // 第一片料信息
					cached_task_second = efemPendingTasks.at(1); // 第二片料信息
								
					std::shared_ptr<FortrendLoadLockSubsystem>get_lk = cached_task_first.target == UnifiedWaferTask::Location::LLA ? lk1 : lk2;
				
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
							//Sleep(2000);
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
					logWarn("EFEM", " efem_auto_step:%d", efem_auto_step);
					// 使用缓存的第二个任务信息
					std::shared_ptr<FortrendLoadLockSubsystem>get_lk = cached_task_second.target == UnifiedWaferTask::Location::LLA ? lk1 : lk2;
				
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
							//Sleep(2000);
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
					logWarn("EFEM", " efem_auto_step:%d", efem_auto_step);
					// 使用缓存的第一个任务信息：放第一片料
					std::shared_ptr<FortrendLoadLockSubsystem>get_lk = cached_task_first.target == UnifiedWaferTask::Location::LLA ? lk1 : lk2;
					if (ewtr && ewtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto cmd2 = ewtr->createPutCommand(get_lk, 1, cached_task_first.targetFeedingSlot);
						ewtr->startCommand(cmd2);
						cmd2->wait();
						if (cmd2->hasError())
						{
							logFailedExcuteCommandHasError(ewtr->getName(), "A手臂放LL晶圆", efem_process_name, efem_auto_step);
						}
						else {//放料完成
							logInform("EFEM", "A手臂放料：taskId=%d, target=%s, slot=%d", cached_task_first.taskId, 
								UnifiedWaferTask::locationToString(cached_task_first.target).c_str(), cached_task_first.targetFeedingSlot);
							efem_auto_step = 158;
							taskManager.updateTaskStatus(cached_task_first.taskId, UnifiedWaferTask::TaskType::EFEM_TRANSFER, UnifiedWaferTask::Status::COMPLETED);
							taskManager.updateTaskStatus(cached_task_first.taskId, UnifiedWaferTask::TaskType::LOADLOCK_TRANSFER, UnifiedWaferTask::Status::QUEUED);
						}
					}
					else {
						logFailedNotNormal(ewtr->getName(), efem_process_name, efem_auto_step);
					}
				}
				break;
				case 158:
				{
					logWarn("EFEM", " efem_auto_step:%d", efem_auto_step);
					// 使用缓存的第二个任务信息：放第二片料（关键修复！）
					std::shared_ptr<FortrendLoadLockSubsystem>get_lk = cached_task_second.target == UnifiedWaferTask::Location::LLA ? lk1 : lk2;
					if (ewtr && ewtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto cmd2 = ewtr->createPutCommand(get_lk, 2, cached_task_second.targetFeedingSlot);
						ewtr->startCommand(cmd2);
						cmd2->wait();
						if (cmd2->hasError())
						{
							logFailedExcuteCommandHasError(ewtr->getName(), "B手臂放LL晶圆", efem_process_name, efem_auto_step);
						}
						else {//放料完成
							logInform("EFEM", "B手臂放料：taskId=%d, target=%s, slot=%d", cached_task_second.taskId, 
								UnifiedWaferTask::locationToString(cached_task_second.target).c_str(), cached_task_second.targetFeedingSlot);
							taskManager.updateTaskStatus(cached_task_second.taskId, UnifiedWaferTask::TaskType::EFEM_TRANSFER, UnifiedWaferTask::Status::COMPLETED);
							taskManager.updateTaskStatus(cached_task_second.taskId, UnifiedWaferTask::TaskType::LOADLOCK_TRANSFER, UnifiedWaferTask::Status::QUEUED);
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
					logWarn("EFEM", "step:200,current_loadlock:%s 下料 Lock thread...", current_loadlock.c_str());
					if (!efem_robot_mutex.is_locked())
					{
						efem_robot_mutex.lock();
					}
					
					//efemReturnPendingTasks 这个数组数据来源是 LL腔  8->3

					loadLockAReturnCompletedTasks = taskManager.getLoadLockReturnCompletedTasks("LLA");
					loadLockBReturnCompletedTasks = taskManager.getLoadLockReturnCompletedTasks("LLB");
					
					if (current_loadlock == "LLA")
					{
						for (auto& task : loadLockAReturnCompletedTasks)
						{
							taskManager.updateTaskStatus(task.taskId, UnifiedWaferTask::EFEM_RETURN, UnifiedWaferTask::QUEUED);
						}
					}
					else
					{
						for (auto& task : loadLockBReturnCompletedTasks)
						{
							taskManager.updateTaskStatus(task.taskId, UnifiedWaferTask::EFEM_RETURN, UnifiedWaferTask::QUEUED);
						}
					}

					UpdateEfemSubTransferDatas();

					// 防止数组越界
					if (efemReturnPendingTasks.size() < 1)
					{
						logError("EFEM", "step:200, efemReturnPendingTasks size < 1, size=%d", efemReturnPendingTasks.size());
						efem_auto_step = 10;
						break;
					}
					elp = efemReturnPendingTasks.at(0).source == UnifiedWaferTask::Location::LP1 ? elp1 : elp2;//下料，实际源头和目标互换
					lk = efemReturnPendingTasks.at(0).target == UnifiedWaferTask::Location::LLA ? lk1 : lk2;

					if (elp == nullptr || lk == nullptr)
					{
						logError("EFEM", "elp or lk is empty");

						logFailedExcuteCommandHasError(elp->getName(), "elp or lk is empty", efem_process_name, efem_auto_step);
						efem_auto_step = 10;
						break;
					}

					if (ui->simulation_cbx->checkState() == Qt::CheckState::Checked)
					{
						logInform(elp->getName().c_str(), "step:200,模拟EFEM给TOOL下料程序....");
						efem_auto_step = 201;
					}
					else
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
				}
				break;
				case 201:
				{
					logWarn("EFEM", "efem_auto_step:%d", efem_auto_step);
					UpdateEfemSubTransferDatas();

					if (efemReturnPendingTasks.size() == 1)
					{
						if (ui->simulation_cbx->checkState() == Qt::CheckState::Checked)
						{
							efem_auto_step = 4000;
						}
						else
						{
							efem_auto_step = 240; //单取LK单放LP
						}
						
					}
					else if (efemReturnPendingTasks.size() >= 2)
					{
						if (ui->simulation_cbx->checkState() == Qt::CheckState::Checked)
						{
							efem_auto_step = 4001;
						}
						else
						{
							efem_auto_step = 250; //双取LK双放LP
						}
					}
					//else if(efemReturnPendingTasks.size() == 0 && efemReturnCompletedTasks.size() == originTaskSize)
					else
					{
						UpdateEfemSubTransferDatas();

						if (ui->simulation_cbx->checkState() == Qt::CheckState::Checked)
						{
							//tool_allow_put_wafer = false;//下料到LP完成
							if (current_loadlock == "LLA") {
								efem_robot_mutex.unlock();
								logWarn("EFEM", "step:201,给LLA下料,unLock thread...");
								tool_allow_put_wafer_LLA = false;
							}
							else if (current_loadlock == "LLB") {
								efem_robot_mutex.unlock();
								logWarn("EFEM", "step:201,给LLB下料,unLock thread...");
								tool_allow_put_wafer_LLB = false;
								
							}
							else
							{
								Sleep(100);
							}
							efem_auto_step = 10;
						}
						else
						{
							//解锁提前，避免
							if (current_loadlock == "LLA") {
								logWarn("EFEM", "step:201,给LLA下料 LLA unLock thread...");
								efem_robot_mutex.unlock();
								tool_allow_put_wafer_LLA = false;
							}
							else if (current_loadlock == "LLB") {
								logWarn("EFEM", "step:201,给LLB下料 LLB unLock thread...");
								efem_robot_mutex.unlock();
								tool_allow_put_wafer_LLB = false;
							}

							if (efemReturnPendingTasks.size() == 0 && efemReturnCompletedTasks.size() == originTaskSize)
							{
								//整个lp下料完成
								logWarn(elp1->getName().c_str(), "整个lp下料完成.");

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

								//这里不解锁，防止干扰下次循环，上料时的上锁，因为在更新
								if (!elp2->hasDoorOpend() && !elp1->hasDoorOpend())
								{
									//if (current_loadlock == "LLA") {
									//	logWarn("EFEM", "step:201,给LLA下料 LLA unLock thread...");
									//	efem_robot_mutex.unlock();
									//	tool_allow_put_wafer_LLA = false;
									//}
									//else if (current_loadlock == "LLB") {
									//	logWarn("EFEM", "step:201,给LLB下料 LLB unLock thread...");
									//	efem_robot_mutex.unlock();
									//	tool_allow_put_wafer_LLB = false;
									//}
									//else
									//{
									//	Sleep(100);
									//}
									efem_auto_step = 10;
								}

							}
							else
							{
								efem_auto_step = 10;
							}
							
							
						}
					}
				}
				break;
#pragma region 单取LK单放LP
				case 240:
				{
					logWarn("EFEM", "efem_auto_step:%d", efem_auto_step);
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
				case 241:
				{
					logWarn("EFEM", "efem_auto_step:%d", efem_auto_step);
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
								Sleep(200);
							}
						}
						else {
							efem_auto_step = 243;
							Sleep(200);
						}
					}
					else {
						logFailedNotNormal(lk->getName(), efem_process_name, efem_auto_step);
					}

				}
				break;
				case 243:
				{
					logWarn("EFEM", "efem_auto_step:%d", efem_auto_step);
					if (ewtr && ewtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto cmd1 = ewtr->createGetCommand(lk, 1, efemReturnPendingTasks.at(0).targetBlankingSlot);
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
					logWarn("EFEM", "efem_auto_step:%d", efem_auto_step);
					UpdateEfemSubTransferDatas();
					if (ewtr && ewtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						if (efemReturnPendingTasks.size() == 0)
						{
							logFailedExcuteCommandHasError(ewtr->getName(), "efemReturnPendingTasks size empty", efem_process_name, efem_auto_step);
						}
						auto cmd2 = ewtr->createPutCommand(elp, 1, efemReturnPendingTasks[0].sourceSlot);
						ewtr->startCommand(cmd2);
						cmd2->wait();
						if (cmd2->hasError())
						{
							logFailedExcuteCommandHasError(ewtr->getName(), "放LP晶圆", efem_process_name, efem_auto_step);
							efem_auto_step = 201;
						}
						else
						{
							UpdateEfemSubTransferDatas();
							taskManager.updateTaskStatus(efemReturnPendingTasks[0].taskId, UnifiedWaferTask::TaskType::EFEM_RETURN, UnifiedWaferTask::Status::COMPLETED);

							//下料到lp，就置位
							if (current_loadlock == "LLA") {
								efem_robot_mutex.unlock();
								logWarn("EFEM", "step:244,给LLA下料 LLA unLock thread...");
								tool_allow_put_wafer_LLA = false;
							}
							else if (current_loadlock == "LLB") {
								efem_robot_mutex.unlock();
								logWarn("EFEM", "step:244,给LLB下料 LLB unLock thread...");
								tool_allow_put_wafer_LLB = false;
							}

							UpdateEfemSubTransferDatas();//2025-8-21  获取最新数据

							//考虑多片情况，返回任务数量是初始配置的数量，且无待下料任务
							if (efemUnkownStatusTasks.size() == 0 &&  efemReturnPendingTasks.size() == 0 && efemReturnCompletedTasks.size() == originTaskSize)
							{
								if(efemReturnCompletedTasks.size() > 0 )
								{
									if (efemReturnCompletedTasks.at(0).source == UnifiedWaferTask::Location::LP1)
									{
										logWarn(ewtr->getName().c_str(), "cycle end efemReturnCompletedTasks=%d", efemReturnCompletedTasks.size());

										current_lp_cycle = is_lp1_cycle = true;
										logWarn(ewtr->getName().c_str(), "is_lp1_cycle true");
									}
									else if (efemReturnCompletedTasks.at(0).source == UnifiedWaferTask::Location::LP2)
									{
										logWarn(ewtr->getName().c_str(), "cycle end efemReturnCompletedTasks=%d", efemReturnCompletedTasks.size());

										current_lp_cycle = is_lp2_cycle = true;
										logWarn(ewtr->getName().c_str(), "is_lp2_cycle true");
									}
								}
								else
								{
									logError(ewtr->getName().c_str(), "cycle end efemReturnCompletedTasks size is zero!");
									efem_auto_step = 201;
								}
							
								efem_auto_step = 201;
							}
							
							else
							{
								efem_auto_step = 201;
							}

							//要加！！！！
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
					logWarn("EFEM", "efem_auto_step:%d", efem_auto_step);
					UpdateEfemSubTransferDatas();
					// 防止数组越界：双取LK双放LP需要至少两个任务
					if (efemReturnPendingTasks.size() < 2)
					{
						logError("EFEM", "efem_auto_step:250, efemReturnPendingTasks size < 2, size=%d", efemReturnPendingTasks.size());
						efem_auto_step = 10;
						break;
					}
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
					logWarn("EFEM", "efem_auto_step:%d", efem_auto_step);
					// 防止数组越界
					if (efemReturnPendingTasks.size() < 2)
					{
						logError("EFEM", "efem_auto_step:251, efemReturnPendingTasks size < 2, size=%d", efemReturnPendingTasks.size());
						efem_auto_step = 10;
						break;
					}
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

								Sleep(200);
							}
						}
						else {
							lkopen = true;

							Sleep(200);
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
								Sleep(200);
							}
						}
						else {
							lk2open = true;
							Sleep(200);
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
					logWarn("EFEM", "efem_auto_step:%d", efem_auto_step);
					std::shared_ptr<FortrendLoadLockSubsystem> lk1 = efemReturnPendingTasks.at(0).target == UnifiedWaferTask::Location::LLA ? lk1 : lk2;

					if (ewtr && ewtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto cmd1 = ewtr->createGetCommand(lk1, 1, efemReturnPendingTasks[0].targetBlankingSlot);
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
					logWarn("EFEM", "efem_auto_step:%d", efem_auto_step);
					// 防止数组越界
					if (efemReturnPendingTasks.size() < 2)
					{
						logError("EFEM", "efem_auto_step:254, efemReturnPendingTasks size < 2, size=%d", efemReturnPendingTasks.size());
						efem_auto_step = 10;
						break;
					}
					std::shared_ptr<FortrendLoadLockSubsystem> lk2 = efemReturnPendingTasks.at(1).target == UnifiedWaferTask::Location::LLA ? lk1 : lk2;

					if (ewtr && ewtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto cmd1 = ewtr->createGetCommand(lk2, 2, efemReturnPendingTasks[1].targetBlankingSlot);
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
					logWarn("EFEM", "efem_auto_step:%d", efem_auto_step);
					// 防止数组越界
					if (efemReturnPendingTasks.size() < 2)
					{
						logError("EFEM", "efem_auto_step:256, efemReturnPendingTasks size < 2, size=%d", efemReturnPendingTasks.size());
						efem_auto_step = 10;
						break;
					}
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
					logWarn("EFEM", "efem_auto_step:%d", efem_auto_step);
					UpdateEfemSubTransferDatas();
					// 防止数组越界
					if (efemReturnPendingTasks.size() < 2)
					{
						logError("EFEM", "efem_auto_step:257, efemReturnPendingTasks size < 2, size=%d", efemReturnPendingTasks.size());
						efem_auto_step = 201;
						break;
					}
					taskManager.updateTaskStatus(efemReturnPendingTasks.at(0).taskId, UnifiedWaferTask::TaskType::EFEM_RETURN, UnifiedWaferTask::Status::COMPLETED);
					taskManager.updateTaskStatus(efemReturnPendingTasks.at(1).taskId, UnifiedWaferTask::TaskType::EFEM_RETURN, UnifiedWaferTask::Status::COMPLETED);

					//下料到lp，就置位
					if (current_loadlock == "LLA") {
						efem_robot_mutex.unlock();
						tool_allow_put_wafer_LLA = false;
					}
					else if (current_loadlock == "LLB") {
						efem_robot_mutex.unlock();
						tool_allow_put_wafer_LLB = false;
					}

					//多片情况下，系统判断开启LP循环的条件是： 整个LP的所有晶圆都下料完成
					//怎么区分是否是lp1传输任务，条件：efemReturnPendingTasks.at(0).source == UnifiedWaferTask::Location::LP1
					// 原先条件： efemUnkownStatusTasks.size() == 0 && efemPendingTasks.size() ==0 && efemReturnPendingTasks.size() == 2 不对！！！


					UpdateEfemSubTransferDatas();//2025-8-21  获取最新数据

					if (efemUnkownStatusTasks.size() == 0 && efemReturnPendingTasks.size() == 0 && efemReturnCompletedTasks.size() == originTaskSize)
					{
						if (efemReturnCompletedTasks.size() >= 2)
						{

							//来自同一个lp1
							if (efemReturnCompletedTasks.at(0).source == UnifiedWaferTask::Location::LP1 && efemReturnCompletedTasks.at(0).source == efemReturnCompletedTasks.at(1).source)
							{
								logWarn(ewtr->getName().c_str(), "cycle end efemUnkownStatusTasks=%d efemReturnCompletedTasks=%d",
									efemUnkownStatusTasks.size(), efemReturnCompletedTasks.size());
								current_lp_cycle = is_lp1_cycle = true;
								logWarn(ewtr->getName().c_str(), "is_lp1_cycle true");
							}
							//来自同一个lp2
							else if (efemReturnCompletedTasks.at(0).source == UnifiedWaferTask::Location::LP2 && efemReturnCompletedTasks.at(0).source == efemReturnCompletedTasks.at(1).source)
							{
								logWarn(ewtr->getName().c_str(), "cycle end efemUnkownStatusTasks=%d efemReturnCompletedTasks=%d",
									efemUnkownStatusTasks.size(), efemReturnCompletedTasks.size());
								current_lp_cycle = is_lp2_cycle = true;
								logWarn(ewtr->getName().c_str(), "is_lp2_cycle true");
							}

							//来自不同的lp，LP1
							else if (efemReturnCompletedTasks.at(0).source == UnifiedWaferTask::Location::LP1 && efemReturnCompletedTasks.at(0).source != efemReturnCompletedTasks.at(1).source)
							{
								logWarn(ewtr->getName().c_str(), "cycle end efemUnkownStatusTasks=%d efemReturnCompletedTasks=%d",
									efemUnkownStatusTasks.size(), efemReturnCompletedTasks.size());
								current_lp_cycle = is_lp1_cycle = true;
								logWarn(ewtr->getName().c_str(), "is_lp1_cycle true");
							}
							//来自不同的lp,LP2
							else if (efemReturnCompletedTasks.at(0).source == UnifiedWaferTask::Location::LP2 && efemReturnCompletedTasks.at(0).source != efemReturnCompletedTasks.at(1).source)
							{
								logWarn(ewtr->getName().c_str(), "cycle end efemUnkownStatusTasks=%d efemReturnCompletedTasks=%d",
									efemUnkownStatusTasks.size(), efemReturnCompletedTasks.size());
								current_lp_cycle = is_lp2_cycle = true;
								logWarn(ewtr->getName().c_str(), "is_lp2_cycle true");
							}
						}
						else
						{
							logError(ewtr->getName().c_str(), "cycle end efemReturnCompletedTasks size is zero!");
							efem_auto_step = 201;
						}
					}

					efem_auto_step = 201;
				}
				break;

#pragma endregion
#pragma endregion

				#pragma region 模拟EFEM给TOOLD单上料
				case 3000:
				{
					taskManager.updateTaskStatus(efemPendingTasks[0].taskId, UnifiedWaferTask::TaskType::EFEM_TRANSFER, UnifiedWaferTask::Status::COMPLETED);
					taskManager.updateTaskStatus(efemPendingTasks[0].taskId, UnifiedWaferTask::TaskType::LOADLOCK_TRANSFER, UnifiedWaferTask::Status::QUEUED);

					Sleep(500);
					logWarn("SimCycle", "EFEM给LL单上料完成.");
					efem_auto_step = 110; //跳转到110

				}
				break;

				#pragma endregion

				#pragma region 模拟EFEM给TOOLD双上料
				case 3001:
				{
					taskManager.updateTaskStatus(efemPendingTasks.at(0).taskId, UnifiedWaferTask::TaskType::EFEM_TRANSFER, UnifiedWaferTask::Status::COMPLETED);
					taskManager.updateTaskStatus(efemPendingTasks.at(0).taskId, UnifiedWaferTask::TaskType::LOADLOCK_TRANSFER, UnifiedWaferTask::Status::QUEUED);
					taskManager.updateTaskStatus(efemPendingTasks.at(1).taskId, UnifiedWaferTask::TaskType::EFEM_TRANSFER, UnifiedWaferTask::Status::COMPLETED);
					taskManager.updateTaskStatus(efemPendingTasks.at(1).taskId, UnifiedWaferTask::TaskType::LOADLOCK_TRANSFER, UnifiedWaferTask::Status::QUEUED);
					Sleep(500);
					logWarn("SimCycle", "EFEM给LL双上料完成.");
					efem_auto_step = 110;
				}
				break;

				#pragma endregion

				#pragma region 模拟EFEM给TOOL单下料
				case 4000:
				{
					UpdateEfemSubTransferDatas();

					//还剩下一片wafer待放回lp
					if (efemUnkownStatusTasks.size() == 0 && efemReturnPendingTasks.size() == 1)
					{
						if (efemReturnPendingTasks.at(0).source == UnifiedWaferTask::Location::LP1)
						{
							logWarn(ewtr->getName().c_str(), "cycle end efemReturnPendingTasks=%d",efemReturnPendingTasks.size());

							current_lp_cycle = is_lp1_cycle = true;
						}
						else if (efemReturnPendingTasks.at(0).source == UnifiedWaferTask::Location::LP2)
						{
							logWarn(ewtr->getName().c_str(), "cycle end efemReturnPendingTasks=%d",efemReturnPendingTasks.size());

							current_lp_cycle = is_lp2_cycle = true;
						}
					}
					taskManager.updateTaskStatus(efemReturnPendingTasks[0].taskId, UnifiedWaferTask::TaskType::EFEM_RETURN, UnifiedWaferTask::Status::COMPLETED);

					efem_auto_step = 201;
					logWarn("SimCycle", "EFEM给LL单下料完成 .");
				}
				break;
				#pragma endregion

				#pragma region 模拟EFEM给TOOL双下料
				case 4001:
				{
					efem_auto_step = 257;
					logWarn("SimCycle", "EFEM给LL双下料完成 .");
				}
				break;

				#pragma endregion
				default:
					break;
				}
			

				efem_step_once_finished = true;
				Sleep(10);	
			}
	
		}
		catch (const std::exception& e) {

			logError("Cyclelog", "EFEM thread crashed:", e.what());
			qCritical() << "EFEM thread crashed:" << e.what();
		}
		catch (...) {
			logError("Cyclelog", "EFEM thread crashed: unknown exception");
			qCritical() << "EFEM thread crashed: unknown exception";
		}

	}
	/*
	* 大气/真空流程，分为取晶圆，放晶圆，下料到EFEM, 破真空，抽真空
	* 
	*/
	void QSlotTransferCycleVTMWidgetPrivate::executeLLATransfer()
	{
		try {

		if (ewtr ==nullptr|| lk1 == nullptr || lk2 == nullptr || wtr == nullptr)
		{
			ewtr = kernel->getKernelModule<EFEMWaferRobotSubsystem>("EWTR");
			lk1 = kernel->getKernelModule<FortrendLoadLockSubsystem>("LLA");
			lk2 = kernel->getKernelModule<FortrendLoadLockSubsystem>("LLB");
			wtr = kernel->getKernelModule<FortrendSunwayRobotSubsystem>("WTR");
		}

			while (!stopRequested)
			{

				Sleep(500);
				{
					std::unique_lock<std::mutex> lock(mtx);
					cv.wait(lock, [this] { return running.load() || stopRequested.load(); });
				}

				if (stopRequested) break;

				// 检查是否需要重置
				if (needReset_LLA.load()) {
					loadlock1_auto_step = 10;
					needReset_LLA = false;
					logWarn(lk1->getName().c_str(), "重置，跳过本次循环剩余部分.");
					continue; // 跳过本次循环剩余部分
				}

				loadlock1_step_once_finished = false;
				
				//onGetStep();
				switch (loadlock1_auto_step)
				{
				case 10:
				{
					originTaskSize = taskManager.getAllTasksSize();

					lp1TaskSize = taskManager.getTasksByLocation(UnifiedWaferTask::Location::LP1).size();
					lp2TaskSize = taskManager.getTasksByLocation(UnifiedWaferTask::Location::LP2).size();

					UpdateLLASubTransferDatas();

					bool isInterlock = isLoadingInterlock("LLA");
					if (isInterlock)
					{
						//logWarn(lk1->getName().c_str(), "有片下料没完成时情景，LLA上料互锁.");
					}

					//在LLA槽中，没有传输任务,说明没有上料需求，所有wafer都完成下料到LP中
					if (loadLockAPendingTasks.size() == 0 && current_lp_cycle)
					{
						logInform(lk1->getName().c_str(), "此时current_lp_cycle循环.");
						loadlock1_auto_step = 6000;
					}

					//优先
					if (!tool_allow_get_wafer_LLA && (loadLockAPendingTasks.size() > 0 || loadLockAReturnCompletedTasks.size() > 0) )
					{
						//有wafer，直接抽真空，走取放晶圆流程/下料流程
						loadlock1_auto_step = 400;
					}

					//需上料 
					else if ((!isInterlock) && (loadLockAPendingTasks.size() == 0 || loadLockAReturnCompletedTasks.size()== 0 ||  0 < efemUnkownStatusTasks.size() <= originTaskSize))
					{
						//无wafer,破真空，让efem上料, 此时lp中的wafer状态是unkown
						if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
							Sleep(200);
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
						logInform("Cycle", "step 20:大气cycle,跳过真空检测.");
						loadlock1_auto_step = 300;
					}
					else
					{	
						//排气压力达到设定值9W9   getExhaustVacuumValueReachesTheSetValue
						if (!lk1->getVacuumPressureGageState() == 1)
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
					tool_allow_get_wafer_LLA = true;
					logInform(lk1->getName().c_str(), "已发送上料请求.");
					loadlock1_auto_step = 302;
				}
				break;
				case 302:
				{
					if (!tool_allow_get_wafer_LLA)
					{	
						//EFEM上料完成
						loadlock1_auto_step = 350;
						loadlock1_put_cassette_finished = true; //放料到loadlock1完成
					}
					else {
						// 添加等待状态提示
						static int wait_count = 0;
						if ((wait_count++ % 20) == 0) {  // 每20次循环打印一次
							logInform(lk1->getName().c_str(), "等待EFEM上料中...");
						}
						Sleep(500);
					}
				}
				break;
				case 350:
				{
					if (loadlock1_put_cassette_finished && !tool_allow_get_wafer_LLA)//上料完成
					{
						//此时task status 改成LockPendingTasks 
						efemCompletedTasks = taskManager.getEfemCompletedTasks();

						auto efemToLLACompletedTasks = taskManager.getTasksByLocation(efemCompletedTasks,UnifiedWaferTask::Location::LLA);

						for (auto& task : efemToLLACompletedTasks)
						{
							taskManager.updateTaskStatus(task.taskId, UnifiedWaferTask::TaskType::LOADLOCK_TRANSFER, UnifiedWaferTask::Status::QUEUED); //TASKID:1 槽号是2
						}
						loadlock1_auto_step = 400;
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
						if (ui->enableAtmosphere->checkState() == Qt::CheckState::Checked)
						{
							logInform("Cycle", "step:400, 大气cycle，跳过真空检测.");
							loadlock1_auto_step = 800;
						}
						else
						{
							auto cmd = lk1->createCloseCassetteDoorCommand();
							lk1->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError())
							{
								logFailedExcuteCommandHasError(lk1->getName(), "关闭放晶圆盒门阀", loadlock1_process_name, loadlock1_auto_step);
							}
							else
							{

								loadlock1_auto_step = 410;
							}
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
						//LoadLock1有真空模式，未达到设定值，角阀未打开
						if (lk1->getVacuumEnable() && (!lk1->getVacuumValueReachesTheSetValue()))
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
						if (lk1->getVacuumValueUpperLimitReachesTheSetValue())//检测是否达到极限值
						{
							loadlock1_auto_step = 800;
						}
						else
						{
							loadlock1_auto_step = 500;//继续抽
						}
					}
					else
					{
						//LLA未达到真空设定值且在抽其他腔室
						if (lk1->getVacuumValueReachesTheSetValue() && loadlock1_get_vacuum && (loadlock2_get_vacuum || tm_get_vacuum))
						{
							logInform("Cycle", "step:510, lk1真空值已经达到设定值，真空规读取数值波动或者泵在抽其他腔室.");
							loadlock1_get_vacuum = false;
						}
						Sleep(500);
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
							logInform("Cycle","step:800,大气cycle，跳过真空检测.");
							loadlock1_auto_step = 810;
						}
						else
						{
							if (lk1->getVacuumEnable() && (!lk1->getVacuumValueReachesTheSetValue()))
							{
								loadlock1_auto_step = 500;
							}
							else
							{
								loadlock1_auto_step = 810;
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

#pragma region 判断是否取晶圆、放晶圆流程、出空casstte的流程
				case 900:
				{
					logInform("Cycle", "llA step 900");
					UpdateLLASubTransferDatas();

					if (loadLockAPendingTasks.size() > 0 || loadLockAReturnPendingTasks.size() > 0)
					{
						loadlock1_auto_step = 901;//取晶圆、放晶圆流程
							
					}
					else if (loadLockAReturnCompletedTasks.size() > 0) //要兼顾到：若LLa有一片待工艺片，还有一片待efem下料的片，怎么解决？
					{
						loadlock1_auto_step = 5000;//出空casstte的流程
					}
					else
					{
						Sleep(500);
					}
				}
				break;
#pragma endregion
				
				case 901:
				{
					if (ui->enableAtmosphere->checkState() == Qt::CheckState::Checked)
					{
						logInform("Cycle", "step: 901,大气cycle，跳过真空检测.");
						loadlock1_auto_step = 950;
					}
					else
					{
						if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
							if (lk1->getVacuumEnable() && (!lk1->getVacuumValueReachesTheSetValue()))
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
						//再检测，由于真空规读取数值波动，还会执行跳到901
						if (lk1->getVacuumValueUpperLimitReachesTheSetValue())
						{
							loadlock1_auto_step = 950;
						}
						else
						{
							loadlock1_auto_step = 901;
						}
					}
					else
					{
						if (lk1->getVacuumValueReachesTheSetValue() && loadlock1_get_vacuum && (loadlock2_get_vacuum || tm_get_vacuum))
						{
							logInform("Cycle", "step:920, lk1真空值已经达到，真空规读取数值波动或者泵在抽其他腔室.");
							loadlock1_get_vacuum = false;//真空值已经达到，泵在抽其他腔室
						}
						Sleep(100);
					}
				}
				break;
				case 950:
				{
					logInform("Cycle", "llA step 950");
					UpdateLLASubTransferDatas();
					if (loadLockAPendingTasks.size() > 0 && !abortCycle)
					{
						loadlock1_auto_step = 1000;//允许取晶圆流程

					}
					else if (loadLockAReturnPendingTasks.size() > 0 && !abortCycle) //2025/8/13 加!pm1_allow_get_put_wafer
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
					loadlock1_move_slot_index = loadLockAPendingTasks.at(0).targetFeedingSlot; //拿第一个task

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
						if (ui->simulation_cbx->checkState() == Qt::CheckState::Checked)
						{
							logInform(elp->getName().c_str(), "step:1010,模拟取晶圆流程程序....");
							efem_auto_step = 1052;
						}
						else
						{
							if (station_cass->getMapping(loadlock1_move_slot_index) == Cassette::Mapping::Present)
							{
								loadlock1_auto_step = 1040;

							}
							else
							{
								logFailed(lk1->getName(), Poco::format("%s 第%d槽不是正常片.", lk1->getName(), loadlock1_move_slot_index));
							}
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
						LLA_start_time = std::chrono::steady_clock::now();
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
						if (ui->enableAtmosphere->checkState() == Qt::CheckState::Checked)
						{
							logInform("cycle", "step:1050,大气模式,不关闭TM腔门!");
							loadlock1_auto_step = 1051;
						}
						else
						{
							if (lk1->getVacuumValueUpperLimitReachesTheSetValue() &&
								tm->getTMCavityVacuumValueUpperLimitReachesTheSetValue())
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
								loadlock1_auto_step = 1040;
							}

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
					if (wtr == nullptr)
					{
						wtr = kernel->getKernelModule<FortrendSunwayRobotSubsystem>("WTR");
					}
					if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL && lk1->hasDoorOpend())
					{
						//robot 用手臂A 从loadlock1 取wafer 
						//互锁条件，TM抽真空会触发关门动作
						if(lk1->getTMCavityDoorOpend())
						{
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
							logInform(lk1->getName().c_str(), "此时Tm腔真空波动，导致触发TM自动抽真空流程.");
							loadlock1_auto_step = 1050;
						}
					}
					else
					{
						logFailedNotNormal(wtr->getName(), loadlock1_process_name, loadlock1_auto_step);
					}
				}
				
				case 1052:
				{
					std::string pmName;
					if (loadLockAPendingTasks.size() > 0)
					{

						//真空机械手取料完成
						pmName = getSelectPmProcessName(loadLockAPendingTasks.at(0));
						logInform(lk1->getName().c_str(), "step:1052,pmName:%s", pmName.c_str());

						taskManager.updateTaskStatus(loadLockAPendingTasks.at(0).taskId, UnifiedWaferTask::TaskType::LOADLOCK_TRANSFER, UnifiedWaferTask::Status::COMPLETED);
						taskManager.updateTaskStatus(loadLockAPendingTasks.at(0).taskId, UnifiedWaferTask::TaskType::PM_PROCESS, UnifiedWaferTask::Status::QUEUED);
					}
					else
					{
						logFailedExcuteCommandHasError(lk1->getName(), "数据更新错误", loadlock1_process_name, loadlock1_auto_step);
						loadlock1_auto_step = 950;
					}

					UpdateLLASubTransferDatas();


					if (ui->enableAtmosphere->checkState() == Qt::CheckState::Checked)
					{
						logInform("cycle", "step:1052,大气模式，不关闭TM腔门!");
						
						tool_allow_lla = true;
						if (!pmName.empty())
						{
							if (pmName == "PM1")
							{
								pm1_allow_get_put_wafer = true;
							}
							else if (pmName == "PM2")
							{
								pm2_allow_get_put_wafer = true;
							}
							else if (pmName == "PM3")
							{
								pm3_allow_get_put_wafer = true;
							}
							else if (pmName == "PM4")
							{
								pm4_allow_get_put_wafer = true;
							}
							else
							{
								logFailedExcuteCommandHasError(lk1->getName(), "配方解析错误", loadlock1_process_name, loadlock1_auto_step);
							}
							logInform(lk1->getName().c_str(), "呼叫:%s 取放片.", pmName.c_str());
						}
						else
						{
							logFailedExcuteCommandHasError(lk1->getName(), "配方解析错误", loadlock1_process_name, loadlock1_auto_step);
						}
						loadlock1_auto_step = 950;
					}
					else
					{
						auto cmd = lk1->createCloseTMCavityDoorCommand();
						lk1->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(lk1->getName(), "关闭传输腔门阀", loadlock1_process_name, loadlock1_auto_step);
						}
						else {

							if (!pmName.empty())
							{
								tool_allow_lla = true;

								if (pmName == "PM1")
								{
									pm1_allow_get_put_wafer = true;
								}
								else if (pmName == "PM2")
								{
									pm2_allow_get_put_wafer = true;
								}
								else if (pmName == "PM3")
								{
									pm3_allow_get_put_wafer = true;
								}
								else if(pmName == "PM4")
								{
									pm4_allow_get_put_wafer = true;
								}
								else
								{
									logFailedExcuteCommandHasError(lk1->getName(), "配方解析错误", loadlock1_process_name, loadlock1_auto_step);
								}
								logInform(lk1->getName().c_str(), "呼叫:%s 取放片.", pmName.c_str());
							}
							else
							{
								logFailedExcuteCommandHasError(lk1->getName(), "配方解析错误", loadlock1_process_name, loadlock1_auto_step);
							}
							loadlock1_auto_step = 950;
						}

					}
					
				}
				break;
#pragma endregion

#pragma region 允许放晶圆流程
				case 2000:
				{
					//放片原则，1.是上进下出， 只能放下层的， 2.是上下两层都可放，原则从下到上放，默认第2种
					//放回到指定ll,对应的槽
					if(lk1== nullptr)
					{
						lk1 = kernel->getKernelModule<FortrendLoadLockSubsystem>("LLA");
					}
					if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						if(loadLockAReturnPendingTasks.size() > 0)
						{
							loadlock1_move_slot_index = loadLockAReturnPendingTasks.at(0).targetBlankingSlot;
							loadlock1_auto_step = 2010;
						}
						else
						{
							logInform("Cycle", "step 2000,loadLockAReturnPendingTasks size < 0");
							logFailedNotNormal(lk1->getName(), loadlock1_process_name, loadlock1_auto_step);
						}
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
							if (ui->simulation_cbx->checkState() == Qt::CheckState::Checked)
							{
								logInform(elp->getName().c_str(), "step:2010,模拟放晶圆流程程序....");
								efem_auto_step = 2070;
							}
							else
							{
								loadlock1_auto_step = 2020;
							}
							loadlock1_auto_step = 2030;
						}
						else
						{
							logFailed(lk1->getName(), Poco::format("%s 第%d槽不是空片.", lk1->getName(), loadlock1_move_slot_index));
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
						LLA_start_time = std::chrono::steady_clock::now();
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
						if (ui->enableAtmosphere->checkState() == Qt::CheckState::Checked)
						{
							logInform("cycle", "大气模式，不关闭隔膜阀!");
							loadlock1_auto_step = 2060; //直接放片
						}
						else 
						{
							auto cmd = tm->createCloseDiaphragmValveCommand(TMCavityValveOpening::TMCavity_Both);
							tm->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError())
							{
								logFailedExcuteCommandHasError(lk1->getName(), "关闭隔膜阀失败！", loadlock1_process_name, loadlock1_auto_step);
							}
							else
							{
								loadlock1_auto_step = 2050;
							}
						}
					}
					else
					{
						logFailedNotNormal(lk1->getName(), loadlock1_process_name, loadlock1_auto_step);
					}
				}
				break;
				case 2050:
				{
					if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						if (lk1->getVacuumValueUpperLimitReachesTheSetValue() &&
							tm->getTMCavityVacuumValueUpperLimitReachesTheSetValue())
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
							loadlock1_auto_step = 2030;
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
						if(lk1->getTMCavityDoorOpend())
						{
							//robot 把手臂A的放到LoadLock1
							auto cmd = wtr->createPutCommand(lk1, loadLockAReturnPendingTasks.at(0).arm, loadlock1_move_slot_index);
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
						else
						{
							loadlock1_auto_step = 2050;
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
					taskManager.updateTaskStatus(loadLockAReturnPendingTasks.at(0).taskId, UnifiedWaferTask::LOADLOCK_RETURN, UnifiedWaferTask::COMPLETED);
					//taskManager.updateTaskStatus(loadLockAReturnPendingTasks.at(0).taskId, UnifiedWaferTask::EFEM_RETURN, UnifiedWaferTask::QUEUED);

					if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						if (ui->enableAtmosphere->checkState() == Qt::CheckState::Checked)
						{
							logInform("cycle", "大气模式，不关闭传输腔门阀!");
							loadlock1_auto_step = 2080;
						}
						else
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
							logInform("Cycle", "step:5000 ,大气cycle，跳过破真空.");
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
							logInform("Cycle", "大气cycle，跳过破真空.");
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
					//tool_allow_put_wafer = true;//呼叫EFEM下料
					tool_allow_put_wafer_LLA = true;
					logWarn(lk1->getName().c_str(), "LLA 呼叫EFEM下料.");
					loadlock1_auto_step = 5024;
				}
				break;
				case 5024:
				{
					if (!tool_allow_put_wafer_LLA) {//下料完成
						logWarn(lk1->getName().c_str(), "LLA 呼叫EFEM下料完成.");
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
						if (ui->enableAtmosphere->checkState() == Qt::CheckState::Checked)
						{
							loadlock1_auto_step = 6000;
						}
						else 
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

					if (loadLockAPendingTasks.size() == 0 && current_lp_cycle)
					{
						if(is_lp1_cycle)
						{
							lp1_cycle_one_time_finished = true; //lp1一次
						}
						else
						{
							lp2_cycle_one_time_finished = true;
						}
						logInform(lk1->getName().c_str(), "当前一次:%s循环完成.", is_lp1_cycle ? "is_lp1_cycle" : "is_lp2_cycle");
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
			

				loadlock1_step_once_finished = true;
			}
	
		}
		catch (const std::exception& e) {
			logError("Cyclelog", "LLATransfer thread crashed:", e.what());
			qCritical() << "LLATransfer thread crashed:" << e.what();
		}
		catch (...) {
			logError("Cyclelog", "LLATransfer thread crashed: unknown exception");
			qCritical() << "LLATransfer thread crashed: unknown exception";
		}


	}

	void QSlotTransferCycleVTMWidgetPrivate::executeLLBTransfer()
	{
		try {

		if (ewtr == nullptr || lk1 == nullptr || lk2 == nullptr || wtr == nullptr)
		{
			ewtr = kernel->getKernelModule<EFEMWaferRobotSubsystem>("EWTR");
			lk1 = kernel->getKernelModule<FortrendLoadLockSubsystem>("LLA");
			lk2 = kernel->getKernelModule<FortrendLoadLockSubsystem>("LLB");
			wtr = kernel->getKernelModule<FortrendSunwayRobotSubsystem>("WTR");
		}
			while (!stopRequested)
			{
				{
					std::unique_lock<std::mutex> lock(mtx);
					cv.wait(lock, [this] { return running.load() || stopRequested.load(); });
				}

				if (stopRequested) break;

				// 检查是否需要重置
				if (needReset_LLB.load()) {
					loadlock2_auto_step = 10;
					needReset_LLB = false;
					logWarn(lk2->getName().c_str(), "重置，跳过本次循环剩余部分.");
					continue; // 跳过本次循环剩余部分
				}

				Sleep(500);

				loadlock2_step_once_finished = false;
				switch (loadlock2_auto_step)
				{
				case 10:
				{
					originTaskSize = taskManager.getAllTasksSize();
					lp1TaskSize = taskManager.getTasksByLocation(UnifiedWaferTask::Location::LP1).size();
					lp2TaskSize = taskManager.getTasksByLocation(UnifiedWaferTask::Location::LP2).size();
					UpdateLLBSubTransferDatas();
					UpdateLLASubTransferDatas();
					
					bool isInterlock = isLoadingInterlock("LLB");
					if (isInterlock)
					{
						//logWarn(lk1->getName().c_str(), "有片下料没完成时情景，LLB上料互锁.");
					}

					//判断是否lp2循环
					if (loadLockBPendingTasks.size() == 0 && current_lp_cycle)
					{
						logInform(lk2->getName().c_str(), "此时current_lp_cycle循环.");
						loadlock2_auto_step = 6000;
					}

					//走Loadlock流程：自己的上料完成
					if (!tool_allow_get_wafer_LLB && (loadLockBPendingTasks.size() > 0 || loadLockBReturnCompletedTasks.size() > 0))
					{
						//有wafer，直接抽真空，走取放晶圆流程，下料流程，
						loadlock2_auto_step = 400;
					}
					
					//上料条件：自己的下料完成且LLA下料完成才行。。。。。
					else if ((!isInterlock) && (loadLockBPendingTasks.size() == 0 || loadLockAReturnCompletedTasks.size() == 0 || 0 < efemUnkownStatusTasks.size() <= originTaskSize))
					{
						//无wafer,破真空，让efem上料, 此时lp中的wafer状态是unkown

						if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
							Sleep(200);
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
						logInform("Cycle", "step 20:大气cycle,跳过真空检测.");
						loadlock2_auto_step = 300;
					}
					else
					{
						//排气压力达到设定值9W9  
						if (!lk2->getVacuumPressureGageState() == 1)
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
					tool_allow_get_wafer_LLB = true;//呼叫LP上料
					logInform(lk2->getName().c_str(), "已发送上料请求.");
					loadlock2_auto_step = 302;
				}
				break;
				case 302:
				{
					if (!tool_allow_get_wafer_LLB)
					{	//EFEM上料完成
						logInform(lk2->getName().c_str(), "EFEM上料完成.");
						loadlock2_auto_step = 350;
						loadlock2_put_cassette_finished = true; //放料到loadlock2完成
					}
					else {
						// 添加等待状态提示
						static int wait_count = 0;
						if ((wait_count++ % 20) == 0) {  // 每20次循环打印一次
							logInform(lk2->getName().c_str(), "等待EFEM上料中...");
						}
						Sleep(200);
					}
				}
				break;
				case 350:
				{
					if (loadlock2_put_cassette_finished && !tool_allow_get_wafer_LLB)//上料完成
					{
						//WARNNING:   2025-8-15: 区分开LLA线程操作，
						efemCompletedTasks = taskManager.getEfemCompletedTasks();
						
						auto efemToLLBCompletedTasks = taskManager.getTasksByLocation(efemCompletedTasks,UnifiedWaferTask::Location::LLB);

						for (auto& task : efemToLLBCompletedTasks)
						{
							taskManager.updateTaskStatus(task.taskId, UnifiedWaferTask::TaskType::LOADLOCK_TRANSFER, UnifiedWaferTask::Status::QUEUED);//TASKID:2,LLB槽号是2
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

						if (ui->enableAtmosphere->checkState() == Qt::CheckState::Checked)
						{
							logInform("Cycle", "step:400, 大气cycle，跳过真空检测.");
							loadlock2_auto_step = 800;
						}
						else
						{
							auto cmd = lk2->createCloseCassetteDoorCommand();
							lk2->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError())
							{
								logFailedExcuteCommandHasError(lk2->getName(), "关闭放晶圆盒门阀", loadlock2_process_name, loadlock2_auto_step);
							}
							else
							{
								loadlock2_auto_step = 410;
							}
							
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
						if (lk2->getVacuumEnable() && (!lk2->getVacuumValueReachesTheSetValue())) //|| !lk2->getAngleValveOpend()
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
						if (lk2->getVacuumValueUpperLimitReachesTheSetValue())//检测是否达到极限值
						{
							loadlock2_auto_step = 800;
						}
						else
						{
							loadlock2_auto_step = 500;//继续抽
						}
					}
					else
					{
						if (lk2->getVacuumValueReachesTheSetValue() && loadlock2_get_vacuum && (loadlock1_get_vacuum || tm_get_vacuum))
						{
							logInform("Cycle", "step:510, lk2真空值已经达到设定值，真空规读取数值波动或者泵在抽其他腔室.");
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
							logInform("Cycle", "step:800,大气cycle，跳过真空检测.");
							loadlock2_auto_step = 810;
						}
						else
						{
							if (lk2->getVacuumEnable() && (!lk2->getVacuumValueReachesTheSetValue()))
							{
								loadlock2_auto_step = 500;
							}
							else
							{
								loadlock2_auto_step = 810;
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
					logInform("Cycle", "llB step 900");
					UpdateLLBSubTransferDatas();

					if (loadLockBPendingTasks.size() > 0 || loadLockBReturnPendingTasks.size() > 0)
					{
						loadlock2_auto_step = 901;//取晶圆、放晶圆流程
					}
					else if (loadLockBReturnCompletedTasks.size() > 0) //要兼顾到：若LLa有一片待工艺片，还有一片待efem下料的片，怎么解决？
					{
						loadlock2_auto_step = 5000;//出空casstte的流程
					}
					else
					{
						Sleep(500);
					}
				}
				break;
#pragma endregion
				
				case 901:
				{
					if (ui->enableAtmosphere->checkState() == Qt::CheckState::Checked)
					{
						logInform("Cycle", "step: 901,大气cycle，跳过真空检测.");
						loadlock2_auto_step = 950;
					}
					else
					{
						if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
							if (lk2->getVacuumEnable() && (!lk2->getVacuumValueReachesTheSetValue()))
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
							loadlock2_auto_step = 901;
						}
					}
					else {
						if (lk2->getVacuumValueReachesTheSetValue() && loadlock2_get_vacuum && (loadlock1_get_vacuum || tm_get_vacuum))
						{
							logInform("Cycle", "step:920, lk2真空值已经达到，泵在抽其他腔室或真空数值波动!.");
							loadlock2_get_vacuum = false;//真空值已经达到，泵在抽其他腔室
						}
						Sleep(100);
					}
				}
				break;
				case 950:
				{
					logInform("Cycle", "LLB step 950");
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
					//下料条件：放回到loadlock，就执行下料吗？
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
					loadlock2_move_slot_index = loadLockBPendingTasks.at(0).targetFeedingSlot; //拿第一个task

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
						if (ui->simulation_cbx->checkState() == Qt::CheckState::Checked)
						{
							logInform(elp->getName().c_str(), "step:1010,LLB模拟取晶圆流程程序....");
							efem_auto_step = 1052;
						}
						else
						{
							if (station_cass->getMapping(loadlock2_move_slot_index) == Cassette::Mapping::Present)
							{
								loadlock2_auto_step = 1040;
							}
							else
							{
								logFailed(lk2->getName(), Poco::format("%s 第%d槽不是正常片.", lk2->getName(), loadlock2_move_slot_index));
							}
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
						LLB_start_time = std::chrono::steady_clock::now();
						// 选择不加真空机械手线程，直接操作，真空检测和门阀，阀动作，csr取放动作
						if (CheckLLVacuumMeetsStandard("LLB", 2010))
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
						if (ui->enableAtmosphere->checkState() == Qt::CheckState::Checked)
						{
							logInform("cycle", "大气模式,不关闭TM腔门!");
							loadlock2_auto_step = 1051;
						}
						else
						{
							if (lk2->getVacuumValueUpperLimitReachesTheSetValue() &&
								tm->getTMCavityVacuumValueUpperLimitReachesTheSetValue())
							{
								//Loadlock2 打开传输腔门阀
								auto cmd = lk2->createOpenTMCavityDoorCommand();
								lk2->startCommand(cmd);
								cmd->wait();
								if (cmd->hasError())
								{
									logFailedExcuteCommandHasError(lk2->getName(), "打开传输腔门阀", loadlock2_process_name, loadlock2_auto_step);
								}
								else
								{
									loadlock2_auto_step = 1051;
								}
							}
							else
							{
								loadlock2_auto_step = 1040;
							}
						}
					}
					else
					{
						logFailedNotNormal(lk2->getName(), loadlock2_process_name, loadlock2_auto_step);
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
						//robot 用手臂A 从loadlock2 取wafer

						if (lk2->getTMCavityDoorOpend())
						{
							auto cmd = wtr->createGetCommand(lk2, loadLockBPendingTasks.at(0).arm, loadlock2_move_slot_index);
							wtr->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError())
							{
								logFailedExcuteCommandHasError(wtr->getName(), "取晶圆", loadlock2_process_name, loadlock2_auto_step);
							}
							else
							{
								loadlock2_auto_step = 1052;
							}
						}
						else
						{
							logInform(lk2->getName().c_str(), "此时Tm腔真空波动导致触发自动抽真空流程.");
							loadlock2_auto_step = 1050;
						}
					}
					else
					{
						logFailedNotNormal(wtr->getName(), loadlock2_process_name, loadlock2_auto_step);
					}
				}
				case 1052:
				{
					std::string pmName;
					if(loadLockBPendingTasks.size() >0 )
					{
						pmName = getSelectPmProcessName(loadLockBPendingTasks.at(0));
						logInform(lk2->getName().c_str(), "step:1052,pmName:%s", pmName.c_str());
						//真空机械手取料完成
						taskManager.updateTaskStatus(loadLockBPendingTasks.at(0).taskId, UnifiedWaferTask::TaskType::LOADLOCK_TRANSFER, UnifiedWaferTask::Status::COMPLETED);
						taskManager.updateTaskStatus(loadLockBPendingTasks.at(0).taskId, UnifiedWaferTask::TaskType::PM_PROCESS, UnifiedWaferTask::Status::QUEUED);
					}
					else
					{
						logFailedExcuteCommandHasError(lk1->getName(), "数据更新错误", loadlock1_process_name, loadlock1_auto_step);
						loadlock2_auto_step = 950;
					}

					UpdateLLBSubTransferDatas();

					if (ui->enableAtmosphere->checkState() == Qt::CheckState::Checked)
					{
							logInform("cycle", "step:1052,大气模式，不关闭TM腔门!");
							tool_allow_llb = true;
							if (!pmName.empty())
							{
								if (pmName == "PM1")
								{
									pm1_allow_get_put_wafer = true;
									
								}
								else if (pmName == "PM2")
								{
									pm2_allow_get_put_wafer = true;
									
								}
								else if (pmName == "PM3")
								{
									pm3_allow_get_put_wafer = true;
									
								}
								else
								{
									pm4_allow_get_put_wafer = true;
									
								}
								logInform(lk2->getName().c_str(), "呼叫:%s 取放片.", pmName.c_str());
							}
							else
							{
								logFailedExcuteCommandHasError(lk1->getName(), "配方解析错误", loadlock1_process_name, loadlock1_auto_step);
							}
						loadlock2_auto_step = 950;
					}
					else
					{
						auto cmd = lk2->createCloseTMCavityDoorCommand();
						lk2->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(lk2->getName(), "关闭传输腔门阀", loadlock2_process_name, loadlock2_auto_step);
						}
						else {
							if (!pmName.empty())
							{
								tool_allow_llb = true;

								if (pmName == "PM1")
								{
									pm1_allow_get_put_wafer = true;
									
								}
								else if (pmName == "PM2")
								{
									pm2_allow_get_put_wafer = true;
									
								}
								else if (pmName == "PM3")
								{
									pm3_allow_get_put_wafer = true;
									
								}
								else
								{
									pm4_allow_get_put_wafer = true;
									
								}
								logInform(lk2->getName().c_str(), "呼叫:%s 取放片.", pmName.c_str());
							}
							else
							{
								logFailedExcuteCommandHasError(lk1->getName(), "配方解析错误", loadlock1_process_name, loadlock1_auto_step);
							}
							loadlock2_auto_step = 950;
						}
					}
				}
				break;
				case 1053:
				{
					//for (int i = 0; i < 4; i++)
					//{
					//	if (pm_allow_get_put_wafer_list[i] == 1)
					//	{
					//		break;
					//	}
					//}
				}
				break;

#pragma endregion

#pragma region 允许放晶圆流程
				case 2000:
				{
					//放片原则，1.是上进下出， 只能放下层的， 2.是上下两层都可放，原则从下到上放，默认第2种
					//放回到指定ll,对应的槽
					if(lk2== nullptr)
					{
						lk2 = kernel->getKernelModule<FortrendLoadLockSubsystem>("LLB");
					}
					if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						if(loadLockBReturnPendingTasks.size() > 0 )
						{
							loadlock2_move_slot_index = loadLockBReturnPendingTasks.at(0).targetBlankingSlot;
							loadlock2_auto_step = 2010;
						}
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
							if (ui->simulation_cbx->checkState() == Qt::CheckState::Checked)
							{
								logInform(elp->getName().c_str(), "step:2010,LLB模拟放晶圆流程程序....");
								efem_auto_step = 2070;
							}
							else
							{
								loadlock2_auto_step = 2030;
							}
							
						}
						else
						{
							logFailed(lk2->getName(), Poco::format("%s 第%d槽不是空片.", lk2->getName(), loadlock2_move_slot_index));
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
						LLB_start_time = std::chrono::steady_clock::now();
						// 真空检测和门阀，阀动作，csr取放动作
						if (CheckLLVacuumMeetsStandard("LLB", 2010))
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
						if (ui->enableAtmosphere->checkState() == Qt::CheckState::Checked)
						{
							logInform("cycle", "大气模式，不关闭隔膜阀!");
							loadlock2_auto_step = 2060; //直接放片
						}
						else
						{
							auto cmd = tm->createCloseDiaphragmValveCommand(TMCavityValveOpening::TMCavity_Both);
							tm->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError())
							{
								logFailedExcuteCommandHasError(lk2->getName(), "关闭隔膜阀失败！", loadlock2_process_name, loadlock2_auto_step);
							}
							else
							{
								loadlock2_auto_step = 2050;
							}
						}
					}
					else
					{
						logFailedNotNormal(lk2->getName(), loadlock2_process_name, loadlock2_auto_step);
					}
				}
				break;
				case 2050:
				{
					if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						if (lk2->getVacuumValueUpperLimitReachesTheSetValue() &&
							tm->getTMCavityVacuumValueUpperLimitReachesTheSetValue())
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
							loadlock2_auto_step = 2030;
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
					if(wtr == nullptr)
					{
						wtr = kernel->getKernelModule<FortrendSunwayRobotSubsystem>("WTR");
					}
					if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL && lk2->hasDoorOpend())
					{
						//robot 把手臂A的放到Loadlock2
						if(lk2->getTMCavityDoorOpend())
						{
							auto cmd = wtr->createPutCommand(lk2, loadLockBReturnPendingTasks.at(0).arm, loadlock2_move_slot_index);
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
						else
						{
							loadlock2_auto_step = 2050;
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
					taskManager.updateTaskStatus(loadLockBReturnPendingTasks.at(0).taskId, UnifiedWaferTask::LOADLOCK_RETURN, UnifiedWaferTask::COMPLETED);
					if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						if (ui->enableAtmosphere->checkState() == Qt::CheckState::Checked)
						{
							logInform("cycle", "step:2070,大气模式，不关闭TM腔门!");
							loadlock2_auto_step = 2080;
						}
						else
						{
							auto cmd = lk2->createCloseTMCavityDoorCommand();
							lk2->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError())
							{
								logFailedExcuteCommandHasError(lk2->getName(), "关闭传输腔门阀", loadlock2_process_name, loadlock2_auto_step);
							}
							else
							{
								loadlock2_auto_step = 2080;
							}
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
							logInform("Cycle", "step:5000 ,大气cycle，跳过破真空.");
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
							logInform("Cycle", "大气cycle，跳过破真空.");
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
					//tool_allow_put_wafer = true;//呼叫EFEM下料
					tool_allow_put_wafer_LLB = true;
					logWarn(lk2->getName().c_str(), "LLB 呼叫EFEM下料.");
					loadlock2_auto_step = 5024;
				}
				break;
				case 5024:
				{
					if (!tool_allow_put_wafer_LLB) {//下料完成
						logWarn(lk2->getName().c_str(), "LLB 呼叫EFEM下料完成.");
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
						if (ui->enableAtmosphere->checkState() == Qt::CheckState::Checked)
						{
							loadlock2_auto_step = 6000;
						}
						else
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
					}
					else {
						logFailedNotNormal(lk2->getName(), loadlock2_process_name, loadlock2_auto_step);
					}
					
				}
				break;
				case 6000:
				{
					if (loadLockBPendingTasks.size() == 0 && current_lp_cycle)
					{
						
						if(is_lp2_cycle)
							lp2_cycle_one_time_finished = true;
						else
							lp1_cycle_one_time_finished = true;

						logInform(lk2->getName().c_str(), "当前一次:%s循环完成.", is_lp2_cycle? "is_lp2_cycle":"is_lp1_cycle");
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
		
				loadlock2_step_once_finished = true;
			}
	
		}
		catch (const std::exception& e) {
			logError("Cyclelog", "LLBTransfer thread crashed:", e.what());
			qCritical() << "LLBTransfer thread crashed:" << e.what();
		}
		catch (...) {
			logError("Cyclelog", "LLBTransfer thread crashed: unknown exception");
			qCritical() << "LLBTransfer thread crashed: unknown exception";
		}


	}


	/*
		处理LL腔体取出的wafer，放到PM1腔内，执行spindle工艺，改变task状态，放回到原先的LL腔体
	*/

	void QSlotTransferCycleVTMWidgetPrivate::executePM1Transfer()
	{
		try {
		std::shared_ptr<FortrendSunwayRobotSubsystem> wtr = kernel->getKernelModule<FortrendSunwayRobotSubsystem>("WTR");
		std::shared_ptr<FortrendPMCavitySubsystem> pm1 = kernel->getKernelModule<FortrendPMCavitySubsystem>("PM1");
		// std::shared_ptr<FortrendPMCavitySubsystem> pm2 = kernel->getKernelModule<FortrendPMCavitySubsystem>("PM2");
		// std::shared_ptr<FortrendPMCavitySubsystem> pm3 = kernel->getKernelModule<FortrendPMCavitySubsystem>("PM3");
		// std::shared_ptr<FortrendPMCavitySubsystem> pm4 = kernel->getKernelModule<FortrendPMCavitySubsystem>("PM4");

		auto cassManager = wtr->getKernel()->getKernelModule<FortrendCassetteManager>();

			while (!stopRequested)
			{
				// Wait until running is true
				{
					std::unique_lock<std::mutex> lock(mtx);
					cv.wait(lock, [this] { return running.load() || stopRequested.load(); });
				}

				if (stopRequested) break;

				// 检查是否需要重置
				if (needReset_PM1.load()) {
					pm1_auto_step = 10;
					needReset_PM1 = false;
					continue; // 跳过本次循环剩余部分
				}

				Sleep(500);
				pm1_step_once_finished = false;
				

				switch (pm1_auto_step.load())
				{
					case 10:
					{
						//获取待放片晶圆
						if (pm1_allow_get_put_wafer)
						{//允许取放

							//要区分那个LL腔！！！
							
							loadLockACompletedTasks = taskManager.getLoadLockCompletedTasks("LLA");
							if (loadLockACompletedTasks.size() > 0)
							{
								logInform("PM1", "更新LLA:LOADLOCK_TRANSFER/COMPLETED ------>PM_PROCESS/QUEUED.");
								for (auto& task : loadLockACompletedTasks)
								{
									taskManager.updateTaskStatus(task.taskId, UnifiedWaferTask::TaskType::PM_PROCESS, UnifiedWaferTask::Status::QUEUED);
								}
							}

							loadLockBCompletedTasks = taskManager.getLoadLockCompletedTasks("LLB");
							if (loadLockBCompletedTasks.size() > 0)
							{
								logInform("PM1", "更新LLB:LOADLOCK_TRANSFER/COMPLETED ------>PM_PROCESS/QUEUED.");
								for (auto& task : loadLockBCompletedTasks)
								{
									taskManager.updateTaskStatus(task.taskId, UnifiedWaferTask::TaskType::PM_PROCESS, UnifiedWaferTask::Status::QUEUED);
								}
							}
							UpdatePmSubTransferDatas("PM1");
							logInform("Cycle", Poco::format("%s = %d", pm1_process_name, pm1_auto_step.load()).c_str());
							pm1_auto_step.store(100);
						}
						else
						{
							Sleep(200);
						}

						if (pm1_allow_goto_craft)
						{//转工艺
							pm1_auto_step.store(2000);
							//pm1_auto_step = 2000;
						}
						else {
							Sleep(200);
						}
					}
					break;

					case 100:
					{
						UpdatePmSubTransferDatas("PM1");
						//判断是放、取,考虑交互手
						if (pmPendingTasks.size() > 0 || pmCompletedTasks.size() > 0)
						{
							if (ui->simulation_cbx->checkState() == Qt::CheckState::Checked)
							{
								pm1_auto_step.store(200); //取放片
							}
							else
							{
								pm1_auto_step.store(200); //取放片
							}
							
						}
						else
						{
							Sleep(10);
						}
					}
					break;
					case 199:
					{
						if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
							auto cmd1 = wtr->createCheckLoadCommand(0, 2); //1手
							wtr->startCommand(cmd1);
							cmd1->wait();


							auto cmd2 = wtr->createCheckLoadCommand(1, 2); //1手
							wtr->startCommand(cmd2);
							cmd2->wait();

							if (cmd1->hasError() || cmd2->hasError())
							{
								logFailedExcuteCommandHasError(wtr->getName(), "查询手指有无晶圆", pm1_process_name, pm1_auto_step.load());
							}
							else
							{
								pm1_auto_step.store(200);
							}
						}
						else
						{
							logFailedNotNormal(wtr->getName(), pm1_process_name, pm1_auto_step.load());
						}
					
					}
					break;
					case 200:
					{
						//考虑交互手
						if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
							if (ui->simulation_cbx->checkState() == Qt::CheckState::Checked)
							{
								pm1_auto_step.store(1010);
								//pm1_auto_step = 1010;
							}
							else
							{
								bool haswaferpm = cassManager->getCassette(pm1.get())->getMapping(1) == Cassette::Present;   //pm中有片
								bool haswaferarm1 = cassManager->getCassette(wtr.get())->getMapping(1) == Cassette::Present; //arm1有片
								bool haswaferarm2 = cassManager->getCassette(wtr.get())->getMapping(2) == Cassette::Present; //arm2有片	
#ifdef DEBUG_TEST_PM
								//去PM取放片
								if (!haswaferpm && haswaferarm1) {//手臂1放料
									pm1_auto_step = 1010;
								}
								else if (!haswaferpm && haswaferarm2) {//手臂2放料
									pm1_auto_step = 1030;
								}
								else if (haswaferpm && !haswaferarm1) //手臂1取料
								{
									pm_auto_step = 1040;
								}
								else if (haswaferpm && !haswaferarm2) //手臂2取料
								{
									pm1_auto_step = 1050;
								}
								else if (haswaferpm && !haswaferarm1 && haswaferarm2) {//手1先取，手2后放
									pm1_auto_step = 1060;
								}
								else if (haswaferpm && haswaferarm1 && !haswaferarm2) {//手2先取，手1后放
									pm1_auto_step = 1070;
								}
								else if (haswaferpm && !haswaferarm2 && !haswaferarm1) {//两个手臂没料，A手取
									pm1_auto_step = 1090;
								}

								else {
									logFailedExcuteCommandHasError(wtr->getName(), "机械手晶圆状态不对", pm1_process_name, pm1_auto_step.load());
								}
#else
								//不去PM取放片
								pm1_auto_step.store(1010);
#endif // DEBUG_TEST_PM

							}
						}
						else
						{
							logFailedNotNormal(wtr->getName(), pm1_process_name, pm1_auto_step.load());
						}

						//pm1_auto_step = 1010;
					}
					break;
					case 1010:
					{//放片
						if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
						#ifdef DEBUG_TEST_PM
							{
								robot_selected_arm = 0; //A手
								auto cmd = wtr->createPutCommand(pm1, robot_selected_arm, 1);
								wtr->startCommand(cmd);
								cmd->wait();
								if (cmd->hasError())
								{
									logFailedExcuteCommandHasError(wtr->getName(), "放晶圆", pm_process_name, pm1_auto_step.load());
								}
								else
								{
									//下层的
									//taskManager.updateTaskStatus(pmPendingTasks.at(0).taskId,UnifiedWaferTask::TaskType::PM_PROCESS, UnifiedWaferTask::Status::IN_PROGRESS);
									pm1_allow_get_put_wafer = false;
									pm1_auto_step.store(2000);
								}
							}
						#else
							//TODO:测试阶段，机械手不去PM放晶圆
							logInform1(wtr->getName().c_str(), "测试阶段，机械手不去PM放晶圆.");
							pm1_auto_step.store(2000);

						#endif  //DEBUG_TEST_PM

						}
						else
						{
							logFailedNotNormal(wtr->getName(), pm1_process_name, pm1_auto_step.load());
						}	
					}
					break;
					case 1030:
					{//放片
						if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
							#ifdef DEBUG_TEST_PM
								robot_selected_arm = 1;
								auto cmd = wtr->createPutCommand(pm1, robot_selected_arm, 1);
								wtr->startCommand(cmd);
								cmd->wait();
								if (cmd->hasError())
								{
									logFailedExcuteCommandHasError(wtr->getName(), "放晶圆", pm_process_name, pm1_auto_step);
								}
								else
								{
									//下层的
									//taskManager.updateTaskStatus(pmPendingTasks.at(0).taskId, UnifiedWaferTask::TaskType::PM_PROCESS, UnifiedWaferTask::Status::IN_PROGRESS);
									pm1_allow_get_put_wafer = false;
									pm1_auto_step.store(2000);
								}
							#else
								//TODO:测试阶段，机械手不去PM放晶圆
								logInform1(wtr->getName().c_str(), "测试阶段，机械手不去PM放晶圆.");
								pm1_auto_step.store(2000);
							#endif
						}
						else
						{
							logFailedNotNormal(wtr->getName(), pm_process_name, pm1_auto_step.load());
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
								logFailedExcuteCommandHasError(wtr->getName(), "取晶圆", pm1_process_name, pm1_auto_step.load());
							}
							else
							{
								//下层的
								taskManager.updateTaskStatus(pmPendingTasks.at(0).taskId, UnifiedWaferTask::TaskType::PM_PROCESS, UnifiedWaferTask::Status::COMPLETED);
								taskManager.updateTaskStatus(pmPendingTasks.at(0).taskId, UnifiedWaferTask::TaskType::LOADLOCK_RETURN, UnifiedWaferTask::Status::QUEUED);

								pm1_allow_get_put_wafer = false;
								pm1_auto_step.store(10);
							}
						}
						else
						{
							logFailedNotNormal(wtr->getName(), pm1_process_name, pm1_auto_step.load());
						}
					}
					break;
					case 1050:
					{
						//取片
						if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
							robot_selected_arm = 1;
							auto cmd = wtr->createGetCommand(pm1, robot_selected_arm, 1);
							wtr->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError())
							{
								logFailedExcuteCommandHasError(wtr->getName(), "取晶圆", pm_process_name, pm1_auto_step.load());
							}
							else
							{
								//下层的
								taskManager.updateTaskStatus(pmPendingTasks.at(0).taskId, UnifiedWaferTask::TaskType::PM_PROCESS, UnifiedWaferTask::Status::COMPLETED);
								taskManager.updateTaskStatus(pmPendingTasks.at(0).taskId, UnifiedWaferTask::TaskType::LOADLOCK_RETURN, UnifiedWaferTask::Status::QUEUED);
								pm1_allow_get_put_wafer = false;
								pm1_auto_step.store(10);
							}
						}
						else
						{
							logFailedNotNormal(wtr->getName(), pm_process_name, pm1_auto_step.load());
						}
					}
					break;
#pragma region 交换料
					case 1060:
					{
						if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
							robot_selected_arm = 0;
							auto cmd = wtr->createGetCommand(pm1, robot_selected_arm, 1);
							wtr->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError())
							{
								logFailedExcuteCommandHasError(wtr->getName(), "取晶圆", pm_process_name, pm1_auto_step.load());
							}
							else {
								pmCompletedTasks = taskManager.getPMCompletedTasks("PM1"); //10
								taskManager.updateTaskStatus(pmCompletedTasks.at(0).taskId, UnifiedWaferTask::TaskType::LOADLOCK_RETURN, UnifiedWaferTask::Status::QUEUED);//7
								pm1_auto_step.store(1061);
							}
						}
						else
						{
							logFailedNotNormal(wtr->getName(), pm_process_name, pm1_auto_step.load());
						}
					}
					break;
					case 1061:
					{
						if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
							robot_selected_arm = 1;
							auto cmd = wtr->createPutCommand(pm1, robot_selected_arm, 1);
							wtr->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError())
							{
								logFailedExcuteCommandHasError(wtr->getName(), "放晶圆", pm_process_name, pm1_auto_step.load());
							}
							else
							{
								pm1_allow_get_put_wafer = false;
								pm1_allow_goto_craft = true;
								pm1_auto_step.store(10);
							}
						}
						else
						{
							logFailedNotNormal(wtr->getName(), pm_process_name, pm1_auto_step.load());
						}
					}
					break;
					case 1070:
					{
						if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
							robot_selected_arm = 1;
							auto cmd = wtr->createGetCommand(pm1, robot_selected_arm, 1);
							wtr->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError())
							{
								logFailedExcuteCommandHasError(wtr->getName(), "取晶圆", pm_process_name, pm1_auto_step.load());
							}
							else {

								pmCompletedTasks = taskManager.getPMCompletedTasks("PM1"); //10
								taskManager.updateTaskStatus(pmCompletedTasks.at(0).taskId, UnifiedWaferTask::TaskType::LOADLOCK_RETURN, UnifiedWaferTask::Status::QUEUED);//7
								//可以呼叫loadlocka/loadlockb 走放片到loadlock的流程
								pm1_auto_step.store(1071);
							}

						}
						else
						{
							logFailedNotNormal(wtr->getName(), pm_process_name, pm1_auto_step.load());
						}
					}
					break;
					case 1071:
					{
						if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
							robot_selected_arm = 0;
							auto cmd = wtr->createPutCommand(pm1, robot_selected_arm, 1);
							wtr->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError())
							{
								logFailedExcuteCommandHasError(wtr->getName(), "放晶圆", pm_process_name, pm1_auto_step.load());
							}
							else
							{
								pm1_allow_get_put_wafer = false;
								pm1_allow_goto_craft = true; //转工艺
								pm1_auto_step.store(10);
							}
						}
						else
						{
							logFailedNotNormal(wtr->getName(), pm_process_name, pm1_auto_step.load());
						}
					}
					break;

					case 1090:
					{
						if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
							robot_selected_arm = 0;
							auto cmd = wtr->createGetCommand(pm1, robot_selected_arm, 1);
							wtr->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError())
							{
								logFailedExcuteCommandHasError(wtr->getName(), "取晶圆", pm1_process_name, pm1_auto_step.load());
							}
							else
							{
								pm1_allow_get_put_wafer = false;
								pmCompletedTasks = taskManager.getPMCompletedTasks("PM1"); //10
								taskManager.updateTaskStatus(pmCompletedTasks.at(0).taskId, UnifiedWaferTask::TaskType::LOADLOCK_RETURN, UnifiedWaferTask::Status::QUEUED);//7
								pm1_auto_step.store(10);
							}
						}
						else
						{
							logFailedNotNormal(wtr->getName(), pm1_process_name, pm1_auto_step.load());
						}
					}
					break;
#pragma endregion
					//工艺步骤
					case 2000:
					{
						UpdatePmSubTransferDatas("PM1");			
#ifdef DEBUG_TEST_PM
						auto PmInstance = QPmRecipeWidget::instance(kernel);
						PmInstance->startPmMotorRun(1);
#else
						Sleep(200);
						logInform("PM1", "2s延迟，来模拟做工艺流程.....");
#endif
						if(pmPendingTasks.size() > 0 )
						{
							taskManager.updateTaskStatus(pmPendingTasks.at(0).taskId, UnifiedWaferTask::TaskType::PM_PROCESS, UnifiedWaferTask::Status::COMPLETED);
							taskManager.updateTaskStatus(pmPendingTasks.at(0).taskId, UnifiedWaferTask::TaskType::LOADLOCK_RETURN, UnifiedWaferTask::Status::QUEUED);
							pm1_auto_step.store(10);
						}
						else
						{
							pm1_auto_step.store(10);
						}
					}
					case 2001:
					{

						UpdatePmSubTransferDatas("PM1");
						if(pmCompletedTasks.size() >0 )
						{
							taskManager.updateTaskStatus(pmCompletedTasks.at(0).taskId, UnifiedWaferTask::TaskType::LOADLOCK_RETURN, UnifiedWaferTask::Status::QUEUED);//7
							pm1_allow_get_put_wafer = false;
							pm1_auto_step.store(10);
							Sleep(10);
						}
						else
						{
							pm1_auto_step.store(10);
							Sleep(10);
						}
					}
					break;
					default:
						break;
				}

				pm1_step_once_finished = true;
			}
	
		}
		catch (const std::exception& e) {
			logError("Cyclelog", "PM1Transfer thread crashed:", e.what());
			qCritical() << "PM1Transfer thread crashed:" << e.what();
		}
		catch (...) {
			logError("Cyclelog", "PM1Transfer thread crashed: unknown exception");
			qCritical() << "PM1Transfer thread crashed: unknown exception";
		}
	}

	void QSlotTransferCycleVTMWidgetPrivate::executePM2Transfer()
	{
		try
		{
			std::shared_ptr<FortrendSunwayRobotSubsystem> wtr = kernel->getKernelModule<FortrendSunwayRobotSubsystem>("WTR");
			std::shared_ptr<FortrendPMCavitySubsystem> pm2 = kernel->getKernelModule<FortrendPMCavitySubsystem>("PM2");
			auto cassManager = wtr->getKernel()->getKernelModule<FortrendCassetteManager>();

				while (!stopRequested)
				{
					Sleep(500);
					// Wait until running is true
					{
						std::unique_lock<std::mutex> lock(mtx);
						cv.wait(lock, [this] { return running.load() || stopRequested.load(); });
					}

					if (stopRequested) break;

					// 检查是否需要重置
					if (needReset_PM2.load()) {
						pm2_auto_step = 10;
						needReset_PM2 = false;
						continue; // 跳过本次循环剩余部分
					}

					pm_step_once_finished = false;
					

					switch (pm2_auto_step.load())
					{
					case 10:
					{
						//获取待放片晶圆
						if (pm2_allow_get_put_wafer)
						{//允许取放

							loadLockACompletedTasks = taskManager.getLoadLockCompletedTasks("LLA");
							if (loadLockACompletedTasks.size() > 0)
							{
								logInform("PM2", "更新LLA:LOADLOCK_TRANSFER/COMPLETED ------>PM_PROCESS/QUEUED.");
								for (auto& task : loadLockACompletedTasks)
								{
									taskManager.updateTaskStatus(task.taskId, UnifiedWaferTask::TaskType::PM_PROCESS, UnifiedWaferTask::Status::QUEUED);
								}
							}

							loadLockBCompletedTasks = taskManager.getLoadLockCompletedTasks("LLB");
							if (loadLockBCompletedTasks.size() > 0)
							{
								logInform("PM2", "更新LLB:LOADLOCK_TRANSFER/COMPLETED ------>PM_PROCESS/QUEUED.");
								for (auto& task : loadLockBCompletedTasks)
								{
									taskManager.updateTaskStatus(task.taskId, UnifiedWaferTask::TaskType::PM_PROCESS, UnifiedWaferTask::Status::QUEUED);
								}
							}

							UpdatePmSubTransferDatas("PM2");
							pm2_auto_step.store(100);
							logInform("Cycle", Poco::format("%s = %d", pm2_process_name, pm2_auto_step.load()).c_str());
							//pm2_auto_step = 100;
						}
						else
						{
							Sleep(200);
						}

						if (pm2_allow_goto_craft)
						{//转工艺
							pm2_auto_step.store(2000);
						}
						else {
							Sleep(200);
						}
					}
					break;

					case 100:
					{
						UpdatePmSubTransferDatas("PM2");
						//判断是放、取,不考虑交互手
						if (pm2PendingTasks.size() > 0 || pm2CompletedTasks.size() > 0)
						{
							if (ui->simulation_cbx->checkState() == Qt::CheckState::Checked)
							{
								pm2_auto_step.store(200); //取放片
							}
							else
							{
								pm2_auto_step.store(200); //取放片
							}
							
						}
						else
						{
							Sleep(10);
						}
					}
					break;
					case 199:
					{
						if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
							auto cmd1 = wtr->createCheckLoadCommand(0, 2); //1手
							wtr->startCommand(cmd1);
							cmd1->wait();


							auto cmd2 = wtr->createCheckLoadCommand(1, 2); //1手
							wtr->startCommand(cmd2);
							cmd2->wait();

							if (cmd1->hasError() || cmd2->hasError())
							{
								logFailedExcuteCommandHasError(wtr->getName(), "查询手指有无晶圆", pm2_process_name, pm2_auto_step.load());
							}
							else
							{
								pm2_auto_step.store(200);
							}
						}
						else
						{
							logFailedNotNormal(wtr->getName(), pm2_process_name, pm2_auto_step.load());
						}

					}
					break;
					case 200:
					{
						//暂不考虑交互手
						if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
							if (ui->simulation_cbx->checkState() == Qt::CheckState::Checked)
							{
								pm2_auto_step.store(1030);
							}
							else
							{
								bool haswaferpm = cassManager->getCassette(pm2.get())->getMapping(1) == Cassette::Present;   //pm2中有片
								bool haswaferarm1 = cassManager->getCassette(wtr.get())->getMapping(1) == Cassette::Present; //arm1有片
								bool haswaferarm2 = cassManager->getCassette(wtr.get())->getMapping(2) == Cassette::Present; //arm2有片
								
#ifdef DEBUG_TEST_PM
								if (!haswaferpm && haswaferarm1) {//手臂1放料
									pm2_auto_step = 1010;
								}
								else if (!haswaferpm && haswaferarm2) {//手臂2放料
									pm2_auto_step = 1030;
								}

								else if (haswaferpm && !haswaferarm1) //手臂1取料
								{
									pm2_auto_step = 1040;
								}
								else if (haswaferpm && !haswaferarm2) //手臂1取料
								{
									pm2_auto_step = 1050;
								}
								else if (haswaferpm && !haswaferarm1 && haswaferarm2) {//手1先取，手2后放
									pm2_auto_step = 1060;
								}
								else if (haswaferpm && haswaferarm1 && !haswaferarm2) {//手2先取，手1后放
									pm2_auto_step = 1070;
								}
								else if (haswaferpm && !haswaferarm2 && !haswaferarm1) {//两个手臂没料，A手取
									pm2_auto_step = 1090;
								}

								else {
									logFailedExcuteCommandHasError(wtr->getName(), "机械手晶圆状态不对", pm2_process_name, pm2_auto_step);
								}
#else
								pm2_auto_step.store(1010);
#endif // DEBUG_TEST_PM
							}
						}
						else
						{
							logFailedNotNormal(wtr->getName(), pm2_process_name, pm2_auto_step.load());
						}
						//pm2_auto_step.store(1010);

					}
					break;
					case 1010:
					{//放片
						if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
#ifdef DEBUG_TEST_PM
						robot_selected_arm = 0;
						auto cmd = wtr->createPutCommand(pm1, robot_selected_arm, 1);
						wtr->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(wtr->getName(), "放晶圆", pm2_process_name, pm2_auto_step.load());
						}
						else
						{
							//下层的
							//taskManager.updateTaskStatus(pmPendingTasks.at(0).taskId,UnifiedWaferTask::TaskType::PM_PROCESS, UnifiedWaferTask::Status::IN_PROGRESS);
							pm2_allow_get_put_wafer = false;	
							pm2_auto_step.store(2000);
						}
#else
						//TODO:测试阶段，机械手不去PM放晶圆	
						logInform1(wtr->getName().c_str(), "测试阶段，机械手不去PM放晶圆.");
						pm2_auto_step.store(2000);
#endif
						}
						else
						{
							logFailedNotNormal(wtr->getName(), pm2_process_name, pm2_auto_step.load());
						}
					}
					break;
					case 1030:
					{//放片
						if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
#ifdef DEBUG_TEST_PM
							robot_selected_arm = 1;
							auto cmd = wtr->createPutCommand(pm2, robot_selected_arm, 1);
							wtr->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError())
							{
								logFailedExcuteCommandHasError(wtr->getName(), "放晶圆", pm2_process_name, pm2_auto_step.load());
							}
							else
							{
								//下层的
								//taskManager.updateTaskStatus(pmPendingTasks.at(0).taskId, UnifiedWaferTask::TaskType::PM_PROCESS, UnifiedWaferTask::Status::IN_PROGRESS);
								pm2_allow_get_put_wafer = false;
								pm2_auto_step.store(2000);
							}
#else
							//TODO:测试阶段，机械手不去PM放晶圆	
							logInform1(wtr->getName().c_str(), "测试阶段，机械手不去PM放晶圆.");
							pm2_auto_step.store(2000);
#endif
						}
						else
						{
							logFailedNotNormal(wtr->getName(), pm2_process_name, pm2_auto_step.load());
						}
					}
					break;
					case 1040:
					{
						//取片
						if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
							robot_selected_arm = 0;
							auto cmd = wtr->createGetCommand(pm2, robot_selected_arm, 1);
							wtr->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError())
							{
								logFailedExcuteCommandHasError(wtr->getName(), "取晶圆", pm2_process_name, pm2_auto_step.load());
							}
							else
							{
								//下层的
								taskManager.updateTaskStatus(pm2PendingTasks.at(0).taskId, UnifiedWaferTask::TaskType::PM_PROCESS, UnifiedWaferTask::Status::COMPLETED);
								taskManager.updateTaskStatus(pm2PendingTasks.at(0).taskId, UnifiedWaferTask::TaskType::LOADLOCK_RETURN, UnifiedWaferTask::Status::QUEUED);

								pm2_allow_get_put_wafer = false;
								pm2_auto_step.store(10);
							}
						}
						else
						{
							logFailedNotNormal(wtr->getName(), pm2_process_name, pm2_auto_step.load());
						}
					}
					break;
					case 1050:
					{
						if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
							robot_selected_arm = 1;
							auto cmd = wtr->createGetCommand(pm2, robot_selected_arm, 1);
							wtr->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError())
							{
								logFailedExcuteCommandHasError(wtr->getName(), "取晶圆", pm2_process_name, pm2_auto_step.load());
							}
							else
							{
								//下层的
								taskManager.updateTaskStatus(pm2PendingTasks.at(0).taskId, UnifiedWaferTask::TaskType::PM_PROCESS, UnifiedWaferTask::Status::COMPLETED);
								taskManager.updateTaskStatus(pm2PendingTasks.at(0).taskId, UnifiedWaferTask::TaskType::LOADLOCK_RETURN, UnifiedWaferTask::Status::QUEUED);
								pm2_allow_get_put_wafer = false;
								pm2_auto_step.store(10);
							}
						}
						else
						{
							logFailedNotNormal(wtr->getName(), pm2_process_name, pm2_auto_step);
						}
					}
					break;
#pragma region 交换料
					case 1060:
					{
						if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
							robot_selected_arm = 0;
							auto cmd = wtr->createGetCommand(pm2, robot_selected_arm, 1);
							wtr->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError())
							{
								logFailedExcuteCommandHasError(wtr->getName(), "取晶圆", pm2_process_name, pm2_auto_step.load());
							}
							else {
								pm2CompletedTasks = taskManager.getPMCompletedTasks("PM2"); //10
								taskManager.updateTaskStatus(pm2CompletedTasks.at(0).taskId, UnifiedWaferTask::TaskType::LOADLOCK_RETURN, UnifiedWaferTask::Status::QUEUED);//7
								pm2_auto_step.store(1061);
							}
						}
						else
						{
							logFailedNotNormal(wtr->getName(), pm2_process_name, pm2_auto_step.load());
						}
					}
					break;
					case 1061:
					{
						if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
							robot_selected_arm = 1;
							auto cmd = wtr->createPutCommand(pm2, robot_selected_arm, 1);
							wtr->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError())
							{
								logFailedExcuteCommandHasError(wtr->getName(), "放晶圆", pm2_process_name, pm2_auto_step.load());	
							}
							else
							{
								pm2_allow_get_put_wafer = false;
								pm2_allow_goto_craft = true;
								pm2_auto_step.store(10);
							}
						}
						else
						{
							logFailedNotNormal(wtr->getName(), pm2_process_name, pm2_auto_step.load());
						}
					}
					break;
					case 1070:
					{
						if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
							robot_selected_arm = 1;
							auto cmd = wtr->createGetCommand(pm2, robot_selected_arm, 1);
							wtr->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError())
							{
								logFailedExcuteCommandHasError(wtr->getName(), "取晶圆", pm2_process_name, pm2_auto_step.load());
							}
							else {

								pm2CompletedTasks = taskManager.getPMCompletedTasks("PM2"); //10	
								taskManager.updateTaskStatus(pm2CompletedTasks.at(0).taskId, UnifiedWaferTask::TaskType::LOADLOCK_RETURN, UnifiedWaferTask::Status::QUEUED);//7
								//可以呼叫loadlocka/loadlockb 走放片到loadlock的流程
								pm2_auto_step.store(1071);
							}

						}
						else
						{
							logFailedNotNormal(wtr->getName(), pm2_process_name, pm2_auto_step.load());
						}
					}
					break;
					case 1071:
					{
						if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
							robot_selected_arm = 0;
							auto cmd = wtr->createPutCommand(pm2, robot_selected_arm, 1);
							wtr->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError())
							{
								logFailedExcuteCommandHasError(wtr->getName(), "放晶圆", pm2_process_name, pm2_auto_step.load());
							}
							else
							{
								pm2_allow_get_put_wafer = false;
								pm2_allow_goto_craft = true; //转工艺
								pm2_auto_step.store(10);
							}
						}
						else
						{
							logFailedNotNormal(wtr->getName(), pm2_process_name, pm2_auto_step.load());
						}
					}
					break;					

#pragma endregion
					case 1090:
					{
						if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
							robot_selected_arm = 0;
							auto cmd = wtr->createGetCommand(pm2, robot_selected_arm, 1);
							wtr->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError())
							{
								logFailedExcuteCommandHasError(wtr->getName(), "取晶圆", pm2_process_name, pm2_auto_step.load());
							}
							else
							{
								pm2_allow_get_put_wafer = false;
								pm2CompletedTasks = taskManager.getPMCompletedTasks("PM2"); //10
								taskManager.updateTaskStatus(pm2CompletedTasks.at(0).taskId, UnifiedWaferTask::TaskType::LOADLOCK_RETURN, UnifiedWaferTask::Status::QUEUED);//7
								pm2_auto_step.store(10);
							}
						}
						else
						{
							logFailedNotNormal(wtr->getName(), pm2_process_name, pm2_auto_step.load());
						}
					}
					break;

					case 2000:
					{
						
						UpdatePmSubTransferDatas("PM2");

#ifdef DEBUG_TEST_PM
						auto PmInstance = QPmRecipeWidget::instance(kernel);
						PmInstance->startPmMotorRun(2);
#else
						//Sleep(2000);
						logInform("PM2", "2s延迟，来模拟做工艺流程.....");
#endif

						if(pm2PendingTasks.size() > 0)
						{
							taskManager.updateTaskStatus(pm2PendingTasks.at(0).taskId, UnifiedWaferTask::TaskType::PM_PROCESS, UnifiedWaferTask::Status::COMPLETED);
							taskManager.updateTaskStatus(pm2PendingTasks.at(0).taskId, UnifiedWaferTask::TaskType::LOADLOCK_RETURN, UnifiedWaferTask::Status::QUEUED);//7
							pm2_auto_step.store(10);
						}
						else
						{
							pm2_auto_step.store(10);
						}
					}
					case 2001:
					{
						UpdatePmSubTransferDatas("PM2");
						if(pm2CompletedTasks.size() > 0)
						{
							taskManager.updateTaskStatus(pm2CompletedTasks.at(0).taskId, UnifiedWaferTask::TaskType::LOADLOCK_RETURN, UnifiedWaferTask::Status::QUEUED);//7
							pm2_allow_get_put_wafer = false;

							pm2_auto_step.store(10);
							Sleep(10);
						}
						else
						{
							pm2_auto_step.store(10);
							Sleep(10);
						}
					}
					break;
					default:
						break;
				}

				
					pm_step_once_finished = true;
				}

		}
		catch (const std::exception& e) {
			logError("Cyclelog", "PM2Transfer thread crashed:", e.what());
			qCritical() << "PM2ransfer thread crashed:" << e.what();
		}
		catch (...) {
			logError("Cyclelog", "PM2Transfer thread crashed: unknown exception");
			qCritical() << "PM2Transfer thread crashed: unknown exception";
		}
		
	}

	void QSlotTransferCycleVTMWidgetPrivate::executePM3Transfer()
	{
		try
		{
			std::shared_ptr<FortrendSunwayRobotSubsystem> wtr = kernel->getKernelModule<FortrendSunwayRobotSubsystem>("WTR");
			std::shared_ptr<FortrendPMCavitySubsystem> pm3 = kernel->getKernelModule<FortrendPMCavitySubsystem>("PM3");
			auto cassManager = wtr->getKernel()->getKernelModule<FortrendCassetteManager>();

				while (!stopRequested)
				{
					Sleep(500);
					// Wait until running is true
					{
						std::unique_lock<std::mutex> lock(mtx);
						cv.wait(lock, [this] { return running.load() || stopRequested.load(); });
					}

					if (stopRequested) break;

					// 检查是否需要重置
					if (needReset_PM3.load()) {
						pm3_auto_step = 10;
						needReset_PM3 = false;
						continue; // 跳过本次循环剩余部分
					}

					pm3_step_once_finished = false;
					//logInform("Cycle", Poco::format("%s = %d", pm3_process_name, pm3_auto_step.load()).c_str());

					switch (pm3_auto_step.load())
					{
					case 10:
					{
						//获取待放片晶圆
						if (pm3_allow_get_put_wafer)
						{//允许取放
							loadLockACompletedTasks = taskManager.getLoadLockCompletedTasks("LLA");
							if (loadLockACompletedTasks.size() > 0)
							{
								logInform("PM3", "更新LLA:LOADLOCK_TRANSFER/COMPLETED ------>PM_PROCESS/QUEUED.");
								for (auto& task : loadLockACompletedTasks)
								{
									taskManager.updateTaskStatus(task.taskId, UnifiedWaferTask::TaskType::PM_PROCESS, UnifiedWaferTask::Status::QUEUED);
								}
							}


							loadLockBCompletedTasks = taskManager.getLoadLockCompletedTasks("LLB");
							if (loadLockBCompletedTasks.size() > 0)
							{
								logInform("PM3", "更新LLB:LOADLOCK_TRANSFER/COMPLETED ------>PM_PROCESS/QUEUED.");
								for (auto& task : loadLockBCompletedTasks)
								{
									taskManager.updateTaskStatus(task.taskId, UnifiedWaferTask::TaskType::PM_PROCESS, UnifiedWaferTask::Status::QUEUED);
								}
							}


							UpdatePmSubTransferDatas("PM3");
							pm3_auto_step.store(100);
							//pm3_auto_step = 100;
						}
						else
						{
							Sleep(200);
						}

						if (pm3_allow_goto_craft)
						{//转工艺
							pm3_auto_step.store(2000);
						}
						else {
							Sleep(200);
						}
					}
					break;

					case 100:
					{
						UpdatePmSubTransferDatas("PM3");
						//判断是放、取,不考虑交互手
						if (pm3PendingTasks.size() > 0 || pm3CompletedTasks.size() > 0)
						{
							pm3_auto_step.store(200); //取放片
						}
						else
						{
							Sleep(10);
						}
					}
					break;
					case 199:
					{
						if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
							auto cmd1 = wtr->createCheckLoadCommand(0, 2); //1手
							wtr->startCommand(cmd1);
							cmd1->wait();

							auto cmd2 = wtr->createCheckLoadCommand(1, 2); //1手
							wtr->startCommand(cmd2);
							cmd2->wait();

							if (cmd1->hasError() || cmd2->hasError())
							{
								logFailedExcuteCommandHasError(wtr->getName(), "查询手指有无晶圆", pm3_process_name, pm3_auto_step);
							}
							else
							{
								pm3_auto_step.store(200);
							}
						}
						else
						{
							logFailedNotNormal(wtr->getName(), pm3_process_name, pm3_auto_step);
						}

					}
					break;
					case 200:
					{
						//暂不考虑交互手
						if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{

							bool haswaferpm = cassManager->getCassette(pm3.get())->getMapping(1) == Cassette::Present;   //pm中有片
							bool haswaferarm1 = cassManager->getCassette(wtr.get())->getMapping(1) == Cassette::Present; //arm1有片
							bool haswaferarm2 = cassManager->getCassette(wtr.get())->getMapping(2) == Cassette::Present; //arm2有片						
#ifdef DEBUG_TEST_PM
							if (!haswaferpm && haswaferarm1) {//手臂1放料
								pm3_auto_step.store(1010);
							}
							else if (!haswaferpm && haswaferarm2) {//手臂2放料
								pm3_auto_step.store(1030);
							}

							else if (haswaferpm && !haswaferarm1) //手臂1取料
							{
								pm3_auto_step.store(1040);
							}
							else if (haswaferpm && !haswaferarm2) //手臂1取料
							{
								pm3_auto_step.store(1050);
							}
							else if (haswaferpm && !haswaferarm1 && haswaferarm2) {//手1先取，手2后放
								pm3_auto_step.store(1050);
							}
							else if (haswaferpm && haswaferarm1 && !haswaferarm2) {//手2先取，手1后放
								pm3_auto_step.store(1070);
							}
							else if (haswaferpm && !haswaferarm2 && !haswaferarm1) {//两个手臂没料，A手取
								pm3_auto_step.store(1090);
							}

							else {
								logFailedExcuteCommandHasError(wtr->getName(), "机械手晶圆状态不对", pm3_process_name, pm3_auto_step);
							}
#else
							pm3_auto_step.store(1010);
#endif
						}
						else
						{
							logFailedNotNormal(wtr->getName(), pm3_process_name, pm3_auto_step.load());
						}
						//pm3_auto_step.store(1010);

					}
					break;
					case 1010:
					{//放片
						if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
#ifdef DEBUG_TEST_PM
							
							robot_selected_arm = 0; //A手
							auto cmd = wtr->createPutCommand(pm3, robot_selected_arm, 1);
							wtr->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError())
							{
								logFailedExcuteCommandHasError(wtr->getName(), "放晶圆", pm3_process_name, pm3_auto_step.load());
							}
							else
							{
								//下层的
								//taskManager.updateTaskStatus(pmPendingTasks.at(0).taskId,UnifiedWaferTask::TaskType::PM_PROCESS, UnifiedWaferTask::Status::IN_PROGRESS);
								pm3_allow_get_put_wafer = false;
								pm3_auto_step.store(2000);
							}
							
#else
							//TODO:测试阶段，机械手不去PM放晶圆
							logInform1(wtr->getName().c_str(), "测试阶段，机械手不去PM放晶圆.");
							pm3_auto_step.store(2000);

#endif  //DEBUG_TEST_PM

						}
						else
						{
							logFailedNotNormal(wtr->getName(), pm3_process_name, pm3_auto_step);
						}
					}
					break;
					case 1030:
					{//放片
						if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
#ifdef DEBUG_TEST_PM
							robot_selected_arm = 1;
							auto cmd = wtr->createPutCommand(pm3, robot_selected_arm, 1);
							wtr->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError())
							{
								logFailedExcuteCommandHasError(wtr->getName(), "放晶圆", pm3_process_name, pm3_auto_step.load());
							}
							else
							{
								//下层的
								//taskManager.updateTaskStatus(pmPendingTasks.at(0).taskId, UnifiedWaferTask::TaskType::PM_PROCESS, UnifiedWaferTask::Status::IN_PROGRESS);

								pm3_allow_get_put_wafer = false;
								pm3_auto_step.store(2000);
							}
#else
							logInform1(wtr->getName().c_str(), "测试阶段，机械手不去PM放晶圆.");
							pm3_allow_get_put_wafer = false;
							pm3_auto_step.store(2000);
#endif
						}
						else
						{
							logFailedNotNormal(wtr->getName(), pm3_process_name, pm3_auto_step);
						}
					}
					break;
					case 1040:
					{
						//取片
						if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
							robot_selected_arm = 0;
							auto cmd = wtr->createGetCommand(pm3, robot_selected_arm, 1);
							wtr->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError())
							{
								logFailedExcuteCommandHasError(wtr->getName(), "取晶圆", pm3_process_name, pm3_auto_step.load());
							}
							else
							{
								//下层的
								taskManager.updateTaskStatus(pm3PendingTasks.at(0).taskId, UnifiedWaferTask::TaskType::PM_PROCESS, UnifiedWaferTask::Status::COMPLETED);
								taskManager.updateTaskStatus(pm3PendingTasks.at(0).taskId, UnifiedWaferTask::TaskType::LOADLOCK_RETURN, UnifiedWaferTask::Status::QUEUED);

								pm3_allow_get_put_wafer = false;
								pm3_auto_step.store(10);
							}
						}
						else
						{
							logFailedNotNormal(wtr->getName(), pm3_process_name, pm3_auto_step.load());
						}
					}
					break;
					case 1050:
					{
						if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
							robot_selected_arm = 1;
							auto cmd = wtr->createGetCommand(pm3, robot_selected_arm, 1);
							wtr->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError())
							{
								logFailedExcuteCommandHasError(wtr->getName(), "取晶圆", pm3_process_name, pm3_auto_step.load());
							}
							else
							{
								//下层的
								taskManager.updateTaskStatus(pm3PendingTasks.at(0).taskId, UnifiedWaferTask::TaskType::PM_PROCESS, UnifiedWaferTask::Status::COMPLETED);
								taskManager.updateTaskStatus(pm3PendingTasks.at(0).taskId, UnifiedWaferTask::TaskType::LOADLOCK_RETURN, UnifiedWaferTask::Status::QUEUED);
								pm3_allow_get_put_wafer = false;
								pm3_auto_step.store(10);
							}
						}
						else
						{
							logFailedNotNormal(wtr->getName(), pm3_process_name, pm3_auto_step.load());
						}
					}
					break;
#pragma region 交换料
					case 1060:
					{
						if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
							robot_selected_arm = 0;
							auto cmd = wtr->createGetCommand(pm3, robot_selected_arm, 1);
							wtr->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError())
							{
								logFailedExcuteCommandHasError(wtr->getName(), "取晶圆", pm3_process_name, pm3_auto_step.load());
							}
							else {
								pm3CompletedTasks = taskManager.getPMCompletedTasks("PM3"); //10
								taskManager.updateTaskStatus(pm3CompletedTasks.at(0).taskId, UnifiedWaferTask::TaskType::LOADLOCK_RETURN, UnifiedWaferTask::Status::QUEUED);//7
								pm3_auto_step.store(1061);
							}
						}
						else
						{
							logFailedNotNormal(wtr->getName(), pm3_process_name, pm3_auto_step.load());
						}
					}
					break;
					case 1061:
					{
						if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
							robot_selected_arm = 1;
							auto cmd = wtr->createPutCommand(pm3, robot_selected_arm, 1);
							wtr->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError())
							{
								logFailedExcuteCommandHasError(wtr->getName(), "放晶圆", pm3_process_name, pm3_auto_step.load());
							}
							else
							{
								pm3_allow_get_put_wafer = false;
								pm3_allow_goto_craft = true;
								pm3_auto_step.store(10);
							}
						}
						else
						{
							logFailedNotNormal(wtr->getName(), pm3_process_name, pm3_auto_step.load());
						}
					}
					break;
					case 1070:
					{
						if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
							robot_selected_arm = 1;
							auto cmd = wtr->createGetCommand(pm3, robot_selected_arm, 1);
							wtr->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError())
							{
								logFailedExcuteCommandHasError(wtr->getName(), "取晶圆", pm3_process_name, pm3_auto_step.load());
							}
							else {

								pm3CompletedTasks = taskManager.getPMCompletedTasks("PM3"); //10
								taskManager.updateTaskStatus(pm3CompletedTasks.at(0).taskId, UnifiedWaferTask::TaskType::LOADLOCK_RETURN, UnifiedWaferTask::Status::QUEUED);//7
								pm3_auto_step.store(1071);
							}

						}
						else
						{
							logFailedNotNormal(wtr->getName(), pm3_process_name, pm3_auto_step.load());
						}
					}
					break;
					case 1071:
					{
						if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
							robot_selected_arm = 0;
							auto cmd = wtr->createPutCommand(pm3, robot_selected_arm, 1);
							wtr->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError())
							{
								logFailedExcuteCommandHasError(wtr->getName(), "放晶圆", pm3_process_name, pm3_auto_step.load());
							}
							else
							{
								pm3_allow_get_put_wafer = false;
								pm3_allow_goto_craft = true; //转工艺
								pm3_auto_step.store(10);
							}
						}
						else
						{
							logFailedNotNormal(wtr->getName(), pm3_process_name, pm3_auto_step.load());
						}
					}
					break;
#pragma endregion
					case 1090:
					{
						if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
							robot_selected_arm = 0;
							auto cmd = wtr->createGetCommand(pm3, robot_selected_arm, 1);
							wtr->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError())
							{
								logFailedExcuteCommandHasError(wtr->getName(), "取晶圆", pm3_process_name, pm3_auto_step.load());
							}
							else
							{
								pm3_allow_get_put_wafer = false;
								pm3CompletedTasks = taskManager.getPMCompletedTasks("PM3"); //10
								taskManager.updateTaskStatus(pm3CompletedTasks.at(0).taskId, UnifiedWaferTask::TaskType::LOADLOCK_RETURN, UnifiedWaferTask::Status::QUEUED);//7
								pm3_auto_step.store(10);
							}
						}
						else
						{
							logFailedNotNormal(wtr->getName(), pm3_process_name, pm3_auto_step.load());
						}
					}
					break;
					case 2000:
					{
						Sleep(500);
						UpdatePmSubTransferDatas("PM3");

#ifdef DEBUG_TEST_PM
						auto PmInstance = QPmRecipeWidget::instance(kernel);
						PmInstance->startPmMotorRun(3);

#else
						logInform("PM3", "模拟做工艺流程.....");
#endif
						if(pm3PendingTasks.size() >0 )
						{
							taskManager.updateTaskStatus(pm3PendingTasks.at(0).taskId, UnifiedWaferTask::TaskType::PM_PROCESS, UnifiedWaferTask::Status::COMPLETED);
							taskManager.updateTaskStatus(pm3PendingTasks.at(0).taskId, UnifiedWaferTask::TaskType::LOADLOCK_RETURN, UnifiedWaferTask::Status::QUEUED);//7
							pm3_auto_step.store(10);
						}
						else
						{
							pm3_auto_step.store(10);
						}
					}
					break;
					case 2001:
					{
						//pmCompletedTasks = taskManager.getPMCompletedTasks("PM3"); //10
						UpdatePmSubTransferDatas("PM3");
						if(pm3CompletedTasks.size() > 0)
						{
							taskManager.updateTaskStatus(pm3CompletedTasks.at(0).taskId, UnifiedWaferTask::TaskType::LOADLOCK_RETURN, UnifiedWaferTask::Status::QUEUED);//7
							pm3_allow_get_put_wafer = false;
							Sleep(10);
							pm3_auto_step.store(10);
						}
						else
						{
							Sleep(10);
							pm3_auto_step.store(10);
						}
					}
					break;
					default:
						break;
					}
					pm3_step_once_finished = true;
				}

		}
		catch (const std::exception& e) {
			logError("Cyclelog", "PM3Transfer thread crashed:", e.what());
			qCritical() << "PM3Transfer thread crashed:" << e.what();
		}
		catch (...) {
			logError("Cyclelog", "PM3Transfer thread crashed: unknown exception");
			qCritical() << "PM3Transfer thread crashed: unknown exception";
		}
	}

	void QSlotTransferCycleVTMWidgetPrivate::executePM4Transfer()
	{
		try
		{
			std::shared_ptr<FortrendSunwayRobotSubsystem> wtr = kernel->getKernelModule<FortrendSunwayRobotSubsystem>("WTR");
			std::shared_ptr<FortrendPMCavitySubsystem> pm4 = kernel->getKernelModule<FortrendPMCavitySubsystem>("PM4");
			auto cassManager = wtr->getKernel()->getKernelModule<FortrendCassetteManager>();

				while (!stopRequested)
				{
					Sleep(500);
					{
						std::unique_lock<std::mutex> lock(mtx);
						cv.wait(lock, [this] { return running.load() || stopRequested.load(); }); //确保线程在需要停止时能被立即唤醒
					}

					if (stopRequested) break;

					// 检查是否需要重置
					if (needReset_PM4.load()) {
						pm4_auto_step = 10;
						needReset_PM4 = false;
						continue; // 跳过本次循环剩余部分
					}

					pm4_step_once_finished = false;
					//logInform("Cycle", Poco::format("%s = %d", pm4_process_name, pm4_auto_step.load()).c_str());

					switch (pm4_auto_step.load())
					{
					case 10:
					{
						//获取待放片晶圆
						if (pm4_allow_get_put_wafer)
						{//允许取放

							loadLockACompletedTasks = taskManager.getLoadLockCompletedTasks("LLA");
							if (loadLockACompletedTasks.size() > 0)
							{
								logInform("PM4", "更新LLA:LOADLOCK_TRANSFER/COMPLETED ------>PM_PROCESS/QUEUED.");
								for (auto& task : loadLockACompletedTasks)
								{
									taskManager.updateTaskStatus(task.taskId, UnifiedWaferTask::TaskType::PM_PROCESS, UnifiedWaferTask::Status::QUEUED);
								}
							}

							loadLockBCompletedTasks = taskManager.getLoadLockCompletedTasks("LLB");
							if (loadLockBCompletedTasks.size() > 0)
							{
								logInform("PM4", "更新LLB:LOADLOCK_TRANSFER/COMPLETED ------>PM_PROCESS/QUEUED.");
								for (auto& task : loadLockBCompletedTasks)
								{
									taskManager.updateTaskStatus(task.taskId, UnifiedWaferTask::TaskType::PM_PROCESS, UnifiedWaferTask::Status::QUEUED);
								}
							}

							UpdatePmSubTransferDatas("PM4");
							pm4_auto_step.store(100); //改成100,不然下不去
						}
						else
						{
							Sleep(200);
						}

						if (pm4_allow_goto_craft)
						{//转工艺
							pm4_auto_step.store(2000);
						}
						else {
							Sleep(200);
						}
					}
					break;

					case 100:
					{
						UpdatePmSubTransferDatas("PM4");
						//判断是放、取,不考虑交互手
						if (pm4PendingTasks.size() > 0 || pm4CompletedTasks.size() > 0)
						{
							pm4_auto_step.store(200); //取放片
						}
						else
						{
							Sleep(10);
						}
					}
					break;
					case 199:
					{
						if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
							auto cmd1 = wtr->createCheckLoadCommand(0, 2); //1手
							wtr->startCommand(cmd1);
							cmd1->wait();


							auto cmd2 = wtr->createCheckLoadCommand(1, 2); //1手
							wtr->startCommand(cmd2);
							cmd2->wait();

							if (cmd1->hasError() || cmd2->hasError())
							{
								logFailedExcuteCommandHasError(wtr->getName(), "查询手指有无晶圆", pm4_process_name, pm4_auto_step);
							}
							else
							{
								pm4_auto_step.store(200);
							}
						}
						else
						{
							logFailedNotNormal(wtr->getName(), pm4_process_name, pm4_auto_step);
						}

					}
					break;
					case 200:
					{
						//暂不考虑交互手
						if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{

							bool haswaferpm = cassManager->getCassette(pm4.get())->getMapping(1) == Cassette::Present;   //pm中有片
							bool haswaferarm1 = cassManager->getCassette(wtr.get())->getMapping(1) == Cassette::Present; //arm1有片
							bool haswaferarm2 = cassManager->getCassette(wtr.get())->getMapping(2) == Cassette::Present; //arm2有片

#ifdef DEBUG_TEST_PM
							if (!haswaferpm && haswaferarm1) {//手臂1放料
								pm4_auto_step.store(1010);
							}
							else if (!haswaferpm && haswaferarm2) {//手臂2放料
								pm4_auto_step.store(1030);
							}

							else if (haswaferpm && !haswaferarm1) //手臂1取料
							{
								pm4_auto_step.store(1040);
							}
							else if (haswaferpm && !haswaferarm2) //手臂1取料
							{
								pm4_auto_step.store(1050);
							}
							else if (haswaferpm && !haswaferarm1 && haswaferarm2) {//手1先取，手2后放
								pm4_auto_step = 1050;
							}
							else if (haswaferpm && haswaferarm1 && !haswaferarm2) {//手2先取，手1后放
								pm4_auto_step = 1070;
							}
							else if (haswaferpm && !haswaferarm2 && !haswaferarm1) {//两个手臂没料，A手取
								pm4_auto_step = 1090;
							}

							else {
								logFailedExcuteCommandHasError(wtr->getName(), "机械手晶圆状态不对", pm4_process_name, pm4_auto_step);
							}
#else
							pm4_auto_step.store(1010);
#endif
						}
						else
						{
							logFailedNotNormal(wtr->getName(), pm4_process_name, pm4_auto_step);
						}
						//没加，导致出错
						//pm4_auto_step.store(1010);

					}
					break;
					case 1010:
					{//放片
						if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
#ifdef DEBUG_TEST_PM
								robot_selected_arm = 0; //A手
								auto cmd = wtr->createPutCommand(pm4, robot_selected_arm, 1);
								wtr->startCommand(cmd);
								cmd->wait();
								if (cmd->hasError())
								{
									logFailedExcuteCommandHasError(wtr->getName(), "放晶圆", pm4_process_name, pm4_auto_step.load());
								}
								else
								{
									//下层的
									//taskManager.updateTaskStatus(pmPendingTasks.at(0).taskId,UnifiedWaferTask::TaskType::PM_PROCESS, UnifiedWaferTask::Status::IN_PROGRESS);
									pm4_allow_get_put_wafer = false;
									pm4_auto_step.store(2000);
								}
#else
							//TODO:测试阶段，机械手不去PM放晶圆
							logInform1(wtr->getName().c_str(), "测试阶段，机械手不去PM放晶圆.");
							pm4_auto_step.store(2000);

#endif  //DEBUG_TEST_PM

						}
						else
						{
							logFailedNotNormal(wtr->getName(), pm4_process_name, pm4_auto_step);
						}
					}
					break;
					case 1030:
					{//放片
						if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{

#ifdef DEBUG_TEST_PM
							robot_selected_arm = 1;
							auto cmd = wtr->createPutCommand(pm4, robot_selected_arm, 1);
							wtr->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError())
							{
								logFailedExcuteCommandHasError(wtr->getName(), "放晶圆", pm4_process_name, pm4_auto_step);
							}
							else
							{
								//下层的
								//taskManager.updateTaskStatus(pmPendingTasks.at(0).taskId, UnifiedWaferTask::TaskType::PM_PROCESS, UnifiedWaferTask::Status::IN_PROGRESS);

								pm4_allow_get_put_wafer = false;
								pm4_auto_step.store(2000);
							}
#else
							logInform1(wtr->getName().c_str(), "测试阶段，机械手不去PM放晶圆.");
							pm4_auto_step.store(2000);
#endif
						}
						else
						{
							logFailedNotNormal(wtr->getName(), pm4_process_name, pm4_auto_step);
						}
					}
					break;
					case 1040:
					{
						//取片
						if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
							robot_selected_arm = 0;
							auto cmd = wtr->createGetCommand(pm4, robot_selected_arm, 1);
							wtr->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError())
							{
								logFailedExcuteCommandHasError(wtr->getName(), "取晶圆", pm4_process_name, pm4_auto_step);
							}
							else
							{
								//下层的
								taskManager.updateTaskStatus(pm4PendingTasks.at(0).taskId, UnifiedWaferTask::TaskType::PM_PROCESS, UnifiedWaferTask::Status::COMPLETED);
								taskManager.updateTaskStatus(pm4PendingTasks.at(0).taskId, UnifiedWaferTask::TaskType::LOADLOCK_RETURN, UnifiedWaferTask::Status::QUEUED);

								pm4_allow_get_put_wafer = false;
								pm4_auto_step.store(10);
							}
						}
						else
						{
							logFailedNotNormal(wtr->getName(), pm4_process_name, pm4_auto_step);
						}
					}
					break;
					case 1050:
					{
						if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
							robot_selected_arm = 1;
							auto cmd = wtr->createGetCommand(pm4, robot_selected_arm, 1);
							wtr->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError())
							{
								logFailedExcuteCommandHasError(wtr->getName(), "取晶圆", pm4_process_name, robot_auto_step);
							}
							else
							{
								//下层的
								taskManager.updateTaskStatus(pm4PendingTasks.at(0).taskId, UnifiedWaferTask::TaskType::PM_PROCESS, UnifiedWaferTask::Status::COMPLETED);
								taskManager.updateTaskStatus(pm4PendingTasks.at(0).taskId, UnifiedWaferTask::TaskType::LOADLOCK_RETURN, UnifiedWaferTask::Status::QUEUED);
								pm4_allow_get_put_wafer = false;
								pm4_auto_step.store(10);
							}
						}
						else
						{
							logFailedNotNormal(wtr->getName(), pm4_process_name, robot_auto_step);
						}
					}
					break;
#pragma region 交换料
					case 1060:
					{
						if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
							robot_selected_arm = 0;
							auto cmd = wtr->createGetCommand(pm4, robot_selected_arm, 1);
							wtr->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError())
							{
								logFailedExcuteCommandHasError(wtr->getName(), "取晶圆", pm4_process_name, pm4_auto_step.load());
							}
							else {
								pm4CompletedTasks = taskManager.getPMCompletedTasks("PM4"); //10
								taskManager.updateTaskStatus(pm4CompletedTasks.at(0).taskId, UnifiedWaferTask::TaskType::LOADLOCK_RETURN, UnifiedWaferTask::Status::QUEUED);//7
								pm4_auto_step.store(1061);
							}
						}
						else
						{
							logFailedNotNormal(wtr->getName(), pm4_process_name, pm4_auto_step.load());
						}
					}
					break;
					case 1061:
					{
						if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
							robot_selected_arm = 1;
							auto cmd = wtr->createPutCommand(pm4, robot_selected_arm, 1);
							wtr->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError())
							{
								logFailedExcuteCommandHasError(wtr->getName(), "放晶圆", pm4_process_name, pm4_auto_step.load());
							}
							else
							{
								pm4_allow_get_put_wafer = false;
								pm4_allow_goto_craft = true;
								pm4_auto_step.store(10);
							}
						}
						else
						{
							logFailedNotNormal(wtr->getName(), pm_process_name, pm1_auto_step.load());
						}
					}
					break;
					case 1070:
					{
						if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
							robot_selected_arm = 1;
							auto cmd = wtr->createGetCommand(pm4, robot_selected_arm, 1);
							wtr->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError())
							{
								logFailedExcuteCommandHasError(wtr->getName(), "取晶圆", pm4_process_name, pm4_auto_step.load());
							}
							else {

								pm4CompletedTasks = taskManager.getPMCompletedTasks("PM4"); //10
								taskManager.updateTaskStatus(pm4CompletedTasks.at(0).taskId, UnifiedWaferTask::TaskType::LOADLOCK_RETURN, UnifiedWaferTask::Status::QUEUED);//7
								//可以呼叫loadlocka/loadlockb 走放片到loadlock的流程
								pm4_auto_step.store(1071);
							}

						}
						else
						{
							logFailedNotNormal(wtr->getName(), pm4_process_name, pm4_auto_step.load());
						}
					}
					break;
					case 1071:
					{
						if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
							robot_selected_arm = 0;
							auto cmd = wtr->createPutCommand(pm4, robot_selected_arm, 1);
							wtr->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError())
							{
								logFailedExcuteCommandHasError(wtr->getName(), "放晶圆", pm4_process_name, pm4_auto_step.load());
							}
							else
							{
								pm4_allow_get_put_wafer = false;
								pm4_allow_goto_craft = true; //转工艺
								pm4_auto_step.store(10);
							}
						}
						else
						{
							logFailedNotNormal(wtr->getName(), pm4_process_name, pm4_auto_step.load());
						}
					}
					break;
#pragma endregion
					case 1090:
					{
						if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
							robot_selected_arm = 0;
							auto cmd = wtr->createGetCommand(pm4, robot_selected_arm, 1);
							wtr->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError())
							{
								logFailedExcuteCommandHasError(wtr->getName(), "取晶圆", pm4_process_name, pm4_auto_step.load());
							}
							else
							{
								pm4_allow_get_put_wafer = false;
								pm4CompletedTasks = taskManager.getPMCompletedTasks("PM4"); //10
								taskManager.updateTaskStatus(pm4CompletedTasks.at(0).taskId, UnifiedWaferTask::TaskType::LOADLOCK_RETURN, UnifiedWaferTask::Status::QUEUED);//7
								pm4_auto_step.store(10);
							}
						}
						else
						{
							logFailedNotNormal(wtr->getName(), pm4_process_name, pm4_auto_step.load());
						}
					}
					break;

					case 2000:
					{
						Sleep(500);
						UpdatePmSubTransferDatas("PM4");
						

#ifdef DEBUG_TEST_PM
						auto PmInstance = QPmRecipeWidget::instance(kernel);
						PmInstance->startPmMotorRun(4);

#else
						logInform("PM4", "模拟做工艺流程.....");
#endif

						if(pm4PendingTasks.size() >0 )
						{
							taskManager.updateTaskStatus(pm4PendingTasks.at(0).taskId, UnifiedWaferTask::TaskType::PM_PROCESS, UnifiedWaferTask::Status::COMPLETED);
							taskManager.updateTaskStatus(pm4PendingTasks.at(0).taskId, UnifiedWaferTask::TaskType::LOADLOCK_RETURN, UnifiedWaferTask::Status::QUEUED);//7
							pm4_auto_step.store(10);
						}
						else
						{
							pm4_auto_step.store(10);
							Sleep(10);
						}
					}
					break;
					case 2001:
					{
						//pmCompletedTasks = taskManager.getPMCompletedTasks("PM4"); //10
						UpdatePmSubTransferDatas("PM4");
						if(pm4CompletedTasks.size() >0 )
						{
							taskManager.updateTaskStatus(pm4CompletedTasks.at(0).taskId, UnifiedWaferTask::TaskType::LOADLOCK_RETURN, UnifiedWaferTask::Status::QUEUED);//7
							pm4_allow_get_put_wafer = false;
							Sleep(10);
							pm4_auto_step.store(10);
						}
						else
						{
							pm4_auto_step.store(10);
							Sleep(10);
						}
					}
					default:
						break;
					}

				
					pm4_step_once_finished = true;
				}
		}
		catch (const std::exception& e) {
			logError("Cyclelog", "PM4Transfer thread crashed:", e.what());
			qCritical() << "PM4Transfer thread crashed:" << e.what();
		}
		catch (...) {
			logError("Cyclelog", "PM4Transfer thread crashed: unknown exception");
			qCritical() << "PM4Transfer thread crashed: unknown exception";
		}



	}

	void QSlotTransferCycleVTMWidgetPrivate::executeTMTransfer()
	{

	}

	void QSlotTransferCycleVTMWidgetPrivate::executeUpdateTransferStatus()
	{
		try {

			elp1 = kernel->getKernelModule<EFEMLPSubsystem>("ELP1");
			elp2 = kernel->getKernelModule<EFEMLPSubsystem>("ELP2");

			while (!stopRequested)
			{
				update_step_once_finished = false;
				Sleep(500);
				//onGetStep();
				
				if (((finished_time_lla == cycle_times_lla) || (finished_time_llb == cycle_times_llb)) 
					&& (!elp1->hasDoorOpend() && !elp2->hasDoorOpend()))
				{
					
					//update lla
					cycleFinished_lla = true;
					onUpdateCycleInfo();
					lp1_cycle_one_time_finished = false;

					//update llb
					cycleFinished_llb = true;
					onUpdateCycleInfo();
					lp2_cycle_one_time_finished = false;

					//update step
					loadlock1_auto_step = 10;
					loadlock2_auto_step = 10;
					vacuum_auto_step = 10;
					efem_auto_step = 10;
					pm1_auto_step.store(10);
					pm2_auto_step.store(10);
					pm3_auto_step.store(10);
					pm4_auto_step.store(10);
					update_auto_step = 10;


					running = false; //阻塞，此时EFEM线程下料完成，wait case 201处！！
					//pauseAllThreads();

					logInform("Cycle", "整机流程结束.");
				}

				{
					std::unique_lock<std::mutex> lock(mtx);
					cv.wait(lock, [this] { return running.load() || stopRequested.load(); });
				}

				if (stopRequested) break;


				switch (update_auto_step)
				{
				case 10:
				{
					UpdateEfemSubTransferDatas();
					Sleep(500);
					
					if (lp1_cycle_one_time_finished && !cycleFinished_lla)
					{//Lp1的一次Cycle已做完
						//2025-11-17: 多加一条检测条件：
						if (efemReturnCompletedTasks.size() == originTaskSize && (originTaskSize > 0))
						{
							update_auto_step = 1030;
						}
					}
					else if (lp2_cycle_one_time_finished && !cycleFinished_llb)
					{//Lp2的一次Cycle已做完
						if (efemReturnCompletedTasks.size() == originTaskSize && (originTaskSize > 0))
						{
							update_auto_step = 1040;
						}
					}
					else {
						Sleep(500);
					}
				}
				break;
				case 1030:
				{
					finished_time_lla++;

					logInform("Cycle", Poco::format("LP1 Cycle第%d次数.", finished_time_lla).c_str());
					if (is_lp1_cycle)
					{
						is_lp1_cycle = false;
						current_lp_cycle = false;
					}
					if (finished_time_lla >= cycle_times_lla)
					{
						//cycle 完成
						logInform("Cycle", Poco::format("LP1 Cycle次数%d已完成.", cycle_times_lla).c_str());
						//finished_time_lla = 0;
						cycleFinished_lla = true;
						onUpdateCycleInfo();
						lp1_cycle_one_time_finished = false;

					}
					else
					{
						// 4---> 0
						efemReturnCompletedTasks = taskManager.getEfemRuturnCompletedTasks();//4
						for (auto& task : efemReturnCompletedTasks)
						{
							taskManager.updateTaskStatus(task.taskId, UnifiedWaferTask::TaskType::UNKNOWN, UnifiedWaferTask::Status::UNKNOWN_PROGRESS);
						}
						taskManager.lessTaskIdSortAlgorithm();
					}
					onUpdateCycleInfo();
					lp1_cycle_one_time_finished = false;

					RerunTheFilmCycle();
				
				}
				break;
				case 1040:
				{
					finished_time_llb++;
					logInform("Cycle", Poco::format("LP2 Cycle第%d次数.", finished_time_llb).c_str());

					if (is_lp2_cycle)
					{
						is_lp2_cycle = false;
						current_lp_cycle = false;
					}
					if (finished_time_llb >= cycle_times_llb)
					{
						logInform("Cycle", Poco::format("LP2 Cycle次数%d已完成.", cycle_times_llb).c_str());

						//finished_time_llb = 0;
						cycleFinished_llb = true;

						onUpdateCycleInfo();
						lp2_cycle_one_time_finished = false;
						//if (!elp1->hasDoorOpend() && !elp2->hasDoorOpend())
						//{
						//	taskManager.stopped_ = true;
						//}
						//break;
					}
					else
					{
						efemReturnCompletedTasks = taskManager.getEfemRuturnCompletedTasks();//4
						for (auto& task : efemReturnCompletedTasks)
						{
							taskManager.updateTaskStatus(task.taskId, UnifiedWaferTask::TaskType::UNKNOWN, UnifiedWaferTask::Status::UNKNOWN_PROGRESS);
						}
						taskManager.lessTaskIdSortAlgorithm();
					}
					onUpdateCycleInfo();
					lp2_cycle_one_time_finished = false;

					RerunTheFilmCycle();
				}
				break;
				default:
					break;
					update_step_once_finished = true;
					Sleep(10);
				}
				
			}
		}
		catch (const std::exception& e) {
			logError("Cyclelog", "UpdateTransfer thread crashed:%s", e.what());
			qCritical() << "UpdateTransfer thread crashed:" << e.what();
			
		}
		catch (...) {
			logError("Cyclelog", "UpdateTransfer thread crashed: unknown exception");
			qCritical() << "UpdateTransfer thread crashed: unknown exception";
		}
	}

	void QSlotTransferCycleVTMWidgetPrivate::RerunTheFilmCycle()
	{
		//running = false; //不停
		pauseAllThreads(); //暂停，只会阻塞所有线程，不退出

		logInform(reset_process_name.c_str(), "重新跑片开始.");
		loadlock1_auto_step = 10;
		loadlock2_auto_step = 10;
		vacuum_auto_step = 10;
		efem_auto_step = 10;
		pm1_auto_step.store(10);
		pm2_auto_step.store(10);
		pm3_auto_step.store(10);
		pm4_auto_step.store(10);
		//重置标志
		needReset_EFEM.store(true);
		needReset_LLA.store(true);
		needReset_LLB.store(true);
		needReset_PM1.store(true);
		needReset_PM2.store(true);
		needReset_PM3.store(true);
		needReset_PM4.store(true);
		needReset_Update.store(true);
		update_auto_step = 10;
		Sleep(500);

		startAllThreads(); //重新唤醒
		//running = true;

		
	}

	void QSlotTransferCycleVTMWidgetPrivate::startAllThreads()
	{
		{
			std::lock_guard<std::mutex> lock(mtx);
			running = true;
			ispause = false;//add
		}
		cv.notify_all();
	}

	void QSlotTransferCycleVTMWidgetPrivate::pauseAllThreads()
	{
		std::shared_ptr<FortrendPumpSubsystem> pump = kernel->getKernelModule<FortrendPumpSubsystem>("PUMP");
		{
			std::lock_guard<std::mutex> lock(mtx);
			running = false; 
			ispause = true;//add

			//2026-1-20 暂停所有的pm电机运动
			auto PmInstance = QPmRecipeWidget::instance(kernel);
			for (int i = 0; i < 4; i++)
			{
				PmInstance->stopPmMotor(i);
			}
			
		}
		cv.notify_all();  // 2025-10-28 :唤醒所有等待的线程，让它们重新检查running状态，实现暂停
	}

	void QSlotTransferCycleVTMWidgetPrivate::stopAllThreads()
	{
		std::shared_ptr<FortrendPumpSubsystem> pump = kernel->getKernelModule<FortrendPumpSubsystem>("PUMP");
		{
			std::lock_guard<std::mutex> lock(mtx);
			running = false;
			stopRequested = true;
			ispause = true;//add
		}
		cv.notify_all();
		
		if (pump != nullptr)
			pump->setProcessAbort(true);//停止所有模组抽真空/破真空流程

		// Wait for threads to finish
		for (auto& thread : threads) {
			if (thread.joinable()) {
				thread.join();
			}
		}
		threads.clear();
	}

	bool QSlotTransferCycleVTMWidgetPrivate::isLoadingInterlock(const std::string &LLName)
	{
		//logInform("check Interlock","检查LLName:%s 上料互锁.",LLName.c_str());

		UpdateLLBSubTransferDatas();
		UpdateLLASubTransferDatas();			
		lk1 = kernel->getKernelModule<FortrendLoadLockSubsystem>("LLA");
		lk2 = kernel->getKernelModule<FortrendLoadLockSubsystem>("LLB");
		bool downHaswaferlk1 = false;
		bool downHaswaferlk2 = false;
		if(lk1!=nullptr && lk2!=nullptr)
		{
			auto cass1Manager = lk1->getKernel()->getKernelModule<FortrendCassetteManager>();
			auto cass2Manager = lk2->getKernel()->getKernelModule<FortrendCassetteManager>();
			downHaswaferlk1 = cass1Manager->getCassette(lk1.get())->getMapping(1) == Cassette::Present; //1层有片
			downHaswaferlk2 = cass1Manager->getCassette(lk2.get())->getMapping(1) == Cassette::Present; //1层有片
		}
		else
		{
			logError("Cyclelog", "isLoadingInterlock  crashed!");
			return false;
		}

		//获得经过LLA的所有料集合，
		if(LLName == "LLB")
		{
			//1.下料请求，获得经过LLA 的有上料标签，或下层有片
			if (tool_allow_put_wafer_LLA || taskManager.CollectionPassedThroughLL("LLA") || downHaswaferlk1)
			{
				//logWarn("Cyclelog", "LLA有待下料或者还没下料,LLB上料 LoadingInterlock!");
				return true; //LLA有待下料或者还没下料
			}
			// LLA下料完成 
			else if (!taskManager.CollectionPassedThroughLL("LLA") && !tool_allow_put_wafer_LLA)
			{
				//LLA下料完成或者初始状态
				//logWarn("Cyclelog", "LLA下料完成,对LLB上料不上锁.");
				return false;
			}
			else
			{
				logWarn("Cyclelog", "未知情况,对LLB上料不上锁.");
				return false;
			}
		}
		else if (LLName == "LLA")
		{
			//下料请求,获得经过LLB的有上料标签，或下层有片
			if (tool_allow_put_wafer_LLB || taskManager.CollectionPassedThroughLL("LLB") || downHaswaferlk2)//或下层有片
			{
				//logWarn("Cyclelog", "LLB有待下料或者还没下料,LLA 上料 LoadingInterlock!");
				return true; //LLB有待下料或者还没下的料
			}
			//true呼叫LP下料， false LP下料完成 
			else if (!taskManager.CollectionPassedThroughLL("LLB") && !tool_allow_put_wafer_LLB)
			{
				//LLA下料完成或者初始状态
				//logWarn("Cyclelog", "LLB下料完成,对LLA上料不上锁.");
				return false;
			}
			else
			{
				logWarn("Cyclelog", " 未知情况,对LLA上料不上锁.");
				return false;
			}
		}
		else
		{
			return false;
		}
		return false;
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
		std::shared_ptr<FortrendLoadLockSubsystem> lk = nullptr;

		std::chrono::steady_clock::duration elapsed;
		bool loadlock_get_vacuum = false;
		if (llName == "LLA")
		{
			lk = lk1;
			//loadlock1_get_vacuum = loadlock_get_vacuum;
		}
		else if (llName == "LLB")
		{
			lk = lk2;
			//loadlock2_get_vacuum = loadlock_get_vacuum;
		}
		else
		{
			logError("cycle", "llName:%s 输入参数不是LL", llName.c_str());
			return false;
		}
		if (lk == nullptr)
		{
			return false;
		}
		if (ui->enableAtmosphere->checkState() == Qt::CheckState::Checked)
		{
			logInform("cycle", "大气模式，lk和tm腔室不检测极限真空值!");
			return true;
		}

		robot_auto_step = preStep;

		while (!stopRequested)
		{

			{
				std::unique_lock<std::mutex> lock(mtx);
				cv.wait(lock, [this] { return running.load() || stopRequested.load(); });
			}

			if (stopRequested) break;

			auto now_time = std::chrono::steady_clock::now();
			if (llName == "LLA")
			{
				elapsed = now_time - LLA_start_time;
			}
			else
			{
				elapsed = now_time - LLB_start_time;
			}
			
			if (elapsed >= std::chrono::minutes(30))
			{
				logError("cycle", "sub:%s,检测LL真空和TM真空超时.", llName.c_str());
				return false;
			}

			switch (robot_auto_step)
			{
			case 2010:
			{
				if (lk->getVacuumValueUpperLimitReachesTheSetValue())
				{
					robot_auto_step = 2050;
				}
				else
				{
					if (llName == "LLA")
					{
						loadlock1_get_vacuum = true;
					}
					else if (llName == "LLB")
					{
						loadlock2_get_vacuum = true;

					}
					robot_auto_step = 2020;
				}
			}
			break;
			case 2020:
			{
				if (llName == "LLA")
				{
					if (loadlock1_get_vacuum == false)
					{
						robot_auto_step = 2010;
					}
					else {
						Sleep(100);
					}
				}
				else if (llName == "LLB")
				{
					if (loadlock2_get_vacuum == false)
					{
						robot_auto_step = 2010;
					}
					else {
						Sleep(100);
					}
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
					logInform("cycle", "lk:%s,tm:%s 腔室都达到了极限真空值!", lk->getName().c_str(),tm->getName().c_str());
					logInform("cycle", Poco::format("当前压力值：%f", lk->getVacuumValue()).c_str());
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

		}
		return false;
	}


	void QSlotTransferCycleVTMWidgetPrivate::resetAction(){

		if (running)
		{
			return;
		}
		efem_robot_mutex.unlock();

		finished_time_lla = 0;

		finished_time_llb = 0;
		
		while (!vacumm_step_once_finished || !loadlock1_step_once_finished || !loadlock2_step_once_finished)
		{
			Sleep(100);
		}
		onUpdateControlEnabled("reset_pbt", false);
		onUpdateControlEnabled("execute_pbt", false);
		onUpdateControlEnabled("pause_pbt", false);
		onUpdateLightButtonStatus("light_reset_pbt",2);
		onUpdateLightButtonStatus("light_running_pbt", 1);

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
		std::shared_ptr<FortrendPumpSubsystem> pump = kernel->getKernelModule<FortrendPumpSubsystem>("PUMP");

		//check modules
		auto cassManager = wtr->getKernel()->getKernelModule<FortrendCassetteManager>();

		//把真空流程置位
		if (pump->getProcessAbort())
		{
			pump->setProcessAbort(false);
		}

		bool reset_loop = true;
		reset_finish = false;
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
					logInform(reset_process_name.c_str(), "整机复位开始.");
					//wtr
					auto cmd = wtr->createResetCommand();
					wtr->startCommand(cmd);
					cmd->wait();
					//ewtr
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
					
					//加home，为了复位关腔门的安全信号
					auto home_cmd = wtr->createHomeCommand();
					wtr->startCommand(home_cmd);
					home_cmd->wait();

					if (cmd->hasError() || home_cmd->hasError())
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
					logError(reset_process_name.c_str(), Poco::format("%s处于未知状态.", wtr->getName()).c_str());
				}

			}
			break;

			case 100:
			{
				if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL &&
					(ewtr->getState() == IKernelSubSystem::State::SUB_NORMAL || ui->disabledefem->checkState() == Qt::CheckState::Checked) &&
					lk1->getState() != IKernelSubSystem::State::SUB_UNKNOWN &&
					lk2->getState() != IKernelSubSystem::State::SUB_UNKNOWN &&
					pump->getState()!= IKernelSubSystem::State::SUB_UNKNOWN &&
					pm1->getState() != IKernelSubSystem::State::SUB_UNKNOWN &&
					pm2->getState() != IKernelSubSystem::State::SUB_UNKNOWN &&
					pm3->getState() != IKernelSubSystem::State::SUB_UNKNOWN &&
					pm4->getState() != IKernelSubSystem::State::SUB_UNKNOWN &&
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
					pm4->startCommand(cmd_pm4);

					auto cmd_tm = tm->createResetCommand();
					tm->startCommand(cmd_tm);

					auto cmd_pump = pump->createResetCommand();
					pump->startCommand(cmd_pump);

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
					cmd_pump->wait();

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
					else if (cmd_pump->hasError())
					{
						rest_step = 15000;
						logError(reset_process_name.c_str(), Poco::format("%s复位指令执行失败！", pump->getName()).c_str());
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
						logError(reset_process_name.c_str(), Poco::format("%s不在正常状态.", wtr->getName()).c_str());
					}
					if (lk1->getState() == IKernelSubSystem::State::SUB_UNKNOWN)
					{
						logError(reset_process_name.c_str(), Poco::format("%s不在正常状态.", lk1->getName()).c_str());
					}
					if (lk2->getState() == IKernelSubSystem::State::SUB_UNKNOWN)
					{
						logError(reset_process_name.c_str(), Poco::format("%s不在正常状态.", lk2->getName()).c_str());
					}
					if (pm1->getState() == IKernelSubSystem::State::SUB_UNKNOWN)
					{
					logError(reset_process_name.c_str(), Poco::format("%s不在正常状态.", pm1->getName()).c_str());
					}
					if (pm2->getState() == IKernelSubSystem::State::SUB_UNKNOWN)
					{
						logError(reset_process_name.c_str(), Poco::format("%s不在正常状态.", pm2->getName()).c_str());
					}
					if (pm3->getState() == IKernelSubSystem::State::SUB_UNKNOWN)
					{
					logError(reset_process_name.c_str(), Poco::format("%s不在正常状态.", pm3->getName()).c_str());
					}
					if (pm4->getState() == IKernelSubSystem::State::SUB_UNKNOWN)
					{
						logError(reset_process_name.c_str(), Poco::format("%s不在正常状态.", pm4->getName()).c_str());
					}
					if (pump->getState() == IKernelSubSystem::State::SUB_UNKNOWN)
					{
						logError(reset_process_name.c_str(), Poco::format("%s不在正常状态.", pump->getName()).c_str());
					}
					if (tm->getState() == IKernelSubSystem::State::SUB_UNKNOWN)
					{
						logError(reset_process_name.c_str(), Poco::format("%s不在正常状态.", tm->getName()).c_str());
					}
					if (elp1->getState() == IKernelSubSystem::State::SUB_UNKNOWN)
					{
						logError(reset_process_name.c_str(), Poco::format("%s不在正常状态.", elp1->getName()).c_str());
					}
					if (elp2->getState() == IKernelSubSystem::State::SUB_UNKNOWN)
					{
						logError(reset_process_name.c_str(), Poco::format("%s不在正常状态.", elp2->getName()).c_str());
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
					pump->getState() == IKernelSubSystem::State::SUB_NORMAL &&
					tm->getState() == IKernelSubSystem::State::SUB_NORMAL&&
					(ewtr->getState() == IKernelSubSystem::State::SUB_NORMAL || ui->disabledefem->checkState() == Qt::CheckState::Checked) &&
					(elp1->getState() == IKernelSubSystem::State::SUB_NORMAL || ui->disabledefem->checkState() == Qt::CheckState::Checked) &&
					(elp2->getState() == IKernelSubSystem::State::SUB_NORMAL || ui->disabledefem->checkState() == Qt::CheckState::Checked)
					)
				{
					//rest_step = 200;
					rest_step = 10000;

				}
				else
				{
					if (lk1->getState() != IKernelSubSystem::State::SUB_NORMAL)
					{
						logError(reset_process_name.c_str(), Poco::format("%s不在正常状态.", lk1->getName()).c_str());
					}

					if (lk2->getState() != IKernelSubSystem::State::SUB_NORMAL)
					{
						logError(reset_process_name.c_str(), Poco::format("%s不在正常状态.", lk2->getName()).c_str());
					}
					if (pm1->getState() != IKernelSubSystem::State::SUB_NORMAL)
					{
					logError(reset_process_name.c_str(), Poco::format("%s不在正常状态.", pm1->getName()).c_str());
					}
					if (pm2->getState() != IKernelSubSystem::State::SUB_NORMAL)
					{
						logError(reset_process_name.c_str(), Poco::format("%s不在正常状态.", pm2->getName()).c_str());
					}
					if (pm3->getState() != IKernelSubSystem::State::SUB_NORMAL)
					{
					logError(reset_process_name.c_str(), Poco::format("%s不在正常状态.", pm3->getName()).c_str());
					}
					if (pm4->getState() != IKernelSubSystem::State::SUB_NORMAL)
					{
						logError(reset_process_name.c_str(), Poco::format("%s不在正常状态.", pm4->getName()).c_str());
					}
					if (pump->getState() != IKernelSubSystem::State::SUB_NORMAL)
					{
						logError(reset_process_name.c_str(), Poco::format("%s不在正常状态.", pump->getName()).c_str());
					}
					if (tm->getState() != IKernelSubSystem::State::SUB_NORMAL)
					{
						logError(reset_process_name.c_str(), Poco::format("%s不在正常状态.", tm->getName()).c_str());
					}
					if (ewtr->getState() != IKernelSubSystem::State::SUB_NORMAL)
					{
						logError(reset_process_name.c_str(), Poco::format("%s不在正常状态.", ewtr->getName()).c_str());
					}
					if (elp1->getState() != IKernelSubSystem::State::SUB_NORMAL)
					{
						logError(reset_process_name.c_str(), Poco::format("%s不在正常状态.", elp1->getName()).c_str());
					}
					if (elp2->getState() != IKernelSubSystem::State::SUB_NORMAL)
					{
						logError(reset_process_name.c_str(), Poco::format("%s不在正常状态.", elp2->getName()).c_str());
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
				logInform(reset_process_name.c_str(), "整机复位完成.");
				/*robot_auto_step = 10;*/
				loadlock1_auto_step = 10;
				loadlock2_auto_step = 10;
				vacuum_auto_step = 10;
				efem_auto_step = 10;
				pm1_auto_step.store(10);
				pm2_auto_step.store(10);
				pm3_auto_step.store(10);
				pm4_auto_step.store(10);
				update_auto_step = 10;
				reset_loop = false;
				reset_finish = true;
				current_lp_cycle = false;  //2025-11-17  重置
				rest_step = -1;
				
			}
			break;
			case 15000:
			{
				logInform(reset_process_name.c_str(), "整机复位失败.");
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

			onUpdateLightButtonStatus("light_reset_pbt", 3);
			onUpdateLightButtonStatus("light_running_pbt", 1);

			//reset_finish = false;

#ifndef CYCLE_SIM_MODE
				//刷新wafer数据回原始的状态
				int taskSize = taskManager.getAllTasksSize();
				auto tasks = taskManager.getAllTasks();

				if (taskSize > 0)
				{
					for (int i = 0; i < taskSize; i++)
					{
						taskManager.updateTaskStatus(tasks[i].taskId, UnifiedWaferTask::TaskType::UNKNOWN, UnifiedWaferTask::Status::UNKNOWN_PROGRESS);
					}
				}
#endif

		}
		onUpdateControlEnabled("reset_pbt", true);

	}

	void QSlotTransferCycleVTMWidgetPrivate::startVacuumAction(){
		std::shared_ptr<FortrendTMCavitySubsystem> tm = kernel->getKernelModule<FortrendTMCavitySubsystem>("TM");
		std::shared_ptr<FortrendPumpSubsystem> pump = kernel->getKernelModule<FortrendPumpSubsystem>("PUMP");
		std::shared_ptr<FortrendLoadLockSubsystem> lk1 = kernel->getKernelModule<FortrendLoadLockSubsystem>("LLA");
		std::shared_ptr<FortrendLoadLockSubsystem> lk2 = kernel->getKernelModule<FortrendLoadLockSubsystem>("LLB");

		while (!stopRequested)
		{
			
			{
				std::unique_lock<std::mutex> lock(mtx);
				cv.wait(lock, [this] { return running.load() || stopRequested.load(); });
			}

			if (stopRequested) break;

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
						else if (loadlock1_get_vacuum && !loadlock2_get_vacuum)//新增LLA、LLB的抽真空顺序判断
						{
							vacuum_auto_step = 10000;
						}
						else if (loadlock2_get_vacuum && !loadlock1_get_vacuum)
						{
							vacuum_auto_step = 20000;
						}
						else if (loadlock1_get_vacuum && loadlock2_get_vacuum && lk1->getVacuumValue() < lk2->getVacuumValue()){
							vacuum_auto_step = 10000;
						}
						else if (loadlock1_get_vacuum && loadlock2_get_vacuum && lk1->getVacuumValue() > lk2->getVacuumValue()){
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


	void QSlotTransferCycleVTMWidgetPrivate::setLoadLock1PutCassetteFinished()
	{
		loadlock1_put_cassette_finished = true;
	}

	void QSlotTransferCycleVTMWidgetPrivate::setLoadLock2PutCassetteFinished(){
		loadlock2_put_cassette_finished = true;
	}

	void QSlotTransferCycleVTMWidgetPrivate::onGetStep()
	{
		Sleep(500);
		logInform("Cycle", Poco::format("%s = %d", loadlock1_process_name, loadlock1_auto_step).c_str());
		logInform("Cycle", Poco::format("%s = %d", loadlock2_process_name, loadlock2_auto_step).c_str());
		logInform("Cycle", Poco::format("%s = %d", robot_process_name, robot_auto_step).c_str());
		logInform("Cycle", Poco::format("%s = %d", vacuum_process_name,vacuum_auto_step).c_str());
		logInform("Cycle", Poco::format("%s = %d", efem_process_name, efem_auto_step).c_str());
		logInform("Cycle", Poco::format("%s = %d", pm1_process_name, pm1_auto_step.load()).c_str());
		logInform("Cycle", Poco::format("%s = %d", pm2_process_name, pm2_auto_step.load()).c_str());
		logInform("Cycle", Poco::format("%s = %d", pm3_process_name, pm3_auto_step.load()).c_str());
		logInform("Cycle", Poco::format("%s = %d", pm4_process_name, pm4_auto_step.load()).c_str());
		logInform("Cycle", Poco::format("%s = %d", update_process_name, update_auto_step).c_str());
	}

	bool QSlotTransferCycleVTMWidgetPrivate::setHLTransferSequence()
	{
		taskManager.clearTasks();

		int llaSlot = 1;  // LLA当前槽位
		int llbSlot = 1;  // LLB当前槽位
		int slots_ = 0;

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


		if (currentTransferMode == FilmTransferMode::Formula_Double_Up_And_Down)
		{
			slots_ = 2;
		}
		else if(currentTransferMode == FilmTransferMode::Formula_Go_Up_And_Down)
		{
			slots_ = 1;
		}
		else
		{
			logInform("Cycle:", "当前传片模式配置错误.");
			return false;
		}

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

			task.selectPmEnableList[0] = task.pm1Enabled; // 隐式转换：true->1, false->0
			task.selectPmEnableList[1] = task.pm2Enabled;
			task.selectPmEnableList[2] = task.pm3Enabled;
			task.selectPmEnableList[3] = task.pm4Enabled;

			// 轮换式LoadLock分配
			const int GROUP_SIZE = slots_; //可配置 1,2
			int groupIndex = i / GROUP_SIZE;

			if(GROUP_SIZE == 2)
			{
				//2. 确定目标LoadLock：偶数组->LLA，奇数组->LLB 
				if (groupIndex % 2 == 0)
				{
					task.target = UnifiedWaferTask::Location::LLA;
					task.target_pm = getSelectPmLocation(task);
					task.targetBlankingSlot = task.targetFeedingSlot = llaSlot++;

					//setWaferId
					//const std::string& id = std::to_string(task.taskId);
					//lk1_cass->setWaferId(task.targetBlankingSlot, id);

					if (llaSlot > GROUP_SIZE) llaSlot = 1;// 组内循环
				}
				else
				{
					task.target = UnifiedWaferTask::Location::LLB;
					task.target_pm = getSelectPmLocation(task);
					task.targetBlankingSlot = task.targetFeedingSlot = llbSlot++;

					//setWaferId
					//const std::string& id = std::to_string(task.taskId);
					//lk2_cass->setWaferId(task.targetBlankingSlot, id);
					if (llbSlot > GROUP_SIZE) llbSlot = 1;// 组内循环

				}
			}
			else
			{
				if (groupIndex % 2 == 0)
				{
					task.target = UnifiedWaferTask::Location::LLA;
					task.target_pm = getSelectPmLocation(task);
					llaSlot = 2;
					task.targetFeedingSlot = llaSlot;//上料槽号，上层
					task.targetBlankingSlot = 1;     //下料槽号，下层
					if (llaSlot > GROUP_SIZE) llaSlot = 2;// 组内循环
				}
				else
				{
					task.target = UnifiedWaferTask::Location::LLB;
					task.target_pm = getSelectPmLocation(task);
					llbSlot = 2;
					task.targetFeedingSlot = llbSlot;//上料槽号，上层
					task.targetBlankingSlot = 1;     //下料槽号，下层
					if (llbSlot > GROUP_SIZE) llbSlot = 2;// 组内循环
				}
			}

			taskManager.addTask(task);//加入到管理者

			// 调试日志
			logInform("TransferSetup", "Row %d: Source %s Group %d -> %s FeedingSlot %d  BlankingSlot  %d taskID %d  Status %s  %s ",
				i, 
				task.locationToString(task.source), 
				groupIndex,
				(task.target == UnifiedWaferTask::Location::LLA) ? "LLA" : "LLB",
				task.targetFeedingSlot,
				task.targetBlankingSlot,
				task.taskId,
				task.statusToString(task.status),
				getSelectPmProcessName(task));
		}

		originTaskSize = taskManager.getAllTasksSize();
		logWarn("Cycle", "originTaskSize:%d", originTaskSize);

		if (ui->sequence_edit_tbw->rowCount() > 0)
		{
			onUpdateProcessControlEnabled(false);
			logInform("Cycle", "传输序列生成完成.");
			return true;
		}
		return false;
	}

	
	void QSlotTransferCycleVTMWidgetPrivate::logFailed(const std::string station_name, const std::string log){
		Q_Q(QSlotTransferCycleVTMWidget);
		pauseAllThreads();

		// 2025-10-28: 保存错误时的状态快照
		std::string errorLocation = station_name + " - " + log;
		saveCurrentStateSnapshot(log, errorLocation);
		
		logError(station_name.c_str(), log.c_str());
		onUpdateControlEnabled("execute_pbt", true);
		onUpdateLightButtonStatus("light_running_pbt", 3);
	}

	void QSlotTransferCycleVTMWidgetPrivate::logFailedNotNormal(const std::string station_name, const std::string process_name, const int step){
		Q_Q(QSlotTransferCycleVTMWidget);
		logFailed(station_name, Poco::format("%s 未处于正常状态， %s：%d", station_name, process_name, step));
	}

	void QSlotTransferCycleVTMWidgetPrivate::logFailedExcuteCommandHasError(const std::string station_name, const std::string command_name, const std::string process_name, const int step){
		Q_Q(QSlotTransferCycleVTMWidget);
		logFailed(station_name, Poco::format("%s %s命令执行失败， %s：%d", station_name, command_name, process_name, step));
	}

	// 2025-10-28: 捕获当前状态快照
	CycleStateSnapshot::Snapshot QSlotTransferCycleVTMWidgetPrivate::captureCurrentState(
		const std::string& errorMsg, const std::string& errorLocation) 
	{
		CycleStateSnapshot::Snapshot snapshot;
		
		// 基本信息
		Poco::DateTime now;
		snapshot.timestamp = Poco::DateTimeFormatter::format(now, Poco::DateTimeFormat::ISO8601_FORMAT);
		snapshot.errorMessage = errorMsg;
		snapshot.errorLocation = errorLocation;
		
		// 各线程step状态
		snapshot.efem_auto_step = efem_auto_step;
		snapshot.loadlock1_auto_step = loadlock1_auto_step;
		snapshot.loadlock2_auto_step = loadlock2_auto_step;
		snapshot.robot_auto_step = robot_auto_step;
		snapshot.vacuum_auto_step = vacuum_auto_step;
		snapshot.pm1_auto_step = pm1_auto_step.load();
		snapshot.pm2_auto_step = pm2_auto_step.load();
		snapshot.pm3_auto_step = pm3_auto_step.load();
		snapshot.pm4_auto_step = pm4_auto_step.load();
		snapshot.update_auto_step = update_auto_step;
		
		// 标志位状态
		snapshot.tool_allow_get_wafer_LLA = tool_allow_get_wafer_LLA;
		snapshot.tool_allow_get_wafer_LLB = tool_allow_get_wafer_LLB;
		snapshot.tool_allow_put_wafer_LLA = tool_allow_put_wafer_LLA;
		snapshot.tool_allow_put_wafer_LLB = tool_allow_put_wafer_LLB;
		snapshot.lp1_cycle_one_time_finished = lp1_cycle_one_time_finished;
		snapshot.lp2_cycle_one_time_finished = lp2_cycle_one_time_finished;
		snapshot.cycleFinished_lla = cycleFinished_lla;
		snapshot.cycleFinished_llb = cycleFinished_llb;
		
		// 任务统计
		snapshot.originTaskSize = originTaskSize;
		snapshot.lp1TaskSize = lp1TaskSize;
		snapshot.lp2TaskSize = lp2TaskSize;
		
		// 其他状态
		snapshot.current_lp_cycle = current_lp_cycle;
		snapshot.robot_selected_arm = robot_selected_arm;
		
		// 任务列表（从taskManager获取）
		snapshot.allTasks = taskManager.getAllTasks();
		
		return snapshot;
	}

	// 2025-10-28: 保存当前状态快照
	void QSlotTransferCycleVTMWidgetPrivate::saveCurrentStateSnapshot(
		const std::string& errorMsg, const std::string& errorLocation) 
	{
		try {
			auto snapshot = captureCurrentState(errorMsg, errorLocation);
			bool success = CycleStateSnapshot::saveSnapshot(snapshot);
			
			if (success) {
				logWarn("Cycle", "✅ 状态快照已保存，可用于调试恢复.");
			} else {
				logError("Cycle", "⚠️ 保存状态快照失败.");
			}
		}
		catch (const std::exception& e) {
			logError("Cycle", "保存快照时发生异常: %s", e.what());
		}
	}

	// 2025-10-28: 从快照恢复状态
	void QSlotTransferCycleVTMWidgetPrivate::restoreStateFromSnapshot(
		const CycleStateSnapshot::Snapshot& snapshot) 
	{
		// 恢复各线程step状态
		efem_auto_step = snapshot.efem_auto_step;
		loadlock1_auto_step = snapshot.loadlock1_auto_step;
		loadlock2_auto_step = snapshot.loadlock2_auto_step;
		robot_auto_step = snapshot.robot_auto_step;
		vacuum_auto_step = snapshot.vacuum_auto_step;
		pm1_auto_step.store(snapshot.pm1_auto_step);
		pm2_auto_step.store(snapshot.pm2_auto_step);
		pm3_auto_step.store(snapshot.pm3_auto_step);
		pm4_auto_step.store(snapshot.pm4_auto_step);
		update_auto_step = snapshot.update_auto_step;
		
		// 恢复标志位状态
		tool_allow_get_wafer_LLA = snapshot.tool_allow_get_wafer_LLA;
		tool_allow_get_wafer_LLB = snapshot.tool_allow_get_wafer_LLB;
		tool_allow_put_wafer_LLA = snapshot.tool_allow_put_wafer_LLA;
		tool_allow_put_wafer_LLB = snapshot.tool_allow_put_wafer_LLB;
		lp1_cycle_one_time_finished = snapshot.lp1_cycle_one_time_finished;
		lp2_cycle_one_time_finished = snapshot.lp2_cycle_one_time_finished;
		cycleFinished_lla = snapshot.cycleFinished_lla;
		cycleFinished_llb = snapshot.cycleFinished_llb;
		
		// 恢复任务统计
		originTaskSize = snapshot.originTaskSize;
		lp1TaskSize = snapshot.lp1TaskSize;
		lp2TaskSize = snapshot.lp2TaskSize;
		
		// 恢复其他状态
		current_lp_cycle = snapshot.current_lp_cycle;
		robot_selected_arm = snapshot.robot_selected_arm;
		
		// 恢复任务列表（清空taskManager并重新添加）
		taskManager.clearTasks();
		for (const auto& task : snapshot.allTasks) {
			taskManager.addTask(task);
		}
		
		logWarn("Cycle", "✅ 状态已从快照恢复: %s", snapshot.timestamp.c_str());
		logWarn("Cycle", "   错误位置: %s", snapshot.errorLocation.c_str());
		logWarn("Cycle", "   任务数量: %d", (int)snapshot.allTasks.size());
	}

	// 2025-10-28: 加载状态快照
	bool QSlotTransferCycleVTMWidgetPrivate::loadStateSnapshot(const std::string& filePath) 
	{
		try {
			CycleStateSnapshot::Snapshot snapshot;
			bool success = CycleStateSnapshot::loadSnapshot(filePath, snapshot);
			
			if (success) {
				restoreStateFromSnapshot(snapshot);
				return true;
			} else {
				logError("Cycle", "加载状态快照失败: %s", filePath.c_str());
				return false;
			}
		}
		catch (const std::exception& e) {
			logError("Cycle", "加载快照时发生异常: %s", e.what());
			return false;
		}
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
		//for (size_t i = 0; i < d->ui->pm_cavity_param_edit_tbw->columnCount(); i++)
		//{
		//	d->ui->pm_cavity_param_edit_tbw->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Stretch);
		//}
		//d->ui->sequence_edit_tbw->horizontalHeader()->sectionResizeMode(QHeaderView::Stretch);
		d->ui->sequence_edit_tbw->setSelectionBehavior(QAbstractItemView::SelectRows);
		//d->ui->pm_cavity_param_edit_tbw->setSelectionBehavior(QAbstractItemView::SelectRows);
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
		connect(d->ui->continue_pbt, &QPushButton::clicked, this, &QSlotTransferCycleVTMWidget::onContinue);

		//connect(d->ui->loadlock1_put_cassette_finished_pbt, &QPushButton::clicked, this, &QSlotTransferCycleVTMWidget::onLoadLock1PutCassetteFinished);
		//connect(d->ui->loadlock2_put_cassette_finished_pbt, &QPushButton::clicked, this, &QSlotTransferCycleVTMWidget::onLoadLock2PutCassetteFinished);

		connect(d->ui->get_step_pbt, &QPushButton::clicked, this, &QSlotTransferCycleVTMWidget::onGetStep);

		// WPH测试信号槽连接
		connect(d->ui->wph_start_pbt, &QPushButton::clicked, this, &QSlotTransferCycleVTMWidget::onWphTestStart);
		connect(d->ui->wph_stop_pbt, &QPushButton::clicked, this, &QSlotTransferCycleVTMWidget::onWphTestStop);

		//connect(s_ptr, &QFortrendStationStatusVTMWidget::signalUpdateRecipe, this, &QSlotTransferCycleVTMWidget::onUpdateRecipe);
		//2025-10-29 改成选择模式
		connect(s_ptr, &QFortrendStationStatusVTMWidget::signalSelectTransferMode, this, &QSlotTransferCycleVTMWidget::onSelectTransferMode);

		d->ui->enableAtmosphere->setEnabled(true);
		d->ui->execute_pbt->setEnabled(true);
		d->ui->continue_pbt->setEnabled(true);

	/*	initPMCavityParamEdieTableWidget();*/

		//d->ui->gbx_pm_parameter->hide();
		d->ui->wph_test_gbx->hide();
		d->ui->loadlock1_put_cassette_finished_pbt->hide();
		d->ui->loadlock2_put_cassette_finished_pbt->hide();

		d->ui->spb_min->hide();
		d->ui->spb_max->hide();
		d->ui->label_3->hide();
		d->ui->label_4->hide();
		//d->ui->smif_feed_btn->hide();
		//d->ui->smif_blanking_btn->hide();
		d->ui->abort_pbt->hide();
		//d->ui->enablesmif1->hide();
		//d->ui->enablesmif2->hide();

		d->onUpdateLightButtonStatus("light_reset_pbt", 1); 
		d->onUpdateLightButtonStatus("light_running_pbt", 1);

		//initialize thread
		initializeThreads();

	}

	QSlotTransferCycleVTMWidget::~QSlotTransferCycleVTMWidget(){
		Q_D(QSlotTransferCycleVTMWidget);

		d->stopAllThreads(); //2025-8-25 新增

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
		//0=单片上下料模式 1=双片上下料模式
		if (d->running)return;
		try{
			QString fileName = QCoreApplication::applicationDirPath() + "/config/config.ini";

			if (fileName == "")return;
			onClearSequence();

			
#if 0
			std::string fileNamePath = fileName.toStdString(); 
			size_t slash = fileNamePath.rfind('/');
			if (slash != std::string::npos)
			{
				std::string Formula = fileNamePath.substr(slash + 1);
				size_t slash_1 = Formula.find('.');
				if (slash_1 != std::string::npos)
				{
					d->filmTransferMode = Formula.substr(0, slash_1);
				}
				else
				{
					logError(d->module_name.c_str(), "解析配方错误.");
					return;
				}
			}
			else
			{
				logError(d->module_name.c_str(), "解析配方错误.");
				return;

			}
			if (d->filmTransferMode == "")
			{
				QMessageBox::warning(this, tr("警告信息"), tr("解析配方错误"));
				logError(d->module_name.c_str(), "解析配方错误.");
				return;
			}
			logInform(d->module_name.c_str(), "current filmTransferMode:%s", d->filmTransferMode.c_str());

			

			QSettings settings(fileName, QSettings::IniFormat);
			int rowCount = settings.value("rowCount", 0).toInt();
			int columnCount = settings.value("columnCount", 0).toInt();
			if (rowCount == 0 || columnCount == 0)
			{
				logError(d->module_name.c_str(), "解析配方错误.");
				QMessageBox::warning(this, tr("警告信息"), tr("配方内容错误"));
				return;
			}

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
			if (pm_rowCount == 0 || pm_columnCount == 0)
			{
				logError(d->module_name.c_str(), "解析配方错误.");
				QMessageBox::warning(this, tr("警告信息"), tr("配方内容错误"));
				return;
			}

			for (int i = 0; i < pm_rowCount; ++i) {
				for (int j = 1; j < pm_columnCount; ++j) {
					QString key = QString("pm_row%1pm_col%2").arg(i).arg(j);
					QString value = settings.value(key, "").toString();

					qDebug() << "KEY:" << key << "value" << value<<"value.toDouble()"<< value.toDouble();

					QWidget* widget = d->ui->pm_cavity_param_edit_tbw->cellWidget(i, j);
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
						QDoubleSpinBox* dsb = (QDoubleSpinBox*)widget;
						dsb->setValue(value.toDouble());
					}
				}
			}
#endif
		}
		catch (KernelException& e){
			logError(d->module_name.c_str(), e.what());
			//throw e;
		}
	}

	void QSlotTransferCycleVTMWidget::onSelectTransferMode(int model)
	{
		Q_D(QSlotTransferCycleVTMWidget);
		if (d->running)return;
		logInform("test","model:%d", model);
		try {
			switch (model)
			{
			case 0:
				d->currentTransferMode = d->FilmTransferMode::Formula_Go_Up_And_Down;
				break;//要加break
			case 1:
				d->currentTransferMode = d->FilmTransferMode::Formula_Double_Up_And_Down;
				break;
			default:
				break;
			}
		}
		catch (KernelException& e) {
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
			//清空任务队列
			taskManager.clearTasks();
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

		/*int pm_rowCount = settings.value("pm_rowCount", 0).toInt();
		int pm_columnCount = settings.value("pm_columnCount", 0).toInt();
		for (int i = 0; i < pm_rowCount; ++i) {
			for (int j = 1; j < pm_columnCount; ++j) {
				QString key = QString("pm_row%1pm_col%2").arg(i).arg(j);
				QString value = settings.value(key, "").toString();
				QWidget *widget = d->ui->pm_cavity_param_edit_tbw->cellWidget(i, j);
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

		}*/
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

		//int pm_rowCount = d->ui->pm_cavity_param_edit_tbw->rowCount();
		//int pm_columnCount = d->ui->pm_cavity_param_edit_tbw->columnCount();
		//settings.setValue("pm_rowCount", pm_rowCount);
		//settings.setValue("pm_columnCount", pm_columnCount);

		//for (int i = 0; i < pm_rowCount; ++i) {
		//	for (int j = 1; j < pm_columnCount; ++j) {
		//		QString key = QString("pm_row%1pm_col%2").arg(i).arg(j);
		//		QWidget *widget = d->ui->pm_cavity_param_edit_tbw->cellWidget(i, j);

		//		//auto className = widget->metaObject()->className();
		//		//qDebug() << "className:" << className << endl;

		//		if (j == 2)
		//		{
		//			QComboBox* combox = (QComboBox*)widget;
		//			settings.setValue(key, combox->currentText());
		//		}
		//		else if (j == 3)
		//		{
		//			QSpinBox* spb = (QSpinBox*)widget;
		//			settings.setValue(key, spb->value());
		//		}
		//		else
		//		{
		//			QDoubleSpinBox *dsb = (QDoubleSpinBox*)widget;
		//			settings.setValue(key, dsb->value());
		//		}

		//		//if (j > 16)
		//		//{
		//		//	QComboBox *combox = (QComboBox*)widget;
		//		//	settings.setValue(key, combox->currentText());
		//		//}
		//		//else{
		//		//	QDoubleSpinBox *dsb = (QDoubleSpinBox*)widget;
		//		//	settings.setValue(key, dsb->value());
		//		//}

		//	}

		//}
	
		
		
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

	void QSlotTransferCycleVTMWidget::onPause(){
		Q_D(QSlotTransferCycleVTMWidget);
		std::shared_ptr<FortrendPMCavitySubsystem> pm1 = d->kernel->getKernelModule<FortrendPMCavitySubsystem>("PM1");
		std::shared_ptr<FortrendPMCavitySubsystem> pm2 = d->kernel->getKernelModule<FortrendPMCavitySubsystem>("PM2");
		std::shared_ptr<FortrendPMCavitySubsystem> pm3 = d->kernel->getKernelModule<FortrendPMCavitySubsystem>("PM3");
		std::shared_ptr<FortrendPMCavitySubsystem> pm4 = d->kernel->getKernelModule<FortrendPMCavitySubsystem>("PM4");

		pm1->setIsRunning(false);
		pm2->setIsRunning(false);
		pm3->setIsRunning(false);
		pm4->setIsRunning(false);



		d->pauseAllThreads();
		//d->running = false;
		//d->ispause = true;

		d->ui->execute_pbt->setEnabled(true);
		d->ui->reset_pbt->setEnabled(true);
		d->ui->pause_pbt->setEnabled(false);

		//2025-8-06
		d->onUpdateProcessControlEnabled(true);
		d->onUpdateLightButtonStatus("light_running_pbt", 4);
	}

	void QSlotTransferCycleVTMWidget::onAbort(){
		Q_D(QSlotTransferCycleVTMWidget);
		//d->setTransferSequence();
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

	void QSlotTransferCycleVTMWidget::onContinue()
	{
		Q_D(QSlotTransferCycleVTMWidget);
		d->startAllThreads();
		d->onUpdateLightButtonStatus("light_running_pbt", 2);
	}

	void QSlotTransferCycleVTMWidget::onReset(){
		Q_D(QSlotTransferCycleVTMWidget);
		std::shared_ptr<FortrendTMCavitySubsystem> tm = d->kernel->getKernelModule<FortrendTMCavitySubsystem>("TM");
		d->plcauto = tm->getPlcMode();

		if(d->ui->simulation_cbx->checkState() != Qt::CheckState::Checked)
		{
			if (!isEnabledplcAuto()) {
				QMessageBox::warning(this, "警告", "PLC不在自动模式.");
				return;
			}
		}
		d->running = false;
		d->ispause = false;

		//2025-8-14 新加，当有完成的cycle,再次跑cycle,刷新这个变量
		d->cycleFinished_lla = false;
		d->cycleFinished_llb = false;

		std::thread thread(&QSlotTransferCycleVTMWidget::resetAction, this);
		thread.detach();
	}

	void QSlotTransferCycleVTMWidget::onStart() {
		Q_D(QSlotTransferCycleVTMWidget);

		//start action & store param
		std::shared_ptr<FortrendPMCavitySubsystem> pm1 = d->kernel->getKernelModule<FortrendPMCavitySubsystem>("PM1");
		std::shared_ptr<FortrendPMCavitySubsystem> pm2 = d->kernel->getKernelModule<FortrendPMCavitySubsystem>("PM2");
		std::shared_ptr<FortrendPMCavitySubsystem> pm3 = d->kernel->getKernelModule<FortrendPMCavitySubsystem>("PM3");
		std::shared_ptr<FortrendPMCavitySubsystem> pm4 = d->kernel->getKernelModule<FortrendPMCavitySubsystem>("PM4");

		//if (!d->reset_finish)
		//{
		//	QMessageBox::warning(this, "警告", "未执行整机复位.");
		//	return;
		//}
		//调试注释
#ifdef CYCLE_SIM_MODE
		if (!pm2->getPMCavityMotorHomeSignal()) {
			QMessageBox::warning(this, "警告", "PM腔步进电机未后退到原位.");
			return;
		}
#endif
		d->cycle_times_lla = d->ui->cycle_setting_times_sbx->value(); //LP1循环次数
		d->cycle_times_llb = d->ui->cycle_setting_times_sbx_2->value();//LP2循环次数

		if (taskManager.getAllTasksSize() == 0 && !d->ispause)// 无任务配置，无暂停，才执行解析流程配方
		{
			//快照回复测试
#ifdef DEBUG_LOAD_SNAPSHOT

			auto snapshots = CycleStateSnapshot::listSnapshots();
			if (!snapshots.empty())
			{
				if (d->loadStateSnapshot(snapshots.back()))
				{
					logWarn("Cycle", "🔄 已从快照恢复，直接跳到错误位置.");
				}
			}
#else
			//正常流程
			if (d->setHLTransferSequence())
			{
				d->ui->cycle_finished_times_spx->setValue(0);
				d->ui->cycle_finished_times_spx_2->setValue(0);
				logInform("Cycle", "传送HL流程配置成功.");
				//return;
			}
			else
			{
				QMessageBox::warning(this, "警告", "传送流程配置错误.");
				return;
			}

			//2025-8-14 默认已经手动处理机台片子，重新把wafer数据刷新到初始状态
			//暂停过，检测机台状态
			if (d->ispause && taskManager.detectionHasNoInitialTypeTasks()) {

				QMessageBox::warning(this, "警告", "请检查机台有无片子，再执行整体复位");
				return;
			}

#endif
#ifdef CYCLE_SIM_MODE
			if (!isEnabledplcAuto()) {
				QMessageBox::warning(this, "警告", "PLC不在自动模式.");
				return;
			}
#endif
			d->startAllThreads();
			//d->running = true;
			d->ispause = false;

			pm1->setIsRunning(d->running);
			pm2->setIsRunning(d->running);
			pm3->setIsRunning(d->running);
			pm4->setIsRunning(d->running);

			Sleep(500);
			d->onUpdateLightButtonStatus("light_running_pbt", 2);
			d->tower->setOutput(FortrendVTMSignalTower::Output::YELLOW_LIGHT, false);
			d->tower->setOutput(FortrendVTMSignalTower::Output::GREEN_LIGHT, true);
			d->ui->execute_pbt->setEnabled(false);
			d->ui->reset_pbt->setEnabled(false);
			d->ui->pause_pbt->setEnabled(true);
		}
		else
		{
			logError("Cycle","无任务配置，无暂停，才执行解析流程配方.");
		}
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
	void QSlotTransferCycleVTMWidget::clickContinue()
	{
		onContinue();
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
		std::shared_ptr<FortrendPMCavitySubsystem> pm1 = d->kernel->getKernelModule<FortrendPMCavitySubsystem>("PM1");
		std::shared_ptr<FortrendPMCavitySubsystem> pm2 = d->kernel->getKernelModule<FortrendPMCavitySubsystem>("PM2");
		std::shared_ptr<FortrendPMCavitySubsystem> pm3 = d->kernel->getKernelModule<FortrendPMCavitySubsystem>("PM3");
		std::shared_ptr<FortrendPMCavitySubsystem> pm4 = d->kernel->getKernelModule<FortrendPMCavitySubsystem>("PM4");

		bool  ups = false;
		bool isCloseInsertingPlateTM = false;
		bool isCloseInsertingPlateLLA = false;
		bool isCloseInsertingPlateLLB = false;
		bool changed = false;
		bool cmdstate = true;
		bool isCloseAngleValveTM = false;
		bool isCloseAngleValveLLA = false;
		bool isCloseAngleValveLLB = false;

		while (true)
		{
			Sleep(500);
			//监控LLA/LLB 上料互锁条件
			//bool isInterlock = d->isLoadingInterlock("LLA");
			//bool isInterlock = d->isLoadingInterlock("LLB");
		


#if 0
			pm2->setIsRunning(d->running);
			tm->readBit("MR35507", ups);
			tm->readBit("MR50001", d->plcauto);

			if (ups!=d->hasUPS)
			{
				d->hasUPS = ups;
				changed = true;
				if (d->hasUPS){
					logError("Cycle", "检测到断电，等待动作执行完后停止cycle.");
				}
				else{
					logError("Cycle", "检测到断电恢复，重新开始cycle.");
				}
			}


			if (d->hasUPS)
			{
				if (d->robot_step_wafer_finished)
				{
					d->robot_step_wafer_finished = false;
					logError("Cycle", "动作已完成，暂停cycle.");
					onPause();
					/*d->running = false;
					d->onUpdateControlEnabled("reset_pbt", true);
					d->onUpdateControlEnabled("execute_pbt", true);
					d->onUpdateControlEnabled("pause_pbt", false);*/
				}
				if (!d->running)
				{
					if (tm&&tm->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						if (tm->getInsertingPlateValveOpend() && cmdstate)
						{
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
				if (!d->running&&changed)
				{
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
			
#endif		
			
		}
	}

	void QSlotTransferCycleVTMWidget::resetAction(){
		Q_D(QSlotTransferCycleVTMWidget);
		d->resetAction();
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

	void FC::QSlotTransferCycleVTMWidget::executeUpdateTransferStatus()
	{
		Q_D(QSlotTransferCycleVTMWidget);
		d->executeUpdateTransferStatus();
	}

	void QSlotTransferCycleVTMWidget::onGetStep(){
		Q_D(QSlotTransferCycleVTMWidget);
		d->onGetStep();
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
	/*	d->ui->pm_cavity_param_edit_tbw->setEnabled(enabled);*/
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

	void QSlotTransferCycleVTMWidget::updateLightButtonStatus(const QString control, const int color)
	{
		Q_D(QSlotTransferCycleVTMWidget);
		if (control == "light_reset_pbt")
		{
			if(color == 1)
			{
				//复位完成
				d->ui->light_running_pbt->setYellow();
				d->ui->reset_status->setText("待复位");
			}
			else if(color == 2)
			{
				d->ui->light_running_pbt->setLightBlue();
				d->ui->reset_status->setText("复位中");
			}
			else
			{
				//复位完成
				d->ui->light_running_pbt->setLightGreen();
				d->ui->reset_status->setText("复位完成");
			}
		}
		else if (control == "light_running_pbt")
		{
			if (color == 1)
			{
				//空闲或者暂停
				d->ui->light_running_pbt->stopAlarm();
				d->ui->light_running_pbt->setYellow();
				d->ui->run_status->setText("空闲");
			}
			else if (color == 2)
			{
				d->ui->light_running_pbt->stopAlarm();
				//运行
				d->ui->light_running_pbt->setLightGreen();
				d->ui->run_status->setText("运行中");
			}
			else if (color == 3)
			{
				//报警
				d->ui->light_running_pbt->setAlarmColor(QColor(255, 0, 0));
				d->ui->light_running_pbt->setNormalColor(QColor(0, 0, 0));
				d->ui->light_running_pbt->startAlarm();
				d->ui->run_status->setText("报警");
			}
			else if(color == 4) 
			{
				d->ui->light_running_pbt->stopAlarm();
				d->ui->light_running_pbt->setYellow();
				d->ui->run_status->setText("暂停");

			}
		}
	}

	//void QSlotTransferCycleVTMWidget::addEditTableWidgetItemDoubleSpinBox(int row, int column, double min_value, double max_value, double single_step, double value, int decimals_value){
	//	Q_D(QSlotTransferCycleVTMWidget);
	//	QDoubleSpinBox *dsb = new QDoubleSpinBox();
	//	dsb->setMaximum(max_value);
	//	dsb->setMinimum(min_value);
	//	dsb->setDecimals(decimals_value);
	//	dsb->setSingleStep(single_step);
	//	dsb->setValue(value);
	//	d->ui->pm_cavity_param_edit_tbw->setCellWidget(row, column, dsb);
	//}
	
	//void QSlotTransferCycleVTMWidget::addEditTableWidgetItemComboBox(int row, int column, int value)
	//{
	//	Q_D(QSlotTransferCycleVTMWidget);
	//	QComboBox* dcb = new QComboBox();
	//	QMap<QString, int> AngleMap;
	//	AngleMap.insert("0",0);
	//	AngleMap.insert("60", 60);
	//	AngleMap.insert("120", 120);
	//	AngleMap.insert("180", 180);
	//	AngleMap.insert("240", 240);
	//	AngleMap.insert("300", 300);
	//	AngleMap.insert("360", 360);
	//	foreach(const QString & str, AngleMap.keys())
	//		dcb->addItem(str, AngleMap.value(str));

	//	dcb->setCurrentIndex(value);

	//	d->ui->pm_cavity_param_edit_tbw->setCellWidget(row,column,dcb);
	//}


	void QSlotTransferCycleVTMWidget::startProcessingThreads()
	{
		Q_D(QSlotTransferCycleVTMWidget);

		std::thread thd_efem(&QSlotTransferCycleVTMWidget::executeEFEMTransfer, this);
		thd_efem.detach();
		
		std::thread thd_LoadLockA(&QSlotTransferCycleVTMWidget::executeLLATransfer, this);
		thd_LoadLockA.detach();

		std::thread thd_LoadLockB(&QSlotTransferCycleVTMWidget::executeLLBTransfer, this);
		thd_LoadLockB.detach();

		std::thread thd_vacumn(&QSlotTransferCycleVTMWidget::startVacuumAction, this);
		thd_vacumn.detach();

		std::thread thread_pm1(&QSlotTransferCycleVTMWidget::executePM1Transfer,this);
		thread_pm1.detach();

		std::thread thread_pm2(&QSlotTransferCycleVTMWidget::executePM2Transfer, this);
		thread_pm2.detach();

		std::thread thread_pm3(&QSlotTransferCycleVTMWidget::executePM3Transfer, this);
		thread_pm3.detach();

		std::thread thread_pm4(&QSlotTransferCycleVTMWidget::executePM4Transfer, this);
		thread_pm4.detach();

		std::thread thread_update(&QSlotTransferCycleVTMWidget::executeUpdateTransferStatus,this);
		thread_update.detach();

	}

	void QSlotTransferCycleVTMWidget::initializeThreads()
	{
		Q_D(QSlotTransferCycleVTMWidget);
		// Create and start threads
		d->threads.emplace_back(&QSlotTransferCycleVTMWidget::executeEFEMTransfer, this);
		d->threads.emplace_back(&QSlotTransferCycleVTMWidget::executeLLATransfer, this);
		d->threads.emplace_back(&QSlotTransferCycleVTMWidget::executeLLBTransfer, this);
		d->threads.emplace_back(&QSlotTransferCycleVTMWidget::startVacuumAction, this);
		d->threads.emplace_back(&QSlotTransferCycleVTMWidget::executePM1Transfer, this);
		d->threads.emplace_back(&QSlotTransferCycleVTMWidget::executePM2Transfer, this);
		d->threads.emplace_back(&QSlotTransferCycleVTMWidget::executePM3Transfer, this);
		d->threads.emplace_back(&QSlotTransferCycleVTMWidget::executePM4Transfer, this);
		d->threads.emplace_back(&QSlotTransferCycleVTMWidget::executeUpdateTransferStatus, this);
	}

	//=============================================================================
	// WPH测试功能实现 - 大气环境下LoadLock之间传片测试
	//=============================================================================

	void QSlotTransferCycleVTMWidget::onWphTestStart()
	{
		Q_D(QSlotTransferCycleVTMWidget);

		if (d->wph_test_running.load()) {
			QMessageBox::warning(this, "警告", "WPH测试正在运行中");
			return;
		}

		// 获取循环次数
		d->wph_cycle_count = d->ui->wph_cycle_count_sbx->value();
		d->wph_completed_count = 0;
		d->ui->wph_completed_count_sbx->setValue(0);

		d->wph_test_running = true;
		d->wph_test_stop_requested = false;

		// 启动WPH测试线程
		d->wph_test_thread = std::thread(&QSlotTransferCycleVTMWidgetPrivate::executeWphTest, d);
		d->wph_test_thread.detach();

		d->ui->wph_start_pbt->setEnabled(false);
		d->ui->wph_stop_pbt->setEnabled(true);

		logInform("WPH_TEST", "WPH测试启动，目标循环次数：%d", d->wph_cycle_count);
	}

	void QSlotTransferCycleVTMWidget::onWphTestStop()
	{
		Q_D(QSlotTransferCycleVTMWidget);

		d->wph_test_stop_requested = true;
		logInform("WPH_TEST", "WPH测试停止请求已发送.");

		d->ui->wph_start_pbt->setEnabled(true);
		d->ui->wph_stop_pbt->setEnabled(false);
	}

	void QSlotTransferCycleVTMWidget::updateWphCompletedCount(int count)
	{
		Q_D(QSlotTransferCycleVTMWidget);
		d->ui->wph_completed_count_sbx->setValue(count);
	}

	void QSlotTransferCycleVTMWidget::executeWphTest()
	{
		Q_D(QSlotTransferCycleVTMWidget);
		d->executeWphTest();
	}

	void QSlotTransferCycleVTMWidgetPrivate::executeWphTest()
	{
		/*
		 * WPH测试流程（大气环境下）：
		 * 晶圆默认在LoadlockA的一槽
		 * 循环流程：
		 * 1. VTM机械手A手从LLA槽1取片
		 * 2. VTM机械手A手放片到LLB槽1
		 * 3. VTM机械手A手从LLB槽1取片
		 * 4. VTM机械手A手放片回LLA槽1
		 * 完成一个循环，打印日志
		 */

		try {
			// 获取子系统
			auto wtr = kernel->getKernelModule<FortrendSunwayRobotSubsystem>("WTR");
			auto lk1 = kernel->getKernelModule<FortrendLoadLockSubsystem>("LLA");
			auto lk2 = kernel->getKernelModule<FortrendLoadLockSubsystem>("LLB");

			if (!wtr || !lk1 || !lk2) {
				logError("WPH_TEST", "无法获取机械手或LoadLock子系统.");
				wph_test_running = false;
				QMetaObject::invokeMethod(q_ptr, "updateWphCompletedCount", Qt::QueuedConnection, Q_ARG(int, wph_completed_count));
				return;
			}

			// 记录开始时间
			auto test_start_time = std::chrono::steady_clock::now();

			logInform("WPH_TEST", "========== WPH测试开始 ==========");
			logInform("WPH_TEST", "测试条件：大气环境，一片晶圆.");
			logInform("WPH_TEST", "传输路径：LLA槽2 <-> LLB槽2.");
			logInform("WPH_TEST", "目标循环次数：%d", wph_cycle_count);

			for (int cycle = 0; cycle < wph_cycle_count && !wph_test_stop_requested.load(); cycle++)
			{
				auto cycle_start_time = std::chrono::steady_clock::now();

				logInform("WPH_TEST", "---------- 循环 %d/%d 开始 ----------", cycle + 1, wph_cycle_count);

				// 检查子系统状态
				if (wtr->getState() != IKernelSubSystem::State::SUB_NORMAL) {
					logError("WPH_TEST", "机械手状态异常，测试中止.");
					break;
				}
				if (lk1->getState() != IKernelSubSystem::State::SUB_NORMAL) {
					logError("WPH_TEST", "LLA状态异常，测试中止.");
					break;
				}
				if (lk2->getState() != IKernelSubSystem::State::SUB_NORMAL) {
					logError("WPH_TEST", "LLB状态异常，测试中止.");
					break;
				}

				// 步骤1：从LLA槽1取片（A手）
				logInform("WPH_TEST", "[步骤1] A手从LLA槽2取片...");
				{
					if (!lk1->getTMCavityDoorOpend()) {
						auto cmd_open = lk1->createOpenTMCavityDoorCommand();
						lk1->startCommand(cmd_open);
						cmd_open->wait();
						if (cmd_open->hasError()) {
							logError("WPH_TEST", "打开LLA TM门失败.");
							break;
						}
					}

					auto cmd_get = wtr->createGetCommand(lk1, 0, 2); // A手=0, 槽2
					wtr->startCommand(cmd_get);
					cmd_get->wait();
					if (cmd_get->hasError()) {
						logError("WPH_TEST", "A手从LLA取片失败.");
						break;
					}
					logInform("WPH_TEST", "[步骤1] 完成.");
				}

				if (wph_test_stop_requested.load()) break;

				// 步骤2：放片到LLB槽1（A手）
				logInform("WPH_TEST", "[步骤2] A手放片到LLB槽2...");
				{
					if (!lk2->getTMCavityDoorOpend()) {
						auto cmd_open = lk2->createOpenTMCavityDoorCommand();
						lk2->startCommand(cmd_open);
						cmd_open->wait();
						if (cmd_open->hasError()) {
							logError("WPH_TEST", "打开LLB TM门失败.");
							break;
						}
					}

					auto cmd_put = wtr->createPutCommand(lk2, 0, 2); // A手=0, 槽2
					wtr->startCommand(cmd_put);
					cmd_put->wait();
					if (cmd_put->hasError()) {
						logError("WPH_TEST", "A手放片到LLB失败.");
						break;
					}
					logInform("WPH_TEST", "[步骤2] 完成.");
				}

				if (wph_test_stop_requested.load()) break;

				// 步骤3：从LLB槽1取片（A手）
				logInform("WPH_TEST", "[步骤3] A手从LLB槽2取片...");
				{
					auto cmd_get = wtr->createGetCommand(lk2, 0, 2); // A手=0, 槽2
					wtr->startCommand(cmd_get);
					cmd_get->wait();
					if (cmd_get->hasError()) {
						logError("WPH_TEST", "A手从LLB取片失败.");
						break;
					}
					logInform("WPH_TEST", "[步骤3] 完成.");
				}

				if (wph_test_stop_requested.load()) break;

				// 步骤4：放片回LLA槽1（A手）
				logInform("WPH_TEST", "[步骤4] A手放片回LLA槽2...");
				{
					auto cmd_put = wtr->createPutCommand(lk1, 0, 2); // A手=0, 槽2
					wtr->startCommand(cmd_put);
					cmd_put->wait();
					if (cmd_put->hasError()) {
						logError("WPH_TEST", "A手放片回LLA失败.");
						break;
					}
					logInform("WPH_TEST", "[步骤4] 完成.");
				}

				// 计算本循环耗时
				auto cycle_end_time = std::chrono::steady_clock::now();
				auto cycle_duration = std::chrono::duration_cast<std::chrono::milliseconds>(cycle_end_time - cycle_start_time).count();

				wph_completed_count = cycle + 1;

				// 更新UI
				QMetaObject::invokeMethod(q_ptr, "updateWphCompletedCount", Qt::QueuedConnection, Q_ARG(int, wph_completed_count));

				logInform("WPH_TEST", "---------- 循环 %d/%d 完成，耗时：%lld ms ----------", 
					cycle + 1, wph_cycle_count, cycle_duration);
			}

			// 计算总耗时和WPH
			auto test_end_time = std::chrono::steady_clock::now();
			auto total_duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(test_end_time - test_start_time).count();
			double total_duration_sec = total_duration_ms / 1000.0;
			double total_duration_hours = total_duration_ms / 3600000.0;

			// WPH计算方式：WPH = 3600秒 / 平均循环时间（秒）
			// 一次循环 = LLA->LLB->LLA 完整往返
			double avg_cycle_time_sec = (wph_completed_count > 0) ? (total_duration_sec / wph_completed_count) : 0;
			double wph = (avg_cycle_time_sec > 0) ? (3600.0 / avg_cycle_time_sec) : 0;

			// 如果总时间超过1小时，也可以用实际数据验证：实际WPH = 完成循环数 / 总时间（小时）
			double actual_wph = (total_duration_hours > 0) ? (wph_completed_count / total_duration_hours) : 0;

			//logInform("WPH_TEST", Poco::format("...：%f", wph)); （使用了 Poco 的格式化）

			logInform("WPH_TEST", "========== WPH测试结束 ==========");
			logInform("WPH_TEST", "完成循环数：%d/%d", wph_completed_count, wph_cycle_count);

			//logInform("WPH_TEST", Poco::format("总耗时：%.2f 秒 (%.4f 小时)", total_duration_sec, total_duration_hours));

			//logInform("WPH_TEST", "平均单次循环时间：%.2f 秒.", avg_cycle_time_sec);

			//logInform("WPH_TEST", "计算WPH（3600/平均循环时间）：%.2f 片/小时.", wph);
			
			//if (total_duration_hours >= 1.0) {
			//	logInform("WPH_TEST", "实际WPH（循环数/总时间）：%.2f 片/小时.", actual_wph);
			//}

			if (wph >= 150) {
				logInform("WPH_TEST", "结果：【通过】WPH >= 150片/小时要求.");
			} else {
				logWarn("WPH_TEST", "结果：【未达标】WPH < 150片/小时要求（需要单次循环时间 <= 24秒）");
			}

			wph_test_running = false;

			// 恢复UI状态
			QMetaObject::invokeMethod(q_ptr->d_ptr->ui->wph_start_pbt, "setEnabled", Qt::QueuedConnection, Q_ARG(bool, true));
			QMetaObject::invokeMethod(q_ptr->d_ptr->ui->wph_stop_pbt, "setEnabled", Qt::QueuedConnection, Q_ARG(bool, false));

		}
		catch (const std::exception& e) {
			logError("WPH_TEST", "WPH测试异常：%s", e.what());
			wph_test_running = false;
		}
		catch (...) {
			logError("WPH_TEST", "WPH测试未知异常.");
			wph_test_running = false;
		}
	}

}