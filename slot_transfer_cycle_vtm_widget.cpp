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
#include <atomic>
#include <iostream>
#include <Windows.h>

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
#include <QTimer>

#include "EFEM/efem_wafer_robot_subsystem.h" 
#include "EFEM/efem_loadport_subsystem.h" 

#include  "Kernel/Fortrend/fortrend_station.h"
#include  "Kernel/Fortrend/aligner_abstract_subsystem.h"
#include  "Kernel/kernel_subsystem_update_command.h"
#include  "kernel/kernel_block_manager.h"

#include  "Aligner/fortrend_aligner_subsystem.h"
#include  "CoolingCavity/fortrend_cooling_cavity_subsystem.h"
#include  "LoadLock/fortrend_loadlock_subsystem.h"
#include  "PMCavity/fortrend_pm_cavity_subsystem.h"
#include  "PMCavity/fortrend_main_pm_cavity_subsystem_widget.h"
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

#include  "Pump/fortrend_pump_mechanical_open_command.h"
#include  "Pump/fortrend_pump_mechanical_close_command.h"
#include  "Pump/fortrend_pump_molecular_open_command.h"
#include  "Pump/fortrend_pump_molecular_close_command.h"

#include  "VTMSignalTower/fortrend_vtm_signal_tower.h"
#include "wafer_process_color_service.h"

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
#include <QtWidgets/QCheckBox>
#include <QMessageBox>
#include  <QLabel>
#include <atomic>

#define SIM_TEST

#if _MSC_VER >1600
#pragma execution_character_set("utf-8")
#endif

namespace FC{

	static void setProcessColorState(IKernel* kernel, const std::string& stationName, int slot, WaferProcessColorService::ProcessColorState state) {
		if (!kernel || slot <= 0) {
			return;
		}

		auto colorService = kernel->getKernelModule<WaferProcessColorService>();
		if (colorService) {
			colorService->setState(stationName, slot, state);
		}
	}

	static void refreshCassetteColorDisplay(IKernel* kernel, const std::shared_ptr<FortrendStation>& station) {
		if (!kernel || !station) {
			return;
		}

		auto cassManager = kernel->getKernelModule<FortrendCassetteManager>();
		if (!cassManager) {
			return;
		}

		auto stationCass = cassManager->getCassette(station.get());
		if (stationCass) {
			stationCass->setPodSize(stationCass->getPodSize(), true);
		}
	}

	struct LoadLockTransferWafer{
		int slot = 0;
		std::string transfer = "lk1_to_lk2";
		bool is_finish = false;
		int selected_arm = 0;
	};

	struct LPTransferWafer{
		int slot_lp = 0;
		int slot_lk = 0;
		std::string transfer = "lk1_to_lk2";
		std::string source_lp = "ELP1";
		bool is_finish_putlk = false;
		int selected_arm = 0;
	};
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
		bool is_finish = false;
	};


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

		void startLoadLock1Action();
		void startLoadLock2Action();
		void startEFEMAction();
		void startPMAction();
		void startUpdateStatusAction();
		void startRobotAction();
		void startVacuumAction();
		void resetAction();
		void setRunning(const bool value);
		void setLoadLock1PutCassetteFinished();
		void setLoadLock2PutCassetteFinished();

		bool setTransferSequence();
		bool setPMCavityParameter();
		void onUpdateCycleInfo();
		void onUpdateProcessControlEnabled(const bool value);
		void onUpdateControlEnabled(const QString control, const bool enabled);
		virtual void onAttributeChange(const IKernelCommand* cmd);

	private:
		QSlotTransferCycleVTMWidget * q_ptr;
		Ui::SlotTransferCycleVTMWidget *ui;
		std::shared_ptr<IKernel> kernel = 0;
		std::shared_ptr<FortrendVTMSignalTower> tower = 0;
		std::shared_ptr<FortrendTMCavitySubsystem> tmplc;
		bool plcauto = false;

		/*
		  调度容器总览（按“自动搬运四段”理解）
		  - 1.LL上料（ELP→LL）：EFEM线程消费 sequence_lp*_get_wafer，把片放入LL后：
		    - 将该片追加到 sequence_loadlock*_transfer_wafer（后续 LL→PM 的待取队列）
		    - 同时把“回片目标”预先追加到 sequence_lp*_put_wafer（后续 LL→ELP 用）

		  - 2.PM上料（LL→PM）：Robot线程消费 sequence_robot_transfer_wafer（母队列），从LL取片放到PM；
		    放片成功后将同一任务追加到 sequence_robot_get_wafer（登记“PM→LL要取回”的目标）

		  - 3.PM下料（PM→LL）：Robot线程消费 sequence_robot_get_wafer，从PM取回后按 target_loadlock 放回LL，
		    放回成功后从 sequence_robot_transfer_wafer 删除完成项

		  - 4.LL下料（LL→ELP）：EFEM线程消费 sequence_lp*_put_wafer，把片从LL放回原始FOUP槽位

		  下面这组容器是“配置来源/按LL分组的原始配置”（TOOL1≈LLA，TOOL2≈LLB）：
		  - sequence_tolk1_wafer / sequence_tolk2_wafer：从界面表格解析出的“要从某LP搬到某LL槽位”的配置行（按LL分组）
		  - *_copy：配置备份，用于循环/复位时快速恢复（避免重新读UI）
		*/

		std::vector<LPTransferWafer> sequence_tolk1_wafer;       // 半自动/自动：LLA(TOOL1) 相关的 LP→LL 配置行集合
		std::vector<LPTransferWafer> sequence_tolk2_wafer;       // 半自动/自动：LLB(TOOL2) 相关的 LP→LL 配置行集合
		std::vector<LPTransferWafer> sequence_tolk1_wafer_copy;  // LLA(TOOL1) 配置备份（循环/复位用）
		std::vector<LPTransferWafer> sequence_tolk2_wafer_copy;  // LLB(TOOL2) 配置备份（循环/复位用）

		/*
		  按LP分组的运行时队列（LP1/LP2各一套）：
		  - sequence_lp*_transfer_wafer：从表格解析出的 LP→LL 配置行（按LP分组，主要用于统计/循环恢复）
		  - sequence_lp*_get_wafer：LL上料阶段（ELP→LL）EFEM要“取FOUP”的待上料队列
		  - sequence_lp*_put_wafer：LL下料阶段（LL→ELP）EFEM要“回FOUP”的待下料队列
		*/
		std::vector<LPTransferWafer> sequence_lp1_transfer_wafer;
		std::vector<LPTransferWafer> sequence_lp1_put_wafer;
		std::vector<LPTransferWafer> sequence_lp1_get_wafer;

		std::vector<LPTransferWafer> sequence_lp2_transfer_wafer;
		std::vector<LPTransferWafer> sequence_lp2_put_wafer;
		std::vector<LPTransferWafer> sequence_lp2_get_wafer;

		/* 机械手任务队列的LP1子集（从表格解析得到，循环/复用时可重新拼回母队列） */
		std::vector<RobotTransferWafer> sequence_robot_transfer_wafer_lp1;

		/* 机械手任务队列的LP2子集（从表格解析得到） */
		std::vector<RobotTransferWafer> sequence_robot_transfer_wafer_lp2;

		/* LP1配置备份（循环/复位用） */
		std::vector<LPTransferWafer> sequence_lp1_transfer_wafer_copy;

		/* LP2配置备份（循环/复位用） */
		std::vector<LPTransferWafer> sequence_lp2_transfer_wafer_copy;

		/*
		  Robot母队列：cycle上配置的所有片（LLA/LLB混合），用于驱动 LL→PM 与 PM→LL 两段。
		  - LL→PM 放片到PM成功：push到 sequence_robot_get_wafer
		  - PM→LL 放回LL成功：从 sequence_robot_transfer_wafer 删除该项
		*/
		std::vector<RobotTransferWafer> sequence_robot_transfer_wafer;

		/* Robot待取回队列：PM中已放片、待从PM取回并放回指定LL的任务集合 */
		std::vector<RobotTransferWafer> sequence_robot_get_wafer;

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
		int loadlock1_auto_step = 0;
		int loadlock2_auto_step = 0;
		int vacuum_auto_step = 0;
		int pm_auto_step = 0;
		int efem_auto_step = 0;
		int update_auto_step = 0;

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

		bool loadlock1_put_cassette_finished = false;
		bool loadlock2_put_cassette_finished = false;

		bool loadlock1_process_finished = false;
		bool loadlock2_process_finished = false;


		bool pm_allow_get_put_wafer = false;
		bool pm_allow_goto_craft = false;

		bool loadlock1_allow_get_wafer = false;
		bool loadlock2_allow_get_wafer = false;
		bool loadlock1_allow_put_wafer = false;
		bool loadlock2_allow_put_wafer = false;

		bool tool1_allow_get_wafer = false;//true呼叫LP1上料，false LP1上料完成
		bool tool2_allow_get_wafer = false;//true呼叫LP2上料，false LP2上料完成
		bool tool1_allow_put_wafer = false;//true呼叫下料到LP1，false 下料到LP1完成
		bool tool2_allow_put_wafer = false;//true呼叫下料到LP2，false 下料到LP2完成

		/**
		true：LP1的当前晶圆已做完工艺
		*/
		bool lp1_cycle_one_time_finished = false;
		/**
		true：LP2的当前晶圆已做完工艺
		*/
		bool lp2_cycle_one_time_finished = false;

		bool is_lp2_cycle = false;
		bool is_lp1_cycle = false;

		bool lp1_box_closed_by_get = false;  //LP1上料关门已完成，防止重复关门
		bool lp2_box_closed_by_get = false;  //LP2上料关门已完成，防止重复关门
		bool lp1_box_closed_by_put = false;  //LP1下料关门已完成，防止重复关门
		bool lp2_box_closed_by_put = false;  //LP2下料关门已完成，防止重复关门

		bool hasUPS = false;
	public:
		
		//2026-5-19 半自动模式相关
		bool semi_auto_mode = false;

		enum class SemiAutoPhase : int { None = 0, LLLoad = 1, PMLoad = 2, PMUnload = 3, LLUnload = 4 };

		std::atomic<SemiAutoPhase> semi_auto_next_phase{ SemiAutoPhase::None };

		std::atomic<int> semi_auto_completed_phase{ 0 };

		struct SemiAutoRequest {
			int foup_slot = 1;
			QString elp = "ELP1";
			QString ll = "LLA";
			int ll_slot = 1;
			QString arm = "A";
			bool enable = true;
		};

		std::mutex semi_auto_mutex;
		SemiAutoRequest semi_auto_request;

		// 本次半自动序列是否涉及对应 LL（由 setTransferSequence 计算）；不涉及则该阶段不等待该腔完成
		std::atomic<bool> semi_auto_need_lla{ false };
		std::atomic<bool> semi_auto_need_llb{ false };
		// LL上料（ELP->LL）阶段：该腔是否已达到“允许 Robot 从 LL 取片”的完成点
		std::atomic<bool> semi_auto_lla_llload_done{ false };
		std::atomic<bool> semi_auto_llb_llload_done{ false };
		// LL下料（LL->ELP）阶段：该腔是否已完成回片到 ELP（对应 EFEM 清掉 tool*_allow_put_wafer）
		std::atomic<bool> semi_auto_lla_llunload_done{ false };
		std::atomic<bool> semi_auto_llb_llunload_done{ false };

		// 半自动模式：按钮分腔触发（位掩码：1=LLA 2=LLB 3=LLA|LLB）
		std::atomic<int> semi_auto_pm_load_target_mask{ 3 };
		std::atomic<int> semi_auto_ll_unload_target_mask{ 3 };

		std::atomic<bool> semi_auto_pm_processing{ false };
		std::atomic<bool> semi_auto_pm_unload_allowed{ false };

		std::atomic<int> return_wafer_count_lla{ 0 };
		std::atomic<int> return_wafer_count_llb{ 0 };


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

		// 重置 semi_auto_need_* 与各阶段 done 标志（启动新一轮半自动/重新配置序列时使用）
		void resetSemiAutoLLFlags();

		// LL上料阶段：涉及的 LLA/LLB 是否都完成（用于 PM上料按钮前置校验与阶段汇总）
		bool isSemiAutoLLLoadAllDone() const;

		// LL下料阶段：涉及的 LLA/LLB 是否都完成（用于阶段汇总，避免单腔完成就提前结束）
		bool isSemiAutoLLUnloadAllDone() const;

		// completed_phase 单调递增更新：多线程下只允许“更大阶段”覆盖“更小阶段”，避免回退/闪烁
		void updateSemiAutoCompletedPhase(const SemiAutoPhase phase);

		// next_phase 仅在“仍等于指定 phase”时才清回 None（CAS），防止覆盖用户点击的下一阶段
		void tryClearSemiAutoNextPhase(const SemiAutoPhase phase);

		// 某阶段完成后的统一收口：更新 completed_phase + 尝试清 next_phase
		void onSemiAutoStageDone(const SemiAutoPhase phase);

		// LL阶段完成上报：按 LLA/LLB 置 done；当涉及的 LL 都 done 后才触发阶段完成（LLLoad/LLUnload）
		void markSemiAutoLLDone(const SemiAutoPhase phase, const std::string& ll_name);

		void increaseReturnWaferCount(const std::string& ll_name, const int delta);

	};

	QSlotTransferCycleVTMWidgetPrivate::QSlotTransferCycleVTMWidgetPrivate(QSlotTransferCycleVTMWidget*p)
		:q_ptr(p){
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

		auto wtr = kernel ? kernel->getKernelModule<FortrendSunwayRobotSubsystem>("WTR") : nullptr;
		if (wtr){
			refreshCassetteColorDisplay(kernel.get(), wtr);
		}

		QMetaObject::invokeMethod(q_ptr, "update_cycle_data", Qt::AutoConnection);
	}

	static const char* semiAutoPhaseToString(QSlotTransferCycleVTMWidgetPrivate::SemiAutoPhase phase){
		switch (phase){
			case QSlotTransferCycleVTMWidgetPrivate::SemiAutoPhase::None: return "None";
			case QSlotTransferCycleVTMWidgetPrivate::SemiAutoPhase::LLLoad: return "LLLoad";
			case QSlotTransferCycleVTMWidgetPrivate::SemiAutoPhase::PMLoad: return "PMLoad";
			case QSlotTransferCycleVTMWidgetPrivate::SemiAutoPhase::PMUnload: return "PMUnload";
			case QSlotTransferCycleVTMWidgetPrivate::SemiAutoPhase::LLUnload: return "LLUnload";
			default: return "Unknown";
		}
	}

	void QSlotTransferCycleVTMWidgetPrivate::resetSemiAutoLLFlags(){
		semi_auto_need_lla.store(false);
		semi_auto_need_llb.store(false);
		semi_auto_lla_llload_done.store(false);
		semi_auto_llb_llload_done.store(false);
		semi_auto_lla_llunload_done.store(false);
		semi_auto_llb_llunload_done.store(false);
	}

	bool QSlotTransferCycleVTMWidgetPrivate::isSemiAutoLLLoadAllDone() const{
		const bool needA = semi_auto_need_lla.load();
		const bool needB = semi_auto_need_llb.load();
		const bool doneA = semi_auto_lla_llload_done.load();
		const bool doneB = semi_auto_llb_llload_done.load();
		return (!needA || doneA) && (!needB || doneB);
	}

	bool QSlotTransferCycleVTMWidgetPrivate::isSemiAutoLLUnloadAllDone() const{
		const bool needA = semi_auto_need_lla.load();
		const bool needB = semi_auto_need_llb.load();
		const bool doneA = semi_auto_lla_llunload_done.load();
		const bool doneB = semi_auto_llb_llunload_done.load();
		return (!needA || doneA) && (!needB || doneB);
	}

	//“半自动已完成到哪一阶段” 这个值更新到更靠后的阶段，但绝不允许回退
	void QSlotTransferCycleVTMWidgetPrivate::updateSemiAutoCompletedPhase(const SemiAutoPhase phase){
		const int target = (int)phase;
		int current = semi_auto_completed_phase.load();
		while (current < target){
			const int before = current;
			if (semi_auto_completed_phase.compare_exchange_weak(current, target)){
				logInform("Cycle", "SemiAuto completed_phase: %d(%s)->%d(%s)",
					before, semiAutoPhaseToString((SemiAutoPhase)before),
					target, semiAutoPhaseToString(phase));
				return;
			}
		}
	}

	void QSlotTransferCycleVTMWidgetPrivate::tryClearSemiAutoNextPhase(const SemiAutoPhase phase){
		SemiAutoPhase expected = phase;
		const bool cleared = semi_auto_next_phase.compare_exchange_strong(expected, SemiAutoPhase::None);
		if (cleared){
			logInform("Cycle", "SemiAuto next_phase cleared: %s->None", semiAutoPhaseToString(phase));
		}
		else{
			logInform("Cycle", "SemiAuto next_phase NOT cleared (expect=%s actual=%s)",
				semiAutoPhaseToString(phase), semiAutoPhaseToString(expected));
		}
	}

	void QSlotTransferCycleVTMWidgetPrivate::onSemiAutoStageDone(const SemiAutoPhase phase){
		if (!semi_auto_mode){
			return;
		}
		updateSemiAutoCompletedPhase(phase);
		tryClearSemiAutoNextPhase(phase);
	}

	void QSlotTransferCycleVTMWidgetPrivate::markSemiAutoLLDone(const SemiAutoPhase phase, const std::string& ll_name){
		if (!semi_auto_mode){
			return;
		}
		if (ll_name == "LLA"){
			if (phase == SemiAutoPhase::LLLoad){
				semi_auto_lla_llload_done.store(true);
			}
			else if (phase == SemiAutoPhase::LLUnload){
				semi_auto_lla_llunload_done.store(true);
			}
		}
		else if (ll_name == "LLB"){
			if (phase == SemiAutoPhase::LLLoad){
				semi_auto_llb_llload_done.store(true);
			}
			else if (phase == SemiAutoPhase::LLUnload){
				semi_auto_llb_llunload_done.store(true);
			}
		}

		if (phase == SemiAutoPhase::LLLoad){
			logInform("Cycle", "SemiAuto LLLoad done: ll=%s needLLA=%d needLLB=%d doneLLA=%d doneLLB=%d",
				ll_name.c_str(),
				semi_auto_need_lla.load(), semi_auto_need_llb.load(),
				semi_auto_lla_llload_done.load(), semi_auto_llb_llload_done.load());
			if (isSemiAutoLLLoadAllDone()){
				onSemiAutoStageDone(SemiAutoPhase::LLLoad);
			}
		}
		else if (phase == SemiAutoPhase::LLUnload){
			logInform("Cycle", "SemiAuto LLUnload done: ll=%s needLLA=%d needLLB=%d doneLLA=%d doneLLB=%d",
				ll_name.c_str(),
				semi_auto_need_lla.load(), semi_auto_need_llb.load(),
				semi_auto_lla_llunload_done.load(), semi_auto_llb_llunload_done.load());
			if (isSemiAutoLLUnloadAllDone()){
				onSemiAutoStageDone(SemiAutoPhase::LLUnload);
			}
		}
	}

	void QSlotTransferCycleVTMWidgetPrivate::increaseReturnWaferCount(const std::string& ll_name, const int delta){
		if (delta <= 0){
			return;
		}
		if (ll_name == "LLA"){
			return_wafer_count_lla.fetch_add(delta);
		}
		else if (ll_name == "LLB"){
			return_wafer_count_llb.fetch_add(delta);
		}
		onUpdateCycleInfo();
	}

	void QSlotTransferCycleVTMWidgetPrivate::resetAction(){
		if (running)
		{
			return;
		}
		while (!robot_step_once_finished || !vacumm_step_once_finished || !loadlock1_step_once_finished || !loadlock2_step_once_finished)
		{
			Sleep(100);
		}
		onUpdateControlEnabled("reset_pbt", false);
		onUpdateControlEnabled("execute_pbt", false);
		onUpdateControlEnabled("pause_pbt", false);

		std::shared_ptr<FortrendLoadLockSubsystem> lk1 = kernel->getKernelModule<FortrendLoadLockSubsystem>("LLA");
		std::shared_ptr<FortrendSunwayRobotSubsystem> wtr = kernel->getKernelModule<FortrendSunwayRobotSubsystem>("WTR");
		std::shared_ptr<FortrendTMCavitySubsystem> tm = kernel->getKernelModule<FortrendTMCavitySubsystem>("TM");
		//std::shared_ptr<FortrendPMCavitySubsystem> pm1 = kernel->getKernelModule<FortrendPMCavitySubsystem>("PM1");
		std::shared_ptr<FortrendPMCavitySubsystem> pm2 = kernel->getKernelModule<FortrendPMCavitySubsystem>("PM");
		//std::shared_ptr<FortrendPMCavitySubsystem> pm3 = kernel->getKernelModule<FortrendPMCavitySubsystem>("PM3");
		std::shared_ptr<FortrendLoadLockSubsystem> lk2 = kernel->getKernelModule<FortrendLoadLockSubsystem>("LLB");

		//std::shared_ptr<FortrendAlignerSubsystem> aligner = kernel->getKernelModule<FortrendAlignerSubsystem>("Aligner");

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
					(ewtr->getState() != IKernelSubSystem::State::SUB_UNKNOWN || ui->disabledefem->checkState() == Qt::CheckState::Checked)
					)
				{
					logInform(reset_process_name.c_str(), "整机复位开始");

					/*auto actSub = wtr->getKernel()->getKernelModule<KernelActionSubsytem>();
					std::shared_ptr<KernelParallelAction> WTRandSMIFReset;
					WTRandSMIFReset.reset(new KernelParallelAction("WTRandSMIFReset"));
					WTRandSMIFReset->addCommand(wtr, wtr->createResetCommand());
					WTRandSMIFReset->addCommand(smif1, smif1->createResetCommand());
					WTRandSMIFReset->addCommand(smif2, smif2->createResetCommand());*/


					auto cmd = wtr->createResetCommand();
					wtr->startCommand(cmd);
					
					if (ui->disabledefem->checkState() == Qt::CheckState::Unchecked){
						auto cmd_ewtr = ewtr->createResetCommand();
						ewtr->startCommand(cmd_ewtr);
						cmd_ewtr->wait();
						if (cmd_ewtr->hasError()){
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

					/*auto cmd_pm1 = pm1->createResetCommand();
					pm1->startCommand(cmd_pm1);*/

					auto cmd_pm2 = pm2->createResetCommand();
					pm2->startCommand(cmd_pm2);

					/*auto cmd_pm3 = pm3->createResetCommand();
					pm3->startCommand(cmd_pm3);*/

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
					//cmd_pm1->wait();
					cmd_pm2->wait();
					//cmd_pm3->wait();
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
					/*else if (cmd_pm1->hasError())
					{
					rest_step = 15000;
					logError(reset_process_name.c_str(), Poco::format("%s复位指令执行失败！", pm1->getName()).c_str());
					}*/
					else if (cmd_pm2->hasError())
					{
						rest_step = 15000;
						logError(reset_process_name.c_str(), Poco::format("%s复位指令执行失败！", pm2->getName()).c_str());
					}
					/*else if (cmd_pm3->hasError())
					{
					rest_step = 15000;
					logError(reset_process_name.c_str(), Poco::format("%s复位指令执行失败！", pm3->getName()).c_str());
					}*/
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
					/*if (pm1->getState() == IKernelSubSystem::State::SUB_UNKNOWN)
					{
					logError(reset_process_name.c_str(), Poco::format("%s不在正常状态", pm1->getName()).c_str());
					}*/
					if (pm2->getState() == IKernelSubSystem::State::SUB_UNKNOWN)
					{
						logError(reset_process_name.c_str(), Poco::format("%s不在正常状态", pm2->getName()).c_str());
					}
					/*if (pm3->getState() == IKernelSubSystem::State::SUB_UNKNOWN)
					{
					logError(reset_process_name.c_str(), Poco::format("%s不在正常状态", pm3->getName()).c_str());
					}*/
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
					//pm1->getState() == IKernelSubSystem::State::SUB_NORMAL &&
					pm2->getState() == IKernelSubSystem::State::SUB_NORMAL &&
					//pm3->getState() == IKernelSubSystem::State::SUB_NORMAL &&
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
					/*if (pm1->getState() != IKernelSubSystem::State::SUB_NORMAL)
					{
					logError(reset_process_name.c_str(), Poco::format("%s不在正常状态", pm1->getName()).c_str());
					}*/
					if (pm2->getState() != IKernelSubSystem::State::SUB_NORMAL)
					{
						logError(reset_process_name.c_str(), Poco::format("%s不在正常状态", pm2->getName()).c_str());
					}
					/*if (pm3->getState() != IKernelSubSystem::State::SUB_NORMAL)
					{
					logError(reset_process_name.c_str(), Poco::format("%s不在正常状态", pm3->getName()).c_str());
					}*/
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
				//auto cmd_pm1 = pm1->createCloseTMCavityDoorCommand();
				auto cmd_pm2 = pm2->createCloseTMCavityDoorCommand();
				//auto cmd_pm3 = pm3->createCloseTMCavityDoorCommand();

				lk1->startCommand(cmd_lk1);
				lk2->startCommand(cmd_lk2);
				//pm1->startCommand(cmd_pm1);
				pm2->startCommand(cmd_pm2);
				//pm3->startCommand(cmd_pm3);

				cmd_lk1->wait();
				cmd_lk2->wait();
				//cmd_pm1->wait();
				cmd_pm2->wait();
				//cmd_pm3->wait();

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
				/*else if (cmd_pm1->hasError())
				{
				rest_step = 15000;
				logError(reset_process_name.c_str(), Poco::format("%s关闭传输腔门阀命令执行失败！", pm1->getName()).c_str());
				}*/
				else if (cmd_pm2->hasError())
				{
					rest_step = 15000;
					logError(reset_process_name.c_str(), Poco::format("%s关闭传输腔门阀命令执行失败！", pm2->getName()).c_str());
				}
				/*else if (cmd_pm3->hasError())
				{
				rest_step = 15000;
				logError(reset_process_name.c_str(), Poco::format("%s关闭传输腔门阀命令执行失败！", pm3->getName()).c_str());
				}*/
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
		int efem_slot_offset = 30;
		int wafer_model = lk1->getWaferModel();
		if (wafer_model == 1) {
			efem_slot_offset = 30;
		} else if (wafer_model == 2) {
			efem_slot_offset = 33;
		}
		while (running)
		{
			efem_step_once_finished = false;
			switch (efem_auto_step)
			{
			case 10:
			{
				if (tool1_allow_get_wafer)//给tool1单上料，不考虑交换料，前面4层
				{
					efem_auto_step = 100;

					if(ui->disabledefem->checkState() == Qt::CheckState::Checked)
						efem_auto_step = 3000;
				}
				else if (tool1_allow_put_wafer)//给tool1下料
				{
					efem_auto_step = 200;
					if (ui->disabledefem->checkState() == Qt::CheckState::Checked)
						efem_auto_step = 4000;
				}
				else if (tool2_allow_get_wafer)//给tool2上料
				{
					efem_auto_step = 1000;
					if (ui->disabledefem->checkState() == Qt::CheckState::Checked)
						efem_auto_step = 5000;
				}
				else if (tool2_allow_put_wafer)//给tool1下料
				{
					efem_auto_step = 2000;
					if (ui->disabledefem->checkState() == Qt::CheckState::Checked)
						efem_auto_step = 6000;
				}
				else
				{
					Sleep(10);
				}
			}
			break;
			#pragma region 给TOOL1上料,先判断是哪个LP
			case 100:
			{
				if (sequence_tolk1_wafer.size()>0){
					if (sequence_tolk1_wafer[0].source_lp=="ELP1"){
						if (elp1->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
							if (!elp1->hasDoorOpend()){
								auto cmd = elp1->createOpenBoxCommand();
								elp1->startCommand(cmd);
								cmd->wait();
								if (cmd->hasError())
								{
									logFailedExcuteCommandHasError(elp1->getName(), "打开晶圆盒", efem_process_name, efem_auto_step);
								}
								else{
									efem_auto_step = 105;
								}
							}
							else{
								auto cmd = elp1->createGetMapCommand();
								elp1->startCommand(cmd);
								cmd->wait();
								if (cmd->hasError())
								{
									logFailedExcuteCommandHasError(elp1->getName(), "获取MAP", efem_process_name, efem_auto_step);
								}
								else{
									efem_auto_step = 105;
								}
							}
						}
						else{
							logFailedNotNormal(elp1->getName(), efem_process_name, efem_auto_step);
						}
					}
					else if (sequence_tolk1_wafer[0].source_lp == "ELP2"){
						if (elp2->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
							if (!elp2->hasDoorOpend()){
								auto cmd = elp2->createOpenBoxCommand();
								elp2->startCommand(cmd);
								cmd->wait();
								if (cmd->hasError())
								{
									logFailedExcuteCommandHasError(elp2->getName(), "打开晶圆盒", efem_process_name, efem_auto_step);
								}
								else{
									efem_auto_step = 105;
								}
							}
							else{
								auto cmd = elp2->createGetMapCommand();
								elp2->startCommand(cmd);
								cmd->wait();
								if (cmd->hasError())
								{
									logFailedExcuteCommandHasError(elp2->getName(), "获取MAP", efem_process_name, efem_auto_step);
								}
								else{
									efem_auto_step = 105;
								}
							}
						}
						else{
							logFailedNotNormal(elp2->getName(), efem_process_name, efem_auto_step);
						}
					}
					else{
						Sleep(50);
					}
				}
				else{
					Sleep(50);
				}

				
				
			}
			break;
			case 105:
			{
				auto cassManager = lk1->getKernel()->getKernelModule<FortrendCassetteManager>();
				auto station_cass_lk = cassManager->getCassette(lk1.get());
				auto lkmaps = station_cass_lk->getAllMapping();//查看
				for (int i = 0; i < lkmaps.size(); i++)
				{
					if (lkmaps[i] == Cassette::Empty){
						if (sequence_tolk1_wafer.size()>0){
							LPTransferWafer getlp;
							getlp.is_finish_putlk = false;
							getlp.slot_lk = sequence_tolk1_wafer[0].slot_lk;
							getlp.slot_lp = sequence_tolk1_wafer[0].slot_lp;
							getlp.transfer = sequence_tolk1_wafer[0].transfer;
							getlp.selected_arm = sequence_tolk1_wafer[0].selected_arm;
							getlp.source_lp = sequence_tolk1_wafer[0].source_lp;
							sequence_lp1_get_wafer.push_back(getlp);
							sequence_tolk1_wafer.erase(sequence_tolk1_wafer.begin());
						}
					}
				}
				/*int count = lkmaps.size() - sequence_lp1_get_wafer.size();
				if (count != 0 && sequence_lp2_transfer_wafer.size()>0){
				for (int i = 0; i < count; i++)
				{
				int key = sequence_lp1_get_wafer.size();
				if (lkmaps[key] == Cassette::Empty){
				if (sequence_lp2_transfer_wafer.size()>i){
				LPTransferWafer getlp;
				getlp.is_finish_putlk = false;
				getlp.slot_lk = sequence_lp2_transfer_wafer[i].slot_lk;
				getlp.slot_lp = sequence_lp2_transfer_wafer[i].slot_lp;
				getlp.transfer = sequence_lp2_transfer_wafer[i].transfer;
				getlp.selected_arm = sequence_lp2_transfer_wafer[i].selected_arm;
				getlp.source_lp = sequence_lp2_transfer_wafer[i].source_lp;
				sequence_lp1_get_wafer.push_back(getlp);
				}
				}
				}
				}*/
				efem_auto_step = 110;
			}
			break;
			case 110:
			{
				if (sequence_lp1_get_wafer.size() == 1){
					efem_auto_step = 115;
				}
				else if (sequence_lp1_get_wafer.size() >= 2){
					efem_auto_step = 150;
				}
				else{//上料完成
					//2026-4-22  只有当lp中的晶圆下料完成，才去关闭晶圆盒门
#if 0
					if (elp1&&elp1->getState() == IKernelSubSystem::State::SUB_NORMAL){
						if (elp1->hasDoorOpend()){
							auto cmd = elp1->createCloseBoxCommand();
							elp1->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError())
							{
								logFailedExcuteCommandHasError(elp1->getName(), "关闭晶圆盒", efem_process_name, efem_auto_step);
							}
						}
					}
					else{
						logFailedNotNormal(elp1->getName(), efem_process_name, efem_auto_step);
					}

					if (elp2&&elp2->getState() == IKernelSubSystem::State::SUB_NORMAL){
						if (elp2->hasDoorOpend()){
							auto cmd = elp2->createCloseBoxCommand();
							elp2->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError())
							{
								logFailedExcuteCommandHasError(elp2->getName(), "关闭晶圆盒", efem_process_name, efem_auto_step);
							}
						}
					}
					else{
						logFailedNotNormal(elp1->getName(), efem_process_name, efem_auto_step);
					}


					if (!elp2->hasDoorOpend() && !elp1->hasDoorOpend()){
						tool1_allow_get_wafer = false;
						efem_auto_step = 10;
					}
#endif
					tool1_allow_get_wafer = false;
					efem_auto_step = 10;
				}
			}
			break;
#pragma region 单取单放LK1
			case 115:
			{
				std::shared_ptr<EFEMLPSubsystem> elp = kernel->getKernelModule<EFEMLPSubsystem>(sequence_lp1_get_wafer[0].source_lp);
				if (elp&& elp->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					if (!elp->hasDoorOpend()){
						auto cmd = elp->createOpenBoxCommand();
						elp->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(elp->getName(), "打开晶圆盒", efem_process_name, efem_auto_step);
						}
						else{
							efem_auto_step = 120;
						}
					}
					else{
						efem_auto_step = 120;
					}
				}
				else{
					logFailedNotNormal(elp->getName(), efem_process_name, efem_auto_step);
				}
				
			}
			break;
			case 120://LP1上料
			{
				if (lk1&& lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					auto cmd1 = lk1->createMoveToSlotCommand(sequence_lp1_get_wafer[0].slot_lk + efem_slot_offset);
					lk1->startCommand(cmd1);
					cmd1->wait();
					if (cmd1->hasError())
					{
						logFailedExcuteCommandHasError(lk1->getName(), "LLA移动到slot", efem_process_name, efem_auto_step);
					}
					else
					{
						efem_auto_step = 125;
					}
				}
				else{
					logFailedNotNormal(lk1->getName(), efem_process_name, efem_auto_step);
				}
				

			}
			break;
			case 125://LP1上料
			{
				std::shared_ptr<EFEMLPSubsystem> elp = kernel->getKernelModule<EFEMLPSubsystem>(sequence_lp1_get_wafer[0].source_lp);
				if (elp&& elp->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					auto cmd = ewtr->createGetCommand(elp, 1, sequence_lp1_get_wafer[0].slot_lp);
					ewtr->startCommand(cmd);
					cmd->wait();
					if (cmd->hasError())
					{
						logFailedExcuteCommandHasError(ewtr->getName(), "取LP1晶圆", efem_process_name, efem_auto_step);
					}
					else
					{
						efem_auto_step = 130;
					}
				}
				else{
					logFailedNotNormal(elp->getName(), efem_process_name, efem_auto_step);
				}

			}
			break;
			case 130://LP1上料
			{
				if (lk1&& lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					if (!lk1->getCassetteDoorOpend()){
						auto cmdopendoor = lk1->createOpenCassetteDoorCommand();
						lk1->startCommand(cmdopendoor);
						cmdopendoor->wait();
						if (cmdopendoor->hasError())
						{
							logFailedExcuteCommandHasError(lk1->getName(), "LLA打开晶圆盒门", efem_process_name, efem_auto_step);
						}
					}
					else{
						Sleep(2000);
						efem_auto_step = 135;
					}
					
				}
				else{
					logFailedNotNormal(lk1->getName(), efem_process_name, efem_auto_step);
				}
			}
			break;
			case 135://LP1上料
			{
				if (ewtr&& ewtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					auto cmd2 = ewtr->createPutCommand(lk1, 1, sequence_lp1_get_wafer[0].slot_lk);
					ewtr->startCommand(cmd2);
					cmd2->wait();
					if (cmd2->hasError())
					{
						logFailedExcuteCommandHasError(ewtr->getName(), "放LLA晶圆", efem_process_name, efem_auto_step);
					}
					else{//放料完成
						LoadLockTransferWafer lktw;
						lktw.is_finish = false;
						lktw.slot = sequence_lp1_get_wafer[0].slot_lk;
						lktw.transfer = sequence_lp1_get_wafer[0].transfer;
						lktw.selected_arm = sequence_lp1_get_wafer[0].selected_arm;
						sequence_loadlock1_transfer_wafer.push_back(lktw);
						sequence_lp1_put_wafer.push_back(sequence_lp1_get_wafer.front());//提前存入放料集合
						sequence_lp1_get_wafer.erase(sequence_lp1_get_wafer.begin());
						efem_auto_step = 110;
					}
				}
				else{
					logFailedNotNormal(ewtr->getName(), efem_process_name, efem_auto_step);
				}
			}
			break;

			//case 140://单取单放LK1
			//{
			//	std::shared_ptr<EFEMLPSubsystem> elp = kernel->getKernelModule<EFEMLPSubsystem>(sequence_lp1_get_wafer[0].source_lp);
			//	if (ewtr&&elp&&lk1 && ewtr->getState() == IKernelSubSystem::State::SUB_NORMAL && elp->getState() == IKernelSubSystem::State::SUB_NORMAL&& lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
			//	{
			//		if (!elp->hasDoorOpend()){
			//			auto cmd = elp->createOpenBoxCommand();
			//			elp->startCommand(cmd);
			//			cmd->wait();
			//			if (cmd->hasError())
			//			{
			//				logFailedExcuteCommandHasError(elp->getName(), "打开晶圆盒", efem_process_name, efem_auto_step);
			//			}
			//		}
			//		int slot = sequence_lp1_get_wafer[0].slot_lp;
			//		int slotlk = sequence_lp1_get_wafer[0].slot_lk;
			//		auto cmd1 = lk1->createMoveToSlotCommand(slotlk + 30);
			//		lk1->startCommand(cmd1);
			//		cmd1->wait();
			//		if (cmd1->hasError())
			//		{
			//			logFailedExcuteCommandHasError(lk1->getName(), "LLA移动到slot", efem_process_name, efem_auto_step);
			//		}
			//		else
			//		{//取完料
			//			auto cmd = ewtr->createGetCommand(elp, 1, slot);
			//			ewtr->startCommand(cmd);
			//			cmd->wait();
			//			if (cmd->hasError())
			//			{
			//				logFailedExcuteCommandHasError(ewtr->getName(), "取LP1晶圆", efem_process_name, efem_auto_step);
			//			}
			//			else
			//			{//移动到SLOT完成
			//				if (!lk1->getCassetteDoorOpend()){
			//					auto cmdopendoor = lk1->createOpenCassetteDoorCommand();
			//					lk1->startCommand(cmdopendoor);
			//					cmdopendoor->wait();
			//					if (cmdopendoor->hasError())
			//					{
			//						logFailedExcuteCommandHasError(lk1->getName(), "LLA打开晶圆盒门", efem_process_name, efem_auto_step);
			//					}
			//				}
			//				Sleep(2000);
			//				auto cmd2 = ewtr->createPutCommand(lk1, 1, slotlk);
			//				ewtr->startCommand(cmd2);
			//				cmd2->wait();
			//				if (cmd2->hasError())
			//				{
			//					logFailedExcuteCommandHasError(ewtr->getName(), "放LLA晶圆", efem_process_name, efem_auto_step);
			//				}
			//				else{//放料完成
			//					LoadLockTransferWafer lktw;
			//					lktw.is_finish = false;
			//					lktw.slot = slotlk;
			//					lktw.transfer = sequence_lp1_get_wafer[0].transfer;
			//					lktw.selected_arm = sequence_lp1_get_wafer[0].selected_arm;
			//					sequence_loadlock1_transfer_wafer.push_back(lktw);
			//					sequence_lp1_put_wafer.push_back(sequence_lp1_get_wafer.front());//提前存入放料集合
			//					sequence_lp1_get_wafer.erase(sequence_lp1_get_wafer.begin());
			//					efem_auto_step = 110;
			//				}
			//			}
			//		}
			//		
			//	}
			//	else
			//	{
			//		logFailedNotNormal(elp->getName(), efem_process_name, efem_auto_step);
			//	}
			//}
			//break;
#pragma endregion
#pragma region 双取LP1双放LK1
			case 150:
			{
				std::shared_ptr<EFEMLPSubsystem> elp1get = kernel->getKernelModule<EFEMLPSubsystem>(sequence_lp1_get_wafer[0].source_lp);
				if (elp1get&& elp1get->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					if (!elp1get->hasDoorOpend()){
						auto cmd = elp1get->createOpenBoxCommand();
						elp1get->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(elp1get->getName(), "打开晶圆盒", efem_process_name, efem_auto_step);
						}
						else{
							efem_auto_step = 151;
						}
					}
					else{
						efem_auto_step = 151;
					}
				}
				else{
					logFailedNotNormal(elp1get->getName(), efem_process_name, efem_auto_step);
				}

			}
			break;
			case 151:
			{
				std::shared_ptr<EFEMLPSubsystem> elp1get = kernel->getKernelModule<EFEMLPSubsystem>(sequence_lp1_get_wafer[1].source_lp);
				if (elp1get&& elp1get->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					if (!elp1get->hasDoorOpend()){
						auto cmd = elp1get->createOpenBoxCommand();
						elp1get->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(elp1get->getName(), "打开晶圆盒", efem_process_name, efem_auto_step);
						}
						else{
							efem_auto_step = 152;
						}
					}
					else{
						efem_auto_step = 152;
					}
				}
				else{
					logFailedNotNormal(elp1get->getName(), efem_process_name, efem_auto_step);
				}
			}
			break;
			case 152://LP1上料
			{
				if (lk1&& lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					auto cmd1 = lk1->createMoveToSlotCommand(sequence_lp1_get_wafer[0].slot_lk + efem_slot_offset);
					lk1->startCommand(cmd1);
					cmd1->wait();
					if (cmd1->hasError())
					{
						logFailedExcuteCommandHasError(lk1->getName(), "LLA移动到slot", efem_process_name, efem_auto_step);
					}
					else
					{
						efem_auto_step = 153;
					}
				}
				else{
					logFailedNotNormal(lk1->getName(), efem_process_name, efem_auto_step);
				}
			}
			break;
			case 153://LP1上料
			{
				std::shared_ptr<EFEMLPSubsystem> elp = kernel->getKernelModule<EFEMLPSubsystem>(sequence_lp1_get_wafer[0].source_lp);
				if (elp&& elp->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					auto cmd = ewtr->createGetCommand(elp, 1, sequence_lp1_get_wafer[0].slot_lp);
					ewtr->startCommand(cmd);
					cmd->wait();
					if (cmd->hasError())
					{
						logFailedExcuteCommandHasError(ewtr->getName(), "A手臂取LP1晶圆", efem_process_name, efem_auto_step);
					}
					else
					{
						efem_auto_step = 154;
					}
				}
				else{
					logFailedNotNormal(elp->getName(), efem_process_name, efem_auto_step);
				}

			}
			break;
			case 154://LP1上料
			{
				std::shared_ptr<EFEMLPSubsystem> elp = kernel->getKernelModule<EFEMLPSubsystem>(sequence_lp1_get_wafer[1].source_lp);
				if (elp&& elp->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					auto cmd = ewtr->createGetCommand(elp, 2, sequence_lp1_get_wafer[1].slot_lp);
					ewtr->startCommand(cmd);
					cmd->wait();
					if (cmd->hasError())
					{
						logFailedExcuteCommandHasError(ewtr->getName(), "B手臂取LP1晶圆", efem_process_name, efem_auto_step);
					}
					else
					{
						efem_auto_step = 155;
					}
				}
				else{
					logFailedNotNormal(elp->getName(), efem_process_name, efem_auto_step);
				}

			}
			break;
			case 155://LP1上料
			{
				if (lk1&& lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					if (!lk1->getCassetteDoorOpend()){
						auto cmdopendoor = lk1->createOpenCassetteDoorCommand();
						lk1->startCommand(cmdopendoor);
						cmdopendoor->wait();
						if (cmdopendoor->hasError())
						{
							logFailedExcuteCommandHasError(lk1->getName(), "LLA打开晶圆盒门", efem_process_name, efem_auto_step);
						}
					}
					else{
						Sleep(2000);
						efem_auto_step = 157;
					}

				}
				else{
					logFailedNotNormal(lk1->getName(), efem_process_name, efem_auto_step);
				}
			}
			break;
			case 157://LP1上料
			{
				if (ewtr&& ewtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					auto cmd2 = ewtr->createPutCommand(lk1, 1, sequence_lp1_get_wafer[0].slot_lk);
					ewtr->startCommand(cmd2);
					cmd2->wait();
					if (cmd2->hasError())
					{
						logFailedExcuteCommandHasError(ewtr->getName(), "A手臂放LLA晶圆", efem_process_name, efem_auto_step);
					}
					else{//放料完成
						LoadLockTransferWafer lktw1;
						lktw1.is_finish = false;
						lktw1.slot = sequence_lp1_get_wafer[0].slot_lk;
						lktw1.transfer = sequence_lp1_get_wafer[0].transfer;
						sequence_loadlock1_transfer_wafer.push_back(lktw1);
						efem_auto_step = 158;
					}
				}
				else{
					logFailedNotNormal(ewtr->getName(), efem_process_name, efem_auto_step);
				}
			}
			break;
			case 158://LP1上料
			{
				if (lk1&& lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					auto cmd1 = lk1->createMoveToSlotCommand(sequence_lp1_get_wafer[1].slot_lk + efem_slot_offset);
					lk1->startCommand(cmd1);
					cmd1->wait();
					if (cmd1->hasError())
					{
						logFailedExcuteCommandHasError(lk1->getName(), "LLA移动到slot", efem_process_name, efem_auto_step);
					}
					else
					{
						Sleep(2000);
						efem_auto_step = 159;
					}
				}
				else{
					logFailedNotNormal(lk1->getName(), efem_process_name, efem_auto_step);
				}
			}
			break;
			case 159://LP1上料
			{
				if (ewtr&& ewtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					auto cmd2 = ewtr->createPutCommand(lk1, 2, sequence_lp1_get_wafer[1].slot_lk);
					ewtr->startCommand(cmd2);
					cmd2->wait();
					if (cmd2->hasError())
					{
						logFailedExcuteCommandHasError(ewtr->getName(), "B手臂放LLA晶圆", efem_process_name, efem_auto_step);
					}
					else{//放料完成
						LoadLockTransferWafer lktw2;
						lktw2.is_finish = false;
						lktw2.slot = sequence_lp1_get_wafer[1].slot_lk;
						lktw2.transfer = sequence_lp1_get_wafer[1].transfer;
						lktw2.selected_arm = sequence_lp1_get_wafer[1].selected_arm;
						sequence_loadlock1_transfer_wafer.push_back(lktw2);
						sequence_lp1_put_wafer.push_back(sequence_lp1_get_wafer.front());//提前存入放料集合
						sequence_lp1_get_wafer.erase(sequence_lp1_get_wafer.begin());
						sequence_lp1_put_wafer.push_back(sequence_lp1_get_wafer.front());//提前存入放料集合
						sequence_lp1_get_wafer.erase(sequence_lp1_get_wafer.begin());
						efem_auto_step = 110;
					}
				}
				else{
					logFailedNotNormal(ewtr->getName(), efem_process_name, efem_auto_step);
				}
			}
			break;
			//case 150://双取LP1双放LK1
			//{
			//	std::shared_ptr<EFEMLPSubsystem> elp1get = kernel->getKernelModule<EFEMLPSubsystem>(sequence_lp1_get_wafer[0].source_lp);
			//	std::shared_ptr<EFEMLPSubsystem> elp2get = kernel->getKernelModule<EFEMLPSubsystem>(sequence_lp1_get_wafer[1].source_lp);
			//	if (ewtr&&elp1get&&elp2get&&lk1 && ewtr->getState() == IKernelSubSystem::State::SUB_NORMAL
			//		&& elp1get->getState() == IKernelSubSystem::State::SUB_NORMAL
			//		&& elp2get->getState() == IKernelSubSystem::State::SUB_NORMAL
			//		&& lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
			//	{
			//		if (!elp1get->hasDoorOpend()){
			//			auto cmd = elp1get->createOpenBoxCommand();
			//			elp1get->startCommand(cmd);
			//			cmd->wait();
			//			if (cmd->hasError())
			//			{
			//				logFailedExcuteCommandHasError(elp1get->getName(), "打开晶圆盒", efem_process_name, efem_auto_step);
			//			}
			//		}
			//		if (!elp2get->hasDoorOpend()){
			//			auto cmd = elp2get->createOpenBoxCommand();
			//			elp2get->startCommand(cmd);
			//			cmd->wait();
			//			if (cmd->hasError())
			//			{
			//				logFailedExcuteCommandHasError(elp2get->getName(), "打开晶圆盒", efem_process_name, efem_auto_step);
			//			}
			//		}
			//		int slot = sequence_lp1_get_wafer[0].slot_lp;
			//		int slotlk = sequence_lp1_get_wafer[0].slot_lk;
			//		int slot2 = sequence_lp1_get_wafer[1].slot_lp;
			//		int slot2lk = sequence_lp1_get_wafer[1].slot_lk;
			//		auto cmd2 = lk1->createMoveToSlotCommand(slotlk + 30);
			//		lk1->startCommand(cmd2);
			//		cmd2->wait();
			//		if (cmd2->hasError())
			//		{
			//			logFailedExcuteCommandHasError(lk1->getName(), "LLA移动到slot", efem_process_name, efem_auto_step);
			//		}
			//		else
			//		{//1爪取完料
			//			auto cmd = ewtr->createGetCommand(elp1get, 1, slot);
			//			ewtr->startCommand(cmd);
			//			cmd->wait();
			//			if (cmd->hasError())
			//			{
			//				logFailedExcuteCommandHasError(ewtr->getName(), "A手臂取LP1晶圆", efem_process_name, efem_auto_step);
			//			}
			//			else
			//			{//2爪取完料
			//				auto cmd1 = ewtr->createGetCommand(elp2get, 2, slot2);
			//				ewtr->startCommand(cmd1);
			//				cmd1->wait();
			//				if (cmd1->hasError())
			//				{
			//					logFailedExcuteCommandHasError(ewtr->getName(), "B手臂取LP1晶圆", efem_process_name, efem_auto_step);
			//				}
			//				else
			//				{//移动到SLOT完成
			//					if (!lk1->getCassetteDoorOpend()){
			//						auto cmdopendoor = lk1->createOpenCassetteDoorCommand();
			//						lk1->startCommand(cmdopendoor);
			//						cmdopendoor->wait();
			//						if (cmdopendoor->hasError())
			//						{
			//							logFailedExcuteCommandHasError(lk1->getName(), "LLA打开晶圆盒门", efem_process_name, efem_auto_step);
			//						}
			//					}
			//					Sleep(2000);
			//					auto cmd3 = ewtr->createPutCommand(lk1, 1, slotlk);
			//					ewtr->startCommand(cmd3);
			//					cmd3->wait();
			//					if (cmd3->hasError())
			//					{
			//						logFailedExcuteCommandHasError(ewtr->getName(), "A手臂放LLA晶圆", efem_process_name, efem_auto_step);
			//					}
			//					else
			//					{//1爪放料完成
			//						LoadLockTransferWafer lktw1;
			//						lktw1.is_finish = false;
			//						lktw1.slot = sequence_lp1_get_wafer[0].slot_lk;
			//						lktw1.transfer = sequence_lp1_get_wafer[0].transfer;
			//						sequence_loadlock1_transfer_wafer.push_back(lktw1);
			//						auto cmd4 = lk1->createMoveToSlotCommand(slot2lk + 30);
			//						lk1->startCommand(cmd4);
			//						cmd4->wait();
			//						if (cmd4->hasError())
			//						{
			//							logFailedExcuteCommandHasError(lk1->getName(), "LLA移动到slot", efem_process_name, efem_auto_step);
			//						}
			//						else
			//						{//移动到SLOT完成
			//							Sleep(2000);
			//							auto cmd5 = ewtr->createPutCommand(lk1, 2, slot2lk);
			//							ewtr->startCommand(cmd5);
			//							cmd5->wait();
			//							if (cmd5->hasError())
			//							{
			//								logFailedExcuteCommandHasError(ewtr->getName(), "B手臂放LLA晶圆", efem_process_name, efem_auto_step);
			//							}
			//							else{
			//								LoadLockTransferWafer lktw2;
			//								lktw2.is_finish = false;
			//								lktw2.slot = sequence_lp1_get_wafer[1].slot_lk;
			//								lktw2.transfer = sequence_lp1_get_wafer[1].transfer;
			//								lktw2.selected_arm = sequence_lp1_get_wafer[1].selected_arm;
			//								sequence_loadlock1_transfer_wafer.push_back(lktw2);
			//								sequence_lp1_put_wafer.push_back(sequence_lp1_get_wafer.front());//提前存入放料集合
			//								sequence_lp1_get_wafer.erase(sequence_lp1_get_wafer.begin());
			//								sequence_lp1_put_wafer.push_back(sequence_lp1_get_wafer.front());//提前存入放料集合
			//								sequence_lp1_get_wafer.erase(sequence_lp1_get_wafer.begin());
			//								efem_auto_step = 110;
			//							}
			//						}
			//					}
			//				}
			//			}
			//		}
			//	}
			//	else
			//	{
			//		logFailedNotNormal(elp1get->getName(), efem_process_name, efem_auto_step);
			//	}
			//}
			//break;
