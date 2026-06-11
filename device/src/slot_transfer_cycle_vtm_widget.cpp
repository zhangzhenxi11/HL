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
#include <algorithm>
#include <functional>
#include "Poco/Format.h"
#include <QThread>

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

#define DEBUG_TEST_PM

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
		std::atomic<std::thread::id*> m_owner{ nullptr };
	public:
		void lock() {
			m_mutex.lock();
			m_owner.store(new std::thread::id(std::this_thread::get_id()));
			m_locked = true;
		}

		void unlock() {
			m_locked = false;
			auto owner = m_owner.exchange(nullptr);
			m_mutex.unlock();
			delete owner;
		}

		bool try_lock() {
			if (m_mutex.try_lock()) {
				m_owner.store(new std::thread::id(std::this_thread::get_id()));
				m_locked = true;
				return true;
			}
			return false;
		}

		bool is_locked() const {
			return m_locked;
		}
		bool is_owned_by_current_thread() const {
			auto owner = m_owner.load();
			return owner != nullptr && *owner == std::this_thread::get_id();
		}

		bool unlock_if_owned_by_current_thread() {
			if (!is_owned_by_current_thread()) {
				return false;
			}
			unlock();
			return true;
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

		bool getArmWaferIsPmPending(int arm);

		struct PM2ScheduleSnapshot;
		struct LLImmediateRepickState;
		struct LoadLockTaskSnapshot;
		struct PmTaskSnapshot;

		bool isSimulationModeEnabled() const
		{
			return simulation_mode_enabled.load();
		}

		bool ensurePm2SubsystemReady(std::shared_ptr<FortrendPMCavitySubsystem>& pmSubsystem);

		// 统一读取PM占片状态，避免各状态机分支重复判空和打印不一致的失败日志。
		bool tryGetPmHasWafer(const std::shared_ptr<FortrendPMCavitySubsystem>& pmSubsystem, const char* pmName, bool& hasWafer) const;

		// 统一组装PM2调度快照，供1051和立即补取等发送前判断复用同一套上下文。
		bool tryBuildPm2ScheduleSnapshot(PM2ScheduleSnapshot& snapshot);

		// 把“当前LL是否必须让PM2优先”集中在一处，避免不同step各写一套规则。
		bool shouldLlWaitForPm2Priority(const PM2ScheduleSnapshot& snapshot, int requestedLlArm, std::string& reason) const;

		// 对齐交互片时序：PM2仍在加工且双手都空时，允许LL按固定arm预装下一片在手等待交换。
		bool canLlImmediateRepickWaitForPm2Craft(const PM2ScheduleSnapshot& snapshot, int repickArm) const;

		bool tryPlanImmediateRepick(const char* loadLockName,
			const LoadLockTaskSnapshot& snapshot,
			bool armAHasWafer,
			bool armBHasWafer,
			UnifiedWaferTask::Location requiredTargetPm,
			LLImmediateRepickState& plan);

		LoadLockTaskSnapshot buildLoadLockTaskSnapshot(const char* loadLockName) const;
		PmTaskSnapshot buildPmTaskSnapshot(const std::string& pmName) const;
		bool runPmRecipeMotorOnUiThread(int pmIndex, const char* stationName, const std::string& processName, std::atomic<int>& autoStep);

		// 发送LL取片请求前统一检查目标手臂是否已被实物、挂起请求或当前robot_step占用。
		bool isRobotArmOccupiedForLlRequest(int targetArm, std::string& reason) const;

		// LL侧发送RQLoad前统一检查WTR是否已被Robot线程占用或即将占用，避免查询与取放片抢发。
		bool shouldLlWaitForWtrFingerQuery(std::string& reason) const;

		// 统一检查WTR当前是否适合继续发手指查询，供A/B两次RQLoad之间复用同一套判断。
		bool shouldWaitBeforeStartWtrFingerQuery(const std::shared_ptr<FortrendSunwayRobotSubsystem>& wtr, std::string& reason) const;


		int getOtherArm(const int arm) const
		{
			return arm == 0 ? 1 : (arm == 1 ? 0 : -1);
		}

		//上料互锁条件
		bool llA_condition = false; //true 满足互锁 ， false不满足没锁

		bool llB_condition = false;//true 满足互锁 ，  false不满足没锁

		std::string LLAPmName; //lla当前处理的pm

		std::string LLBPmName; //llb当前处理的pm


		//仅在真实下料冲突，或对侧LL下层完成片需要优先回LP时上锁。
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
		std::atomic<bool> simulation_mode_enabled{ false };
		std::atomic<bool> simulate_pm2_enabled{ false };
		std::atomic<bool> pm2_exchange_in_flight{ false };
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

		CheckableMutex wtr_robot_mutex; //真空机械手锁

		std::mutex feeding_mutex; //上料锁

		std::mutex blanking_mutex;//下料锁
		

		/************************EFEM*********************************/

		//2026-1-23 双上料时缓存两个任务的完整信息，避免case 157更新状态后数组变化
		UnifiedWaferTask cached_task_first;  // 第一片料缓存
		UnifiedWaferTask cached_task_second; // 第二片料缓存
			
		/************************loadLock*********************************/

		std::vector<UnifiedWaferTask> loadLockPendingTasks;

		std::vector<UnifiedWaferTask> loadLockCompletedTasks;

		std::vector<UnifiedWaferTask> loadLockReturnPendingTasks;

		std::vector<UnifiedWaferTask> loadLockReturnCompletedTasks;


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
		int robot_step = 10;
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
		bool pm2_need_return_wafer = false;  //PM2工艺完成后需取回晶圆（非交换场景）

		//2026-5-16
		bool pm2_allow_loading_wafer = false; //PM2上料请求
		bool pm2_allow_down_wafer = false;    //PM2下料请求


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

		// ===== 机械手集中调度：请求标志（由 LL/PM 线程设置，Robot线程读取并清除）=====
		struct RobotTransferRequest {
			std::atomic<bool> requested{ false };  // 请求标志
			std::atomic<bool> done{ false };       // 完成标志
			std::atomic<bool> success{ false };    // 执行结果
			std::atomic<bool> expedited{ false };  // LL回片后的立即补取优先级
			std::atomic<int> arm{ 0 };             // 使用哪个手臂 (0=A, 1=B)
			std::atomic<int> slot{ 1 };            // LL slot编号
		};

		// 记录“LL回片后立即补取”的计划信息。
		// 它不是像 PM 交换片那样的一条底层组合指令，而是让 LL 线程跨多个 case
		// 记住：回片完成后，还要用哪只手、从哪个 LL 槽位再补取哪一个 task。
		struct LLImmediateRepickState {
			bool enabled = false;
			int taskId = -1;
			int arm = -1;
			int slot = -1;

			void reset()
			{
				enabled = false;
				taskId = -1;
				arm = -1;
				slot = -1;
			}
		};

		// 交换操作需要两个arm信息
		struct ExchangeInfo {
			std::atomic<int> getArm{ 0 };   // 取片用的arm
			std::atomic<int> putArm{ 1 };   // 放片用的arm
		};

		struct PM2ScheduleSnapshot {
			bool hasWaferPm = false;
			bool pm2CraftInProgress = false;
			bool armAHasWafer = false;
			bool armAHasPending = false;
			bool armBHasWafer = false;
			bool armBHasPending = false;
			int pm2PendingCount = 0;
			int pm2CompletedCount = 0;
			int returnPendingCount = 0;
			int preferredPmArm = -1;
		};

		struct LoadLockTaskSnapshot {
			std::vector<UnifiedWaferTask> pendingTasks;
			std::vector<UnifiedWaferTask> completedTasks;
			std::vector<UnifiedWaferTask> returnPendingTasks;
			std::vector<UnifiedWaferTask> returnCompletedTasks;
		};

		struct PmTaskSnapshot {
			std::vector<UnifiedWaferTask> pendingTasks;
			std::vector<UnifiedWaferTask> completedTasks;
			std::vector<UnifiedWaferTask> progressingTasks;
		};

		// LL取片请求：从LL取到机械手
		RobotTransferRequest robot_get_from_lla;
		RobotTransferRequest robot_get_from_llb;
		LLImmediateRepickState llaImmediateRepick;
		LLImmediateRepickState llbImmediateRepick;// 记录LL回片后是否需要立即补取，以及补取的taskId、手臂和槽位信息

		// PM放片请求：从机械手放到PM
		RobotTransferRequest robot_put_to_pm1;
		RobotTransferRequest robot_put_to_pm2;
		RobotTransferRequest robot_put_to_pm3;
		RobotTransferRequest robot_put_to_pm4;

		// PM取片请求：从PM取到机械手
		RobotTransferRequest robot_get_from_pm1;
		RobotTransferRequest robot_get_from_pm2;
		RobotTransferRequest robot_get_from_pm3;
		RobotTransferRequest robot_get_from_pm4;

		// LL放片请求：从机械手放回LL
		RobotTransferRequest robot_put_to_lla;
		RobotTransferRequest robot_put_to_llb;

		// PM交换请求（取出已加工片+放入新片）
		RobotTransferRequest robot_exchange_pm1;
		RobotTransferRequest robot_exchange_pm2;
		RobotTransferRequest robot_exchange_pm3;
		RobotTransferRequest robot_exchange_pm4;

		ExchangeInfo exchange_info_pm1;
		ExchangeInfo exchange_info_pm2;
		ExchangeInfo exchange_info_pm3;
		ExchangeInfo exchange_info_pm4;
		std::atomic<int> lla_return_task_id{ -1 };
		std::atomic<int> lla_return_slot{ -1 };
		std::atomic<int> llb_return_task_id{ -1 };
		std::atomic<int> llb_return_slot{ -1 };
		std::atomic<int> pm2_craft_task_id{ -1 };

		// 重置所有机械手请求标志
		void resetAllRobotFlags() {
			robot_get_from_lla.requested.store(false); robot_get_from_lla.done.store(false); robot_get_from_lla.expedited.store(false);
			robot_get_from_llb.requested.store(false); robot_get_from_llb.done.store(false); robot_get_from_llb.expedited.store(false);
			robot_put_to_pm1.requested.store(false); robot_put_to_pm1.done.store(false);
			robot_put_to_pm2.requested.store(false); robot_put_to_pm2.done.store(false);
			robot_put_to_pm3.requested.store(false); robot_put_to_pm3.done.store(false);
			robot_put_to_pm4.requested.store(false); robot_put_to_pm4.done.store(false);
			robot_get_from_pm1.requested.store(false); robot_get_from_pm1.done.store(false);
			robot_get_from_pm2.requested.store(false); robot_get_from_pm2.done.store(false);
			robot_get_from_pm3.requested.store(false); robot_get_from_pm3.done.store(false);
			robot_get_from_pm4.requested.store(false); robot_get_from_pm4.done.store(false);
			robot_put_to_lla.requested.store(false); robot_put_to_lla.done.store(false);
			robot_put_to_llb.requested.store(false); robot_put_to_llb.done.store(false);
			robot_exchange_pm1.requested.store(false); robot_exchange_pm1.done.store(false);
			robot_exchange_pm2.requested.store(false); robot_exchange_pm2.done.store(false);
			robot_exchange_pm3.requested.store(false); robot_exchange_pm3.done.store(false);
			robot_exchange_pm4.requested.store(false); robot_exchange_pm4.done.store(false);
			llaImmediateRepick.reset();
			llbImmediateRepick.reset();
		}

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

	bool QSlotTransferCycleVTMWidgetPrivate::ensurePm2SubsystemReady(std::shared_ptr<FortrendPMCavitySubsystem>& pmSubsystem)
	{
		pmSubsystem.reset();
		if (kernel == nullptr)
		{
			logWarn("PM2", "获取PM2子系统失败: kernel为空.");
			return false;
		}

		if (pm2 == nullptr)
		{
			pm2 = kernel->getKernelModule<FortrendPMCavitySubsystem>("PM2");
			if (pm2 != nullptr)
			{
				logInform("PM2", "检测到成员pm2为空，已重新获取PM2子系统.");
			}
		}

		pmSubsystem = pm2;
		if (pmSubsystem == nullptr)
		{
			logWarn("PM2", "获取PM2子系统失败: 成员pm2为空，刷新后仍为空.");
			return false;
		}

		return true;
	}

	bool QSlotTransferCycleVTMWidgetPrivate::tryGetPmHasWafer(const std::shared_ptr<FortrendPMCavitySubsystem>& pmSubsystem, const char* pmName, bool& hasWafer) const
	{
		hasWafer = false;
		if (pmSubsystem == nullptr)
		{
			logWarn(pmName, "读取PM占片状态失败: pm为空.");
			return false;
		}

		auto pmKernel = pmSubsystem->getKernel();
		if (pmKernel == nullptr)
		{
			logWarn(pmName, "读取PM占片状态失败: pmKernel为空.");
			return false;
		}

		auto cassManager = pmKernel->getKernelModule<FortrendCassetteManager>();
		if (cassManager == nullptr)
		{
			logWarn(pmName, "读取PM占片状态失败: cassManager为空.");
				return false;
		}

		auto cassette = cassManager->getCassette(pmSubsystem.get());
		if (cassette == nullptr)
		{
			logWarn(pmName, "读取PM占片状态失败: cassette为空.");
			return false;
		}

		hasWafer = cassette->getMapping(1) == Cassette::Present;
		return true;
	}

	bool QSlotTransferCycleVTMWidgetPrivate::tryBuildPm2ScheduleSnapshot(PM2ScheduleSnapshot& snapshot)
	{
		snapshot = PM2ScheduleSnapshot{};
		if (wtr == nullptr)
		{
			logWarn("PM2", "构建PM2调度快照失败: WTR为空.");
			return false;
		}

		std::shared_ptr<FortrendPMCavitySubsystem> pm2Subsystem;
		if (!ensurePm2SubsystemReady(pm2Subsystem))
		{
			logWarn("PM2", "构建PM2调度快照失败: PM2子系统未就绪.");
			return false;
		}

		if (!tryGetPmHasWafer(pm2Subsystem, "PM2", snapshot.hasWaferPm))
		{
			return false;
		}

		snapshot.armAHasWafer = wtr->hasObject(0);
		snapshot.armBHasWafer = wtr->hasObject(1);

		auto pm2PendingTasksLocal = taskManager.getPMPendingTasks("PM2");
		snapshot.pm2PendingCount = static_cast<int>(pm2PendingTasksLocal.size());

		auto pm2CompletedTasksLocal = taskManager.getPMCompletedTasks("PM2");
		snapshot.pm2CompletedCount = static_cast<int>(pm2CompletedTasksLocal.size());

		auto loadlockReturnPendingTasks = taskManager.getLoadLockReturnPendingTasks();
		loadlockReturnPendingTasks.erase(
			std::remove_if(loadlockReturnPendingTasks.begin(), loadlockReturnPendingTasks.end(),
				[](const UnifiedWaferTask& task)
				{
					return task.target_pm != UnifiedWaferTask::PM2;
				}),
			loadlockReturnPendingTasks.end());
		snapshot.returnPendingCount = static_cast<int>(loadlockReturnPendingTasks.size());
		snapshot.pm2CraftInProgress =
			snapshot.hasWaferPm &&
			snapshot.pm2PendingCount > 0 &&
			snapshot.pm2CompletedCount == 0 &&
			snapshot.returnPendingCount == 0;

		snapshot.armAHasPending = snapshot.armAHasWafer &&
			std::any_of(pm2PendingTasksLocal.begin(), pm2PendingTasksLocal.end(), [](const UnifiedWaferTask& t) { return t.arm == 0; });
		snapshot.armBHasPending = snapshot.armBHasWafer &&
			std::any_of(pm2PendingTasksLocal.begin(), pm2PendingTasksLocal.end(), [](const UnifiedWaferTask& t) { return t.arm == 1; });

		const int craftTaskId = pm2_craft_task_id.load();
		if (craftTaskId >= 0)
		{
			const auto craftTaskIt = std::find_if(pm2PendingTasksLocal.begin(), pm2PendingTasksLocal.end(),
				[craftTaskId](const UnifiedWaferTask& task)
				{
					return task.taskId == craftTaskId;
				});
			if (craftTaskIt != pm2PendingTasksLocal.end())
			{
				snapshot.preferredPmArm = craftTaskIt->arm;
			}
		}

		if (snapshot.preferredPmArm < 0 && !loadlockReturnPendingTasks.empty())
		{
			snapshot.preferredPmArm = loadlockReturnPendingTasks.front().arm;
		}
		else if (snapshot.preferredPmArm < 0 && !pm2CompletedTasksLocal.empty())
		{
			snapshot.preferredPmArm = pm2CompletedTasksLocal.front().arm;
		}

		return true;
	}

	bool QSlotTransferCycleVTMWidgetPrivate::tryPlanImmediateRepick(const char* loadLockName,
		const LoadLockTaskSnapshot& snapshot,
		bool armAHasWafer,
		bool armBHasWafer,
		UnifiedWaferTask::Location requiredTargetPm,
		LLImmediateRepickState& plan)
	{
		plan.reset();
		if (!(armAHasWafer ^ armBHasWafer))
		{
			return false;
		}

		const int occupiedArm = armAHasWafer ? 0 : 1;
		const auto returnIt = std::find_if(snapshot.returnPendingTasks.begin(), snapshot.returnPendingTasks.end(),
			[occupiedArm, requiredTargetPm](const UnifiedWaferTask& task)
			{
				return task.arm == occupiedArm && task.target_pm == requiredTargetPm;
			});
		if (returnIt == snapshot.returnPendingTasks.end())
		{
			logInform(loadLockName, "固定手臂立即补取未触发: 未找到 occupiedArm=%d 且 target_pm=%d 对应回片任务",
				occupiedArm, static_cast<int>(requiredTargetPm));
			return false;
		}

		const auto pendingIt = std::find_if(snapshot.pendingTasks.begin(), snapshot.pendingTasks.end(),
			[occupiedArm](const UnifiedWaferTask& task) { return task.arm == occupiedArm; });
		if (pendingIt == snapshot.pendingTasks.end())
		{
			logInform(loadLockName, "固定手臂立即补取未触发: pending队列中未找到 occupiedArm=%d 对应任务", occupiedArm);
			return false;
		}

		plan.enabled = true;
		plan.taskId = pendingIt->taskId;
		plan.arm = pendingIt->arm;
		plan.slot = pendingIt->targetFeedingSlot;

		logInform(loadLockName,
			"固定手臂立即补取规划完成: occupiedArm=%d, repickArm=%d, repickSlot=%d, taskId=%d",
			occupiedArm, plan.arm, plan.slot, plan.taskId);
		return true;
	}

	bool QSlotTransferCycleVTMWidgetPrivate::shouldLlWaitForPm2Priority(const PM2ScheduleSnapshot& snapshot, int requestedLlArm, std::string& reason) const
	{
		if (!snapshot.hasWaferPm)
		{
			reason.clear();
			return false;
		}

		int expectedPmArm = snapshot.preferredPmArm;
		std::string waitReason = "PM2有片优先";

		if (snapshot.returnPendingCount > 0)
		{
			waitReason = "PM2待回片优先";
		}
		else if (snapshot.pm2CompletedCount > 0)
		{
			waitReason = "PM2已有完成片待取";
		}
		else if (!snapshot.armAHasWafer && !snapshot.armBHasWafer)
		{
			if (snapshot.pm2CraftInProgress)
			{
				if (snapshot.preferredPmArm >= 0 && requestedLlArm == snapshot.preferredPmArm)
				{
					reason =
						"reason=PM2加工中但请求预装手臂与未来回片臂冲突,必须保留回片臂" +
						std::string(", requestedLlArm=") + std::to_string(requestedLlArm) +
						", pm2Has=" + std::to_string(static_cast<int>(snapshot.hasWaferPm)) +
						", pm2Crafting=" + std::to_string(static_cast<int>(snapshot.pm2CraftInProgress)) +
						", armA_has=" + std::to_string(static_cast<int>(snapshot.armAHasWafer)) +
						", armA_pending=" + std::to_string(static_cast<int>(snapshot.armAHasPending)) +
						", armB_has=" + std::to_string(static_cast<int>(snapshot.armBHasWafer)) +
						", armB_pending=" + std::to_string(static_cast<int>(snapshot.armBHasPending)) +
						", pending=" + std::to_string(snapshot.pm2PendingCount) +
						", completed=" + std::to_string(snapshot.pm2CompletedCount) +
						", return_pending=" + std::to_string(snapshot.returnPendingCount) +
						", expectedPmArm=" + std::to_string(expectedPmArm);
					return true;
				}
				reason =
					"reason=PM2加工中且双手为空,允许LL预装下一片" +
					std::string(", requestedLlArm=") + std::to_string(requestedLlArm) +
					std::string(", pm2Has=") + std::to_string(static_cast<int>(snapshot.hasWaferPm)) +
					", pm2Crafting=" + std::to_string(static_cast<int>(snapshot.pm2CraftInProgress)) +
					", armA_has=" + std::to_string(static_cast<int>(snapshot.armAHasWafer)) +
					", armA_pending=" + std::to_string(static_cast<int>(snapshot.armAHasPending)) +
					", armB_has=" + std::to_string(static_cast<int>(snapshot.armBHasWafer)) +
					", armB_pending=" + std::to_string(static_cast<int>(snapshot.armBHasPending)) +
					", pending=" + std::to_string(snapshot.pm2PendingCount) +
					", completed=" + std::to_string(snapshot.pm2CompletedCount) +
					", return_pending=" + std::to_string(snapshot.returnPendingCount) +
					", expectedPmArm=" + std::to_string(expectedPmArm);
				return false;
			}
			waitReason = "PM2有片且双手为空,应先从PM2取片";
			if (expectedPmArm < 0)
			{
				expectedPmArm = 1;
			}
		}
		else if (!snapshot.armAHasWafer && snapshot.armBHasPending)
		{
			waitReason = "PM2应执行A取B放";
			expectedPmArm = 0;
		}
		else if (snapshot.armAHasPending && !snapshot.armBHasWafer)
		{
			waitReason = "PM2应执行B取A放";
			expectedPmArm = 1;
		}
		reason =
			"reason=" + waitReason +
			", pm2Has=" + std::to_string(static_cast<int>(snapshot.hasWaferPm)) +
			", pm2Crafting=" + std::to_string(static_cast<int>(snapshot.pm2CraftInProgress)) +
			", armA_has=" + std::to_string(static_cast<int>(snapshot.armAHasWafer)) +
			", armA_pending=" + std::to_string(static_cast<int>(snapshot.armAHasPending)) +
			", armB_has=" + std::to_string(static_cast<int>(snapshot.armBHasWafer)) +
			", armB_pending=" + std::to_string(static_cast<int>(snapshot.armBHasPending)) +
			", pending=" + std::to_string(snapshot.pm2PendingCount) +
			", completed=" + std::to_string(snapshot.pm2CompletedCount) +
			", return_pending=" + std::to_string(snapshot.returnPendingCount) +
			", expectedPmArm=" + std::to_string(expectedPmArm);
		return true;
	}
	
	bool QSlotTransferCycleVTMWidgetPrivate::canLlImmediateRepickWaitForPm2Craft(const PM2ScheduleSnapshot& snapshot, int repickArm) const
	{
		if (!snapshot.hasWaferPm)
		{
			return false;
		}

		if (!snapshot.pm2CraftInProgress)
		{
			return false;
		}

		if (snapshot.returnPendingCount > 0 || snapshot.pm2CompletedCount > 0)
		{
			return false;
		}

		if (snapshot.armAHasWafer || snapshot.armBHasWafer)
		{
			return false;
		}

		if (snapshot.preferredPmArm >= 0 && repickArm == snapshot.preferredPmArm)
		{
			return false;
		}

		return repickArm == 0 || repickArm == 1;
	}

	bool QSlotTransferCycleVTMWidgetPrivate::isRobotArmOccupiedForLlRequest(int targetArm, std::string& reason) const
	{
		if (targetArm < 0 || targetArm > 1)
		{
			reason = "targetArm非法";
			return true;
		}

		if (wtr == nullptr)
		{
			reason = "WTR为空";
			return true;
		}

		if (wtr->hasObject(targetArm))
		{
			reason = "目标手臂实物已有片";
			return true;
		}

		auto checkRequest = [&](const RobotTransferRequest& request, const char* requestName) -> bool
		{
			if (request.requested.load() && request.arm.load() == targetArm)
			{
				reason = std::string(requestName) + ".requested=1";
				return true;
			}
			return false;
		};

		auto checkExchange = [&](const RobotTransferRequest& request, const ExchangeInfo& info, const char* requestName) -> bool
		{
			if (request.requested.load())
			{
				const int getArm = info.getArm.load();
				const int putArm = info.putArm.load();
				if (getArm == targetArm || putArm == targetArm)
				{
					reason = std::string(requestName) + ".requested=1(getArm=" + std::to_string(getArm) +
						", putArm=" + std::to_string(putArm) + ")";
					return true;
				}
			}
			return false;
		};

		if (checkRequest(robot_get_from_lla, "robot_get_from_lla")) return true;
		if (checkRequest(robot_get_from_llb, "robot_get_from_llb")) return true;
		if (checkRequest(robot_put_to_lla, "robot_put_to_lla")) return true;
		if (checkRequest(robot_put_to_llb, "robot_put_to_llb")) return true;
		if (checkRequest(robot_put_to_pm1, "robot_put_to_pm1")) return true;
		if (checkRequest(robot_put_to_pm2, "robot_put_to_pm2")) return true;
		if (checkRequest(robot_put_to_pm3, "robot_put_to_pm3")) return true;
		if (checkRequest(robot_put_to_pm4, "robot_put_to_pm4")) return true;
		if (checkRequest(robot_get_from_pm1, "robot_get_from_pm1")) return true;
		if (checkRequest(robot_get_from_pm2, "robot_get_from_pm2")) return true;
		if (checkRequest(robot_get_from_pm3, "robot_get_from_pm3")) return true;
		if (checkRequest(robot_get_from_pm4, "robot_get_from_pm4")) return true;
		if (checkExchange(robot_exchange_pm1, exchange_info_pm1, "robot_exchange_pm1")) return true;
		if (checkExchange(robot_exchange_pm2, exchange_info_pm2, "robot_exchange_pm2")) return true;
		if (checkExchange(robot_exchange_pm3, exchange_info_pm3, "robot_exchange_pm3")) return true;
		if (checkExchange(robot_exchange_pm4, exchange_info_pm4, "robot_exchange_pm4")) return true;

		switch (robot_step)
		{
		case 1000:
			if (robot_get_from_lla.arm.load() == targetArm) { reason = "robot_step=1000(GET_FROM_LLA)"; return true; }
			break;
		case 1100:
			if (robot_get_from_llb.arm.load() == targetArm) { reason = "robot_step=1100(GET_FROM_LLB)"; return true; }
			break;
		case 2000:
			if (robot_put_to_pm1.arm.load() == targetArm) { reason = "robot_step=2000(PUT_TO_PM1)"; return true; }
			break;
		case 2100: 
			if (robot_put_to_pm2.arm.load() == targetArm) { reason = "robot_step=2100(PUT_TO_PM2)"; return true; }
			break;
		case 2200:
			if (robot_put_to_pm3.arm.load() == targetArm) { reason = "robot_step=2200(PUT_TO_PM3)"; return true; }
			break;
		case 2300:
			if (robot_put_to_pm4.arm.load() == targetArm) { reason = "robot_step=2300(PUT_TO_PM4)"; return true; }
			break;
		case 3000:
			if (robot_get_from_pm1.arm.load() == targetArm) { reason = "robot_step=3000(GET_FROM_PM1)"; return true; }
			break;
		case 3100:
			if (robot_get_from_pm2.arm.load() == targetArm) { reason = "robot_step=3100(GET_FROM_PM2)"; return true; }
			break;
		case 3200:
			if (robot_get_from_pm3.arm.load() == targetArm) { reason = "robot_step=3200(GET_FROM_PM3)"; return true; }
			break;
		case 3300:
			if (robot_get_from_pm4.arm.load() == targetArm) { reason = "robot_step=3300(GET_FROM_PM4)"; return true; }
			break;
		case 4000:
			if (robot_put_to_lla.arm.load() == targetArm) { reason = "robot_step=4000(PUT_TO_LLA)"; return true; }
			break;
		case 4100:
			if (robot_put_to_llb.arm.load() == targetArm) { reason = "robot_step=4100(PUT_TO_LLB)"; return true; }
			break;
		case 5000:
			if (exchange_info_pm1.getArm.load() == targetArm || exchange_info_pm1.putArm.load() == targetArm) { reason = "robot_step=5000(EXCHANGE_PM1)"; return true; }
			break;
		case 5100:
			if (exchange_info_pm2.getArm.load() == targetArm || exchange_info_pm2.putArm.load() == targetArm) { reason = "robot_step=5100(EXCHANGE_PM2)"; return true; }
			break;
		case 5200:
			if (exchange_info_pm3.getArm.load() == targetArm || exchange_info_pm3.putArm.load() == targetArm) { reason = "robot_step=5200(EXCHANGE_PM3)"; return true; }
			break;
		case 5300:
			if (exchange_info_pm4.getArm.load() == targetArm || exchange_info_pm4.putArm.load() == targetArm) { reason = "robot_step=5300(EXCHANGE_PM4)"; return true; }
			break;
		default:
			break;
		}

		reason.clear();
		return false;
	}

	bool QSlotTransferCycleVTMWidgetPrivate::shouldLlWaitForWtrFingerQuery(std::string& reason) const
	{
		auto checkRequest = [&](const RobotTransferRequest& request, const char* requestName) -> bool
		{
			if (request.requested.load())
			{
				reason = std::string(requestName) + ".requested=1";
				return true;
			}
			return false;
		};

		auto checkExchange = [&](const RobotTransferRequest& request, const ExchangeInfo& info, const char* requestName) -> bool
		{
			if (request.requested.load())
			{
				reason = std::string(requestName) + ".requested=1(getArm=" +
					std::to_string(info.getArm.load()) + ", putArm=" +
					std::to_string(info.putArm.load()) + ")";
				return true;
			}
			return false;
		};

		if (robot_step != 10)
		{
			reason = "robot_step=" + std::to_string(robot_step);
			return true;
		}

		if (checkRequest(robot_get_from_lla, "robot_get_from_lla")) return true;
		if (checkRequest(robot_get_from_llb, "robot_get_from_llb")) return true;
		if (checkRequest(robot_put_to_lla, "robot_put_to_lla")) return true;
		if (checkRequest(robot_put_to_llb, "robot_put_to_llb")) return true;
		if (checkRequest(robot_put_to_pm1, "robot_put_to_pm1")) return true;
		if (checkRequest(robot_put_to_pm2, "robot_put_to_pm2")) return true;
		if (checkRequest(robot_put_to_pm3, "robot_put_to_pm3")) return true;
		if (checkRequest(robot_put_to_pm4, "robot_put_to_pm4")) return true;
		if (checkRequest(robot_get_from_pm1, "robot_get_from_pm1")) return true;
		if (checkRequest(robot_get_from_pm2, "robot_get_from_pm2")) return true;
		if (checkRequest(robot_get_from_pm3, "robot_get_from_pm3")) return true;
		if (checkRequest(robot_get_from_pm4, "robot_get_from_pm4")) return true;
		if (checkExchange(robot_exchange_pm1, exchange_info_pm1, "robot_exchange_pm1")) return true;
		if (checkExchange(robot_exchange_pm2, exchange_info_pm2, "robot_exchange_pm2")) return true;
		if (checkExchange(robot_exchange_pm3, exchange_info_pm3, "robot_exchange_pm3")) return true;
		if (checkExchange(robot_exchange_pm4, exchange_info_pm4, "robot_exchange_pm4")) return true;

		reason.clear();
		return false;
	}

	bool QSlotTransferCycleVTMWidgetPrivate::shouldWaitBeforeStartWtrFingerQuery(
		const std::shared_ptr<FortrendSunwayRobotSubsystem>& wtr, std::string& reason) const
	{
		if (!wtr)
		{
			reason = "WTR missing";
			return true;
		}

		if (wtr->getState() != IKernelSubSystem::State::SUB_NORMAL)
		{
			reason = "WTR state not normal";
			return true;
		}

		if (wtr->isBusy())
		{
			reason = "WTR busy";
			return true;
		}

		return shouldLlWaitForWtrFingerQuery(reason);
	}

	void QSlotTransferCycleVTMWidgetPrivate::UpdatePmSubTransferDatas(std::string pmName)
	{
		if(pmName == "PM1")
		{
			pmPendingTasks = taskManager.getPMPendingTasks(pmName);
			pmCompletedTasks = taskManager.getPMCompletedTasks(pmName);
			pmProgressingTasks = taskManager.getPMProcessTasks(pmName);
		}
		else if (pmName == "PM2")
		{
			pm2PendingTasks = taskManager.getPMPendingTasks(pmName);
			pm2CompletedTasks = taskManager.getPMCompletedTasks(pmName);
			pm2ProgressingTasks = taskManager.getPMProcessTasks(pmName);
		}
		else if (pmName == "PM3")
		{
			pm3PendingTasks = taskManager.getPMPendingTasks(pmName);
			pm3CompletedTasks = taskManager.getPMCompletedTasks(pmName);
			pm3ProgressingTasks = taskManager.getPMProcessTasks(pmName);
		}
		else
		{
			pm4PendingTasks = taskManager.getPMPendingTasks(pmName);
			pm4CompletedTasks = taskManager.getPMCompletedTasks(pmName);
			pm4ProgressingTasks = taskManager.getPMProcessTasks(pmName);
		}
	}

	QSlotTransferCycleVTMWidgetPrivate::LoadLockTaskSnapshot QSlotTransferCycleVTMWidgetPrivate::buildLoadLockTaskSnapshot(const char* loadLockName) const
	{
		LoadLockTaskSnapshot snapshot;
		snapshot.pendingTasks = taskManager.getLoadLockPendingTasks(loadLockName);
		snapshot.completedTasks = taskManager.getLoadLockCompletedTasks(loadLockName);
		snapshot.returnPendingTasks = taskManager.getLoadLockReturnPendingTasks(loadLockName);
		snapshot.returnCompletedTasks = taskManager.getLoadLockReturnCompletedTasks(loadLockName);
		return snapshot;
	}

	QSlotTransferCycleVTMWidgetPrivate::PmTaskSnapshot QSlotTransferCycleVTMWidgetPrivate::buildPmTaskSnapshot(const std::string& pmName) const
	{
		PmTaskSnapshot snapshot;
		snapshot.pendingTasks = taskManager.getPMPendingTasks(pmName);
		snapshot.completedTasks = taskManager.getPMCompletedTasks(pmName);
		snapshot.progressingTasks = taskManager.getPMProcessTasks(pmName);
		return snapshot;
	}

	bool QSlotTransferCycleVTMWidgetPrivate::runPmRecipeMotorOnUiThread(int pmIndex, const char* stationName, const std::string& processName, std::atomic<int>& autoStep)
	{
		auto pmWidget = QPmRecipeWidget::instance(kernel);
		if (pmWidget == nullptr)
		{
			logFailed(stationName, Poco::format("无法获取到工艺界面实例，无法执行工艺流程， %s：%d", processName, autoStep.load()));
			autoStep.store(10);
			return false;
		}

		const bool isUiThread = (QThread::currentThread() == pmWidget->thread());
		if (isUiThread)
		{
			pmWidget->startPmMotorRun(pmIndex);
		}
		else if (!QMetaObject::invokeMethod(pmWidget, "startPmMotorRunInvoke", Qt::BlockingQueuedConnection, Q_ARG(int, pmIndex)))
		{
			logFailed(stationName, Poco::format("切换到UI线程启动工艺失败， %s：%d", processName, autoStep.load()));
			autoStep.store(10);
			return false;
		}

		while (!stopRequested)
		{
			bool isRunning = false;
			bool hasError = false;
			QString errorMessage;
			if (isUiThread)
			{
				isRunning = pmWidget->isPmMotorRunning(pmIndex);
				hasError = pmWidget->hasPmMotorError(pmIndex);
				if (hasError)
				{
					errorMessage = QString::fromStdString(pmWidget->getPmMotorError(pmIndex));
				}
			}
			else if (!QMetaObject::invokeMethod(pmWidget, "isPmMotorRunningInvoke", Qt::BlockingQueuedConnection, Q_RETURN_ARG(bool, isRunning), Q_ARG(int, pmIndex))
				|| !QMetaObject::invokeMethod(pmWidget, "hasPmMotorErrorInvoke", Qt::BlockingQueuedConnection, Q_RETURN_ARG(bool, hasError), Q_ARG(int, pmIndex))
				|| (hasError && !QMetaObject::invokeMethod(pmWidget, "getPmMotorErrorQString", Qt::BlockingQueuedConnection, Q_RETURN_ARG(QString, errorMessage), Q_ARG(int, pmIndex))))
			{
				logFailed(stationName, Poco::format("读取工艺运行状态失败， %s：%d", processName, autoStep.load()));
				autoStep.store(10);
				return false;
			}

			if (!isRunning)
			{
				if (hasError)
				{
					logFailed(stationName, Poco::format("工艺执行失败: %s， %s：%d", errorMessage.toStdString(), processName, autoStep.load()));
					return false;
				}
				return true;
			}

			Sleep(100);
		}

		return true;
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
					if (efem_robot_mutex.unlock_if_owned_by_current_thread()) {
						logWarn("EFEM", "重置请求到达，EFEM线程已主动释放上下料锁.");
					}
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
					Sleep(500);
					const auto efemReturnCompletedSnapshot = taskManager.getEfemRuturnCompletedTasks();
					// 检查循环完成状态
					if (efemReturnCompletedSnapshot.size() == originTaskSize && (originTaskSize != 0))
					{
						if (!efemReturnCompletedSnapshot.empty())
						{
							if (efemReturnCompletedSnapshot.at(0).source == UnifiedWaferTask::Location::LP1)
							{
								if (!lp1_cycle_one_time_finished)
								{
									logWarn("cycle", "当前一次lp1循环完成.");
									lp1_cycle_one_time_finished = true;//一次lp1循环完成
								}
							}
							else if (efemReturnCompletedSnapshot.at(0).source == UnifiedWaferTask::Location::LP2)
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
					const auto efemReturnPendingSnapshot = taskManager.getEfemRuturnPendingTasks();
					const auto efemPendingSnapshot = taskManager.getEfemPendingTasks();
					auto efemReturnPendingLLATasks = taskManager.getTasksByLocation(efemReturnPendingSnapshot, UnifiedWaferTask::Location::LLA);
					auto efemReturnPendingLLBTasks = taskManager.getTasksByLocation(efemReturnPendingSnapshot, UnifiedWaferTask::Location::LLB);
					auto efemPendingLLATasks = taskManager.getTasksByLocation(efemPendingSnapshot, UnifiedWaferTask::Location::LLA);
					auto efemPendingLLBTasks = taskManager.getTasksByLocation(efemPendingSnapshot, UnifiedWaferTask::Location::LLB);

					// 处理下料请求（优先）
					if (tool_allow_put_wafer_LLA || efemReturnPendingLLATasks.size() > 0) {

						if (!efem_robot_mutex.is_locked())
						{
							efem_robot_mutex.lock();
						}
						current_loadlock = "LLA";
						efem_auto_step = 200;  // 下料流程
					}
					else if (tool_allow_put_wafer_LLB || efemReturnPendingLLBTasks.size() > 0) {
						if (!efem_robot_mutex.is_locked())
						{
							efem_robot_mutex.lock();
						}
						current_loadlock = "LLB";
						efem_auto_step = 200;  // 下料流程
					}
					// 处理上料请求
					else if (tool_allow_get_wafer_LLA || efemPendingLLATasks.size() > 0) {
						if (!efem_robot_mutex.is_locked())
						{
							efem_robot_mutex.lock();
						}
						current_loadlock = "LLA";
						efem_auto_step = 100;  // 上料流程
					}
					else if (tool_allow_get_wafer_LLB || efemPendingLLBTasks.size() > 0) {
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
					//logWarn("EFEM", "efem_auto_step:100,给%s上料 Lock thread...", current_loadlock.c_str());
					std::vector<UnifiedWaferTask> currentLoadLockUnknownTasks;
					if (current_loadlock == "LLA")
					{
						currentLoadLockUnknownTasks = taskManager.getEfemUnkownStatusLLATasks();
						lk = lk1;
					}
					else
					{
						currentLoadLockUnknownTasks = taskManager.getEfemUnkownStatusLLBTasks();
						lk = lk2;
					}

					if (currentLoadLockUnknownTasks.size() > 0)
					{
						elp = currentLoadLockUnknownTasks.at(0).source == UnifiedWaferTask::Location::LP1 ? elp1 : elp2;
						logWarn("EFEM", "step:100,当前lk:%s", lk->getName().c_str());

						//UI中确定的数据，elp就是确定的
						//lk = efemUnkownStatusTasks.at(0).target == UnifiedWaferTask::Location::LLA ? lk1 : lk2;

						if (elp == nullptr && lk == nullptr)
						{
							logFailedExcuteCommandHasError(elp->getName(), "elp or lk is nullptr", efem_process_name, efem_auto_step);
						}

						if (elp->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
							if (isSimulationModeEnabled())
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
						logWarn("EFEM", "step:100, 当前 %s 已经没有 UNKNOWN 状态任务，取消上料请求", lk->getName().c_str());
						if (current_loadlock == "LLA") {
							tool_allow_get_wafer_LLA = false;
						} else {
							tool_allow_get_wafer_LLB = false;
						}
						efem_robot_mutex.unlock();
						efem_auto_step = 11; // 继续监控上料请求，避免死循环在100
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
							const auto efemAllocationLlaInitSnapshot = taskManager.getEfemUnkownStatusLLATasks();
							if(efemAllocationLlaInitSnapshot.size() > 0)
							{
								int allocatedTaskIndex = 0;
								for (int i = 0; i < lkmaps.size() && allocatedTaskIndex < count && allocatedTaskIndex < efemAllocationLlaInitSnapshot.size(); i++)
								{
									if (lkmaps[i] == Cassette::Empty) //空片的槽号
									{
										taskManager.updateTaskStatus(efemAllocationLlaInitSnapshot[allocatedTaskIndex].taskId, UnifiedWaferTask::TaskType::EFEM_TRANSFER,
											UnifiedWaferTask::Status::QUEUED);
										allocatedTaskIndex++;
									}
								}
							}
						}
						else if(lk->getName() == "LLB")
						{
							const auto efemAllocationLlbInitSnapshot = taskManager.getEfemUnkownStatusLLBTasks();

							if(efemAllocationLlbInitSnapshot.size() > 0)
							{
								int allocatedTaskIndex = 0;
								for (int i = 0; i < lkmaps.size() && allocatedTaskIndex < count && allocatedTaskIndex < efemAllocationLlbInitSnapshot.size(); i++)
								{
									if (lkmaps[i] == Cassette::Empty) //空片的槽号
									{
										taskManager.updateTaskStatus(efemAllocationLlbInitSnapshot[allocatedTaskIndex].taskId, UnifiedWaferTask::TaskType::EFEM_TRANSFER,
											UnifiedWaferTask::Status::QUEUED);
										allocatedTaskIndex++;
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
					const auto efemPendingSnapshot = taskManager.getTasksByLocation(
						taskManager.getEfemPendingTasks(),
						current_loadlock == "LLA" ? UnifiedWaferTask::Location::LLA : UnifiedWaferTask::Location::LLB);
					
					if (efemPendingSnapshot.size() == 1)
					{
						if (isSimulationModeEnabled())
						{
							efem_auto_step = 3000;
						}
						else
						{
							efem_auto_step = 115;//单取LP单放LK
						}				
					}
					else if (efemPendingSnapshot.size() >= 2)
					{
						if (isSimulationModeEnabled())
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
							// 2026-1-09  下料完成步骤去执行关lp..
#if 0

							if (isSimulationModeEnabled())
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
						const auto efemPendingSnapshot = taskManager.getEfemPendingTasks();
						if (efemPendingSnapshot.empty())
						{
							logFailedExcuteCommandHasError(ewtr->getName(), "EFEM待上料任务为空", efem_process_name, efem_auto_step);
							break;
						}
						auto cmd = ewtr->createGetCommand(elp, 1, efemPendingSnapshot.front().sourceSlot);//1:1手，2：2手
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
							//加日志
							logFailedExcuteCommandHasError(ealigner->getName(), "寻边器位置1不为空", efem_process_name, efem_auto_step);
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
						const auto efemPendingSnapshot = taskManager.getEfemPendingTasks();
						if (efemPendingSnapshot.empty())
						{
							logFailedExcuteCommandHasError(ewtr->getName(), "EFEM待上料任务为空", efem_process_name, efem_auto_step);
							break;
						}
						const auto currentTask = efemPendingSnapshot.front();
						auto cmd2 = ewtr->createPutCommand(lk, 1, currentTask.targetFeedingSlot);
						ewtr->startCommand(cmd2);
						cmd2->wait();
						if (cmd2->hasError())
						{
							logFailedExcuteCommandHasError(ewtr->getName(), "放LL晶圆", efem_process_name, efem_auto_step);
						}
						else
						{
							//更新指定ID task状态改变，转移到其他状态下的队列中
							taskManager.updateTaskStatus(currentTask.taskId, UnifiedWaferTask::TaskType::EFEM_TRANSFER, UnifiedWaferTask::Status::COMPLETED);
							taskManager.updateTaskStatus(currentTask.taskId, UnifiedWaferTask::TaskType::LOADLOCK_TRANSFER, UnifiedWaferTask::Status::QUEUED);
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
					const auto efemPendingSnapshot = taskManager.getEfemPendingTasks();
					if (efemPendingSnapshot.size() < 2)
					{
						logError("EFEM", "efem_auto_step:150, efemPendingTasks size < 2, size=%d", efemPendingSnapshot.size());
						efem_auto_step = 110;
						break;
					}
					elp = efemPendingSnapshot.at(0).source == UnifiedWaferTask::Location::LP1 ? elp1 : elp2;

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
					const auto efemPendingSnapshot = taskManager.getEfemPendingTasks();
					if (efemPendingSnapshot.size() < 2)
					{
						logError("EFEM", "efem_auto_step:151, efemPendingTasks size < 2, size=%d", efemPendingSnapshot.size());
						efem_auto_step = 110;
						break;
					}
					elp = efemPendingSnapshot.at(1).source == UnifiedWaferTask::Location::LP1 ? elp1 : elp2;

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
					const auto efemPendingSnapshot = taskManager.getEfemPendingTasks();
					if (efemPendingSnapshot.size() < 2)
					{
						logError("EFEM", "efem_auto_step:152, efemPendingTasks size < 2, size=%d", efemPendingSnapshot.size());
						efem_auto_step = 110;
						break;
					}
					const auto firstTask = efemPendingSnapshot.at(0);
					elp = firstTask.source == UnifiedWaferTask::Location::LP1 ? elp1 : elp2;
					if (elp && elp->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto cmd = ewtr->createGetCommand(elp, 1, firstTask.sourceSlot);
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
					const auto efemPendingSnapshot = taskManager.getEfemPendingTasks();
					if (efemPendingSnapshot.size() < 2)
					{
						logError("EFEM", "efem_auto_step:154, efemPendingTasks size < 2, size=%d", efemPendingSnapshot.size());
						efem_auto_step = 110;
						break;
					}
					const auto secondTask = efemPendingSnapshot.at(1);
					elp = secondTask.source == UnifiedWaferTask::Location::LP1 ? elp1 : elp2;
					if (elp && elp->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto cmd = ewtr->createGetCommand(elp, 2, secondTask.sourceSlot);
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
					const auto efemPendingSnapshot = taskManager.getEfemPendingTasks();
					if (efemPendingSnapshot.size() < 2)
					{
						logError("EFEM", "efem_auto_step:155, efemPendingTasks size < 2, size=%d", efemPendingSnapshot.size());
						efem_auto_step = 110;
						break;
					}
					// 缓存两个任务的完整信息，避免case 157更新状态后数组变化导致访问错误
					cached_task_first = efemPendingSnapshot.at(0);  // 第一片料信息
					cached_task_second = efemPendingSnapshot.at(1); // 第二片料信息
								
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
					// 使用缓存的第二个任务信息：放第二片料（关键修复.）
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

					const auto loadLockAReturnCompletedSnapshot = taskManager.getLoadLockReturnCompletedTasks("LLA");
					const auto loadLockBReturnCompletedSnapshot = taskManager.getLoadLockReturnCompletedTasks("LLB");
					
					if (current_loadlock == "LLA")
					{
						for (const auto& task : loadLockAReturnCompletedSnapshot)
						{
							taskManager.updateTaskStatus(task.taskId, UnifiedWaferTask::EFEM_RETURN, UnifiedWaferTask::QUEUED);
						}
					}
					else
					{
						for (const auto& task : loadLockBReturnCompletedSnapshot)
						{
							taskManager.updateTaskStatus(task.taskId, UnifiedWaferTask::EFEM_RETURN, UnifiedWaferTask::QUEUED);
						}
					}

					// 防止数组越界
					const auto efemReturnPendingSnapshot = taskManager.getEfemRuturnPendingTasks();
					if (efemReturnPendingSnapshot.empty())
					{
						logError("EFEM", "step:200, efemReturnPendingTasks size < 1, size=%d", efemReturnPendingSnapshot.size());
						efem_auto_step = 10;
						break;
					}
					const auto currentTask = efemReturnPendingSnapshot.front();
					elp = currentTask.source == UnifiedWaferTask::Location::LP1 ? elp1 : elp2;//下料，实际源头和目标互换
					lk = currentTask.target == UnifiedWaferTask::Location::LLA ? lk1 : lk2;

					if (elp == nullptr || lk == nullptr)
					{
						logError("EFEM", "elp or lk is empty");

						logFailedExcuteCommandHasError(elp->getName(), "elp or lk is empty", efem_process_name, efem_auto_step);
						efem_auto_step = 10;
						break;
					}

					if (isSimulationModeEnabled())
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
					const auto efemReturnPendingSnapshot = taskManager.getEfemRuturnPendingTasks();

					if (efemReturnPendingSnapshot.size() == 1)
					{
						if (isSimulationModeEnabled())
						{
							efem_auto_step = 4000;
						}
						else
						{
							efem_auto_step = 240; //单取LK单放LP
						}
						
					}
					else if (efemReturnPendingSnapshot.size() >= 2)
					{
						if (isSimulationModeEnabled())
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
						const auto efemReturnPendingSnapshotAfterUpdate = taskManager.getEfemRuturnPendingTasks();
						const auto efemReturnCompletedSnapshotAfterUpdate = taskManager.getEfemRuturnCompletedTasks();

						if (isSimulationModeEnabled())
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

							if (efemReturnPendingSnapshotAfterUpdate.size() == 0 && efemReturnCompletedSnapshotAfterUpdate.size() == originTaskSize)
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
						const auto efemReturnPendingSnapshot = taskManager.getEfemRuturnPendingTasks();
						if (efemReturnPendingSnapshot.empty())
						{
							logFailedExcuteCommandHasError(ewtr->getName(), "EFEM待下料任务为空", efem_process_name, efem_auto_step);
							break;
						}
						auto cmd1 = ewtr->createGetCommand(lk, 1, efemReturnPendingSnapshot.front().targetBlankingSlot);
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
					if (ewtr && ewtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						const auto efemReturnPendingSnapshot = taskManager.getEfemRuturnPendingTasks();
						if (efemReturnPendingSnapshot.empty())
						{
							logFailedExcuteCommandHasError(ewtr->getName(), "efemReturnPendingTasks size empty", efem_process_name, efem_auto_step);
							break;
						}
						const auto currentTask = efemReturnPendingSnapshot.front();
						auto cmd2 = ewtr->createPutCommand(elp, 1, currentTask.sourceSlot);
						ewtr->startCommand(cmd2);
						cmd2->wait();
						if (cmd2->hasError())
						{
							logFailedExcuteCommandHasError(ewtr->getName(), "放LP晶圆", efem_process_name, efem_auto_step);
							efem_auto_step = 201;
						}
						else
						{
							taskManager.updateTaskStatus(currentTask.taskId, UnifiedWaferTask::TaskType::EFEM_RETURN, UnifiedWaferTask::Status::COMPLETED);

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
							//考虑多片情况，返回任务数量是初始配置的数量，且无待下料任务
							const auto efemUnknownSnapshot = taskManager.getEfemUnkownStatusTasks();
							const auto efemReturnPendingSnapshotAfterUpdate = taskManager.getEfemRuturnPendingTasks();
							const auto efemReturnCompletedSnapshot = taskManager.getEfemRuturnCompletedTasks();
							if (efemUnknownSnapshot.empty() && efemReturnPendingSnapshotAfterUpdate.empty() && efemReturnCompletedSnapshot.size() == originTaskSize)
							{
								if (!efemReturnCompletedSnapshot.empty())
								{
									if (efemReturnCompletedSnapshot.at(0).source == UnifiedWaferTask::Location::LP1)
									{
										logWarn(ewtr->getName().c_str(), "cycle end efemReturnCompletedTasks=%d", efemReturnCompletedSnapshot.size());

										current_lp_cycle = is_lp1_cycle = true;
										logWarn(ewtr->getName().c_str(), "is_lp1_cycle true");
									}
									else if (efemReturnCompletedSnapshot.at(0).source == UnifiedWaferTask::Location::LP2)
									{
										logWarn(ewtr->getName().c_str(), "cycle end efemReturnCompletedTasks=%d", efemReturnCompletedSnapshot.size());

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

							//要加....
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
					const auto efemReturnPendingSnapshot = taskManager.getEfemRuturnPendingTasks();
					// 防止数组越界：双取LK双放LP需要至少两个任务
					if (efemReturnPendingSnapshot.size() < 2)
					{
						logError("EFEM", "efem_auto_step:250, efemReturnPendingTasks size < 2, size=%d", efemReturnPendingSnapshot.size());
						efem_auto_step = 10;
						break;
					}
					std::shared_ptr<EFEMLPSubsystem> elp = efemReturnPendingSnapshot.at(0).source == UnifiedWaferTask::Location::LP1 ? elp1 : elp2;
					std::shared_ptr<EFEMLPSubsystem> elp2put = efemReturnPendingSnapshot.at(1).source == UnifiedWaferTask::Location::LP1 ? elp1 : elp2;

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
					const auto efemReturnPendingSnapshot = taskManager.getEfemRuturnPendingTasks();
					// 防止数组越界
					if (efemReturnPendingSnapshot.size() < 2)
					{
						logError("EFEM", "efem_auto_step:251, efemReturnPendingTasks size < 2, size=%d", efemReturnPendingSnapshot.size());
						efem_auto_step = 10;
						break;
					}
					std::shared_ptr<FortrendLoadLockSubsystem> lk1 = efemReturnPendingSnapshot.at(0).target == UnifiedWaferTask::Location::LLA ? this->lk1 : this->lk2;
					std::shared_ptr<FortrendLoadLockSubsystem> lk2 = efemReturnPendingSnapshot.at(1).target == UnifiedWaferTask::Location::LLA ? this->lk1 : this->lk2;

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
					const auto efemReturnPendingSnapshot = taskManager.getEfemRuturnPendingTasks();
					if (efemReturnPendingSnapshot.size() < 2)
					{
						logError("EFEM", "efem_auto_step:253, efemReturnPendingTasks size < 2, size=%d", efemReturnPendingSnapshot.size());
						efem_auto_step = 10;
						break;
					}
					std::shared_ptr<FortrendLoadLockSubsystem> lk1 = efemReturnPendingSnapshot.at(0).target == UnifiedWaferTask::Location::LLA ? this->lk1 : this->lk2;

					if (ewtr && ewtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto cmd1 = ewtr->createGetCommand(lk1, 1, efemReturnPendingSnapshot.at(0).targetBlankingSlot);
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
					const auto efemReturnPendingSnapshot = taskManager.getEfemRuturnPendingTasks();
					// 防止数组越界
					if (efemReturnPendingSnapshot.size() < 2)
					{
						logError("EFEM", "efem_auto_step:254, efemReturnPendingTasks size < 2, size=%d", efemReturnPendingSnapshot.size());
						efem_auto_step = 10;
						break;
					}
					std::shared_ptr<FortrendLoadLockSubsystem> lk2 = efemReturnPendingSnapshot.at(1).target == UnifiedWaferTask::Location::LLA ? this->lk1 : this->lk2;

					if (ewtr && ewtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto cmd1 = ewtr->createGetCommand(lk2, 2, efemReturnPendingSnapshot.at(1).targetBlankingSlot);
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
					const auto efemReturnPendingSnapshot = taskManager.getEfemRuturnPendingTasks();
					// 防止数组越界
					if (efemReturnPendingSnapshot.size() < 2)
					{
						logError("EFEM", "efem_auto_step:256, efemReturnPendingTasks size < 2, size=%d", efemReturnPendingSnapshot.size());
						efem_auto_step = 10;
						break;
					}
					if (ewtr && ewtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						std::shared_ptr<EFEMLPSubsystem> elp = efemReturnPendingSnapshot.at(0).source == UnifiedWaferTask::Location::LP1 ? elp1 : elp2;
						std::shared_ptr<EFEMLPSubsystem> elp2put = efemReturnPendingSnapshot.at(1).source == UnifiedWaferTask::Location::LP1 ? elp1 : elp2;

						auto cmd3 = ewtr->createPutCommand(elp, 1, efemReturnPendingSnapshot.at(0).sourceSlot);
						ewtr->startCommand(cmd3);

						auto cmd4 = ewtr->createPutCommand(elp2put, 2, efemReturnPendingSnapshot.at(1).sourceSlot);
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
					const auto efemReturnPendingSnapshot = taskManager.getEfemRuturnPendingTasks();
					// 防止数组越界
					if (efemReturnPendingSnapshot.size() < 2)
					{
						logError("EFEM", "efem_auto_step:257, efemReturnPendingTasks size < 2, size=%d", efemReturnPendingSnapshot.size());
						efem_auto_step = 201;
						break;
					}
					const auto firstTask = efemReturnPendingSnapshot.at(0);
					const auto secondTask = efemReturnPendingSnapshot.at(1);
					taskManager.updateTaskStatus(firstTask.taskId, UnifiedWaferTask::TaskType::EFEM_RETURN, UnifiedWaferTask::Status::COMPLETED);
					taskManager.updateTaskStatus(secondTask.taskId, UnifiedWaferTask::TaskType::EFEM_RETURN, UnifiedWaferTask::Status::COMPLETED);

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
					// 原先条件： efemUnkownStatusTasks.size() == 0 && efemPendingTasks.size() ==0 && efemReturnPendingTasks.size() == 2 不对...


					const auto efemUnknownSnapshot = taskManager.getEfemUnkownStatusTasks();
					const auto efemReturnPendingSnapshotAfterUpdate = taskManager.getEfemRuturnPendingTasks();
					const auto efemReturnCompletedSnapshot = taskManager.getEfemRuturnCompletedTasks();

					if (efemUnknownSnapshot.empty() && efemReturnPendingSnapshotAfterUpdate.empty() && efemReturnCompletedSnapshot.size() == originTaskSize)
					{
						if (efemReturnCompletedSnapshot.size() >= 2)
						{

							//来自同一个lp1
							if (efemReturnCompletedSnapshot.at(0).source == UnifiedWaferTask::Location::LP1 && efemReturnCompletedSnapshot.at(0).source == efemReturnCompletedSnapshot.at(1).source)
							{
								logWarn(ewtr->getName().c_str(), "cycle end efemUnkownStatusTasks=%d efemReturnCompletedTasks=%d",
									efemUnknownSnapshot.size(), efemReturnCompletedSnapshot.size());
								current_lp_cycle = is_lp1_cycle = true;
								logWarn(ewtr->getName().c_str(), "is_lp1_cycle true");
							}
							//来自同一个lp2
							else if (efemReturnCompletedSnapshot.at(0).source == UnifiedWaferTask::Location::LP2 && efemReturnCompletedSnapshot.at(0).source == efemReturnCompletedSnapshot.at(1).source)
							{
								logWarn(ewtr->getName().c_str(), "cycle end efemUnkownStatusTasks=%d efemReturnCompletedTasks=%d",
									efemUnknownSnapshot.size(), efemReturnCompletedSnapshot.size());
								current_lp_cycle = is_lp2_cycle = true;
								logWarn(ewtr->getName().c_str(), "is_lp2_cycle true");
							}

							//来自不同的lp，LP1
							else if (efemReturnCompletedSnapshot.at(0).source == UnifiedWaferTask::Location::LP1 && efemReturnCompletedSnapshot.at(0).source != efemReturnCompletedSnapshot.at(1).source)
							{
								logWarn(ewtr->getName().c_str(), "cycle end efemUnkownStatusTasks=%d efemReturnCompletedTasks=%d",
									efemUnknownSnapshot.size(), efemReturnCompletedSnapshot.size());
								current_lp_cycle = is_lp1_cycle = true;
								logWarn(ewtr->getName().c_str(), "is_lp1_cycle true");
							}
							//来自不同的lp,LP2
							else if (efemReturnCompletedSnapshot.at(0).source == UnifiedWaferTask::Location::LP2 && efemReturnCompletedSnapshot.at(0).source != efemReturnCompletedSnapshot.at(1).source)
							{
								logWarn(ewtr->getName().c_str(), "cycle end efemUnkownStatusTasks=%d efemReturnCompletedTasks=%d",
									efemUnknownSnapshot.size(), efemReturnCompletedSnapshot.size());
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
					const auto efemPendingSnapshot = taskManager.getEfemPendingTasks();
					if (efemPendingSnapshot.empty())
					{
						logError("SimCycle", "EFEM单上料模拟时待上料任务为空");
						efem_auto_step = 110;
						break;
					}
					const auto currentTask = efemPendingSnapshot.front();
					taskManager.updateTaskStatus(currentTask.taskId, UnifiedWaferTask::TaskType::EFEM_TRANSFER, UnifiedWaferTask::Status::COMPLETED);
					taskManager.updateTaskStatus(currentTask.taskId, UnifiedWaferTask::TaskType::LOADLOCK_TRANSFER, UnifiedWaferTask::Status::QUEUED);

					Sleep(500);
					logWarn("SimCycle", "EFEM给LL单上料完成.");
					efem_auto_step = 110; //跳转到110

				}
				break;

				#pragma endregion

				#pragma region 模拟EFEM给TOOLD双上料
				case 3001:
				{
					const auto efemPendingSnapshot = taskManager.getEfemPendingTasks();
					if (efemPendingSnapshot.size() < 2)
					{
						logError("SimCycle", "EFEM双上料模拟时待上料任务不足, size=%d", efemPendingSnapshot.size());
						efem_auto_step = 110;
						break;
					}
					const auto firstTask = efemPendingSnapshot.at(0);
					const auto secondTask = efemPendingSnapshot.at(1);
					taskManager.updateTaskStatus(firstTask.taskId, UnifiedWaferTask::TaskType::EFEM_TRANSFER, UnifiedWaferTask::Status::COMPLETED);
					taskManager.updateTaskStatus(firstTask.taskId, UnifiedWaferTask::TaskType::LOADLOCK_TRANSFER, UnifiedWaferTask::Status::QUEUED);
					taskManager.updateTaskStatus(secondTask.taskId, UnifiedWaferTask::TaskType::EFEM_TRANSFER, UnifiedWaferTask::Status::COMPLETED);
					taskManager.updateTaskStatus(secondTask.taskId, UnifiedWaferTask::TaskType::LOADLOCK_TRANSFER, UnifiedWaferTask::Status::QUEUED);
					Sleep(500);
					logWarn("SimCycle", "EFEM给LL双上料完成.");
					efem_auto_step = 110;
				}
				break;

				#pragma endregion

				#pragma region 模拟EFEM给TOOL单下料
				case 4000:
				{
					const auto efemUnknownSnapshot = taskManager.getEfemUnkownStatusTasks();
					const auto efemReturnPendingSnapshot = taskManager.getEfemRuturnPendingTasks();

					//还剩下一片wafer待放回lp
					if (efemUnknownSnapshot.empty() && efemReturnPendingSnapshot.size() == 1)
					{
						if (efemReturnPendingSnapshot.at(0).source == UnifiedWaferTask::Location::LP1)
						{
							logWarn(ewtr->getName().c_str(), "cycle end efemReturnPendingTasks=%d", efemReturnPendingSnapshot.size());

							current_lp_cycle = is_lp1_cycle = true;
						}
						else if (efemReturnPendingSnapshot.at(0).source == UnifiedWaferTask::Location::LP2)
						{
							logWarn(ewtr->getName().c_str(), "cycle end efemReturnPendingTasks=%d", efemReturnPendingSnapshot.size());

							current_lp_cycle = is_lp2_cycle = true;
						}
					}
					if (efemReturnPendingSnapshot.empty())
					{
						logError("SimCycle", "EFEM单下料模拟时待下料任务为空");
						efem_auto_step = 201;
						break;
					}
					taskManager.updateTaskStatus(efemReturnPendingSnapshot.front().taskId, UnifiedWaferTask::TaskType::EFEM_RETURN, UnifiedWaferTask::Status::COMPLETED);

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
			if (efem_robot_mutex.unlock_if_owned_by_current_thread()) {
				logWarn("Cyclelog", "EFEM thread crashed, released efem_robot_mutex in owner thread.");
			}
			pauseAllThreads();
			saveCurrentStateSnapshot(e.what(), Poco::format("EFEMTransfer step:%d", efem_auto_step));
			onUpdateControlEnabled("execute_pbt", true);
			onUpdateLightButtonStatus("light_running_pbt", 3);
			logError("Cyclelog", "EFEM thread crashed:", e.what());
			qCritical() << "EFEM thread crashed:" << e.what();
		}
		catch (...) {
			if (efem_robot_mutex.unlock_if_owned_by_current_thread()) {
				logWarn("Cyclelog", "EFEM thread crashed, released efem_robot_mutex in owner thread.");
			}
			pauseAllThreads();
			saveCurrentStateSnapshot("unknown exception", Poco::format("EFEMTransfer step:%d", efem_auto_step));
			onUpdateControlEnabled("execute_pbt", true);
			onUpdateLightButtonStatus("light_running_pbt", 3);

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
					llaImmediateRepick.reset();
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

					const auto llaSnapshot = buildLoadLockTaskSnapshot("LLA");
					const auto efemUnknownLlaTasks = taskManager.getEfemUnkownStatusLLATasks();

					bool isInterlock = isLoadingInterlock("LLA");
					if (isInterlock)
					{
						//logWarn(lk1->getName().c_str(), "有片下料没完成时情景，LLA上料互锁.");
					}

					//在LLA槽中，没有传输任务,说明没有上料需求，所有wafer都完成下料到LP中
					if (llaSnapshot.pendingTasks.empty() && llaSnapshot.returnPendingTasks.empty() && llaSnapshot.returnCompletedTasks.empty() && current_lp_cycle)
					{
						logInform(lk1->getName().c_str(), "此时current_lp_cycle循环.");
						loadlock1_auto_step = 6000;
					}

                    const bool hasLlaWaferReady =
                        (!tool_allow_get_wafer_LLA) &&
                        (!llaSnapshot.pendingTasks.empty() || !llaSnapshot.returnPendingTasks.empty() || !llaSnapshot.returnCompletedTasks.empty());

					/*
					本侧LL上层已空
					&& LP还有待上料片
					&& 跨侧互锁没触发
					=> 才允许上料
					*/
                    const bool canRequestLlaLoad =
                        (!tool_allow_get_wafer_LLA) &&
                        llaSnapshot.pendingTasks.empty() &&
                        llaSnapshot.returnPendingTasks.empty() &&
                        llaSnapshot.returnCompletedTasks.empty() &&
                        (!efemUnknownLlaTasks.empty() && efemUnknownLlaTasks.size() <= originTaskSize);

                    //优先
                    if (hasLlaWaferReady)
                    {
                        //有wafer，直接抽真空，走取放晶圆流程/下料流程
                        loadlock1_auto_step = 400;
                    }

                    //需上料
                    else if ((!isInterlock) && canRequestLlaLoad)
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
					const auto llaSnapshot = buildLoadLockTaskSnapshot("LLA");
					if (!llaSnapshot.returnPendingTasks.empty() || !llaSnapshot.returnCompletedTasks.empty())
					{
						loadlock1_auto_step = 400;
						break;
					}
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
						const auto efemCompletedSnapshot = taskManager.getEfemCompletedTasks();
						auto efemToLLACompletedTasks = taskManager.getTasksByLocation(efemCompletedSnapshot, UnifiedWaferTask::Location::LLA);

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
					if(ewtr == nullptr)
					{
						ewtr = kernel->getKernelModule<EFEMWaferRobotSubsystem>("EWTR");
					}

					if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						if (ui->enableAtmosphere->checkState() == Qt::CheckState::Checked)
						{
							logInform("Cycle", "step:400, 大气cycle，跳过真空检测.");
							loadlock1_auto_step = 800;
						}
						else
						{
							if (!ewtr->isBusy())
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
							else
							{

								// 添加等待状态提示
								static int wait_ewtr_count = 0;
								if ((wait_ewtr_count++ % 20) == 0) {  // 每20次循环打印一次
									logInform(ewtr->getName().c_str(), "等待EFEM取放晶圆手臂空闲中...");
								}
								Sleep(500);
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
					const auto llaSnapshot = buildLoadLockTaskSnapshot("LLA");

					if (!llaSnapshot.pendingTasks.empty() || !llaSnapshot.returnPendingTasks.empty())
					{
						loadlock1_auto_step = 901;//取晶圆、放晶圆流程
							
					}
					else if (!llaSnapshot.returnCompletedTasks.empty()) //要兼顾到：若LLa有一片待工艺片，还有一片待efem下料的片，怎么解决？
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
					//logInform("Cycle", "llA step 950");
					const auto llaSnapshot = buildLoadLockTaskSnapshot("LLA");
					if (!llaSnapshot.returnCompletedTasks.empty())
					{
						loadlock1_auto_step = 5000;//出空Cassette流程
					}
					else if (!llaSnapshot.returnPendingTasks.empty() && !abortCycle) //2025/8/13 加!pm1_allow_get_put_wafer
					{
						//放晶圆
						loadlock1_auto_step = 2000;//允许放晶圆流程
							
					}
					else if (!llaSnapshot.pendingTasks.empty() && !abortCycle)
					{
						loadlock1_auto_step = 1000;//允许取晶圆流程
					}
					else if (!abortCycle)
					{
						loadlock1_auto_step = 10;
					}
					Sleep(500);
				}
				break;

#pragma region 允许取晶圆流程
				case 1000:
				{
					//取片原则，1.是上进下出， 只能取上层的， 2.是上下两层都可取，原则从下到上取，下层：1，上层：2 ，默认第2种
					const auto llaSnapshot = buildLoadLockTaskSnapshot("LLA");
					if (llaSnapshot.pendingTasks.empty())
					{
						logFailed(lk1->getName(), Poco::format("%s 待取片任务为空， %s：%d", lk1->getName(), loadlock1_process_name, loadlock1_auto_step));
						loadlock1_auto_step = 900;
						break;
					}
					loadlock1_move_slot_index = llaSnapshot.pendingTasks.front().targetFeedingSlot; //拿第一个task

					//当取完结束，更新状态
					logInform("Test", "loadlock1_move_slot_index %d loadLockACompletedTasks=%d", loadlock1_move_slot_index, llaSnapshot.completedTasks.size());
					loadlock1_auto_step = 1010;
				}
				break;
				case 1010:
				{
					if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto cassManager = lk1->getKernel()->getKernelModule<FortrendCassetteManager>();
						auto station_cass = cassManager->getCassette(lk1.get());
						if (isSimulationModeEnabled())
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
					lk2 = kernel->getKernelModule<FortrendLoadLockSubsystem>("LLB");
					if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL && lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						logInform("cycle", "step:1040,CheckLLAVacuum...");
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
									loadlock1_auto_step = 1999;
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
				case 1999:
				{
					if (wtr == nullptr)
					{
						wtr = kernel->getKernelModule<FortrendSunwayRobotSubsystem>("WTR");
					}

					std::string waitReason;
					const bool shouldWaitRobotWtr = shouldWaitBeforeStartWtrFingerQuery(wtr, waitReason);
					if (!shouldWaitRobotWtr)
					{
						auto cmd1 = wtr->createRQLoadCommand(0); //A手
						wtr->startCommand(cmd1);
						cmd1->wait();

						std::string waitReasonAfterArmA;
						const bool shouldWaitRobotWtrAfterArmA = shouldWaitBeforeStartWtrFingerQuery(wtr, waitReasonAfterArmA);
						if (shouldWaitRobotWtrAfterArmA)
						{
							static int wait_count_after_arm_a = 0;
							if ((wait_count_after_arm_a++ % 20) == 0)
							{
								logWarn(lk1->getName().c_str(), "wait WTR finger query after arm A. %s", waitReasonAfterArmA.c_str());
							}
							Sleep(200);
							loadlock1_auto_step = 1999;
							break;
						}


						auto cmd2 = wtr->createRQLoadCommand(1); //B手
						wtr->startCommand(cmd2);
						cmd2->wait();

						if (cmd1->hasError() || cmd2->hasError())
						{
							logFailedExcuteCommandHasError(lk1->getName(), "查询手指有无晶圆", loadlock1_process_name, loadlock1_auto_step);
						}
						else
						{
							loadlock1_auto_step = 1051;
						}
					}
					else
					{
						static int wait_count = 0;
						if ((wait_count++ % 20) == 0)
						{
							if (!waitReason.empty())
							{
								logWarn(lk1->getName().c_str(), "wait WTR is idle. %s", waitReason.c_str());
							}
							else
							{
								logWarn(lk1->getName().c_str(), "wait WTR is idle.");
							}
						}
						Sleep(200);
						loadlock1_auto_step = 1999;
						//logFailedNotNormal(lk1->getName(), loadlock1_process_name, loadlock1_auto_step);
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
							logInform(lk1->getName().c_str(), "step:1051,loadLockA PendingTasks 有片,触发Robot取片流程.");


							if (!wtr->isBusy())
							{
								logInform(lk1->getName().c_str(), "step 1051 WTR空闲，准备取片.");
							}
							else
							{
								static int wait_count = 0;
								if ((wait_count++ % 20) == 0)
								{
									logWarn(lk1->getName().c_str(), "WTR忙碌中,等待空闲. busy=%s", wtr->isBusy()?"true":"false");
								}
								Sleep(200);
								break;
							}

							std::shared_ptr<FortrendPMCavitySubsystem> pm2Subsystem;
							if (!ensurePm2SubsystemReady(pm2Subsystem))
							{
								Sleep(200);
								break;
							}
							bool pm2HasWafer = false;
							if (pm2_exchange_in_flight.load())
							{
								static int exchange_wait_count = 0;
								if ((exchange_wait_count++ % 20) == 0)
								{
									logInform(lk1->getName().c_str(), "PM2交换进行中, LLA暂不发起新的取片请求.");
								}
								Sleep(200);
								break;
							}

							if (!tryGetPmHasWafer(pm2Subsystem, "PM2", pm2HasWafer))
							{
								Sleep(200);
								break;
							}
							const auto llaSnapshot = buildLoadLockTaskSnapshot("LLA");
							if (llaSnapshot.pendingTasks.empty())
							{
								logWarn(lk1->getName().c_str(), "step 1051读取到LLA待取任务为空，回退等待后重试.");
								loadlock1_auto_step = 950;
								Sleep(200);
								break;
							}
							const int desiredArm = llaSnapshot.pendingTasks.front().arm;

							const bool armAHasWafer = wtr->hasObject(0);
							const bool armBHasWafer = wtr->hasObject(1);

							logWarn(lk1->getName().c_str(),
								"检测状态:pm2Has=%d, armA_has=%d, armB_has=%d",
								(int)pm2HasWafer, (int)armAHasWafer, (int)armBHasWafer);

							if (pm2HasWafer && (armAHasWafer || armBHasWafer))
							{
								llaImmediateRepick.reset();

								const bool hasSingleArmBusy = armAHasWafer ^ armBHasWafer;
								const int occupiedArm = armAHasWafer ? 0 : (armBHasWafer ? 1 : -1);

								auto returnIt = llaSnapshot.returnPendingTasks.end();
								auto pendingIt = llaSnapshot.pendingTasks.end();

								if (hasSingleArmBusy && occupiedArm >= 0)
								{
									returnIt = std::find_if(llaSnapshot.returnPendingTasks.begin(), llaSnapshot.returnPendingTasks.end(),
										[occupiedArm](const UnifiedWaferTask& task) { return task.arm == occupiedArm; });
									if (!llaSnapshot.pendingTasks.empty())
									{
										pendingIt = std::find_if(llaSnapshot.pendingTasks.begin(), llaSnapshot.pendingTasks.end(),
											[occupiedArm](const UnifiedWaferTask& task) { return task.arm == occupiedArm; });
										if (pendingIt == llaSnapshot.pendingTasks.end())
										{
											logInform(lk1->getName().c_str(),
												"LLA回片后立即补取保持固定手臂: pending队列中未找到 returnArm=%d 对应任务, 本轮跳过立即补取",
												occupiedArm);
										}
									}
								}

								if (returnIt != llaSnapshot.returnPendingTasks.end() && pendingIt != llaSnapshot.pendingTasks.end())
								{
									llaImmediateRepick.enabled = true;
									llaImmediateRepick.taskId = pendingIt->taskId;
									llaImmediateRepick.arm = pendingIt->arm;
									llaImmediateRepick.slot = pendingIt->targetFeedingSlot;

									logInform(lk1->getName().c_str(),
										"step:1051,识别到LLA回片后立即补取场景, occupiedArm=%d, repickArm=%d, repickSlot=%d, taskId=%d",
										occupiedArm, llaImmediateRepick.arm, llaImmediateRepick.slot, llaImmediateRepick.taskId);

									loadlock1_auto_step = 2000;
									break;
								}
								else
								{
									static int wait_count = 0;
									if ((wait_count++ % 20) == 0)
									{
										logWarn(lk1->getName().c_str(),
											"PM2有片且WTR手臂已有晶圆,回退到950优先处理回片/下料. pm2Has=%d, armA_has=%d, armB_has=%d",
											(int)pm2HasWafer, (int)armAHasWafer, (int)armBHasWafer);
									}
									loadlock1_auto_step = 950;
									Sleep(200);
									break;
								}
							}
							else
							{
								PM2ScheduleSnapshot pm2Snapshot;
								std::string pm2WaitReason;
								if (!tryBuildPm2ScheduleSnapshot(pm2Snapshot))
								{
									static int wait_count = 0;
									if ((wait_count++ % 20) == 0)
									{
										logWarn(lk1->getName().c_str(), "step 1051读取PM2调度快照失败，回退等待后重试.");
									}
									loadlock1_auto_step = 950;
									Sleep(200);
									break;
								}
								const bool allowPreloadWaitForCraft = canLlImmediateRepickWaitForPm2Craft(pm2Snapshot, desiredArm);
								if (!allowPreloadWaitForCraft && shouldLlWaitForPm2Priority(pm2Snapshot, desiredArm, pm2WaitReason))
								{
									static int wait_count = 0;
									if ((wait_count++ % 20) == 0)
									{
										logWarn(lk1->getName().c_str(),
											"step 1051禁止从LLA取片，当前应等待PM2优先. %s",
											pm2WaitReason.c_str());
									}
									loadlock1_auto_step = 950;
									Sleep(200);
									break;
								}
								if (allowPreloadWaitForCraft)
								{
									logInform(lk1->getName().c_str(),
										"step 1051命中预装窗口: PM2加工中,允许LLA按固定arm=%d预装下一片等待交换.",
										desiredArm);
								}

								int targetArm = desiredArm;
								std::string armBusyReason;
								if ((targetArm >= 0 && targetArm <= 1) && isRobotArmOccupiedForLlRequest(targetArm, armBusyReason))
								{
									static int wait_count = 0;
									if ((wait_count++ % 20) == 0)
									{
										logWarn(lk1->getName().c_str(),
											"WTR手臂%d发送前不可用(%s), 保持任务固定手臂并等待后重试",
											desiredArm, armBusyReason.c_str());
									}
									Sleep(200);
									break;
								}

								logInform(lk1->getName().c_str(), "step 1051 send robot_get_from_lla");
								// 设置请求标志，委托Robot线程执行
								robot_get_from_lla.arm.store(targetArm);
								robot_get_from_lla.slot.store(loadlock1_move_slot_index);
								robot_get_from_lla.done.store(false);
								robot_get_from_lla.success.store(false);
								robot_get_from_lla.expedited.store(false);
								robot_get_from_lla.requested.store(true);
								loadlock1_auto_step = 1055; // 等待Robot线程完成
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
						logFailedNotNormal(lk1->getName(), loadlock1_process_name, loadlock1_auto_step);
					}
				}
				break;
				case 1055: // 等待Robot线程从LLA取片完成
				{
					if (robot_get_from_lla.done.load())//指令去执行了，等待结果
					{
						if (robot_get_from_lla.success.load())//结果成功
						{
							loadlock1_auto_step = 1052;
						} 
						else
						{
							logFailedExcuteCommandHasError(lk1->getName(), "取晶圆", loadlock1_process_name, loadlock1_auto_step);
							loadlock1_auto_step = 950;
							Sleep(2000);
						}
					} 
					else
					{
						// 添加等待状态提示
						static int wait_count = 0;
						if ((wait_count++ % 20) == 0) {  // 每20次循环打印一次
							logInform(lk1->getName().c_str(), "等待从LLA取晶圆完成...");
						}
						Sleep(500);
					}
				}
				break;
				case 1052:
				{
					const auto llaSnapshot = buildLoadLockTaskSnapshot("LLA");
					if (!llaSnapshot.pendingTasks.empty())
					{
						const auto currentTask = llaSnapshot.pendingTasks.front();
						LLAPmName = getSelectPmProcessName(currentTask);
						taskManager.updateTaskStatus(currentTask.taskId, UnifiedWaferTask::TaskType::LOADLOCK_TRANSFER, UnifiedWaferTask::Status::COMPLETED);
						taskManager.updateTaskStatus(currentTask.taskId, UnifiedWaferTask::TaskType::PM_PROCESS, UnifiedWaferTask::Status::QUEUED);
					}
					else
					{
						logFailedExcuteCommandHasError(lk1->getName(), "数据更新错误", loadlock1_process_name, loadlock1_auto_step);
						loadlock1_auto_step = 950;
					}

					loadlock1_auto_step = 1053;
				}
				break;

				case 1053:
				{
					if (wtr == nullptr)
					{
						wtr = kernel->getKernelModule<FortrendSunwayRobotSubsystem>("WTR");
					}
					
					if (ui->enableAtmosphere->checkState() == Qt::CheckState::Checked)
					{
						//std::string pmName = getSelectPmProcessName(loadLockAPendingTasks.at(0));
						logInform("cycle", "step:1053,大气模式，不关闭TM腔门!");

						tool_allow_lla = true;
						if (!LLAPmName.empty())
						{
							if (LLAPmName == "PM1")
							{
								pm1_allow_get_put_wafer = true;
							}
							else if (LLAPmName == "PM2")
							{
								//pm2_allow_get_put_wafer = true;

								pm2_allow_loading_wafer = true;
							}
							else if (LLAPmName == "PM3")
							{
								pm3_allow_get_put_wafer = true;
							}
							else if (LLAPmName == "PM4")
							{
								pm4_allow_get_put_wafer = true;
							}
							else
							{
								logFailedExcuteCommandHasError(lk1->getName(), "配方解析错误", loadlock1_process_name, loadlock1_auto_step);
							}
							logInform(lk1->getName().c_str(), "呼叫:%s 取放片.", LLAPmName.c_str());
						}
						else
						{
							logFailedExcuteCommandHasError(lk1->getName(), "配方解析错误", loadlock1_process_name, loadlock1_auto_step);
						}
						loadlock1_auto_step = 950;
					}
					else
					{
						//2026-3-29 避免 Resources : WTR   has be lock by WTR.
						if (lk1->getWtrOriginSafeSignal() && !wtr->isBusy())
						{
							auto cmd = lk1->createCloseTMCavityDoorCommand();
							lk1->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError())
							{
								logFailedExcuteCommandHasError(lk1->getName(), "关闭传输腔门阀", loadlock1_process_name, loadlock1_auto_step);
							}
							else {
								
								if (!LLAPmName.empty())
								{
									tool_allow_lla = true;

									if (LLAPmName == "PM1")
									{
										pm1_allow_get_put_wafer = true;
									}
									else if (LLAPmName == "PM2")
									{
										//pm2_allow_get_put_wafer = true;
										pm2_allow_loading_wafer = true;
									}
									else if (LLAPmName == "PM3")
									{
										pm3_allow_get_put_wafer = true;
									}
									else if (LLAPmName == "PM4")
									{
										pm4_allow_get_put_wafer = true;
									}
									else
									{
										logFailedExcuteCommandHasError(lk1->getName(), "配方解析错误", loadlock1_process_name, loadlock1_auto_step);
									}
									logInform(lk1->getName().c_str(), "呼叫:%s 取放片.", LLAPmName.c_str());
								}
								else
								{
									logFailedExcuteCommandHasError(lk1->getName(), "配方解析错误", loadlock1_process_name, loadlock1_auto_step);
								}
								loadlock1_auto_step = 950;
							}
						}
						else
						{
							static int wait_log_count1 = 0;
							if (wait_log_count1++ % 50 == 0) {
								logInform(lk1->getName().c_str(), "等待 WTR 安全信号或 WTR空闲...");
							}
							Sleep(200);
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
						const auto llaSnapshot = buildLoadLockTaskSnapshot("LLA");
						if (!llaSnapshot.returnPendingTasks.empty())
						{
							const auto currentTask = llaSnapshot.returnPendingTasks.front();
							loadlock1_move_slot_index = currentTask.targetBlankingSlot;
							int put_arm = currentTask.arm;

							bool haswaferarm1 = wtr->hasObject(put_arm);

							if(haswaferarm1)
							{
								loadlock1_auto_step = 2010;
							}
							else
							{
								static int wait_count = 0;
								if ((wait_count++ % 20) == 0) {
									logInform(lk1->getName().c_str(), "WTR手臂没有晶圆，无法放片，等待下一次循环.");
								}
								Sleep(500);
								loadlock1_auto_step = 2000;
							}
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
							if (isSimulationModeEnabled())
							{
								logInform(elp->getName().c_str(), "step:2010,模拟放晶圆流程程序....");
								loadlock1_auto_step = 2070;
							}
							else
							{
								loadlock1_auto_step = 2030;
							}
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
					lk2 = kernel->getKernelModule<FortrendLoadLockSubsystem>("LLB");
					if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL && lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						logInform("cycle", "step:2030,CheckLLAVacuum...");

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
							loadlock1_auto_step = 2060; 
						}
						else 
						{
							auto cmd = tm->createCloseDiaphragmValveCommand(TMCavityValveOpening::TMCavity_Both);
							tm->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError())
							{
								logFailedExcuteCommandHasError(lk1->getName(), "关闭隔膜阀失败.", loadlock1_process_name, loadlock1_auto_step);
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
								loadlock1_auto_step = 2055;
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
				case 2055:
				{
					if (wtr == nullptr)
					{
						wtr = kernel->getKernelModule<FortrendSunwayRobotSubsystem>("WTR");
					}

					std::string waitReason;
					const bool shouldWaitRobotWtr = shouldWaitBeforeStartWtrFingerQuery(wtr, waitReason);
					if (!shouldWaitRobotWtr)
					{
						auto cmd1 = wtr->createRQLoadCommand(0); //A手
						wtr->startCommand(cmd1);
						cmd1->wait();

						std::string waitReasonAfterArmA;
						const bool shouldWaitRobotWtrAfterArmA = shouldWaitBeforeStartWtrFingerQuery(wtr, waitReasonAfterArmA);
						if (shouldWaitRobotWtrAfterArmA)
						{
							static int wait_count_after_arm_a = 0;
							if ((wait_count_after_arm_a++ % 20) == 0)
							{
								logWarn(lk1->getName().c_str(), "step 2055 wait WTR finger query after arm A. %s", waitReasonAfterArmA.c_str());
							}
							Sleep(200);
							break;
						}

						auto cmd2 = wtr->createRQLoadCommand(1); //B手
						wtr->startCommand(cmd2);
						cmd2->wait();

						if (cmd1->hasError() || cmd2->hasError())
						{
							logFailedExcuteCommandHasError(lk1->getName(), "放片前查询手指有无晶圆", loadlock1_process_name, loadlock1_auto_step);
						}
						else
						{
							loadlock1_auto_step = 2060;
						}
					}
					else
					{
						static int wait_count = 0;
						if ((wait_count++ % 20) == 0)
						{
							if (!waitReason.empty())
							{
								logWarn(lk1->getName().c_str(), "step 2055 wait WTR is idle. %s", waitReason.c_str());
							}
							else
							{
								logWarn(lk1->getName().c_str(), "step 2055 wait WTR is idle.");
							}
						}
						Sleep(200);
					}
				}
				break;
				case 2060:
				{
					const auto llaSnapshot = buildLoadLockTaskSnapshot("LLA");
					if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL && lk1->hasDoorOpend())
					{
						//// 新增：再次检查 returnPendingTasks 是否为空
						if (llaSnapshot.returnPendingTasks.empty())
						{
							logFailed(lk1->getName(), Poco::format("%s 放片任务队列为空 %s：%d", lk1->getName(), loadlock1_process_name, loadlock1_auto_step));
							loadlock1_auto_step = 900;
							break;
						}
						
						if (wtr->isBusy())
						{
							static int wait_count = 0;
							if ((wait_count++ % 20) == 0)
							{
								logWarn(lk1->getName().c_str(), "step 2060 WTR忙碌中,等待空闲. busy=true");
							}
							Sleep(200);
							break;
						}

						//2026-5-16 ：手上有片 且 做完工艺的片子

						const auto llaSnapshot = buildLoadLockTaskSnapshot("LLA");
						if (llaSnapshot.returnPendingTasks.empty())
						{
							logFailed(lk1->getName(), Poco::format("%s 待回片任务为空， %s：%d", lk1->getName(), loadlock1_process_name, loadlock1_auto_step));
							loadlock1_auto_step = 900;
							break;
						}
						const auto currentReturnTask = llaSnapshot.returnPendingTasks.front();
						loadlock1_move_slot_index = currentReturnTask.targetBlankingSlot;
						const auto put_arm = currentReturnTask.arm;

						bool haswaferarm1 = wtr->hasObject(put_arm);
						
						if (!haswaferarm1)
						{
							static int wait_count = 0;
							if ((wait_count++ % 20) == 0)
							{
								logWarn(lk1->getName().c_str(), "准备放片到LLA，但手指 %d 还没有片子，等待WTR取片完成...", put_arm);
							}
							Sleep(200);
							loadlock1_auto_step = 2055; // 回退到2055重新查询
							break;
						}

						logInform(lk1->getName().c_str(), "准备放片到LLA，目标槽位%d，使用手臂%d", loadlock1_move_slot_index, put_arm);

						if(lk1->getTMCavityDoorOpend() && haswaferarm1)
						{
							// 设置请求标志，委托Robot线程执行放片到LLA
							lla_return_task_id.store(currentReturnTask.taskId);
							lla_return_slot.store(currentReturnTask.targetBlankingSlot);
							robot_put_to_lla.arm.store(put_arm);
							robot_put_to_lla.slot.store(loadlock1_move_slot_index);
							robot_put_to_lla.done.store(false);
							robot_put_to_lla.success.store(false);
							robot_put_to_lla.requested.store(true);
							loadlock1_auto_step = 2065; // 等待Robot线程完成
						}
						else
						{
							lla_return_task_id.store(-1);
							lla_return_slot.store(-1);
							loadlock1_auto_step = 2000;
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
				case 2065: // 等待Robot线程放片到LLA完成
				{
					if (robot_put_to_lla.done.load()) 
					{
						if (robot_put_to_lla.success.load())
						{
							loadlock1_auto_step = llaImmediateRepick.enabled ? 2066 : 2070;
						}
						else
						{
								lla_return_task_id.store(-1);
								lla_return_slot.store(-1);
							llaImmediateRepick.reset();
							logFailed(wtr->getName(), "LLA放片失败.STEP=2065");
							logFailedNotNormal(wtr->getName(), loadlock1_process_name, loadlock1_auto_step);
							Sleep(2000);
							loadlock1_auto_step = 950; 
						}
					} 
					else 
					{
						// 添加等待状态提示
						static int wait_count = 0;
						if ((wait_count++ % 20) == 0) {
							logInform(wtr->getName().c_str(), "step 2065,等待放LLA晶圆完成...");
						}
						Sleep(500);
					}
				}
				break;
				case 2066:
				{
					auto llaSnapshot = buildLoadLockTaskSnapshot("LLA");
					if (llaSnapshot.returnPendingTasks.empty())
					{
						logFailed(lk1->getName(), Poco::format("%s 回片成功后找不到待回片任务， %s：%d", lk1->getName(), loadlock1_process_name, loadlock1_auto_step));
						llaImmediateRepick.reset();
						loadlock1_auto_step = 900;
						break;
					}

					const int returnTaskId = lla_return_task_id.load();
					const auto returnTaskIt = std::find_if(llaSnapshot.returnPendingTasks.begin(), llaSnapshot.returnPendingTasks.end(),
						[returnTaskId](const UnifiedWaferTask& task) { return task.taskId == returnTaskId; });
					if (returnTaskIt == llaSnapshot.returnPendingTasks.end())
					{
						logError(lk1->getName().c_str(), "LLA return completion lost locked return task: taskId=%d, refuse incorrect status writeback.", returnTaskId);
						lla_return_task_id.store(-1);
						lla_return_slot.store(-1);
						llaImmediateRepick.reset();
						loadlock1_auto_step = 900;
						break;
					}
					taskManager.updateTaskStatus(returnTaskIt->taskId, UnifiedWaferTask::LOADLOCK_RETURN, UnifiedWaferTask::COMPLETED);
					lla_return_task_id.store(-1);
					llaSnapshot = buildLoadLockTaskSnapshot("LLA");

					if (!llaImmediateRepick.enabled)
					{
						loadlock1_auto_step = 2071;
						break;
					}

					auto repickTaskIt = std::find_if(llaSnapshot.pendingTasks.begin(), llaSnapshot.pendingTasks.end(),
						[this](const UnifiedWaferTask& task) { return task.taskId == llaImmediateRepick.taskId; });
					if (repickTaskIt == llaSnapshot.pendingTasks.end())
					{
						logWarn(lk1->getName().c_str(), "LLA立即补取目标task已不存在，跳过补取并继续收尾.");
						llaImmediateRepick.reset();
						loadlock1_auto_step = 2071;
						break;
					}
					if (repickTaskIt->arm != llaImmediateRepick.arm)
					{
						logWarn(lk1->getName().c_str(),
							"LLA立即补取目标task固定手臂不匹配: taskId=%d, configuredArm=%d, plannedArm=%d，跳过补取.",
							repickTaskIt->taskId, repickTaskIt->arm, llaImmediateRepick.arm);
						llaImmediateRepick.reset();
						loadlock1_auto_step = 2071;
						break;
					}

					if (!lk1->getTMCavityDoorOpend())
					{
						logWarn(lk1->getName().c_str(),
							"LLA立即补取前状态不满足: tmDoor=%d, arm=%d，跳过补取.",
							(int)lk1->getTMCavityDoorOpend(),
							llaImmediateRepick.arm);
						llaImmediateRepick.reset();
						loadlock1_auto_step = 2071;
						break;
					}

					PM2ScheduleSnapshot pm2Snapshot;
					std::string pm2WaitReason;
					if (!tryBuildPm2ScheduleSnapshot(pm2Snapshot))
					{
						logWarn(lk1->getName().c_str(), "LLA立即补取前读取PM2调度快照失败，跳过补取.");
						llaImmediateRepick.reset();
						loadlock1_auto_step = 2071;
						break;
					}
					const bool allowRepickWaitForCraft = canLlImmediateRepickWaitForPm2Craft(pm2Snapshot, llaImmediateRepick.arm);
					if (!allowRepickWaitForCraft && shouldLlWaitForPm2Priority(pm2Snapshot, llaImmediateRepick.arm, pm2WaitReason))
					{
						logWarn(lk1->getName().c_str(),
							"LLA立即补取前检测到当前应等待PM2优先，跳过补取. arm=%d, %s",
							llaImmediateRepick.arm, pm2WaitReason.c_str());
						llaImmediateRepick.reset();
						loadlock1_auto_step = 2071;
						break;
					}
					if (allowRepickWaitForCraft)
					{
						logInform(lk1->getName().c_str(),
							"LLA立即补取命中预装窗口: PM2仍在加工, 允许固定arm=%d预装下一片在手等待交换.",
							llaImmediateRepick.arm);
					}

					std::string armBlockedReason;
					if (isRobotArmOccupiedForLlRequest(llaImmediateRepick.arm, armBlockedReason))
					{
						logWarn(lk1->getName().c_str(),
							"LLA立即补取前状态不满足: arm=%d, blockReason=%s，跳过补取.",
							llaImmediateRepick.arm, armBlockedReason.c_str());
						llaImmediateRepick.reset();
						loadlock1_auto_step = 2071;
						break;
					}

					loadlock1_move_slot_index = repickTaskIt->targetFeedingSlot;
					robot_get_from_lla.arm.store(llaImmediateRepick.arm);
					robot_get_from_lla.slot.store(loadlock1_move_slot_index);
					robot_get_from_lla.done.store(false);
					robot_get_from_lla.success.store(false);
					robot_get_from_lla.expedited.store(true);
					robot_get_from_lla.requested.store(true);
					logInform(lk1->getName().c_str(),
						"LLA回片后立即补取一片, arm=%d, slot=%d, taskId=%d",
						llaImmediateRepick.arm, loadlock1_move_slot_index, llaImmediateRepick.taskId);
					loadlock1_auto_step = 2067;
				}
				break;
				case 2067:
				{
					if (robot_get_from_lla.done.load())
					{
						if (robot_get_from_lla.success.load())
						{
							taskManager.updateTaskStatus(llaImmediateRepick.taskId, UnifiedWaferTask::LOADLOCK_TRANSFER, UnifiedWaferTask::COMPLETED);
							taskManager.updateTaskStatus(llaImmediateRepick.taskId, UnifiedWaferTask::PM_PROCESS, UnifiedWaferTask::QUEUED);
							logInform(lk1->getName().c_str(),
								"LLA回片后立即补取完成, arm=%d, slot=%d, taskId=%d",
								llaImmediateRepick.arm, llaImmediateRepick.slot, llaImmediateRepick.taskId);
						}
						else
						{
							logWarn(lk1->getName().c_str(), "LLA回片后立即补取失败，继续后续关门收尾.");
						}

						llaImmediateRepick.reset();
						loadlock1_auto_step = 2071;
					}
					else
					{
						static int wait_count = 0;
						if ((wait_count++ % 20) == 0) {
							logInform(lk1->getName().c_str(), "step 2067,等待LLA回片后的立即补取完成...");
						}
						Sleep(200);
					}
				}
				break;

				case 2070:
				{
					const auto llaSnapshot = buildLoadLockTaskSnapshot("LLA");
					if (llaSnapshot.returnPendingTasks.empty())
					{
						logFailed(lk1->getName(), Poco::format("%s 放片完成但任务队列为空， %s：%d", lk1->getName(), loadlock1_process_name, loadlock1_auto_step));
						loadlock1_auto_step = 900;
						break;
					}					
					const int returnTaskId = lla_return_task_id.load();
					const auto returnTaskIt = std::find_if(llaSnapshot.returnPendingTasks.begin(), llaSnapshot.returnPendingTasks.end(),
						[returnTaskId](const UnifiedWaferTask& task) { return task.taskId == returnTaskId; });
					if (returnTaskIt == llaSnapshot.returnPendingTasks.end())
					{
						logError(lk1->getName().c_str(), "LLA put completion lost locked return task: taskId=%d, refuse incorrect status writeback.", returnTaskId);
						lla_return_task_id.store(-1);
						lla_return_slot.store(-1);
						loadlock1_auto_step = 900;
						break;
					}
					taskManager.updateTaskStatus(returnTaskIt->taskId, UnifiedWaferTask::LOADLOCK_RETURN, UnifiedWaferTask::Status::COMPLETED);
					lla_return_task_id.store(-1);
					loadlock1_auto_step = 2071;
				}
				break;
				case 2071:
				{
					if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						if (ui->enableAtmosphere->checkState() == Qt::CheckState::Checked)
						{
							logInform("cycle", "大气模式，不关闭传输腔门阀!");
							loadlock1_auto_step = 2080;
						}
						else
						{
							const auto llaSnapshot = buildLoadLockTaskSnapshot("LLA");
							if (!llaSnapshot.pendingTasks.empty())
							{
								const int desiredArm = llaSnapshot.pendingTasks.front().arm;
								PM2ScheduleSnapshot pm2Snapshot;
								if (tryBuildPm2ScheduleSnapshot(pm2Snapshot))
								{
									const bool allowPreloadBeforeUnload = canLlImmediateRepickWaitForPm2Craft(pm2Snapshot, desiredArm);
									if (allowPreloadBeforeUnload)
									{
										std::string armBusyReason;
										if (!isRobotArmOccupiedForLlRequest(desiredArm, armBusyReason))
										{
											loadlock1_move_slot_index = llaSnapshot.pendingTasks.front().targetFeedingSlot;
											logInform(lk1->getName().c_str(),
												"LLA关门破真空前命中预装窗口: 先按固定arm=%d预装下一片, 再执行下料收尾.",
												desiredArm);
											lla_return_slot.store(-1);
											robot_get_from_lla.arm.store(desiredArm);
											robot_get_from_lla.slot.store(loadlock1_move_slot_index);
											robot_get_from_lla.done.store(false);
											robot_get_from_lla.success.store(false);
											robot_get_from_lla.expedited.store(false);
											robot_get_from_lla.requested.store(true);
											loadlock1_auto_step = 1055;
											break;
										}
									}
								}
							}

							//2026-3-27 检测mapping，再去关门
							auto cassManager = lk1->getKernel()->getKernelModule<FortrendCassetteManager>();
							auto station_cass = cassManager->getCassette(lk1.get());
							const int returnSlot = lla_return_slot.load();
							if (returnSlot > 0 && station_cass->getMapping(returnSlot) == Cassette::Mapping::Present)
							{
								if (lk1->getWtrOriginSafeSignal() && !wtr->isBusy())
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
										lla_return_slot.store(-1);
										loadlock1_auto_step = 2080;
									}
								}
								else
								{
									static int wait_log_count2 = 0;
									if (wait_log_count2++ % 50 == 0) {
										logInform(lk1->getName().c_str(), "等待 WTR 安全信号或 WTR空闲...");
									}
									Sleep(200);
								}
							}
							else
							{
								lla_return_slot.store(-1);
								logFailedExcuteCommandHasError(lk1->getName(), "检测mapping无片，不执行关闭传输腔门阀", loadlock1_process_name, loadlock1_auto_step);

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
					if (ewtr == nullptr)
					{
						ewtr = kernel->getKernelModule<EFEMWaferRobotSubsystem>("EWTR");
					}

					if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						if (ui->enableAtmosphere->checkState() == Qt::CheckState::Checked)
						{
							loadlock1_auto_step = 6000;
						}
						else 
						{
							if (!ewtr->isBusy())
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
								static int wait_log_count3 = 0;
								if (wait_log_count3++ % 50 == 0) {
									logInform(lk1->getName().c_str(), "等待 EFEM 下料完成...");
								}
								Sleep(200);
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
					const auto llaPendingSnapshot = taskManager.getLoadLockPendingTasks("LLA");

					if (llaPendingSnapshot.empty() && current_lp_cycle)
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
			pauseAllThreads();
			saveCurrentStateSnapshot(e.what(), Poco::format("LLATransfer step:%d", loadlock1_auto_step));
			onUpdateControlEnabled("execute_pbt", true);
			onUpdateLightButtonStatus("light_running_pbt", 3);
			logError("Cyclelog", "LLATransfer thread crashed:", e.what());
			qCritical() << "LLATransfer thread crashed:" << e.what();
		}
		catch (...) {
			pauseAllThreads();
			saveCurrentStateSnapshot("unknown exception", Poco::format("LLATransfer step:%d", loadlock1_auto_step));
			onUpdateControlEnabled("execute_pbt", true);
			onUpdateLightButtonStatus("light_running_pbt", 3);
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
					llbImmediateRepick.reset();
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
					const auto llbSnapshot = buildLoadLockTaskSnapshot("LLB");
					const auto efemUnknownLlbTasks = taskManager.getEfemUnkownStatusLLBTasks();
					
					bool isInterlock = isLoadingInterlock("LLB");
					if (isInterlock)
					{
						//logWarn(lk1->getName().c_str(), "有片下料没完成时情景，LLB上料互锁.");
					}

                    //判断是否lp2循环
					if (llbSnapshot.pendingTasks.empty() && llbSnapshot.returnPendingTasks.empty() && llbSnapshot.returnCompletedTasks.empty() && current_lp_cycle)
                    {
                        logInform(lk2->getName().c_str(), "此时current_lp_cycle循环.");
                        loadlock2_auto_step = 6000;
                    }

                    const bool hasLlbWaferReady =
                        (!tool_allow_get_wafer_LLB) &&
                        (!llbSnapshot.pendingTasks.empty() || !llbSnapshot.returnPendingTasks.empty() || !llbSnapshot.returnCompletedTasks.empty());
                   
					const bool canRequestLlbLoad =
                        (!tool_allow_get_wafer_LLB) &&
                        llbSnapshot.pendingTasks.empty() &&
                        llbSnapshot.returnPendingTasks.empty() &&
                        llbSnapshot.returnCompletedTasks.empty() &&
                        (!efemUnknownLlbTasks.empty() && efemUnknownLlbTasks.size() <= originTaskSize);

                    //走Loadlock流程：自己的上料完成
                    if (hasLlbWaferReady)  
                    {
                        //有wafer，直接抽真空，走取放晶圆流程，下料流程，
                        loadlock2_auto_step = 400;
                    }

                    //上料条件：仅根据当前LL自身状态判断是否需要上料
                    else if ((!isInterlock) && canRequestLlbLoad)
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
					const auto llbSnapshot = buildLoadLockTaskSnapshot("LLB");
					if (!llbSnapshot.returnPendingTasks.empty() || !llbSnapshot.returnCompletedTasks.empty())
					{
						loadlock2_auto_step = 400;
						break;
					}
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
						const auto efemCompletedSnapshot = taskManager.getEfemCompletedTasks();
						
						auto efemToLLBCompletedTasks = taskManager.getTasksByLocation(efemCompletedSnapshot, UnifiedWaferTask::Location::LLB);

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
					if (ewtr == nullptr)
					{
						ewtr = kernel->getKernelModule<EFEMWaferRobotSubsystem>("EWTR");
					}

					if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{

						if (ui->enableAtmosphere->checkState() == Qt::CheckState::Checked)
						{
							logInform("Cycle", "step:400, 大气cycle，跳过真空检测.");
							loadlock2_auto_step = 800;
						}
						else
						{
							if(!ewtr->isBusy())
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
							else
							{
								static int wait_log_count4 = 0;
								if (wait_log_count4++ % 50 == 0) {
									logInform(lk2->getName().c_str(), "等待 EFEM 下料完成...");
								}
								Sleep(200);
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
					const auto llbSnapshot = buildLoadLockTaskSnapshot("LLB");

					if (!llbSnapshot.pendingTasks.empty() || !llbSnapshot.returnPendingTasks.empty())
					{
						loadlock2_auto_step = 901;//取晶圆、放晶圆流程
					}
					else if (!llbSnapshot.returnCompletedTasks.empty()) //要兼顾到：若LLa有一片待工艺片，还有一片待efem下料的片，怎么解决？
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
					//logInform("Cycle", "LLB step 950");
					const auto llbSnapshot = buildLoadLockTaskSnapshot("LLB");

					if (!llbSnapshot.returnCompletedTasks.empty())
					{
						loadlock2_auto_step = 5000;//出空Cassette流程
					}
					else if (!llbSnapshot.returnPendingTasks.empty() && !abortCycle)
					{
						//放晶圆
						loadlock2_auto_step = 2000;//允许放晶圆流程
					}
					else if (!llbSnapshot.pendingTasks.empty() && !abortCycle)
					{
						//取晶圆
						loadlock2_auto_step = 1000;//允许取晶圆流程
					}
					else if (!abortCycle)
					{
						loadlock2_auto_step = 10;
					}
					Sleep(500);
				}
				break;
#pragma region 允许取晶圆流程
				case 1000:
				{
					//取片原则，1.是上进下出， 只能取上层的， 2.是上下两层都可取，原则从下到上取，下层：1，上层：2 ，默认第2种
					const auto llbSnapshot = buildLoadLockTaskSnapshot("LLB");
					if (llbSnapshot.pendingTasks.empty())
					{
						logFailed(lk2->getName(), Poco::format("%s 待取片任务为空， %s：%d", lk2->getName(), loadlock2_process_name, loadlock2_auto_step));
						loadlock2_auto_step = 900;
						break;
					}
					loadlock2_move_slot_index = llbSnapshot.pendingTasks.front().targetFeedingSlot; //拿第一个task

					//当取完结束，更新状态
					logInform("Test", "loadlock2_move_slot_index %d loadLockBCompletedTasks=%d", loadlock2_move_slot_index, llbSnapshot.completedTasks.size());
					loadlock2_auto_step = 1010;
				}
				break;
				case 1010:
				{
					if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto cassManager = lk2->getKernel()->getKernelModule<FortrendCassetteManager>();
						auto station_cass = cassManager->getCassette(lk2.get());
						if (isSimulationModeEnabled())
						{
							logInform(elp->getName().c_str(), "step:1010,LLB模拟取晶圆流程程序....");
							loadlock2_auto_step = 1052;
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
					lk1 = kernel->getKernelModule<FortrendLoadLockSubsystem>("LLA");
					if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL && lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						logInform("cycle", "step:1040,CheckLLBVacuum...");
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
									loadlock2_auto_step = 1999;
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
				case 1999:
				{
					if (wtr == nullptr)
					{
						wtr = kernel->getKernelModule<FortrendSunwayRobotSubsystem>("WTR");
					}

					std::string waitReason;
					const bool shouldWaitRobotWtr = shouldWaitBeforeStartWtrFingerQuery(wtr, waitReason);
					if (!shouldWaitRobotWtr)
					{
						auto cmd1 = wtr->createRQLoadCommand(0); //A手
						wtr->startCommand(cmd1);
						cmd1->wait();

						std::string waitReasonAfterArmA;
						const bool shouldWaitRobotWtrAfterArmA = shouldWaitBeforeStartWtrFingerQuery(wtr, waitReasonAfterArmA);
						if (shouldWaitRobotWtrAfterArmA)
						{
							static int wait_count_after_arm_a = 0;
							if ((wait_count_after_arm_a++ % 20) == 0)
							{
								logWarn(lk2->getName().c_str(), "wait WTR finger query after arm A. %s", waitReasonAfterArmA.c_str());
							}
							Sleep(200);
							loadlock2_auto_step = 1999;
							break;
						}


						auto cmd2 = wtr->createRQLoadCommand(1); //B手
						wtr->startCommand(cmd2);
						cmd2->wait();

						if (cmd1->hasError() || cmd2->hasError())
						{
							logFailedExcuteCommandHasError(lk2->getName(), "查询手指有无晶圆", loadlock2_process_name, loadlock2_auto_step);
						}
						else
						{
							loadlock2_auto_step = 1051;
						}
					}
					else
					{
						//logFailedNotNormal(lk2->getName(), loadlock2_process_name, loadlock2_auto_step);
						static int wait_count = 0;
						if ((wait_count++ % 20) == 0)
						{
							if (!waitReason.empty())
							{
								logWarn(lk2->getName().c_str(), "wait WTR is idle. %s", waitReason.c_str());
							}
							else
							{
								logWarn(lk2->getName().c_str(),"wait WTR is idle.");
							}
						}
						Sleep(200);
						loadlock2_auto_step = 1999;
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

						if (!wtr->isBusy())
						{
							logInform(lk2->getName().c_str(), "step 1051 WTR空闲，准备取片.");
						}
						else
						{
							static int wait_count = 0;
							if ((wait_count++ % 20) == 0)
							{
								logWarn(lk2->getName().c_str(), "WTR忙碌中,等待空闲. busy=%s", wtr->isBusy() ? "true" : "false");
							}
							Sleep(200);
							break;
						}

						if (lk2->getTMCavityDoorOpend())
						{
							logInform(lk2->getName().c_str(), "step:1051,loadLockB PendingTasks 有片,触发Robot取片流程.");

							std::shared_ptr<FortrendPMCavitySubsystem> pm2Subsystem;
							if (!ensurePm2SubsystemReady(pm2Subsystem))
							{
								Sleep(200);
								break;
							}
							bool pm2HasWafer = false;
							if (pm2_exchange_in_flight.load())
							{
								static int exchange_wait_count = 0;
								if ((exchange_wait_count++ % 20) == 0)
								{
									logInform(lk2->getName().c_str(), "PM2交换进行中, LLB暂不发起新的取片请求.");
								}
								Sleep(200);
								break;
							}
							// 读取 PM 占片状态是否成功，成功读到了 PM 状态，继续取片流程；否则，等待 200ms 后重试
							if (!tryGetPmHasWafer(pm2Subsystem, "PM2", pm2HasWafer))
							{
								Sleep(200);
								break;
							}

							const auto llbSnapshot = buildLoadLockTaskSnapshot("LLB");
							if (llbSnapshot.pendingTasks.empty())
							{
								logWarn(lk2->getName().c_str(), "step 1051读取到LLB待取任务为空，回退等待后重试.");
								loadlock2_auto_step = 950;
								Sleep(200);
								break;
							}
							const int desiredArm = llbSnapshot.pendingTasks.front().arm;//任务手臂
							const bool armAHasWafer = wtr->hasObject(0);
							const bool armBHasWafer = wtr->hasObject(1);

							logWarn(lk2->getName().c_str(),
								"检测状态:pm2Has=%d, armA_has=%d, armB_has=%d",
								(int)pm2HasWafer, (int)armAHasWafer, (int)armBHasWafer);

							if (pm2HasWafer && (armAHasWafer || armBHasWafer))
							{
								llbImmediateRepick.reset();

								const bool hasSingleArmBusy = armAHasWafer ^ armBHasWafer;
								const int occupiedArm = armAHasWafer ? 0 : (armBHasWafer ? 1 : -1);

								auto returnIt = llbSnapshot.returnPendingTasks.end();
								auto pendingIt = llbSnapshot.pendingTasks.end();

								if (hasSingleArmBusy && occupiedArm >= 0)
								{
									returnIt = std::find_if(llbSnapshot.returnPendingTasks.begin(), llbSnapshot.returnPendingTasks.end(),
										[occupiedArm](const UnifiedWaferTask& task) { return task.arm == occupiedArm; });

									if (!llbSnapshot.pendingTasks.empty())
									{
										pendingIt = std::find_if(llbSnapshot.pendingTasks.begin(), llbSnapshot.pendingTasks.end(),
											[occupiedArm](const UnifiedWaferTask& task) { return task.arm == occupiedArm; });
										if (pendingIt == llbSnapshot.pendingTasks.end())
										{
											logInform(lk2->getName().c_str(),
												"LLB回片后立即补取保持固定手臂: pending队列中未找到 returnArm=%d 对应任务, 本轮跳过立即补取",
												occupiedArm);
										}
									}
								}

								if (returnIt != llbSnapshot.returnPendingTasks.end() && pendingIt != llbSnapshot.pendingTasks.end())
								{
									llbImmediateRepick.enabled = true;
									llbImmediateRepick.taskId = pendingIt->taskId;
									llbImmediateRepick.arm = pendingIt->arm;
									llbImmediateRepick.slot = pendingIt->targetFeedingSlot;

									logInform(lk2->getName().c_str(),
										"step:1051,识别到LLB回片后立即补取场景, occupiedArm=%d, repickArm=%d, repickSlot=%d, taskId=%d",
										occupiedArm, llbImmediateRepick.arm, llbImmediateRepick.slot, llbImmediateRepick.taskId);

									loadlock2_auto_step = 2000;
									break;
								}
								else
								{
									static int wait_count = 0;
									if ((wait_count++ % 20) == 0)
									{
										logWarn(lk2->getName().c_str(),
											"PM2有片且WTR手臂已有晶圆,回退到950优先处理回片/下料. pm2Has=%d, armA_has=%d, armB_has=%d",
											(int)pm2HasWafer, (int)armAHasWafer, (int)armBHasWafer);
									}
									loadlock2_auto_step = 950;
									Sleep(200);
									break;
								}
							}
							else
							{
								PM2ScheduleSnapshot pm2Snapshot;
								std::string pm2WaitReason;
								if (!tryBuildPm2ScheduleSnapshot(pm2Snapshot))
								{
									static int wait_count = 0;
									if ((wait_count++ % 20) == 0)
									{
										logWarn(lk2->getName().c_str(), "step 1051读取PM2调度快照失败，回退等待后重试.");
									}
									loadlock2_auto_step = 950;
									Sleep(200);
									break;
								}
								const bool allowPreloadWaitForCraft = canLlImmediateRepickWaitForPm2Craft(pm2Snapshot, desiredArm);
								if (!allowPreloadWaitForCraft && shouldLlWaitForPm2Priority(pm2Snapshot, desiredArm, pm2WaitReason))
								{
									static int wait_count = 0;
									if ((wait_count++ % 20) == 0)
									{
										logWarn(lk2->getName().c_str(),
											"step 1051禁止从LLB取片，当前应等待PM2优先. %s",
											pm2WaitReason.c_str());
									}
									loadlock2_auto_step = 950;
									Sleep(200);
									break;
								}
								if (allowPreloadWaitForCraft)
								{
									logInform(lk2->getName().c_str(),
										"step 1051命中预装窗口: PM2加工中,允许LLB按固定arm=%d预装下一片等待交换.",
										desiredArm);
								}

								int targetArm = desiredArm;
								std::string armBusyReason;
								if ((targetArm >= 0 && targetArm <= 1) && isRobotArmOccupiedForLlRequest(targetArm, armBusyReason))
								{
									static int wait_count = 0;
									if ((wait_count++ % 20) == 0)
									{
										logWarn(lk2->getName().c_str(),
											"WTR手臂%d发送前不可用(%s), 保持任务固定手臂并等待后重试",
											desiredArm, armBusyReason.c_str());
									}
									Sleep(200);
									break;
								}

								logInform(lk2->getName().c_str(), "step 1051 send robot_get_from_llb");
								// 设置请求标志，委托Robot线程执行
								robot_get_from_llb.arm.store(targetArm);
								robot_get_from_llb.slot.store(loadlock2_move_slot_index);
								robot_get_from_llb.done.store(false);
								robot_get_from_llb.success.store(false);
								robot_get_from_llb.expedited.store(false);
								robot_get_from_llb.requested.store(true);
								loadlock2_auto_step = 1055; // 等待Robot线程完成
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
				break;
				case 1055: // 等待Robot线程从LLB取片完成
				{
					if (robot_get_from_llb.done.load()) 
					{
						if (robot_get_from_llb.success.load()) 
						{
							loadlock2_auto_step = 1052;
						} 
						else 
						{
							logFailedExcuteCommandHasError(lk2->getName(), "取晶圆", loadlock2_process_name, loadlock2_auto_step);
							
							loadlock2_auto_step = 950; 
							Sleep(2000);
						}
					} 
					else 
					{
						// 添加等待状态提示
						static int wait_count = 0;
						if ((wait_count++ % 20) == 0) {  // 每20次循环打印一次
							logInform(lk2->getName().c_str(), "等待从LLB取晶圆完成...");
						}
						Sleep(500);
					} 

				}
				break;
				case 1052:
				{
					const auto llbSnapshot = buildLoadLockTaskSnapshot("LLB");
					if (!llbSnapshot.pendingTasks.empty())
					{
						const auto currentTask = llbSnapshot.pendingTasks.front();
						LLBPmName = getSelectPmProcessName(currentTask);

						//真空机械手取料完成
						taskManager.updateTaskStatus(currentTask.taskId, UnifiedWaferTask::TaskType::LOADLOCK_TRANSFER, UnifiedWaferTask::Status::COMPLETED);
						taskManager.updateTaskStatus(currentTask.taskId, UnifiedWaferTask::TaskType::PM_PROCESS, UnifiedWaferTask::Status::QUEUED);
					}
					else
					{
						logFailedExcuteCommandHasError(lk2->getName(), "数据更新错误", loadlock2_process_name, loadlock2_auto_step);
						loadlock2_auto_step = 950;
					}
					loadlock2_auto_step = 1053;

				}
				break;
				case 1053:
				{
					if (wtr == nullptr)
					{
						wtr = kernel->getKernelModule<FortrendSunwayRobotSubsystem>("WTR");
					}

					if (ui->enableAtmosphere->checkState() == Qt::CheckState::Checked)
					{
						logInform("cycle", "step:1052,大气模式，不关闭TM腔门!");
						tool_allow_llb = true;
						
						logInform(lk2->getName().c_str(), "step:1053,LLBPmName:%s", LLBPmName.c_str());

						if (!LLBPmName.empty())
						{
							if (LLBPmName == "PM1")
							{
								pm1_allow_get_put_wafer = true;

							}
							else if (LLBPmName == "PM2")
							{
								//pm2_allow_get_put_wafer = true;
								pm2_allow_loading_wafer = true;

							}
							else if (LLBPmName == "PM3")
							{
								pm3_allow_get_put_wafer = true;

							}
							else
							{
								pm4_allow_get_put_wafer = true;

							}
							logInform(lk2->getName().c_str(), "呼叫:%s 取放片.", LLBPmName.c_str());
						}
						else
						{
							logFailedExcuteCommandHasError(lk2->getName(), "配方解析错误", loadlock2_process_name, loadlock2_auto_step);
						}
						loadlock2_auto_step = 950;
					}
					else
					{
						//2026-3-29 避免 Resources : WTR   has be lock by WTR.
						if (lk2->getWtrOriginSafeSignal() && !wtr->isBusy())
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
								//std::string pmName = getSelectPmProcessName(loadLockBPendingTasks.at(0));
								logInform(lk2->getName().c_str(), "step:1053,LLBPmName:%s", LLBPmName.c_str());

								if (!LLBPmName.empty())
								{
									tool_allow_llb = true;

									if (LLBPmName == "PM1")
									{
										pm1_allow_get_put_wafer = true;

									}
									else if (LLBPmName == "PM2")
									{
										//pm2_allow_get_put_wafer = true;
										pm2_allow_loading_wafer = true;

									}
									else if (LLBPmName == "PM3")
									{
										pm3_allow_get_put_wafer = true;

									}
									else
									{
										pm4_allow_get_put_wafer = true;

									}
									logInform(lk2->getName().c_str(), "呼叫:%s 取放片.", LLBPmName.c_str());
								}
								else
								{
									logFailedExcuteCommandHasError(lk2->getName(), "配方解析错误", loadlock2_process_name, loadlock2_auto_step);
								}
								loadlock2_auto_step = 950;

							}
						}
						else{
					
							static int wait_log_count3 = 0;
							if (wait_log_count3++ % 50 == 0) {
								logInform(lk2->getName().c_str(), "等待 WTR 安全信号或 WTR空闲...");
							}
							Sleep(200);
						}
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
						const auto llbSnapshot = buildLoadLockTaskSnapshot("LLB");
						if (!llbSnapshot.returnPendingTasks.empty())
						{
							const auto currentTask = llbSnapshot.returnPendingTasks.front();
							loadlock2_move_slot_index = currentTask.targetBlankingSlot;
							int put_arm = currentTask.arm;

							bool haswaferarm = wtr->hasObject(put_arm);
							if (haswaferarm)
							{
								loadlock2_auto_step = 2010;
							}
							else
							{
								static int wait_count = 0;
								if ((wait_count++ % 20) == 0) {
									logInform(lk2->getName().c_str(), "WTR手臂%d没有晶圆，等待取片完成后再放片.", put_arm);
								}
								Sleep(200);
								loadlock2_auto_step = 2000;
							}
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
							if (isSimulationModeEnabled())
							{
								logInform(elp->getName().c_str(), "step:2010,LLB模拟放晶圆流程程序....");
								loadlock2_auto_step = 2070;
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
					lk1 = kernel->getKernelModule<FortrendLoadLockSubsystem>("LLA");
					if (wtr == nullptr)
					{
						wtr = kernel->getKernelModule<FortrendSunwayRobotSubsystem>("WTR");
					}
					if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL && lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						//if (lk1->getWtrOriginSafeSignal() && lk2->getWtrOriginSafeSignal() && !wtr->isBusy())
						{
							logInform("cycle", "step:2030,CheckLLBVacuum...");
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
							loadlock2_auto_step = 2060;
						}
						else
						{
							auto cmd = tm->createCloseDiaphragmValveCommand(TMCavityValveOpening::TMCavity_Both);
							tm->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError())
							{
								logFailedExcuteCommandHasError(lk2->getName(), "关闭隔膜阀失败.", loadlock2_process_name, loadlock2_auto_step);
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
								loadlock2_auto_step = 2055;
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
				case 2055:
				{
					if (wtr == nullptr)
					{
						wtr = kernel->getKernelModule<FortrendSunwayRobotSubsystem>("WTR");
					}

					std::string waitReason;
					const bool shouldWaitRobotWtr = shouldWaitBeforeStartWtrFingerQuery(wtr, waitReason);
					if (!shouldWaitRobotWtr)
					{
						auto cmd1 = wtr->createRQLoadCommand(0); //A手
						wtr->startCommand(cmd1);
						cmd1->wait();

						std::string waitReasonAfterArmA;
						const bool shouldWaitRobotWtrAfterArmA = shouldWaitBeforeStartWtrFingerQuery(wtr, waitReasonAfterArmA);
						if (shouldWaitRobotWtrAfterArmA)
						{
							static int wait_count_after_arm_a = 0;
							if ((wait_count_after_arm_a++ % 20) == 0)
							{
								logWarn(lk2->getName().c_str(), "step 2055 wait WTR finger query after arm A. %s", waitReasonAfterArmA.c_str());
							}
							Sleep(200);
							break;
						}

						auto cmd2 = wtr->createRQLoadCommand(1); //B手
						wtr->startCommand(cmd2);
						cmd2->wait();

						if (cmd1->hasError() || cmd2->hasError())
						{
							logFailedExcuteCommandHasError(lk2->getName(), "放片前查询手指有无晶圆", loadlock2_process_name, loadlock2_auto_step);
						}
						else
						{
							loadlock2_auto_step = 2060;
						}
					}
					else
					{
						static int wait_count = 0;
						if ((wait_count++ % 20) == 0)
						{
							if (!waitReason.empty())
							{
								logWarn(lk2->getName().c_str(), "step 2055 wait WTR is idle. %s", waitReason.c_str());
							}
							else
							{
								logWarn(lk2->getName().c_str(), "step 2055 wait WTR is idle.");
							}
						}
						Sleep(200);
					}
				}
				break;
				case 2060:
				{
					const auto llbSnapshot = buildLoadLockTaskSnapshot("LLB");
					if(wtr == nullptr)
					{
						wtr = kernel->getKernelModule<FortrendSunwayRobotSubsystem>("WTR");
					}
					if (llbSnapshot.returnPendingTasks.empty())
					{
						logFailed(lk2->getName(), Poco::format("%s 放片任务队列为空 %s：%d", lk2->getName(), loadlock2_process_name, loadlock2_auto_step));
						loadlock2_auto_step = 900;
						break;
					}

					if (wtr->isBusy())
					{
						static int wait_count = 0;
						if ((wait_count++ % 20) == 0)
						{
							logWarn(lk2->getName().c_str(), "step 2060 WTR忙碌中,等待空闲. busy=true");
						}
						Sleep(200);
						break;
					}

					if (llbSnapshot.returnPendingTasks.empty())
					{
						logFailed(lk2->getName(), Poco::format("%s 待回片任务为空， %s：%d", lk2->getName(), loadlock2_process_name, loadlock2_auto_step));
						loadlock2_auto_step = 900;
						break;
					}
					const auto currentReturnTask = llbSnapshot.returnPendingTasks.front();
					loadlock2_move_slot_index = currentReturnTask.targetBlankingSlot;
					const auto put_arm = currentReturnTask.arm;
					bool haswaferarm = wtr->hasObject(put_arm);

					if (!haswaferarm)
					{
						static int wait_count = 0;
						if ((wait_count++ % 20) == 0)
						{
							logWarn(lk2->getName().c_str(), "准备放片到LLB，但手指 %d 还没有片子，等待WTR取片完成...", put_arm);
						}
						Sleep(200);
						loadlock2_auto_step = 2055; // 回退到2055重新查询
						break;
					}

					logInform(lk2->getName().c_str(), "准备放片到LLB，目标槽位%d，使用手臂%d", loadlock2_move_slot_index, put_arm);

					if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL && lk2->hasDoorOpend())
					{
						//robot 把手臂A的放到Loadlock2
						if(lk2->getTMCavityDoorOpend() && haswaferarm)
						{
							// 设置请求标志，委托Robot线程执行放片到LLB
							llb_return_task_id.store(currentReturnTask.taskId);
							llb_return_slot.store(currentReturnTask.targetBlankingSlot);
							robot_put_to_llb.arm.store(put_arm);
							robot_put_to_llb.slot.store(loadlock2_move_slot_index);
							robot_put_to_llb.done.store(false);
							robot_put_to_llb.success.store(false);
							robot_put_to_llb.requested.store(true);
							loadlock2_auto_step = 2065; // 等待Robot线程完成
						}
						else
						{
							llb_return_task_id.store(-1);
							llb_return_slot.store(-1);
							loadlock2_auto_step = 2000; //跳转到2000
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
				case 2065: // 等待Robot线程放片到LLB完成
				{
					if (robot_put_to_llb.done.load())
					{
						if (robot_put_to_llb.success.load())
						{
							loadlock2_auto_step = llbImmediateRepick.enabled ? 2066 : 2070;
						}
						else 
						{
								llb_return_task_id.store(-1);
								llb_return_slot.store(-1);
							llbImmediateRepick.reset();
							logFailed(wtr->getName(), "LLB放片失败,step=2065");
							Sleep(2000);
							loadlock2_auto_step = 950; // 回退重试
						}
					} else 
					{
						// 添加等待状态提示
						static int wait_count = 0;
						if ((wait_count++ % 20) == 0) {
							logInform(wtr->getName().c_str(), "step 2065,等待放LLB晶圆完成...");
						}
						Sleep(500);
					}
				}
				break;
				case 2066:
				{
					auto llbSnapshot = buildLoadLockTaskSnapshot("LLB");
					if (llbSnapshot.returnPendingTasks.empty())
					{
						logFailed(lk2->getName(), Poco::format("%s 回片成功后找不到待回片任务， %s：%d", lk2->getName(), loadlock2_process_name, loadlock2_auto_step));
						llbImmediateRepick.reset();
						loadlock2_auto_step = 900;
						break;
					}

					const int returnTaskId = llb_return_task_id.load();
					const auto returnTaskIt = std::find_if(llbSnapshot.returnPendingTasks.begin(), llbSnapshot.returnPendingTasks.end(),
						[returnTaskId](const UnifiedWaferTask& task) { return task.taskId == returnTaskId; });
					if (returnTaskIt == llbSnapshot.returnPendingTasks.end())
					{
						logError(lk2->getName().c_str(), "LLB return completion lost locked return task: taskId=%d, refuse incorrect status writeback.", returnTaskId);
						llb_return_task_id.store(-1);
						llb_return_slot.store(-1);
						llbImmediateRepick.reset();
						loadlock2_auto_step = 900;
						break;
					}
					taskManager.updateTaskStatus(returnTaskIt->taskId, UnifiedWaferTask::LOADLOCK_RETURN, UnifiedWaferTask::COMPLETED);
					llb_return_task_id.store(-1);
					llbSnapshot = buildLoadLockTaskSnapshot("LLB");

					// 检测是否满足立即补取条件，满足则直接进入补取流程，不满足则进入正常收尾流程
					if (!llbImmediateRepick.enabled)
					{
						loadlock2_auto_step = 2071;
						break;
					}

					auto repickTaskIt = std::find_if(llbSnapshot.pendingTasks.begin(), llbSnapshot.pendingTasks.end(),
						[this](const UnifiedWaferTask& task) { return task.taskId == llbImmediateRepick.taskId; });

					if (repickTaskIt == llbSnapshot.pendingTasks.end())
					{
						logWarn(lk2->getName().c_str(), "LLB立即补取目标task已不存在，跳过补取并继续收尾.");
						llbImmediateRepick.reset();
						loadlock2_auto_step = 2071;
						break;
					}
					if (repickTaskIt->arm != llbImmediateRepick.arm)
					{
						logWarn(lk2->getName().c_str(),
							"LLB立即补取目标task固定手臂不匹配: taskId=%d, configuredArm=%d, plannedArm=%d，跳过补取.",
							repickTaskIt->taskId, repickTaskIt->arm, llbImmediateRepick.arm);
						llbImmediateRepick.reset();
						loadlock2_auto_step = 2071;
						break;
					}

					if (!lk2->getTMCavityDoorOpend())
					{
						logWarn(lk2->getName().c_str(),
							"LLB立即补取前状态不满足: tmDoor=%d, arm=%d，跳过补取.",
							(int)lk2->getTMCavityDoorOpend(),
							llbImmediateRepick.arm);
						llbImmediateRepick.reset();
						loadlock2_auto_step = 2071;
						break;
					}

					PM2ScheduleSnapshot pm2Snapshot;
					std::string pm2WaitReason;
					if (!tryBuildPm2ScheduleSnapshot(pm2Snapshot))
					{
						logWarn(lk2->getName().c_str(), "LLB立即补取前读取PM2调度快照失败，跳过补取.");
						llbImmediateRepick.reset();
						loadlock2_auto_step = 2071;
						break;
					}
					const bool allowRepickWaitForCraft = canLlImmediateRepickWaitForPm2Craft(pm2Snapshot, llbImmediateRepick.arm);
					if (!allowRepickWaitForCraft && shouldLlWaitForPm2Priority(pm2Snapshot, llbImmediateRepick.arm, pm2WaitReason))
					{
						logWarn(lk2->getName().c_str(),
							"LLB立即补取前检测到当前应等待PM2优先，跳过补取. arm=%d, %s",
							llbImmediateRepick.arm, pm2WaitReason.c_str());
						llbImmediateRepick.reset();
						loadlock2_auto_step = 2071;
						break;
					}
					if (allowRepickWaitForCraft)
					{
						logInform(lk2->getName().c_str(),
							"LLB立即补取命中预装窗口: PM2仍在加工, 允许固定arm=%d预装下一片在手等待交换.",
							llbImmediateRepick.arm);
					}

					std::string armBlockedReason;
					if (isRobotArmOccupiedForLlRequest(llbImmediateRepick.arm, armBlockedReason))
					{
						logWarn(lk2->getName().c_str(),
							"LLB立即补取前状态不满足: arm=%d, blockReason=%s，跳过补取.",
							llbImmediateRepick.arm, armBlockedReason.c_str());
						llbImmediateRepick.reset();
						loadlock2_auto_step = 2071;
						break;
					}

					loadlock2_move_slot_index = repickTaskIt->targetFeedingSlot;
					robot_get_from_llb.arm.store(llbImmediateRepick.arm);
					robot_get_from_llb.slot.store(loadlock2_move_slot_index);
					robot_get_from_llb.done.store(false);
					robot_get_from_llb.success.store(false);
					robot_get_from_llb.expedited.store(true);
					robot_get_from_llb.requested.store(true);
					logInform(lk2->getName().c_str(),
						"LLB回片后立即补取一片, arm=%d, slot=%d, taskId=%d",
						llbImmediateRepick.arm, loadlock2_move_slot_index, llbImmediateRepick.taskId);
					loadlock2_auto_step = 2067;
				}
				break;
				case 2067:
				{
					if (robot_get_from_llb.done.load())
					{
						if (robot_get_from_llb.success.load())
						{
							taskManager.updateTaskStatus(llbImmediateRepick.taskId, UnifiedWaferTask::LOADLOCK_TRANSFER, UnifiedWaferTask::COMPLETED);
							taskManager.updateTaskStatus(llbImmediateRepick.taskId, UnifiedWaferTask::PM_PROCESS, UnifiedWaferTask::QUEUED);
							logInform(lk2->getName().c_str(),
								"LLB回片后立即补取完成, arm=%d, slot=%d, taskId=%d",
								llbImmediateRepick.arm, llbImmediateRepick.slot, llbImmediateRepick.taskId);
						}
						else
						{
							logWarn(lk2->getName().c_str(), "LLB回片后立即补取失败，继续后续关门收尾.");
						}

						llbImmediateRepick.reset();
						loadlock2_auto_step = 2071;
					}
					else
					{
						static int wait_count = 0;
						if ((wait_count++ % 20) == 0) {
							logInform(lk2->getName().c_str(), "step 2067,等待LLB回片后的立即补取完成...");
						}
						Sleep(200);
					}
				}
				break;
				case 2070:
				{
					//下料到LL的晶圆 8->3
					const auto llbSnapshot = buildLoadLockTaskSnapshot("LLB");
					if (llbSnapshot.returnPendingTasks.empty())
					{
						logFailed(lk2->getName(), Poco::format("%s 放片完成但任务队列为空， %s：%d", lk2->getName(), loadlock2_process_name, loadlock2_auto_step));
						loadlock2_auto_step = 900;
						break;
					}
					const int returnTaskId = llb_return_task_id.load();
					const auto returnTaskIt = std::find_if(llbSnapshot.returnPendingTasks.begin(), llbSnapshot.returnPendingTasks.end(),
						[returnTaskId](const UnifiedWaferTask& task) { return task.taskId == returnTaskId; });
					if (returnTaskIt == llbSnapshot.returnPendingTasks.end())
					{
						logError(lk2->getName().c_str(), "LLB put completion lost locked return task: taskId=%d, refuse incorrect status writeback.", returnTaskId);
						llb_return_task_id.store(-1);
						llb_return_slot.store(-1);
						loadlock2_auto_step = 900;
						break;
					}
					taskManager.updateTaskStatus(returnTaskIt->taskId, UnifiedWaferTask::LOADLOCK_RETURN, UnifiedWaferTask::Status::COMPLETED);
					llb_return_task_id.store(-1);

					loadlock2_auto_step = 2071;
				}
				break;
				case 2071:
				{
					if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						if (ui->enableAtmosphere->checkState() == Qt::CheckState::Checked)
						{
							logInform("cycle", "step:2070,大气模式，不关闭TM腔门!");
							loadlock2_auto_step = 2080;
						}
						else
						{
							const auto llbSnapshot = buildLoadLockTaskSnapshot("LLB");
							if (!llbSnapshot.pendingTasks.empty())
							{
								const int desiredArm = llbSnapshot.pendingTasks.front().arm;
								PM2ScheduleSnapshot pm2Snapshot;
								if (tryBuildPm2ScheduleSnapshot(pm2Snapshot))
								{
									const bool allowPreloadBeforeUnload = canLlImmediateRepickWaitForPm2Craft(pm2Snapshot, desiredArm);
									if (allowPreloadBeforeUnload)
									{
										std::string armBusyReason;
										if (!isRobotArmOccupiedForLlRequest(desiredArm, armBusyReason))
										{
											loadlock2_move_slot_index = llbSnapshot.pendingTasks.front().targetFeedingSlot;
											logInform(lk2->getName().c_str(),
												"LLB关门破真空前命中预装窗口: 先按固定arm=%d预装下一片, 再执行下料收尾.",
												desiredArm);
											llb_return_slot.store(-1);
											robot_get_from_llb.arm.store(desiredArm);
											robot_get_from_llb.slot.store(loadlock2_move_slot_index);
											robot_get_from_llb.done.store(false);
											robot_get_from_llb.success.store(false);
											robot_get_from_llb.expedited.store(false);
											robot_get_from_llb.requested.store(true);
											loadlock2_auto_step = 1055;
											break;
										}
									}
								}
							}

							//2026-3-27 检测mapping，再去关门
							auto cassManager = lk2->getKernel()->getKernelModule<FortrendCassetteManager>();
							auto station_cass = cassManager->getCassette(lk2.get());
							const int returnSlot = llb_return_slot.load();
							if (returnSlot > 0 && station_cass->getMapping(returnSlot) == Cassette::Mapping::Present)
							{
								if (lk2->getWtrOriginSafeSignal() && !wtr->isBusy())
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
										llb_return_slot.store(-1);
										loadlock2_auto_step = 2080;
									}
								}
								else
								{
									static int wait_log_count4 = 0;
									if (wait_log_count4++ % 50 == 0) {
										logInform(lk2->getName().c_str(), "等待 WTR 安全信号或 WTR空闲...");
									}
									Sleep(200);
								}
							}
							else
							{
								llb_return_slot.store(-1);
								logFailedExcuteCommandHasError(lk2->getName(), "检测mapping无片，不执行关闭传输腔门阀", loadlock2_process_name, loadlock2_auto_step);
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
					if (ewtr == nullptr)
					{
						ewtr = kernel->getKernelModule<EFEMWaferRobotSubsystem>("EWTR");
					}

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
								if (!ewtr->isBusy())
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
									static int wait_count = 0;
									if ((wait_count++ % 20) == 0) {
										logWarn(lk2->getName().c_str(), "等待 EWTR 空闲...");
									}
									Sleep(200);
									loadlock2_auto_step = 5025; //回退继续等待
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
					const auto llbPendingSnapshot = taskManager.getLoadLockPendingTasks("LLB");
					if (llbPendingSnapshot.empty() && current_lp_cycle)
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
		}
		catch (const std::exception& e) {
			pauseAllThreads();
			saveCurrentStateSnapshot(e.what(), Poco::format("LLBTransfer step:%d", loadlock2_auto_step));
			logError("Cyclelog", "LLBTransfer thread crashed:%s", e.what());
			qCritical() << "LLBTransfer thread crashed:" << e.what();
		}
		catch (...) {
			pauseAllThreads();
			saveCurrentStateSnapshot("unknown exception", Poco::format("LLBTransfer step:%d", loadlock2_auto_step));
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

							//要区分那个LL腔...
							
							const auto llaSnapshot = buildLoadLockTaskSnapshot("LLA");
							if (!llaSnapshot.completedTasks.empty())
							{
								logInform("PM1", "更新LLA:LOADLOCK_TRANSFER/COMPLETED ------>PM_PROCESS/QUEUED.");
								for (const auto& task : llaSnapshot.completedTasks)
								{
									taskManager.updateTaskStatus(task.taskId, UnifiedWaferTask::TaskType::PM_PROCESS, UnifiedWaferTask::Status::QUEUED);
								}
							}

							const auto llbSnapshot = buildLoadLockTaskSnapshot("LLB");
							if (!llbSnapshot.completedTasks.empty())
							{
								logInform("PM1", "更新LLB:LOADLOCK_TRANSFER/COMPLETED ------>PM_PROCESS/QUEUED.");
								for (const auto& task : llbSnapshot.completedTasks)
								{
									taskManager.updateTaskStatus(task.taskId, UnifiedWaferTask::TaskType::PM_PROCESS, UnifiedWaferTask::Status::QUEUED);
								}
							}
							logInform("Cycle", Poco::format("%s = %d", pm1_process_name, pm1_auto_step.load()).c_str());
							pm1_auto_step.store(100);
						}
						else if (pm1_allow_goto_craft)
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
						const auto pm1Snapshot = buildPmTaskSnapshot("PM1");
						//判断是放、取,考虑交互手
						if (!pm1Snapshot.pendingTasks.empty() || !pm1Snapshot.completedTasks.empty())
						{
							if (isSimulationModeEnabled())
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
						std::string waitReason;
						const bool shouldWaitRobotWtr = shouldWaitBeforeStartWtrFingerQuery(wtr, waitReason);
						if (!shouldWaitRobotWtr)
						{
							auto cmd1 = wtr->createRQLoadCommand(0); //A手
							wtr->startCommand(cmd1);
							cmd1->wait();

							std::string waitReasonAfterArmA;
							const bool shouldWaitRobotWtrAfterArmA = shouldWaitBeforeStartWtrFingerQuery(wtr, waitReasonAfterArmA);
							if (shouldWaitRobotWtrAfterArmA)
							{
								static int wait_count_after_arm_a = 0;
								if ((wait_count_after_arm_a++ % 20) == 0)
								{
									logWarn(wtr->getName().c_str(), "PM1 step 199 wait WTR finger query after arm A. %s", waitReasonAfterArmA.c_str());
								}
								Sleep(200);
								break;
							}


							auto cmd2 = wtr->createRQLoadCommand(1); //B手
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
							static int wait_count = 0;
							if ((wait_count++ % 20) == 0)
							{
								if (!waitReason.empty())
								{
									logWarn(wtr->getName().c_str(), "PM1 step 199 wait WTR finger query. %s", waitReason.c_str());
								}
								else if (wtr->isBusy())
								{
									logWarn(wtr->getName().c_str(), "PM1 step 199 wait WTR finger query. WTR busy.");
								}
								else
								{
									logWarn(wtr->getName().c_str(), "PM1 step 199 wait WTR finger query. WTR state not normal.");
								}
							}
							Sleep(200);
						}
					
					}
					break;
					case 200:
					{
						//考虑交互手
						if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
							if (isSimulationModeEnabled())
							{
								pm1_auto_step.store(1010);
								//pm1_auto_step = 1010;
							}
							else
							{
								// 使用hasObject(arm)检查手臂状态，更符合硬件安全检查机制
								bool haswaferpm = cassManager->getCassette(pm1.get())->getMapping(1) == Cassette::Present;   //pm中有片
								bool haswaferarm1 = wtr->hasObject(0); //arm1有片  A臂(索引0)
								bool haswaferarm2 = wtr->hasObject(1); //arm2有片  B臂(索引1)	
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
									pm1_auto_step = 1040;
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
								// 设置请求标志，委托Robot线程执行放片到PM1
								robot_put_to_pm1.arm.store(robot_selected_arm);
								robot_put_to_pm1.done.store(false);
								robot_put_to_pm1.success.store(false);
								robot_put_to_pm1.requested.store(true);
								pm1_auto_step.store(1015); // 等待Robot线程完成
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
					case 1015: // 等待Robot线程放片到PM1完成
					{
						if (robot_put_to_pm1.done.load()) {
							if (robot_put_to_pm1.success.load()) {
								pm1_allow_get_put_wafer = false;
								pm1_auto_step.store(2000);
							} else {
								logInform(wtr->getName().c_str(), "PM1放片失败，2秒后重试 step=%d", pm1_auto_step.load());
								Sleep(2000);
								if (robot_put_to_pm1.arm.load() == 0)
									pm1_auto_step.store(1010);
								else
									pm1_auto_step.store(1030);
							}
						} else if (!robot_put_to_pm1.requested.load()) {
							logInform(wtr->getName().c_str(), "PM1 1015步：请求标志异常(done=false,requested=false)，回退重发 step=%d", pm1_auto_step.load());
							Sleep(1000);
							if (robot_put_to_pm1.arm.load() == 0)
								pm1_auto_step.store(1010);
							else
								pm1_auto_step.store(1030);
						}
					}
					break;
					case 1030:
					{//放片 - B臂放到PM1
						robot_put_to_pm1.arm.store(1);
						robot_put_to_pm1.slot.store(1);
						robot_put_to_pm1.done.store(false);
						robot_put_to_pm1.success.store(false);
						robot_put_to_pm1.requested.store(true);
						pm1_auto_step.store(1015); // 复用同一个等待步骤
					}
					break;
					case 1040:
					{
						// 取片 - A臂从PM1取片
						robot_get_from_pm1.arm.store(0);
						robot_get_from_pm1.done.store(false);
						robot_get_from_pm1.success.store(false);
						robot_get_from_pm1.requested.store(true);
						pm1_auto_step.store(1045);
					}
					break;
					case 1045:
					{
						// 等待取片完成
						if (robot_get_from_pm1.done.load())
						{
							if (robot_get_from_pm1.success.load())
							{
								const auto pm1Snapshot = buildPmTaskSnapshot("PM1");
								if (pm1Snapshot.pendingTasks.empty())
								{
									logWarn(wtr->getName().c_str(), "PM1取片完成但待加工任务为空，回退检查 step=%d", pm1_auto_step.load());
									pm1_auto_step.store(10);
									break;
								}
								const auto currentTask = pm1Snapshot.pendingTasks.front();
								taskManager.updateTaskStatus(currentTask.taskId, UnifiedWaferTask::TaskType::PM_PROCESS, UnifiedWaferTask::Status::COMPLETED);
								taskManager.updateTaskStatus(currentTask.taskId, UnifiedWaferTask::TaskType::LOADLOCK_RETURN, UnifiedWaferTask::Status::QUEUED);
								pm1_allow_get_put_wafer = false;
								pm1_auto_step.store(10);
							}
							else
							{
								logFailed(wtr->getName(), Poco::format("PM1取片失败(A臂)， %s：%d", pm1_process_name, pm1_auto_step.load()));
							}
						}
					}
					break;
					case 1050:
					{
						//取片
						if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
							robot_selected_arm = 1;
							// 设置请求标志，委托Robot线程执行从PM1取片（B臂）
							robot_get_from_pm1.arm.store(robot_selected_arm);
							robot_get_from_pm1.done.store(false);
							robot_get_from_pm1.success.store(false);
							robot_get_from_pm1.requested.store(true);
							pm1_auto_step.store(1045); // 复用同一个等待步骤
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
						// 交换：A臂取 + B臂放 (先取后放)
						exchange_info_pm1.getArm.store(0);
						exchange_info_pm1.putArm.store(1);
						robot_exchange_pm1.done.store(false);
						robot_exchange_pm1.success.store(false);
						robot_exchange_pm1.requested.store(true);
						pm1_auto_step.store(1065);
					}
					break;
					case 1065:
					{
						// 等待exchange完成
						if (robot_exchange_pm1.done.load())
						{
							if (robot_exchange_pm1.success.load())
							{
								const auto pm1Snapshot = buildPmTaskSnapshot("PM1");
								// 防御性检查：防止vector为空导致at(0)崩溃
								if (pm1Snapshot.completedTasks.empty()) {
									logWarn(wtr->getName().c_str(), "PM1交换完成但无已完成任务，回退检查 step=%d", pm1_auto_step.load());
									pm1_auto_step.store(1060);
									return;
								}
								taskManager.updateTaskStatus(pm1Snapshot.completedTasks.front().taskId, UnifiedWaferTask::TaskType::LOADLOCK_RETURN, UnifiedWaferTask::Status::QUEUED);
								pm1_allow_get_put_wafer = false;
								pm1_allow_goto_craft = true;
								pm1_auto_step.store(10);
							}
							else
							{
								logInform(wtr->getName().c_str(), "PM1交换料失败(A取B放)，2秒后重试 step=%d", pm1_auto_step.load());
								Sleep(2000);
								pm1_auto_step.store(1060);
							}
						} else if (!robot_exchange_pm1.requested.load()) {
							logInform(wtr->getName().c_str(), "PM1 1065步：请求标志异常(done=false,requested=false)，回退重发 step=%d", pm1_auto_step.load());
							Sleep(1000);
							pm1_auto_step.store(1060);
						}
					}
					break;
					case 1070:
					{
						// 交换：B臂取 + A臂放 (先取后放)
						exchange_info_pm1.getArm.store(1);
						exchange_info_pm1.putArm.store(0);
						robot_exchange_pm1.done.store(false);
						robot_exchange_pm1.success.store(false);
						robot_exchange_pm1.requested.store(true);
						pm1_auto_step.store(1075);
					}
					break;
					case 1075:
					{
						// 等待exchange完成
						if (robot_exchange_pm1.done.load())
						{
							if (robot_exchange_pm1.success.load())
							{
								const auto pm1Snapshot = buildPmTaskSnapshot("PM1");
								// 防御性检查：防止vector为空导致at(0)崩溃
								if (pm1Snapshot.completedTasks.empty()) {
									logWarn(wtr->getName().c_str(), "PM1交换完成但无已完成任务，回退检查 step=%d", pm1_auto_step.load());
									pm1_auto_step.store(1070);
									return;
								}
								taskManager.updateTaskStatus(pm1Snapshot.completedTasks.front().taskId, UnifiedWaferTask::TaskType::LOADLOCK_RETURN, UnifiedWaferTask::Status::QUEUED);
								pm1_allow_get_put_wafer = false;
								pm1_allow_goto_craft = true;
								pm1_auto_step.store(10);
							}
							else
							{
								logInform(wtr->getName().c_str(), "PM1交换料失败(B取A放)，2秒后重试 step=%d", pm1_auto_step.load());
								Sleep(2000);
								pm1_auto_step.store(1070);
							}
						} else if (!robot_exchange_pm1.requested.load()) {
							logInform(wtr->getName().c_str(), "PM1 1075步：请求标志异常(done=false,requested=false)，回退重发 step=%d", pm1_auto_step.load());
							Sleep(1000);
							pm1_auto_step.store(1070);
						}
					}
					break;

					case 1090:
					{
						// 最终取片（仅取，不放）：A臂从PM1取片
						robot_get_from_pm1.arm.store(0);
						robot_get_from_pm1.done.store(false);
						robot_get_from_pm1.success.store(false);
						robot_get_from_pm1.requested.store(true);
						pm1_auto_step.store(1095);
					}
					break;
					case 1095:
					{
						// 等待取片完成
						if (robot_get_from_pm1.done.load())
						{
							if (robot_get_from_pm1.success.load())
							{
								pm1_allow_get_put_wafer = false;
								const auto pm1Snapshot = buildPmTaskSnapshot("PM1");
								// 防御性检查：防止vector为空导致at(0)崩溃
								if (pm1Snapshot.completedTasks.empty()) {
									logWarn(wtr->getName().c_str(), "PM1最终取片完成但无已完成任务，回退检查 step=%d", pm1_auto_step.load());
									pm1_auto_step.store(1090);
									return;
								}
								taskManager.updateTaskStatus(pm1Snapshot.completedTasks.front().taskId, UnifiedWaferTask::TaskType::LOADLOCK_RETURN, UnifiedWaferTask::Status::QUEUED);
								pm1_auto_step.store(10);
							}
							else
							{
								logInform(wtr->getName().c_str(), "PM1最终取片失败，2秒后重试 step=%d", pm1_auto_step.load());
								Sleep(2000);
								pm1_auto_step.store(1090);
							}
						} else if (!robot_get_from_pm1.requested.load()) {
							logInform(wtr->getName().c_str(), "PM1 1095步：请求标志异常(done=false,requested=false)，回退重发 step=%d", pm1_auto_step.load());
							Sleep(1000);
							pm1_auto_step.store(1090);
						}
					}
					break;
#pragma endregion
					//工艺步骤
					case 2000:
					{
						UpdatePmSubTransferDatas("PM1");

						if (pm1 != nullptr)
						{
							if (pm1->getWithWaferModeEnable())
							{
								if (!runPmRecipeMotorOnUiThread(0, pm1->getName().c_str(), pm1_process_name, pm1_auto_step))
								{
									break;
								}
							}
							else
							{
								logFailed(pm1->getName(), Poco::format("%s 没有片，无法执行工艺流程， %s：%d", pm1->getName(), pm1_process_name, pm1_auto_step.load()));
							}
						}
						else
						{
							logFailed(wtr->getName(), Poco::format("PM1模块指针为空， %s：%d", pm1_process_name, pm1_auto_step.load()));
						}

						//Sleep(200);
						//logInform("PM1", "2s延迟，来模拟做工艺流程.....");

						const auto pm1Snapshot = buildPmTaskSnapshot("PM1");
						if (!pm1Snapshot.pendingTasks.empty())
						{
							const auto currentTask = pm1Snapshot.pendingTasks.front();
							taskManager.updateTaskStatus(currentTask.taskId, UnifiedWaferTask::TaskType::PM_PROCESS, UnifiedWaferTask::Status::COMPLETED);
							taskManager.updateTaskStatus(currentTask.taskId, UnifiedWaferTask::TaskType::LOADLOCK_RETURN, UnifiedWaferTask::Status::QUEUED);
							pm1_auto_step.store(10);
						}
						else
						{
							pm1_auto_step.store(10);
						}
					}
					break;
					case 2001:
					{

						const auto pm1Snapshot = buildPmTaskSnapshot("PM1");
						if(!pm1Snapshot.completedTasks.empty())
						{
							taskManager.updateTaskStatus(pm1Snapshot.completedTasks.at(0).taskId, UnifiedWaferTask::TaskType::LOADLOCK_RETURN, UnifiedWaferTask::Status::QUEUED);//7
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
			pauseAllThreads();
			saveCurrentStateSnapshot(e.what(), Poco::format("PM1Transfer step:%d", pm1_auto_step.load()));
			onUpdateControlEnabled("execute_pbt", true);
			onUpdateLightButtonStatus("light_running_pbt", 3);

			logError("Cyclelog", "PM1Transfer thread crashed:", e.what());
			qCritical() << "PM1Transfer thread crashed:" << e.what();
		}
		catch (...) {
			pauseAllThreads();
			saveCurrentStateSnapshot("unknown exception", Poco::format("PM1Transfer step:%d", pm1_auto_step.load()));
			onUpdateControlEnabled("execute_pbt", true);
			onUpdateLightButtonStatus("light_running_pbt", 3);

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
						pm2_need_return_wafer = false;
						pm2_craft_task_id.store(-1);
						needReset_PM2 = false;
						continue; // 跳过本次循环剩余部分
					}

					pm_step_once_finished = false;
					
					switch (pm2_auto_step.load())
					{
					case 10:
					{
						pm2_auto_step.store(100);

						if (pm2_allow_goto_craft)
						{//转工艺
							pm2_auto_step.store(2000);
						}
						else {
							pm2_auto_step.store(100);
							
						}
						Sleep(200);
					}
					break;

					case 100:
					{
						const auto pm2Snapshot = buildPmTaskSnapshot("PM2");

						auto loadlockReturnPendingTasks = taskManager.getLoadLockReturnPendingTasks();

						loadlockReturnPendingTasks.erase(
							std::remove_if(loadlockReturnPendingTasks.begin(), loadlockReturnPendingTasks.end(),
								[](const UnifiedWaferTask& task)
								{
									return task.target_pm != UnifiedWaferTask::PM2;
								}),
							loadlockReturnPendingTasks.end());


						if (!pm2Snapshot.pendingTasks.empty() ||  loadlockReturnPendingTasks.size() > 0)
						{
							if (isSimulationModeEnabled())
							{
								pm2_auto_step.store(200); //取放片
							}
							else
							{
								//2026-5-17,不能一直查询RQLoad，会污染wtr收取放rps信息
								//pm2_auto_step.store(199); //取放片
								pm2_auto_step.store(200);
							}
							
						}
						else
						{
							pm2_auto_step.store(10);
							Sleep(10);
						}
					}
					break;
					case 199:
					{
						std::string waitReason;
						const bool shouldWaitRobotWtr = shouldWaitBeforeStartWtrFingerQuery(wtr, waitReason);
						if (!shouldWaitRobotWtr)
						{
							auto cmd1 = wtr->createRQLoadCommand(0); //1手
							wtr->startCommand(cmd1);
							cmd1->wait();

							std::string waitReasonAfterArmA;
							const bool shouldWaitRobotWtrAfterArmA = shouldWaitBeforeStartWtrFingerQuery(wtr, waitReasonAfterArmA);
							if (shouldWaitRobotWtrAfterArmA)
							{
								static int wait_count_after_arm_a = 0;
								if ((wait_count_after_arm_a++ % 20) == 0)
								{
									logWarn(wtr->getName().c_str(), "PM2 step 199 wait WTR finger query after arm A. %s", waitReasonAfterArmA.c_str());
								}
								Sleep(200);
								break;
							}


							auto cmd2 = wtr->createRQLoadCommand(1); //1手
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
							static int wait_count = 0;
							if ((wait_count++ % 20) == 0)
							{
								if (!waitReason.empty())
								{
									logWarn(wtr->getName().c_str(), "PM2 step 199 wait WTR finger query. %s", waitReason.c_str());
								}
								else if (wtr->isBusy())
								{
									logWarn(wtr->getName().c_str(), "PM2 step 199 wait WTR finger query. WTR busy.");
								}
								else
								{
									logWarn(wtr->getName().c_str(), "PM2 step 199 wait WTR finger query. WTR state not normal.");
								}
							}
							Sleep(200);
						}

					}
					break;
					case 200:
					{
						if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
							if (isSimulationModeEnabled())
							{
								pm2_auto_step.store(1030);
							}
							else
							{
								bool haswaferpm = false;
								if (!tryGetPmHasWafer(pm2, "PM2", haswaferpm))
								{
									pm2_auto_step.store(10);
									Sleep(100);
									break;
								}
								bool haswaferarm1 = wtr->hasObject(0); //arm1有片  A臂(索引0)
								bool haswaferarm2 = wtr->hasObject(1); //arm2有片  B臂(索引1)
								const auto pm2Snapshot = buildPmTaskSnapshot("PM2");

								auto loadlockReturnPendingTasks = taskManager.getLoadLockReturnPendingTasks();
								loadlockReturnPendingTasks.erase(
									std::remove_if(loadlockReturnPendingTasks.begin(), loadlockReturnPendingTasks.end(),
										[](const UnifiedWaferTask& task)
										{
											return task.target_pm != UnifiedWaferTask::PM2;
										}),
									loadlockReturnPendingTasks.end());

								bool arm1HasPending = haswaferarm1 && std::any_of(pm2Snapshot.pendingTasks.begin(), pm2Snapshot.pendingTasks.end(), [](const UnifiedWaferTask& t) { return t.arm == 0; });
								bool arm2HasPending = haswaferarm2 && std::any_of(pm2Snapshot.pendingTasks.begin(), pm2Snapshot.pendingTasks.end(), [](const UnifiedWaferTask& t) { return t.arm == 1; });

								logInform("PM2",
									"PM2调度200: haswaferpm=%d, armA_has=%d, armA_pending=%d, armB_has=%d, armB_pending=%d, pending=%d, return_pending=%d",
									(int)haswaferpm,
									(int)haswaferarm1,
									(int)arm1HasPending,
									(int)haswaferarm2,
									(int)arm2HasPending,
									(int)pm2Snapshot.pendingTasks.size(),
									(int)loadlockReturnPendingTasks.size());

								//pm上料
								if (!haswaferpm)
								{
									if (!pm2Snapshot.pendingTasks.empty())
									{
										if (arm1HasPending)
										{//A手放料

											pm2_auto_step.store(1010);
										}
										else if (arm2HasPending)
										{//B手放料

											pm2_auto_step.store(1030);
										}
										else if (!haswaferarm1 && !haswaferarm2)
										{
											logInform("PM2", "PM2没有晶圆，两个手臂也没有晶圆，等待放料...");
											pm2_auto_step.store(10);
										}
										else
										{
											logWarn("PM2", "PM2无片但手臂有片，且未匹配到待加工任务，等待修正...");
											pm2_auto_step.store(10);
											Sleep(100);
										}
									}
									else
									{
										static int wait_count = 0;
										if ((wait_count++ % 20) == 0) {
											logInform("PM2", "PM2无片且没有待加工任务，等待中...");
										}
										Sleep(500);
										pm2_auto_step.store(10);
										
									}
								}
								else //PM下料
								{
									if (!haswaferarm1 && !haswaferarm2)
									{
										//查找pm中晶圆task,找到arm
										if (loadlockReturnPendingTasks.size() > 0)
										{
											auto task = loadlockReturnPendingTasks.at(0);
											if (task.arm == 0)
											{
												logInform("PM2", "step:200 send--->A臂从PM2取片,因为有待下料任务且是A臂的.");
												pm2_auto_step.store(1040);
											}
											else if (task.arm == 1)
											{
												logInform("PM2", "step:200 send--->B臂从PM2取片,因为有待下料任务且是B臂的.");
												pm2_auto_step.store(1050);
											}
											else
											{
												logWarn("PM2", "待下料任务的arm字段异常，无法判断是A臂还是B臂取片，等待中...");
												pm2_auto_step.store(10);
												Sleep(100);
											}
										}
										else if (!pm2Snapshot.completedTasks.empty())
										{
											auto task = pm2Snapshot.completedTasks.front();
											if (task.arm == 0)
											{
												logInform("PM2", "step:200 send--->A臂从PM2取片,因为PM完成任务记录为A臂.");
												pm2_auto_step.store(1040);
											}
											else if (task.arm == 1)
											{
												logInform("PM2", "step:200 send--->B臂从PM2取片,因为PM完成任务记录为B臂.");
												pm2_auto_step.store(1050);
											}
											else
											{
												logWarn("PM2", "PM完成任务的arm字段异常，无法判断是A臂还是B臂取片，等待中...");
												pm2_auto_step.store(10);
												Sleep(100);
											}
										}
										else
										{
											logWarn("PM2", "PM2有片但没有待下料任务，固定手臂规则下不默认换臂取片，等待任务对齐...");
											pm2_auto_step.store(10);
											Sleep(100);
										}

									}
									else
									{
										if (!pm2Snapshot.pendingTasks.empty())
										{
											if (pm2_exchange_in_flight.load() || robot_exchange_pm2.requested.load())
											{
												logInform("PM2", "PM2交换请求已在处理中，等待Robot线程消费. step=%d", pm2_auto_step.load());
												pm2_auto_step.store(10);
												Sleep(100);
												break;
											}
											int returnTaskArm = -1;
											if (!loadlockReturnPendingTasks.empty())
											{
												returnTaskArm = loadlockReturnPendingTasks.front().arm;
											}
											else if (!pm2Snapshot.completedTasks.empty())
											{
												returnTaskArm = pm2Snapshot.completedTasks.front().arm;
											}

											if (returnTaskArm == 0 && !haswaferarm1 && arm2HasPending)
											{
												const auto pendingIt = std::find_if(pm2Snapshot.pendingTasks.begin(), pm2Snapshot.pendingTasks.end(),
													[](const UnifiedWaferTask& task) { return task.arm == 1; });
												if (pendingIt == pm2Snapshot.pendingTasks.end())
												{
													logWarn("PM2", "PM2交换料未发起: returnArm=0 但未找到B臂pending task，回到等待.");
													pm2_auto_step.store(10);
													Sleep(100);
													break;
												}
												pm2_exchange_in_flight.store(true);
												exchange_info_pm2.getArm.store(0);
												exchange_info_pm2.putArm.store(1);
												pm2_craft_task_id.store(pendingIt->taskId);
												logInform("PM2", "PM2交换料：A取B放，依据固定手臂 task.arm(A取/B放), pendingTaskId=%d.",
													pendingIt->taskId);
												pm2_auto_step.store(1060);
											}
											else if (returnTaskArm == 1 && arm1HasPending && !haswaferarm2)
											{
												const auto pendingIt = std::find_if(pm2Snapshot.pendingTasks.begin(), pm2Snapshot.pendingTasks.end(),
													[](const UnifiedWaferTask& task) { return task.arm == 0; });
												if (pendingIt == pm2Snapshot.pendingTasks.end())
												{
													logWarn("PM2", "PM2交换料未发起: returnArm=1 但未找到A臂pending task，回到等待.");
													pm2_auto_step.store(10);
													Sleep(100);
													break;
												}
												pm2_exchange_in_flight.store(true);
												exchange_info_pm2.getArm.store(1);
												exchange_info_pm2.putArm.store(0);
												pm2_craft_task_id.store(pendingIt->taskId);
												logInform("PM2", "PM2交换料：B取A放，依据固定手臂 task.arm(B取/A放), pendingTaskId=%d.",
													pendingIt->taskId);
												pm2_auto_step.store(1070);
											}
											else
											{
												logWarn("PM2", "PM2交换料前状态与固定手臂task.arm不一致: returnArm=%d, armA_has=%d, armA_pending=%d, armB_has=%d, armB_pending=%d，等待修正...",
													returnTaskArm, haswaferarm1, arm1HasPending, haswaferarm2, arm2HasPending);
												pm2_auto_step.store(10);
												Sleep(100);
											}
										}
										else
										{
											logWarn("PM2", "PM2有片但没有待加工任务，无法判断取放，等待中...");
											pm2_auto_step.store(10);
											Sleep(100);
										}
									}
								}

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
					{//放片 - A臂放到PM2
						logInform("PM2","step:1010 send--->A臂放到PM2.");
						robot_put_to_pm2.arm.store(0);
						robot_put_to_pm2.slot.store(1);
						robot_put_to_pm2.done.store(false);
						robot_put_to_pm2.success.store(false);
						robot_put_to_pm2.requested.store(true);
						pm2_auto_step.store(1015);
					}
					break;
					case 1015:
					{
						// 等待放片完成
						if (robot_put_to_pm2.done.load())
						{
							if (robot_put_to_pm2.success.load())
							{
								logInform("PM2", "step:1015,arm=%d,放到PM2成功-->recive", robot_put_to_pm2.arm);
								const auto pm2Snapshot = buildPmTaskSnapshot("PM2");
								const auto pendingIt = std::find_if(pm2Snapshot.pendingTasks.begin(), pm2Snapshot.pendingTasks.end(),
									[this](const UnifiedWaferTask& task)
									{
										return task.arm == robot_put_to_pm2.arm.load();
									});
								if (pendingIt != pm2Snapshot.pendingTasks.end())
								{
									pm2_craft_task_id.store(pendingIt->taskId);
								}
								else
								{
									pm2_craft_task_id.store(-1);
									logWarn("PM2", "PM2放片成功后未找到与arm=%d对应的pending task，后续工艺回写将退回队首兜底.",
										robot_put_to_pm2.arm.load());
								}
								//pm2_allow_get_put_wafer = false;
								//pm2_allow_loading_wafer = false;
								pm2_allow_goto_craft = true;
								pm2_auto_step.store(2000);
							}
							else
							{
								logWarn("PM2", "放到PM2放片失败，回到step 200重新判定当前手臂/PM状态.");
								pm2_craft_task_id.store(-1);
								Sleep(200);
								pm2_auto_step.store(200);
							}
						} 
						else if (!robot_put_to_pm2.requested.load())
						{
							logWarn("PM2", "PM2 1015步检测到请求已撤销，回到step 200重新调度.");
							pm2_craft_task_id.store(-1);
							Sleep(100);
							pm2_auto_step.store(200);
						}
						else 
						{
							// 添加等待状态提示
							static int wait_count = 0;
							if ((wait_count++ % 20) == 0) { 
								logInform("PM2", "step 1015,等待放PM2晶圆完成...");
							}
							Sleep(500);
						}
					}
					break;
					case 1030:
					{//放片 - B臂放到PM2
						logInform("PM2", "step:1030 send--->B臂放到PM2.");
						robot_put_to_pm2.arm.store(1);
						robot_put_to_pm2.slot.store(1);
						robot_put_to_pm2.done.store(false);
						robot_put_to_pm2.success.store(false);
						robot_put_to_pm2.requested.store(true);
						pm2_auto_step.store(1015); // 复用同一个等待步骤
					}
					break;
					case 1040:
					{
						// 取片 - A臂从PM2取片
						logInform("PM2", "step:1040 send--->A臂从PM2取片.");
						robot_get_from_pm2.arm.store(0);
						robot_get_from_pm2.done.store(false);
						robot_get_from_pm2.success.store(false);
						robot_get_from_pm2.requested.store(true);
						pm2_auto_step.store(1045);
					}
					break;
					case 1045:
					{
						// 等待取片完成
						if (robot_get_from_pm2.done.load())
						{
							if (robot_get_from_pm2.success.load())
							{
								logInform("PM2", "step:1045,arm=%d,从PM2取片成功-->recive", robot_get_from_pm2.arm);
						/*		taskManager.updateTaskStatus(pm2PendingTasks.at(0).taskId, UnifiedWaferTask::TaskType::PM_PROCESS, UnifiedWaferTask::Status::COMPLETED);
								taskManager.updateTaskStatus(pm2PendingTasks.at(0).taskId, UnifiedWaferTask::TaskType::LOADLOCK_RETURN, UnifiedWaferTask::Status::QUEUED);*/

								//pm2_allow_get_put_wafer = false;
								//pm2_allow_loading_wafer = false;

								pm2_auto_step.store(10);
							}
							else
							{
								logWarn(wtr->getName().c_str(), "PM2取片失败，arm=%d，回到step 200重新调度.", robot_get_from_pm2.arm.load());
								Sleep(200);
								pm2_auto_step.store(200);
							}
						}
						else if (!robot_get_from_pm2.requested.load())
						{
							logWarn("PM2", "PM2 1045步检测到请求已撤销，回到step 200重新调度.");
							Sleep(100);
							pm2_auto_step.store(200);
						}
						else
						{
							// 添加等待状态提示
							static int wait_count = 0;
							if ((wait_count++ % 20) == 0) {
								logInform("PM2", "step 1045,等待取PM2晶圆完成...");
							}
							Sleep(500);
						}

					}
					break;
					case 1050:
					{
						// 取片 - B臂从PM2取片
						logInform("PM2", "step:1050 send--->B臂从PM2取片.");
						robot_get_from_pm2.arm.store(1);
						robot_get_from_pm2.done.store(false);
						robot_get_from_pm2.success.store(false);
						robot_get_from_pm2.requested.store(true);
						pm2_auto_step.store(1045); // 复用同一个等待步骤
					}
					break;
#pragma region 交换料
					case 1060:
					{
						logInform("PM2", "step:1060 交换料,send--->对PM2，A取，B放.");
						bool arm1HasWafer = wtr->hasObject(0);  // A臂(索引0)
						bool arm2HasWafer = wtr->hasObject(1);  // B臂(索引1)
						if (arm1HasWafer || !arm2HasWafer) {
							logError(wtr->getName().c_str(), "PM2交换前状态异常(1060)：arm1Has=%d, arm2Has=%d，等待修正...", arm1HasWafer, arm2HasWafer);
							pm2_exchange_in_flight.store(false);
							pm2_auto_step.store(200);  
							return;
						}
						// 交换：A臂取 + B臂放 (先取后放)
						exchange_info_pm2.getArm.store(0);//a
						exchange_info_pm2.putArm.store(1);//b
						logInform("PM2", "step:1060, 置位PM2交换请求 getArm=%d, putArm=%d", exchange_info_pm2.getArm.load(), exchange_info_pm2.putArm.load());
						robot_exchange_pm2.done.store(false);
						robot_exchange_pm2.success.store(false);
						robot_exchange_pm2.requested.store(true);
						pm2_auto_step.store(1065);
					}
					break;
					case 1065:
					{
						// 等待exchange完成
						if (robot_exchange_pm2.done.load())
						{
							pm2_exchange_in_flight.store(false);
							if (robot_exchange_pm2.success.load())
							{
								logInform(wtr->getName().c_str(), "PM2 exchange success (A-get B-put), plan locked before issue, step=%d, goto craft 2000", pm2_auto_step.load());
								//pm2_allow_get_put_wafer = false;
								//pm2_allow_loading_wafer = false;
								pm2_allow_goto_craft = true; //转到工艺
								pm2_auto_step.store(10);
							}
							else
							{
								logWarn("PM2", "PM2交换料失败(A取B放)，回到step 200重新调度.");
								pm2_craft_task_id.store(-1);
								Sleep(200);
								pm2_auto_step.store(200);
							}
						} 
						else if (!robot_exchange_pm2.requested.load())
						{
							pm2_exchange_in_flight.store(false);
							pm2_craft_task_id.store(-1);
							logWarn("PM2", "PM2 1065步检测到交换请求已撤销，回到step 200重新调度.");
							Sleep(100);
							pm2_auto_step.store(200);
						}
						else 
						{	
							// 添加等待状态提示
							static int wait_count = 0;
							if ((wait_count++ % 20) == 0) {
								logInform("PM2", "step 1065,等待PM2交换料成功(A取B放)...");
							}
							Sleep(500);
						}
					}
					break;
					case 1070:
					{
						logInform("PM2", "step:1070 交换料,send--->对PM2，B取，A放.");
						bool arm1HasWafer = wtr->hasObject(0);  // A臂(索引0)
						bool arm2HasWafer = wtr->hasObject(1);  // B臂(索引1)
						if (!arm1HasWafer || arm2HasWafer) 
						{
							logWarn(wtr->getName().c_str(), "PM2交换前状态异常(1070)：arm1Has=%d, arm2Has=%d，等待修正...", arm1HasWafer, arm2HasWafer);
							pm2_exchange_in_flight.store(false);
							pm2_auto_step.store(200);
							return;
						}
						// 交换：B臂取 + A臂放 (先取后放)
						exchange_info_pm2.getArm.store(1);
						exchange_info_pm2.putArm.store(0);
						logInform("PM2", "step:1070, 置位PM2交换请求 getArm=%d, putArm=%d", exchange_info_pm2.getArm.load(), exchange_info_pm2.putArm.load());
						robot_exchange_pm2.done.store(false);
						robot_exchange_pm2.success.store(false);
						robot_exchange_pm2.requested.store(true);
						pm2_auto_step.store(1075);
					}
					break;
					case 1075:
					{
						// 等待exchange完成
						if (robot_exchange_pm2.done.load())
						{
							pm2_exchange_in_flight.store(false);
							if (robot_exchange_pm2.success.load())
							{
								logInform(wtr->getName().c_str(), "PM2 exchange success (B-get A-put), plan locked before issue, step=%d, goto craft 2000", pm2_auto_step.load());
								//pm2_allow_get_put_wafer = false;
								//pm2_allow_loading_wafer = false;
								pm2_allow_goto_craft = true;
								pm2_auto_step.store(10);
							}
							else
							{
								logWarn("PM2", "PM2交换料失败(B取A放)，回到step 200重新调度.");
								pm2_craft_task_id.store(-1);
								Sleep(200);
								pm2_auto_step.store(200);
							}
						}
						else if (!robot_exchange_pm2.requested.load())
						{
							pm2_exchange_in_flight.store(false);
							pm2_craft_task_id.store(-1);
							logWarn("PM2", "PM2 1075步检测到交换请求已撤销，回到step 200重新调度.");
							Sleep(100);
							pm2_auto_step.store(200);
						}
						else
						{
							// 添加等待状态提示
							static int wait_count = 0;
							if ((wait_count++ % 20) == 0) {
								logInform("PM2", "step 1075,等待PM2交换料成功(B取A放)...");
							}
							Sleep(500);
						}
					}
					break;

#pragma endregion
					case 1090:
					{
						// 最终取片（仅取，不放）：A臂从PM2取片
						logInform("PM2", "step:1090 send--->A臂从PM2取片.");
						robot_get_from_pm2.arm.store(0);
						robot_get_from_pm2.done.store(false);
						robot_get_from_pm2.success.store(false);
						robot_get_from_pm2.requested.store(true);
						pm2_auto_step.store(1095);
					}
					break;
					case 1095:
					{
						// 等待取片完成
						if (robot_get_from_pm2.done.load())
						{
							if (robot_get_from_pm2.success.load())
							{
								logInform("PM2", "step:1095,arm=%d,从PM2取片成功-->recive", robot_get_from_pm2.arm);
								//pm2_allow_loading_wafer = false;
								//pm2_allow_get_put_wafer = false;
								pm2_auto_step.store(10);
							}
							else
							{
								logWarn("PM2", "PM2最终取片失败，回到step 200重新调度.");
								Sleep(200);
								pm2_auto_step.store(200);
							}
						} 
						else if (!robot_get_from_pm2.requested.load())
						{
							logWarn("PM2", "PM2 1095步检测到请求已撤销，回到step 200重新调度.");
							Sleep(100);
							pm2_auto_step.store(200);
						}
						else
						{
							// 添加等待状态提示
							static int wait_count = 0;
							if ((wait_count++ % 20) == 0) {
								logInform("PM2", "step 1095,等待取PM2晶圆完成...");
							}
							Sleep(500);
						}
					}
					break;

					//case 300:
					//{//PM2工艺完成，等待LL取回晶圆（非交换场景）

					//	logInform("PM2", "step:300 send--->PM2工艺完成，等待LL取回晶圆,非交换场景.");

					//	auto cassManager = wtr->getKernel()->getKernelModule<FortrendCassetteManager>();
					//	bool haswaferpm = cassManager->getCassette(pm2.get())->getMapping(1) == Cassette::Present;
					//	if (!haswaferpm)
					//	{
					//		logInform("PM2", "LL已取回晶圆，PM2回程流程结束.");
					//		pm2_need_return_wafer = false;

					//		// 检查是否还有待处理的晶圆（如另一片LL在WTR上等待）
					//		UpdatePmSubTransferDatas("PM2");
					//		if (pm2PendingTasks.size() > 0 || pm2CompletedTasks.size() > 0)
					//		{
					//			pm2_allow_get_put_wafer = true;
					//			logInform("PM2", "PM2回程结束后检测到待处理任务，重新允许取放片");
					//		}

					//		pm2_auto_step.store(10);
					//	}
					//	else
					//	{
					//		Sleep(200);
					//	}

					//}
					//break;

					case 2000:
					{
						UpdatePmSubTransferDatas("PM2");
						if (pm2 != nullptr)
						{
							if (pm2->getWithWaferModeEnable())
							{
								if (simulate_pm2_enabled.load())
								{
									logInform("PM2", "PM2模拟工艺已启用，仅执行取放片并模拟工艺耗时20s.");
									for (int elapsedMs = 0; elapsedMs < 20000 && !stopRequested.load(); elapsedMs += 100)
									{
										Sleep(100);
									}
									if (stopRequested.load())
									{
										break;
									}
								}
								else if (!runPmRecipeMotorOnUiThread(1, pm2->getName().c_str(), pm2_process_name, pm2_auto_step))
								{
									break;
								}
							}
							else
							{
								logFailed(pm2->getName(), Poco::format("%s 没有片，无法执行工艺流程， %s：%d", pm2->getName(), pm2_process_name, pm2_auto_step.load()));
							}
						}
						else
						{
							logFailed(wtr->getName(), Poco::format("PM2模块指针为空， %s：%d", pm2_process_name, pm2_auto_step.load()));
						}

						const auto pm2Snapshot = buildPmTaskSnapshot("PM2");
						if (!pm2Snapshot.pendingTasks.empty())
						{
							UnifiedWaferTask currentTask = pm2Snapshot.pendingTasks.front();
							const int craftTaskId = pm2_craft_task_id.load();
							if (craftTaskId >= 0)
							{
								const auto taskIt = std::find_if(pm2Snapshot.pendingTasks.begin(), pm2Snapshot.pendingTasks.end(),
									[craftTaskId](const UnifiedWaferTask& task)
									{
										return task.taskId == craftTaskId;
									});
								if (taskIt == pm2Snapshot.pendingTasks.end())
								{
									logError("PM2", "PM2工艺完成后未找到锁定的pending task: taskId=%d，拒绝错误回写任务状态.", craftTaskId);
									pm2_allow_goto_craft = false;
									pm2_craft_task_id.store(-1);
									pm2_auto_step.store(10);
									break;
								}
								currentTask = *taskIt;
							}
							taskManager.updateTaskStatus(currentTask.taskId, UnifiedWaferTask::TaskType::PM_PROCESS, UnifiedWaferTask::Status::COMPLETED);
							taskManager.updateTaskStatus(currentTask.taskId, UnifiedWaferTask::TaskType::LOADLOCK_RETURN, UnifiedWaferTask::Status::QUEUED);//7
							//pm2_need_return_wafer = true;
							pm2_allow_goto_craft = false;
							pm2_craft_task_id.store(-1);
							pm2_auto_step.store(10);
						}
						else
						{
							pm2_craft_task_id.store(-1);
							pm2_auto_step.store(10);
						}
					}
					break;
					case 2001:
					{
						const auto pm2Snapshot = buildPmTaskSnapshot("PM2");
						if(!pm2Snapshot.completedTasks.empty())
						{
							taskManager.updateTaskStatus(pm2Snapshot.completedTasks.at(0).taskId, UnifiedWaferTask::TaskType::LOADLOCK_RETURN, UnifiedWaferTask::Status::QUEUED);//7
							//pm2_allow_get_put_wafer = false;
							//pm2_allow_loading_wafer = false;

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
			pauseAllThreads();
			saveCurrentStateSnapshot(e.what(), Poco::format("PM2Transfer step:%d", pm2_auto_step.load()));
			onUpdateControlEnabled("execute_pbt", true);
			onUpdateLightButtonStatus("light_running_pbt", 3);

			logError("Cyclelog", "PM2Transfer thread crashed:", e.what());
			qCritical() << "PM2ransfer thread crashed:" << e.what();
		}
		catch (...) {
			pauseAllThreads();
			saveCurrentStateSnapshot("unknown exception", Poco::format("PM2Transfer step:%d", pm2_auto_step.load()));
			onUpdateControlEnabled("execute_pbt", true);
			onUpdateLightButtonStatus("light_running_pbt", 3);

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
							const auto llaSnapshot = buildLoadLockTaskSnapshot("LLA");
							if (!llaSnapshot.completedTasks.empty())
							{
								logInform("PM3", "更新LLA:LOADLOCK_TRANSFER/COMPLETED ------>PM_PROCESS/QUEUED.");
								for (const auto& task : llaSnapshot.completedTasks)
								{
									taskManager.updateTaskStatus(task.taskId, UnifiedWaferTask::TaskType::PM_PROCESS, UnifiedWaferTask::Status::QUEUED);
								}
							}


							const auto llbSnapshot = buildLoadLockTaskSnapshot("LLB");
							if (!llbSnapshot.completedTasks.empty())
							{
								logInform("PM3", "更新LLB:LOADLOCK_TRANSFER/COMPLETED ------>PM_PROCESS/QUEUED.");
								for (const auto& task : llbSnapshot.completedTasks)
								{
									taskManager.updateTaskStatus(task.taskId, UnifiedWaferTask::TaskType::PM_PROCESS, UnifiedWaferTask::Status::QUEUED);
								}
							}


							pm3_auto_step.store(100);
							//pm3_auto_step = 100;
						}
						else if (pm3_allow_goto_craft)
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
						const auto pm3Snapshot = buildPmTaskSnapshot("PM3");
						//判断是放、取,不考虑交互手
						if (!pm3Snapshot.pendingTasks.empty() || !pm3Snapshot.completedTasks.empty())
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
						std::string waitReason;
						const bool shouldWaitRobotWtr = shouldWaitBeforeStartWtrFingerQuery(wtr, waitReason);
						if (!shouldWaitRobotWtr)
						{
							auto cmd1 = wtr->createRQLoadCommand(0); //1手
							wtr->startCommand(cmd1);
							cmd1->wait();

							std::string waitReasonAfterArmA;
							const bool shouldWaitRobotWtrAfterArmA = shouldWaitBeforeStartWtrFingerQuery(wtr, waitReasonAfterArmA);
							if (shouldWaitRobotWtrAfterArmA)
							{
								static int wait_count_after_arm_a = 0;
								if ((wait_count_after_arm_a++ % 20) == 0)
								{
									logWarn(wtr->getName().c_str(), "PM3 step 199 wait WTR finger query after arm A. %s", waitReasonAfterArmA.c_str());
								}
								Sleep(200);
								break;
							}

							auto cmd2 = wtr->createRQLoadCommand(1); //1手
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
							static int wait_count = 0;
							if ((wait_count++ % 20) == 0)
							{
								if (!waitReason.empty())
								{
									logWarn(wtr->getName().c_str(), "PM3 step 199 wait WTR finger query. %s", waitReason.c_str());
								}
								else if (wtr->isBusy())
								{
									logWarn(wtr->getName().c_str(), "PM3 step 199 wait WTR finger query. WTR busy.");
								}
								else
								{
									logWarn(wtr->getName().c_str(), "PM3 step 199 wait WTR finger query. WTR state not normal.");
								}
							}
							Sleep(200);
						}

					}
					case 200:
					{
						//暂不考虑交互手
						if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
							// 使用hasObject(arm)检查手臂状态，更符合硬件安全检查机制
							bool haswaferpm = cassManager->getCassette(pm3.get())->getMapping(1) == Cassette::Present;   //pm中有片
							bool haswaferarm1 = wtr->hasObject(0); //arm1有片  A臂(索引0)
							bool haswaferarm2 = wtr->hasObject(1); //arm2有片  B臂(索引1)						
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
					{//放片 - A臂放到PM3
						robot_put_to_pm3.arm.store(0);
						robot_put_to_pm3.slot.store(1);
						robot_put_to_pm3.done.store(false);
						robot_put_to_pm3.success.store(false);
						robot_put_to_pm3.requested.store(true);
						pm3_auto_step.store(1015);
					}
					break;
					case 1015:
					{
						// 等待放片完成
						if (robot_put_to_pm3.done.load())
						{
							if (robot_put_to_pm3.success.load())
							{
								pm3_allow_get_put_wafer = false;
								pm3_auto_step.store(2000);
							}
							else
							{
								logFailed(wtr->getName(), Poco::format("PM3放片失败(A臂)， %s：%d", pm3_process_name, pm3_auto_step.load()));
							}
						}
					}
					break;
					case 1030:
					{//放片 - B臂放到PM3
						robot_put_to_pm3.arm.store(1);
						robot_put_to_pm3.slot.store(1);
						robot_put_to_pm3.done.store(false);
						robot_put_to_pm3.success.store(false);
						robot_put_to_pm3.requested.store(true);
						pm3_auto_step.store(1015); // 复用同一个等待步骤
					}
					break;
					case 1040:
					{
						// 取片 - A臂从PM3取片
						robot_get_from_pm3.arm.store(0);
						robot_get_from_pm3.done.store(false);
						robot_get_from_pm3.success.store(false);
						robot_get_from_pm3.requested.store(true);
						pm3_auto_step.store(1045);
					}
					break;
					case 1045:
					{
						// 等待取片完成
						if (robot_get_from_pm3.done.load())
						{
							if (robot_get_from_pm3.success.load())
							{
								const auto pm3Snapshot = buildPmTaskSnapshot("PM3");
								if (pm3Snapshot.pendingTasks.empty())
								{
									logWarn(wtr->getName().c_str(), "PM3取片完成但待加工任务为空，回退检查 step=%d", pm3_auto_step.load());
									pm3_auto_step.store(10);
									break;
								}
								const auto currentTask = pm3Snapshot.pendingTasks.front();
								taskManager.updateTaskStatus(currentTask.taskId, UnifiedWaferTask::TaskType::PM_PROCESS, UnifiedWaferTask::Status::COMPLETED);
								taskManager.updateTaskStatus(currentTask.taskId, UnifiedWaferTask::TaskType::LOADLOCK_RETURN, UnifiedWaferTask::Status::QUEUED);
								pm3_allow_get_put_wafer = false;
								pm3_auto_step.store(10);
							}
							else
							{
								logFailed(wtr->getName(), Poco::format("PM3取片失败(A臂)， %s：%d", pm3_process_name, pm3_auto_step.load()));
							}
						}
					}
					break;
					case 1050:
					{
						// 取片 - B臂从PM3取片
						robot_get_from_pm3.arm.store(1);
						robot_get_from_pm3.done.store(false);
						robot_get_from_pm3.success.store(false);
						robot_get_from_pm3.requested.store(true);
						pm3_auto_step.store(1045); // 复用同一个等待步骤
					}
					break;
#pragma region 交换料
					case 1060:
					{
						// 交换：A臂取 + B臂放 (先取后放)
						exchange_info_pm3.getArm.store(0);
						exchange_info_pm3.putArm.store(1);
						robot_exchange_pm3.done.store(false);
						robot_exchange_pm3.success.store(false);
						robot_exchange_pm3.requested.store(true);
						pm3_auto_step.store(1065);
					}
					break;
					case 1065:
					{
						// 等待exchange完成
						if (robot_exchange_pm3.done.load())
						{
							if (robot_exchange_pm3.success.load())
							{
								const auto pm3Snapshot = buildPmTaskSnapshot("PM3");
								// 防御性检查：防止vector为空导致at(0)崩溃
								if (pm3Snapshot.completedTasks.empty()) {
									logWarn(wtr->getName().c_str(), "PM3交换完成但无已完成任务，回退检查 step=%d", pm3_auto_step.load());
									pm3_auto_step.store(1060);
									return;
								}
								taskManager.updateTaskStatus(pm3Snapshot.completedTasks.front().taskId, UnifiedWaferTask::TaskType::LOADLOCK_RETURN, UnifiedWaferTask::Status::QUEUED);
								pm3_allow_get_put_wafer = false;
								pm3_allow_goto_craft = true;
								pm3_auto_step.store(10);
							}
							else
							{
								logFailed(wtr->getName(), Poco::format("PM3交换料失败(A取B放)， %s：%d", pm3_process_name, pm3_auto_step.load()));
							}
						}
					}
					break;
					case 1070:
					{
						// 交换：B臂取 + A臂放 (先取后放)
						exchange_info_pm3.getArm.store(1);
						exchange_info_pm3.putArm.store(0);
						robot_exchange_pm3.done.store(false);
						robot_exchange_pm3.success.store(false);
						robot_exchange_pm3.requested.store(true);
						pm3_auto_step.store(1075);
					}
					break;
					case 1075:
					{
						// 等待exchange完成
						if (robot_exchange_pm3.done.load())
						{
							if (robot_exchange_pm3.success.load())
							{
								const auto pm3Snapshot = buildPmTaskSnapshot("PM3");
								// 防御性检查：防止vector为空导致at(0)崩溃
								if (pm3Snapshot.completedTasks.empty()) {
									logWarn(wtr->getName().c_str(), "PM3交换完成但无已完成任务，回退检查 step=%d", pm3_auto_step.load());
									pm3_auto_step.store(1070);
									return;
								}
								taskManager.updateTaskStatus(pm3Snapshot.completedTasks.front().taskId, UnifiedWaferTask::TaskType::LOADLOCK_RETURN, UnifiedWaferTask::Status::QUEUED);
								pm3_allow_get_put_wafer = false;
								pm3_allow_goto_craft = true;
								pm3_auto_step.store(10);
							}
							else
							{
								logFailed(wtr->getName(), Poco::format("PM3交换料失败(B取A放)， %s：%d", pm3_process_name, pm3_auto_step.load()));
							}
						}
					}
					break;
#pragma endregion
					case 1090:
					{
						// 最终取片（仅取，不放）：A臂从PM3取片
						robot_get_from_pm3.arm.store(0);
						robot_get_from_pm3.done.store(false);
						robot_get_from_pm3.success.store(false);
						robot_get_from_pm3.requested.store(true);
						pm3_auto_step.store(1095);
					}
					break;
					case 1095:
					{
						// 等待取片完成
						if (robot_get_from_pm3.done.load())
						{
							if (robot_get_from_pm3.success.load())
							{
								pm3_allow_get_put_wafer = false;
								const auto pm3Snapshot = buildPmTaskSnapshot("PM3");
								// 防御性检查：防止vector为空导致at(0)崩溃
								if (pm3Snapshot.completedTasks.empty()) {
									logWarn(wtr->getName().c_str(), "PM3最终取片完成但无已完成任务，回退检查 step=%d", pm3_auto_step.load());
									pm3_auto_step.store(1090);
									return;
								}
								taskManager.updateTaskStatus(pm3Snapshot.completedTasks.front().taskId, UnifiedWaferTask::TaskType::LOADLOCK_RETURN, UnifiedWaferTask::Status::QUEUED);
								pm3_auto_step.store(10);
							}
							else
							{
								logInform(wtr->getName().c_str(), "PM3最终取片失败，2秒后重试 step=%d", pm3_auto_step.load());
								Sleep(2000);
								pm3_auto_step.store(1090);
							}
						} else if (!robot_get_from_pm3.requested.load()) {
							logInform(wtr->getName().c_str(), "PM3 1095步：请求标志异常(done=false,requested=false)，回退重发 step=%d", pm3_auto_step.load());
							Sleep(1000);
							pm3_auto_step.store(1090);
						}
					}
					break;
					case 2000:
					{
						Sleep(500);
						UpdatePmSubTransferDatas("PM3");

#ifdef DEBUG_TEST_PM
						if (!runPmRecipeMotorOnUiThread(2, "PM3", pm3_process_name, pm3_auto_step))
						{
							break;
						}

#else
						logInform("PM3", "模拟做工艺流程.....");
#endif
						const auto pm3Snapshot = buildPmTaskSnapshot("PM3");
						if (!pm3Snapshot.pendingTasks.empty())
						{
							const auto currentTask = pm3Snapshot.pendingTasks.front();
							taskManager.updateTaskStatus(currentTask.taskId, UnifiedWaferTask::TaskType::PM_PROCESS, UnifiedWaferTask::Status::COMPLETED);
							taskManager.updateTaskStatus(currentTask.taskId, UnifiedWaferTask::TaskType::LOADLOCK_RETURN, UnifiedWaferTask::Status::QUEUED);//7
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
						const auto pm3Snapshot = buildPmTaskSnapshot("PM3");
						if (!pm3Snapshot.completedTasks.empty())
						{
							taskManager.updateTaskStatus(pm3Snapshot.completedTasks.front().taskId, UnifiedWaferTask::TaskType::LOADLOCK_RETURN, UnifiedWaferTask::Status::QUEUED);//7
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
			pauseAllThreads();
			saveCurrentStateSnapshot(e.what(), Poco::format("PM3Transfer step:%d", pm3_auto_step.load()));
			onUpdateControlEnabled("execute_pbt", true);
			onUpdateLightButtonStatus("light_running_pbt", 3);
			logError("Cyclelog", "PM3Transfer thread crashed:", e.what());
			qCritical() << "PM3Transfer thread crashed:" << e.what();
		}
		catch (...) {
			pauseAllThreads();
			saveCurrentStateSnapshot("unknown exception", Poco::format("PM3Transfer step:%d", pm3_auto_step.load()));
			onUpdateControlEnabled("execute_pbt", true);
			onUpdateLightButtonStatus("light_running_pbt", 3);
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

							const auto llaSnapshot = buildLoadLockTaskSnapshot("LLA");
							if (!llaSnapshot.completedTasks.empty())
							{
								logInform("PM4", "更新LLA:LOADLOCK_TRANSFER/COMPLETED ------>PM_PROCESS/QUEUED.");
								for (const auto& task : llaSnapshot.completedTasks)
								{
									taskManager.updateTaskStatus(task.taskId, UnifiedWaferTask::TaskType::PM_PROCESS, UnifiedWaferTask::Status::QUEUED);
								}
							}

							const auto llbSnapshot = buildLoadLockTaskSnapshot("LLB");
							if (!llbSnapshot.completedTasks.empty())
							{
								logInform("PM4", "更新LLB:LOADLOCK_TRANSFER/COMPLETED ------>PM_PROCESS/QUEUED.");
								for (const auto& task : llbSnapshot.completedTasks)
								{
									taskManager.updateTaskStatus(task.taskId, UnifiedWaferTask::TaskType::PM_PROCESS, UnifiedWaferTask::Status::QUEUED);
								}
							}

							pm4_auto_step.store(100); //改成100,不然下不去
						}
						else if (pm4_allow_goto_craft)
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
						const auto pm4Snapshot = buildPmTaskSnapshot("PM4");
						//判断是放、取,不考虑交互手
						if (!pm4Snapshot.pendingTasks.empty() || !pm4Snapshot.completedTasks.empty())
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
						std::string waitReason;
						const bool shouldWaitRobotWtr = shouldWaitBeforeStartWtrFingerQuery(wtr, waitReason);
						if (!shouldWaitRobotWtr)
						{
							auto cmd1 = wtr->createRQLoadCommand(0); //1手
							wtr->startCommand(cmd1);
							cmd1->wait();

							std::string waitReasonAfterArmA;
							const bool shouldWaitRobotWtrAfterArmA = shouldWaitBeforeStartWtrFingerQuery(wtr, waitReasonAfterArmA);
							if (shouldWaitRobotWtrAfterArmA)
							{
								static int wait_count_after_arm_a = 0;
								if ((wait_count_after_arm_a++ % 20) == 0)
								{
									logWarn(wtr->getName().c_str(), "PM4 step 199 wait WTR finger query after arm A. %s", waitReasonAfterArmA.c_str());
								}
								Sleep(200);
								break;
							}


							auto cmd2 = wtr->createRQLoadCommand(1); //1手
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
							static int wait_count = 0;
							if ((wait_count++ % 20) == 0)
							{
								if (!waitReason.empty())
								{
									logWarn(wtr->getName().c_str(), "PM4 step 199 wait WTR finger query. %s", waitReason.c_str());
								}
								else if (wtr->isBusy())
								{
									logWarn(wtr->getName().c_str(), "PM4 step 199 wait WTR finger query. WTR busy.");
								}
								else
								{
									logWarn(wtr->getName().c_str(), "PM4 step 199 wait WTR finger query. WTR state not normal.");
								}
							}
							Sleep(200);
						}

					}
					break;
					case 200:
					{
						//暂不考虑交互手
						if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
							// 使用hasObject(arm)检查手臂状态，更符合硬件安全检查机制
							bool haswaferpm = cassManager->getCassette(pm4.get())->getMapping(1) == Cassette::Present;   //pm中有片
							bool haswaferarm1 = wtr->hasObject(0); //arm1有片  A臂(索引0)
							bool haswaferarm2 = wtr->hasObject(1); //arm2有片  B臂(索引1)

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
					{//放片 - A臂放到PM4
						robot_put_to_pm4.arm.store(0);
						robot_put_to_pm4.slot.store(1);
						robot_put_to_pm4.done.store(false);
						robot_put_to_pm4.success.store(false);
						robot_put_to_pm4.requested.store(true);
						pm4_auto_step.store(1015);
					}
					break;
					case 1015:
					{
						// 等待放片完成
						if (robot_put_to_pm4.done.load())
						{
							if (robot_put_to_pm4.success.load())
							{
								pm4_allow_get_put_wafer = false;
								pm4_auto_step.store(2000);
							}
							else
							{
								logInform(wtr->getName().c_str(), "PM4放片失败，2秒后重试 step=%d", pm4_auto_step.load());
								Sleep(2000);
								// 根据arm判断回退到哪个请求步骤
								if (robot_put_to_pm4.arm.load() == 0)
									pm4_auto_step.store(1010);
								else
									pm4_auto_step.store(1030);
							}
						} else if (!robot_put_to_pm4.requested.load()) {
							logInform(wtr->getName().c_str(), "PM4 1015步：请求标志异常(done=false,requested=false)，回退重发 step=%d", pm4_auto_step.load());
							Sleep(1000);
							if (robot_put_to_pm4.arm.load() == 0)
								pm4_auto_step.store(1010);
							else
								pm4_auto_step.store(1030);
						}
					}
					break;
					case 1030:
					{//放片 - B臂放到PM4
						robot_put_to_pm4.arm.store(1);
						robot_put_to_pm4.slot.store(1);
						robot_put_to_pm4.done.store(false);
						robot_put_to_pm4.success.store(false);
						robot_put_to_pm4.requested.store(true);
						pm4_auto_step.store(1015); // 复用同一个等待步骤
					}
					break;
					case 1040:
					{
						// 取片 - A臂从PM4取片
						robot_get_from_pm4.arm.store(0);
						robot_get_from_pm4.done.store(false);
						robot_get_from_pm4.success.store(false);
						robot_get_from_pm4.requested.store(true);
						pm4_auto_step.store(1045);
					}
					break;
					case 1045:
					{
						// 等待取片完成
						if (robot_get_from_pm4.done.load())
						{
							if (robot_get_from_pm4.success.load())
							{
								const auto pm4Snapshot = buildPmTaskSnapshot("PM4");
								if (pm4Snapshot.pendingTasks.empty())
								{
									logWarn(wtr->getName().c_str(), "PM4取片完成但待加工任务为空，回退检查 step=%d", pm4_auto_step.load());
									pm4_auto_step.store(10);
									break;
								}
								const auto currentTask = pm4Snapshot.pendingTasks.front();
								taskManager.updateTaskStatus(currentTask.taskId, UnifiedWaferTask::TaskType::PM_PROCESS, UnifiedWaferTask::Status::COMPLETED);
								taskManager.updateTaskStatus(currentTask.taskId, UnifiedWaferTask::TaskType::LOADLOCK_RETURN, UnifiedWaferTask::Status::QUEUED);
								pm4_allow_get_put_wafer = false;
								pm4_auto_step.store(10);
							}
							else
							{
								logInform(wtr->getName().c_str(), "PM4取片失败，2秒后重试 step=%d", pm4_auto_step.load());
								Sleep(2000);
								if (robot_get_from_pm4.arm.load() == 0)
									pm4_auto_step.store(1040);
								else
									pm4_auto_step.store(1050);
							}
						} else if (!robot_get_from_pm4.requested.load()) {
							logInform(wtr->getName().c_str(), "PM4 1045步：请求标志异常(done=false,requested=false)，回退重发 step=%d", pm4_auto_step.load());
							Sleep(1000);
							if (robot_get_from_pm4.arm.load() == 0)
								pm4_auto_step.store(1040);
								else
									pm4_auto_step.store(1050);
						}
					}
					break;
					case 1050:
					{
						// 取片 - B臂从PM4取片
						robot_get_from_pm4.arm.store(1);
						robot_get_from_pm4.done.store(false);
						robot_get_from_pm4.success.store(false);
						robot_get_from_pm4.requested.store(true);
						pm4_auto_step.store(1045); // 复用同一个等待步骤
					}
					break;
#pragma region 交换料
					case 1060:
					{
						// 交换：A臂取 + B臂放 (先取后放)
						exchange_info_pm4.getArm.store(0);
						exchange_info_pm4.putArm.store(1);
						robot_exchange_pm4.done.store(false);
						robot_exchange_pm4.success.store(false);
						robot_exchange_pm4.requested.store(true);
						pm4_auto_step.store(1065);
					}
					break;
					case 1065:
					{
						// 等待exchange完成
						if (robot_exchange_pm4.done.load())
						{
							if (robot_exchange_pm4.success.load())
							{
								const auto pm4Snapshot = buildPmTaskSnapshot("PM4");
								// 防御性检查：防止vector为空导致at(0)崩溃
								if (pm4Snapshot.completedTasks.empty()) {
									logWarn(wtr->getName().c_str(), "PM4交换完成但无已完成任务，回退检查 step=%d", pm4_auto_step.load());
									pm4_auto_step.store(1060);
									return;
								}
								taskManager.updateTaskStatus(pm4Snapshot.completedTasks.front().taskId, UnifiedWaferTask::TaskType::LOADLOCK_RETURN, UnifiedWaferTask::Status::QUEUED);
								pm4_allow_get_put_wafer = false;
								pm4_allow_goto_craft = true;
								pm4_auto_step.store(10);
							}
							else
							{
								logInform(wtr->getName().c_str(), "PM4交换料失败(A取B放)，2秒后重试 step=%d", pm4_auto_step.load());
								Sleep(2000);
								pm4_auto_step.store(1060);
							}
						} else if (!robot_exchange_pm4.requested.load()) {
							logInform(wtr->getName().c_str(), "PM4 1065步：请求标志异常(done=false,requested=false)，回退重发 step=%d", pm4_auto_step.load());
							Sleep(1000);
							pm4_auto_step.store(1060);
						}
					}
					break;
					case 1070:
					{
						// 交换：B臂取 + A臂放 (先取后放)
						exchange_info_pm4.getArm.store(1);
						exchange_info_pm4.putArm.store(0);
						robot_exchange_pm4.done.store(false);
						robot_exchange_pm4.success.store(false);
						robot_exchange_pm4.requested.store(true);
						pm4_auto_step.store(1075);
					}
					break;
					case 1075:
					{
						// 等待exchange完成
						if (robot_exchange_pm4.done.load())
						{
							if (robot_exchange_pm4.success.load())
							{
								const auto pm4Snapshot = buildPmTaskSnapshot("PM4");
								// 防御性检查：防止vector为空导致at(0)崩溃
								if (pm4Snapshot.completedTasks.empty()) {
									logWarn(wtr->getName().c_str(), "PM4交换完成但无已完成任务，回退检查 step=%d", pm4_auto_step.load());
									pm4_auto_step.store(1070);
									return;
								}
								taskManager.updateTaskStatus(pm4Snapshot.completedTasks.front().taskId, UnifiedWaferTask::TaskType::LOADLOCK_RETURN, UnifiedWaferTask::Status::QUEUED);
								pm4_allow_get_put_wafer = false;
								pm4_allow_goto_craft = true;
								pm4_auto_step.store(10);
							}
							else
							{
								logInform(wtr->getName().c_str(), "PM4交换料失败(B取A放)，2秒后重试 step=%d", pm4_auto_step.load());
								Sleep(2000);
								pm4_auto_step.store(1070);
							}
						} else if (!robot_exchange_pm4.requested.load()) {
							logInform(wtr->getName().c_str(), "PM4 1075步：请求标志异常(done=false,requested=false)，回退重发 step=%d", pm4_auto_step.load());
							Sleep(1000);
							pm4_auto_step.store(1070);
						}
					}
					break;
#pragma endregion
					case 1090:
					{
						// 最终取片（仅取，不放）：A臂从PM4取片
						robot_get_from_pm4.arm.store(0);
						robot_get_from_pm4.done.store(false);
						robot_get_from_pm4.success.store(false);
						robot_get_from_pm4.requested.store(true);
						pm4_auto_step.store(1095);
					}
					break;
					case 1095:
					{
						// 等待取片完成
						if (robot_get_from_pm4.done.load())
						{
							if (robot_get_from_pm4.success.load())
							{
								pm4_allow_get_put_wafer = false;
								const auto pm4Snapshot = buildPmTaskSnapshot("PM4");
								// 防御性检查：防止vector为空导致at(0)崩溃
								if (pm4Snapshot.completedTasks.empty()) {
									logWarn(wtr->getName().c_str(), "PM4最终取片完成但无已完成任务，回退检查 step=%d", pm4_auto_step.load());
									pm4_auto_step.store(1090);
									return;
								}
								taskManager.updateTaskStatus(pm4Snapshot.completedTasks.front().taskId, UnifiedWaferTask::TaskType::LOADLOCK_RETURN, UnifiedWaferTask::Status::QUEUED);
								pm4_auto_step.store(10);
							}
							else
							{
								logInform(wtr->getName().c_str(), "PM4最终取片失败，2秒后重试 step=%d", pm4_auto_step.load());
								Sleep(2000);
								pm4_auto_step.store(1090);
							}
						} else if (!robot_get_from_pm4.requested.load()) {
							logInform(wtr->getName().c_str(), "PM4 1095步：请求标志异常(done=false,requested=false)，回退重发 step=%d", pm4_auto_step.load());
							Sleep(1000);
							pm4_auto_step.store(1090);
						}
					}
					break;

					case 2000:
					{
						Sleep(500);
						UpdatePmSubTransferDatas("PM4");
						

#ifdef DEBUG_TEST_PM
						if (!runPmRecipeMotorOnUiThread(3, "PM4", pm4_process_name, pm4_auto_step))
						{
							break;
						}

#else
						logInform("PM4", "模拟做工艺流程.....");
#endif

						const auto pm4Snapshot = buildPmTaskSnapshot("PM4");
						if (!pm4Snapshot.pendingTasks.empty())
						{
							const auto currentTask = pm4Snapshot.pendingTasks.front();
							taskManager.updateTaskStatus(currentTask.taskId, UnifiedWaferTask::TaskType::PM_PROCESS, UnifiedWaferTask::Status::COMPLETED);
							taskManager.updateTaskStatus(currentTask.taskId, UnifiedWaferTask::TaskType::LOADLOCK_RETURN, UnifiedWaferTask::Status::QUEUED);//7
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
						const auto pm4Snapshot = buildPmTaskSnapshot("PM4");
						if (!pm4Snapshot.completedTasks.empty())
						{
							taskManager.updateTaskStatus(pm4Snapshot.completedTasks.front().taskId, UnifiedWaferTask::TaskType::LOADLOCK_RETURN, UnifiedWaferTask::Status::QUEUED);//7
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
			pauseAllThreads();
			saveCurrentStateSnapshot(e.what(), Poco::format("PM4Transfer step:%d", pm4_auto_step.load()));
			onUpdateControlEnabled("execute_pbt", true);
			onUpdateLightButtonStatus("light_running_pbt", 3);

			logError("Cyclelog", "PM4Transfer thread crashed:", e.what());
			qCritical() << "PM4Transfer thread crashed:" << e.what();
		}
		catch (...) {
			pauseAllThreads();
			saveCurrentStateSnapshot("unknown exception", Poco::format("PM4Transfer step:%d", pm4_auto_step.load()));
			onUpdateControlEnabled("execute_pbt", true);
			onUpdateLightButtonStatus("light_running_pbt", 3);
			logError("Cyclelog", "PM4Transfer thread crashed: unknown exception");
			qCritical() << "PM4Transfer thread crashed: unknown exception";
		}



	}

	void QSlotTransferCycleVTMWidgetPrivate::executeTMTransfer()
	{
		try {
			auto cassManager = wtr->getKernel()->getKernelModule<FortrendCassetteManager>();
			std::shared_ptr<FortrendPMCavitySubsystem> pm1 = kernel->getKernelModule<FortrendPMCavitySubsystem>("PM1");
			std::shared_ptr<FortrendPMCavitySubsystem> pm2 = kernel->getKernelModule<FortrendPMCavitySubsystem>("PM2");
			std::shared_ptr<FortrendPMCavitySubsystem> pm3 = kernel->getKernelModule<FortrendPMCavitySubsystem>("PM3");
			std::shared_ptr<FortrendPMCavitySubsystem> pm4 = kernel->getKernelModule<FortrendPMCavitySubsystem>("PM4");
			auto canExecuteGet = [this](const char* stationName, int step, int arm) -> bool
			{
				const bool armHasWafer = wtr->hasObject(arm);
				logInform(wtr->getName().c_str(), "Robot线程：step %d,GET前检查 station=%s arm=%d hasObject=%d", step, stationName, arm, (int)armHasWafer);
				if (armHasWafer)
				{
					logWarn(wtr->getName().c_str(), "Robot线程：step %d,取消GET station=%s arm=%d, 原因:手臂已有片", step, stationName, arm);
					return false;
				}
				return true;
			};
			auto canExecutePut = [this](const char* stationName, int step, int arm) -> bool
			{
				const bool armHasWafer = wtr->hasObject(arm);
				logInform(wtr->getName().c_str(), "Robot线程：step %d,PUT前检查 station=%s arm=%d hasObject=%d", step, stationName, arm, (int)armHasWafer);
				if (!armHasWafer)
				{
					logWarn(wtr->getName().c_str(), "Robot线程：step %d,取消PUT station=%s arm=%d, 原因:手臂无片", step, stationName, arm);
					return false;
				}
				return true;
			};
			auto canExecuteExchange = [this](const char* stationName, int step, int getArm, int putArm) -> bool
			{
				const bool getArmHasWafer = wtr->hasObject(getArm);
				const bool putArmHasWafer = wtr->hasObject(putArm);
				logInform(wtr->getName().c_str(), "Robot线程：step %d,EXCHANGE前检查 station=%s getArm=%d hasObject=%d putArm=%d hasObject=%d",
					step, stationName, getArm, (int)getArmHasWafer, putArm, (int)putArmHasWafer);
				if (getArmHasWafer || !putArmHasWafer)
				{
					logWarn(wtr->getName().c_str(), "Robot线程：step %d,取消EXCHANGE station=%s getArm=%d putArm=%d, 原因:执行前手臂状态已变化",
						step, stationName, getArm, putArm);
					return false;
				}
				return true;
			};
			

			while (!stopRequested)
			{
				Sleep(50);

				// 暂停等待（与其他线程一致的模式）
				{
					std::unique_lock<std::mutex> lock(mtx);
					cv.wait(lock, [this] { return running.load() || stopRequested.load(); });
				}
				if (stopRequested) break;

				switch (robot_step)
				{
				// ===================== IDLE：按优先级轮询请求标志 =====================
				case 10:
				{
					// 优先级1: PUT_TO_LL（释放手臂）
					if (robot_put_to_lla.requested.load()) { robot_step = 4000; break; }
					if (robot_put_to_llb.requested.load()) { robot_step = 4100; break; }
					// 优先级1.5: LL回片后的立即补取，入口仍由LL线程发起，TM只做配合执行
					if (robot_get_from_lla.requested.load() && robot_get_from_lla.expedited.load()) { robot_step = 1000; break; }
					if (robot_get_from_llb.requested.load() && robot_get_from_llb.expedited.load()) { robot_step = 1100; break; }
					// 优先级2: GET_FROM_PM（取回已完成工艺的片）
					if (robot_get_from_pm1.requested.load()) { robot_step = 3000; break; }
					if (robot_get_from_pm2.requested.load()) { robot_step = 3100; break; }
					if (robot_get_from_pm3.requested.load()) { robot_step = 3200; break; }
					if (robot_get_from_pm4.requested.load()) { robot_step = 3300; break; }
					// 优先级3: EXCHANGE_AT_PM
					if (robot_exchange_pm1.requested.load()) { robot_step = 5000; break; }
					if (robot_exchange_pm2.requested.load()) { robot_step = 5100; break; }
					if (robot_exchange_pm3.requested.load()) { robot_step = 5200; break; }
					if (robot_exchange_pm4.requested.load()) { robot_step = 5300; break; }
					// 优先级4: PUT_TO_PM
					if (robot_put_to_pm1.requested.load()) { robot_step = 2000; break; }
					if (robot_put_to_pm2.requested.load()) { robot_step = 2100; break; }
					if (robot_put_to_pm3.requested.load()) { robot_step = 2200; break; }
					if (robot_put_to_pm4.requested.load()) { robot_step = 2300; break; }
					// 优先级5: GET_FROM_LL
					if (robot_get_from_lla.requested.load()) { robot_step = 1000; break; }
					if (robot_get_from_llb.requested.load()) { robot_step = 1100; break; }
				}
				break;

				// ===================== 1000: GET_FROM_LLA =====================
				case 1000:
				{
					int arm = robot_get_from_lla.arm.load();
					int slot = robot_get_from_lla.slot.load();
					logInform(wtr->getName().c_str(), "Robot线程：step：1000,从LLA取片, arm=%d, slot=%d", arm, slot);
					auto cmd = wtr->createGetCommand(lk1, arm, slot);
					wtr->startCommand(cmd);
					cmd->wait();
					if (cmd->hasError())
					{
						auto alarm_msg = cmd->alarmMessage();
						robot_get_from_lla.success.store(false);
						logFailed(wtr->getName(), Poco::format("从LLA取片失败, 错误码:%d", alarm_msg->code()));
					} 
					else 
					{
						robot_get_from_lla.success.store(true);
					}
					robot_get_from_lla.requested.store(false);
					robot_get_from_lla.expedited.store(false);
					robot_get_from_lla.done.store(true);
					robot_step = 10;
				}
				break;

				// ===================== 1100: GET_FROM_LLB =====================
				case 1100:
				{
					int arm = robot_get_from_llb.arm.load();
					int slot = robot_get_from_llb.slot.load();
					logInform(wtr->getName().c_str(), "Robot线程：从LLB取片, arm=%d, slot=%d", arm, slot);
					auto cmd = wtr->createGetCommand(lk2, arm, slot);
					wtr->startCommand(cmd);
					cmd->wait();
					if (cmd->hasError()) 
					{
						auto alarm_msg = cmd->alarmMessage();
					    robot_get_from_llb.success.store(false);
					    logFailed(wtr->getName(), Poco::format("从LLB取片失败, 错误码:%d", alarm_msg->code()));	
					} 
					else
					{
						robot_get_from_llb.success.store(true);
					}
					robot_get_from_llb.requested.store(false);
					robot_get_from_llb.expedited.store(false);
					robot_get_from_llb.done.store(true);
					robot_step = 10;
				}
				break;

				// ===================== 2000: PUT_TO_PM1 =====================
				case 2000:
				{
					int arm = robot_put_to_pm1.arm.load();
					logInform(wtr->getName().c_str(), "Robot线程：放片到PM1, arm=%d", arm);
					auto cmd = wtr->createPutCommand(pm1, arm, 1);
					wtr->startCommand(cmd);
					cmd->wait();
					if (cmd->hasError())
					{
						robot_put_to_pm1.success.store(false);
						logFailed(wtr->getName(), "放片到PM1失败");
					} 
					else {
						robot_put_to_pm1.success.store(true);
					}
					robot_put_to_pm1.requested.store(false);
					robot_put_to_pm1.done.store(true);
					robot_step = 10;
				}
				break;

				// ===================== 2100: PUT_TO_PM2 =====================
				case 2100:
				{
					int arm = robot_put_to_pm2.arm.load();
					logInform(wtr->getName().c_str(), "Robot线程：step 2100,放片到PM2, arm=%d", arm);
					if (!canExecutePut("PM2", 2100, arm))
					{
						robot_put_to_pm2.success.store(false);
						robot_put_to_pm2.requested.store(false);
						robot_put_to_pm2.done.store(true);
						robot_step = 10;
						break;
					}
					auto cmd = wtr->createPutCommand(pm2, arm, 1);
					wtr->startCommand(cmd);
					cmd->wait();
					if (cmd->hasError())
					{
						robot_put_to_pm2.success.store(false);
						logFailed(wtr->getName(), "放片到PM2失败");
					} 
					else
					{
						robot_put_to_pm2.success.store(true);
						logInform(wtr->getName().c_str(), "step:2100, arm=%d 放到PM2成功-->-->send", arm);
					}
					robot_put_to_pm2.requested.store(false);
					robot_put_to_pm2.done.store(true);
					robot_step = 10;
				}
				break;

				// ===================== 2200: PUT_TO_PM3 =====================
				case 2200:
				{
					int arm = robot_put_to_pm3.arm.load();
					logInform(wtr->getName().c_str(), "Robot线程：step 2200 ,放片到PM3, arm=%d", arm);
					auto cmd = wtr->createPutCommand(pm3, arm, 1);
					wtr->startCommand(cmd);
					cmd->wait();
					if (cmd->hasError()) 
					{
						robot_put_to_pm3.success.store(false);
						logFailed(wtr->getName(), "放片到PM3失败");
					}
					else
					{
						robot_put_to_pm3.success.store(true);
					}
					robot_put_to_pm3.requested.store(false);
					robot_put_to_pm3.done.store(true);
					robot_step = 10;
				}
				break;

				// ===================== 2300: PUT_TO_PM4 =====================
				case 2300:
				{
					int arm = robot_put_to_pm4.arm.load();
					logInform(wtr->getName().c_str(), "Robot线程：step 2300 放片到PM4, arm=%d", arm);
					auto cmd = wtr->createPutCommand(pm4, arm, 1);
					wtr->startCommand(cmd);
					cmd->wait();
					if (cmd->hasError())
					{
						auto alarm_msg = cmd->alarmMessage();
						robot_put_to_pm4.success.store(false);
						logFailed(wtr->getName(), Poco::format("从PUT_TO_PM4失败, 错误码:%d", alarm_msg->code()));
					} 
					else 
					{
						robot_put_to_pm4.success.store(true);
					}
					robot_put_to_pm4.requested.store(false);
					robot_put_to_pm4.done.store(true);
					robot_step = 10;
				}
				break;

				// ===================== 3000: GET_FROM_PM1 =====================
				case 3000:
				{
					int arm = robot_get_from_pm1.arm.load();
					logInform(wtr->getName().c_str(), "Robot线程：step 3000,从PM1取片, arm=%d", arm);
					auto cmd = wtr->createGetCommand(pm1, arm, 1);
					wtr->startCommand(cmd);
					cmd->wait();
					if (cmd->hasError()) 
					{
						auto alarm_msg = cmd->alarmMessage();
						robot_get_from_pm1.success.store(false);
						logFailed(wtr->getName(), Poco::format("从PM1取片失败, 错误码:%d", alarm_msg->code()));
					}
					else 
					{
						robot_get_from_pm1.success.store(true);
					}
					robot_get_from_pm1.requested.store(false);
					robot_get_from_pm1.done.store(true);
					robot_step = 10;
				}
				break;

				// ===================== 3100: GET_FROM_PM2 =====================
				case 3100:
				{
					int arm = robot_get_from_pm2.arm.load();
					if (!canExecuteGet("PM2", 3100, arm))
					{
						robot_get_from_pm2.success.store(false);
						robot_get_from_pm2.requested.store(false);
						robot_get_from_pm2.done.store(true);
						robot_step = 10;
						break;
					}
					auto cmd = wtr->createGetCommand(pm2, arm, 1);
					wtr->startCommand(cmd);
					cmd->wait();
					if (cmd->hasError()) 
					{
						auto alarm_msg = cmd->alarmMessage();
						robot_get_from_pm2.success.store(false);
						logFailed(wtr->getName(), Poco::format("从PM2取片失败, 错误码:%d", alarm_msg->code()));
					}
					else 
					{
						robot_get_from_pm2.success.store(true);
						logInform(wtr->getName().c_str(), "step:3100, arm=%d 从PM2取片成功-->send", arm);
					}
					robot_get_from_pm2.requested.store(false);
					robot_get_from_pm2.done.store(true);
					robot_step = 10;
				}
				break;

				// ===================== 3200: GET_FROM_PM3 =====================
				case 3200:
				{
					int arm = robot_get_from_pm3.arm.load();
					logInform(wtr->getName().c_str(), "Robot线程：step:3200,从PM3取片, arm=%d", arm);
					auto cmd = wtr->createGetCommand(pm3, arm, 1);
					wtr->startCommand(cmd);
					cmd->wait();
					if (cmd->hasError())
					{
						auto alarm_msg = cmd->alarmMessage();
						robot_get_from_pm3.success.store(false);
						logFailed(wtr->getName(), Poco::format("从PM3取片失败, 错误码:%d", alarm_msg->code()));
					} 
					else {
						robot_get_from_pm3.success.store(true);
					}
					robot_get_from_pm3.requested.store(false);
					robot_get_from_pm3.done.store(true);
					robot_step = 10;
				}
				break;

				// ===================== 3300: GET_FROM_PM4 =====================
				case 3300:
				{
					int arm = robot_get_from_pm4.arm.load();
					logInform(wtr->getName().c_str(), "Robot线程：step:3300,从PM4取片, arm=%d", arm);
					auto cmd = wtr->createGetCommand(pm4, arm, 1);
					wtr->startCommand(cmd);
					cmd->wait();
					if (cmd->hasError())
					{
						auto alarm_msg = cmd->alarmMessage();
						robot_get_from_pm4.success.store(false);
						logFailed(wtr->getName(), Poco::format("从PM4取片失败, 错误码:%d", alarm_msg->code()));
					}
					else {
						robot_get_from_pm4.success.store(true);
					}
					robot_get_from_pm4.requested.store(false);
					robot_get_from_pm4.done.store(true);
					robot_step = 10;
				}
				break;

				// ===================== 4000: PUT_TO_LLA =====================
				case 4000:
				{
					int arm = robot_put_to_lla.arm.load();
					int slot = robot_put_to_lla.slot.load();
					logInform(wtr->getName().c_str(), "Robot线程：step:4000,放片到LLA, arm=%d, slot=%d", arm, slot);
					auto cmd = wtr->createPutCommand(lk1, arm, slot);
					wtr->startCommand(cmd);
					cmd->wait();
					if (cmd->hasError()) 
					{
						auto alarm_msg = cmd->alarmMessage();
						robot_put_to_lla.success.store(false);
						logFailed(wtr->getName(), Poco::format("放片到LLA失败, 错误码:%d", alarm_msg->code()));
					}
					else 
					{
						robot_put_to_lla.success.store(true);
					}
					robot_put_to_lla.requested.store(false);
					robot_put_to_lla.done.store(true);
					robot_step = 10;
				}
				break;

				// ===================== 4100: PUT_TO_LLB =====================
				case 4100:
				{
					int arm = robot_put_to_llb.arm.load();
					int slot = robot_put_to_llb.slot.load();
					logInform(wtr->getName().c_str(), "Robot线程：放片到LLB, arm=%d, slot=%d", arm, slot);
					auto cmd = wtr->createPutCommand(lk2, arm, slot);
					wtr->startCommand(cmd);
					cmd->wait();
					if (cmd->hasError())
					{
						auto alarm_msg = cmd->alarmMessage();
						robot_put_to_llb.success.store(false);
						logFailed(wtr->getName(), Poco::format("放片到LLB失败, 错误码:%d", alarm_msg->code()));	
					} 
					else
					{
						robot_put_to_llb.success.store(true);
					}
					robot_put_to_llb.requested.store(false);
					robot_put_to_llb.done.store(true);
					robot_step = 10;
				}
				break;

				// ===================== 5000: EXCHANGE_PM1（先取后放）=====================
				case 5000:
				{
					int getArm = exchange_info_pm1.getArm.load();
					int putArm = exchange_info_pm1.putArm.load();
					logInform(wtr->getName().c_str(), "Robot线程：PM1交换, getArm=%d, putArm=%d", getArm, putArm);
					// 第一步：取出已加工片
					auto cmd_get = wtr->createGetCommand(pm1, getArm, 1);
					wtr->startCommand(cmd_get);
					cmd_get->wait();
					if (cmd_get->hasError()) {
						robot_exchange_pm1.success.store(false);
						logFailed(wtr->getName(), "PM1交换取片失败");
						robot_exchange_pm1.requested.store(false);
						robot_exchange_pm1.done.store(true);
						robot_step = 10;
						break;
					}
					// 第二步：放入新片
					auto cmd_put = wtr->createPutCommand(pm1, putArm, 1);
					wtr->startCommand(cmd_put);
					cmd_put->wait();
					if (cmd_put->hasError()) {
						robot_exchange_pm1.success.store(false);
						logFailed(wtr->getName(), "PM1交换放片失败");
					} else {
						robot_exchange_pm1.success.store(true);
					}
					robot_exchange_pm1.requested.store(false);
					robot_exchange_pm1.done.store(true);
					robot_step = 10;
				}
				break;

				// ===================== 5100: EXCHANGE_PM2 =====================
				case 5100:
				{
					int getArm = exchange_info_pm2.getArm.load();
					int putArm = exchange_info_pm2.putArm.load();
					logInform(wtr->getName().c_str(), "Robot线程：step：5100,PM2交换片, getArm=%d, putArm=%d", getArm, putArm);
					logInform("PM2", "Robot线程开始消费PM2交换请求, getArm=%d, putArm=%d", getArm, putArm);
					if (!canExecuteExchange("PM2", 5100, getArm, putArm))
					{
						robot_exchange_pm2.success.store(false);
						robot_exchange_pm2.requested.store(false);
						robot_exchange_pm2.done.store(true);
						robot_step = 10;
						break;
					}

					// 这里才是真正的一放一取“组合动作”：
					// 同一个 robot_step 里先从 PM2 取，再向 PM2 放，底层上是一组连续的交换片命令。
					// 和 LLImmediateRepickState 不同，后者只是给 LL 线程记住“回片后还要补取”的计划，
					// 具体动作仍拆在多个 LL case 里分步完成。
					auto get_start = std::chrono::steady_clock::now();
					auto cmd_get = wtr->createGetCommand(pm2, getArm, 1);
					wtr->startCommand(cmd_get);
					cmd_get->wait();
					auto get_end = std::chrono::steady_clock::now();
					logInform(wtr->getName().c_str(), "Robot线程：PM2交换取片耗时(ms)=%lld, getArm=%d", (long long)std::chrono::duration_cast<std::chrono::milliseconds>(get_end - get_start).count(), getArm);

					if (cmd_get->hasError())
					{
						auto alarm_msg = cmd_get->alarmMessage();
						if (alarm_msg->code() == 0x3000)
						{
							logFailed(wtr->getName(), "PM2交换取片失败, 错误码:0x3000 手臂已占");
						}

						robot_exchange_pm2.success.store(false);
						robot_exchange_pm2.requested.store(false);
						robot_exchange_pm2.done.store(true);
						robot_step = 10;
						break;
					}
					else
					{
						auto put_start = std::chrono::steady_clock::now();
						auto cmd_put = wtr->createPutCommand(pm2, putArm, 1);
						wtr->startCommand(cmd_put);
						cmd_put->wait();
						auto put_end = std::chrono::steady_clock::now();
						logInform(wtr->getName().c_str(), "Robot线程：PM2交换放片耗时(ms)=%lld, putArm=%d", (long long)std::chrono::duration_cast<std::chrono::milliseconds>(put_end - put_start).count(), putArm);
						if (cmd_put->hasError())
						{
							robot_exchange_pm2.success.store(false);
							logFailed(wtr->getName(), "PM2交换放片失败");
						}
						else
						{
							robot_exchange_pm2.success.store(true);
							logInform(wtr->getName().c_str(), "Robot线程：PM2交换片成功, getArm=%d, putArm=%d", getArm, putArm);
						}
					}

					robot_exchange_pm2.requested.store(false);
					robot_exchange_pm2.done.store(true);
					robot_step = 10;
				}
				break;

				// ===================== 5200: EXCHANGE_PM3 =====================
				case 5200:
				{
					int getArm = exchange_info_pm3.getArm.load();
					int putArm = exchange_info_pm3.putArm.load();
					logInform(wtr->getName().c_str(), "Robot线程：PM3交换, getArm=%d, putArm=%d", getArm, putArm);
					auto cmd_get = wtr->createGetCommand(pm3, getArm, 1);
					wtr->startCommand(cmd_get);
					cmd_get->wait();
					if (cmd_get->hasError()) {
						auto alarm_msg = cmd_get->alarmMessage();
						if (alarm_msg->code() == 0x3000) {
							logInform(wtr->getName().c_str(), "Robot线程：PM3交换失败, 0x3000手臂已占, getArm=%d", getArm);
							auto cmd_clear = wtr->createClearErrorCommand();
							wtr->startCommand(cmd_clear);
							cmd_clear->wait();
						}
						robot_exchange_pm3.success.store(false);
						logFailed(wtr->getName(), "PM3交换取片失败");
						robot_exchange_pm3.requested.store(false);
						robot_exchange_pm3.done.store(true);
						robot_step = 10;
						break;
					}
					auto cmd_put = wtr->createPutCommand(pm3, putArm, 1);
					wtr->startCommand(cmd_put);
					cmd_put->wait();
					if (cmd_put->hasError()) {
						robot_exchange_pm3.success.store(false);
						logFailed(wtr->getName(), "PM3交换放片失败");
					} else {
						robot_exchange_pm3.success.store(true);
					}
					robot_exchange_pm3.requested.store(false);
					robot_exchange_pm3.done.store(true);
					robot_step = 10;
				}
				break;

				// ===================== 5300: EXCHANGE_PM4 =====================
				case 5300:
				{
					int getArm = exchange_info_pm4.getArm.load();
					int putArm = exchange_info_pm4.putArm.load();
					logInform(wtr->getName().c_str(), "Robot线程：PM4交换, getArm=%d, putArm=%d", getArm, putArm);
					auto cmd_get = wtr->createGetCommand(pm4, getArm, 1);
					wtr->startCommand(cmd_get);
					cmd_get->wait();
					if (cmd_get->hasError()) {
						auto alarm_msg = cmd_get->alarmMessage();
						if (alarm_msg->code() == 0x3000) {
							logInform(wtr->getName().c_str(), "Robot线程：PM4交换失败, 0x3000手臂已占, getArm=%d", getArm);
							auto cmd_clear = wtr->createClearErrorCommand();
							wtr->startCommand(cmd_clear);
							cmd_clear->wait();
						}
						robot_exchange_pm4.success.store(false);
						logFailed(wtr->getName(), "PM4交换取片失败");
						robot_exchange_pm4.requested.store(false);
						robot_exchange_pm4.done.store(true);
						robot_step = 10;
						break;
					}
					auto cmd_put = wtr->createPutCommand(pm4, putArm, 1);
					wtr->startCommand(cmd_put);
					cmd_put->wait();
					if (cmd_put->hasError()) {
						robot_exchange_pm4.success.store(false);
						logFailed(wtr->getName(), "PM4交换放片失败");
					} else {
						robot_exchange_pm4.success.store(true);
					}
					robot_exchange_pm4.requested.store(false);
					robot_exchange_pm4.done.store(true);
					robot_step = 10;
				}
				break;

				default:
					robot_step = 10;
					break;
				}
			}
		}
		catch (std::exception& e) {
			logFailed(wtr->getName(), Poco::format("Robot线程异常: %s", std::string(e.what())));
			pauseAllThreads();
		}
		catch (...) {
			logFailed(wtr->getName(), "Robot线程异常: 未知异常");
			pauseAllThreads();
		}
	}

	void QSlotTransferCycleVTMWidgetPrivate::executeUpdateTransferStatus()
	{
		try {

			elp1 = kernel->getKernelModule<EFEMLPSubsystem>("ELP1");
			elp2 = kernel->getKernelModule<EFEMLPSubsystem>("ELP2");

			auto resetEfemReturnCompletedTasksSafely = [this](UnifiedWaferTask::Location source)
			{
				//判断是否可以重置EFEM搬回任务
				if (!taskManager.canResetEfemReturnCompletedTasks(source))
				{
					return;
				}

				const auto completedTasks = taskManager.getEfemRuturnCompletedTasksBySource(source);
				for (const auto& task : completedTasks)
				{
					taskManager.updateTaskStatus(task.taskId, UnifiedWaferTask::TaskType::UNKNOWN, UnifiedWaferTask::Status::UNKNOWN_PROGRESS);
				}

				if (!completedTasks.empty())
				{
					taskManager.lessTaskIdSortAlgorithm();
				}
			};

			auto isSafeCycleBoundary = [this]()
			{
				return !taskManager.hasInFlightTasksExceptCompletedEfemReturn();
			};

			while (!stopRequested)
			{
				update_step_once_finished = false;
				Sleep(500);
				//onGetStep();
				
				if (((finished_time_lla == cycle_times_lla) || (finished_time_llb == cycle_times_llb)) 
					&& (!elp1->hasDoorOpend() && !elp2->hasDoorOpend())
					&& isSafeCycleBoundary())
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
					robot_step = 10;//2026-5-14: robot step也更新到10

					pm1_auto_step.store(10);
					pm2_auto_step.store(10);
					pm3_auto_step.store(10);
					pm4_auto_step.store(10);
					update_auto_step = 10;


					running = false; //阻塞，此时EFEM线程下料完成，wait case 201处..
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
					const auto efemReturnCompletedSnapshot = taskManager.getEfemRuturnCompletedTasks();
					Sleep(500);
					
					if (lp1_cycle_one_time_finished && !cycleFinished_lla)
					{//Lp1的一次Cycle已做完
						//2025-11-17: 多加一条检测条件：
						if (efemReturnCompletedSnapshot.size() == originTaskSize && (originTaskSize > 0) && isSafeCycleBoundary())
						{
							update_auto_step = 1030;
						}
					}
					else if (lp2_cycle_one_time_finished && !cycleFinished_llb)
					{//Lp2的一次Cycle已做完
						if (efemReturnCompletedSnapshot.size() == originTaskSize && (originTaskSize > 0) && isSafeCycleBoundary())
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
						resetEfemReturnCompletedTasksSafely(UnifiedWaferTask::Location::LP1);
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
						resetEfemReturnCompletedTasksSafely(UnifiedWaferTask::Location::LP2);
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
			pauseAllThreads();
			saveCurrentStateSnapshot(e.what(), Poco::format("executeUpdateTransferStatus step:%d", update_auto_step));
			onUpdateControlEnabled("execute_pbt", true);
			onUpdateLightButtonStatus("light_running_pbt", 3);

			logError("Cyclelog", "UpdateTransfer thread crashed:%s", e.what());
			qCritical() << "UpdateTransfer thread crashed:" << e.what();
			
		}
		catch (...) {
			pauseAllThreads();
			saveCurrentStateSnapshot("unknown exception", Poco::format("executeUpdateTransferStatus step:%d", update_auto_step));
			onUpdateControlEnabled("execute_pbt", true);
			onUpdateLightButtonStatus("light_running_pbt", 3);

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
		robot_step = 10;//2026-5-14: robot step也更新到10
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
		resetAllRobotFlags();
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
			logWarn("Cyclelog", "start pause All Threads .....");
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
		logWarn("Cyclelog", "pause All Threads  end!");
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
		// 仅在真实下料冲突，或对侧LL下层完成片需要优先回LP时上锁。
		std::string otherLL;
		bool otherUnloadRequested = false;
		if (LLName == "LLB")
		{
			otherLL = "LLA";
			otherUnloadRequested = tool_allow_put_wafer_LLA;
		}
		else if (LLName == "LLA")
		{
			otherLL = "LLB";
			otherUnloadRequested = tool_allow_put_wafer_LLB;
		}
		else
		{
			return false;
		}

		const bool hasUnloadConflict =
			otherUnloadRequested || taskManager.hasEfemUnloadInProgress(otherLL);

		const bool hasLowerSlotPriorityReturn =
			taskManager.hasLoadLockLowerPriorityReturn(otherLL);

		return hasUnloadConflict || hasLowerSlotPriorityReturn;
	}

	bool QSlotTransferCycleVTMWidgetPrivate::getArmWaferIsPmPending(int arm)
	{
		auto task = taskManager.getRobotTaskInfo(arm); //任务状态

		if (task.taskType == UnifiedWaferTask::PM_PROCESS)
		{
			if (task.status == UnifiedWaferTask::COMPLETED)
			{
				return false;
			}
			else
			{
				return true;
			}
		}
		else
		{
			logWarn("Cyclelog", "getArmWaferIsPmPending: arm %d 没有PM_PROCESS任务.", arm);
			return false;
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
				static int vacuum_check_fail_count = 0; // 防抖计数器
				if (tm->getTMCavityVacuumValueUpperLimitReachesTheSetValue())
				{
					vacuum_check_fail_count = 0; // 重置计数器
					robot_auto_step = 2100;
				}
				else
				{
					vacuum_check_fail_count++;
					if (vacuum_check_fail_count >= 3) // 连续3次检测不达标才触发抽真空
					{
						logWarn("Cycle", "TM真空值连续3次未达标，触发抽真空. 当前值: %.2f", tm->getTMCavityVacuumValue());
						tm_get_vacuum = true;
						vacuum_check_fail_count = 0; // 重置计数器
						robot_auto_step = 2060;
					}
					else
					{
						Sleep(500); // 等待一段时间后再次检测
					}
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

		finished_time_lla = 0;

		finished_time_llb = 0;
		
		//while (!vacumm_step_once_finished || !loadlock1_step_once_finished || !loadlock2_step_once_finished)
		//{
		//	Sleep(100);
		//}
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
							logError(reset_process_name.c_str(), Poco::format("%s复位指令执行失败.", cmd_ewtr->getName()).c_str());
						}
					}
					
					//加home，为了复位关腔门的安全信号
					auto home_cmd = wtr->createHomeCommand();
					wtr->startCommand(home_cmd);
					home_cmd->wait();

					if (cmd->hasError() || home_cmd->hasError())
					{
						rest_step = 15000;
						logError(reset_process_name.c_str(), Poco::format("%s复位指令执行失败.", wtr->getName()).c_str());
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
							logError(reset_process_name.c_str(), Poco::format("%s复位指令执行失败.", cmd_elp1->getName()).c_str());
						}
						else if (cmd_elp2->hasError())
						{
							rest_step = 15000;
							logError(reset_process_name.c_str(), Poco::format("%s复位指令执行失败.", cmd_elp2->getName()).c_str());
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
						logError(reset_process_name.c_str(), Poco::format("%s复位指令执行失败.", lk1->getName()).c_str());
					}
					else if (cmd_lk2->hasError())
					{
						rest_step = 15000;
						logError(reset_process_name.c_str(), Poco::format("%s复位指令执行失败.", lk2->getName()).c_str());
					}
					else if (cmd_pm1->hasError())
					{
					rest_step = 15000;
					logError(reset_process_name.c_str(), Poco::format("%s复位指令执行失败.", pm1->getName()).c_str());
					}
					else if (cmd_pm2->hasError())
					{
						rest_step = 15000;
						logError(reset_process_name.c_str(), Poco::format("%s复位指令执行失败.", pm2->getName()).c_str());
					}
					else if (cmd_pm3->hasError())
					{
					rest_step = 15000;
					logError(reset_process_name.c_str(), Poco::format("%s复位指令执行失败.", pm3->getName()).c_str());
					}
					else if (cmd_pm4->hasError())
					{
						rest_step = 15000;
						logError(reset_process_name.c_str(), Poco::format("%s复位指令执行失败.", pm4->getName()).c_str());
					}
					else if (cmd_tm->hasError())
					{
						rest_step = 15000;
						logError(reset_process_name.c_str(), Poco::format("%s复位指令执行失败.", tm->getName()).c_str());
					}
					else if (cmd_pump->hasError())
					{
						rest_step = 15000;
						logError(reset_process_name.c_str(), Poco::format("%s复位指令执行失败.", pump->getName()).c_str());
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
					logError(reset_process_name.c_str(), Poco::format("%s关闭传输腔门阀命令执行失败.", lk1->getName()).c_str());
				}
				else if (cmd_lk2->hasError())
				{
					rest_step = 15000;
					logError(reset_process_name.c_str(), Poco::format("%s关闭传输腔门阀命令执行失.", lk2->getName()).c_str());
				}
				else if (cmd_pm1->hasError())
				{
				rest_step = 15000;
				logError(reset_process_name.c_str(), Poco::format("%s关闭传输腔门阀命令执行失败.", pm1->getName()).c_str());
				}
				else if (cmd_pm2->hasError())
				{
					rest_step = 15000;
					logError(reset_process_name.c_str(), Poco::format("%s关闭传输腔门阀命令执行失败.", pm2->getName()).c_str());
				}
				else if (cmd_pm3->hasError())
				{
				rest_step = 15000;
				logError(reset_process_name.c_str(), Poco::format("%s关闭传输腔门阀命令执行失败.", pm3->getName()).c_str());
				}
				else if (cmd_pm4->hasError())
				{
					rest_step = 15000;
					logError(reset_process_name.c_str(), Poco::format("%s关闭传输腔门阀命令执行失败.", pm4->getName()).c_str());
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
				robot_step = 10; //2026-5-14  重置机器手步骤
				pm1_auto_step.store(10);
				pm2_auto_step.store(10);
				pm3_auto_step.store(10);
				pm4_auto_step.store(10);
				update_auto_step = 10;
				reset_loop = false;
				reset_finish = true;
				current_lp_cycle = false;  //2025-11-17  重置
				rest_step = -1;
				resetAllRobotFlags(); //2026-5-14 复位所有机器人相关标志位
				
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
						// 如果有抽真空需求，先检测机器人是否安全且空闲
						if (tm_get_vacuum || loadlock1_get_vacuum || loadlock2_get_vacuum)
						{
							// 核心检测：LLA、LLB 的机器人原点信号必须为真（代表手臂已缩回），且机器人软件状态不忙
							if (lk1->getWtrOriginSafeSignal() && lk2->getWtrOriginSafeSignal() && !wtr->isBusy())
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
							}
							else
							{
								// 如果不满足条件，则留在 case 10 继续等待
								// 可以适当打印日志以便观察（增加 static 计数器避免日志刷屏）
								static int wait_log_count = 0;
								if (wait_log_count++ % 50 == 0) {
									logInform("VacuumCycle", "等待 WTR 安全信号或 WTR 空闲以启动真空流程...");
								}
								Sleep(200);
							}
						}
						else
						{
							Sleep(10);
						}
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
		logInform("Cycle", Poco::format("%s = %d", robot_process_name, robot_step).c_str());
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
		//注释掉继续按钮
		d->ui->continue_pbt->setEnabled(false);
		d->ui->continue_pbt->hide();

		d->ui->wph_test_gbx->hide();
		d->ui->loadlock1_put_cassette_finished_pbt->hide();
		d->ui->loadlock2_put_cassette_finished_pbt->hide();
		d->simulation_mode_enabled.store(d->ui->simulation_cbx->checkState() == Qt::CheckState::Checked);
		connect(d->ui->simulation_cbx, &QCheckBox::stateChanged, this,
			[d](int state)
			{
				d->simulation_mode_enabled.store(state == Qt::CheckState::Checked);
			});
		d->simulate_pm2_enabled.store(d->ui->simulate_pm2_cbx->checkState() == Qt::CheckState::Checked);
		connect(d->ui->simulate_pm2_cbx, &QCheckBox::stateChanged, this,
			[d](int state)
			{
				d->simulate_pm2_enabled.store(state == Qt::CheckState::Checked);
			});

		d->ui->spb_min->hide();
		d->ui->spb_max->hide();
		d->ui->label_3->hide();
		d->ui->label_4->hide();
		d->ui->abort_pbt->hide();
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
			QMessageBox::warning(this, tr("警告信息"), tr("加载路径不能为空."));
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
			QMessageBox::warning(this, tr("警告信息"), tr("请先添加顺序."));
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
			QMessageBox::warning(this, tr("警告信息"), tr("保存路径不能为空."));
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

		logWarn("Cyclelog", "start onPause.....");
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

		//2026-3-24 注释掉暂停按钮使能
		//d->ui->pause_pbt->setEnabled(false);

		//2025-8-06
		d->onUpdateProcessControlEnabled(true);
		d->onUpdateLightButtonStatus("light_running_pbt", 4);
		logWarn("Cyclelog", "onPause end.....");
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

		d->ui->pause_pbt->setEnabled(true); 
		d->ui->execute_pbt->setEnabled(false);
		d->ui->reset_pbt->setEnabled(false);
		d->onUpdateLightButtonStatus("light_running_pbt", 2);
	}

	void QSlotTransferCycleVTMWidget::onReset(){
		Q_D(QSlotTransferCycleVTMWidget);
		std::shared_ptr<FortrendTMCavitySubsystem> tm = d->kernel->getKernelModule<FortrendTMCavitySubsystem>("TM");
		d->plcauto = tm->getPlcMode();

		if(!d->isSimulationModeEnabled())
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
		logWarn("Cyclelog", "start all workflow.....");
		//start action & store param
		std::shared_ptr<FortrendPMCavitySubsystem> pm1 = d->kernel->getKernelModule<FortrendPMCavitySubsystem>("PM1");
		std::shared_ptr<FortrendPMCavitySubsystem> pm2 = d->kernel->getKernelModule<FortrendPMCavitySubsystem>("PM2");
		std::shared_ptr<FortrendPMCavitySubsystem> pm3 = d->kernel->getKernelModule<FortrendPMCavitySubsystem>("PM3");
		std::shared_ptr<FortrendPMCavitySubsystem> pm4 = d->kernel->getKernelModule<FortrendPMCavitySubsystem>("PM4");

		d->cycle_times_lla = d->ui->cycle_setting_times_sbx->value(); //LP1循环次数
		d->cycle_times_llb = d->ui->cycle_setting_times_sbx_2->value();//LP2循环次数

		if (taskManager.getAllTasksSize() == 0 && !d->ispause)// 无任务配置，无暂停，才执行解析流程配方
		{
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

			//auto task1 = taskManager.getRobotTaskInfo(0); //A臂任务状态
			//auto task2 = taskManager.getRobotTaskInfo(1); //B臂任务状态


			//2025-8-14 默认已经手动处理机台片子，重新把wafer数据刷新到初始状态
			//暂停过，检测机台状态
			if (d->ispause && taskManager.detectionHasNoInitialTypeTasks()) {

				QMessageBox::warning(this, "警告", "请检查机台有无片子，再执行整体复位");
				return;
			}
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
			logError("Cycle","重新加载配方,并整机复位,再执行.");
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

		// 机械手集中调度线程
		std::thread thd_robot(&QSlotTransferCycleVTMWidget::executeTMTransfer, this);
		thd_robot.detach();

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
		d->threads.emplace_back(&QSlotTransferCycleVTMWidget::executeTMTransfer, this);
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