#pragma endregion
			
			#pragma endregion
            #pragma region 给TOOL1下料
			case 200:
			{
				std::shared_ptr<EFEMLPSubsystem> elp = kernel->getKernelModule<EFEMLPSubsystem>(sequence_lp1_put_wafer[0].source_lp);
				if (elp&&elp->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					if (!elp->hasDoorOpend()){
						auto cmd = elp->createOpenBoxCommand();
						elp->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(elp->getName(), "打开晶圆盒", efem_process_name, efem_auto_step);
						}
						else{
							efem_auto_step = 201;
						}
					}
					else{
						auto cmd = elp->createGetMapCommand();
						elp->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(elp->getName(), "获取MAP", efem_process_name, efem_auto_step);
						}
						else{
							efem_auto_step = 201;
						}
					}
				}

			}
			break;
			case 201:
			{
				if (sequence_lp1_put_wafer.size() == 1){
					efem_auto_step = 240;
				}
				else if (sequence_lp1_put_wafer.size() >= 2){
					efem_auto_step = 250;
				}
				else{//下料完成
					if (elp1&&elp1->getState() == IKernelSubSystem::State::SUB_NORMAL){
						if (elp1->hasDoorOpend()){
							auto cmd = elp1->createCloseBoxCommand();
							elp1->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError())
							{
								logFailedExcuteCommandHasError(elp1->getName(), "关闭晶圆盒", efem_process_name, efem_auto_step);
							}
						}

					}
					else{
						logFailedNotNormal(elp1->getName(), efem_process_name, efem_auto_step);
					}

					if (elp2&&elp2->getState() == IKernelSubSystem::State::SUB_NORMAL){
						if (elp2->hasDoorOpend()){
							auto cmd = elp2->createCloseBoxCommand();
							elp2->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError())
							{
								logFailedExcuteCommandHasError(elp2->getName(), "关闭晶圆盒", efem_process_name, efem_auto_step);
							}
						}
					}
					else{
						logFailedNotNormal(elp1->getName(), efem_process_name, efem_auto_step);
					}

					if (!elp2->hasDoorOpend() && !elp1->hasDoorOpend()){
						tool1_allow_put_wafer = false;
						if (semi_auto_mode)
						{
							markSemiAutoLLDone(SemiAutoPhase::LLUnload, "LLA");
						}
						efem_auto_step = 10;
					}

				}
			}
			break;
#pragma region 单取LK1单放LP1
			case 240://单取LK1单放LP1
			{
				std::shared_ptr<EFEMLPSubsystem> elp = kernel->getKernelModule<EFEMLPSubsystem>(sequence_lp1_put_wafer[0].source_lp);
				if (elp&&elp->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					if (!elp->hasDoorOpend()){
						auto cmd = elp->createOpenBoxCommand();
						elp->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(elp->getName(), "打开晶圆盒", efem_process_name, efem_auto_step);
						}
						else{
							efem_auto_step = 241;
						}
					}
					else{
						efem_auto_step = 241;
					}
				}
				else{
					logFailedNotNormal(elp->getName(), efem_process_name, efem_auto_step);
				}
				
			}
			break;
			case 241:
			{
				if (lk1&&lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					auto cmd = lk1->createMoveToSlotCommand(sequence_lp1_put_wafer[0].slot_lk + efem_slot_offset);
					lk1->startCommand(cmd);
					cmd->wait();
					if (cmd->hasError())
					{
						logFailedExcuteCommandHasError(lk1->getName(), "LLA移动到slot", efem_process_name, efem_auto_step);
					}
					else
					{
						efem_auto_step = 242;
					}
				}
				else{
					logFailedNotNormal(lk1->getName(), efem_process_name, efem_auto_step);
				}

			}
			break;
			case 242:
			{
				if (lk1&&lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					if (!lk1->getCassetteDoorOpend()){
						auto cmdopendoor = lk1->createOpenCassetteDoorCommand();
						lk1->startCommand(cmdopendoor);
						cmdopendoor->wait();
						if (cmdopendoor->hasError())
						{
							logFailedExcuteCommandHasError(lk1->getName(), "LLA打开晶圆盒门", efem_process_name, efem_auto_step);
						}
						else{
							efem_auto_step = 243; 
							Sleep(2000);
						}
					}
					else{
						efem_auto_step = 243; 
						Sleep(2000);
					}
				}
				else{
					logFailedNotNormal(lk1->getName(), efem_process_name, efem_auto_step);
				}
			}
			break;
			case 243:
			{
				if (ewtr&&ewtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					auto cmd1 = ewtr->createGetCommand(lk1, 1, sequence_lp1_put_wafer[0].slot_lk);
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
				else{
					logFailedNotNormal(ewtr->getName(), efem_process_name, efem_auto_step);
				}
			}
			break;
			case 244:
			{
				if (ewtr&&ewtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					std::shared_ptr<EFEMLPSubsystem> elp = kernel->getKernelModule<EFEMLPSubsystem>(sequence_lp1_put_wafer[0].source_lp);
					auto cmd2 = ewtr->createPutCommand(elp, 1, sequence_lp1_put_wafer[0].slot_lp);
					ewtr->startCommand(cmd2);
					cmd2->wait();
					if (cmd2->hasError())
					{
						logFailedExcuteCommandHasError(ewtr->getName(), "放LP1晶圆", efem_process_name, efem_auto_step);
					}
					else{
						if (sequence_lp1_put_wafer[0].source_lp == "ELP1"){
							if (sequence_lp1_transfer_wafer.size() >= 1){
								sequence_lp1_transfer_wafer.erase(sequence_lp1_transfer_wafer.begin());
							}
							else{
								logError(ewtr->getName().c_str(), "ELP1 sequence_lp1_put_wafer=%d sequence_lp1_transfer_wafer=%d", sequence_lp1_put_wafer.size(), sequence_lp1_transfer_wafer.size());
							}

						}
						else if (sequence_lp1_put_wafer[0].source_lp == "ELP2"){
							if (sequence_lp2_transfer_wafer.size() >= 1){
								sequence_lp2_transfer_wafer.erase(sequence_lp2_transfer_wafer.begin());
							}
							else{
								logError(ewtr->getName().c_str(), "ELP2 sequence_lp1_put_wafer=%d sequence_lp1_transfer_wafer=%d", sequence_lp1_put_wafer.size(), sequence_lp2_transfer_wafer.size());
							}
						}
						if (sequence_lp1_put_wafer.size() == 1 && sequence_lp1_put_wafer[0].source_lp == "ELP2"&&sequence_lp2_transfer_wafer.size() == 0){
							logInform(ewtr->getName().c_str(), "cycle end sequence_lp2_put_wafer=%d sequence_lp2_transfer_wafer=%d", sequence_lp2_put_wafer.size(), sequence_lp2_transfer_wafer.size());
							is_lp2_cycle = true;
						}
						if (sequence_lp1_put_wafer.size() == 1 && sequence_lp1_put_wafer[0].source_lp == "ELP1"&&sequence_lp1_transfer_wafer.size() == 0){
							logInform(ewtr->getName().c_str(), "cycle end sequence_lp1_put_wafer=%d sequence_lp1_transfer_wafer=%d", sequence_lp1_put_wafer.size(), sequence_lp1_transfer_wafer.size());
							is_lp1_cycle = true;
						}
						sequence_lp1_put_wafer.erase(sequence_lp1_put_wafer.begin());
						//sequence_lp1_transfer_wafer.erase(sequence_lp1_transfer_wafer.begin());
						efem_auto_step = 201;
					}
				}
				else{
					logFailedNotNormal(ewtr->getName(), efem_process_name, efem_auto_step);
				}
			}
			break;
#pragma endregion
#pragma region 双取LK1双放LP1
			case 250://双取LK1双放LP1
			{
				std::shared_ptr<EFEMLPSubsystem> elp = kernel->getKernelModule<EFEMLPSubsystem>(sequence_lp1_put_wafer[0].source_lp);
				std::shared_ptr<EFEMLPSubsystem> elp2put = kernel->getKernelModule<EFEMLPSubsystem>(sequence_lp1_put_wafer[1].source_lp);
				bool elpopen = false;
				bool elp2open = false;
				if (elp&&elp->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					if (!elp->hasDoorOpend()){
						auto cmd = elp->createOpenBoxCommand();
						elp->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(elp->getName(), "打开晶圆盒", efem_process_name, efem_auto_step);
						}
						else{
							elpopen = true;
						}
					}
					else{
						elpopen = true;
					}
				}
				else{
					logFailedNotNormal(elp->getName(), efem_process_name, efem_auto_step);
				}

				if (elp2put&&elp2put->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					if (!elp2put->hasDoorOpend()){
						auto cmd = elp2put->createOpenBoxCommand();
						elp2put->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(elp2put->getName(), "打开晶圆盒", efem_process_name, efem_auto_step);
						}
						else{
							elp2open = true;
						}
					}
					else{
						elp2open = true;
					}
				}
				else{
					logFailedNotNormal(elp2put->getName(), efem_process_name, efem_auto_step);
				}

				if (elpopen&&elp2open){
					efem_auto_step = 251;
				}
			}
			break;
			case 251:
			{
				if (lk1&&lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					auto cmd = lk1->createMoveToSlotCommand(sequence_lp1_put_wafer[0].slot_lk + efem_slot_offset);
					lk1->startCommand(cmd);
					cmd->wait();
					if (cmd->hasError())
					{
						logFailedExcuteCommandHasError(lk1->getName(), "LLA移动到slot", efem_process_name, efem_auto_step);
					}
					else
					{
						efem_auto_step = 252;
					}
				}
				else{
					logFailedNotNormal(lk1->getName(), efem_process_name, efem_auto_step);
				}

			}
			break;
			case 252:
			{
				if (lk1&&lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					if (!lk1->getCassetteDoorOpend()){
						auto cmdopendoor = lk1->createOpenCassetteDoorCommand();
						lk1->startCommand(cmdopendoor);
						cmdopendoor->wait();
						if (cmdopendoor->hasError())
						{
							logFailedExcuteCommandHasError(lk1->getName(), "LLA打开晶圆盒门", efem_process_name, efem_auto_step);
						}
						else{
							efem_auto_step = 253;
							Sleep(2000);
						}
					}
					else{
						efem_auto_step = 253;
						Sleep(2000);
					}
				}
				else{
					logFailedNotNormal(lk1->getName(), efem_process_name, efem_auto_step);
				}
			}
			break;
			case 253:
			{
				if (ewtr&&ewtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					auto cmd1 = ewtr->createGetCommand(lk1, 1, sequence_lp1_put_wafer[0].slot_lk);
					ewtr->startCommand(cmd1);
					cmd1->wait();
					if (cmd1->hasError())
					{
						logFailedExcuteCommandHasError(ewtr->getName(), "A手臂取LLA晶圆", efem_process_name, efem_auto_step);
					}
					else
					{
						efem_auto_step = 254;
					}
				}
				else{
					logFailedNotNormal(ewtr->getName(), efem_process_name, efem_auto_step);
				}
			}
			break;
			case 254:
			{
				if (lk1&&lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					auto cmd4 = lk1->createMoveToSlotCommand(sequence_lp1_put_wafer[1].slot_lk + efem_slot_offset);
					lk1->startCommand(cmd4);
					cmd4->wait();
					if (cmd4->hasError())
					{
						logFailedExcuteCommandHasError(lk1->getName(), "LLA移动到slot", efem_process_name, efem_auto_step);
					}
					else
					{
						Sleep(2000);
						efem_auto_step = 255;
					}
				}
				else{
					logFailedNotNormal(lk1->getName(), efem_process_name, efem_auto_step);
				}
			}
			break;
			case 255:
			{
				if (ewtr&&ewtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					auto cmd1 = ewtr->createGetCommand(lk1, 2, sequence_lp1_put_wafer[1].slot_lk);
					ewtr->startCommand(cmd1);
					cmd1->wait();
					if (cmd1->hasError())
					{
						logFailedExcuteCommandHasError(ewtr->getName(), "B手臂取LLA晶圆", efem_process_name, efem_auto_step);
					}
					else
					{
						efem_auto_step = 256;
					}
				}
				else{
					logFailedNotNormal(ewtr->getName(), efem_process_name, efem_auto_step);
				}
			}
			break;
			case 256:
			{
				if (ewtr&&ewtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					std::shared_ptr<EFEMLPSubsystem> elp1put = kernel->getKernelModule<EFEMLPSubsystem>(sequence_lp1_put_wafer[0].source_lp);
					auto cmd3 = ewtr->createPutCommand(elp1put, 1, sequence_lp1_put_wafer[0].slot_lp);
					ewtr->startCommand(cmd3);
					cmd3->wait();
					if (cmd3->hasError())
					{
						logFailedExcuteCommandHasError(ewtr->getName(), "A手臂放LP1晶圆", efem_process_name, efem_auto_step);
					}
					else
					{
						efem_auto_step = 257;
					}
				}
				else{
					logFailedNotNormal(ewtr->getName(), efem_process_name, efem_auto_step);
				}
			}
			break;
			case 257:
			{
				if (ewtr&&ewtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					std::shared_ptr<EFEMLPSubsystem> elp2put = kernel->getKernelModule<EFEMLPSubsystem>(sequence_lp1_put_wafer[1].source_lp);
					auto cmd5 = ewtr->createPutCommand(elp2put, 2, sequence_lp1_put_wafer[1].slot_lp);
					ewtr->startCommand(cmd5);
					cmd5->wait();
					if (cmd5->hasError())
					{
						logFailedExcuteCommandHasError(ewtr->getName(), "B手臂放LP1晶圆", efem_process_name, efem_auto_step);
					}
					else
					{

							if (sequence_lp1_put_wafer[0].source_lp == "ELP1"){
								if (sequence_lp1_transfer_wafer.size() >= 1){
									sequence_lp1_transfer_wafer.erase(sequence_lp1_transfer_wafer.begin());
								}
								else{
									logError(ewtr->getName().c_str(), "ELP1 sequence_lp1_put_wafer=%d sequence_lp1_transfer_wafer=%d", sequence_lp1_put_wafer.size(), sequence_lp1_transfer_wafer.size());
								}

							}
							else if (sequence_lp1_put_wafer[0].source_lp == "ELP2"){
								if (sequence_lp2_transfer_wafer.size() >= 1){
									sequence_lp2_transfer_wafer.erase(sequence_lp2_transfer_wafer.begin());
								}
								else{
									logError(ewtr->getName().c_str(), "ELP2 sequence_lp1_put_wafer=%d sequence_lp1_transfer_wafer=%d", sequence_lp1_put_wafer.size(), sequence_lp2_transfer_wafer.size());
								}
							}

							if (sequence_lp1_put_wafer[1].source_lp == "ELP1"){
								if (sequence_lp1_transfer_wafer.size() >= 1){
									sequence_lp1_transfer_wafer.erase(sequence_lp1_transfer_wafer.begin());
								}
								else{
									logError(ewtr->getName().c_str(), "ELP1 sequence_lp1_put_wafer=%d sequence_lp1_transfer_wafer=%d", sequence_lp1_put_wafer.size(), sequence_lp1_transfer_wafer.size());
								}

							}
							else if (sequence_lp1_put_wafer[1].source_lp == "ELP2"){
								if (sequence_lp2_transfer_wafer.size() >= 1){
									sequence_lp2_transfer_wafer.erase(sequence_lp2_transfer_wafer.begin());
								}
								else{
									logError(ewtr->getName().c_str(), "ELP2 sequence_lp1_put_wafer=%d sequence_lp1_transfer_wafer=%d", sequence_lp1_put_wafer.size(), sequence_lp2_transfer_wafer.size());
								}
							}
						
						if (sequence_lp1_put_wafer.size() == 2 && sequence_lp1_put_wafer[0].source_lp != sequence_lp1_put_wafer[1].source_lp
							&&sequence_lp1_put_wafer[0].source_lp == "ELP1"&&sequence_lp1_transfer_wafer.size() == 0){
							logInform(ewtr->getName().c_str(), "cycle end2 sequence_lp1_put_wafer=%d sequence_lp1_transfer_wafer=%d", sequence_lp1_put_wafer.size(), sequence_lp1_transfer_wafer.size());
							is_lp1_cycle = true;
						}

						if (sequence_lp1_put_wafer.size() == 2 && sequence_lp1_put_wafer[0].source_lp != sequence_lp1_put_wafer[1].source_lp
							&&sequence_lp1_put_wafer[0].source_lp == "ELP2"&&sequence_lp2_transfer_wafer.size() == 0){
							logInform(ewtr->getName().c_str(), "cycle end2 sequence_lp2_put_wafer=%d sequence_lp2_transfer_wafer=%d", sequence_lp2_put_wafer.size(), sequence_lp2_transfer_wafer.size());
							is_lp2_cycle = true;
						}
						
						if (sequence_lp1_put_wafer.size() == 2 && sequence_lp1_put_wafer[1].source_lp == "ELP2"&&sequence_lp2_transfer_wafer.size() == 0){
							logInform(ewtr->getName().c_str(), "cycle end sequence_lp2_put_wafer=%d sequence_lp2_transfer_wafer=%d", sequence_lp2_put_wafer.size(), sequence_lp2_transfer_wafer.size());
							is_lp2_cycle = true;
						}
						if (sequence_lp1_put_wafer.size() == 2 && sequence_lp1_put_wafer[1].source_lp == "ELP1"&&sequence_lp1_transfer_wafer.size() == 0){
							logInform(ewtr->getName().c_str(), "cycle end sequence_lp1_put_wafer=%d sequence_lp1_transfer_wafer=%d", sequence_lp1_put_wafer.size(), sequence_lp1_transfer_wafer.size());
							is_lp1_cycle = true;
						}
						sequence_lp1_put_wafer.erase(sequence_lp1_put_wafer.begin());
						sequence_lp1_put_wafer.erase(sequence_lp1_put_wafer.begin());
						//sequence_lp1_transfer_wafer.erase(sequence_lp1_transfer_wafer.begin());
						//sequence_lp1_transfer_wafer.erase(sequence_lp1_transfer_wafer.begin());
						efem_auto_step = 201;
					}
				}
				else{
					logFailedNotNormal(ewtr->getName(), efem_process_name, efem_auto_step);
				}
			}
			break;
#pragma endregion
#pragma endregion
            #pragma region 给TOOL2上料,先判断是哪个LP
			case 1000:
			{
				if (sequence_tolk2_wafer.size()>0){
					if (sequence_tolk2_wafer[0].source_lp == "ELP1"){
						if (elp1->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
							if (!elp1->hasDoorOpend()){
								auto cmd = elp1->createOpenBoxCommand();
								elp1->startCommand(cmd);
								cmd->wait();
								if (cmd->hasError())
								{
									logFailedExcuteCommandHasError(elp1->getName(), "打开晶圆盒", efem_process_name, efem_auto_step);
								}
								else{
									efem_auto_step = 1200;
								}
							}
							else{
								auto cmd = elp1->createGetMapCommand();
								elp1->startCommand(cmd);
								cmd->wait();
								if (cmd->hasError())
								{
									logFailedExcuteCommandHasError(elp1->getName(), "获取MAP", efem_process_name, efem_auto_step);
								}
								else{
									efem_auto_step = 1200;
								}
							}
						}
						else{
							logFailedNotNormal(elp1->getName(), efem_process_name, efem_auto_step);
						}
					}
					else if (sequence_tolk2_wafer[0].source_lp == "ELP2"){
						if (elp2->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
							if (!elp2->hasDoorOpend()){
								auto cmd = elp2->createOpenBoxCommand();
								elp2->startCommand(cmd);
								cmd->wait();
								if (cmd->hasError())
								{
									logFailedExcuteCommandHasError(elp2->getName(), "打开晶圆盒", efem_process_name, efem_auto_step);
								}
								else{
									efem_auto_step = 1200;
								}
							}
							else{
								auto cmd = elp2->createGetMapCommand();
								elp2->startCommand(cmd);
								cmd->wait();
								if (cmd->hasError())
								{
									logFailedExcuteCommandHasError(elp2->getName(), "获取MAP", efem_process_name, efem_auto_step);
								}
								else{
									efem_auto_step = 1200;
								}
							}
						}
						else{
							logFailedNotNormal(elp1->getName(), efem_process_name, efem_auto_step);
						}
					}
					else{
						Sleep(100);
					}
				}
				else{
					Sleep(50);
				}

			}
			break;
			case 1200:
			{
				auto cassManager = lk2->getKernel()->getKernelModule<FortrendCassetteManager>();
				auto station_cass_lk = cassManager->getCassette(lk2.get());
				auto lkmaps = station_cass_lk->getAllMapping();//查看
				for (int i = 0; i < lkmaps.size(); i++)
				{
					if (lkmaps[i] == Cassette::Empty&&sequence_tolk2_wafer.size()>0){
						LPTransferWafer getlp;
						getlp.is_finish_putlk = false;
						getlp.slot_lk = sequence_tolk2_wafer[0].slot_lk;
						getlp.slot_lp = sequence_tolk2_wafer[0].slot_lp;
						getlp.transfer = sequence_tolk2_wafer[0].transfer;
						getlp.selected_arm = sequence_tolk2_wafer[0].selected_arm;
						getlp.source_lp = sequence_tolk2_wafer[0].source_lp;
						sequence_lp2_get_wafer.push_back(getlp);
						sequence_tolk2_wafer.erase(sequence_tolk2_wafer.begin());
					}
				}
				/*int count = lkmaps.size() - sequence_lp2_get_wafer.size();
				if (count != 0 && sequence_lp1_transfer_wafer.size()>0){
					for (int i = 0; i < count; i++)
					{
						int key = sequence_lp2_get_wafer.size();
						if (lkmaps[key] == Cassette::Empty){
							if (sequence_lp1_transfer_wafer.size()>i){
								LPTransferWafer getlp;
								getlp.is_finish_putlk = false;
								getlp.slot_lk = sequence_lp1_transfer_wafer[i].slot_lk;
								getlp.slot_lp = sequence_lp1_transfer_wafer[i].slot_lp;
								getlp.transfer = sequence_lp1_transfer_wafer[i].transfer;
								getlp.selected_arm = sequence_lp1_transfer_wafer[i].selected_arm;
								getlp.source_lp = sequence_lp1_transfer_wafer[i].source_lp;
								sequence_lp2_get_wafer.push_back(getlp);
							}
						}
					}
				}*/
				efem_auto_step = 1300;
			}
			break;
			case 1300:
			{
				if (sequence_lp2_get_wafer.size() == 1){
					efem_auto_step = 1400;
				}
				else if (sequence_lp2_get_wafer.size() >= 2){
					efem_auto_step = 1500;
				}
				else{//上料完成
#if 0	
					if (elp1&&elp1->getState() == IKernelSubSystem::State::SUB_NORMAL){
						if (elp1->hasDoorOpend()){
							auto cmd = elp1->createCloseBoxCommand();
							elp1->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError())
							{
								logFailedExcuteCommandHasError(elp1->getName(), "关闭晶圆盒", efem_process_name, efem_auto_step);
							}
						}
					}
					else{
						logFailedNotNormal(elp1->getName(), efem_process_name, efem_auto_step);
					}

					if (elp2&&elp2->getState() == IKernelSubSystem::State::SUB_NORMAL){
						if (elp2->hasDoorOpend()){
							auto cmd = elp2->createCloseBoxCommand();
							elp2->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError())
							{
								logFailedExcuteCommandHasError(elp2->getName(), "关闭晶圆盒", efem_process_name, efem_auto_step);
							}
						}
					}
					else{
						logFailedNotNormal(elp2->getName(), efem_process_name, efem_auto_step);
					}

					if (!elp2->hasDoorOpend() && !elp1->hasDoorOpend()){
						tool2_allow_get_wafer = false;
						efem_auto_step = 10;
					}
#endif
					tool2_allow_get_wafer = false;
					efem_auto_step = 10;
				}
			}
			break;
#pragma region 单取单放lk2
			case 1400:
			{
				std::shared_ptr<EFEMLPSubsystem> elp = kernel->getKernelModule<EFEMLPSubsystem>(sequence_lp2_get_wafer[0].source_lp);
				if (elp&& elp->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					if (!elp->hasDoorOpend()){
						auto cmd = elp->createOpenBoxCommand();
						elp->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(elp->getName(), "打开晶圆盒", efem_process_name, efem_auto_step);
						}
						else{
							efem_auto_step = 1401;
						}
					}
					else{
						efem_auto_step = 1401;
					}
				}
				else{
					logFailedNotNormal(elp->getName(), efem_process_name, efem_auto_step);
				}

			}
			break;
			case 1401://LP2上料
			{
				if (lk2&& lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					auto cmd1 = lk2->createMoveToSlotCommand(sequence_lp2_get_wafer[0].slot_lk + efem_slot_offset);
					lk2->startCommand(cmd1);
					cmd1->wait();
					if (cmd1->hasError())
					{
						logFailedExcuteCommandHasError(lk2->getName(), "LLB移动到slot", efem_process_name, efem_auto_step);
					}
					else
					{
						efem_auto_step = 1402;
					}
				}
				else{
					logFailedNotNormal(lk2->getName(), efem_process_name, efem_auto_step);
				}


			}
			break;
			case 1402://LP2上料
			{
				std::shared_ptr<EFEMLPSubsystem> elp = kernel->getKernelModule<EFEMLPSubsystem>(sequence_lp2_get_wafer[0].source_lp);
				if (elp&& elp->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					auto cmd = ewtr->createGetCommand(elp, 1, sequence_lp2_get_wafer[0].slot_lp);
					ewtr->startCommand(cmd);
					cmd->wait();
					if (cmd->hasError())
					{
						logFailedExcuteCommandHasError(ewtr->getName(), "取LP2晶圆", efem_process_name, efem_auto_step);
					}
					else
					{
						efem_auto_step = 1403;
					}
				}
				else{
					logFailedNotNormal(elp->getName(), efem_process_name, efem_auto_step);
				}

			}
			break;
			case 1403://LP2上料
			{
				if (lk2&& lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					if (!lk2->getCassetteDoorOpend()){
						auto cmdopendoor = lk2->createOpenCassetteDoorCommand();
						lk2->startCommand(cmdopendoor);
						cmdopendoor->wait();
						if (cmdopendoor->hasError())
						{
							logFailedExcuteCommandHasError(lk2->getName(), "LLB打开晶圆盒门", efem_process_name, efem_auto_step);
						}
					}
					else{
						Sleep(2000);
						efem_auto_step = 1404;
					}

				}
				else{
					logFailedNotNormal(lk2->getName(), efem_process_name, efem_auto_step);
				}
			}
			break;
			case 1404://LP2上料
			{
				if (ewtr&& ewtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					auto cmd2 = ewtr->createPutCommand(lk2, 1, sequence_lp2_get_wafer[0].slot_lk);
					ewtr->startCommand(cmd2);
					cmd2->wait();
					if (cmd2->hasError())
					{
						logFailedExcuteCommandHasError(ewtr->getName(), "放LLB晶圆", efem_process_name, efem_auto_step);
					}
					else{//放料完成
						LoadLockTransferWafer lktw;
						lktw.is_finish = false;
						lktw.slot = sequence_lp2_get_wafer[0].slot_lk;
						lktw.transfer = sequence_lp2_get_wafer[0].transfer;
						lktw.selected_arm = sequence_lp2_get_wafer[0].selected_arm;
						sequence_loadlock2_transfer_wafer.push_back(lktw);
						sequence_lp2_put_wafer.push_back(sequence_lp2_get_wafer.front());//提前存入放料集合
						sequence_lp2_get_wafer.erase(sequence_lp2_get_wafer.begin());
						efem_auto_step = 1300;
					}
				}
				else{
					logFailedNotNormal(ewtr->getName(), efem_process_name, efem_auto_step);
				}
			}
			break;
#pragma endregion
#pragma region 双取LP2双放lk2
			case 1500:
			{
				std::shared_ptr<EFEMLPSubsystem> elp1get = kernel->getKernelModule<EFEMLPSubsystem>(sequence_lp2_get_wafer[0].source_lp);
				if (elp1get&& elp1get->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					if (!elp1get->hasDoorOpend()){
						auto cmd = elp1get->createOpenBoxCommand();
						elp1get->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(elp1get->getName(), "打开晶圆盒", efem_process_name, efem_auto_step);
						}
						else{
							efem_auto_step = 1501;
						}
					}
					else{
						efem_auto_step = 1501;
					}
				}
				else{
					logFailedNotNormal(elp1get->getName(), efem_process_name, efem_auto_step);
				}

			}
			break;
			case 1501:
			{
				std::shared_ptr<EFEMLPSubsystem> eLP2get = kernel->getKernelModule<EFEMLPSubsystem>(sequence_lp2_get_wafer[1].source_lp);
				if (eLP2get&& eLP2get->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					if (!eLP2get->hasDoorOpend()){
						auto cmd = eLP2get->createOpenBoxCommand();
						eLP2get->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(eLP2get->getName(), "打开晶圆盒", efem_process_name, efem_auto_step);
						}
						else{
							efem_auto_step = 1502;
						}
					}
					else{
						efem_auto_step = 1502;
					}
				}
				else{
					logFailedNotNormal(eLP2get->getName(), efem_process_name, efem_auto_step);
				}
			}
			break;
			case 1502://LP2上料
			{
				if (lk2&& lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					auto cmd1 = lk2->createMoveToSlotCommand(sequence_lp2_get_wafer[0].slot_lk + efem_slot_offset);
					lk2->startCommand(cmd1);
					cmd1->wait();
					if (cmd1->hasError())
					{
						logFailedExcuteCommandHasError(lk2->getName(), "LLB移动到slot", efem_process_name, efem_auto_step);
					}
					else
					{
						efem_auto_step = 1503;
					}
				}
				else{
					logFailedNotNormal(lk2->getName(), efem_process_name, efem_auto_step);
				}
			}
			break;
			case 1503://LP2上料
			{
				std::shared_ptr<EFEMLPSubsystem> elp = kernel->getKernelModule<EFEMLPSubsystem>(sequence_lp2_get_wafer[0].source_lp);
				if (elp&& elp->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					auto cmd = ewtr->createGetCommand(elp, 1, sequence_lp2_get_wafer[0].slot_lp);
					ewtr->startCommand(cmd);
					cmd->wait();
					if (cmd->hasError())
					{
						logFailedExcuteCommandHasError(ewtr->getName(), "A手臂取LP2晶圆", efem_process_name, efem_auto_step);
					}
					else
					{
						efem_auto_step = 1504;
					}
				}
				else{
					logFailedNotNormal(elp->getName(), efem_process_name, efem_auto_step);
				}

			}
			break;
			case 1504://LP2上料
			{
				std::shared_ptr<EFEMLPSubsystem> elp = kernel->getKernelModule<EFEMLPSubsystem>(sequence_lp2_get_wafer[1].source_lp);
				if (elp&& elp->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					auto cmd = ewtr->createGetCommand(elp, 2, sequence_lp2_get_wafer[1].slot_lp);
					ewtr->startCommand(cmd);
					cmd->wait();
					if (cmd->hasError())
					{
						logFailedExcuteCommandHasError(ewtr->getName(), "B手臂取LP2晶圆", efem_process_name, efem_auto_step);
					}
					else
					{
						efem_auto_step = 1505;
					}
				}
				else{
					logFailedNotNormal(elp->getName(), efem_process_name, efem_auto_step);
				}

			}
			break;
			case 1505://LP2上料
			{
				if (lk2&& lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					if (!lk2->getCassetteDoorOpend()){
						auto cmdopendoor = lk2->createOpenCassetteDoorCommand();
						lk2->startCommand(cmdopendoor);
						cmdopendoor->wait();
						if (cmdopendoor->hasError())
						{
							logFailedExcuteCommandHasError(lk2->getName(), "LLB打开晶圆盒门", efem_process_name, efem_auto_step);
						}
					}
					else{
						Sleep(2000);
						efem_auto_step = 1507;
					}

				}
				else{
					logFailedNotNormal(lk2->getName(), efem_process_name, efem_auto_step);
				}
			}
			break;
			case 1507://LP2上料
			{
				if (ewtr&& ewtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					auto cmd2 = ewtr->createPutCommand(lk2, 1, sequence_lp2_get_wafer[0].slot_lk);
					ewtr->startCommand(cmd2);
					cmd2->wait();
					if (cmd2->hasError())
					{
						logFailedExcuteCommandHasError(ewtr->getName(), "A手臂放LLB晶圆", efem_process_name, efem_auto_step);
					}
					else{//放料完成
						LoadLockTransferWafer lktw1;
						lktw1.is_finish = false;
						lktw1.slot = sequence_lp2_get_wafer[0].slot_lk;
						lktw1.transfer = sequence_lp2_get_wafer[0].transfer;
						sequence_loadlock2_transfer_wafer.push_back(lktw1);
						efem_auto_step = 1508;
					}
				}
				else{
					logFailedNotNormal(ewtr->getName(), efem_process_name, efem_auto_step);
				}
			}
			break;
			case 1508://LP2上料
			{
				if (lk2&& lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					auto cmd1 = lk2->createMoveToSlotCommand(sequence_lp2_get_wafer[1].slot_lk + efem_slot_offset);
					lk2->startCommand(cmd1);
					cmd1->wait();
					if (cmd1->hasError())
					{
						logFailedExcuteCommandHasError(lk2->getName(), "LLB移动到slot", efem_process_name, efem_auto_step);
					}
					else
					{
						Sleep(2000);
						efem_auto_step = 1509;
					}
				}
				else{
					logFailedNotNormal(lk2->getName(), efem_process_name, efem_auto_step);
				}
			}
			break;
			case 1509://LP2上料
			{
				if (ewtr&& ewtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					auto cmd2 = ewtr->createPutCommand(lk2, 2, sequence_lp2_get_wafer[1].slot_lk);
					ewtr->startCommand(cmd2);
					cmd2->wait();
					if (cmd2->hasError())
					{
						logFailedExcuteCommandHasError(ewtr->getName(), "B手臂放LLB晶圆", efem_process_name, efem_auto_step);
					}
					else{//放料完成
						LoadLockTransferWafer lktw2;
						lktw2.is_finish = false;
						lktw2.slot = sequence_lp2_get_wafer[1].slot_lk;
						lktw2.transfer = sequence_lp2_get_wafer[1].transfer;
						lktw2.selected_arm = sequence_lp2_get_wafer[1].selected_arm;
						sequence_loadlock2_transfer_wafer.push_back(lktw2);
						sequence_lp2_put_wafer.push_back(sequence_lp2_get_wafer.front());//提前存入放料集合
						sequence_lp2_get_wafer.erase(sequence_lp2_get_wafer.begin());
						sequence_lp2_put_wafer.push_back(sequence_lp2_get_wafer.front());//提前存入放料集合
						sequence_lp2_get_wafer.erase(sequence_lp2_get_wafer.begin());
						efem_auto_step = 1300;
					}
				}
				else{
					logFailedNotNormal(ewtr->getName(), efem_process_name, efem_auto_step);
				}
			}
			break;

#pragma endregion
			//case 1400://单取LP2单放LK1
			//{
			//	std::shared_ptr<EFEMLPSubsystem> elp = kernel->getKernelModule<EFEMLPSubsystem>(sequence_lp2_get_wafer[0].source_lp);
			//	if (ewtr&&elp&&lk2 && ewtr->getState() == IKernelSubSystem::State::SUB_NORMAL && elp->getState() == IKernelSubSystem::State::SUB_NORMAL&& lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
			//	{
			//		if (!elp->hasDoorOpend()){
			//			auto cmd = elp->createOpenBoxCommand();
			//			elp->startCommand(cmd);
			//			cmd->wait();
			//			if (cmd->hasError())
			//			{
			//				logFailedExcuteCommandHasError(elp->getName(), "打开晶圆盒", efem_process_name, efem_auto_step);
			//			}
			//		}
			//		int slot = sequence_lp2_get_wafer[0].slot_lp;
			//		int slotlk = sequence_lp2_get_wafer[0].slot_lk;
			//		auto cmd1 = lk2->createMoveToSlotCommand(slotlk + 30);
			//		lk2->startCommand(cmd1);
			//		cmd1->wait();
			//		if (cmd1->hasError())
			//		{
			//			logFailedExcuteCommandHasError(lk2->getName(), "LLB移动到slot", efem_process_name, efem_auto_step);
			//		}
			//		else
			//		{//取完料
			//			
			//			auto cmd = ewtr->createGetCommand(elp, 1, slot);
			//			ewtr->startCommand(cmd);
			//			cmd->wait();
			//			if (cmd->hasError())
			//			{
			//				logFailedExcuteCommandHasError(ewtr->getName(), "取LP2晶圆", efem_process_name, efem_auto_step);
			//			}
			//			else
			//			{//移动到SLOT完成
			//				if (!lk2->getCassetteDoorOpend()){
			//					auto cmdopendoor = lk2->createOpenCassetteDoorCommand();
			//					lk2->startCommand(cmdopendoor);
			//					cmdopendoor->wait();
			//					if (cmdopendoor->hasError())
			//					{
			//						logFailedExcuteCommandHasError(lk2->getName(), "LLB打开晶圆盒门", efem_process_name, efem_auto_step);
			//					}
			//				}
			//				Sleep(2000);
			//				auto cmd2 = ewtr->createPutCommand(lk2, 1, slotlk);
			//				ewtr->startCommand(cmd2);
			//				cmd2->wait();
			//				if (cmd2->hasError())
			//				{
			//					logFailedExcuteCommandHasError(ewtr->getName(), "放LLB晶圆", efem_process_name, efem_auto_step);
			//				}
			//				else{//放料完成
			//					LoadLockTransferWafer lktw;
			//					lktw.is_finish = false;
			//					lktw.slot = slotlk;
			//					lktw.transfer = sequence_lp2_get_wafer[0].transfer;
			//					lktw.selected_arm = sequence_lp2_get_wafer[0].selected_arm;
			//					sequence_loadlock2_transfer_wafer.push_back(lktw);
			//					sequence_lp2_put_wafer.push_back(sequence_lp2_get_wafer.front());//提前存入放料集合
			//					sequence_lp2_get_wafer.erase(sequence_lp2_get_wafer.begin());
			//					efem_auto_step = 1300;
			//				}
			//			}
			//		}
			//	}
			//	else
			//	{
			//		logFailedNotNormal(elp->getName(), efem_process_name, efem_auto_step);
			//	}
			//}
			//break;
			//case 1500://双取LP2双放LK1
			//{
			//	std::shared_ptr<EFEMLPSubsystem> elp1get = kernel->getKernelModule<EFEMLPSubsystem>(sequence_lp2_get_wafer[0].source_lp);
			//	std::shared_ptr<EFEMLPSubsystem> elp2get = kernel->getKernelModule<EFEMLPSubsystem>(sequence_lp2_get_wafer[1].source_lp);
			//	if (ewtr&&elp2get&&elp1get&&lk2 && ewtr->getState() == IKernelSubSystem::State::SUB_NORMAL 
			//		&&elp2get->getState() == IKernelSubSystem::State::SUB_NORMAL
			//		&&elp1get->getState() == IKernelSubSystem::State::SUB_NORMAL
			//		&& lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
			//	{
			//		if (!elp1get->hasDoorOpend()){
			//			auto cmd = elp1get->createOpenBoxCommand();
			//			elp1get->startCommand(cmd);
			//			cmd->wait();
			//			if (cmd->hasError())
			//			{
			//				logFailedExcuteCommandHasError(elp1get->getName(), "打开晶圆盒", efem_process_name, efem_auto_step);
			//			}
			//		}
			//		if (!elp2get->hasDoorOpend()){
			//			auto cmd = elp2get->createOpenBoxCommand();
			//			elp2get->startCommand(cmd);
			//			cmd->wait();
			//			if (cmd->hasError())
			//			{
			//				logFailedExcuteCommandHasError(elp2get->getName(), "打开晶圆盒", efem_process_name, efem_auto_step);
			//			}
			//		}
			//		int slot = sequence_lp2_get_wafer[0].slot_lp;
			//		int slotlk = sequence_lp2_get_wafer[0].slot_lk;
			//		int slot2 = sequence_lp2_get_wafer[1].slot_lp;
			//		int slot2lk = sequence_lp2_get_wafer[1].slot_lk;
			//		auto cmd2 = lk2->createMoveToSlotCommand(slotlk + 30);
			//		lk2->startCommand(cmd2);
			//		cmd2->wait();
			//		if (cmd2->hasError())
			//		{
			//			logFailedExcuteCommandHasError(lk2->getName(), "LLB移动到slot", efem_process_name, efem_auto_step);
			//		}
			//		else
			//		{//1爪取完料
			//			auto cmd = ewtr->createGetCommand(elp1get, 1, slot);
			//			ewtr->startCommand(cmd);
			//			cmd->wait();
			//			if (cmd->hasError())
			//			{
			//				logFailedExcuteCommandHasError(ewtr->getName(), "A手臂取LP2晶圆", efem_process_name, efem_auto_step);
			//			}
			//			else
			//			{//2爪取完料
			//				auto cmd1 = ewtr->createGetCommand(elp2get, 2, slot2);
			//				ewtr->startCommand(cmd1);
			//				cmd1->wait();
			//				if (cmd1->hasError())
			//				{
			//					logFailedExcuteCommandHasError(ewtr->getName(), "B手臂取LP2晶圆", efem_process_name, efem_auto_step);
			//				}
			//				else
			//				{//移动到SLOT完成
			//					if (!lk2->getCassetteDoorOpend()){
			//						auto cmdopendoor = lk2->createOpenCassetteDoorCommand();
			//						lk2->startCommand(cmdopendoor);
			//						cmdopendoor->wait();
			//						if (cmdopendoor->hasError())
			//						{
			//							logFailedExcuteCommandHasError(lk2->getName(), "LLB打开晶圆盒门", efem_process_name, efem_auto_step);
			//						}
			//					}
			//					Sleep(2000);
			//					auto cmd3 = ewtr->createPutCommand(lk2, 1, slotlk);
			//					ewtr->startCommand(cmd3);
			//					cmd3->wait();
			//					if (cmd3->hasError())
			//					{
			//						logFailedExcuteCommandHasError(ewtr->getName(), "A手臂放LLB晶圆", efem_process_name, efem_auto_step);
			//					}
			//					else
			//					{//1爪放料完成
			//						LoadLockTransferWafer lktw1;
			//						lktw1.is_finish = false;
			//						lktw1.slot = sequence_lp2_get_wafer[0].slot_lk;
			//						lktw1.transfer = sequence_lp2_get_wafer[0].transfer;
			//						sequence_loadlock2_transfer_wafer.push_back(lktw1);
			//						auto cmd4 = lk2->createMoveToSlotCommand(slot2lk + 30);
			//						lk2->startCommand(cmd4);
			//						cmd4->wait();
			//						if (cmd4->hasError())
			//						{
			//							logFailedExcuteCommandHasError(lk2->getName(), "LLB移动到slot", efem_process_name, efem_auto_step);
			//						}
			//						else
			//						{//移动到SLOT完成
			//							Sleep(2000);
			//							auto cmd5 = ewtr->createPutCommand(lk2, 2, slot2lk);
			//							ewtr->startCommand(cmd5);
			//							cmd5->wait();
			//							if (cmd5->hasError())
			//							{
			//								logFailedExcuteCommandHasError(ewtr->getName(), "B手臂放LLB晶圆", efem_process_name, efem_auto_step);
			//							}
			//							else{
			//								LoadLockTransferWafer lktw2;
			//								lktw2.is_finish = false;
			//								lktw2.slot = sequence_lp2_get_wafer[1].slot_lk;
			//								lktw2.transfer = sequence_lp2_get_wafer[1].transfer;
			//								lktw2.selected_arm = sequence_lp2_get_wafer[1].selected_arm;
			//								sequence_loadlock2_transfer_wafer.push_back(lktw2);
			//								sequence_lp2_put_wafer.push_back(sequence_lp2_get_wafer.front());//提前存入放料集合
			//								sequence_lp2_get_wafer.erase(sequence_lp2_get_wafer.begin());
			//								sequence_lp2_put_wafer.push_back(sequence_lp2_get_wafer.front());//提前存入放料集合
			//								sequence_lp2_get_wafer.erase(sequence_lp2_get_wafer.begin());
			//								efem_auto_step = 1300;
			//							}
			//						}
			//					}
			//				}
			//			}
			//		}
			//	}
			//	else
			//	{
			//		logFailedNotNormal(elp2get->getName(), efem_process_name, efem_auto_step);
			//	}
			//}
			//break;
			#pragma endregion
			#pragma region 给TOOL2下料
			case 2000:
			{
				std::shared_ptr<EFEMLPSubsystem> elp = kernel->getKernelModule<EFEMLPSubsystem>(sequence_lp2_put_wafer[0].source_lp);
				if (elp&&elp->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					if (!elp->hasDoorOpend()){
						auto cmd = elp->createOpenBoxCommand();
						elp->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(elp->getName(), "打开晶圆盒", efem_process_name, efem_auto_step);
						}
						else{
							efem_auto_step = 2200;
						}
					}
					else{
						auto cmd = elp->createGetMapCommand();
						elp->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(elp->getName(), "获取MAP", efem_process_name, efem_auto_step);
						}
						else{
							efem_auto_step = 2200;
						}
					}
				}
				else{
					logFailedNotNormal(elp->getName(), efem_process_name, efem_auto_step);
				}

			}
			break;
			case 2200:
			{
				//auto cassManager = lk2->getKernel()->getKernelModule<FortrendCassetteManager>();
				//auto station_cass_lk = cassManager->getCassette(lk2.get());
				//auto lkmaps = station_cass_lk->getAllMapping();//查看
				//for (int i = 0; i < lkmaps.size(); i++)
				//{
				//	if (lkmaps[i] == Cassette::Present&&sequence_lp2_transfer_wafer.size()>i){
				//		LPTransferWafer getlp;
				//		getlp.is_finish_putlk = false;
				//		getlp.slot_lk = i + 1;
				//		getlp.slot_lp = sequence_lp2_transfer_wafer[i].slot_lp;
				//		sequence_lp2_put_wafer.push_back(getlp);
				//	}
				//}
				efem_auto_step = 2300;
			}
			break;
			case 2300:
			{
				if (sequence_lp2_put_wafer.size() == 1){
					efem_auto_step = 2400;
				}
				else if (sequence_lp2_put_wafer.size() >= 2){
					efem_auto_step = 2500;
				}
				else{//下料完成
					if (elp2&&elp2->getState() == IKernelSubSystem::State::SUB_NORMAL){
						if (elp2->hasDoorOpend()){
							auto cmd = elp2->createCloseBoxCommand();
							elp2->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError())
							{
								logFailedExcuteCommandHasError(elp2->getName(), "关闭晶圆盒", efem_process_name, efem_auto_step);
							}
						}
					}
					else{
						logFailedNotNormal(elp1->getName(), efem_process_name, efem_auto_step);
					}

					if (elp1&&elp1->getState() == IKernelSubSystem::State::SUB_NORMAL){
						if (elp1->hasDoorOpend()){
							auto cmd = elp1->createCloseBoxCommand();
							elp1->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError())
							{
								logFailedExcuteCommandHasError(elp1->getName(), "关闭晶圆盒", efem_process_name, efem_auto_step);
							}
						}

					}
					else{
						logFailedNotNormal(elp1->getName(), efem_process_name, efem_auto_step);
					}



					if (!elp2->hasDoorOpend() && !elp1->hasDoorOpend()){
						tool2_allow_put_wafer = false;
						if (semi_auto_mode)
						{
							markSemiAutoLLDone(SemiAutoPhase::LLUnload, "LLB");
						}
						efem_auto_step = 10;
					}

				}
			}
			break;
#pragma region 单取lk2单放LP1
			case 2400://单取lk2单放LP1
			{
				std::shared_ptr<EFEMLPSubsystem> elp = kernel->getKernelModule<EFEMLPSubsystem>(sequence_lp2_put_wafer[0].source_lp);
				if (elp&&elp->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					if (!elp->hasDoorOpend()){
						auto cmd = elp->createOpenBoxCommand();
						elp->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(elp->getName(), "打开晶圆盒", efem_process_name, efem_auto_step);
						}
						else{
							efem_auto_step = 2401;
						}
					}
					else{
						efem_auto_step = 2401;
					}
				}
				else{
					logFailedNotNormal(elp->getName(), efem_process_name, efem_auto_step);
				}

			}
			break;
			case 2401:
			{
				if (lk2&&lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					auto cmd = lk2->createMoveToSlotCommand(sequence_lp2_put_wafer[0].slot_lk + efem_slot_offset);
					lk2->startCommand(cmd);
					cmd->wait();
					if (cmd->hasError())
					{
						logFailedExcuteCommandHasError(lk2->getName(), "LLB移动到slot", efem_process_name, efem_auto_step);
					}
					else
					{
						efem_auto_step = 2402;
					}
				}
				else{
					logFailedNotNormal(lk2->getName(), efem_process_name, efem_auto_step);
				}

			}
			break;
			case 2402:
			{
				if (lk2&&lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					if (!lk2->getCassetteDoorOpend()){
						auto cmdopendoor = lk2->createOpenCassetteDoorCommand();
						lk2->startCommand(cmdopendoor);
						cmdopendoor->wait();
						if (cmdopendoor->hasError())
						{
							logFailedExcuteCommandHasError(lk2->getName(), "LLB打开晶圆盒门", efem_process_name, efem_auto_step);
						}
						else{
							efem_auto_step = 2403;
							Sleep(2000);
						}
					}
					else{
						efem_auto_step = 2403;
						Sleep(2000);
					}
				}
				else{
					logFailedNotNormal(lk2->getName(), efem_process_name, efem_auto_step);
				}
			}
			break;
			case 2403:
			{
				if (ewtr&&ewtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					auto cmd1 = ewtr->createGetCommand(lk2, 1, sequence_lp2_put_wafer[0].slot_lk);
					ewtr->startCommand(cmd1);
					cmd1->wait();
					if (cmd1->hasError())
					{
						logFailedExcuteCommandHasError(ewtr->getName(), "取LLB晶圆", efem_process_name, efem_auto_step);
					}
					else
					{
						efem_auto_step = 2404;
					}
				}
				else{
					logFailedNotNormal(ewtr->getName(), efem_process_name, efem_auto_step);
				}
			}
			break;
			case 2404:
			{
				if (ewtr&&ewtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					std::shared_ptr<EFEMLPSubsystem> elp = kernel->getKernelModule<EFEMLPSubsystem>(sequence_lp2_put_wafer[0].source_lp);
					auto cmd2 = ewtr->createPutCommand(elp, 1, sequence_lp2_put_wafer[0].slot_lp);
					ewtr->startCommand(cmd2);
					cmd2->wait();
					if (cmd2->hasError())
					{
						logFailedExcuteCommandHasError(ewtr->getName(), "放LP2晶圆", efem_process_name, efem_auto_step);
					}
					else{
						if (sequence_lp2_put_wafer[0].source_lp == "ELP1"){
							if (sequence_lp1_transfer_wafer.size() >= 1){
								sequence_lp1_transfer_wafer.erase(sequence_lp1_transfer_wafer.begin());
							}
							else{
								logError(ewtr->getName().c_str(), "ELP1 sequence_lp2_put_wafer=%d sequence_lp1_transfer_wafer=%d", sequence_lp2_put_wafer.size(), sequence_lp1_transfer_wafer.size());
							}

						}
						else if (sequence_lp2_put_wafer[0].source_lp == "ELP2"){
							if (sequence_lp2_transfer_wafer.size() >= 1){
								sequence_lp2_transfer_wafer.erase(sequence_lp2_transfer_wafer.begin());
							}
							else{
								logError(ewtr->getName().c_str(), "ELP2 sequence_lp2_put_wafer=%d sequence_lp1_transfer_wafer=%d", sequence_lp2_put_wafer.size(), sequence_lp1_transfer_wafer.size());
							}
						}

						if (sequence_lp2_put_wafer.size() == 1 && sequence_lp2_put_wafer[0].source_lp == "ELP1"&&sequence_lp1_transfer_wafer.size() == 0){
							logInform(ewtr->getName().c_str(), "cycle end sequence_lp1_put_wafer=%d sequence_lp1_transfer_wafer=%d", sequence_lp1_put_wafer.size(), sequence_lp1_transfer_wafer.size());
							is_lp1_cycle = true;
						}
						if (sequence_lp2_put_wafer.size() == 1 && sequence_lp2_put_wafer[0].source_lp == "ELP2"&&sequence_lp2_transfer_wafer.size() == 0){
							logInform(ewtr->getName().c_str(), "cycle end sequence_lp2_put_wafer=%d sequence_lp2_transfer_wafer=%d", sequence_lp2_put_wafer.size(), sequence_lp2_transfer_wafer.size());
							is_lp2_cycle = true;
						}
						sequence_lp2_put_wafer.erase(sequence_lp2_put_wafer.begin());
						efem_auto_step = 2300;
					}
				}
				else{
					logFailedNotNormal(ewtr->getName(), efem_process_name, efem_auto_step);
				}
			}
			break;
#pragma endregion
#pragma region 双取lk2双放LP1
			case 2500://双取lk2双放LP1
			{
				std::shared_ptr<EFEMLPSubsystem> elp = kernel->getKernelModule<EFEMLPSubsystem>(sequence_lp2_put_wafer[0].source_lp);
				std::shared_ptr<EFEMLPSubsystem> elp2put = kernel->getKernelModule<EFEMLPSubsystem>(sequence_lp2_put_wafer[1].source_lp);
				bool elpopen = false;
				bool elp2open = false;
				if (elp&&elp->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					if (!elp->hasDoorOpend()){
						auto cmd = elp->createOpenBoxCommand();
						elp->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(elp->getName(), "打开晶圆盒", efem_process_name, efem_auto_step);
						}
						else{
							elpopen = true;
						}
					}
					else{
						elpopen = true;
					}
				}
				else{
					logFailedNotNormal(elp->getName(), efem_process_name, efem_auto_step);
				}

				if (elp2put&&elp2put->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					if (!elp2put->hasDoorOpend()){
						auto cmd = elp2put->createOpenBoxCommand();
						elp2put->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(elp2put->getName(), "打开晶圆盒", efem_process_name, efem_auto_step);
						}
						else{
							elp2open = true;
						}
					}
					else{
						elp2open = true;
					}
				}
				else{
					logFailedNotNormal(elp2put->getName(), efem_process_name, efem_auto_step);
				}

				if (elpopen&&elp2open){
					efem_auto_step = 2501;
				}
			}
			break;
			case 2501:
			{
				if (lk2&&lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					auto cmd = lk2->createMoveToSlotCommand(sequence_lp2_put_wafer[0].slot_lk + efem_slot_offset);
					lk2->startCommand(cmd);
					cmd->wait();
					if (cmd->hasError())
					{
						logFailedExcuteCommandHasError(lk2->getName(), "LLB移动到slot", efem_process_name, efem_auto_step);
					}
					else
					{
						efem_auto_step = 2502;
					}
				}
				else{
					logFailedNotNormal(lk2->getName(), efem_process_name, efem_auto_step);
				}

			}
			break;
			case 2502:
			{
				if (lk2&&lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					if (!lk2->getCassetteDoorOpend()){
						auto cmdopendoor = lk2->createOpenCassetteDoorCommand();
						lk2->startCommand(cmdopendoor);
						cmdopendoor->wait();
						if (cmdopendoor->hasError())
						{
							logFailedExcuteCommandHasError(lk2->getName(), "LLB打开晶圆盒门", efem_process_name, efem_auto_step);
						}
						else{
							efem_auto_step = 2503;
							Sleep(2000);
						}
					}
					else{
						efem_auto_step = 2503;
						Sleep(2000);
					}
				}
				else{
					logFailedNotNormal(lk2->getName(), efem_process_name, efem_auto_step);
				}
			}
			break;
			case 2503:
			{
				if (ewtr&&ewtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					auto cmd1 = ewtr->createGetCommand(lk2, 1, sequence_lp2_put_wafer[0].slot_lk);
					ewtr->startCommand(cmd1);
					cmd1->wait();
					if (cmd1->hasError())
					{
						logFailedExcuteCommandHasError(ewtr->getName(), "A手臂取LLB晶圆", efem_process_name, efem_auto_step);
					}
					else
					{
						efem_auto_step = 2504;
					}
				}
				else{
					logFailedNotNormal(ewtr->getName(), efem_process_name, efem_auto_step);
				}
			}
			break;
			case 2504:
			{
				if (lk2&&lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					auto cmd4 = lk2->createMoveToSlotCommand(sequence_lp2_put_wafer[1].slot_lk + efem_slot_offset);
					lk2->startCommand(cmd4);
					cmd4->wait();
					if (cmd4->hasError())
					{
						logFailedExcuteCommandHasError(lk2->getName(), "LLB移动到slot", efem_process_name, efem_auto_step);
					}
					else
					{
						Sleep(2000);
						efem_auto_step = 2505;
					}
				}
				else{
					logFailedNotNormal(lk2->getName(), efem_process_name, efem_auto_step);
				}
			}
			break;
			case 2505:
			{
				if (ewtr&&ewtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					auto cmd1 = ewtr->createGetCommand(lk2, 2, sequence_lp2_put_wafer[1].slot_lk);
					ewtr->startCommand(cmd1);
					cmd1->wait();
					if (cmd1->hasError())
					{
						logFailedExcuteCommandHasError(ewtr->getName(), "B手臂取LLB晶圆", efem_process_name, efem_auto_step);
					}
					else
					{
						efem_auto_step = 2506;
					}
				}
				else{
					logFailedNotNormal(ewtr->getName(), efem_process_name, efem_auto_step);
				}
			}
			break;
			case 2506:
			{
				if (ewtr&&ewtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					std::shared_ptr<EFEMLPSubsystem> elp1put = kernel->getKernelModule<EFEMLPSubsystem>(sequence_lp2_put_wafer[0].source_lp);
					auto cmd3 = ewtr->createPutCommand(elp1put, 1, sequence_lp2_put_wafer[0].slot_lp);
					ewtr->startCommand(cmd3);
					cmd3->wait();
					if (cmd3->hasError())
					{
						logFailedExcuteCommandHasError(ewtr->getName(), "A手臂放LP2晶圆", efem_process_name, efem_auto_step);
					}
					else
					{
						efem_auto_step = 2507;
					}
				}
				else{
					logFailedNotNormal(ewtr->getName(), efem_process_name, efem_auto_step);
				}
			}
			break;
			case 2507:
			{
				if (ewtr&&ewtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					std::shared_ptr<EFEMLPSubsystem> elp2put = kernel->getKernelModule<EFEMLPSubsystem>(sequence_lp2_put_wafer[1].source_lp);
					auto cmd5 = ewtr->createPutCommand(elp2put, 2, sequence_lp2_put_wafer[1].slot_lp);
					ewtr->startCommand(cmd5);
					cmd5->wait();
					if (cmd5->hasError())
					{
						logFailedExcuteCommandHasError(ewtr->getName(), "B手臂放LP2晶圆", efem_process_name, efem_auto_step);
					}
					else
					{
						if (sequence_lp2_put_wafer[0].source_lp == "ELP1"){
							if (sequence_lp1_transfer_wafer.size() >= 1){
								sequence_lp1_transfer_wafer.erase(sequence_lp1_transfer_wafer.begin());
							}
							else{
								logError(ewtr->getName().c_str(), "ELP1 sequence_lp2_put_wafer=%d sequence_lp1_transfer_wafer=%d", sequence_lp2_put_wafer.size(), sequence_lp1_transfer_wafer.size());
							}

						}
						else if (sequence_lp2_put_wafer[0].source_lp == "ELP2"){
							if (sequence_lp2_transfer_wafer.size() >= 1){
								sequence_lp2_transfer_wafer.erase(sequence_lp2_transfer_wafer.begin());
							}
							else{
								logError(ewtr->getName().c_str(), "ELP2 sequence_lp2_put_wafer=%d sequence_lp2_transfer_wafer=%d", sequence_lp2_put_wafer.size(), sequence_lp2_transfer_wafer.size());
							}
						}

						if (sequence_lp2_put_wafer[1].source_lp == "ELP1"){
							if (sequence_lp1_transfer_wafer.size() >= 1){
								sequence_lp1_transfer_wafer.erase(sequence_lp1_transfer_wafer.begin());
							}
							else{
								logError(ewtr->getName().c_str(), "ELP1 sequence_lp2_put_wafer=%d sequence_lp1_transfer_wafer=%d", sequence_lp2_put_wafer.size(), sequence_lp1_transfer_wafer.size());
							}

						}
						else if (sequence_lp2_put_wafer[1].source_lp == "ELP2"){
							if (sequence_lp2_transfer_wafer.size() >= 1){
								sequence_lp2_transfer_wafer.erase(sequence_lp2_transfer_wafer.begin());
							}
							else{
								logError(ewtr->getName().c_str(), "ELP2 sequence_lp2_put_wafer=%d sequence_lp2_transfer_wafer=%d", sequence_lp2_put_wafer.size(), sequence_lp2_transfer_wafer.size());
							}
						}

						//if (sequence_lp1_put_wafer.size() == 2 && sequence_lp1_put_wafer[0].source_lp != sequence_lp1_put_wafer[1].source_lp
						//	&&sequence_lp1_put_wafer[0].source_lp == "ELP1"&&sequence_lp1_transfer_wafer.size() == 0){
						//	logInform(ewtr->getName().c_str(), "cycle end2 sequence_lp1_put_wafer=%d sequence_lp1_transfer_wafer=%d", sequence_lp1_put_wafer.size(), sequence_lp1_transfer_wafer.size());
						//	is_lp1_cycle = true;
						//}

						//if (sequence_lp1_put_wafer.size() == 2 && sequence_lp1_put_wafer[0].source_lp != sequence_lp1_put_wafer[1].source_lp
						//	&&sequence_lp1_put_wafer[0].source_lp == "ELP2"&&sequence_lp2_transfer_wafer.size() == 0){
						//	logInform(ewtr->getName().c_str(), "cycle end2 sequence_lp2_put_wafer=%d sequence_lp2_transfer_wafer=%d", sequence_lp2_put_wafer.size(), sequence_lp2_transfer_wafer.size());
						//	is_lp2_cycle = true;
						//}
						//
						if (sequence_lp2_put_wafer.size() == 2 && sequence_lp2_put_wafer[0].source_lp != sequence_lp2_put_wafer[1].source_lp
							&& sequence_lp2_put_wafer[0].source_lp == "ELP1"&&sequence_lp1_transfer_wafer.size() == 0){
							logInform(ewtr->getName().c_str(), "cycle end2 sequence_lp1_put_wafer=%d sequence_lp1_transfer_wafer=%d", sequence_lp1_put_wafer.size(), sequence_lp1_transfer_wafer.size());
							is_lp1_cycle = true;
						}
						if (sequence_lp2_put_wafer.size() == 2 && sequence_lp2_put_wafer[0].source_lp != sequence_lp2_put_wafer[1].source_lp
							&& sequence_lp2_put_wafer[0].source_lp == "ELP2"&&sequence_lp2_transfer_wafer.size() == 0){
							logInform(ewtr->getName().c_str(), "cycle end2 sequence_lp2_put_wafer=%d sequence_lp2_transfer_wafer=%d", sequence_lp2_put_wafer.size(), sequence_lp2_transfer_wafer.size());
							is_lp2_cycle = true;
						}

						if (sequence_lp2_put_wafer.size() == 2 && sequence_lp2_put_wafer[1].source_lp == "ELP1"&&sequence_lp1_transfer_wafer.size() == 0){
							logInform(ewtr->getName().c_str(), "cycle end sequence_lp1_put_wafer=%d sequence_lp1_transfer_wafer=%d", sequence_lp1_put_wafer.size(), sequence_lp1_transfer_wafer.size());
							is_lp1_cycle = true;
						}
						if (sequence_lp2_put_wafer.size() == 2 && sequence_lp2_put_wafer[1].source_lp == "ELP2"&&sequence_lp2_transfer_wafer.size() == 0){
							logInform(ewtr->getName().c_str(), "cycle end sequence_lp2_put_wafer=%d sequence_lp2_transfer_wafer=%d", sequence_lp2_put_wafer.size(), sequence_lp2_transfer_wafer.size());
							is_lp2_cycle = true;
						}
						sequence_lp2_put_wafer.erase(sequence_lp2_put_wafer.begin());
						sequence_lp2_put_wafer.erase(sequence_lp2_put_wafer.begin());
						/*if (sequence_lp2_transfer_wafer.size()>=2){
						sequence_lp2_transfer_wafer.erase(sequence_lp2_transfer_wafer.begin());
						sequence_lp2_transfer_wafer.erase(sequence_lp2_transfer_wafer.begin());
						}
						else if (sequence_lp2_transfer_wafer.size() >= 2){
						sequence_lp1_transfer_wafer.erase(sequence_lp1_transfer_wafer.begin());
						sequence_lp1_transfer_wafer.erase(sequence_lp1_transfer_wafer.begin());
						}
						else{
						logError(ewtr->getName().c_str(), "sequence_lp2_put_wafer=%d sequence_lp1_transfer_wafer=%d", sequence_lp2_put_wafer.size(), sequence_lp1_transfer_wafer.size());
						}*/
						efem_auto_step = 2300;
					}
				}
				else{
					logFailedNotNormal(ewtr->getName(), efem_process_name, efem_auto_step);
				}
			}
			break;
#pragma endregion
			#pragma endregion

#pragma region 模拟EFEM给TOOL1上料
			case 3000:
			{
				for (int i = 0; i < sequence_lp1_transfer_wafer.size(); i++)
				{
					LoadLockTransferWafer lktw;
					lktw.is_finish = false;
					lktw.slot = sequence_lp1_transfer_wafer[i].slot_lk;
					lktw.transfer = sequence_lp1_transfer_wafer[i].transfer;
					lktw.selected_arm = sequence_lp1_transfer_wafer[i].selected_arm;
					sequence_loadlock1_transfer_wafer.push_back(lktw);
					sequence_lp1_put_wafer.push_back(sequence_lp1_transfer_wafer[i]);//提前存入放料集合
				}
				if (!lk1->getCassetteDoorOpend()){
					auto cmdopendoor = lk1->createOpenCassetteDoorCommand();
					lk1->startCommand(cmdopendoor);
					cmdopendoor->wait();
					if (cmdopendoor->hasError())
					{
						logFailedExcuteCommandHasError(lk1->getName(), "LLA打开晶圆盒门", efem_process_name, efem_auto_step);
					}
				}
				Sleep(5000);
				logInform("LLA", "EFEM给LLA上料完成 数量%d %d", sequence_lp1_transfer_wafer.size(), sequence_loadlock1_transfer_wafer.size());
				tool1_allow_get_wafer = false;
				efem_auto_step = 10;
			}
			break;
#pragma endregion

#pragma region 模拟EFEM给TOOL1下料
			case 4000:
			{
				if (!lk1->getCassetteDoorOpend()){
					auto cmdopendoor = lk1->createOpenCassetteDoorCommand();
					lk1->startCommand(cmdopendoor);
					cmdopendoor->wait();
					if (cmdopendoor->hasError())
					{
						logFailedExcuteCommandHasError(lk1->getName(), "LLA打开晶圆盒门", efem_process_name, efem_auto_step);
					}
				}
				logInform("LLA", "EFEM给LLA下料完成 数量%d", sequence_lp1_transfer_wafer.size());
				sequence_lp1_put_wafer.clear();
				sequence_lp1_transfer_wafer.clear();
				Sleep(5000);
				tool1_allow_put_wafer = false;
				efem_auto_step = 10;
			}
			break;
#pragma endregion

#pragma region 模拟EFEM给TOOL2上料
			case 5000:
			{
				for (int i = 0; i < sequence_lp2_transfer_wafer.size(); i++)
				{
					LoadLockTransferWafer lktw;
					lktw.is_finish = false;
					lktw.slot = sequence_lp2_transfer_wafer[i].slot_lk;
					lktw.transfer = sequence_lp2_transfer_wafer[i].transfer;
					lktw.selected_arm = sequence_lp2_transfer_wafer[i].selected_arm;
					sequence_loadlock2_transfer_wafer.push_back(lktw);
					sequence_lp2_put_wafer.push_back(sequence_lp2_transfer_wafer[i]);//提前存入放料集合
				}
				if (!lk2->getCassetteDoorOpend()){
					auto cmdopendoor = lk2->createOpenCassetteDoorCommand();
					lk2->startCommand(cmdopendoor);
					cmdopendoor->wait();
					if (cmdopendoor->hasError())
					{
						logFailedExcuteCommandHasError(lk2->getName(), "LLB打开晶圆盒门", efem_process_name, efem_auto_step);
					}
				}
				Sleep(5000);
				logInform("LLB", "EFEM给LLB上料完成 数量%d %d", sequence_lp2_transfer_wafer.size(), sequence_loadlock2_transfer_wafer.size());
				tool2_allow_get_wafer = false;
				efem_auto_step = 10;
				
			}
			break;
#pragma endregion

#pragma region 模拟EFEM给TOOL2下料
			case 6000:
			{
				if (!lk2->getCassetteDoorOpend()){
					auto cmdopendoor = lk2->createOpenCassetteDoorCommand();
					lk2->startCommand(cmdopendoor);
					cmdopendoor->wait();
					if (cmdopendoor->hasError())
					{
						logFailedExcuteCommandHasError(lk2->getName(), "LLB打开晶圆盒门", efem_process_name, efem_auto_step);
					}
				}
				logInform("LLB", "EFEM给LLB下料完成 数量%d", sequence_lp2_transfer_wafer.size());
				sequence_lp2_put_wafer.clear();
				sequence_lp2_transfer_wafer.clear();
				Sleep(5000);
				tool2_allow_put_wafer = false;
				efem_auto_step = 10;
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
				if (lp1_cycle_one_time_finished &&!cycleFinished_lla){//LLA的Cycle已做完
					update_auto_step = 1030;
				}
				else if (lp2_cycle_one_time_finished &&!cycleFinished_llb){
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
				finished_time_lla++;
				if (is_lp1_cycle){
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
					logInform("Cycle", "LP1第%d次Cycle完成 copysize=%d size=%d robot_lla=%d robot=%d", finished_time_lla, sequence_lp1_transfer_wafer_copy.size(), sequence_lp1_transfer_wafer.size(), sequence_robot_transfer_wafer_lp1.size(), sequence_robot_transfer_wafer.size());
					// 将sequence_robot_transfer_wafer_lp1中的所有元素添加到sequence_robot_transfer_wafer的末尾
					if (sequence_robot_transfer_wafer_lp1.size()>0){
						std::lock_guard<std::mutex> lock1(vec_mutex_robot);
						sequence_robot_transfer_wafer.insert(sequence_robot_transfer_wafer.end(), sequence_robot_transfer_wafer_lp1.begin(), sequence_robot_transfer_wafer_lp1.end());
					}
					if (sequence_lp1_transfer_wafer_copy.size() > 0){
						std::lock_guard<std::mutex> lock(vec_mutex_lla);
						std::copy(sequence_lp1_transfer_wafer_copy.begin(), sequence_lp1_transfer_wafer_copy.end(), std::back_inserter(sequence_lp1_transfer_wafer));
					}
					if (sequence_tolk1_wafer_copy.size()>0 && sequence_tolk1_wafer.size()==0){
						std::copy(sequence_tolk1_wafer_copy.begin(), sequence_tolk1_wafer_copy.end(), std::back_inserter(sequence_tolk1_wafer));
					}
					if (sequence_tolk2_wafer_copy.size()>0 && sequence_tolk2_wafer.size() == 0){
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
					if (sequence_robot_transfer_wafer_lp2.size()>0){
						std::lock_guard<std::mutex> lock1(vec_mutex_robot);
						sequence_robot_transfer_wafer.insert(sequence_robot_transfer_wafer.end(), sequence_robot_transfer_wafer_lp2.begin(), sequence_robot_transfer_wafer_lp2.end());
					}

					if (sequence_lp2_transfer_wafer_copy.size() > 0){
						std::lock_guard<std::mutex> lock(vec_mutex_llb);
						std::copy(sequence_lp2_transfer_wafer_copy.begin(), sequence_lp2_transfer_wafer_copy.end(), std::back_inserter(sequence_lp2_transfer_wafer));
					}

					if (sequence_tolk1_wafer_copy.size()>0 && sequence_tolk1_wafer.size() == 0){
						std::copy(sequence_tolk1_wafer_copy.begin(), sequence_tolk1_wafer_copy.end(), std::back_inserter(sequence_tolk1_wafer));
					}
					if (sequence_tolk2_wafer_copy.size()>0 && sequence_tolk2_wafer.size() == 0){
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
		std::shared_ptr<FortrendPMCavitySubsystem> pm2 = kernel->getKernelModule<FortrendPMCavitySubsystem>("PM");
		//std::shared_ptr<FortrendPMCavitySubsystem> pm3 = kernel->getKernelModule<FortrendPMCavitySubsystem>("PM3");

		std::shared_ptr<FortrendAlignerSubsystem> aligner = kernel->getKernelModule<FortrendAlignerSubsystem>("Aligner");

		//check modules
		auto cassManager = wtr->getKernel()->getKernelModule<FortrendCassetteManager>();

		while (running)
		{
			robot_step_once_finished = false;
			switch (robot_auto_step)
			{
			case 10:
			{
				if (semi_auto_mode){
					SemiAutoPhase currentPhase = semi_auto_next_phase.load();
					bool allowRobotRun = false;
					if (currentPhase == SemiAutoPhase::PMLoad)
					{
						const int mask = semi_auto_pm_load_target_mask.load();
						bool found = false;
						if (sequence_robot_transfer_wafer.size() > 0)
						{
							std::lock_guard<std::mutex> lock(vec_mutex_robot);
							for (size_t i = 0; i < sequence_robot_transfer_wafer.size(); ++i)
							{
								const auto& item = sequence_robot_transfer_wafer[i];
								if (((mask & 1) != 0) && item.source_loadlock == lk1->getName() && loadlock1_allow_get_wafer)
								{
									if (i != 0)
									{
										std::swap(sequence_robot_transfer_wafer[0], sequence_robot_transfer_wafer[i]);
									}
									found = true;
									break;
								}
								if (((mask & 2) != 0) && item.source_loadlock == lk2->getName() && loadlock2_allow_get_wafer)
								{
									if (i != 0)
									{
										std::swap(sequence_robot_transfer_wafer[0], sequence_robot_transfer_wafer[i]);
									}
									found = true;
									break;
								}
							}
						}
						allowRobotRun = found;
					}
					else if (currentPhase == SemiAutoPhase::PMUnload)
					{
						allowRobotRun = (sequence_robot_get_wafer.size() > 0) && pm_allow_get_put_wafer && !pm_allow_goto_craft;
					}
					if (!allowRobotRun)
					{
						Sleep(50);
						break;
					}
				}
				if (sequence_robot_transfer_wafer.size() > 0 || sequence_robot_get_wafer.size() > 0){
					if (tm->getVacuumEnable() && (!tm->getTMCavityVacuumValueReachesTheSetValue() || !tm->getInsertingPlateValveOpend()))
					{
						robot_auto_step = 100;
					}
					else
					{
						robot_auto_step = 1000;
					}
				}
				else{
					Sleep(100);
				}

			}
			break;

			case 100:
			{
				if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					tm_get_vacuum = true;//开始抽TM腔真空
					robot_auto_step = 110;
				}
				else
				{
					logFailedNotNormal(tm->getName(), robot_process_name, robot_auto_step);
				}
			}
			break;
			case 110:
			{
				if (tm_get_vacuum == false)//TM腔抽真空完成
				{
					robot_auto_step = 120;
				}
				else
				{
					Sleep(100);
				}
			}
			break;
			case 120://等待三个腔室抽真空完成
			{
				//检测腔室的真空是否达到上限
				if (tm->getTMCavityVacuumValueUpperLimitReachesTheSetValue())
				{
					robot_auto_step = 1000;
				}
				else
				{
					if (!tm->getTMCavityVacuumValueUpperLimitReachesTheSetValue()){
						robot_auto_step = 100;
					}
					else{
						Sleep(100);
					}	
					
				}
			}
			break;


			case 1000:
			{
				//if (sequence_robot_transfer_wafer.size() == 0){
				//	robot_auto_step = 1020;
				//}
				//else if ((sequence_loadlock1_transfer_wafer.size() > 0 && !tool1_allow_get_wafer) || (sequence_loadlock2_transfer_wafer.size()>0 && !tool2_allow_get_wafer))
				//{//LL腔有待做工艺晶圆，且EFEM没在上料
				//	robot_auto_step = 1010;
				//}
				//else{
				//	Sleep(100);
				//}
				if (sequence_robot_transfer_wafer.size() > 0 || sequence_robot_get_wafer.size()>0)
				{
					robot_auto_step = 1010;
				}
				else{
					robot_auto_step = 1020;
				}
			}
			break;
			case 1010:
			{
				if (sequence_loadlock1_transfer_wafer.size() == 0 && sequence_loadlock1_put_wafer.size()>0
					&&sequence_robot_get_wafer.size() == 1 && sequence_robot_get_wafer[0].source_loadlock == lk1->getName()
					&& sequence_robot_get_wafer[0].target_loadlock == lk1->getName()){//最后一片料
					robot_selected_arm = sequence_robot_get_wafer[0].selected_arm;
					robot_auto_step = 4000;
				}
				else if (sequence_loadlock2_transfer_wafer.size() == 0 && sequence_loadlock2_put_wafer.size()>0
					&& sequence_robot_get_wafer.size() == 1 && sequence_robot_get_wafer[0].source_loadlock == lk2->getName()
					&& sequence_robot_get_wafer[0].target_loadlock == lk2->getName()){//最后一片料
					robot_selected_arm = sequence_robot_get_wafer[0].selected_arm;
					robot_auto_step = 4000;
				}
                else if (sequence_robot_transfer_wafer.size() > 0 && sequence_robot_transfer_wafer[0].source_loadlock == lk1->getName() && sequence_robot_transfer_wafer[0].target_loadlock == lk1->getName())
				{
					robot_selected_arm = sequence_robot_transfer_wafer[0].selected_arm;
					robot_auto_step = 2000;
				}
				else if (sequence_robot_get_wafer.size() > 0 && sequence_robot_get_wafer[0].source_loadlock == lk1->getName() && sequence_robot_get_wafer[0].target_loadlock == lk1->getName()){
					robot_selected_arm = sequence_robot_get_wafer[0].selected_arm;
					//logInform(wtr->getName().c_str(), "%s robot_auto_step = 4000 2= %d", wtr->getName(), robot_auto_step);
					robot_auto_step = 4000;
				}
				else if (sequence_robot_transfer_wafer.size() > 0 && sequence_robot_transfer_wafer[0].source_loadlock == lk2->getName() && sequence_robot_transfer_wafer[0].target_loadlock == lk2->getName())
				{
					robot_selected_arm = sequence_robot_transfer_wafer[0].selected_arm;
					robot_auto_step = 10000;
				}
				else if (sequence_robot_get_wafer.size() > 0 && sequence_robot_get_wafer[0].source_loadlock == lk2->getName() && sequence_robot_get_wafer[0].target_loadlock == lk2->getName()){
					robot_selected_arm = sequence_robot_get_wafer[0].selected_arm;
					//logInform(wtr->getName().c_str(), "%s robot_auto_step = 4000 3= %d", wtr->getName(), robot_auto_step);
					robot_auto_step = 4000;
				}
				else{
					logFailed(wtr->getName(), Poco::format("%s 传输顺序错误, 步骤 = %d", wtr->getName(), loadlock1_auto_step));
				}
			}
			break;
			case 1020:
			{
				if ((cycleFinished_lla || sequence_robot_transfer_wafer_lp1.size() == 0) && (cycleFinished_llb || sequence_robot_transfer_wafer_lp2.size() == 0)){
					logInform("Cycle", "WTR Cycle已完成");
					robot_auto_step = 10;
					onUpdateProcessControlEnabled(true);
					cycleFinished_lla = false;
					cycleFinished_llb = false;
					running = false;
				}
				else{

					if (sequence_robot_transfer_wafer.size() > 0 || sequence_robot_get_wafer.size()>0)
					{
						robot_auto_step = 1010;
					}
					else{
						robot_auto_step = 1020;
					}

					Sleep(100);
				}
				
			}
			break;

#pragma region 取LoadLock1的晶圆
			/* ------------------ 取LoadLock1的晶圆 ----------------------- */
			case 2000:
			{
				if (loadlock1_allow_get_wafer)
				{
					robot_auto_step = 2010;
				}
				else
				{
					Sleep(20);
				}

			}
			break;
			case 2010:
			{
				if (lk1->getVacuumValueUpperLimitReachesTheSetValue())
				{
					robot_auto_step = 2030;
				}
				else
				{
					loadlock1_get_vacuum = true;
					robot_auto_step = 2020;
				}
			}
			break;
			case 2020:
			{
				if (loadlock1_get_vacuum == false)
				{
					robot_auto_step = 2010;
				}
				else{
					Sleep(100);
				}
			}
			break;

			case 2030:
			{
				robot_auto_step = 2050;
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
				else{
					Sleep(100);
				}
			}
			break;
			case 2100:
			{
				if (lk1->getVacuumValueUpperLimitReachesTheSetValue() && tm->getTMCavityVacuumValueUpperLimitReachesTheSetValue())
				{
					robot_auto_step = 2110;
				}
				else{
					robot_auto_step = 2010;
				}
			}
			break;
			case 2110:
			{
				if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					//LoadLock1 打开传输腔门阀
					auto cmd = lk1->createOpenTMCavityDoorCommand();
					lk1->startCommand(cmd);
					cmd->wait();
					if (cmd->hasError())
					{
						logFailedExcuteCommandHasError(lk1->getName(), "打开传输腔门阀", robot_process_name, robot_auto_step);
					}
					else
					{
						robot_auto_step = 2130;
					}

				}
				else
				{
					logFailedNotNormal(lk1->getName(), robot_process_name, robot_auto_step);
				}
			}
			break;
			case 2130:
			{
				if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL && lk1->hasDoorOpend())
				{
					//robot 用手臂A 从loadlock1 取wafer 
					auto cmd = wtr->createGetCommand(lk1, robot_selected_arm, 1);
					wtr->startCommand(cmd);
					cmd->wait();
					if (cmd->hasError())
					{
						logFailedExcuteCommandHasError(wtr->getName(), "取晶圆", robot_process_name, robot_auto_step);
					}
					else{
						loadlock1_allow_get_wafer = false;
						robot_auto_step = 2200;
					}

				}
				else
				{
					logFailedNotNormal(wtr->getName(), robot_process_name, robot_auto_step);
				}
			}
			break;
			//case 2140:
			//{
			//	if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
			//	{
			//		//放寻边
			//		auto cmd = wtr->createPutCommand(aligner, robot_selected_arm, 1);
			//		wtr->startCommand(cmd);
			//		cmd->wait();
			//		if (cmd->hasError())
			//		{
			//			logFailedExcuteCommandHasError(wtr->getName(), "放晶圆", robot_process_name, robot_auto_step);
			//			auto alarm_msg = cmd->alarmMessage();
			//			robot_auto_step = -100;
			//		}
			//		else{
			//			robot_auto_step = 2150;
			//		}

			//	}
			//	else
			//	{
			//		logFailedNotNormal(wtr->getName(), robot_process_name, robot_auto_step);
			//	}
			//}
			//break;
			//case 2150:
			//{
			//	if (aligner->getState() == IKernelSubSystem::State::SUB_NORMAL)
			//	{
			//		//寻边
			//		auto cmd = aligner->createAlignCommand();
			//		aligner->startCommand(cmd);
			//		cmd->wait();
			//		if (cmd->hasError())
			//		{
			//			logFailedExcuteCommandHasError(aligner->getName(), "寻边", robot_process_name, robot_auto_step);
			//			auto alarm_msg = cmd->alarmMessage();
			//			robot_auto_step = -100;
			//		}
			//		else{
			//			robot_auto_step = 2160;
			//		}

			//	}
			//	else
			//	{
			//		logFailedNotNormal(aligner->getName(), robot_process_name, robot_auto_step);
			//	}
			//}
			//break;
			//case 2160:
			//{
			//	if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
			//	{
			//		auto cmd = wtr->createGetCommand(aligner, robot_selected_arm, 1);
			//		wtr->startCommand(cmd);
			//		cmd->wait();
			//		if (cmd->hasError())
			//		{
			//			logFailedExcuteCommandHasError(wtr->getName(), "取晶圆", robot_process_name, robot_auto_step);
			//			robot_auto_step = -100;
			//		}
			//		else{
			//			robot_auto_step = 2200;
			//		}

			//	}
			//	else
			//	{
			//		logFailedNotNormal(wtr->getName(), robot_process_name, robot_auto_step);
			//	}
			//}
			//break;

			case 2200:
			{
				robot_auto_step = 2250;
			}
			break;
			case 2250:
			{
                if (sequence_robot_transfer_wafer[0].pm2_is_enable)
				{
					robot_auto_step = 4000;
				}
				else if (sequence_robot_transfer_wafer[0].target_loadlock==lk2->getName())
				{
					robot_auto_step = 6000;
				}
				else if (sequence_robot_transfer_wafer[0].target_loadlock == lk1->getName()){
					robot_auto_step = 14000;
				}
				else{
					Sleep(100);
				}
			}
			break;
#pragma endregion

#pragma region 放PM，交换料，取PM
			/* ------------------ 放PM2的晶圆 ----------------------- */
			case 4000:
			{
				if (pm_allow_get_put_wafer)
				{
					robot_auto_step = 4001;
				}
				else
				{
					Sleep(20);
				}
			}
			break;
			case 4001:
			{
				if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					bool haswaferpm = cassManager->getCassette(pm2.get())->getMapping(1) == Cassette::Present;
					bool haswaferarm1 = cassManager->getCassette(wtr.get())->getMapping(1) == Cassette::Present;
					bool haswaferarm2 = cassManager->getCassette(wtr.get())->getMapping(2) == Cassette::Present;

					if (semi_auto_mode)
					{
						const SemiAutoPhase currentPhase = semi_auto_next_phase.load();
						if (currentPhase == SemiAutoPhase::PMLoad)
						{
							if (haswaferpm)
							{
								logFailed(wtr->getName(), "SemiAuto PMLoad: PM内已有晶圆，请先执行PM下料。");
								Sleep(50);
							}
							else if (robot_selected_arm == 0 && haswaferarm1)
							{
								robot_auto_step = 4010;
							}
							else if (robot_selected_arm == 1 && haswaferarm2)
							{
								robot_auto_step = 4030;
							}
							else
							{
								logFailed(wtr->getName(), "SemiAuto PMLoad: 机械手目标手臂无晶圆，无法上料。");
								Sleep(50);
							}
							break;
						}
						else if (currentPhase == SemiAutoPhase::PMUnload)
						{
							if (!haswaferpm)
							{
								logFailed(wtr->getName(), "SemiAuto PMUnload: PM内无晶圆，无法下料。");
								Sleep(50);
							}
							else
							{
								robot_auto_step = (robot_selected_arm == 0) ? 4090 : 4091;
							}
							break;
						}
					}

					if (!haswaferpm&&haswaferarm1){//手臂1放料
						robot_auto_step = 4010;
					}
					else if (!haswaferpm&&haswaferarm2){//手臂2放料
						robot_auto_step = 4030;
					}
					else if (haswaferpm&&!haswaferarm1&&haswaferarm2){//手1先取，手2后放
						robot_auto_step = 4050;
					}
					else if (haswaferpm&&haswaferarm1&&!haswaferarm2){//手2先取，手1后放
						robot_auto_step = 4070;
					}
					else if (haswaferpm&&!haswaferarm2&&!haswaferarm1){//两个手臂没料，A取
						robot_auto_step = 4090;
					}
					else{
						logFailedExcuteCommandHasError(wtr->getName(), "机械手晶圆状态不对", robot_process_name, robot_auto_step);
					}
				}
				else
				{
					logFailedNotNormal(wtr->getName(), robot_process_name, robot_auto_step);
				}
			}
			break;
			case 4010:
			{
				if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					robot_selected_arm = 0;
					auto cmd = wtr->createPutCommand(pm2, robot_selected_arm, 1);
					wtr->startCommand(cmd);
					cmd->wait();
					if (cmd->hasError())
					{
						logFailedExcuteCommandHasError(wtr->getName(), "放晶圆", robot_process_name, robot_auto_step);
					}
					else
					{
						pm_allow_get_put_wafer = false;
						pm_allow_goto_craft = true;
						if (semi_auto_mode)
						{
							semi_auto_pm_unload_allowed.store(false);
							onSemiAutoStageDone(SemiAutoPhase::PMLoad);
						}
						if (sequence_robot_transfer_wafer.size() > 0)
						{
							sequence_robot_get_wafer.push_back(sequence_robot_transfer_wafer[0]);
							robot_auto_step = 1000;
						}
					}
				}
				else
				{
					logFailedNotNormal(wtr->getName(), robot_process_name, robot_auto_step);
				}
			}
			break;
			case 4030:
			{
				if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					robot_selected_arm = 1;
					auto cmd = wtr->createPutCommand(pm2, robot_selected_arm, 1);
					wtr->startCommand(cmd);
					cmd->wait();
					if (cmd->hasError())
					{
						logFailedExcuteCommandHasError(wtr->getName(), "放晶圆", robot_process_name, robot_auto_step);
					}
					else
					{
						pm_allow_get_put_wafer = false;
						pm_allow_goto_craft = true;
						if (semi_auto_mode)
						{
							semi_auto_pm_unload_allowed.store(false);
							onSemiAutoStageDone(SemiAutoPhase::PMLoad);
						}
						if (sequence_robot_transfer_wafer.size() > 0)
						{
							sequence_robot_get_wafer.push_back(sequence_robot_transfer_wafer[0]);
							robot_auto_step = 1000;
						}
					}
				}
				else
				{
					logFailedNotNormal(wtr->getName(), robot_process_name, robot_auto_step);
				}
			}
			break;
			case 4050:
			{
				if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					robot_selected_arm = 0;
					auto cmd = wtr->createGetCommand(pm2, robot_selected_arm, 1);
					wtr->startCommand(cmd);
					cmd->wait();
					if (cmd->hasError())
					{
						logFailedExcuteCommandHasError(wtr->getName(), "取晶圆", robot_process_name, robot_auto_step);
					}
					else{
						if (sequence_robot_get_wafer.size() > 0){
							target_loadlock = sequence_robot_get_wafer[0].target_loadlock;
							sequence_robot_get_wafer.erase(sequence_robot_get_wafer.begin());
						}
						robot_auto_step = 4051;
					}
				}
				else
				{
					logFailedNotNormal(wtr->getName(), robot_process_name, robot_auto_step);
				}
			}
			break;
			case 4051:
			{
				if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					auto cmd = wtr->createPutCommand(pm2, 1, 1);
					wtr->startCommand(cmd);
					cmd->wait();
					if (cmd->hasError())
					{
						logFailedExcuteCommandHasError(wtr->getName(), "放晶圆", robot_process_name, robot_auto_step);
					}
					else
					{
						pm_allow_get_put_wafer = false;
						pm_allow_goto_craft = true;
						if (semi_auto_mode)
						{
							semi_auto_pm_unload_allowed.store(false);
							onSemiAutoStageDone(SemiAutoPhase::PMLoad);
						}
						robot_auto_step = 4140;
						if (sequence_robot_transfer_wafer.size() > 0)
						{
							sequence_robot_get_wafer.push_back(sequence_robot_transfer_wafer[0]);
						}
					}
				}
				else
				{
					logFailedNotNormal(wtr->getName(), robot_process_name, robot_auto_step);
				}
			}
			break;
			case 4070:
			{
				if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					robot_selected_arm = 1;
					auto cmd = wtr->createGetCommand(pm2, robot_selected_arm, 1);
					wtr->startCommand(cmd);
					cmd->wait();
					if (cmd->hasError())
					{
						logFailedExcuteCommandHasError(wtr->getName(), "取晶圆", robot_process_name, robot_auto_step);
					}
					else{
						if (sequence_robot_get_wafer.size() > 0){
							target_loadlock = sequence_robot_get_wafer[0].target_loadlock;
							sequence_robot_get_wafer.erase(sequence_robot_get_wafer.begin());
						}
						robot_auto_step = 4071;
					}

				}
				else
				{
					logFailedNotNormal(wtr->getName(), robot_process_name, robot_auto_step);
				}
			}
			break;
			case 4071:
			{
				if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					auto cmd = wtr->createPutCommand(pm2, 0, 1);
					wtr->startCommand(cmd);
					cmd->wait();
					if (cmd->hasError())
					{
						logFailedExcuteCommandHasError(wtr->getName(), "放晶圆", robot_process_name, robot_auto_step);
					}
					else
					{
						pm_allow_get_put_wafer = false;
						pm_allow_goto_craft = true;
						if (semi_auto_mode)
						{
							semi_auto_pm_unload_allowed.store(false);
							onSemiAutoStageDone(SemiAutoPhase::PMLoad);
						}
						robot_auto_step = 4140;
						if (sequence_robot_transfer_wafer.size() > 0)
						{
							sequence_robot_get_wafer.push_back(sequence_robot_transfer_wafer[0]);
						}
					}

				}
				else
				{
					logFailedNotNormal(wtr->getName(), robot_process_name, robot_auto_step);
				}
			}
			break;
			case 4090:
			{
				if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					robot_selected_arm = 0;
					auto cmd = wtr->createGetCommand(pm2, robot_selected_arm, 1);
					wtr->startCommand(cmd);
					cmd->wait();
					if (cmd->hasError())
					{
						logFailedExcuteCommandHasError(wtr->getName(), "取晶圆", robot_process_name, robot_auto_step);
					}
					else
					{
						if (sequence_robot_get_wafer.size() > 0){
							target_loadlock = sequence_robot_get_wafer[0].target_loadlock;
							sequence_robot_get_wafer.erase(sequence_robot_get_wafer.begin());
						}
						pm_allow_get_put_wafer = false;
						robot_auto_step = 4140;
					}
				}
				else
				{
					logFailedNotNormal(wtr->getName(), robot_process_name, robot_auto_step);
				}
			}
			break;
			case 4091:
			{
				if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					robot_selected_arm = 1;
					auto cmd = wtr->createGetCommand(pm2, robot_selected_arm, 1);
					wtr->startCommand(cmd);
					cmd->wait();
					if (cmd->hasError())
					{
						logFailedExcuteCommandHasError(wtr->getName(), "取晶圆", robot_process_name, robot_auto_step);
					}
					else
					{
						if (sequence_robot_get_wafer.size() > 0){
							target_loadlock = sequence_robot_get_wafer[0].target_loadlock;
							sequence_robot_get_wafer.erase(sequence_robot_get_wafer.begin());
						}
						pm_allow_get_put_wafer = false;
						robot_auto_step = 4140;
					}
				}
				else
				{
					logFailedNotNormal(wtr->getName(), robot_process_name, robot_auto_step);
				}
			}
			break;
			//case 4110:
			//{
			//	if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
			//	{
			//		bool haswaferpm = cassManager->getCassette(pm2.get())->getMapping(1) == Cassette::Present;
			//		bool haswaferarm1 = cassManager->getCassette(wtr.get())->getMapping(1) == Cassette::Present;
			//		bool haswaferarm2 = cassManager->getCassette(wtr.get())->getMapping(2) == Cassette::Present;
			//		
			//		if (!haswaferpm&&haswaferarm1){//手臂1放料
			//			robot_selected_arm = 0;
			//			auto cmd = wtr->createPutCommand(pm2, robot_selected_arm, 1);
			//			wtr->startCommand(cmd);
			//			cmd->wait();
			//			if (cmd->hasError())
			//			{
			//				logFailedExcuteCommandHasError(wtr->getName(), "放晶圆", robot_process_name, robot_auto_step);
			//			}
			//			else
			//			{
			//				pm_allow_get_put_wafer = false;
			//				pm_allow_goto_craft = true;
			//				if (sequence_robot_transfer_wafer.size() > 0)
			//				{
			//					sequence_robot_get_wafer.push_back(sequence_robot_transfer_wafer[0]);
			//				}
			//				robot_auto_step = 1000;
			//				//if (sequence_robot_transfer_wafer.size() > 0 && sequence_robot_transfer_wafer[0].target_loadlock == lk2->getName()){
			//				//	if (sequence_loadlock2_transfer_wafer.size() == 0 && sequence_loadlock2_put_wafer.size() == 1){
			//				//		robot_auto_step = 20000;
			//				//	}
			//				//	else{
			//				//		robot_auto_step = 6200;
			//				//	}

			//				//}
			//				//else{
			//				//	if (sequence_loadlock1_transfer_wafer.size() == 0 && sequence_loadlock1_put_wafer.size() == 1){//最后一片料
			//				//		robot_auto_step = 20000;
			//				//	}
			//				//	else{
			//				//		robot_auto_step = 14200;
			//				//	}

			//				//}
			//			}
			//		 }
			//		else if (!haswaferpm&&haswaferarm2){//手臂2放料
			//			robot_selected_arm = 1;
			//			auto cmd = wtr->createPutCommand(pm2, robot_selected_arm, 1);
			//			wtr->startCommand(cmd);
			//			cmd->wait();
			//			if (cmd->hasError())
			//			{
			//				logFailedExcuteCommandHasError(wtr->getName(), "放晶圆", robot_process_name, robot_auto_step);
			//			}
			//			else
			//			{
			//				pm_allow_get_put_wafer = false;
			//				pm_allow_goto_craft = true;
			//				if (sequence_robot_transfer_wafer.size() > 0)
			//				{
			//					sequence_robot_get_wafer.push_back(sequence_robot_transfer_wafer[0]);
			//				}
			//				robot_auto_step = 1000;
			//				//if (sequence_robot_transfer_wafer.size() > 0 && sequence_robot_transfer_wafer[0].target_loadlock == lk2->getName()){
			//				//	if (sequence_loadlock2_transfer_wafer.size() == 0 && sequence_loadlock2_put_wafer.size()==1){
			//				//		robot_auto_step = 20000;
			//				//	}
			//				//	else{
			//				//		robot_auto_step = 6200;
			//				//	}

			//				//}
			//				//else{
			//				//	if (sequence_loadlock1_transfer_wafer.size() == 0 && sequence_loadlock1_put_wafer.size() == 1){//最后一片料
			//				//		robot_auto_step = 20000;
			//				//	}
			//				//	else{
			//				//		robot_auto_step = 14200;
			//				//	}
			//				//	
			//				//}

			//					
			//			}
			//		}
			//		else if (haswaferpm&&!haswaferarm1&&haswaferarm2){//手1先取，手2后放
			//			robot_selected_arm = 0;
			//			auto cmd = wtr->createGetCommand(pm2, robot_selected_arm, 1);
			//			wtr->startCommand(cmd);
			//			cmd->wait();
			//			if (cmd->hasError())
			//			{
			//				logFailedExcuteCommandHasError(wtr->getName(), "取晶圆", robot_process_name, robot_auto_step);
			//			}
			//			else{
			//				if (sequence_robot_get_wafer.size() > 0){
			//					target_loadlock = sequence_robot_get_wafer[0].target_loadlock;
			//					sequence_robot_get_wafer.erase(sequence_robot_get_wafer.begin());
			//				}

			//				auto cmd = wtr->createPutCommand(pm2, 1, 1);
			//				wtr->startCommand(cmd);
			//				cmd->wait();
			//				if (cmd->hasError())
			//				{
			//					logFailedExcuteCommandHasError(wtr->getName(), "放晶圆", robot_process_name, robot_auto_step);
			//				}
			//				else
			//				{
			//					pm_allow_get_put_wafer = false;
			//					pm_allow_goto_craft = true;
			//					robot_auto_step = 4140;
			//					if (sequence_robot_transfer_wafer.size() > 0)
			//					{
			//						sequence_robot_get_wafer.push_back(sequence_robot_transfer_wafer[0]);
			//					}
			//				}
			//			}
			//		}
			//		else if (haswaferpm&&haswaferarm1&&!haswaferarm2){//手2先取，手1后放
			//			robot_selected_arm = 1;
			//			auto cmd = wtr->createGetCommand(pm2, robot_selected_arm, 1);
			//			wtr->startCommand(cmd);
			//			cmd->wait();
			//			if (cmd->hasError())
			//			{
			//				logFailedExcuteCommandHasError(wtr->getName(), "取晶圆", robot_process_name, robot_auto_step);
			//			}
			//			else{
			//				if (sequence_robot_get_wafer.size() > 0){
			//					target_loadlock = sequence_robot_get_wafer[0].target_loadlock;
			//					sequence_robot_get_wafer.erase(sequence_robot_get_wafer.begin());
			//				}
			//				auto cmd = wtr->createPutCommand(pm2, 0, 1);
			//				wtr->startCommand(cmd);
			//				cmd->wait();
			//				if (cmd->hasError())
			//				{
			//					logFailedExcuteCommandHasError(wtr->getName(), "放晶圆", robot_process_name, robot_auto_step);
			//				}
			//				else
			//				{
			//					pm_allow_get_put_wafer = false;
			//					pm_allow_goto_craft = true;
			//					robot_auto_step = 4140;
			//					if (sequence_robot_transfer_wafer.size() > 0)
			//					{
			//						sequence_robot_get_wafer.push_back(sequence_robot_transfer_wafer[0]);
			//					}
			//				}
			//			}
			//		}
			//		else if (haswaferpm&&!haswaferarm2&&!haswaferarm1){//两个手臂没料，A取
			//			robot_selected_arm = 0;
			//			auto cmd = wtr->createGetCommand(pm2, robot_selected_arm, 1);
			//			wtr->startCommand(cmd);
			//			cmd->wait();
			//			if (cmd->hasError())
			//			{
			//				logFailedExcuteCommandHasError(wtr->getName(), "取晶圆", robot_process_name, robot_auto_step);
			//			}
			//			else
			//			{
			//				if (sequence_robot_get_wafer.size() > 0){
			//					target_loadlock = sequence_robot_get_wafer[0].target_loadlock;
			//					sequence_robot_get_wafer.erase(sequence_robot_get_wafer.begin());
			//				}
			//				pm_allow_get_put_wafer = false;
			//				robot_auto_step = 4140;
			//			}
			//		}
			//		else{
			//			logFailedExcuteCommandHasError(wtr->getName(), "PM取晶圆", robot_process_name, robot_auto_step);
			//		}
			//	}
			//	else
			//	{
			//		logFailedNotNormal(wtr->getName(), robot_process_name, robot_auto_step);
			//	}
			//}
			//break;
			
			case 4140:
			{
				if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					if (target_loadlock==lk1->getName()){
						robot_auto_step = 14000;
					}
					else{
						robot_auto_step = 6000;
					}
					
				}
				else
				{
					logFailedNotNormal(wtr->getName(), robot_process_name, robot_auto_step);
				}
			}
			break;

#pragma endregion

#pragma region 放LoadLock2晶圆
			/* ------------------ 放LoadLock2的晶圆 ----------------------- */

			case 6000:
			{
				if (tm->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					robot_auto_step = 6005;
				}
				else{
					logFailedNotNormal(tm->getName(), robot_process_name, robot_auto_step);
				}
			}
			break;
			//新增寻边
			//case 6002:
			//{
			//	if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
			//	{
			//		//放寻边
			//		auto cmd = wtr->createPutCommand(aligner, robot_selected_arm, 1);
			//		wtr->startCommand(cmd);
			//		cmd->wait();
			//		if (cmd->hasError())
			//		{
			//			logFailedExcuteCommandHasError(wtr->getName(), "放晶圆", robot_process_name, robot_auto_step);
			//			auto alarm_msg = cmd->alarmMessage();
			//			robot_auto_step = -100;
			//		}
			//		else{
			//			robot_auto_step = 6003;
			//		}

			//	}
			//	else
			//	{
			//		logFailedNotNormal(wtr->getName(), robot_process_name, robot_auto_step);
			//	}
			//}
			//break;
			//case 6003:
			//{
			//	if (aligner->getState() == IKernelSubSystem::State::SUB_NORMAL)
			//	{
			//		//寻边
			//		auto cmd = aligner->createAlignCommand();
			//		aligner->startCommand(cmd);
			//		cmd->wait();
			//		if (cmd->hasError())
			//		{
			//			logFailedExcuteCommandHasError(aligner->getName(), "寻边", robot_process_name, robot_auto_step);
			//			auto alarm_msg = cmd->alarmMessage();
			//			robot_auto_step = -100;
			//		}
			//		else{
			//			robot_auto_step = 6004;
			//		}

			//	}
			//	else
			//	{
			//		logFailedNotNormal(aligner->getName(), robot_process_name, robot_auto_step);
			//	}
			//}
			//break;
			//case 6004:
			//{
			//	if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
			//	{
			//		auto cmd = wtr->createGetCommand(aligner, robot_selected_arm, 1);
			//		wtr->startCommand(cmd);
			//		cmd->wait();
			//		if (cmd->hasError())
			//		{
			//			logFailedExcuteCommandHasError(wtr->getName(), "取晶圆", robot_process_name, robot_auto_step);
			//			robot_auto_step = -100;
			//		}
			//		else{
			//			robot_auto_step = 6005;
			//		}

			//	}
			//	else
			//	{
			//		logFailedNotNormal(wtr->getName(), robot_process_name, robot_auto_step);
			//	}
			//}
			//break;

			case 6005:
			{
				if (loadlock2_allow_put_wafer)
				{
					robot_auto_step = 6010;
				}
				else
				{
					Sleep(100);

				}
			}
			break;


			case 6010:
			{
				if (lk2->getVacuumValueUpperLimitReachesTheSetValue())
				{
					robot_auto_step = 6030;
				}
				else
				{
					robot_auto_step = 6020;
					loadlock2_get_vacuum = true;
				}
			}
			break;
			case 6020:
			{
				if (loadlock2_get_vacuum == false)
				{
					robot_auto_step = 6010;
				}
				else
				{
					Sleep(100);

				}
			}
			break;
			case 6030://关闭角阀和插板阀
			{
				robot_auto_step = 6040;
			}
			break;
			case 6040:
			{
				if (tm->getTMCavityVacuumValueUpperLimitReachesTheSetValue())
				{
					robot_auto_step = 6100;
				}
				else{
					robot_auto_step = 6050;
					tm_get_vacuum = true;
				}

			}
			break;
			case 6050:
			{
				if (tm_get_vacuum == false)
				{
					robot_auto_step = 6040;
				}
				else
				{
					Sleep(100);

				}
			}
			break;

			case 6100:
			{

				if (tm->getTMCavityVacuumValueUpperLimitReachesTheSetValue() && lk2->getVacuumValueUpperLimitReachesTheSetValue())
				{
					robot_auto_step = 6105;
				}
				else{
					Sleep(100);
				}


			}
			break;
			case 6105:
			{
				robot_auto_step = 6106;
			}
			break;
			case 6106:
			{
				if (tm->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					auto cmd = tm->createCloseDiaphragmValveCommand(TMCavityValveOpening::TMCavity_Both);
					tm->startCommand(cmd);
					cmd->wait();
					if (cmd->hasError())
					{
						logFailedExcuteCommandHasError(tm->getName(), "关闭隔膜阀失败！", robot_process_name, robot_auto_step);
					}
					else
					{
						robot_auto_step = 6110;
					}
				}
				else{
					logFailedNotNormal(tm->getName(), robot_process_name, robot_auto_step);
				}
			}
			break;
			case 6110:
			{
				if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					auto cmd = lk2->createOpenTMCavityDoorCommand();
					lk2->startCommand(cmd);
					cmd->wait();
					if (cmd->hasError())
					{
						logFailedExcuteCommandHasError(lk2->getName(), "打开传输腔门阀", robot_process_name, robot_auto_step);
					}
					else
					{
						robot_auto_step = 6130;
					}

				}
				else
				{
					logFailedNotNormal(lk2->getName(), robot_process_name, robot_auto_step);

				}

			}
			break;

			case 6130:
			{
				if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL && lk2->hasDoorOpend())
				{
					auto cmd = wtr->createPutCommand(lk2, robot_selected_arm, 1);
					wtr->startCommand(cmd);
					cmd->wait();
					if (cmd->hasError())
					{
						logFailedExcuteCommandHasError(wtr->getName(), "放晶圆", robot_process_name, robot_auto_step);
					}
					else{
						if (semi_auto_mode && semi_auto_next_phase.load() == SemiAutoPhase::PMUnload){
							increaseReturnWaferCount(lk2->getName(), 1);
						}
						robot_auto_step = 6200;
					}

				}
				else
				{
					logFailedNotNormal(wtr->getName(), robot_process_name, robot_auto_step);

				}
			}
			break;
			case 6200:
			{
				loadlock2_allow_put_wafer = false;
				if (semi_auto_mode)
				{
					onSemiAutoStageDone(SemiAutoPhase::PMUnload);
				}
				robot_auto_step = 20000;
			}
			break;
			break;
#pragma endregion

#pragma region 取LoadLock2晶圆
			/* ---------------- LoadLock2 允许取料 --------------*/
			case 10000:
			{
				if (loadlock2_allow_get_wafer)
				{
					robot_auto_step = 10010;
				}
			}
			break;
			case 10010:
			{
				if (lk2->getVacuumValueUpperLimitReachesTheSetValue())
				{
					robot_auto_step = 10030;
				}
				else
				{
					robot_auto_step = 10020;
					loadlock2_get_vacuum = true;
				}
			}
			break;
			case 10020:
			{
				if (loadlock2_get_vacuum == false)
				{
					robot_auto_step = 10010;
				}
				else
				{
					Sleep(20);

				}
			}
			break;
			case 10030:
			{
				//if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
				//{
				//	//LoadLock2 关闭角阀
				//	auto cmd = lk2->createCloseAngleValveCommand();
				//	lk2->startCommand(cmd);
				//	cmd->wait();
				//	if (cmd->hasError())
				//	{
				//		logFailedExcuteCommandHasError(lk2->getName(), "关闭角阀", robot_process_name, robot_auto_step);
				//	}
				//	else
				//	{
				//		robot_auto_step = 10040;
				//	}

				//}
				//else
				//{
				//	logFailedNotNormal(lk2->getName(), robot_process_name, robot_auto_step);
				//}
				robot_auto_step = 10040;
			}
			break;
			case 10040:
			{

				if (tm->getTMCavityVacuumValueUpperLimitReachesTheSetValue())
				{
					robot_auto_step = 10100;
				}
				else{
					robot_auto_step = 10050;
					tm_get_vacuum = true;
				}


			}
			break;
			case 10050:
			{
				if (tm_get_vacuum == false)
				{
					robot_auto_step = 10040;
				}
				else
				{
					Sleep(20);

				}
			}
			break;
			case 10100:
			{

				if (tm->getTMCavityVacuumValueUpperLimitReachesTheSetValue() && lk2->getVacuumValueUpperLimitReachesTheSetValue())
				{
					robot_auto_step = 10110;
				}
				else
				{
					robot_auto_step = 10010;
				}


			}
			break;

			case 10110:
			{

				if (loadlock2_allow_get_wafer)
				{
					if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto cmd = lk2->createOpenTMCavityDoorCommand();
						lk2->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(lk2->getName(), "打开传输腔门阀", robot_process_name, robot_auto_step);
						}
						else
						{
							robot_auto_step = 10120;
						}
					}
					else{
						logFailedNotNormal(lk2->getName(), robot_process_name, robot_auto_step);
					}


				}
				else
				{
					Sleep(20);
				}
			}
			break;
			case 10120:
			{
				if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					//robot 用手臂A 取wafer 到 手臂A
					auto cmd = wtr->createGetCommand(lk2, robot_selected_arm, 1);
					wtr->startCommand(cmd);
					cmd->wait();
					if (cmd->hasError())
					{
						logFailedExcuteCommandHasError(wtr->getName(), "取晶圆", robot_process_name, robot_auto_step);
					}
					else{
						robot_auto_step = 10200;
					}

				}
				else
				{
					logFailedNotNormal(wtr->getName(), robot_process_name, robot_auto_step);
				}
			}
			break;
			//新增寻边
			//case 10121:
			//{
			//	if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
			//	{
			//		//放寻边
			//		auto cmd = wtr->createPutCommand(aligner, robot_selected_arm, 1);
			//		wtr->startCommand(cmd);
			//		cmd->wait();
			//		if (cmd->hasError())
			//		{
			//			logFailedExcuteCommandHasError(wtr->getName(), "放晶圆", robot_process_name, robot_auto_step);
			//			auto alarm_msg = cmd->alarmMessage();
			//			robot_auto_step = -100;
			//		}
			//		else{
			//			robot_auto_step = 10122;
			//		}

			//	}
			//	else
			//	{
			//		logFailedNotNormal(wtr->getName(), robot_process_name, robot_auto_step);
			//	}
			//}
			//break;
			//case 10122:
			//{
			//	if (aligner->getState() == IKernelSubSystem::State::SUB_NORMAL)
			//	{
			//		//寻边
			//		auto cmd = aligner->createAlignCommand();
			//		aligner->startCommand(cmd);
			//		cmd->wait();
			//		if (cmd->hasError())
			//		{
			//			logFailedExcuteCommandHasError(aligner->getName(), "寻边", robot_process_name, robot_auto_step);
			//			auto alarm_msg = cmd->alarmMessage();
			//			robot_auto_step = -100;
			//		}
			//		else{
			//			robot_auto_step = 10123;
			//		}

			//	}
			//	else
			//	{
			//		logFailedNotNormal(aligner->getName(), robot_process_name, robot_auto_step);
			//	}
			//}
			//break;
			//case 10123:
			//{
			//	if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
			//	{
			//		auto cmd = wtr->createGetCommand(aligner, robot_selected_arm, 1);
			//		wtr->startCommand(cmd);
			//		cmd->wait();
			//		if (cmd->hasError())
			//		{
			//			logFailedExcuteCommandHasError(wtr->getName(), "取晶圆", robot_process_name, robot_auto_step);
			//			robot_auto_step = -100;
			//		}
			//		else{
			//			robot_auto_step = 10200;
			//		}

			//	}
			//	else
			//	{
			//		logFailedNotNormal(wtr->getName(), robot_process_name, robot_auto_step);
			//	}
			//}
			//break;
			case 10200:
			{

				loadlock2_allow_get_wafer = false;
				robot_auto_step = 10500;
			}
			break;

			case 10500:
			{

				if (sequence_robot_transfer_wafer.size()>0&&sequence_robot_transfer_wafer[0].pm2_is_enable)//放PM
				{
					robot_auto_step = 4000;
				}
				else if (sequence_robot_transfer_wafer.size()>0 && !sequence_robot_transfer_wafer[0].pm2_is_enable){
					robot_auto_step = 14000;
				}
				else{
					Sleep(200);
				}
			}
			break;

#pragma endregion

#pragma region 放LoadLock1晶圆

			/* ---------------- LoadLock1 允许放料 --------------*/
			case 14000:
			{
				if (tm->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					robot_auto_step = 14005;
				}
				else
				{
					logFailedNotNormal(tm->getName(), robot_process_name, robot_auto_step);
				}
			}
			break;
			//新增寻边
			//case 14002:
			//{
			//	if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
			//	{
			//		//放寻边
			//		auto cmd = wtr->createPutCommand(aligner, robot_selected_arm, 1);
			//		wtr->startCommand(cmd);
			//		cmd->wait();
			//		if (cmd->hasError())
			//		{
			//			logFailedExcuteCommandHasError(wtr->getName(), "放晶圆", robot_process_name, robot_auto_step);
			//			auto alarm_msg = cmd->alarmMessage();
			//			robot_auto_step = -100;
			//		}
			//		else{
			//			robot_auto_step = 14003;
			//		}

			//	}
			//	else
			//	{
			//		logFailedNotNormal(wtr->getName(), robot_process_name, robot_auto_step);
			//	}
			//}
			//break;
			//case 14003:
			//{
			//	if (aligner->getState() == IKernelSubSystem::State::SUB_NORMAL)
			//	{
			//		//寻边
			//		auto cmd = aligner->createAlignCommand();
			//		aligner->startCommand(cmd);
			//		cmd->wait();
			//		if (cmd->hasError())
			//		{
			//			logFailedExcuteCommandHasError(aligner->getName(), "寻边", robot_process_name, robot_auto_step);
			//			auto alarm_msg = cmd->alarmMessage();
			//			robot_auto_step = -100;
			//		}
			//		else{
			//			robot_auto_step = 14004;
			//		}

			//	}
			//	else
			//	{
			//		logFailedNotNormal(aligner->getName(), robot_process_name, robot_auto_step);
			//	}
			//}
			//break;
			//case 14004:
			//{
			//	if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
			//	{
			//		auto cmd = wtr->createGetCommand(aligner, robot_selected_arm, 1);
			//		wtr->startCommand(cmd);
			//		cmd->wait();
			//		if (cmd->hasError())
			//		{
			//			logFailedExcuteCommandHasError(wtr->getName(), "取晶圆", robot_process_name, robot_auto_step);
			//			robot_auto_step = -100;
			//		}
			//		else{
			//			robot_auto_step = 14005;
			//		}

			//	}
			//	else
			//	{
			//		logFailedNotNormal(wtr->getName(), robot_process_name, robot_auto_step);
			//	}
			//}
			//break;
			case 14005:
			{
				if (loadlock1_allow_put_wafer)
				{
					robot_auto_step = 14010;
				}
				else{
					Sleep(100);
				}
			}
			break;

			case 14010:
			{
				if (lk1->getVacuumValueUpperLimitReachesTheSetValue())
				{
					robot_auto_step = 14030;
				}
				else
				{
					loadlock1_get_vacuum = true;
					robot_auto_step = 14020;
				}
			}
			break;
			case 14020:
			{
				if (loadlock1_get_vacuum == false)
				{
					robot_auto_step = 14010;
				}
				else{
					Sleep(100);
				}
			}
			break;

			case 14030://关闭角阀和插板阀
			{

				robot_auto_step = 14050;
			}
			break;
			case 14050:
			{
				if (tm->getTMCavityVacuumValueUpperLimitReachesTheSetValue())
				{
					robot_auto_step = 14100;
				}
				else
				{
					tm_get_vacuum = true;
					robot_auto_step = 14060;
				}
			}
			break;
			case 14060:
			{
				if (tm_get_vacuum == false)
				{
					robot_auto_step = 14050;
				}
				else{
					Sleep(100);
				}
			}
			break;
			case 14100:
			{
				if (lk1->getVacuumValueUpperLimitReachesTheSetValue() && tm->getTMCavityVacuumValueUpperLimitReachesTheSetValue())
				{
					//robot_auto_step = 14110;
					robot_auto_step = 14105;
				}
				else{
					Sleep(100);
				}
			}
			break;
			case 14105:
			{
				robot_auto_step = 14106;
			}
			break;
			case 14106:
			{
				if (tm->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					auto cmd = tm->createCloseDiaphragmValveCommand(TMCavityValveOpening::TMCavity_Both);
					tm->startCommand(cmd);
					cmd->wait();
					if (cmd->hasError())
					{
						logFailedExcuteCommandHasError(tm->getName(), "关闭隔膜阀失败！", robot_process_name, robot_auto_step);
					}
					else
					{
						robot_auto_step = 14110;
					}
				}
				else
				{
					logFailedNotNormal(tm->getName(), robot_process_name, robot_auto_step);
				}
			}
			break;
			case 14110:
			{
				if (loadlock1_allow_put_wafer && lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					auto cmd = lk1->createOpenTMCavityDoorCommand();
					lk1->startCommand(cmd);
					cmd->wait();
					if (cmd->hasError())
					{
						logFailedExcuteCommandHasError(lk1->getName(), "打开传输腔门阀", robot_process_name, robot_auto_step);
					}
					else
					{
						robot_auto_step = 14120;
					}

				}
				else
				{
					logFailedNotNormal(lk1->getName(), robot_process_name, robot_auto_step);
				}

			}
			break;
			case 14120:
			{
				if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL && lk1->hasDoorOpend())
				{
					//robot 把手臂A的放到LoadLock1
					auto cmd = wtr->createPutCommand(lk1, robot_selected_arm, 1);
					wtr->startCommand(cmd);
					cmd->wait();
					if (cmd->hasError())
					{
						//auto alarm_msg = cmd->alarmMessage();
						//if (alarm_msg->code() == 734)
						//{
						//	auto cmd_clear = wtr->createClearErrorCommand();
						//	wtr->startCommand(cmd_clear);
						//	cmd->wait();
						//	robot_auto_step = 14150;
						//}
						//else
						//{
						//	logFailedExcuteCommandHasError(wtr->getName(), "放晶圆", robot_process_name, robot_auto_step);
						//	//错误处理
						//}
						logFailedExcuteCommandHasError(wtr->getName(), "放晶圆", robot_process_name, robot_auto_step);
					}
					else{
						if (semi_auto_mode && semi_auto_next_phase.load() == SemiAutoPhase::PMUnload){
							increaseReturnWaferCount(lk1->getName(), 1);
						}
						robot_auto_step = 14200;
					}

				}
				else if (lk1->hasDoorOpend() == false)
				{
					logFailed(lk1->getName(), Poco::format("%s 传输腔门阀未打开， %s：%d", lk1->getName(), robot_process_name, robot_auto_step));
				}
				else
				{
					logFailedNotNormal(wtr->getName(), robot_process_name, robot_auto_step);
				}
			}
			break;
			case 14150:
			{
				if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					auto cmd = wtr->createCheckLoadCommand(robot_selected_arm, 20);
					wtr->startCommand(cmd);
					cmd->wait();
					if (cmd->hasError())
					{
						logFailedExcuteCommandHasError(wtr->getName(), "查询手指有无晶圆", robot_process_name, robot_auto_step);
					}
					robot_auto_step = 14160;
				}
				else
				{
					logFailedNotNormal(wtr->getName(), robot_process_name, robot_auto_step);
				}
			}
			break;
			case 14160:
			{
				if (wtr->hasObject(robot_selected_arm)){
					logFailed(wtr->getName(), Poco::format("机械手臂上有晶圆，放片失败。步骤：%d", robot_auto_step).c_str());
					robot_auto_step = 14150;
				}
				else
				{
					robot_auto_step = 14200;

				}
			}
			break;
			case 14200:
			{
				loadlock1_allow_put_wafer = false;
				if (semi_auto_mode)
				{
					onSemiAutoStageDone(SemiAutoPhase::PMUnload);
				}
				robot_auto_step = 20000;

			}
			break;
#pragma endregion
			case 20000:
			{
				if (sequence_robot_transfer_wafer.size() > 0)
				{
					std::lock_guard<std::mutex> lock(vec_mutex_robot);
					if (abortCycle){//中止当前Cycle
						sequence_robot_transfer_wafer.clear();
					}
					else{
						sequence_robot_transfer_wafer.erase(sequence_robot_transfer_wafer.begin());
					}
				}

				robot_auto_step = 1000;
			}
			break;
			default:
				break;
			}
			robot_step_once_finished = true;
		}
	}

	void QSlotTransferCycleVTMWidgetPrivate::startLoadLock1Action(){
		Q_Q(QSlotTransferCycleVTMWidget);
		std::shared_ptr<FortrendLoadLockSubsystem> lk1 = kernel->getKernelModule<FortrendLoadLockSubsystem>("LLA");

		while (running)
		{
			loadlock1_step_once_finished = false;
			switch (loadlock1_auto_step)
			{
			case 10:
			{
				loadlock1_process_finished = false;
				if (sequence_lp1_transfer_wafer.size() == 0 && !is_lp2_cycle&&is_lp1_cycle){
					loadlock1_auto_step = 6000;
				}
				if (sequence_robot_transfer_wafer.size()>0 && sequence_tolk1_wafer.size()>0
					&& sequence_robot_transfer_wafer[0].transfer != sequence_tolk1_wafer[0].transfer){
					Sleep(20000);
				}
				if (sequence_loadlock1_transfer_wafer.size() == 0 && sequence_tolk1_wafer.size()>0
					&& !tool1_allow_get_wafer&& !tool1_allow_put_wafer&&!is_lp1_cycle)//需要上料
				{
					if (semi_auto_mode && semi_auto_next_phase.load() != SemiAutoPhase::LLLoad)
					{
						Sleep(50);
						break;
					}
					if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						Sleep(2000);
						auto cmd = lk1->createMappingCommand();
						lk1->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(lk1->getName(), "扫描晶圆盒", loadlock1_process_name, loadlock1_auto_step);
						}
						else{
							loadlock1_auto_step = 20;
						}
					}
					else
					{
						logFailedNotNormal(lk1->getName(), loadlock1_process_name, loadlock1_auto_step);
					}

				}
				else if (sequence_loadlock1_transfer_wafer.size()>0 && !tool1_allow_get_wafer&& !tool1_allow_put_wafer&&!is_lp1_cycle)
				{
					loadlock1_auto_step = 400;
				}
				else{
					Sleep(50);
				}
			}
			break;
			case 20://需要上晶圆
			{
				//排气压力达到设定值9W9
				if (!lk1->getExhaustVacuumValueReachesTheSetValue())
				{
					loadlock1_auto_step = 100;
				}
				else
				{
					loadlock1_auto_step = 300;
				}

			}
			break;

			/* ---------------- 破真空流程 -------------------- */
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
					else{
						loadlock1_auto_step = 300;
					}
				}
				else{
					logFailedNotNormal(lk1->getName(), loadlock1_process_name, loadlock1_auto_step);
				}
			}
			break;

			//开门放Cassette流程
			case 300://破完真空
			{
				if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					/*auto cmd = lk1->createOpenCassetteDoorCommand();
					lk1->startCommand(cmd);
					cmd->wait();
					if (cmd->hasError())
					{
						logFailedExcuteCommandHasError(lk1->getName(), "打开放晶圆盒门阀", loadlock1_process_name, loadlock1_auto_step);
					}
					else{*/

						loadlock1_put_cassette_finished = false;
						loadlock1_auto_step = 301;
					//}
				}
				else
				{
					logFailedNotNormal(lk1->getName(), loadlock1_process_name, loadlock1_auto_step);
				}
			}
			break;
			case 301:
			{
				/*if (cycleFinished_lla){
					loadlock1_auto_step = 10;
					Sleep(20);
				}
				else{*/
					if (tool2_allow_get_wafer || tool2_allow_put_wafer){
						loadlock1_auto_step = 20;//LK2已经在上下料
						Sleep(1000);
					}
					else{
						tool1_allow_get_wafer = true;//呼叫EFEM上料
						loadlock1_auto_step = 302;
					}
				/*}*/
			}
			break;
			case 302:
			{
				if (!tool1_allow_get_wafer){//EFEM上料完成
					loadlock1_auto_step = 350;
					loadlock1_put_cassette_finished = true;
				}
				else{
					Sleep(50);
				}
			}
			break;
			case 350:
			{
				if (loadlock1_put_cassette_finished && !tool1_allow_get_wafer)//放料完成
				{
					loadlock1_auto_step = 400;
				}
				else
				{
					Sleep(50);
					//logFailedNotNormal(lk1->getName(), loadlock1_process_name, loadlock1_auto_step);
				}
			}
			break;
#pragma endregion
			case 400://有晶圆盒
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
					else{
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
					if (lk1->getVacuumEnable() && (!lk1->getVacuumValueReachesTheSetValue()||!lk1->getInsertingPlateValveOpend()))
					{
						loadlock1_auto_step = 500;
					}
					else
					{
						loadlock1_auto_step = 800;
					}
				}
				else{
					logFailedNotNormal(lk1->getName(), loadlock1_process_name, loadlock1_auto_step);
				}
			}
			break;
			case 500:
			{
				if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					loadlock1_get_vacuum = true;
					loadlock1_auto_step = 510;
				}
				else{
					logFailedNotNormal(lk1->getName(), loadlock1_process_name, loadlock1_auto_step);
				}
			}
			break;
			case 510:
			{
				if (loadlock1_get_vacuum == false)
				{
					if (lk1->getVacuumValueUpperLimitReachesTheSetValue())
					{
						loadlock1_auto_step = 800;
					}
					else
					{
						loadlock1_auto_step = 500;
					}
				}
				else{
					if (lk1->getVacuumValueReachesTheSetValue() && loadlock1_get_vacuum && (loadlock2_get_vacuum || tm_get_vacuum)){
						loadlock1_get_vacuum = false;//真空值已经达到，分子泵在抽其他腔室
					}
					Sleep(100);
				}
			}
			break;



			/* ---------------- Mapping流程 -------------------- */
			case 800:
			{
				if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					if (lk1->getVacuumEnable() && (!lk1->getVacuumValueReachesTheSetValue() || !lk1->getInsertingPlateValveOpend()))
					{
						loadlock1_auto_step = 500;
					}
					else
					{
						if (lk1->hasBoxPresent()){
							auto cmd = lk1->createMappingCommand();
							lk1->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError())
							{
								logFailedExcuteCommandHasError(lk1->getName(), "扫描晶圆盒", loadlock1_process_name, loadlock1_auto_step);
							}
							else{
								loadlock1_auto_step = 810;
							}
						}
						else{
							Sleep(100);
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
					if (semi_auto_mode){
						loadlock1_auto_step = 850;
					}
					else{
						loadlock1_auto_step = 900;
					}
				}
				else
				{
					logFailedNotNormal(lk1->getName(), loadlock1_process_name, loadlock1_auto_step);
				}
			}
			break;

			case 850:
			{
				if (semi_auto_next_phase.load() == SemiAutoPhase::LLLoad)
				{
					loadlock1_auto_step = 900;
				}
				else
				{
					Sleep(50);
				}
			}
			break;
			case 900:
			{
				/*if (sequence_loadlock1_put_wafer.size() == 0){
					robot_step_wafer_finished = true;
				}*/

				if (sequence_loadlock1_transfer_wafer.size() > 0 || sequence_loadlock1_put_wafer.size() >0)
				{
					/*if (sequence_loadlock1_put_wafer.size() >0){
						robot_step_wafer_finished = false;
					}*/
					loadlock1_auto_step = 910;
				}
				else
				{
					loadlock1_auto_step = 5000;
				}
			}
			break;
			case 910:
			{
				if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					if (lk1->getVacuumEnable() && (!lk1->getVacuumValueReachesTheSetValue() || !lk1->getInsertingPlateValveOpend()))
					{
						loadlock1_get_vacuum = true;
						loadlock1_auto_step = 920;
					}
					else{
						loadlock1_auto_step = 950;
					}
				}
				else{
					logFailedNotNormal(lk1->getName(), loadlock1_process_name, loadlock1_auto_step);
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
				else{
					if (lk1->getVacuumValueReachesTheSetValue() && loadlock1_get_vacuum && (loadlock2_get_vacuum || tm_get_vacuum)){
						loadlock1_get_vacuum = false;//真空值已经达到，分子泵在抽其他腔室
					}
					Sleep(100);
				}
			}
			break;
			case 950:
			{
				//if (sequence_loadlock1_put_wafer.size()>=2){
				//	loadlock1_auto_step = 2000;
				//}
				//else if ((sequence_loadlock1_transfer_wafer.size() - sequence_loadlock1_put_wafer.size())>0 && sequence_loadlock1_transfer_wafer.size()>0 && (sequence_loadlock1_transfer_wafer[0].transfer == "lk1_to_lk1" || sequence_loadlock1_transfer_wafer[0].transfer == "lk2_to_lk1") && !hasUPS)//hasUPS断电后不再取料，只放料
				//{
				//	loadlock1_auto_step = 1000;
				//}
				//else if (sequence_loadlock1_put_wafer.size()==1)//hasUPS断电后不再取料，只放料
				//{
				//	loadlock1_auto_step = 2000;
				//}
				//else
				//{
				//	Sleep(200);
				//}
				if (sequence_loadlock1_transfer_wafer.size()>0 && sequence_loadlock1_transfer_wafer[0].transfer == "lk1_to_lk2"&&!hasUPS&&!abortCycle)//hasUPS断电后不再取料，只放料
				{
					loadlock1_auto_step = 1000;
				}
				else if (sequence_loadlock1_put_wafer.size() >= 2){
					loadlock1_auto_step = 2000;
				}
				else if (sequence_loadlock1_transfer_wafer.size()>0 && sequence_loadlock1_transfer_wafer[0].transfer == "lk1_to_lk1"&&!hasUPS&&!abortCycle)
				{
					loadlock1_auto_step = 1000;
				}
				else
				{
					if (sequence_loadlock1_put_wafer.size() > 0){
						loadlock1_auto_step = 2000;
					}
					else{
						if (sequence_loadlock1_transfer_wafer.size()==0){
							loadlock1_auto_step = 5000;
						}
					}
				}

			}
			break;
#pragma region 允许取晶圆流程
			/* ---------------- 允许取晶圆流程 -------------------- */
			case 1000:
			{
				if (lk1->getWithWaferModeEnable() == false)
				{
					auto cassManager = lk1->getKernel()->getKernelModule<FortrendCassetteManager>();
					auto station_cass = cassManager->getCassette(lk1.get());
					auto all_mapping = station_cass->getAllMapping();
					int	slot = sequence_loadlock1_transfer_wafer[0].slot - 1;
					all_mapping[slot] = Cassette::Mapping::Present;
					std::vector<int> subSlots;
					for (int i = 0; i < lk1->getCassetteSlotCount(); i++){

						subSlots.push_back(i + 1);
					}
					station_cass->setMapping(subSlots, all_mapping);

				}
				loadlock1_move_slot_index = sequence_loadlock1_transfer_wafer[0].slot;
				logInform("Test", "loadlock1_move_slot_index %d loadlock1size=%d", loadlock1_move_slot_index, sequence_loadlock1_transfer_wafer.size());
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
						loadlock1_auto_step = 1030;
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
			case 1030:
			{
				if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					Sleep(500);
					auto cmd = lk1->createMoveToSlotCommand(loadlock1_move_slot_index);
					lk1->startCommand(cmd);
					cmd->wait();
					if (cmd->hasError())
					{
						logFailedExcuteCommandHasError(lk1->getName(), "移动到指定槽号", loadlock1_process_name, loadlock1_auto_step);
					}
					else{
						loadlock1_auto_step = 1040;
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
					loadlock1_allow_get_wafer = true;
					if (semi_auto_mode)
					{
						markSemiAutoLLDone(SemiAutoPhase::LLLoad, "LLA");
					}
					loadlock1_auto_step = 1050;
				}
				else
				{
					logFailedNotNormal(lk1->getName(), loadlock1_process_name, loadlock1_auto_step);
				}
			}
			break;
			case 1050:
			{
				if (loadlock1_allow_get_wafer == false)
				{
					if (sequence_loadlock1_transfer_wafer.size() > 0)
					{
						sequence_loadlock1_put_wafer.push_back(sequence_loadlock1_transfer_wafer[0]);
						std::lock_guard<std::mutex> lock(vec_mutex_lla);
						sequence_loadlock1_transfer_wafer.erase(sequence_loadlock1_transfer_wafer.begin());
					}

					auto cmd = lk1->createCloseTMCavityDoorCommand();
					lk1->startCommand(cmd);
					cmd->wait();
					if (cmd->hasError())
					{
						logFailedExcuteCommandHasError(lk1->getName(), "关闭传输腔门阀", loadlock1_process_name, loadlock1_auto_step);
					}
					else{
						loadlock1_auto_step = 1070;
					}
					
				}
				else
				{
					Sleep(20);
				}
			}
			break;
			case 1070:
			{
				if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					if (sequence_loadlock1_transfer_wafer.size() > 0 || sequence_loadlock1_put_wafer.size() >0)
					{
						loadlock1_auto_step = 900;
					}
					else
					{
						loadlock1_auto_step = 5000;
					}
				}
				else
				{
					logFailedNotNormal(lk1->getName(), loadlock1_process_name, loadlock1_auto_step);
				}
			}
			break;

#pragma endregion
			/* ---------------- 允许放晶圆流程 -------------------- */
			case 2000:
			{
				if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					loadlock1_move_slot_index = sequence_loadlock1_put_wafer[0].slot;
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
				break;
			case 2030:
			{
				if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					Sleep(500);
					auto cmd = lk1->createMoveToSlotCommand(loadlock1_move_slot_index);
					lk1->startCommand(cmd);
					cmd->wait();
					if (cmd->hasError())
					{
						logFailedExcuteCommandHasError(lk1->getName(), "移动到指定槽号", loadlock1_process_name, loadlock1_auto_step);
					}
					else{
						loadlock1_auto_step = 2040;
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
				if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					loadlock1_allow_put_wafer = true;
					loadlock1_auto_step = 2050;
				}
				else
				{
					logFailedNotNormal(lk1->getName(), loadlock1_process_name, loadlock1_auto_step);
				}
			}
			break;
			case 2050:
			{
				if (loadlock1_allow_put_wafer == false)
				{
					if (sequence_loadlock1_put_wafer.size() > 0)
					{
						sequence_loadlock1_put_wafer.erase(sequence_loadlock1_put_wafer.begin());
						if (sequence_loadlock1_put_wafer.size() == 0 && abortCycle){
							std::lock_guard<std::mutex> lock(vec_mutex_lla);
							sequence_loadlock1_transfer_wafer.clear();
						}
					}
					auto cmd = lk1->createCloseTMCavityDoorCommand();
					lk1->startCommand(cmd);
					cmd->wait();
					if (cmd->hasError())
					{
						logFailedExcuteCommandHasError(lk1->getName(), "关闭传输腔门阀", loadlock1_process_name, loadlock1_auto_step);
					}
					else{
						loadlock1_auto_step = 2070;
					}
				}
				else
				{
					Sleep(20);
				}
			}
			break;
			case 2070:
			{
				if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{

					if (sequence_loadlock1_transfer_wafer.size() > 0 || sequence_loadlock1_put_wafer.size() >0)
					{
						loadlock1_auto_step = 900;
					}
					else
					{
						loadlock1_auto_step = 5000;
					}
				}
				else
				{
					logFailedNotNormal(lk1->getName(), loadlock1_process_name, loadlock1_auto_step);
				}
			}
			break;




			/* ---------------- 出空Cassette流程 -------------------- */
			case 5000:
			{
				if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{

					auto cmd = lk1->createMappingCommand();
					lk1->startCommand(cmd);
					cmd->wait();
					if (cmd->hasError())
					{
						logFailedExcuteCommandHasError(lk1->getName(), "扫描晶圆盒", loadlock1_process_name, loadlock1_auto_step);
					}
					else{
						loadlock1_auto_step = semi_auto_mode ? 5020 : 5021;
					}
				}
				else
				{
					logFailedNotNormal(lk1->getName(), loadlock1_process_name, loadlock1_auto_step);
				}
			}
			break;
			case 5020:
			{
				if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					if (semi_auto_mode){
						if (semi_auto_next_phase.load() != SemiAutoPhase::LLUnload) {
							Sleep(50);
						}
						else if ((semi_auto_ll_unload_target_mask.load() & 1) == 0){
							Sleep(50);
						}
						else{
							loadlock1_auto_step = 5021;
						}
					}
					else{
						loadlock1_auto_step = 5021;
					}
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
					else{
						loadlock1_auto_step = 5022;
					}
				}
				else{
					logFailedNotNormal(lk1->getName(), loadlock1_process_name, loadlock1_auto_step);
				}
			}
			break;

			case 5022:
			{
				if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					if (lk1->getFastDiaphragmValveOpend() || lk1->getSlowDiaphragmValveOpend() || lk1->getVacuumValue() <= 99600){
						loadlock1_auto_step = 5021;
					}
					else{
					auto cmd = lk1->createOpenCassetteDoorCommand();
					lk1->startCommand(cmd);
					cmd->wait();
					if (cmd->hasError())
					{
						logFailedExcuteCommandHasError(lk1->getName(), "打开放晶圆盒门阀", loadlock1_process_name, loadlock1_auto_step);
					}
					else{
						loadlock1_auto_step = 5023;
					}
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
				tool1_allow_put_wafer = true;//呼叫EFEM下料
				loadlock1_auto_step = 5024;
			}
			break;
			case 5024:
			{
				if (!tool1_allow_put_wafer){//下料完成
					loadlock1_auto_step = 5025;
				}
				else{
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
					else{
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
				if (sequence_lp1_transfer_wafer.size() == 0 && !is_lp2_cycle&&is_lp1_cycle){
					lp1_cycle_one_time_finished = true;
					loadlock1_process_finished = true;
				}
				else{
					loadlock1_auto_step = 10;
				}
				
			}
			break;
			case 6010:
			{
				if (lp1_cycle_one_time_finished == false)
				{
					loadlock1_auto_step = 10;
				}
				else{
					Sleep(50);
				}
			}
			break;
			default:
				break;
			}
			}
			loadlock1_step_once_finished = true;
			Sleep(10);

		}
	}

	void QSlotTransferCycleVTMWidgetPrivate::startLoadLock2Action(){

		std::shared_ptr<FortrendLoadLockSubsystem> lk2 = kernel->getKernelModule<FortrendLoadLockSubsystem>("LLB");
		while (running)
		{
			loadlock2_step_once_finished = false;
			switch (loadlock2_auto_step)
			{
			case 10:
			{
				loadlock2_process_finished = false;
				if (sequence_lp2_transfer_wafer.size() == 0 && !is_lp1_cycle&&is_lp2_cycle){
					loadlock2_auto_step = 6000;
				}
				if (sequence_robot_transfer_wafer.size()>0 && sequence_tolk2_wafer.size()>0
					&& sequence_robot_transfer_wafer[0].transfer != sequence_tolk2_wafer[0].transfer){
					Sleep(20000);
				}
				if (sequence_loadlock2_transfer_wafer.size() == 0 && 
					sequence_tolk2_wafer.size()>0 && !tool2_allow_get_wafer&& !tool2_allow_put_wafer&&!is_lp2_cycle)// || 
				{
					if (semi_auto_mode && semi_auto_next_phase.load() != SemiAutoPhase::LLLoad)
					{
						Sleep(50);
						break;
					}
					if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						Sleep(2000);
						auto cmd = lk2->createMappingCommand();
						lk2->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(lk2->getName(), "扫描晶圆盒", loadlock2_process_name, loadlock2_auto_step);
						}
						else{
							loadlock2_auto_step = 20;
						}
					}
					else{
						logFailedNotNormal(lk2->getName(), loadlock2_process_name, loadlock2_auto_step);
					}
				}
				else if (sequence_loadlock2_transfer_wafer.size()>0 && !tool2_allow_get_wafer&& !tool2_allow_put_wafer&&!is_lp2_cycle)
				{
					loadlock2_auto_step = 400;
				}
				else{
					Sleep(50);
				}
			}
			break;
			case 20:
			{
				if (!lk2->getExhaustVacuumValueReachesTheSetValue())
				{
					loadlock2_auto_step = 100;
					//logInform(lk2->getName().c_str(), Poco::format("%s 步骤 = %d", lk2->getName(), loadlock2_auto_step).c_str());
				}
				else
				{
					loadlock2_auto_step = 300;
					//logInform(lk2->getName().c_str(), Poco::format("%s 步骤 = %d", lk2->getName(), loadlock2_auto_step).c_str());
				}

			}
			break;

			/* ---------------- 破真空流程 -------------------- */
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
					else{
						loadlock2_auto_step = 300;
					}
				}
				else{
					logFailedNotNormal(lk2->getName(), loadlock2_process_name, loadlock2_auto_step);
				}
			}
			break;

			//开门放Cassette流程
			case 300:
			{
				if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					if (lk2->getFastDiaphragmValveOpend() || lk2->getSlowDiaphragmValveOpend() || lk2->getVacuumValue() <= 99600){
						loadlock2_auto_step = 100;
					}
					else{
					/*auto cmd = lk2->createOpenCassetteDoorCommand();
					lk2->startCommand(cmd);
					cmd->wait();
					if (cmd->hasError())
					{
						logFailedExcuteCommandHasError(lk2->getName(), "打开放晶圆盒门阀", loadlock2_process_name, loadlock2_auto_step);
					}
					else{*/
						loadlock2_auto_step = 301;
					//}
				}
				}
				else
				{
					logFailedNotNormal(lk2->getName(), loadlock2_process_name, loadlock2_auto_step);
				}
			}
			break;
			case 301:
			{
				/*if (cycleFinished_llb){
					loadlock2_auto_step = 10;
					Sleep(20);
				}
				else{*/
					if (tool1_allow_get_wafer || tool1_allow_put_wafer){
						loadlock2_auto_step = 20;//LK1已经在上下料
						Sleep(1000);
					}
					else{
						tool2_allow_get_wafer = true;//呼叫EFEM上料
						loadlock2_auto_step = 302;
					}
				/*}*/
				
			}
			break;
			case 302:
			{
				if (!tool2_allow_get_wafer){//EFEM上料完成
					loadlock2_auto_step = 350;
					loadlock2_put_cassette_finished = true;
				}
				else{
					Sleep(50);
				}
			}
			break;
			case 350:
			{
				if (loadlock2_put_cassette_finished && !tool2_allow_get_wafer)//放料完成
				{
					loadlock2_auto_step = 400;
				}
				else
				{
					Sleep(50);
					//logFailedNotNormal(lk2->getName(), loadlock2_process_name, loadlock2_auto_step);
				}
			}
			break;
			case 400:
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
					else{
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
					if (lk2->getVacuumEnable() && (!lk2->getVacuumValueReachesTheSetValue() || !lk2->getInsertingPlateValveOpend()))
					{
						loadlock2_auto_step = 500;
					}
					else
					{
						loadlock2_auto_step = 800;
					}
				}
				else
				{
					logFailedNotNormal(lk2->getName(), loadlock2_process_name, loadlock2_auto_step);
				}

			}
			break;


			case 500:
			{
				if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					loadlock2_get_vacuum = true;
					loadlock2_auto_step = 510;
				}
				else{
					logFailedNotNormal(lk2->getName(), loadlock2_process_name, loadlock2_auto_step);
				}
			}
			break;
			case 510:
			{
				if (loadlock2_get_vacuum == false)
				{
					if (lk2->getVacuumValueUpperLimitReachesTheSetValue())
					{
						loadlock2_auto_step = 800;
					}
					else
					{
						loadlock2_auto_step = 500;
					}
				}
				else{
					if (lk2->getVacuumValueReachesTheSetValue() && loadlock2_get_vacuum && (loadlock1_get_vacuum || tm_get_vacuum)){
						loadlock2_get_vacuum = false;//真空值已经达到，分子泵在抽其他腔室
					}
					Sleep(100);
				}
			}
			break;



			/* ---------------- Mapping流程 -------------------- */
			case 800:
			{
				if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					auto cmd = lk2->createMappingCommand();
					lk2->startCommand(cmd);
					cmd->wait();
					if (cmd->hasError())
					{
						logFailedExcuteCommandHasError(lk2->getName(), "扫描晶圆盒", loadlock2_process_name, loadlock2_auto_step);
					}
					else{
						loadlock2_auto_step = 810;
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
					if (semi_auto_mode){
						loadlock2_auto_step = 850;
					}
					else{
						loadlock2_auto_step = 900;
					}
				}
				else
				{
					logFailedNotNormal(lk2->getName(), loadlock2_process_name, loadlock2_auto_step);
				}
			}
			break;

			case 850:
			{
				if (semi_auto_next_phase.load() == SemiAutoPhase::LLLoad)
				{
					loadlock2_auto_step = 900;
				}
				else
				{
					Sleep(50);
				}
			}
			break;
			case 900:
			{
				if (sequence_loadlock2_put_wafer.size() == 0){
					robot_step_wafer_finished = true;
				}

				if (sequence_loadlock2_transfer_wafer.size() > 0 || sequence_loadlock2_put_wafer.size() >0)
				{
					if (sequence_loadlock2_put_wafer.size() >0){
						robot_step_wafer_finished = false;
					}
					loadlock2_auto_step = 910;
				}
				else
				{
					loadlock2_auto_step = 5000;
				}

				
			}
			break;
			case 910:
			{
				if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					if (lk2->getVacuumEnable() && (!lk2->getVacuumValueReachesTheSetValue() || !lk2->getInsertingPlateValveOpend()))
					{
					loadlock2_get_vacuum = true;
					loadlock2_auto_step = 920;
					}else{
						loadlock2_auto_step = 950;
					}
				}
				else{
					logFailedNotNormal(lk2->getName(), loadlock2_process_name, loadlock2_auto_step);
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
				else{
					if (lk2->getVacuumValueReachesTheSetValue() && loadlock2_get_vacuum && (loadlock1_get_vacuum || tm_get_vacuum)){
						loadlock2_get_vacuum = false;//真空值已经达到，分子泵在抽其他腔室
					}
					Sleep(100);
				}
			}
			break;
			case 950:
			{
				//if (sequence_loadlock2_transfer_wafer[0].transfer=="lk2_to_lk1")
				//{
				//	loadlock2_auto_step = 1000;
				//}
				//else if (sequence_loadlock2_transfer_wafer[0].transfer == "lk2_to_lk2")
				//{
				//	loadlock2_auto_step = 1000;
				//}
				//else if (sequence_loadlock2_transfer_wafer[0].transfer == "lk1_to_lk2")
				//{
				//	loadlock2_auto_step = 2000;
				//}
				//else
				//{
				//	//loadlock2_auto_step = 2000;
				//	Sleep(100);
				//}

				if (sequence_loadlock2_transfer_wafer.size()>0 && sequence_loadlock2_transfer_wafer[0].transfer == "lk2_to_lk1"&&!hasUPS&&!abortCycle)//hasUPS断电后不再取料，只放料
				{
					loadlock2_auto_step = 1000;
				}
				else if (sequence_loadlock2_put_wafer.size() >= 2){
					loadlock2_auto_step = 2000;
				}
				else if (sequence_loadlock2_transfer_wafer.size()>0 && sequence_loadlock2_transfer_wafer[0].transfer == "lk2_to_lk2"&&!hasUPS&&!abortCycle)
				{
					loadlock2_auto_step = 1000;
				}
				else
				{
					if (sequence_loadlock2_put_wafer.size() > 0){
						loadlock2_auto_step = 2000;
					}
					else{
						if (sequence_loadlock2_transfer_wafer.size() == 0){
							loadlock2_auto_step = 5000;
						}
					}
				}
			}
			break;

			/* ---------------- 允许取晶圆流程 -------------------- */
			case 1000:
			{
				if (lk2->getWithWaferModeEnable() == false)
				{
					auto cassManager = lk2->getKernel()->getKernelModule<FortrendCassetteManager>();
					auto station_cass = cassManager->getCassette(lk2.get());
					auto all_mapping = station_cass->getAllMapping();
					int	slot = sequence_loadlock2_transfer_wafer[0].slot - 1;
					all_mapping[slot] = Cassette::Mapping::Present;
					std::vector<int> subSlots;
					for (int i = 0; i < lk2->getCassetteSlotCount(); i++){

						subSlots.push_back(i + 1);
					}
					station_cass->setMapping(subSlots, all_mapping);

				}
				loadlock2_move_slot_index = sequence_loadlock2_transfer_wafer[0].slot;
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
						loadlock2_auto_step = 1030;
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
			case 1030:
			{
				if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					Sleep(500);
					auto cmd = lk2->createMoveToSlotCommand(loadlock2_move_slot_index);
					lk2->startCommand(cmd);
					cmd->wait();
					if (cmd->hasError())
					{
						logFailedExcuteCommandHasError(lk2->getName(), "移动到指定槽号", loadlock2_process_name, loadlock2_auto_step);
					}
					else{
						Sleep(50);
						loadlock2_auto_step = 1040;
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
					loadlock2_allow_get_wafer = true;
					if (semi_auto_mode)
					{
						markSemiAutoLLDone(SemiAutoPhase::LLLoad, "LLB");
					}
					loadlock2_auto_step = 1050;
				}
				else
				{
					logFailedNotNormal(lk2->getName(), loadlock2_process_name, loadlock2_auto_step);
				}
			}
			break;
			case 1050:
			{
				if (loadlock2_allow_get_wafer == false)
				{
					if (sequence_loadlock2_transfer_wafer.size() > 0)
					{
						sequence_loadlock2_put_wafer.push_back(sequence_loadlock2_transfer_wafer[0]);
						std::lock_guard<std::mutex> lock(vec_mutex_llb);
						sequence_loadlock2_transfer_wafer.erase(sequence_loadlock2_transfer_wafer.begin());
					}
					auto cmd = lk2->createCloseTMCavityDoorCommand();
					lk2->startCommand(cmd);
					cmd->wait();
					if (cmd->hasError())
					{
						logFailedExcuteCommandHasError(lk2->getName(), "关闭传输腔门阀", loadlock2_process_name, loadlock2_auto_step);
					}
					else{
						loadlock2_auto_step = 1070;
					}
				}
				else
				{
					Sleep(20);
				}
			}
			break;
			case 1070:
			{
				if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					if (sequence_loadlock2_transfer_wafer.size() > 0 || sequence_loadlock2_put_wafer.size() >0)
					{
						loadlock2_auto_step = 900;
					}
					else
					{
						loadlock2_auto_step = 5000;
					}
				}
				else
				{
					logFailedNotNormal(lk2->getName(), loadlock2_process_name, loadlock2_auto_step);
				}
			}
			break;


			/* ---------------- 允许放晶圆流程 -------------------- */
			case 2000:
			{
				if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					loadlock2_move_slot_index = sequence_loadlock2_put_wafer[0].slot;
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
						logFailed(lk2->getName(), Poco::format("%s 第%d槽不是空片,loadlock2步骤= %d", lk2->getName(), loadlock2_move_slot_index, loadlock2_auto_step));
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
					Sleep(500);
					auto cmd = lk2->createMoveToSlotCommand(loadlock2_move_slot_index);
					lk2->startCommand(cmd);
					cmd->wait();
					if (cmd->hasError())
					{
						logFailedExcuteCommandHasError(lk2->getName(), "移动到指定槽号", loadlock2_process_name, loadlock2_auto_step);
					}
					else{
						Sleep(50);
						loadlock2_auto_step = 2040;
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
				if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					loadlock2_allow_put_wafer = true;
					loadlock2_auto_step = 2050;
				}
				else
				{
					logFailedNotNormal(lk2->getName(), loadlock2_process_name, loadlock2_auto_step);
				}
			}
			break;
			case 2050:
			{
				if (loadlock2_allow_put_wafer == false)
				{
					if (sequence_loadlock2_put_wafer.size() > 0)
					{
						sequence_loadlock2_put_wafer.erase(sequence_loadlock2_put_wafer.begin());
						if (sequence_loadlock2_put_wafer.size()==0&&abortCycle){
							std::lock_guard<std::mutex> lock(vec_mutex_llb);
							sequence_loadlock2_transfer_wafer.clear();
						}
					}
					auto cmd = lk2->createCloseTMCavityDoorCommand();
					lk2->startCommand(cmd);
					cmd->wait();
					if (cmd->hasError())
					{
						logFailedExcuteCommandHasError(lk2->getName(), "关闭传输腔门阀", loadlock2_process_name, loadlock2_auto_step);
					}
					else{
						loadlock2_auto_step = 2070;

					}
				}
				else
				{
					Sleep(20);
				}
			}
			break;
			case 2070:
			{
				if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{

					if (sequence_loadlock2_transfer_wafer.size() > 0 || sequence_loadlock2_put_wafer.size() >0)
					{
						loadlock2_auto_step = 900;
					}
					else
					{
						loadlock2_auto_step = 5000;
					}
				}
				else
				{
					logFailedNotNormal(lk2->getName(), loadlock2_process_name, loadlock2_auto_step);
				}
			}
			break;


			/* ---------------- 出Cassette流程 -------------------- */
			case 5000:
			{
				if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					auto cmd = lk2->createMappingCommand();
					lk2->startCommand(cmd);
					cmd->wait();
					if (cmd->hasError())
					{
						logFailedExcuteCommandHasError(lk2->getName(), "扫描晶圆盒", loadlock2_process_name, loadlock2_auto_step);
					}
					else{
						loadlock2_auto_step = semi_auto_mode ? 5020 : 5021;
					}
				}
				else
				{
					logFailedNotNormal(lk2->getName(), loadlock2_process_name, loadlock2_auto_step);
				}

			}
			break;
			case 5010:
			{
				Sleep(50);
			}
			break;
			case 5011:
			{
				Sleep(50);
			}
			break;
			case 5020:
			{
				if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					if (semi_auto_mode){
						if (semi_auto_next_phase.load() != SemiAutoPhase::LLUnload) {
							Sleep(50);
						}
						else if ((semi_auto_ll_unload_target_mask.load() & 2) == 0){
							Sleep(50);
						}
						else{
							loadlock2_auto_step = 5021;
						}
					}
					else{
						loadlock2_auto_step = 5021;
					}
				}
				else{
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
					else{
						loadlock2_auto_step = 5022;
					}
				}
				else{
					logFailedNotNormal(lk2->getName(), loadlock2_process_name, loadlock2_auto_step);
				}
			}
			break;

			case 5022:
			{
				if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
				{
					if (lk2->getFastDiaphragmValveOpend() || lk2->getSlowDiaphragmValveOpend() || lk2->getVacuumValue() <= 99600){
						loadlock2_auto_step = 5021;
					}
					else{
					auto cmd = lk2->createOpenCassetteDoorCommand();
					lk2->startCommand(cmd);
					cmd->wait();
					if (cmd->hasError())
					{
						logFailedExcuteCommandHasError(lk2->getName(), "打开放晶圆盒门阀", loadlock2_process_name, loadlock2_auto_step);
					}
					else{
						loadlock2_auto_step = 5023;
					}
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
				tool2_allow_put_wafer = true;//呼叫EFEM下料
				loadlock2_auto_step = 5024;
			}
			break;
			case 5024:
			{
				if (!tool2_allow_put_wafer){//下料完成
					loadlock2_auto_step = 5025;
				}
				else{
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
					else{
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
				    if (sequence_lp2_transfer_wafer.size() == 0 && !is_lp1_cycle&&is_lp2_cycle){
						lp2_cycle_one_time_finished = true;
						loadlock2_process_finished = true;
					}
					else{
						loadlock2_auto_step = 10;
					}
			}
			break;
			case 6010:
			{
				if (lp2_cycle_one_time_finished == false)
				{
					loadlock2_auto_step = 10;
				}
				else{
					Sleep(50);
				}
				

			}
			break;
			default:
				break;
			}
			loadlock2_step_once_finished = true;
			Sleep(10);
		}

	}

	void QSlotTransferCycleVTMWidgetPrivate::startPMAction(){
		std::shared_ptr<FortrendPMCavitySubsystem> pm2 = kernel->getKernelModule<FortrendPMCavitySubsystem>("PM");
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
				setProcessColorState(pm2->getKernel(), pm2->getName(), 1, WaferProcessColorService::ProcessColorState::Normal);
				refreshCassetteColorDisplay(pm2->getKernel(), pm2);
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
							setProcessColorState(pm2->getKernel(), pm2->getName(), 1, WaferProcessColorService::ProcessColorState::Processed);
							refreshCassetteColorDisplay(pm2->getKernel(), pm2);
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
		sequence_robot_transfer_wafer_lp1.clear();
		sequence_robot_transfer_wafer_lp2.clear();
		sequence_loadlock1_transfer_wafer.clear();
		sequence_loadlock2_transfer_wafer.clear();
		sequence_loadlock1_put_wafer.clear();
		sequence_loadlock2_put_wafer.clear();
		sequence_lp1_transfer_wafer.clear();
		sequence_lp2_transfer_wafer.clear();

		semi_auto_next_phase.store(SemiAutoPhase::None);
		resetSemiAutoLLFlags();
		semi_auto_pm_load_target_mask.store(3);
		semi_auto_ll_unload_target_mask.store(3);
		semi_auto_pm_processing.store(false);
		semi_auto_pm_unload_allowed.store(false);
		return_wafer_count_lla.store(0);
		return_wafer_count_llb.store(0);
		//semi_auto_completed_phase.store(0);
		//{
		//	std::lock_guard<std::mutex> lock(semi_auto_mutex);
		//	semi_auto_request = SemiAutoRequest();
		//}

		sequence_lp1_transfer_wafer_copy.clear();
		sequence_lp2_transfer_wafer_copy.clear();
		sequence_tolk1_wafer.clear();
		sequence_tolk2_wafer.clear();
		sequence_tolk1_wafer_copy.clear();
		sequence_tolk2_wafer_copy.clear();

		int lk1slot = 1;
		int lk2slot = 1;
		int lp1num = 1;
		int lp2num = 1;
		int lp1num2 = 1;
		int lp2num2 = 1;
		std::string lastlp = "";
		std::string lasttransfer = "";
		bool lk1_goto_lk2 = false;
		bool lk2_goto_lk1 = false;
		//新增不规则晶圆数计算LL腔槽号
		int currentSlotNumber = 1;
		bool isRobotLLA = true;

		//2026-4-15 新增LL腔槽数设置，默认为3
		int LL_slot_count = 3;

		if (semi_auto_mode)
		{
			logInform("Cycle", "半自动 setTransferSequence: rowCount=%d", ui->semi_auto_sequence_tbw->rowCount());
			int validRows = 0;
			int elp1Rows = 0;
			int elp2Rows = 0;
			int llaRows = 0;
			int llbRows = 0;
			for (int i = 0; i < ui->semi_auto_sequence_tbw->rowCount(); ++i) {
				QComboBox *foup_slot = qobject_cast<QComboBox*>(ui->semi_auto_sequence_tbw->cellWidget(i, 0));
				QComboBox *elp = qobject_cast<QComboBox*>(ui->semi_auto_sequence_tbw->cellWidget(i, 1));
				QComboBox *ll = qobject_cast<QComboBox*>(ui->semi_auto_sequence_tbw->cellWidget(i, 2));
				QComboBox *ll_slot = qobject_cast<QComboBox*>(ui->semi_auto_sequence_tbw->cellWidget(i, 3));
				QComboBox *arm = qobject_cast<QComboBox*>(ui->semi_auto_sequence_tbw->cellWidget(i, 4));
				QCheckBox *pm2_enable = qobject_cast<QCheckBox*>(ui->semi_auto_sequence_tbw->cellWidget(i, 5));
				if (!foup_slot || !elp || !ll || !ll_slot || !arm || !pm2_enable)
				{
					logInform("Cycle", "半自动 setTransferSequence: row=%d 组件缺失", i);
					continue;
				}

				const int foupSlot = foup_slot->currentText().toInt();
				const QString elpText = elp->currentText().trimmed();
				const QString llText = ll->currentText().trimmed();
				const int llSlot = ll_slot->currentText().toInt();
				const QString armText = arm->currentText().trimmed();
				const bool pm2IsEnabled = pm2_enable->isChecked();

				logInform("Cycle", "半自动 setTransferSequence: row=%d elp=%s ll=%s foup_slot=%d ll_slot=%d arm=%s pm2_enable=%d",
					i,
					elpText.toStdString().c_str(),
					llText.toStdString().c_str(),
					foupSlot,
					llSlot,
					armText.toStdString().c_str(),
					pm2IsEnabled ? 1 : 0);

				validRows++;
				if (elpText == "ELP1") elp1Rows++;
				else if (elpText == "ELP2") elp2Rows++;
				if (llText == "LLA") llaRows++;
				else if (llText == "LLB") llbRows++;

				LPTransferWafer lp;
				lp.slot_lp = foupSlot;
				lp.slot_lk = llSlot;
				lp.source_lp = elpText.toStdString();
				lp.transfer = (llText == "LLB") ? "lk2_to_lk2" : "lk1_to_lk1";
				lp.selected_arm = (armText == "B") ? 1 : 0;
				lp.is_finish_putlk = false;

				RobotTransferWafer robot;
				robot.source_loadlock = llText.toStdString();
				robot.target_loadlock = llText.toStdString();
				robot.source_lp = elpText.toStdString();
				robot.slot_lp = lp.slot_lp;
				robot.slot_lk = lp.slot_lk;
				robot.transfer = lp.transfer;
				robot.selected_arm = lp.selected_arm;
				robot.pm2_is_enable = pm2IsEnabled;
				robot.is_finish = false;

				if (elpText == "ELP1"){
					sequence_lp1_transfer_wafer.push_back(lp);
					sequence_lp1_transfer_wafer_copy.push_back(lp);
					sequence_robot_transfer_wafer_lp1.push_back(robot);
				}
				else{
					sequence_lp2_transfer_wafer.push_back(lp);
					sequence_lp2_transfer_wafer_copy.push_back(lp);
					sequence_robot_transfer_wafer_lp2.push_back(robot);
				}

				if (llText == "LLB"){
					sequence_tolk2_wafer.push_back(lp);
					sequence_tolk2_wafer_copy.push_back(lp);
				}
				else{
					sequence_tolk1_wafer.push_back(lp);
					sequence_tolk1_wafer_copy.push_back(lp);
				}

				sequence_robot_transfer_wafer.push_back(robot);
			}

			logInform("Cycle", "半自动 setTransferSequence: validRows=%d ELP1=%d ELP2=%d LLA=%d LLB=%d",
				validRows, elp1Rows, elp2Rows, llaRows, llbRows);
			logInform("Cycle", "半自动 setTransferSequence: queueSize lp1=%d lp2=%d robot=%d lk1=%d lk2=%d",
				(int)sequence_lp1_transfer_wafer.size(),
				(int)sequence_lp2_transfer_wafer.size(),
				(int)sequence_robot_transfer_wafer.size(),
				(int)sequence_loadlock1_transfer_wafer.size(),
				(int)sequence_loadlock2_transfer_wafer.size());

			semi_auto_need_lla.store(!sequence_tolk1_wafer.empty());
			semi_auto_need_llb.store(!sequence_tolk2_wafer.empty());
			semi_auto_lla_llload_done.store(false);
			semi_auto_llb_llload_done.store(false);
			semi_auto_lla_llunload_done.store(false);
			semi_auto_llb_llunload_done.store(false);
			logInform("Cycle", "SemiAuto needLL: LLA=%d LLB=%d (reset LLLoad/LLUnload done flags)",
				semi_auto_need_lla.load(), semi_auto_need_llb.load());

			if (validRows > 0 && sequence_robot_transfer_wafer.size() > 0)
			{
				onUpdateProcessControlEnabled(false);
				logInform("Cycle", "半自动流程配置成功");
				return true;
			}
			logInform("Cycle", "半自动流程配置失败：validRows=%d robotQueue=%d", validRows, (int)sequence_robot_transfer_wafer.size());
			return false;
		}

		for (int i = 0; i < ui->sequence_edit_tbw->rowCount(); ++i) {
			QWidget *widget_direction = ui->sequence_edit_tbw->cellWidget(i, 1);
			QComboBox *direction = (QComboBox*)widget_direction;
			QWidget *widget_arm = ui->sequence_edit_tbw->cellWidget(i, 3);
			QComboBox *arm = (QComboBox*)widget_arm;

			QWidget *widget_loadlock1 = ui->sequence_edit_tbw->cellWidget(i, 0);
			QComboBox *loadlock1_slot = (QComboBox*)widget_loadlock1;

			QWidget *widget_loadlock2 = ui->sequence_edit_tbw->cellWidget(i, 2);
			QComboBox *loadlock2_slot = (QComboBox*)widget_loadlock2;

			/*QWidget *widget_pm1 = ui->sequence_edit_tbw->cellWidget(i, 4);
			QCheckBox *pm1 = (QCheckBox*)widget_pm1;*/

			QWidget *widget_pm2 = ui->sequence_edit_tbw->cellWidget(i, 4);
			QCheckBox *pm2 = (QCheckBox*)widget_pm2;

			/*QWidget *widget_pm3 = ui->sequence_edit_tbw->cellWidget(i, 6);
			QCheckBox *pm3 = (QCheckBox*)widget_pm3;*/

			int lp1slot = loadlock1_slot->currentText().toInt();
			int lp2slot = loadlock2_slot->currentText().toInt();
			if (ui->disabledefem->checkState() == Qt::CheckState::Checked){
				lk1slot = lp1slot % LL_slot_count == 0 ? LL_slot_count : lp1slot % LL_slot_count;
				lk2slot = lp2slot % LL_slot_count == 0 ? LL_slot_count : lp2slot % LL_slot_count;
			}
			else{
				lk1slot = lk1slot > LL_slot_count ? 1 : lk1slot;
				lk2slot = lk2slot > LL_slot_count ? 1 : lk2slot;
			}
			
			
			lp1num = lp1num > LL_slot_count * 2 ? 1 : lp1num;
			lp2num = lp2num > LL_slot_count * 2 ? 1 : lp2num;

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
           if (direction->currentText() == "LP1<----->LP1"){

			   if (lastlp != "ELP1"&&i != 0){
				   if (lp2num2 >= 1 && lp2num2 < LL_slot_count){//lk2_to_lk2
					   lp1num = lp2num2 + LL_slot_count + 1;
					   lk2slot = lk1slot + 1;
					   lk1_goto_lk2 = true;
					   logInform("Cycle", "i=%d lp1num=%d lk2slot=%d lp2num=%d", i, lp1num, lk2slot, lp2num2);
				   }
				   else if (lp2num2 >= LL_slot_count + 1 && lp2num2 < LL_slot_count * 2){
					   lp1num = lp2num2 - LL_slot_count + 2;
					   lk2slot = lk1slot + 1;
					   lk1_goto_lk2 = true;
					   logInform("Cycle", "i=%d lp1num=%d lk2slot=%d lp2num2=%d ", i, lp1num, lk2slot, lp2num2);
				   }
				   else if (lp2num2 == LL_slot_count * 2){
					   lp1num = LL_slot_count + 2;
				   }
			   }
			   lastlp = "ELP1";
			   lp1.source_lp = "ELP1";
			  
				if (isRobotLLA){//lp1num <= 4
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
					if (lk1_goto_lk2&&lp1num==LL_slot_count){
						lk1_goto_lk2 = false;
						lk2slot = 1;
						logInform("Cycle", "i=%d updatelk2slot=%d", i, lk2slot);
					}
				}
				else{
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
			
				logInform("Cycle", "i=%d lp1=%s %d %d %s %d %s", i, lp1.transfer, lp1.slot_lk, robot.slot_lp, lastlp, lp1num, robot.source_loadlock);
				lp1num2 = lp1num;
				lp1num++;
			}
			else if (direction->currentText() == "LP2<----->LP2"){
				if (lastlp != "ELP2"&&i!=0){
					if (lp1num2 >= 1 && lp1num2 < LL_slot_count){
						lp2num = lp1num2 + LL_slot_count + 1;
						lk1slot = lk2slot + 1;
						lk2_goto_lk1 = true;
						logInform("Cycle", "i=%d lp2num=%d lk1slot=%d lp1num2=%d", i, lp2num, lk1slot, lp1num2);
					}
					else if (lp1num2 >= LL_slot_count + 1 && lp1num2 < LL_slot_count * 2){
						lp2num = lp1num2 - LL_slot_count + 2;
						lk1slot = lk2slot + 1;
						lk2_goto_lk1 = true;
						logInform("Cycle", "i=%d lp2num=%d lk1slot=%d lp1num2=%d", i, lp2num, lk1slot, lp1num2);
					}
					else if (lp1num2==LL_slot_count * 2){
						lp2num = LL_slot_count + 2;
					}
					
				}
				lastlp = "ELP2";
				lp2.source_lp = "ELP2";
				if (!isRobotLLA){//lp2num <= 4
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
					if (lk2_goto_lk1&&lp2num == LL_slot_count){
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
				logInform("Cycle", "i=%d lp2=%s lk=%d lpnum=%d lastlp=%s lp2num=%d robotsource_loadlock=%s", i, lp2.transfer, lp2.slot_lk, robot.slot_lp, lastlp, lp2num, robot.source_loadlock);
				lp2num2 = lp2num;
				lp2num++;
			}
			else
			{
				return false;
			}

			++currentSlotNumber;
			if (currentSlotNumber > LL_slot_count) {
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
		d->ui->sequence_edit_tbw->setVisible(true);
		d->ui->pm_cavity_param_edit_tbw->setSelectionBehavior(QAbstractItemView::SelectRows);

		d->ui->semi_auto_sequence_tbw->setVisible(true);
		for (size_t i = 0; i < d->ui->semi_auto_sequence_tbw->columnCount(); i++)
		{
			d->ui->semi_auto_sequence_tbw->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Stretch);
		}
		d->ui->semi_auto_sequence_tbw->setSelectionBehavior(QAbstractItemView::SelectRows);

		{
			QString fileName = QDir::currentPath() + "/config/" + "config.ini";
			QSettings settings(fileName, QSettings::IniFormat);
			int tab_index = settings.value("SemiAutoTransferMode", false).toBool() ? 1 : 0;
			if (d->ui->mode_tabw)
			{
				d->ui->mode_tabw->setCurrentIndex(tab_index);
			}
			d->semi_auto_mode = (tab_index == 1);
			if (d->ui->semi_auto_mode_lbl)
			{
				d->ui->semi_auto_mode_lbl->setText(d->semi_auto_mode ? "当前模式：半自动" : "当前模式：全自动");
			}
		}
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

		if (d->ui->mode_tabw)
		{
			connect(d->ui->mode_tabw, &QTabWidget::currentChanged, this, [this, d](int index) {
				d->semi_auto_mode = (index == 1);
				if (d->ui->semi_auto_mode_lbl)
				{
					d->ui->semi_auto_mode_lbl->setText(d->semi_auto_mode ? "当前模式：半自动" : "当前模式：全自动");
				}
				QString fileName = QDir::currentPath() + "/config/" + "config.ini";
				QSettings settings(fileName, QSettings::IniFormat);
				settings.setValue("SemiAutoTransferMode", d->semi_auto_mode);
			});
		}

		connect(d->ui->semi_auto_add_item_pbt, &QPushButton::clicked, this, [this, d]() {
			int row_count = d->ui->semi_auto_sequence_tbw->rowCount();
			d->ui->semi_auto_sequence_tbw->insertRow(row_count);

			std::shared_ptr<EFEMLPSubsystem> elp1 = d->kernel->getKernelModule<EFEMLPSubsystem>("ELP1");
			int slot_count = (int)elp1->getSlotCount();
			if (slot_count <= 0) slot_count = 1;

			QComboBox* foup_slot_cbx = new QComboBox();
			for (int i = 1; i <= slot_count; i++)
			{
				foup_slot_cbx->addItem(QString::number(i));
			}
			d->ui->semi_auto_sequence_tbw->setCellWidget(row_count, 0, foup_slot_cbx);

			QComboBox* elp_cbx = new QComboBox();
			elp_cbx->addItem("ELP1");
			elp_cbx->addItem("ELP2");
			d->ui->semi_auto_sequence_tbw->setCellWidget(row_count, 1, elp_cbx);

			QComboBox* ll_cbx = new QComboBox();
			ll_cbx->addItem("LLA");
			ll_cbx->addItem("LLB");
			d->ui->semi_auto_sequence_tbw->setCellWidget(row_count, 2, ll_cbx);

			QComboBox* ll_slot_cbx = new QComboBox();
			for (int i = 1; i <= 25; i++)
			{
				ll_slot_cbx->addItem(QString::number(i));
			}
			d->ui->semi_auto_sequence_tbw->setCellWidget(row_count, 3, ll_slot_cbx);

			QComboBox* arm_cbx = new QComboBox();
			arm_cbx->addItem("A");
			arm_cbx->addItem("B");
			d->ui->semi_auto_sequence_tbw->setCellWidget(row_count, 4, arm_cbx);

			QCheckBox* enable_ckb = new QCheckBox();
			enable_ckb->setChecked(true);
			d->ui->semi_auto_sequence_tbw->setCellWidget(row_count, 5, enable_ckb);
		});

		connect(d->ui->semi_auto_delete_selected_item_pbt, &QPushButton::clicked, this, [this, d]() {
			int row_count = d->ui->semi_auto_sequence_tbw->rowCount();
			if (row_count <= 0)
			{
				return;
			}
			int selected_row = d->ui->semi_auto_sequence_tbw->currentRow();
			if (selected_row < 0)
			{
				return;
			}
			d->ui->semi_auto_sequence_tbw->removeRow(selected_row);
		});

		connect(d->ui->semi_auto_clear_sequence_pbt, &QPushButton::clicked, this, [this, d]() {
			if (d->ui->semi_auto_sequence_tbw->rowCount() <= 0)
			{
				return;
			}
			d->ui->semi_auto_sequence_tbw->setRowCount(0);
		});

		connect(d->ui->semi_auto_save_sequence_pbt, &QPushButton::clicked, this, [this, d]() {
			if (d->ui->semi_auto_sequence_tbw->rowCount() < 1)
			{
				QMessageBox::warning(this, tr("警告信息"), tr("请先添加顺序！"));
				return;
			}

			QString cycle_dir = QCoreApplication::applicationDirPath() + "/CycleIni";
			QDir dir(cycle_dir);
			if (!dir.exists())
			{
				dir.mkpath(cycle_dir);
			}

			QString fileName = QFileDialog::getSaveFileName(this, "保存顺序", cycle_dir, "配置文件(*.ini)");
			if (fileName.isEmpty())
			{
				QMessageBox::warning(this, tr("警告信息"), tr("保存路径不能为空！"));
				return;
			}

			QSettings settings(fileName, QSettings::IniFormat);
			int rowCount = d->ui->semi_auto_sequence_tbw->rowCount();
			settings.setValue("rowCount", rowCount);

			for (int i = 0; i < rowCount; i++)
			{
				for (int j = 0; j < 5; j++)
				{
					QString key = QString("row%1col%2").arg(i).arg(j);
					QWidget* widget = d->ui->semi_auto_sequence_tbw->cellWidget(i, j);
					QComboBox* combox = (QComboBox*)widget;
					settings.setValue(key, combox ? combox->currentText() : "");
				}

				{
					QString key = QString("row%1col%2").arg(i).arg(5);
					QWidget* widget = d->ui->semi_auto_sequence_tbw->cellWidget(i, 5);
					QCheckBox* ckb = (QCheckBox*)widget;
					settings.setValue(key, ckb ? ckb->isChecked() : false);
				}
			}
		});

		connect(d->ui->semi_auto_load_sequence_pbt, &QPushButton::clicked, this, [this, d]() {
			QString cycle_dir = QCoreApplication::applicationDirPath() + "/CycleIni";
			QDir dir(cycle_dir);
			if (!dir.exists())
			{
				dir.mkpath(cycle_dir);
			}

			QString fileName = QFileDialog::getOpenFileName(this, "加载顺序", cycle_dir, "配置文件(*.ini)");
			if (fileName.isEmpty())
			{
				return;
			}

			QSettings settings(fileName, QSettings::IniFormat);
			int rowCount = settings.value("rowCount", 0).toInt();

			std::shared_ptr<EFEMLPSubsystem> elp1 = d->kernel->getKernelModule<EFEMLPSubsystem>("ELP1");
			int slot_count = (int)elp1->getSlotCount();
			if (slot_count <= 0) slot_count = 1;

			d->ui->semi_auto_sequence_tbw->setRowCount(0);
			for (int i = 0; i < rowCount; i++)
			{
				d->ui->semi_auto_sequence_tbw->insertRow(i);

				QString foup_slot = settings.value(QString("row%1col%2").arg(i).arg(0), "1").toString();
				QString elp = settings.value(QString("row%1col%2").arg(i).arg(1), "ELP1").toString();
				QString ll = settings.value(QString("row%1col%2").arg(i).arg(2), "LLA").toString();
				QString ll_slot = settings.value(QString("row%1col%2").arg(i).arg(3), "1").toString();
				QString arm = settings.value(QString("row%1col%2").arg(i).arg(4), "A").toString();
				bool enable = settings.value(QString("row%1col%2").arg(i).arg(5), true).toBool();

				QComboBox* foup_slot_cbx = new QComboBox();
				for (int k = 1; k <= slot_count; k++)
				{
					foup_slot_cbx->addItem(QString::number(k));
				}
				foup_slot_cbx->setCurrentText(foup_slot);
				d->ui->semi_auto_sequence_tbw->setCellWidget(i, 0, foup_slot_cbx);

				QComboBox* elp_cbx = new QComboBox();
				elp_cbx->addItem("ELP1");
				elp_cbx->addItem("ELP2");
				elp_cbx->setCurrentText(elp);
				d->ui->semi_auto_sequence_tbw->setCellWidget(i, 1, elp_cbx);

				QComboBox* ll_cbx = new QComboBox();
				ll_cbx->addItem("LLA");
				ll_cbx->addItem("LLB");
				ll_cbx->setCurrentText(ll);
				d->ui->semi_auto_sequence_tbw->setCellWidget(i, 2, ll_cbx);

				QComboBox* ll_slot_cbx = new QComboBox();
				for (int k = 1; k <= 25; k++)
				{
					ll_slot_cbx->addItem(QString::number(k));
				}
				ll_slot_cbx->setCurrentText(ll_slot);
				d->ui->semi_auto_sequence_tbw->setCellWidget(i, 3, ll_slot_cbx);

				QComboBox* arm_cbx = new QComboBox();
				arm_cbx->addItem("A");
				arm_cbx->addItem("B");
				arm_cbx->setCurrentText(arm);
				d->ui->semi_auto_sequence_tbw->setCellWidget(i, 4, arm_cbx);

				QCheckBox* enable_ckb = new QCheckBox();
				enable_ckb->setChecked(enable);
				d->ui->semi_auto_sequence_tbw->setCellWidget(i, 5, enable_ckb);
			}
		});

		connect(d->ui->semi_auto_trigger_load_pbt, &QPushButton::clicked, this, [this]() {
			Q_D(QSlotTransferCycleVTMWidget);
			if (!d->semi_auto_mode || !d->running) {
				QMessageBox::warning(this, "提示", "请先切换到半自动Tab并点击启动。");
				return;
			}
			d->semi_auto_lla_llload_done.store(false);
			d->semi_auto_llb_llload_done.store(false);
			d->semi_auto_lla_llunload_done.store(false);
			d->semi_auto_llb_llunload_done.store(false);
			d->semi_auto_pm_load_target_mask.store(3);
			d->semi_auto_ll_unload_target_mask.store(3);
			d->semi_auto_pm_processing.store(false);
			d->semi_auto_pm_unload_allowed.store(false);
			logInform("Cycle", "SemiAuto button: LLLoad (completed=%d(%s) next=%s needLLA=%d needLLB=%d)",
				d->semi_auto_completed_phase.load(),
				semiAutoPhaseToString((QSlotTransferCycleVTMWidgetPrivate::SemiAutoPhase)d->semi_auto_completed_phase.load()),
				semiAutoPhaseToString(d->semi_auto_next_phase.load()),
				d->semi_auto_need_lla.load(), d->semi_auto_need_llb.load());
			d->semi_auto_next_phase.store(QSlotTransferCycleVTMWidgetPrivate::SemiAutoPhase::LLLoad);
		});

		auto onSemiAutoPMLoadClicked = [this](int llMask, const char* llName) {
			Q_D(QSlotTransferCycleVTMWidget);
			if (!d->semi_auto_mode || !d->running) {
				QMessageBox::warning(this, "提示", "请先切换到半自动Tab并点击启动。");
				return;
			}
			if (!d->isSemiAutoLLLoadAllDone()) {
				QMessageBox::warning(this, "提示", "当前还未到PM上料阶段。");
				return;
			}
			d->semi_auto_pm_load_target_mask.store(llMask);
			d->semi_auto_pm_unload_allowed.store(false);
			logInform("Cycle", "SemiAuto button: PMLoad(%s) (completed=%d(%s) next=%s)",
				llName,
				d->semi_auto_completed_phase.load(),
				semiAutoPhaseToString((QSlotTransferCycleVTMWidgetPrivate::SemiAutoPhase)d->semi_auto_completed_phase.load()),
				semiAutoPhaseToString(d->semi_auto_next_phase.load()));
			d->semi_auto_next_phase.store(QSlotTransferCycleVTMWidgetPrivate::SemiAutoPhase::PMLoad);
		};

		connect(d->ui->semi_auto_trigger_pm_load_lla_pbt, &QPushButton::clicked, this, [onSemiAutoPMLoadClicked]() {
			onSemiAutoPMLoadClicked(1, "LLA");
		});

		connect(d->ui->semi_auto_trigger_pm_load_llb_pbt, &QPushButton::clicked, this, [onSemiAutoPMLoadClicked]() {
			onSemiAutoPMLoadClicked(2, "LLB");
		});

		connect(d->ui->semi_auto_trigger_unload_pbt, &QPushButton::clicked, this, [this]() {
			Q_D(QSlotTransferCycleVTMWidget);
			if (!d->semi_auto_mode || !d->running) {
				QMessageBox::warning(this, "提示", "请先切换到半自动Tab并点击启动。");
				return;
			}
			if (d->semi_auto_completed_phase.load() < (int)QSlotTransferCycleVTMWidgetPrivate::SemiAutoPhase::PMLoad) {
				QMessageBox::warning(this, "提示", "当前还未到PM下料阶段。");
				return;
			}
			if (d->sequence_robot_get_wafer.size() == 0 || d->pm_allow_goto_craft || !d->pm_allow_get_put_wafer) {
				QMessageBox::warning(this, "提示", "当前还未到PM下料阶段。");
				return;
			}
			logInform("Cycle", "SemiAuto button: PMUnload (completed=%d(%s) next=%s robot_get_size=%d pm_allow_goto_craft=%d pm_allow_get_put=%d)",
				d->semi_auto_completed_phase.load(),
				semiAutoPhaseToString((QSlotTransferCycleVTMWidgetPrivate::SemiAutoPhase)d->semi_auto_completed_phase.load()),
				semiAutoPhaseToString(d->semi_auto_next_phase.load()),
				(int)d->sequence_robot_get_wafer.size(),
				d->pm_allow_goto_craft,
				d->pm_allow_get_put_wafer);
			d->semi_auto_pm_unload_allowed.store(false);
			d->semi_auto_next_phase.store(QSlotTransferCycleVTMWidgetPrivate::SemiAutoPhase::PMUnload);
		});

		auto getSemiAutoPmEnabledRowCountForLL = [d](const QString& llName) -> int {
			int count = 0;
			for (int i = 0; i < d->ui->semi_auto_sequence_tbw->rowCount(); ++i) {
				QComboBox* ll = qobject_cast<QComboBox*>(d->ui->semi_auto_sequence_tbw->cellWidget(i, 2));
				QCheckBox* enable = qobject_cast<QCheckBox*>(d->ui->semi_auto_sequence_tbw->cellWidget(i, 5));
				if (!ll || !enable) {
					continue;
				}
				if (!enable->isChecked()) {
					continue;
				}
				if (ll->currentText().trimmed() == llName) {
					count++;
				}
			}
			return count;
		};

		auto onSemiAutoLLUnloadClicked = [this, getSemiAutoPmEnabledRowCountForLL](int llMask, const char* llName) {
			Q_D(QSlotTransferCycleVTMWidget);
			if (!d->semi_auto_mode || !d->running) {
				QMessageBox::warning(this, "提示", "请先切换到半自动Tab并点击启动。");
				return;
			}
			if (d->semi_auto_completed_phase.load() < (int)QSlotTransferCycleVTMWidgetPrivate::SemiAutoPhase::PMUnload) {
				QMessageBox::warning(this, "提示", "当前还未到LL下料阶段。");
				return;
			}
			if ((llMask & 1) && d->sequence_loadlock1_put_wafer.size() == 0) {
				QMessageBox::warning(this, "提示", "当前LLA还未到LL下料阶段。");
				return;
			}
			if ((llMask & 2) && d->sequence_loadlock2_put_wafer.size() == 0) {
				QMessageBox::warning(this, "提示", "当前LLB还未到LL下料阶段。");
				return;
			}
			if (d->sequence_loadlock1_put_wafer.size() == 0 && d->sequence_loadlock2_put_wafer.size() == 0) {
				QMessageBox::warning(this, "提示", "当前还未到LL下料阶段。");
				return;
			}
			const QString llNameText = QString::fromUtf8(llName).trimmed();
			const int expectedReturnCount = getSemiAutoPmEnabledRowCountForLL(llNameText);
			const int currentReturnCount = (llMask & 1) ? d->return_wafer_count_lla.load() : d->return_wafer_count_llb.load();
			if (currentReturnCount < expectedReturnCount) {
				logInform("Cycle", "SemiAuto LLUnload blocked: ll=%s return=%d/%d (need all PM-enabled rows returned)",
					llName,
					currentReturnCount,
					expectedReturnCount);
				QMessageBox::warning(this, "提示",
					QString("%1回片计数未完成：%2/%3，禁止下料。")
						.arg(llNameText)
						.arg(currentReturnCount)
						.arg(expectedReturnCount));
				return;
			}
			d->semi_auto_ll_unload_target_mask.store(llMask);
			if (llMask & 1){
				d->semi_auto_lla_llunload_done.store(false);
			}
			if (llMask & 2){
				d->semi_auto_llb_llunload_done.store(false);
			}
			logInform("Cycle", "SemiAuto button: LLUnload(%s) (completed=%d(%s) next=%s ll1_put_size=%d ll2_put_size=%d needLLA=%d needLLB=%d)",
				llName,
				d->semi_auto_completed_phase.load(),
				semiAutoPhaseToString((QSlotTransferCycleVTMWidgetPrivate::SemiAutoPhase)d->semi_auto_completed_phase.load()),
				semiAutoPhaseToString(d->semi_auto_next_phase.load()),
				(int)d->sequence_loadlock1_put_wafer.size(),
				(int)d->sequence_loadlock2_put_wafer.size(),
				d->semi_auto_need_lla.load(), d->semi_auto_need_llb.load());
			d->semi_auto_next_phase.store(QSlotTransferCycleVTMWidgetPrivate::SemiAutoPhase::LLUnload);
		};

		connect(d->ui->semi_auto_trigger_ll_unload_lla_pbt, &QPushButton::clicked, this, [onSemiAutoLLUnloadClicked]() {
			onSemiAutoLLUnloadClicked(1, "LLA");
		});

		connect(d->ui->semi_auto_trigger_ll_unload_llb_pbt, &QPushButton::clicked, this, [onSemiAutoLLUnloadClicked]() {
			onSemiAutoLLUnloadClicked(2, "LLB");
		});

		auto refreshSemiAutoButtonState = [d]() {
			auto styleForPhase = [d](int phaseValue) -> QString {
				int currentPhase = (int)d->semi_auto_next_phase.load();
				int completedPhase = d->semi_auto_completed_phase.load();
				if (currentPhase == phaseValue) {
					return "background-color: rgb(255, 193, 7); color: black;";
				}
				if (completedPhase >= phaseValue) {
					return "background-color: rgb(76, 175, 80); color: white;";
				}
				return "background-color: rgb(224, 224, 224); color: black;";
			};

			auto styleForPhaseMask = [d, styleForPhase](int phaseValue, int currentMask, int btnMask) -> QString {
				int currentPhase = (int)d->semi_auto_next_phase.load();
				if (currentPhase != phaseValue){
					return styleForPhase(phaseValue);
				}
				if ((currentMask & btnMask) == 0){
					return "background-color: rgb(224, 224, 224); color: black;";
				}
				return "background-color: rgb(255, 193, 7); color: black;";
			};

			d->ui->semi_auto_trigger_load_pbt->setStyleSheet(styleForPhase((int)QSlotTransferCycleVTMWidgetPrivate::SemiAutoPhase::LLLoad));
			d->ui->semi_auto_trigger_pm_load_lla_pbt->setStyleSheet(styleForPhaseMask((int)QSlotTransferCycleVTMWidgetPrivate::SemiAutoPhase::PMLoad, d->semi_auto_pm_load_target_mask.load(), 1));
			d->ui->semi_auto_trigger_pm_load_llb_pbt->setStyleSheet(styleForPhaseMask((int)QSlotTransferCycleVTMWidgetPrivate::SemiAutoPhase::PMLoad, d->semi_auto_pm_load_target_mask.load(), 2));
			d->ui->semi_auto_trigger_unload_pbt->setStyleSheet(styleForPhase((int)QSlotTransferCycleVTMWidgetPrivate::SemiAutoPhase::PMUnload));
			d->ui->semi_auto_trigger_ll_unload_lla_pbt->setStyleSheet(styleForPhaseMask((int)QSlotTransferCycleVTMWidgetPrivate::SemiAutoPhase::LLUnload, d->semi_auto_ll_unload_target_mask.load(), 1));
			d->ui->semi_auto_trigger_ll_unload_llb_pbt->setStyleSheet(styleForPhaseMask((int)QSlotTransferCycleVTMWidgetPrivate::SemiAutoPhase::LLUnload, d->semi_auto_ll_unload_target_mask.load(), 2));
			d->ui->semi_auto_trigger_unload_pbt->setEnabled(d->semi_auto_pm_unload_allowed.load() && !d->semi_auto_pm_processing.load());

			if (d->semi_auto_mode && d->running) {
				const bool llStageOk = d->semi_auto_completed_phase.load() >= (int)QSlotTransferCycleVTMWidgetPrivate::SemiAutoPhase::PMUnload;
				const int expectedA = [&]() -> int {
					int count = 0;
					for (int i = 0; i < d->ui->semi_auto_sequence_tbw->rowCount(); ++i) {
						QComboBox* ll = qobject_cast<QComboBox*>(d->ui->semi_auto_sequence_tbw->cellWidget(i, 2));
						QCheckBox* enable = qobject_cast<QCheckBox*>(d->ui->semi_auto_sequence_tbw->cellWidget(i, 5));
						if (!ll || !enable) continue;
						if (!enable->isChecked()) continue;
						if (ll->currentText().trimmed() == "LLA") count++;
					}
					return count;
				}();
				const int expectedB = [&]() -> int {
					int count = 0;
					for (int i = 0; i < d->ui->semi_auto_sequence_tbw->rowCount(); ++i) {
						QComboBox* ll = qobject_cast<QComboBox*>(d->ui->semi_auto_sequence_tbw->cellWidget(i, 2));
						QCheckBox* enable = qobject_cast<QCheckBox*>(d->ui->semi_auto_sequence_tbw->cellWidget(i, 5));
						if (!ll || !enable) continue;
						if (!enable->isChecked()) continue;
						if (ll->currentText().trimmed() == "LLB") count++;
					}
					return count;
				}();
				const int currentA = d->return_wafer_count_lla.load();
				const int currentB = d->return_wafer_count_llb.load();
				const bool allowLla = llStageOk && d->sequence_loadlock1_put_wafer.size() > 0 && currentA >= expectedA;
				const bool allowLlb = llStageOk && d->sequence_loadlock2_put_wafer.size() > 0 && currentB >= expectedB;
				d->ui->semi_auto_trigger_ll_unload_lla_pbt->setEnabled(allowLla);
				d->ui->semi_auto_trigger_ll_unload_llb_pbt->setEnabled(allowLlb);
			}
			else {
				d->ui->semi_auto_trigger_ll_unload_lla_pbt->setEnabled(false);
				d->ui->semi_auto_trigger_ll_unload_llb_pbt->setEnabled(false);
			}
		};
		refreshSemiAutoButtonState();
		QTimer *semi_auto_btn_timer = new QTimer(this);
		connect(semi_auto_btn_timer, &QTimer::timeout, this, [refreshSemiAutoButtonState]() {
			refreshSemiAutoButtonState();
		});
		semi_auto_btn_timer->start(200);


		connect(d->ui->execute_pbt, &QPushButton::clicked, this, &QSlotTransferCycleVTMWidget::onStart);
		connect(d->ui->pause_pbt, &QPushButton::clicked, this, &QSlotTransferCycleVTMWidget::onPause);
		connect(d->ui->reset_pbt, &QPushButton::clicked, this, &QSlotTransferCycleVTMWidget::onReset);
		connect(d->ui->semi_auto_execute_pbt, &QPushButton::clicked, this, &QSlotTransferCycleVTMWidget::onStart);
		connect(d->ui->semi_auto_pause_pbt, &QPushButton::clicked, this, &QSlotTransferCycleVTMWidget::onPause);
		connect(d->ui->semi_auto_reset_pbt, &QPushButton::clicked, this, &QSlotTransferCycleVTMWidget::onReset);

		connect(d->ui->abort_pbt, &QPushButton::clicked, this, &QSlotTransferCycleVTMWidget::onAbort);
		connect(d->ui->loadlock1_put_cassette_finished_pbt, &QPushButton::clicked, this, &QSlotTransferCycleVTMWidget::onLoadLock1PutCassetteFinished);
		connect(d->ui->loadlock2_put_cassette_finished_pbt, &QPushButton::clicked, this, &QSlotTransferCycleVTMWidget::onLoadLock2PutCassetteFinished);

		connect(d->ui->get_step_pbt, &QPushButton::clicked, this, &QSlotTransferCycleVTMWidget::onGetStep);

		connect(s_ptr, &QFortrendStationStatusVTMWidget::signalUpdateRecipe, this, &QSlotTransferCycleVTMWidget::onUpdateRecipe);
		if (s_ptr){
			auto pm_main_widget = s_ptr->findChild<QMainPMCavitySubsystemWidget*>();
			if (pm_main_widget){
				connect(pm_main_widget, &QMainPMCavitySubsystemWidget::sigProcessStarted, this,
					[this](const QString& pmName, const QString&, int) {
						Q_D(QSlotTransferCycleVTMWidget);
						const QString pm = pmName.trimmed().toUpper();
						if (pm == "PM" || pm == "PM2"){
							d->semi_auto_pm_processing.store(true);
							d->semi_auto_pm_unload_allowed.store(false);
						}
					});
				connect(pm_main_widget, &QMainPMCavitySubsystemWidget::sigProcessFinished, this,
					[this](const QString& pmName, const QString&, int, int, bool normalFinished) {
						Q_D(QSlotTransferCycleVTMWidget);
						const QString pm = pmName.trimmed().toUpper();
						if (pm == "PM" || pm == "PM2"){
							d->semi_auto_pm_processing.store(false);
							d->semi_auto_pm_unload_allowed.store(normalFinished);
						}
					});
			}
		}


		d->ui->execute_pbt->setEnabled(false);
		d->ui->semi_auto_execute_pbt->setEnabled(false);
		initPMCavityParamEdieTableWidget();
		d->ui->gbx_pm_parameter->hide();
		d->ui->loadlock1_put_cassette_finished_pbt->hide();
		d->ui->loadlock2_put_cassette_finished_pbt->hide();

		d->ui->spb_min->hide();
		d->ui->spb_max->hide();
		d->ui->label_3->hide();
		d->ui->label_4->hide();
		d->ui->smif_feed_btn->hide();
		d->ui->smif_blanking_btn->hide();
		d->ui->abort_pbt->hide();
		d->ui->enablesmif1->hide();
		d->ui->enablesmif2->hide();

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

		std::shared_ptr<EFEMLPSubsystem> elp1 = d->kernel->getKernelModule<EFEMLPSubsystem>("ELP1");
		std::shared_ptr<EFEMLPSubsystem> elp2 = d->kernel->getKernelModule<EFEMLPSubsystem>("ELP2");

		QComboBox *loadlock1_slot_selected_cbx = new QComboBox();
		for (size_t i = 1; i <=elp1->getSlotCount(); i++)
		{
			loadlock1_slot_selected_cbx->addItem(std::to_string(i).c_str());
		}
		d->ui->sequence_edit_tbw->setCellWidget(row_count, 0, loadlock1_slot_selected_cbx);

		QComboBox *loadlock2_slot_selected_cbx = new QComboBox();
		for (size_t i = 1; i <= elp2->getSlotCount(); i++)
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

		/*QCheckBox *pm1_ckb = new QCheckBox();
		pm1_ckb->setText("启用");
		d->ui->sequence_edit_tbw->setCellWidget(row_count, 4, pm1_ckb);*/

		QCheckBox *pm2_ckb = new QCheckBox();
		pm2_ckb->setText("启用");
		d->ui->sequence_edit_tbw->setCellWidget(row_count, 4, pm2_ckb);

		/*QCheckBox *pm3_ckb = new QCheckBox();
		pm3_ckb->setText("启用");
		d->ui->sequence_edit_tbw->setCellWidget(row_count, 6, pm3_ckb);*/

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
		int columnCount = d->ui->sequence_edit_tbw->columnCount();
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
				if (j > 16)
				{
					QComboBox *combox = (QComboBox*)widget;
					settings.setValue(key, combox->currentText());
				}
				else{
					QDoubleSpinBox *dsb = (QDoubleSpinBox*)widget;
					settings.setValue(key, dsb->value());
				}

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
		std::shared_ptr<FortrendPMCavitySubsystem> pm2 = d->kernel->getKernelModule<FortrendPMCavitySubsystem>("PM");
		pm2->setIsRunning(false);
		pm2->setPMCavityPause(true);
		d->running = false;
		d->ispause = true;

		d->ui->execute_pbt->setEnabled(true);
		d->ui->reset_pbt->setEnabled(true);
		d->ui->pause_pbt->setEnabled(false);
		d->ui->semi_auto_execute_pbt->setEnabled(true);
		d->ui->semi_auto_reset_pbt->setEnabled(true);
		d->ui->semi_auto_pause_pbt->setEnabled(false);
		
	}

	void QSlotTransferCycleVTMWidget::onAbort(){
		Q_D(QSlotTransferCycleVTMWidget);
		std::shared_ptr<FortrendPMCavitySubsystem> pm3 = d->kernel->getKernelModule<FortrendPMCavitySubsystem>("PM");
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
#ifndef SIM_TEST


		if (!isEnabledplcAuto()){
			QMessageBox::warning(this, "警告", "PLC不在自动模式.");
			return;
		}
#endif // SIM_TEST

		d->running = false;
		d->ispause = false;
		d->semi_auto_next_phase.store(QSlotTransferCycleVTMWidgetPrivate::SemiAutoPhase::None);
		d->semi_auto_completed_phase.store(0);
		std::thread thread(&QSlotTransferCycleVTMWidget::resetAction, this);
		thread.detach();
	}

	void QSlotTransferCycleVTMWidget::onStart(){
		Q_D(QSlotTransferCycleVTMWidget);

		auto wtr = d->kernel->getKernelModule<EFEMWaferRobotSubsystem>("EWTR");
		std::string mess = "当前EFEM执行模式为："+ wtr->getWaferModelStr() +"，请人工确认 LoadPort 中是否为：" + wtr->getWaferModelStr();
		if (QMessageBox::warning(this, "提示", mess.c_str(),QMessageBox::Yes,QMessageBox::No) == QMessageBox::No)
		{
			return;
		}

		//start action & store param
		std::shared_ptr<FortrendPMCavitySubsystem> pm = d->kernel->getKernelModule<FortrendPMCavitySubsystem>("PM");
		if (!pm->getPMCavityMotorHomeSignal()){
			QMessageBox::warning(this, "警告", "PM腔步进电机未后退到原位.");
			return;
		}

		if (pm->getAxisIsRunning()) {
			QMessageBox::warning(this, "警告", "PM腔横移轴Cycle运行中，请先停止！");
			return;
		}
		d->cycle_times_lla = d->ui->cycle_setting_times_sbx->value();
		d->cycle_times_llb = d->ui->cycle_setting_times_sbx_2->value();
		{
			QString fileName = QDir::currentPath() + "/config/" + "config.ini";
			QSettings settings(fileName, QSettings::IniFormat);
			d->semi_auto_mode = settings.value("SemiAutoTransferMode", false).toBool() || (d->ui->mode_tabw && d->ui->mode_tabw->currentIndex() == 1);
			if (d->ui->semi_auto_mode_lbl)
			{
				d->ui->semi_auto_mode_lbl->setText(d->semi_auto_mode ? "当前模式：半自动" : "当前模式：全自动");
			}
		}
		//show parameter d->sequence_loadlock1_transfer_wafer.size() == 0 && d->sequence_loadlock2_transfer_wafer.size() == 0 &&
		if (d->sequence_lp1_transfer_wafer.size() == 0 && d->sequence_lp2_transfer_wafer.size() == 0 &&
			d->sequence_robot_transfer_wafer.size() == 0 && !d->ispause)//暂停重新启动的情况不需要重新配置
		{
			if (d->setTransferSequence())
			{
				d->ui->cycle_finished_times_spx->setValue(0);
				d->ui->cycle_finished_times_spx_2->setValue(0);
				logInform("Cycle", "传送流程配置成功。");
			}
			else
			{
				QMessageBox::warning(this, "警告", "传送流程配置错误.");
				return;
			}
		}
		if (!isEnabledplcAuto()){
			QMessageBox::warning(this, "警告", "PLC不在自动模式.");
			return;
		}

		d->running = true;
		pm->setIsRunning(d->running);
		d->ispause = false;
		d->semi_auto_next_phase.store(QSlotTransferCycleVTMWidgetPrivate::SemiAutoPhase::None);
		d->semi_auto_completed_phase.store(0);

		//读取半自动传输模式配置
		{
			QString fileName = QDir::currentPath() + "/config/" + "config.ini";
			QSettings settings(fileName, QSettings::IniFormat);
			d->semi_auto_mode = settings.value("SemiAutoTransferMode", false).toBool() || (d->ui->mode_tabw && d->ui->mode_tabw->currentIndex() == 1);
			if (d->semi_auto_mode){
				logInform("Cycle", "当前传输模式：半自动");
			}
			else{
				logInform("Cycle", "当前传输模式：全自动");
			}
			if (d->ui->semi_auto_mode_lbl)
			{
				d->ui->semi_auto_mode_lbl->setText(d->semi_auto_mode ? "当前模式：半自动" : "当前模式：全自动");
			}
		}

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

		d->tower->setOutput(FortrendVTMSignalTower::Output::YELLOW_LIGHT, false);
		d->tower->setOutput(FortrendVTMSignalTower::Output::GREEN_LIGHT, true);
		d->ui->execute_pbt->setEnabled(false);
		d->ui->reset_pbt->setEnabled(false);
		d->ui->pause_pbt->setEnabled(true);
		d->ui->semi_auto_execute_pbt->setEnabled(false);
		d->ui->semi_auto_reset_pbt->setEnabled(false);
		d->ui->semi_auto_pause_pbt->setEnabled(true);
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
		std::shared_ptr<FortrendPMCavitySubsystem> pm2 = d->kernel->getKernelModule<FortrendPMCavitySubsystem>("PM");
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
	}

	void QSlotTransferCycleVTMWidget::resetAction(){
		Q_D(QSlotTransferCycleVTMWidget);
		d->resetAction();
	}

	void QSlotTransferCycleVTMWidget::initPMCavityParamEdieTableWidget(){
		Q_D(QSlotTransferCycleVTMWidget);

		
		//addAnPMItem("PM1");
		addAnPMItem("PM");
		//addAnPMItem("PM3");

	}

	void QSlotTransferCycleVTMWidget::addAnPMItem(const QString name){
		Q_D(QSlotTransferCycleVTMWidget);
		int row_count = d->ui->pm_cavity_param_edit_tbw->rowCount();
		d->ui->pm_cavity_param_edit_tbw->insertRow(row_count);

		QTableWidgetItem* item = new QTableWidgetItem;
		item->setText(name);
		item->setFlags(item->flags() & ~Qt::ItemIsEditable);
		d->ui->pm_cavity_param_edit_tbw->setItem(row_count, 0, item);

		addEditTableWidgetItemDoubleSpinBox(row_count, 1, 20.0, 650.0, 50, 100);    //温度设定
		addEditTableWidgetItemDoubleSpinBox(row_count, 2, 1.0, 20.0, 1, 10);		 //粗抽压力
		addEditTableWidgetItemDoubleSpinBox(row_count, 3, 0.00008, 0.1, 0.005, 0.02, 5); //精抽压力
		addEditTableWidgetItemDoubleSpinBox(row_count, 4, 0.0008, 1, 0.05, 0.02, 4); //溅射压力
		addEditTableWidgetItemDoubleSpinBox(row_count, 5, 0.0, 200.0, 10, 50);		 //溅射流量1
		addEditTableWidgetItemDoubleSpinBox(row_count, 6, 0.0, 200.0, 10, 50);		 //溅射流量2
		addEditTableWidgetItemDoubleSpinBox(row_count, 7, 0.0, 200.0, 10, 50);		 //溅射流量3
		addEditTableWidgetItemDoubleSpinBox(row_count, 8, 0.0, 1000.0, 50, 500);   //溅射功率1
		addEditTableWidgetItemDoubleSpinBox(row_count, 9, 1.0, 100.0, 10, 50);	 //溅射功率增速1
		addEditTableWidgetItemDoubleSpinBox(row_count, 10, 0.0, 1000.0, 50, 500);  //溅射功率2
		addEditTableWidgetItemDoubleSpinBox(row_count, 11, 1.0, 100.0, 10, 50);	 //溅射功率增速2
		addEditTableWidgetItemDoubleSpinBox(row_count, 12, 0.0, 1000.0, 50, 500);  //溅射功率3
		addEditTableWidgetItemDoubleSpinBox(row_count, 13, 1.0, 100.0, 10, 50);	 //溅射功率增速3
		addEditTableWidgetItemDoubleSpinBox(row_count, 14, 0.0, 60.0, 5.0, 10.0);		 //预溅射事件
		addEditTableWidgetItemDoubleSpinBox(row_count, 15, 0.0, 180.0, 10, 45.0);	 //工艺溅射旋转速度
		addEditTableWidgetItemDoubleSpinBox(row_count, 16, 0.0, 120.0, 10, 30);    //工艺溅射时间

		QComboBox *cathode_power_selection_1_cbx = new QComboBox();
		cathode_power_selection_1_cbx->addItem("无");
		cathode_power_selection_1_cbx->addItem("DC");
		cathode_power_selection_1_cbx->addItem("RF");
		d->ui->pm_cavity_param_edit_tbw->setCellWidget(row_count, 17, cathode_power_selection_1_cbx);

		QComboBox *cathode_power_selection_2_cbx = new QComboBox();
		cathode_power_selection_2_cbx->addItem("无");
		cathode_power_selection_2_cbx->addItem("DC");
		cathode_power_selection_2_cbx->addItem("RF");
		d->ui->pm_cavity_param_edit_tbw->setCellWidget(row_count, 18, cathode_power_selection_2_cbx);

		QComboBox *cathode_power_selection_3_cbx = new QComboBox();
		cathode_power_selection_3_cbx->addItem("无");
		cathode_power_selection_3_cbx->addItem("DC");
		cathode_power_selection_3_cbx->addItem("RF");
		d->ui->pm_cavity_param_edit_tbw->setCellWidget(row_count, 19, cathode_power_selection_3_cbx);
	}

	void QSlotTransferCycleVTMWidget::UpdataListData()
	{
		Q_D(QSlotTransferCycleVTMWidget);
		QList<int> lists;
		lists.append(1);
		lists.append(5);
		lists.append(15);
		foreach(int value, lists) {
			for (int i = 0; i < d->ui->sequence_edit_tbw->rowCount(); ++i) {
				QWidget* cellWidget = d->ui->sequence_edit_tbw->cellWidget(i, 0);
				QComboBox* culomm1 = qobject_cast<QComboBox*>(cellWidget);
				if (culomm1) {

					if (culomm1->currentText() == QString::number(value)) {

						//                    QMessageBox::information(this, "提示", "第一列第 " + QString::number(i + 1) + " 行的选择值是"+culomm1->currentText());
						d->ui->sequence_edit_tbw->removeRow(i);
					}

				}
			}
		}
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
		d->ui->semi_auto_return_count_lla_spx->setValue(d->return_wafer_count_lla.load());
		d->ui->semi_auto_return_count_llb_spx->setValue(d->return_wafer_count_llb.load());
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
		d->ui->semi_auto_execute_pbt->setEnabled(enabled);
	}

	void QSlotTransferCycleVTMWidget::updateControlEnabled(const QString control, const bool enabled){
		Q_D(QSlotTransferCycleVTMWidget);
		if (control == "execute_pbt")
		{
			d->ui->execute_pbt->setEnabled(enabled);
			d->ui->semi_auto_execute_pbt->setEnabled(enabled);
		}
		else if (control == "pause_pbt")
		{
			d->ui->pause_pbt->setEnabled(enabled);
			d->ui->semi_auto_pause_pbt->setEnabled(enabled);
		}
		else if (control == "reset_pbt")
		{
			d->ui->reset_pbt->setEnabled(enabled);
			d->ui->semi_auto_reset_pbt->setEnabled(enabled);
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
}

