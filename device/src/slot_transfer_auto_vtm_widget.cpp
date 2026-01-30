/**
* @file            slot_transfer_auto_vtm_widget.h
* @brief           slot transfer auto vtm widget
* @author			xielonghua
*/

// Library: FortrendUI
// Package: VTM

#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <Windows.h>
#include <filesystem>


#include "Poco/Format.h"

#include  "slot_transfer_auto_vtm_widget.h" 
#include  "device/ui_slot_transfer_auto_vtm_widget.h"
#include  "Kernel/FortrendUI/slot_transfer_editor_widget.h"
#include  "Kernel/kernel.h"
#include  "kernel/kernel_log.h"
#include  "Kernel/kernel_action_subsystem.h"
#include  "kernel/kernel_parallel_action.h"
#include  "kernel/kernel_subsystem.h"
#include  "Kernel/Fortrend/fortrend_cassette_manager.h"
#include  "Kernel/Fortrend/cassette.h"

#include  "Kernel/Fortrend/fortrend_station.h"
#include  "Kernel/Fortrend/aligner_abstract_subsystem.h"
#include  "Kernel/kernel_subsystem_update_command.h"
#include  "kernel/kernel_block_manager.h"

#include  "Aligner/fortrend_aligner_subsystem.h"
#include  "CoolingCavity/fortrend_cooling_cavity_subsystem.h"
#include  "LoadLock/fortrend_loadlock_defined.h"
#include  "LoadLock/fortrend_loadlock_subsystem.h"
#include  "PMCavity/fortrend_pm_cavity_subsystem.h"
#include  "TMCavity/fortrend_tm_cavity_defined.h"
#include  "TMCavity/fortrend_tm_cavity_subsystem.h"
#include  "SunwayRobot/fortrend_sunwayrobot_subsystem.h"

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
#include  "PMCavity/fortrend_pm_cavity_defined.h"

#include  "Pump/fortrend_pump_subsystem.h"
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
#include  <QHeaderView>
#include  <QAbstractItemView>
#include  <QVariant>
#include  <QVariantList>
#include  <QCoreApplication>
#include  <QDir>
#include <QRegExp>


#if _MSC_VER >1600
#pragma execution_character_set("utf-8")
#endif

namespace FC{


	struct LoadLockTransferWaferAuto{
		int slot = 0;

	};
	struct RobotTransferWaferAuto{
		std::string pm_name = "";
		int selected_arm = 0;
		int slot = 0;
		PMCavityProcessParameters pm_parameter;
		bool is_finished = false;

	};

	/**
	* QSlotTransferCycleWTMWidgetPrivate
	*/
	class QSlotTransferAutoVTMWidgetPrivate : public KernelListener < IKernelCommand > {
	public:
		Q_DECLARE_PUBLIC(QSlotTransferAutoVTMWidget)
			QSlotTransferAutoVTMWidgetPrivate(QSlotTransferAutoVTMWidget*p);

		void startLoadLock1Action();
		void startLoadLock2Action();
		void startRobotAction();
		void startVacuumAction();
		void resetAction();
		void moniterPMCavityAlarmAction();
		void moniterPMCavityProcessDataAction();
		void setRunning(const bool value);

		void setLoadLock1PutCassetteFinished();
		void setLoadLock2PutCassetteFinished();

		bool setTransferSequence();
		bool setTransferSequenceLoadLock1();
		bool setTransferSequenceLoadLock2();
		bool setTransferSequenceRobot();
		void onUpdateAutoInfo();
		void onUpdateControlEnabled(const QString name, const bool enabled);
		void onUpdateProcessEditEnabled(const bool enabled);
		virtual void onAttributeChange(const IKernelCommand* cmd);
	public:
		float coating_time_pm1 = 0.01;	//PM1镀膜时间
		float coating_time_pm2 = 0.01;	//PM2镀膜时间
		float coating_time_pm3 = 0.01;	//PM3镀膜时间

		short processing_step_pm1 = -1;	//PM1工艺步骤
		short processing_step_pm2 = -1;	//PM2工艺步骤
		short processing_step_pm3 = -1;	//PM3工艺步骤

	private:
		QSlotTransferAutoVTMWidget * q_ptr;
		Ui::SlotTransferAutoVTMWidget *ui;
		std::shared_ptr<IKernel> kernel = 0;
		std::shared_ptr<FortrendVTMSignalTower> tower = 0;

		std::vector<RobotTransferWaferAuto> sequence_robot_transfer_wafer;
		std::vector<int> sequence_loadlock1_transfer_wafer;
		std::vector<int> sequence_loadlock2_transfer_wafer;

		std::thread thd_show_message;			//显示PM腔报警信息线程
		std::thread thd_pm_cavity_process;		//显示PM腔工艺进程信息
		std::thread thd_vacuum_auto;			//真空自动线程
		std::thread thd_robot_auto;				//机械手自动线程
		std::thread thd_loadlock1_auto;			//LoadLock1自动线程
		std::thread thd_loadlock2_auto;			//LoadLock2自动线程
		

		bool robot_step_once_finished = true;
		bool vacumm_step_once_finished = true;
		bool loadlock1_step_once_finished = true;
		bool loadlock2_step_once_finished = true;

		bool thread_enabled = false;
		bool process_edit_enabled = false;  //工艺流程编辑使能

		bool running = false;
		int robot_auto_step = 0;
		int loadlock1_auto_step = 0;
		int loadlock2_auto_step = 0;
		int vacuum_auto_step = 0;

		bool loadlock1_get_vacuum = false;		//LoadLock1获得真空
		bool loadlock2_get_vacuum = false;		//LoadLock2获得真空
		bool tm_get_vacuum = false;				//TM腔获得真空

		bool tm_angle_valve_recard_state = false;    //TM腔记录角阀状态
		bool tm_air_inflation = false;				 //TM腔充气
		bool tm_vacuum_in_range = false;			 //TM腔真空值在范围内

		int loadlock1_move_slot_index = 1;		//LoadLock1移动的槽号
		int loadlock2_move_slot_index = 1;		//LoadLock2移动的槽号
		int robot_selected_arm = 0;				//机械手臂选择

		bool loadlock1_put_cassette_finished = false;		//LoadLock放Cassette完成
		bool loadlock2_put_cassette_finished = false;		//LoadLock放Cassette完成

		bool loadlock1_process_finished = false;   //LoadLock1流程完成
		bool loadlock2_process_finished = false;   //LoadLock2流程完成
		bool robot_process_finished = false;	   //机械手流程完成

		bool loadlock1_allow_get_wafer = false;		//LoadLock1允许取片
		bool loadlock2_allow_get_wafer = false;		//LoadLock2允许取片
		bool loadlock1_allow_put_wafer = false;		//LoadLock1允许放片
		bool loadlock2_allow_put_wafer = false;		//LoadLock2允许放片



		const std::string loadlock1_process_name = "LoadLock1流程步骤";
		const std::string loadlock2_process_name = "LoadLock2流程步骤";
		const std::string robot_process_name = "机械手流程步骤";
		const std::string vacuum_process_name = "真空控制流程步骤";
		const std::string reset_process_name = "reset";
		const std::string module_name = "Auto";

		const int sleep_time = 8000;

	private:
		void logFailed(const std::string station_name, const std::string log);
		void logFailedNotNormal(const std::string station_name, const std::string process_name, const int step);
		void logFailedExcuteCommandHasError(const std::string station_name, const std::string command_name, const std::string process_name, const int step);

	};

	QSlotTransferAutoVTMWidgetPrivate::QSlotTransferAutoVTMWidgetPrivate(QSlotTransferAutoVTMWidget*p)
		:q_ptr(p){
		//
	}

	bool QSlotTransferAutoVTMWidgetPrivate::setTransferSequence(){

		if (ui->sequence_edit_tbw->rowCount() < 1)
		{
			return false;
		}
		if (setTransferSequenceRobot() && setTransferSequenceLoadLock1() && setTransferSequenceLoadLock2())
		{
			logInform("Auto", "Auto开始.");
			return true;
		}
		return false;
	}

	bool QSlotTransferAutoVTMWidgetPrivate::setTransferSequenceLoadLock1(){
		sequence_loadlock1_transfer_wafer.clear();
		for (int i = 0; i < ui->sequence_edit_tbw->rowCount(); ++i)
		{
			QWidget *widget_slot = ui->sequence_edit_tbw->cellWidget(i, 0);
			QComboBox *slot_cbx = (QComboBox*)widget_slot;
			int select_slot = slot_cbx->currentText().remove("槽号").toInt();
			if (select_slot < 0)
			{
				return false;
			}
			sequence_loadlock1_transfer_wafer.push_back(select_slot);
		}

		logInform("Auto", "LoadLock1流程添加完成.");
		return true;
	}

	bool QSlotTransferAutoVTMWidgetPrivate::setTransferSequenceLoadLock2(){
		sequence_loadlock2_transfer_wafer.clear();
		for (int i = 0; i < ui->sequence_edit_tbw->rowCount(); ++i)
		{
			QWidget *widget_slot = ui->sequence_edit_tbw->cellWidget(i, 0);
			QComboBox *slot_cbx = (QComboBox*)widget_slot;
			int select_slot = slot_cbx->currentText().remove("槽号").toInt();
			if (select_slot < 0)
			{
				return false;
			}
			sequence_loadlock2_transfer_wafer.push_back(select_slot);
		}
		logInform("Auto", "LoadLock2流程添加完成.");
		return true;
	}

	bool QSlotTransferAutoVTMWidgetPrivate::setTransferSequenceRobot(){

		sequence_robot_transfer_wafer.clear();
		for (int i = 0; i < ui->sequence_edit_tbw->rowCount(); ++i)
		{
			QWidget *widget_slot = ui->sequence_edit_tbw->cellWidget(i, 0);
			QComboBox *slot_cbx = (QComboBox*)widget_slot;
			int select_slot = slot_cbx->currentText().remove("槽号").toInt();
			if (select_slot < 0)
			{
				return false;
			}

			RobotTransferWaferAuto robot;
			robot.selected_arm = 0;
			robot.slot = select_slot;
			QWidget *widget_pm = ui->sequence_edit_tbw->cellWidget(i, 1);
			QComboBox *pm_cbx = (QComboBox*)widget_pm;
			robot.pm_name = pm_cbx->currentText().toStdString();

			QWidget *widget_temp = ui->sequence_edit_tbw->cellWidget(i, 2);
			QDoubleSpinBox *temperature_dsb = (QDoubleSpinBox*)widget_temp;
			robot.pm_parameter.heating_temperature = temperature_dsb->value();

			QWidget *widget_extraction_pressure = ui->sequence_edit_tbw->cellWidget(i, 3);
			QDoubleSpinBox *extraction_pressure_dsb = (QDoubleSpinBox*)widget_extraction_pressure;
			robot.pm_parameter.initial_extraction_pressure = extraction_pressure_dsb->value();

			QWidget *widget_purified_pressure = ui->sequence_edit_tbw->cellWidget(i, 4);
			QDoubleSpinBox *purified_pressure_dsb = (QDoubleSpinBox*)widget_purified_pressure;
			robot.pm_parameter.purified_extraction_pressure = purified_pressure_dsb->value();

			QWidget *widget_sputtering_pressure = ui->sequence_edit_tbw->cellWidget(i, 5);
			QDoubleSpinBox *sputtering_pressure_dsb = (QDoubleSpinBox*)widget_sputtering_pressure;
			robot.pm_parameter.sputtering_pressure = sputtering_pressure_dsb->value();

			QWidget *widget_flow_rate1 = ui->sequence_edit_tbw->cellWidget(i, 6);
			QDoubleSpinBox *flow_rate1_dsb = (QDoubleSpinBox*)widget_flow_rate1;
			robot.pm_parameter.sputtering_flow_rate1 = flow_rate1_dsb->value();

			QWidget *widget_flow_rate2 = ui->sequence_edit_tbw->cellWidget(i, 7);
			QDoubleSpinBox *flow_rate2_dsb = (QDoubleSpinBox*)widget_flow_rate2;
			robot.pm_parameter.sputtering_flow_rate2 = flow_rate2_dsb->value();

			QWidget *widget_flow_rate3 = ui->sequence_edit_tbw->cellWidget(i, 8);
			QDoubleSpinBox *flow_rate3_dsb = (QDoubleSpinBox*)widget_flow_rate3;
			robot.pm_parameter.sputtering_flow_rate3 = flow_rate3_dsb->value();

			QWidget *widget_power1 = ui->sequence_edit_tbw->cellWidget(i, 9);
			QDoubleSpinBox *flow_power1_dsb = (QDoubleSpinBox*)widget_power1;
			robot.pm_parameter.sputtering_power1 = flow_power1_dsb->value();

			QWidget *widget_power_gear_up1 = ui->sequence_edit_tbw->cellWidget(i, 10);
			QDoubleSpinBox *flow_power_gear_up1_dsb = (QDoubleSpinBox*)widget_power_gear_up1;
			robot.pm_parameter.sputtering_power_gear_up1 = flow_power_gear_up1_dsb->value();

			QWidget *widget_power2 = ui->sequence_edit_tbw->cellWidget(i, 11);
			QDoubleSpinBox *flow_power2_dsb = (QDoubleSpinBox*)widget_power2;
			robot.pm_parameter.sputtering_power2 = flow_power2_dsb->value();

			QWidget *widget_power_gear_up2 = ui->sequence_edit_tbw->cellWidget(i, 12);
			QDoubleSpinBox *flow_power_gear_up2_dsb = (QDoubleSpinBox*)widget_power_gear_up2;
			robot.pm_parameter.sputtering_power_gear_up2 = flow_power_gear_up2_dsb->value();

			QWidget *widget_power3 = ui->sequence_edit_tbw->cellWidget(i, 13);
			QDoubleSpinBox *flow_power3_dsb = (QDoubleSpinBox*)widget_power3;
			robot.pm_parameter.sputtering_power3 = flow_power3_dsb->value();

			QWidget *widget_power_gear_up3 = ui->sequence_edit_tbw->cellWidget(i, 14);
			QDoubleSpinBox *flow_power_gear_up3_dsb = (QDoubleSpinBox*)widget_power_gear_up3;
			robot.pm_parameter.sputtering_power_gear_up3 = flow_power_gear_up3_dsb->value();

			QWidget *widget_sputtering_time = ui->sequence_edit_tbw->cellWidget(i, 15);
			QDoubleSpinBox *sputtering_time_dsb = (QDoubleSpinBox*)widget_sputtering_time;
			robot.pm_parameter.pre_sputtering_time = sputtering_time_dsb->value();

			QWidget *widget_substrate_speed = ui->sequence_edit_tbw->cellWidget(i, 16);
			QDoubleSpinBox *substrate_speed_dsb = (QDoubleSpinBox*)widget_substrate_speed;
			robot.pm_parameter.substrate_speed = substrate_speed_dsb->value();

			QWidget *widget_process_sputtering_time = ui->sequence_edit_tbw->cellWidget(i, 17);
			QDoubleSpinBox *process_sputtering_time_dsb = (QDoubleSpinBox*)widget_process_sputtering_time;
			robot.pm_parameter.process_sputtering_time = process_sputtering_time_dsb->value();

			QWidget *widget_cathode_power_selection_1 = ui->sequence_edit_tbw->cellWidget(i, 18);
			QComboBox *cathode_power_selection_1_cbx = (QComboBox*)widget_cathode_power_selection_1;
			robot.pm_parameter.cathode_power_selection_1 = cathode_power_selection_1_cbx->currentIndex();

			QWidget *widget_cathode_power_selection_2 = ui->sequence_edit_tbw->cellWidget(i, 19);
			QComboBox *cathode_power_selection_2_cbx = (QComboBox*)widget_cathode_power_selection_2;
			robot.pm_parameter.cathode_power_selection_2 = cathode_power_selection_2_cbx->currentIndex();

			QWidget *widget_cathode_power_selection_3 = ui->sequence_edit_tbw->cellWidget(i, 20);
			QComboBox *cathode_power_selection_3_cbx = (QComboBox*)widget_cathode_power_selection_3;
			robot.pm_parameter.cathode_power_selection_3 = cathode_power_selection_3_cbx->currentIndex();

			sequence_robot_transfer_wafer.push_back(robot);
		}
		logInform("Auto", "机械手流程添加完成.");
		return true;
	}

	void QSlotTransferAutoVTMWidgetPrivate::onUpdateAutoInfo(){
		QMetaObject::invokeMethod(q_ptr, "updateAutoData", Qt::AutoConnection);
	}

	void QSlotTransferAutoVTMWidgetPrivate::onUpdateControlEnabled(const QString name, const bool enabled){
		QMetaObject::invokeMethod(q_ptr, "updateControlEnabled", Qt::AutoConnection,
			Q_ARG(QString, name), Q_ARG(bool, enabled));
	}

	void QSlotTransferAutoVTMWidgetPrivate::onUpdateProcessEditEnabled(const bool enabled){
		process_edit_enabled = enabled;
		QMetaObject::invokeMethod(q_ptr, "updateProcessEnabled", Qt::AutoConnection);
	}

	void QSlotTransferAutoVTMWidgetPrivate::onAttributeChange(const IKernelCommand* cmd){
		if (cmd->getState() == IKernelCommand::CMD_IDEL || cmd->getState() == IKernelCommand::CMD_BUSY){
			return;
		}
		QMetaObject::invokeMethod(q_ptr, "updateAutoData", Qt::AutoConnection);
	}

	void QSlotTransferAutoVTMWidgetPrivate::resetAction(){
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
		std::shared_ptr<FortrendPMCavitySubsystem> pm2 = kernel->getKernelModule<FortrendPMCavitySubsystem>("PM2");
		//std::shared_ptr<FortrendPMCavitySubsystem> pm3 = kernel->getKernelModule<FortrendPMCavitySubsystem>("PM3");
		std::shared_ptr<FortrendLoadLockSubsystem> lk2 = kernel->getKernelModule<FortrendLoadLockSubsystem>("LLB");
		//if (!pm1){ return; }
		//if (!pm2){ return; }
		//if (!pm3){ return; }
		
		tower->setOutput(FortrendVTMSignalTower::Output::GREEN_LIGHT, false);
		//check modules
		auto cassManager = wtr->getKernel()->getKernelModule<FortrendCassetteManager>();
		bool reset_loop = true;
		bool reset_finish = false;
		int rest_step = 0;
		while (reset_loop)
		{
			switch (rest_step)
			{

			case 0:
			{
				if (wtr->getState() != IKernelSubSystem::State::SUB_UNKNOWN)
				{
					logInform(reset_process_name.c_str(), "整机复位开始.");
					auto cmd = wtr->createResetCommand();
					wtr->startCommand(cmd);
					cmd->wait();
					if (cmd->hasError())
					{
						rest_step = 15000;
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
					lk1->getState() != IKernelSubSystem::State::SUB_UNKNOWN &&
					lk2->getState() != IKernelSubSystem::State::SUB_UNKNOWN &&
					//pm1->getState() != IKernelSubSystem::State::SUB_UNKNOWN &&
					pm2->getState() != IKernelSubSystem::State::SUB_UNKNOWN &&
				/*	pm3->getState() != IKernelSubSystem::State::SUB_UNKNOWN &&*/
					tm->getState() != IKernelSubSystem::State::SUB_UNKNOWN)
				{
					auto cmd_lk1 = lk1->createResetCommand();
					lk1->startCommand(cmd_lk1);

					auto cmd_lk2 = lk2->createResetCommand();
					lk2->startCommand(cmd_lk2);

					/*auto cmd_pm1 = pm1->createResetCommand();
					pm1->startCommand(cmd_pm1);*/

					auto cmd_pm2 = pm2->createResetCommand();
					pm2->startCommand(cmd_pm2);

				/*	auto cmd_pm3 = pm3->createResetCommand();
					pm3->startCommand(cmd_pm3);*/

					auto cmd_tm = tm->createResetCommand();
					tm->startCommand(cmd_tm);

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
					/*if (pm1->getState() == IKernelSubSystem::State::SUB_UNKNOWN)
					{
						logError(reset_process_name.c_str(), Poco::format("%s不在正常状态.", pm1->getName()).c_str());
					}*/
					if (pm2->getState() == IKernelSubSystem::State::SUB_UNKNOWN)
					{
						logError(reset_process_name.c_str(), Poco::format("%s不在正常状态.", pm2->getName()).c_str());
					}
					/*if (pm3->getState() == IKernelSubSystem::State::SUB_UNKNOWN)
					{
						logError(reset_process_name.c_str(), Poco::format("%s不在正常状态.", pm3->getName()).c_str());
					}*/
					if (tm->getState() == IKernelSubSystem::State::SUB_UNKNOWN)
					{
						logError(reset_process_name.c_str(), Poco::format("%s不在正常状态.", tm->getName()).c_str());
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
					tm->getState() == IKernelSubSystem::State::SUB_NORMAL
					)
				{
					rest_step = 200;

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
					/*if (pm1->getState() != IKernelSubSystem::State::SUB_NORMAL)
					{
						logError(reset_process_name.c_str(), Poco::format("%s不在正常状态.", pm1->getName()).c_str());
					}*/
					if (pm2->getState() != IKernelSubSystem::State::SUB_NORMAL)
					{
						logError(reset_process_name.c_str(), Poco::format("%s不在正常状态.", pm2->getName()).c_str());
					}
					/*if (pm3->getState() != IKernelSubSystem::State::SUB_NORMAL)
					{
						logError(reset_process_name.c_str(), Poco::format("%s不在正常状态.", pm3->getName()).c_str());
					}*/
					if (tm->getState() != IKernelSubSystem::State::SUB_NORMAL)
					{
						logError(reset_process_name.c_str(), Poco::format("%s不在正常状态.", tm->getName()).c_str());
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
			/*	else if (cmd_pm3->hasError())
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
				logInform(reset_process_name.c_str(), "整机复位完成.");
				robot_auto_step = 10;
				loadlock1_auto_step = 10;
				loadlock2_auto_step = 10;
				vacuum_auto_step = 10;
				reset_loop = false;
				reset_finish = true;
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
			onUpdateControlEnabled("loadlock1_put_cassette_finished_pbt", false);
			
		}
		onUpdateControlEnabled("reset_pbt", true);

	}

	void QSlotTransferAutoVTMWidgetPrivate::moniterPMCavityAlarmAction(){
		//std::shared_ptr<FortrendPMCavitySubsystem> pm1 = kernel->getKernelModule<FortrendPMCavitySubsystem>("PM1");
		std::shared_ptr<FortrendPMCavitySubsystem> pm2 = kernel->getKernelModule<FortrendPMCavitySubsystem>("PM2");
		//std::shared_ptr<FortrendPMCavitySubsystem> pm3 = kernel->getKernelModule<FortrendPMCavitySubsystem>("PM3");
		/*if (!pm1){ return; }
		if (!pm2){ return; }
		if (!pm3){ return; }*/
		bool pm1_show = false;
		bool pm2_show = false;
		bool pm3_show = false;
		int pm1_count = 0;
		int pm2_count = 0;
		int pm3_count = 0;
		while (thread_enabled)
		{
			while (thread_enabled)
			{
				//if (pm1&&pm1->getPMCavityHasAlarm())
				//{
				//	++pm1_count;
				//	if (!pm1_show && (pm1_count > 3))
				//	{
				//		pm1_show = true;

				//		QString pm1_message = QString::fromStdString(pm1->getPMCavityAlarmMessage());
				//		if (pm1_message.length() < 1)
				//		{
				//			pm1_message = "未知错误！";
				//			pm1_message = QString::fromStdString(pm1->getName()).append("：").append(pm1_message);
				//			continue;
				//		}

				//		QMetaObject::invokeMethod(q_ptr, "showMessage", Qt::AutoConnection,
				//			Q_ARG(QString, pm1_message));
				//	}

				//}
				//else
				//{
				//	pm1_count = 0;
				//	if (pm1_show)
				//	{
				//		pm1_show = false;
				//	}

				//}
				if (pm2&&pm2->getPMCavityHasAlarm())
				{
					++pm2_count;
					if (!pm2_show && (pm2_count > 3))
					{
						pm2_show = true;
						QString pm2_message = QString::fromStdString(pm2->getPMCavityAlarmMessage());
						if (pm2_message.length() < 1)
						{
							pm2_message = "未知错误！";
							pm2_message = QString::fromStdString(pm2->getName()).append("：").append(pm2_message);
							continue;
						}

						QMetaObject::invokeMethod(q_ptr, "showMessage", Qt::AutoConnection,
							Q_ARG(QString, pm2_message));
					}

				}
				else{
					pm2_count = 0;
					if (pm2_show)
					{
						pm2_show = false;
					}

				}
				/*if (pm3&&pm3->getPMCavityHasAlarm())
				{
					++pm3_count;
					if (!pm3_show && (pm3_count > 3))
					{
						pm3_show = true;
						QString pm3_message = QString::fromStdString(pm3->getPMCavityAlarmMessage());
						if (pm3_message.length() < 1)
						{
							pm3_message = "未知错误！";
							pm3_message = QString::fromStdString(pm3->getName()).append("：").append(pm3_message);
							continue;
						}

						QMetaObject::invokeMethod(q_ptr, "showMessage", Qt::AutoConnection,
							Q_ARG(QString, pm3_message));
					}


				}
				else{
					pm3_count = 0;
					if (pm3_show)
					{
						pm3_show = false;
					}

				}*/
				Sleep(200);
			}
			Sleep(200);
		}
	}

	void QSlotTransferAutoVTMWidgetPrivate::moniterPMCavityProcessDataAction(){
		//std::shared_ptr<FortrendPMCavitySubsystem> pm1 = kernel->getKernelModule<FortrendPMCavitySubsystem>("PM1");
		std::shared_ptr<FortrendPMCavitySubsystem> pm2 = kernel->getKernelModule<FortrendPMCavitySubsystem>("PM2");
		//std::shared_ptr<FortrendPMCavitySubsystem> pm3 = kernel->getKernelModule<FortrendPMCavitySubsystem>("PM3");
		//if (!pm1){ return; }
		//if (!pm2){ return; }
		//if (!pm3){ return; }
		float current_pm1_coating_time = 0;
		short current_pm1_processing_step = 0;
		float current_pm2_coating_time = 0;
		short current_pm2_processing_step = 0;
		float current_pm3_coating_time = 0;
		short current_pm3_processing_step = 0;

		while (thread_enabled)
		{
			//bool change_pm1 = false;
			//current_pm1_coating_time = pm1->getPMCavityCoatingTime();
			//if (current_pm1_coating_time != coating_time_pm1)
			//{
			//	coating_time_pm1 = current_pm1_coating_time;
			//	change_pm1 = true;
			//}
			//current_pm1_processing_step = pm1->getPMCavityProcessingStep();
			//if (current_pm1_processing_step != processing_step_pm1)
			//{
			//	processing_step_pm1 = current_pm1_processing_step;
			//	change_pm1 = true;
			//}
			//if (change_pm1)
			//{
			//	QMetaObject::invokeMethod(q_ptr, "updateAutoData", Qt::AutoConnection,
			//		Q_ARG(QString, "PM1"));
			//}
			bool change_pm2 = false;
			current_pm2_coating_time = pm2->getPMCavityCoatingTime();
			if (current_pm2_coating_time != coating_time_pm2)
			{
				coating_time_pm2 = current_pm2_coating_time;
				change_pm2 = true;
			}
			current_pm2_processing_step = pm2->getPMCavityProcessingStep();
			if (current_pm2_processing_step != processing_step_pm2)
			{
				processing_step_pm2 = current_pm2_processing_step;
				change_pm2 = true;
			}
			if (change_pm2)
			{
				QMetaObject::invokeMethod(q_ptr, "updateAutoData", Qt::AutoConnection,
					Q_ARG(QString, "PM"));
			}
			/*bool change_pm3 = false;
			current_pm3_coating_time = pm3->getPMCavityCoatingTime();
			if (current_pm3_coating_time != coating_time_pm3)
			{
				coating_time_pm3 = current_pm3_coating_time;
				change_pm3 = true;
			}
			current_pm3_processing_step = pm3->getPMCavityProcessingStep();
			if (current_pm3_processing_step != processing_step_pm3)
			{
				processing_step_pm3 = current_pm3_processing_step;
				change_pm3 = true;
			}
			if (change_pm3)
			{
				QMetaObject::invokeMethod(q_ptr, "updateAutoData", Qt::AutoConnection,
					Q_ARG(QString, "PM3"));
			}*/
			Sleep(100);
		}
	}

	void QSlotTransferAutoVTMWidgetPrivate::startVacuumAction(){

		std::shared_ptr<FortrendLoadLockSubsystem> lk1 = kernel->getKernelModule<FortrendLoadLockSubsystem>("LLA");
		std::shared_ptr<FortrendSunwayRobotSubsystem> wtr = kernel->getKernelModule<FortrendSunwayRobotSubsystem>("WTR");
		std::shared_ptr<FortrendTMCavitySubsystem> tm = kernel->getKernelModule<FortrendTMCavitySubsystem>("TM");
		std::shared_ptr<FortrendLoadLockSubsystem> lk2 = kernel->getKernelModule<FortrendLoadLockSubsystem>("LLB");
		//std::shared_ptr<FortrendPMCavitySubsystem> pm1 = kernel->getKernelModule<FortrendPMCavitySubsystem>("PM1");
		std::shared_ptr<FortrendPMCavitySubsystem> pm2 = kernel->getKernelModule<FortrendPMCavitySubsystem>("PM2");
		//std::shared_ptr<FortrendPMCavitySubsystem> pm3 = kernel->getKernelModule<FortrendPMCavitySubsystem>("PM3");
		std::shared_ptr<FortrendPumpSubsystem> pump = kernel->getKernelModule<FortrendPumpSubsystem>("PUMP");
	/*	if (!pm1){ return; }
		if (!pm2){ return; }
		if (!pm3){ return; }*/
		while (thread_enabled)
		{
			while (running)
			{
				vacumm_step_once_finished = false;
				if (tm->getVacuumEnable())
				{
					//打开机械泵
					if (pump->getMechanicalPumpOpened() == false)
					{
						if (pump->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
							auto cmd = pump->createMechanicalOpenCommand();
							pump->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError())
							{
								logFailedExcuteCommandHasError(pump->getName(), "打开机械泵", vacuum_process_name, 1);
							}
							else
							{
								logInform(pump->getName().c_str(), "打开机械泵成功.");
							}
						}
						else
						{
							logFailedNotNormal(pump->getName(), vacuum_process_name, 1);
						}
					}
					//分子泵关闭
					if (pump->getMolecularPumpOpenedTM() == false)
					{
						if (loadlock1_get_vacuum)
						{
							if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
							{
								if (lk1->getSlowDiaphragmValveOpend() || lk1->getFastDiaphragmValveOpend())
								{
									auto cmd = lk1->createCloseDiaphragmValveCommand(LoadLockValveOpening::LoadLock_Both);
									lk1->startCommand(cmd);
									cmd->wait();
									if (cmd->hasError())
									{
										logFailedExcuteCommandHasError(lk1->getName(), "关闭隔膜阀", vacuum_process_name, 15);
										break;
									}
									else
									{
										logInform(lk1->getName().c_str(), "关闭隔膜阀成功步骤15.");
									}
								}
								if (lk1->getAngleValveOpend() == false)
								{
									auto cmd = lk1->createOpenAngleValveCommand();
									lk1->startCommand(cmd);
									cmd->wait();
									if (cmd->hasError())
									{
										logFailedExcuteCommandHasError(lk1->getName(), "打开角阀", vacuum_process_name, 20);
										break;
									}
									else
									{
										logInform(lk1->getName().c_str(), "打开角阀成功步骤20.");
									}
								}
							}
							else
							{
								logFailedNotNormal(lk1->getName(), vacuum_process_name, 15);
							}

							if (lk1->getVacuumValueReachesTheSetValue())
							{
								loadlock1_get_vacuum = false;
							}

						}
						if (loadlock2_get_vacuum)
						{
							if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
							{
								if (lk2->getSlowDiaphragmValveOpend() || lk2->getFastDiaphragmValveOpend())
								{
									auto cmd = lk2->createCloseDiaphragmValveCommand(LoadLockValveOpening::LoadLock_Both);
									lk2->startCommand(cmd);
									cmd->wait();
									if (cmd->hasError())
									{
										logFailedExcuteCommandHasError(lk2->getName(), "关闭隔膜阀", vacuum_process_name, 25);
										break;
									}
									else
									{
										logInform(lk2->getName().c_str(), "关闭隔膜阀成功步骤25.");
									}
								}
								if (lk2->getAngleValveOpend() == false)
								{
									auto cmd = lk2->createOpenAngleValveCommand();
									lk2->startCommand(cmd);
									cmd->wait();
									if (cmd->hasError())
									{
										logFailedExcuteCommandHasError(lk2->getName(), "打开角阀", vacuum_process_name, 30);
										break;
									}
									else
									{
										logInform(lk2->getName().c_str(), "打开角阀成功步骤30.");
									}
								}
							}
							else
							{
								logFailedNotNormal(lk2->getName(), vacuum_process_name, 15);
							}

							if (lk2->getVacuumValueReachesTheSetValue())
							{
								loadlock2_get_vacuum = false;
							}
						}
						if (tm_get_vacuum)
						{
							if (tm->getSlowDiaphragmValveOpend() || tm->getFastDiaphragmValveOpend())
							{
								if (tm->getState() == IKernelSubSystem::State::SUB_NORMAL)
								{
									auto cmd = tm->createCloseDiaphragmValveCommand(TMCavityValveOpening::TMCavity_Both);
									tm->startCommand(cmd);
									cmd->wait();
									if (cmd->hasError())
									{
										logFailedExcuteCommandHasError(tm->getName(), "关闭隔膜阀", vacuum_process_name, 45);
										break;
									}
									else
									{
										logInform(tm->getName().c_str(), "关闭隔膜阀成功步骤45.");
									}
								}
								else{
									logFailedNotNormal(tm->getName(), vacuum_process_name, 45);
								}
							}
							if (tm->getAngleValveOpend() == false)
							{
								if (tm->getState() == IKernelSubSystem::State::SUB_NORMAL)
								{
									//打开角阀
									auto cmd = tm->createOpenAngleValveCommand();
									tm->startCommand(cmd);
									cmd->wait();
									if (cmd->hasError())
									{
										logFailedExcuteCommandHasError(tm->getName(), "打开角阀", vacuum_process_name, 50);
									}
									else{
										logInform(tm->getName().c_str(), "打开角阀成功步骤50.");
									}
								}
								else{
									logFailedNotNormal(tm->getName(), vacuum_process_name, 50);
								}
							}
							//粗抽压力
							if (tm->getTMCavityRoughVacuumReachesTheSetValue() == false)
							{
								if (tm->getState() == IKernelSubSystem::State::SUB_NORMAL)
								{
									//关闭插板阀
									if (tm->getInsertingPlateValveOpend())
									{
										auto cmd = tm->createCloseInsertingPlateValveCommand();
										tm->startCommand(cmd);
										cmd->wait();
										if (cmd->hasError())
										{
											logFailedExcuteCommandHasError(tm->getName(), "关闭插板阀", vacuum_process_name, 60);
											break;
										}
									}
									//打开高真空挡板阀
									if (tm->getHeightVacuumBaffleValveOpend() == false)
									{
										auto cmd = tm->createOpenHeightVacuumBaffleValveCommand();
										tm->startCommand(cmd);
										cmd->wait();
										if (cmd->hasError())
										{
											logFailedExcuteCommandHasError(tm->getName(), "打开高真空挡板阀", vacuum_process_name, 70);
											break;
										}
									}
								}
								else{
									logFailedNotNormal(tm->getName(), vacuum_process_name, 60);
								}
							}
							else
							{
								if (loadlock1_get_vacuum == false && loadlock2_get_vacuum == false)
								{
									//关闭插板阀
									if (tm->getInsertingPlateValveOpend())
									{
										if (tm->getState() == IKernelSubSystem::State::SUB_NORMAL)
										{
											auto cmd = tm->createCloseInsertingPlateValveCommand();
											tm->startCommand(cmd);
											cmd->wait();
											if (cmd->hasError())
											{
												logFailedExcuteCommandHasError(tm->getName(), "关闭插板阀", vacuum_process_name, 80);
												break;
											}
										}
										else
										{
											logFailedNotNormal(tm->getName(), vacuum_process_name, 80);
										}
									}
									//打开分子泵
									if (lk1->getVacuumValueReachesTheSetValue() && lk2->getVacuumValueReachesTheSetValue()
										&& tm->getTMCavityRoughVacuumReachesTheSetValue() && tm->getAngleValveOpend() &&
										tm->getMoleculePipelineVacuumValue() < 10.0)
									{
										if (pump->getMolecularPumpOpenedTM() == false && pump->getState() == IKernelSubSystem::State::SUB_NORMAL)
										{
											auto cmd = pump->createMolecularOpenCommand("TM");
											pump->startCommand(cmd);
											cmd->wait();
											if (cmd->hasError())
											{
												logFailedExcuteCommandHasError(pump->getName(), "打开分子泵", vacuum_process_name, 90);
												break;
											}
											else
											{
												logInform(pump->getName().c_str(), "打开分子泵完成.");
											}
										}
										else{
											logFailedNotNormal(pump->getName(), vacuum_process_name, 90);
										}
									}
								}

							}


						}
					}
					//分子泵打开
					else
					{
						switch (vacuum_auto_step)
						{
						case 10:
						{
							if (loadlock1_get_vacuum)
							{
								vacuum_auto_step = 1000;
							}
							else if (loadlock2_get_vacuum)
							{
								vacuum_auto_step = 2000;
							}
							else if (tm_get_vacuum)
							{
								if (tm->getTMCavityRoughVacuumReachesTheSetValue() == false)
								{
									vacuum_auto_step = 5000;
								}
								else{
									vacuum_auto_step = 5100;
								}
							}
							else if (tm_air_inflation)
							{
								vacuum_auto_step = 10000;
							}
							else{

							}
						}
						break;
						case 1000:
						{
							if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
							{
								if (lk1->getSlowDiaphragmValveOpend() || lk1->getFastDiaphragmValveOpend())
								{
									auto cmd = lk1->createCloseDiaphragmValveCommand(LoadLockValveOpening::LoadLock_Both);
									lk1->startCommand(cmd);
									cmd->wait();
									if (cmd->hasError())
									{
										logFailedExcuteCommandHasError(lk1->getName(), "关闭隔膜阀", vacuum_process_name, vacuum_auto_step);

									}
									else
									{
										vacuum_auto_step = 1010;
									}
								}
								else{
									vacuum_auto_step = 1010;
								}


							}
							else
							{
								logFailedNotNormal(lk1->getName(), vacuum_process_name, vacuum_auto_step);
							}
						}
						break;
						case 1010:
						{
							if (tm->getState() == IKernelSubSystem::State::SUB_NORMAL)
							{
								tm_angle_valve_recard_state = tm->getAngleValveOpend();
								if (tm_angle_valve_recard_state)
								{
									auto cmd = tm->createCloseAngleValveCommand();
									tm->startCommand(cmd);
									cmd->wait();
									if (cmd->hasError())
									{
										logFailedExcuteCommandHasError(tm->getName(), "关闭角阀", vacuum_process_name, vacuum_auto_step);
									}
									else{
										vacuum_auto_step = 1030;
									}
								}
								else{
									vacuum_auto_step = 1030;
								}
							}
							else
							{
								logFailedNotNormal(tm->getName(), vacuum_process_name, vacuum_auto_step);
							}
						}
						break;

						case 1030:
						{
							if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
							{
								//后打开角阀
								if (lk1->getAngleValveOpend() == false)
								{
									auto cmd = lk1->createOpenAngleValveCommand();
									lk1->startCommand(cmd);
									cmd->wait();
									if (cmd->hasError())
									{
										logFailedExcuteCommandHasError(lk1->getName(), "打开角阀", vacuum_process_name, vacuum_auto_step);
									}
									else
									{
										vacuum_auto_step = 1040;
									}
								}
								else{
									vacuum_auto_step = 1040;
								}


							}
							else
							{
								logFailedNotNormal(lk1->getName(), vacuum_process_name, vacuum_auto_step);
							}
						}
						break;
						case 1040:
						{
							if (lk1->getVacuumValueReachesTheSetValue())
							{
								loadlock1_get_vacuum = false;
								if (tm_angle_valve_recard_state)
								{
									vacuum_auto_step = 1050;
								}
								else{

									vacuum_auto_step = 1060;
								}
							}
						}
						break;
						case 1050:
						{
							if (tm->getState() == IKernelSubSystem::State::SUB_NORMAL)
							{
								auto cmd = tm->createOpenAngleValveCommand();
								tm->startCommand(cmd);
								cmd->wait();
								if (cmd->hasError())
								{
									logFailedExcuteCommandHasError(tm->getName(), "打开角阀", vacuum_process_name, vacuum_auto_step);
								}
								else{
									vacuum_auto_step = 1060;
								}
							}
							else
							{
								logFailedNotNormal(tm->getName(), vacuum_process_name, vacuum_auto_step);
							}
						}
						break;
						case 1060:
						{
							if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
							{
								auto cmd = lk1->createCloseAngleValveCommand();
								lk1->startCommand(cmd);
								cmd->wait();
								if (cmd->hasError())
								{
									logFailedExcuteCommandHasError(lk1->getName(), "关闭角阀", vacuum_process_name, vacuum_auto_step);
								}
								else{
									vacuum_auto_step = 10;
								}
							}
							else
							{
								logFailedNotNormal(lk1->getName(), vacuum_process_name, vacuum_auto_step);
							}
						}
						break;

						case 2000:
						{
							if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
							{
								if (lk2->getSlowDiaphragmValveOpend() || lk2->getFastDiaphragmValveOpend())
								{
									auto cmd = lk2->createCloseDiaphragmValveCommand(LoadLockValveOpening::LoadLock_Both);
									lk2->startCommand(cmd);
									cmd->wait();
									if (cmd->hasError())
									{
										logFailedExcuteCommandHasError(lk2->getName(), "关闭隔膜阀", vacuum_process_name, vacuum_auto_step);

									}
									else
									{
										vacuum_auto_step = 2010;
									}
								}
								else{
									vacuum_auto_step = 2010;
								}


							}
							else
							{
								logFailedNotNormal(lk2->getName(), vacuum_process_name, vacuum_auto_step);
							}
						}
						break;
						case 2010:
						{
							if (tm->getState() == IKernelSubSystem::State::SUB_NORMAL)
							{
								tm_angle_valve_recard_state = tm->getAngleValveOpend();
								if (tm_angle_valve_recard_state)
								{
									auto cmd = tm->createCloseAngleValveCommand();
									tm->startCommand(cmd);
									cmd->wait();
									if (cmd->hasError())
									{
										logFailedExcuteCommandHasError(tm->getName(), "关闭角阀", vacuum_process_name, vacuum_auto_step);
									}
									else{
										vacuum_auto_step = 2030;
									}
								}
								else{
									vacuum_auto_step = 2030;
								}
							}
							else
							{
								logFailedNotNormal(tm->getName(), vacuum_process_name, vacuum_auto_step);
							}
						}
						break;

						case 2030:
						{
							if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
							{
								//后打开角阀
								if (lk2->getAngleValveOpend() == false)
								{
									auto cmd = lk2->createOpenAngleValveCommand();
									lk2->startCommand(cmd);
									cmd->wait();
									if (cmd->hasError())
									{
										logFailedExcuteCommandHasError(lk2->getName(), "打开角阀", vacuum_process_name, vacuum_auto_step);
									}
									else
									{
										vacuum_auto_step = 2040;
									}
								}
								else{
									vacuum_auto_step = 2040;
								}


							}
							else
							{
								logFailedNotNormal(lk2->getName(), vacuum_process_name, vacuum_auto_step);
							}
						}
						break;
						case 2040:
						{
							if (lk2->getVacuumValueReachesTheSetValue())
							{
								loadlock2_get_vacuum = false;
								if (tm_angle_valve_recard_state)
								{
									vacuum_auto_step = 2050;
								}
								else{

									vacuum_auto_step = 2060;
								}
							}
							else{
								Sleep(100);
							}
						}
						break;
						case 2050:
						{
							if (tm->getState() == IKernelSubSystem::State::SUB_NORMAL)
							{
								auto cmd = tm->createOpenAngleValveCommand();
								tm->startCommand(cmd);
								cmd->wait();
								if (cmd->hasError())
								{
									logFailedExcuteCommandHasError(tm->getName(), "打开角阀", vacuum_process_name, vacuum_auto_step);
								}
								else{
									vacuum_auto_step = 2060;
								}
							}
							else
							{
								logFailedNotNormal(tm->getName(), vacuum_process_name, vacuum_auto_step);
							}
						}
						break;
						case 2060:
						{
							if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
							{
								auto cmd = lk2->createCloseAngleValveCommand();
								lk2->startCommand(cmd);
								cmd->wait();
								if (cmd->hasError())
								{
									logFailedExcuteCommandHasError(lk2->getName(), "关闭角阀", vacuum_process_name, vacuum_auto_step);
								}
								else{
									vacuum_auto_step = 10;
								}
							}
							else
							{
								logFailedNotNormal(lk2->getName(), vacuum_process_name, vacuum_auto_step);
							}
						}
						break;

						case 5000:
						{
							if (tm->getState() == IKernelSubSystem::State::SUB_NORMAL)
							{
								//关闭插板阀
								if (tm->getInsertingPlateValveOpend())
								{
									auto cmd = tm->createCloseInsertingPlateValveCommand();
									tm->startCommand(cmd);
									cmd->wait();
									if (cmd->hasError())
									{
										logFailedExcuteCommandHasError(tm->getName(), "关闭插板阀", vacuum_process_name, vacuum_auto_step);
									}
									else{
										vacuum_auto_step = 5010;
									}
								}
								else{
									vacuum_auto_step = 5010;
								}
							}
							else
							{
								logFailedNotNormal(tm->getName(), vacuum_process_name, vacuum_auto_step);
							}
						}
						break;
						case 5010:
						{
							if (tm->getState() == IKernelSubSystem::State::SUB_NORMAL)
							{
								if (tm->getAngleValveOpend())
								{
									//关闭角阀
									auto cmd = tm->createCloseAngleValveCommand();
									tm->startCommand(cmd);
									cmd->wait();
									if (cmd->hasError())
									{
										logFailedExcuteCommandHasError(tm->getName(), "关闭角阀", vacuum_process_name, vacuum_auto_step);
									}
									else{
										vacuum_auto_step = 5020;
									}
								}
								else{
									vacuum_auto_step = 5020;
								}
							}
							else
							{
								logFailedNotNormal(tm->getName(), vacuum_process_name, vacuum_auto_step);
							}
						}
						break;
						case 5020:
						{
							if (tm->getState() == IKernelSubSystem::State::SUB_NORMAL)
							{
								//打开高真空挡板阀
								if (tm->getHeightVacuumBaffleValveOpend() == false)
								{
									auto cmd = tm->createOpenHeightVacuumBaffleValveCommand();
									tm->startCommand(cmd);
									cmd->wait();
									if (cmd->hasError())
									{
										logFailedExcuteCommandHasError(tm->getName(), "打开高真空挡板阀", vacuum_process_name, vacuum_auto_step);
										break;
									}
									else
									{
										vacuum_auto_step = 5050;
									}
								}
								else{
									vacuum_auto_step = 5050;
								}
							}
							else
							{
								logFailedNotNormal(tm->getName(), vacuum_process_name, vacuum_auto_step);
							}
						}
						break;
						case 5050:
						{
							//达到粗抽压力
							if (tm->getTMCavityRoughVacuumReachesTheSetValue())
							{
								vacuum_auto_step = 5100;
							}
							else
							{
								Sleep(100);
							}
						}
						break;


						//
						case 5100:
						{
							if (tm->getState() == IKernelSubSystem::State::SUB_NORMAL)
							{
								//打开角阀
								auto cmd = tm->createOpenAngleValveCommand();
								tm->startCommand(cmd);
								cmd->wait();
								if (cmd->hasError())
								{
									logFailedExcuteCommandHasError(tm->getName(), "打开角阀", vacuum_process_name, vacuum_auto_step);
								}
								else{
									vacuum_auto_step = 5110;
								}
							}
							else
							{
								logFailedNotNormal(tm->getName(), vacuum_process_name, vacuum_auto_step);
							}
						}
						break;
						case 5110:
						{
							//打开分子泵
							if (tm->getTMCavityRoughVacuumReachesTheSetValue() && tm->getAngleValveOpend())
							{
								if (pump->getMolecularPumpOpenedTM() == false || pump->getMolecularPumpReachSpeedTM() == false)
								{
									if (pump->getState() == IKernelSubSystem::State::SUB_NORMAL)
									{
										auto cmd = pump->createMolecularOpenCommand("TM");
										pump->startCommand(cmd);
										cmd->wait();
										if (cmd->hasError())
										{
											logFailedExcuteCommandHasError(pump->getName(), "打开分子泵", vacuum_process_name, vacuum_auto_step);

										}
										else
										{
											vacuum_auto_step = 5120;
										}
									}
									else{
										logFailedNotNormal(pump->getName(), vacuum_process_name, vacuum_auto_step);
									}
								}
								else
								{
									vacuum_auto_step = 5120;
								}
							}
							else
							{
								vacuum_auto_step = 5000;
							}


						}
						break;
						case 5120:
						{
							//打开分子泵
							if (tm->getTMCavityRoughVacuumReachesTheSetValue() && tm->getAngleValveOpend())
							{
								if (pump->getMolecularPumpReachSpeedTM())
								{
									vacuum_auto_step = 5150;

								}
								else{
									vacuum_auto_step = 5110;
								}
							}
							else{
								Sleep(100);
							}

						}
						break;
						case 5150:
						{
							if (tm->getState() == IKernelSubSystem::State::SUB_NORMAL)
							{
								//关闭高真空挡板阀
								if (tm->getHeightVacuumBaffleValveOpend())
								{
									auto cmd = tm->createCloseHeightVacuumBaffleValveCommand();
									tm->startCommand(cmd);
									cmd->wait();
									if (cmd->hasError())
									{
										logFailedExcuteCommandHasError(tm->getName(), "关闭高真空挡板阀", vacuum_process_name, vacuum_auto_step);
									}
									else
									{
										vacuum_auto_step = 5160;
									}
								}
								else{
									vacuum_auto_step = 5160;
								}
							}
							else
							{
								logFailedNotNormal(tm->getName(), vacuum_process_name, vacuum_auto_step);
							}

						}
						break;
						case 5160:
						{
							if (tm->getState() == IKernelSubSystem::State::SUB_NORMAL)
							{
								if (tm->getFastDiaphragmValveOpend() || tm->getSlowDiaphragmValveOpend())
								{
									//关闭隔膜阀
									auto cmd = tm->createCloseDiaphragmValveCommand(TMCavityValveOpening::TMCavity_Both);
									tm->startCommand(cmd);
									cmd->wait();
									if (cmd->hasError())
									{
										logFailedExcuteCommandHasError(tm->getName(), "关闭隔膜阀", vacuum_process_name, vacuum_auto_step);
									}
									else{
										vacuum_auto_step = 5170;
									}
								}
								else{
									vacuum_auto_step = 5170;
								}
							}
							else
							{
								logFailedNotNormal(tm->getName(), vacuum_process_name, vacuum_auto_step);
							}

						}
						break;
						case 5170:
						{
							if (tm->getState() == IKernelSubSystem::State::SUB_NORMAL)
							{
								if (tm->getInsertingPlateValveOpend() == false)
								{
									//打开插板阀
									auto cmd = tm->createOpenInsertingPlateValveCommand();
									tm->startCommand(cmd);
									cmd->wait();
									if (cmd->hasError())
									{
										logFailedExcuteCommandHasError(tm->getName(), "打开插板阀", vacuum_process_name, vacuum_auto_step);
									}
									else{
										vacuum_auto_step = 5180;
									}
								}
								else{
									vacuum_auto_step = 5180;
								}
							}
							else
							{
								logFailedNotNormal(tm->getName(), vacuum_process_name, vacuum_auto_step);
							}

						}
						break;
						case 5180:
						{
							if (tm->getTMCavityVacuumValueReachesTheSetValue())
							{
								tm_get_vacuum = false;
								vacuum_auto_step = 10;
							}
							else
							{
								if (pump->getMolecularPumpOpenedTM())
								{
									Sleep(100);
								}
								else
								{
									vacuum_auto_step = 5000;
								}
							}

						}
						break;
						case 5200:
						{
							if (tm->getState() == IKernelSubSystem::State::SUB_NORMAL)
							{
								//关闭插板阀
								auto cmd = tm->createCloseInsertingPlateValveCommand();
								tm->startCommand(cmd);
								cmd->wait();
								if (cmd->hasError())
								{
									logFailedExcuteCommandHasError(tm->getName(), "关闭插板阀", vacuum_process_name, vacuum_auto_step);
								}
								else{
									vacuum_auto_step = 5210;
								}
							}
							else
							{
								logFailedNotNormal(tm->getName(), vacuum_process_name, vacuum_auto_step);
							}

						}
						break;
						case 5210:
						{
							tm_get_vacuum = false;
							vacuum_auto_step = 10;

						}
						break;

						case 10000:
						{
							if (tm->getState() == IKernelSubSystem::State::SUB_NORMAL)
							{
								if (tm->getInsertingPlateValveOpend())
								{
									//关闭插板阀
									auto cmd = tm->createCloseInsertingPlateValveCommand();
									tm->startCommand(cmd);
									cmd->wait();
									if (cmd->hasError())
									{
										logFailedExcuteCommandHasError(tm->getName(), "关闭插板阀", vacuum_process_name, vacuum_auto_step);
									}
									else
									{
										vacuum_auto_step = 10010;
									}
								}
								else{
									vacuum_auto_step = 10010;
								}
							}
							else
							{
								logFailedNotNormal(tm->getName(), vacuum_process_name, vacuum_auto_step);
							}
						}
						break;

						case 10010:
						{
							if (tm->getState() == IKernelSubSystem::State::SUB_NORMAL)
							{
								if (tm->getHeightVacuumBaffleValveOpend())
								{
									//关闭高真空挡板阀
									auto cmd = tm->createCloseHeightVacuumBaffleValveCommand();
									tm->startCommand(cmd);
									cmd->wait();
									if (cmd->hasError())
									{
										logFailedExcuteCommandHasError(tm->getName(), "关闭高真空挡板阀", vacuum_process_name, vacuum_auto_step);
										break;
									}
									else
									{
										vacuum_auto_step = 10020;
									}
								}
								else{
									vacuum_auto_step = 10020;
								}
							}
							else
							{
								logFailedNotNormal(tm->getName(), vacuum_process_name, vacuum_auto_step);
							}
						}
						break;
						case 10020:
						{
							if (tm->getState() == IKernelSubSystem::State::SUB_NORMAL)
							{
								if (tm->getSlowDiaphragmValveOpend() == false)
								{
									//打开隔膜阀
									auto cmd = tm->createOpenDiaphragmValveCommand(TMCavityValveOpening::TMCavity_Slow);
									tm->startCommand(cmd);
									cmd->wait();
									if (cmd->hasError())
									{
										logFailedExcuteCommandHasError(tm->getName(), "打开隔膜阀慢充", vacuum_process_name, vacuum_auto_step);
										break;
									}
									else
									{
										vacuum_auto_step = 10030;
									}
								}
								else{
									vacuum_auto_step = 10030;
								}
							}
							else
							{
								logFailedNotNormal(tm->getName(), vacuum_process_name, vacuum_auto_step);
							}
						}
						break;
						case 10030:
						{
							if (tm->getState() == IKernelSubSystem::State::SUB_NORMAL)
							{
								//达到粗抽压力
								if (tm->getTMCavityVacuumValueUpperLimitReachesTheSetValue() == false)
								{
									if (tm->getSlowDiaphragmValveOpend())
									{
										//关闭隔膜阀
										auto cmd = tm->createCloseDiaphragmValveCommand(TMCavityValveOpening::TMCavity_Both);
										tm->startCommand(cmd);
										cmd->wait();
										if (cmd->hasError())
										{
											logFailedExcuteCommandHasError(tm->getName(), "关闭隔膜阀", vacuum_process_name, vacuum_auto_step);
										}
										else
										{
											vacuum_auto_step = 10050;
										}
									}
								}
								else{
									Sleep(50);
								}
							}
							else
							{
								logFailedNotNormal(tm->getName(), vacuum_process_name, vacuum_auto_step);
							}
						}
						break;
						case 10050:
						{
							tm_air_inflation = false;
							vacuum_auto_step = 10;
						}
						break;
						default:
							break;

						}

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
			Sleep(500);
		}
	}

	void QSlotTransferAutoVTMWidgetPrivate::startRobotAction(){
		std::shared_ptr<FortrendLoadLockSubsystem> lk1 = kernel->getKernelModule<FortrendLoadLockSubsystem>("LLA");
		std::shared_ptr<FortrendSunwayRobotSubsystem> wtr = kernel->getKernelModule<FortrendSunwayRobotSubsystem>("WTR");
		std::shared_ptr<FortrendTMCavitySubsystem> tm = kernel->getKernelModule<FortrendTMCavitySubsystem>("TM");
		std::shared_ptr<FortrendLoadLockSubsystem> lk2 = kernel->getKernelModule<FortrendLoadLockSubsystem>("LLB");
		//std::shared_ptr<FortrendPMCavitySubsystem> pm1 = kernel->getKernelModule<FortrendPMCavitySubsystem>("PM1");
		std::shared_ptr<FortrendPMCavitySubsystem> pm2 = kernel->getKernelModule<FortrendPMCavitySubsystem>("PM2");
		//std::shared_ptr<FortrendPMCavitySubsystem> pm3 = kernel->getKernelModule<FortrendPMCavitySubsystem>("PM3");
		std::shared_ptr<FortrendPumpSubsystem> pump = kernel->getKernelModule<FortrendPumpSubsystem>("PUMP");
		std::shared_ptr<FortrendAlignerSubsystem> aligner = kernel->getKernelModule<FortrendAlignerSubsystem>("Aligner");//寻边器

		//check modules
		auto cassManager = wtr->getKernel()->getKernelModule<FortrendCassetteManager>();
		/*if (!pm1){ return; }
		if (!pm2){ return; }
		if (!pm3){ return; }*/
		while (thread_enabled)
		{
			while (running)
			{
				robot_step_once_finished = false;
				switch (robot_auto_step)
				{
				case 10:
				{
					if (tm->getVacuumEnable())
					{
						robot_auto_step = 100;
					}
					else
					{
						robot_auto_step = 1000;
					}

				}
				break;

				case 100:
				{
					if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						tm_get_vacuum = true;
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
					if (tm_get_vacuum == false)
					{
						robot_auto_step = 120;
					}
					else
					{
						Sleep(100);
					}
				}
				break;
				case 120:
				{
					if (tm->getTMCavityVacuumValueUpperLimitReachesTheSetValue())
					{
						robot_auto_step = 1000;
					}
					else
					{
						robot_auto_step = 100;
					}
				}
				break;




				/* ------------------ 取LoadLock1的晶圆 ----------------------- */
				#pragma region Loadlock1
				case 1000:
				{
					if (loadlock1_allow_get_wafer)
					{
						robot_auto_step = 1010;
					}
					else
					{
						Sleep(20);
					}

				}
				break;
				case 1010:
				{
					if (lk1->getVacuumValueUpperLimitReachesTheSetValue())
					{
						robot_auto_step = 1030;
					}
					else
					{
						loadlock1_get_vacuum = true;
						robot_auto_step = 1020;
					}
				}
				break;
				case 1020:
				{
					if (loadlock1_get_vacuum == false)
					{
						robot_auto_step = 1010;
					}
					else{
						Sleep(100);
					}
				}
				break;

				case 1030:
				{
					if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{

						//LoadLock1 关闭角阀
						auto cmd = lk1->createCloseAngleValveCommand();
						lk1->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(lk1->getName(), "关闭角阀", robot_process_name, robot_auto_step);
						}
						else
						{
							robot_auto_step = 1050;
						}

					}
					else
					{
						logFailedNotNormal(lk1->getName(), robot_process_name, robot_auto_step);
					}
				}
				break;
				case 1050:
				{
					if (tm->getTMCavityVacuumValueUpperLimitReachesTheSetValue())
					{
						robot_auto_step = 1100;
					}
					else
					{
						tm_get_vacuum = true;
						robot_auto_step = 1060;
					}
				}
				break;
				case 1060:
				{
					if (tm_get_vacuum == false)
					{
						robot_auto_step = 1050;
					}
					else{
						Sleep(100);
					}
				}
				break;
				case 1100:
				{
					if (lk1->getVacuumValueUpperLimitReachesTheSetValue() && tm->getTMCavityVacuumValueUpperLimitReachesTheSetValue())
					{
						robot_auto_step = 1110;
					}
					else{
						robot_auto_step = 1010;
					}
				}
				break;
				case 1110:
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
							robot_auto_step = 1130;
						}

					}
					else
					{
						logFailedNotNormal(lk1->getName(), robot_process_name, robot_auto_step);
					}
				}
				break;
				case 1130:
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
							auto alarm_msg = cmd->alarmMessage();
							if (alarm_msg->code() == 745)
							{
								auto cmd_clear = wtr->createClearErrorCommand();
								wtr->startCommand(cmd_clear);
								cmd->wait();
								robot_auto_step = 1150;
							}
							else
							{

								robot_auto_step = -100;
							}
						}
						else{
							robot_auto_step = 1200;
						}

					}
					else
					{
						logFailedNotNormal(wtr->getName(), robot_process_name, robot_auto_step);
					}
				}
				break;
				case 1150:
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
						robot_auto_step = 1160;
					}
					else
					{
						logFailedNotNormal(wtr->getName(), robot_process_name, robot_auto_step);
					}
				}
				break;
				case 1160:
				{
					if (wtr->hasObject(robot_selected_arm)){
						auto cassManager = lk1->getKernel()->getKernelModule<FortrendCassetteManager>();
						auto station_cass = cassManager->getCassette(lk1.get());
						auto all_mapping = station_cass->getAllMapping();
						int last_slot = lk1->getLastMoveSlot() - 1;
						all_mapping[last_slot] = Cassette::Mapping::Empty;
						std::vector<int> all_slots;
						for (size_t i = 0; i < lk1->getCassetteSlotCount(); i++)
						{
							all_slots.push_back(i + 1);
						}

						station_cass->setMapping(all_slots, all_mapping);
						robot_auto_step = 1200;
					}
					else{
						logFailed(wtr->getName(), Poco::format("机械手臂上无晶圆，步骤：%d", robot_auto_step).c_str());
						robot_auto_step = 1150;

					}
				}
				break;
				case 1200:
				{
					loadlock1_allow_get_wafer = false;
					robot_auto_step = 3000;
				}
				break;
#pragma endregion

				/* ------------------ 选择工艺流程 ----------------------- */
				case 3000:
				{
					if (sequence_robot_transfer_wafer.size() > 0)
					{
						robot_auto_step = 3010;
					}
					else{
						robot_auto_step = 3030;

					}
				}
				break;
				case 3010:
				{
					/*if (sequence_robot_transfer_wafer[0].pm_name == pm1->getName())
					{
						robot_selected_arm = sequence_robot_transfer_wafer[0].selected_arm;
						robot_auto_step = 5000;
					}*/
					/*else*/ if (sequence_robot_transfer_wafer[0].pm_name == pm2->getName())
					{
						robot_selected_arm = sequence_robot_transfer_wafer[0].selected_arm;
						robot_auto_step = 10000;
					}
					/*else if (sequence_robot_transfer_wafer[0].pm_name == pm3->getName())
					{
						robot_selected_arm = sequence_robot_transfer_wafer[0].selected_arm;
						robot_auto_step = 15000;
					}*/
					else
					{
						logFailed(wtr->getName(), Poco::format("%s 传输顺序错误, 步骤 = %d", wtr->getName(), loadlock1_auto_step));
					}
				}
				break;
				case 3030:
				{

					robot_auto_step = 50000;

				}
				break;

				/* ------------------ 放PM2的晶圆 ----------------------- */
				case 10000:
				{
					if (pm2->getPMCavityRemoteMode())
					{
						robot_auto_step = 10010;
					}
					else
					{
						Sleep(1000);
					}
				}
				break;
				case 10010:
				{
					if (pm2->getPMCavityUploadRequest())
					{
						robot_auto_step = 10020;
					}
					else
					{
						Sleep(1000);
					}
				}
				break;
				case 10020:
				{
					if (pm2->getPMCavitySafeSignal())
					{
						if (tm->getVacuumEnable())
						{
							robot_auto_step = 10030;
						}
						else{
							robot_auto_step = 10100;
						}
					}
					else
					{
						Sleep(1000);
					}
				}
				break;
				case 10030:
				{
					if (tm->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						if (tm->getSlowDiaphragmValveOpend() == false)
						{
							auto cmd = tm->createOpenDiaphragmValveCommand(TMCavityValveOpening::TMCavity_Slow);
							tm->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError())
							{
								logFailedExcuteCommandHasError(tm->getName(), "打开隔膜阀", robot_process_name, robot_auto_step);
							}
							else
							{
								robot_auto_step = 10050;
							}
						}
						else
						{
							robot_auto_step = 10050;
						}

					}
					else{
						logFailedNotNormal(tm->getName(), robot_process_name, robot_auto_step);
					}
				}
				break;
				case 10050:
				{
					if (tm->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						if (tm->getPIDOpend() == false)
						{
							auto cmd = tm->createOpenPIDCommand();
							tm->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError())
							{
								logFailedExcuteCommandHasError(tm->getName(), "打开PID调节失败！", robot_process_name, robot_auto_step);
							}
							else
							{
								robot_auto_step = 10060;
							}
						}
						else
						{
							robot_auto_step = 10060;
						}
					}
					else{
						logFailedNotNormal(tm->getName(), robot_process_name, robot_auto_step);
					}
				}
				break;
				case 10060:
				{
					if (tm->getTMCavityVacuumValueReachesThePIDSetValue())
					{
						Sleep(sleep_time);
						robot_auto_step = 10080;
					}
					else
					{
						Sleep(500);
					}
				}
				break;

				case 10080:
				{
					Sleep(sleep_time);
					//达到设定值
					if ((tm->getTMCavityVacuumValue() * pm2->getPMCavityMagnitude() - pm2->getVacuumValue()) >= 0.0)
					{
						robot_auto_step = 10100;
					}
					else
					{
						robot_auto_step = 10030;
						//logInform1(tm->getName().c_str(), Poco::format("传输腔与PM腔压差未达到设定值，%s步骤:%d", robot_process_name, robot_auto_step).c_str());
					}
				}
				break;


				case 10100:
				{
					if (pm2->getVacuumValueReachesTheSetValue() && tm->getTMCavityVacuumValueUpperLimitReachesTheSetValue())
					{
						if (pm2->getState() == IKernelSubSystem::State::SUB_NORMAL)
						{
							/*auto cmd = pm2->createWriteProcessParametersCommand(sequence_robot_transfer_wafer[0].pm_parameter);
							pm2->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError())
							{
								logFailedExcuteCommandHasError(pm2->getName(), "写入工艺参数", robot_process_name, robot_auto_step);
							}
							else
							{
								robot_auto_step = 10110;
							}*/
						}
						else
						{
							logFailedNotNormal(pm2->getName(), robot_process_name, robot_auto_step);
						}
					}
					else{
						Sleep(500);
					}
				}
				break;
				case 10110:
				{
					if (pm2->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto cmd = pm2->createOpenTMCavityDoorCommand();
						pm2->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(pm2->getName(), "打开传输腔门阀", robot_process_name, robot_auto_step);
						}
						else
						{
							robot_auto_step = 10120;
						}

					}
					else
					{
						logFailedNotNormal(pm2->getName(), robot_process_name, robot_auto_step);
					}
				}
				break;
				case 10120:
				{
					if (tm->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto cmd = tm->createClosePIDCommand();
						tm->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(tm->getName(), "关闭PID调节失败！", robot_process_name, robot_auto_step);
						}
						else
						{
							robot_auto_step = 10130;
						}
					}
					else{
						logFailedNotNormal(tm->getName(), robot_process_name, robot_auto_step);
					}
				}
				break;
				case 10130:
				{
					if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto cmd = wtr->createPutCommand(pm2, robot_selected_arm, 1);
						wtr->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(wtr->getName(), "放晶圆", robot_process_name, robot_auto_step);
							auto alarm_msg = cmd->alarmMessage();
							if (alarm_msg->code() == 734 || alarm_msg->code() == 736)
							{
								auto cmd_clear = wtr->createClearErrorCommand();
								wtr->startCommand(cmd_clear);
								cmd->wait();
								robot_auto_step = 10140;

							}
							else
							{
								robot_auto_step = -100;
							}
						}
						else
						{
							robot_auto_step = 10160;
						}

					}
					else
					{
						logFailedNotNormal(wtr->getName(), robot_process_name, robot_auto_step);
					}
				}
				break;
				case 10140:
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
						robot_auto_step = 10150;
					}
					else
					{
						logFailedNotNormal(wtr->getName(), robot_process_name, robot_auto_step);
					}
				}
				break;
				case 10150:
				{
					if (wtr->hasObject(robot_selected_arm))
					{
						logFailed(wtr->getName().c_str(), Poco::format("机械手放片异常，手臂还存在晶圆片，请检查！步骤：%s", robot_auto_step).c_str());
						robot_auto_step = 10140;
					}
					else
					{
						auto pm2_cass = cassManager->getCassette(pm2.get());
						auto pm2_all_mapping = pm2_cass->getAllMapping();
						pm2_all_mapping[0] = Cassette::Mapping::Present;
						std::vector<int> pm2_all_slot;
						pm2_all_slot.push_back(1);
						pm2_cass->setMapping(pm2_all_slot, pm2_all_mapping);
						robot_auto_step = 10160;
					}
				}
				break;

				case 10160:
				{
					if (pm2->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto cmd = pm2->createCloseTMCavityDoorCommand();
						pm2->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(pm2->getName(), "关闭传输腔门阀", robot_process_name, robot_auto_step);
						}
						else
						{
							robot_auto_step = 10170;
						}

					}
					else
					{
						logFailedNotNormal(pm2->getName(), robot_process_name, robot_auto_step);
					}
				}
				break;
				case 10170:
				{
					if (tm->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto cmd = tm->createCloseDiaphragmValveCommand(TMCavityValveOpening::TMCavity_Both);
						tm->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(tm->getName(), "关闭隔膜阀", robot_process_name, robot_auto_step);
						}
						else
						{
							robot_auto_step = 10180;
						}

					}
					else
					{
						logFailedNotNormal(tm->getName(), robot_process_name, robot_auto_step);
					}
				}
				break;
				case 10180:
				{
					if (pm2->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto cmd = pm2->createUploadFinishedCommand();
						pm2->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(pm2->getName(), "上片完成", robot_process_name, robot_auto_step);
						}
						else
						{
							robot_auto_step = 10200;
						}

					}
					else
					{
						logFailedNotNormal(pm2->getName(), robot_process_name, robot_auto_step);
					}
				}
				break;

				case 10200:
				{
					if (pm2->getPMCavityGetRequest())
					{
						robot_auto_step = 10210;
					}
					else{
						Sleep(1000);
					}
				}
				break;
				case 10210:
				{
					if (pm2->getPMCavityRemoteMode())
					{
						robot_auto_step = 10220;
					}
					else{
						Sleep(1000);
					}
				}
				break;
				case 10220:
				{
					if (pm2->getPMCavitySafeSignal())
					{
						if (tm->getVacuumEnable())
						{
							robot_auto_step = 10230;
						}
						else{
							robot_auto_step = 10300;
						}
					}
					else{
						Sleep(100);
					}
				}
				break;
				case 10230:
				{
					if (tm->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						
						auto cmd = tm->createOpenDiaphragmValveCommand(TMCavityValveOpening::TMCavity_Slow);
						tm->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(tm->getName(), "打开隔膜阀", robot_process_name, robot_auto_step);
						}
						else
						{
							robot_auto_step = 10250;
						}
						

					}
					else{
						logFailedNotNormal(tm->getName(), robot_process_name, robot_auto_step);
					}
				}
				break;
				case 10250:
				{
					if (tm->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						if (tm->getPIDOpend() == false)
						{
							auto cmd = tm->createOpenPIDCommand();
							tm->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError())
							{
								logFailedExcuteCommandHasError(tm->getName(), "打开PID调节失败！", robot_process_name, robot_auto_step);
							}
							else
							{
								robot_auto_step = 10260;
							}
						}
						else
						{
							robot_auto_step = 10260;
						}
					}
					else{
						logFailedNotNormal(tm->getName(), robot_process_name, robot_auto_step);
					}
				}
				break;
				case 10260:
				{
					if (tm->getTMCavityVacuumValueReachesThePIDSetValue())
					{
						Sleep(sleep_time);
						robot_auto_step = 10280;
					}
					else
					{
						Sleep(500);
					}
				}
				break;

				case 10280:
				{
					Sleep(sleep_time);
					//达到设定值
					if ((tm->getTMCavityVacuumValue() * pm2->getPMCavityMagnitude() - pm2->getVacuumValue()) >= 0.0)
					{
						robot_auto_step = 10300;
					}
					else
					{
						robot_auto_step = 10230;
						//logInform1(tm->getName().c_str(), Poco::format("传输腔与PM腔压差未达到设定值，%s步骤:%d", robot_process_name, robot_auto_step).c_str());
					}
				}
				break;

				case 10300:
				{
					if (pm2->getVacuumValueReachesTheSetValue() && tm->getTMCavityVacuumValueUpperLimitReachesTheSetValue())
					{
						robot_auto_step = 10310;
					}
					else{
						Sleep(500);
					}
				}
				break;
				case 10310:
				{
					if (pm2->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto cmd = pm2->createOpenTMCavityDoorCommand();
						pm2->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(pm2->getName(), "打开传输腔门阀", robot_process_name, robot_auto_step);
						}
						else
						{
							robot_auto_step = 10320;
						}

					}
					else
					{
						logFailedNotNormal(pm2->getName(), robot_process_name, robot_auto_step);
					}
				}
				break;
				case 10320:
				{
					if (tm->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto cmd = tm->createClosePIDCommand();
						tm->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(tm->getName(), "关闭PID调节失败！", robot_process_name, robot_auto_step);
						}
						else
						{
							robot_auto_step = 10330;
						}
					}
					else{
						logFailedNotNormal(tm->getName(), robot_process_name, robot_auto_step);
					}
				}
				break;
				case 10330:
				{
					if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto cmd = wtr->createGetCommand(pm2, robot_selected_arm, 1);
						wtr->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							auto alarm_msg = cmd->alarmMessage();
							if (alarm_msg->code() == 745)
							{
								auto cmd_clear = wtr->createClearErrorCommand();
								wtr->startCommand(cmd_clear);
								cmd->wait();
								robot_auto_step = 10340;
							}
							else
							{
								logFailedExcuteCommandHasError(wtr->getName(), "取晶圆", robot_process_name, robot_auto_step);
								robot_auto_step = -100;
							}
						}
						else
						{
							robot_auto_step = 10360;
						}

					}
					else
					{
						logFailedNotNormal(wtr->getName(), robot_process_name, robot_auto_step);
					}
				}
				break;
				case 10340:
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
						else{
							robot_auto_step = 10350;
						}

					}
					else
					{
						logFailedNotNormal(wtr->getName(), robot_process_name, robot_auto_step);
					}
				}
				break;
				case 10350:
				{
					if (wtr->hasObject(robot_selected_arm))
					{
						auto cassManager = pm2->getKernel()->getKernelModule<FortrendCassetteManager>();
						auto station_cass = cassManager->getCassette(pm2.get());
						auto all_mapping = station_cass->getAllMapping();
						all_mapping[0] = Cassette::Mapping::Empty;
						std::vector<int> all_slots;
						all_slots.push_back(1);
						station_cass->setMapping(all_slots, all_mapping);
						robot_auto_step = 10360;
					}
					else{
						logError(wtr->getName().c_str(), "机械手臂晶圆检测异常.");
						robot_auto_step = 10340;
					}
				}
				break;


				case 10360:
				{
					if (pm2->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto cmd = pm2->createCloseTMCavityDoorCommand();
						pm2->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(pm2->getName(), "关闭传输腔门阀", robot_process_name, robot_auto_step);
						}
						else
						{
							robot_auto_step = 10370;
						}

					}
					else
					{
						logFailedNotNormal(pm2->getName(), robot_process_name, robot_auto_step);
					}
				}
				break;
				case 10370:
				{
					if (tm->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto cmd = tm->createCloseDiaphragmValveCommand(TMCavityValveOpening::TMCavity_Both);
						tm->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(tm->getName(), "关闭隔膜阀", robot_process_name, robot_auto_step);
						}
						else
						{
							robot_auto_step = 10380;
						}

					}
					else
					{
						logFailedNotNormal(tm->getName(), robot_process_name, robot_auto_step);
					}
				}
				break;
				case 10380:
				{
					if (pm2->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto cmd = pm2->createGetFinishedCommand();
						pm2->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(pm2->getName(), "取片完成", robot_process_name, robot_auto_step);
						}
						else
						{
							robot_auto_step = 10500;
						}

					}
					else
					{
						logFailedNotNormal(pm2->getName(), robot_process_name, robot_auto_step);
					}
				}
				break;
				
				case 10500:
				{
					robot_auto_step = 30000;
				}
				break;


				/* ------------------ 放LoadLock2的晶圆 ----------------------- */
				case 20000:
				{
					if (loadlock2_allow_put_wafer)
					{
						robot_auto_step = 20010;
					}
					else
					{
						Sleep(20);

					}
				}
				break;
				case 20010:
				{
					if (lk2->getVacuumValueUpperLimitReachesTheSetValue())
					{
						robot_auto_step = 20030;
					}
					else
					{
						robot_auto_step = 20020;
						loadlock2_get_vacuum = true;
					}
				}
				break;
				case 20020:
				{
					if (loadlock2_get_vacuum == false)
					{
						robot_auto_step = 20010;
					}
					else
					{
						Sleep(20);

					}
				}
				break;
				case 20030:
				{
					if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{

						//LoadLock2 关闭角阀
						auto cmd = lk2->createCloseAngleValveCommand();
						lk2->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(lk2->getName(), "关闭角阀", robot_process_name, robot_auto_step);
						}
						else
						{
							robot_auto_step = 20050;
						}

					}
					else
					{
						logFailedNotNormal(lk2->getName(), robot_process_name, robot_auto_step);
					}
				}
				break;
				case 20050:
				{
					if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						if (tm->getTMCavityVacuumValueUpperLimitReachesTheSetValue())
						{
							robot_auto_step = 20100;
						}
						else{
							robot_auto_step = 20060;
							tm_get_vacuum = true;
						}

					}
					else
					{
						logFailedNotNormal(lk2->getName(), robot_process_name, robot_auto_step);
					}
				}
				break;
				case 20060:
				{
					if (tm_get_vacuum == false)
					{
						robot_auto_step = 20100;
					}
					else
					{
						Sleep(20);

					}
				}
				break;

				case 20100:
				{
					if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						if (tm->getTMCavityVacuumValueUpperLimitReachesTheSetValue() && lk2->getVacuumValueUpperLimitReachesTheSetValue())
						{
							robot_auto_step = 20120;
						}
						else
						{
							robot_auto_step = 20010;
						}

					}
					else
					{
						logFailedNotNormal(lk2->getName(), robot_process_name, robot_auto_step);
					}
				}
				break;

				case 20120:
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
							robot_auto_step = 20130;
						}

					}
					else
					{
						logFailedNotNormal(lk2->getName(), robot_process_name, robot_auto_step);

					}

				}
				break;
				case 20130:
				{
					if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL && lk2->hasDoorOpend())
					{
						auto cmd = wtr->createPutCommand(lk2, robot_selected_arm, 1);
						wtr->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(wtr->getName(), "放晶圆", robot_process_name, robot_auto_step);
							auto alarm_msg = cmd->alarmMessage();
							if (alarm_msg->code() == 734)
							{
								auto cmd_clear = wtr->createClearErrorCommand();
								wtr->startCommand(cmd_clear);
								cmd->wait();
								robot_auto_step = 20150;
							}
							else
							{
								robot_auto_step = -100;
							}

						}
						else{
							robot_auto_step = 20200;
						}

					}
					else
					{
						logFailedNotNormal(lk2->getName(), robot_process_name, robot_auto_step);

					}
				}
				break;

				case 20150:
				{
					if (wtr->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto cmd = wtr->createCheckLoadCommand(robot_selected_arm, 21);
						wtr->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(wtr->getName(), "查询手指有无晶圆", robot_process_name, robot_auto_step);
						}
						robot_auto_step = 20160;
					}
					else
					{
						logFailedNotNormal(wtr->getName(), robot_process_name, robot_auto_step);
					}
				}
				break;
				case 20160:
				{
					if (wtr->hasObject(robot_selected_arm)){
						logFailed(wtr->getName(), Poco::format("机械手臂上有晶圆，放片失败。步骤：%d", robot_auto_step).c_str());
						robot_auto_step = 20150;
					}
					else
					{
						robot_auto_step = 20200;
					}
				}
				break;
				case 20200:
				{
					loadlock2_allow_put_wafer = false;
					if (sequence_robot_transfer_wafer.size() > 0)
					{
						robot_auto_step = 1000;
					}
					else
					{
						robot_auto_step = 50000;
					}
				}
				break;



				case 30000:
				{
					if (sequence_robot_transfer_wafer.size() <= 1)
					{
						robot_auto_step = 20000;  //完成
					}
					else
					{
						if (sequence_robot_transfer_wafer[0].slot == sequence_robot_transfer_wafer[1].slot)
						{
							robot_auto_step = 3000;
						}
						else
						{
							robot_auto_step = 20000;  //放LoadLock2出料
						}
					}
					sequence_robot_transfer_wafer.erase(sequence_robot_transfer_wafer.begin());
				}
				break;


				case 50000:
				{
					logInform("Auto", "机械手工艺流程结束.");
					robot_process_finished = true;
					robot_auto_step = 10;
				}
				break;
				default:
					break;
				}
				robot_step_once_finished = true;
				Sleep(10);
			}
			Sleep(100);
		}

	}

	void QSlotTransferAutoVTMWidgetPrivate::startLoadLock1Action()
	{
		Q_Q(QSlotTransferAutoVTMWidget);
		std::shared_ptr<FortrendLoadLockSubsystem> lk1 = kernel->getKernelModule<FortrendLoadLockSubsystem>("LLA");
		while (thread_enabled)
		{
			while (running)
			{
				loadlock1_step_once_finished = false;
				switch (loadlock1_auto_step)
				{
				case 10:
				{
					loadlock1_process_finished = false;
					if (!lk1->hasBoxPresent())
					{
						loadlock1_auto_step = 20;
					}
					else
					{
						loadlock1_auto_step = 400;
					}
				}
				break;
				case 20:
				{
					//排气压力达到设定值
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
				case 100:
				{
					if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto cmd = lk1->createCloseAngleValveCommand();
						lk1->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(lk1->getName(), "关闭角阀", loadlock1_process_name, loadlock1_auto_step);
						}
						else{
							loadlock1_auto_step = 110;
						}

					}
					else{
						logFailedNotNormal(lk1->getName(), loadlock1_process_name, loadlock1_auto_step);
					}
				}
				break;
				//打开隔膜阀1 慢充
				case 110:
				{
					if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto cmd = lk1->createOpenDiaphragmValveCommand(LoadLockValveOpening::LoadLock_Slow);
						lk1->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(lk1->getName(), "打开隔膜阀慢充", loadlock1_process_name, loadlock1_auto_step);
						}
						else{
							loadlock1_auto_step = 120;
						}
					}
					else{
						logFailedNotNormal(lk1->getName(), loadlock1_process_name, loadlock1_auto_step);
					}
				}
				break;
				case 120:
				{
					if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						loadlock1_auto_step = 130;
					}
					else{
						logFailedNotNormal(lk1->getName(), loadlock1_process_name, loadlock1_auto_step);
					}
				}
				break;
				case 130:
				{
					if (lk1->getQuickInflationValueReachesTheSetValue())
					{

						loadlock1_auto_step = 140;
					}
					else{
						Sleep(100);
					}
				}
				break;
				case 140:
				{
					if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto cmd = lk1->createCloseDiaphragmValveCommand(LoadLockValveOpening::LoadLock_Slow);
						lk1->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(lk1->getName(), "关闭隔膜阀慢充", loadlock1_process_name, loadlock1_auto_step);
						}
						else{
							loadlock1_auto_step = 150;
						}
					}
					else{
						logFailedNotNormal(lk1->getName(), loadlock1_process_name, loadlock1_auto_step);
					}
				}
				break;
				//打开隔膜阀2 快充
				case 150:
				{
					if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto cmd = lk1->createOpenDiaphragmValveCommand(LoadLockValveOpening::LoadLock_Fast);
						lk1->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(lk1->getName(), "打开隔膜阀快充", loadlock1_process_name, loadlock1_auto_step);
						}
						else{
							loadlock1_auto_step = 160;
						}
					}
					else{
						logFailedNotNormal(lk1->getName(), loadlock1_process_name, loadlock1_auto_step);
					}
				}
				break;
				case 160:
				{
					if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						if (lk1->getExhaustVacuumValueReachesTheSetValue())
						{
							loadlock1_auto_step = 170;
						}
						else{
							Sleep(20);
						}
					}
					else{
						logFailedNotNormal(lk1->getName(), loadlock1_process_name, loadlock1_auto_step);
					}
				}
				break;
				case 170:
				{
					if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto cmd = lk1->createCloseDiaphragmValveCommand(LoadLockValveOpening::LoadLock_Fast);
						lk1->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(lk1->getName(), "关闭隔膜阀快充", loadlock1_process_name, loadlock1_auto_step);
						}
						else{
							loadlock1_auto_step = 180;
						}
					}
					else{
						logFailedNotNormal(lk1->getName(), loadlock1_process_name, loadlock1_auto_step);
					}
				}
				break;
				case 180:
				{
					if (lk1->getExhaustVacuumValueReachesTheSetValue() && lk1->getVacuumPressureGageState() == 0)
					{
						loadlock1_auto_step = 300;
					}
					else{
						loadlock1_auto_step = 150;
					}
				}
				break;

				//开门放Cassette流程
				case 300:
				{
					if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{

						auto cmd = lk1->createOpenCassetteDoorCommand();
						lk1->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(lk1->getName(), "打开放晶圆盒门阀", loadlock1_process_name, loadlock1_auto_step);
						}
						else{
							loadlock1_auto_step = 310;
						}
					}
					else
					{
						logFailedNotNormal(lk1->getName(), loadlock1_process_name, loadlock1_auto_step);
					}
				}
				break;
				case 310:
				{
					if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						loadlock1_put_cassette_finished = false;
						if (loadlock1_process_finished)
						{

							loadlock1_auto_step = 320;
						}
						else{
							loadlock1_auto_step = 340;
						}

					}
					else
					{
						logFailedNotNormal(lk1->getName(), loadlock1_process_name, loadlock1_auto_step);
					}
				}
				break;
				//换Cassette
				case 320:
				{
					if (!lk1->hasBoxPresent())
					{
						loadlock1_process_finished = false;
						loadlock1_auto_step = 340;

					}
					else
					{
						Sleep(50);
					}
				}
				break;
				case 340:
				{
					if (lk1->hasBoxPresent())
					{
						ui->loadlock1_put_cassette_finished_pbt->setEnabled(true);
						loadlock1_auto_step = 350;
					}
					else
					{
						Sleep(50);
					}
				}
				break;
				case 350:
				{
					if (loadlock1_put_cassette_finished && lk1->hasBoxPresent())
					{
						ui->loadlock1_put_cassette_finished_pbt->setEnabled(false);

						loadlock1_auto_step = 400;
					}
					else
					{

						loadlock1_auto_step = 310;
					}
				}
				break;
				case 400:
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
							loadlock1_auto_step = 500;
						}
					}
					else
					{
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
							loadlock1_auto_step = 520;
						}
						else
						{
							loadlock1_auto_step = 500;
						}
					}
					else{
						Sleep(100);
					}
				}
				break;
				case 520:
				{
					if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto cmd = lk1->createCloseAngleValveCommand();
						lk1->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(lk1->getName(), "关闭角阀", loadlock1_process_name, loadlock1_auto_step);
						}
						else{
							loadlock1_auto_step = 530;
						}

					}
					else{
						logFailedNotNormal(lk1->getName(), loadlock1_process_name, loadlock1_auto_step);
					}
				}
				break;
				case 530:
				{
					if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						loadlock1_auto_step = 800;

					}
					else{
						logFailedNotNormal(lk1->getName(), loadlock1_process_name, loadlock1_auto_step);
					}
				}
				break;


				/* ---------------- Mapping流程 -------------------- */
				case 800:
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
							loadlock1_auto_step = 810;
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

				case 900:
				{
					if (sequence_loadlock1_transfer_wafer.size() > 0)
					{
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
						loadlock1_get_vacuum = true;
						loadlock1_auto_step = 920;
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
						Sleep(100);
					}
				}
				break;
				case 950:
				{
					loadlock1_auto_step = 1000;

				}
				break;

				/* ---------------- 允许取晶圆流程 -------------------- */
				case 1000:
				{

					loadlock1_move_slot_index = sequence_loadlock1_transfer_wafer[0];
					loadlock1_auto_step = 1010;

				}
				break;
				case 1010:
				{
					auto cassManager = lk1->getKernel()->getKernelModule<FortrendCassetteManager>();
					auto station_cass = cassManager->getCassette(lk1.get());
					if (station_cass->getMapping(loadlock1_move_slot_index) == Cassette::Mapping::Present)
					{
						loadlock1_auto_step = 1030;
					}
					else
					{
						logFailed(lk1->getName(), Poco::format("%s 第%d槽不存在正常片.", lk1->getName(), loadlock1_move_slot_index));
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
						loadlock1_auto_step = 1060;
					}
					else
					{
						Sleep(20);
					}
				}
				break;
				case 1060:
				{
					if (sequence_loadlock1_transfer_wafer.size() > 0)
					{
						if (sequence_loadlock1_transfer_wafer.size() >= 2 &&
							(sequence_loadlock1_transfer_wafer[0] != sequence_loadlock1_transfer_wafer[1]))
						{
							loadlock1_auto_step = 1070;
						}
						sequence_loadlock1_transfer_wafer.erase(sequence_loadlock1_transfer_wafer.begin());
					}
					else
					{
						loadlock1_auto_step = 1070;
					}
				}
				break;
				case 1070:
				{
					if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{

						auto cmd = lk1->createCloseTMCavityDoorCommand();
						lk1->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(lk1->getName(), "关闭传输腔门阀", loadlock1_process_name, loadlock1_auto_step);
						}
						else{
							loadlock1_auto_step = 1080;
						}
					}
					else
					{
						logFailedNotNormal(lk1->getName(), loadlock1_process_name, loadlock1_auto_step);
					}
				}
				break;
				case 1080:
				{
					if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						if (sequence_loadlock1_transfer_wafer.size() > 0)
						{
							loadlock1_auto_step = 900;
						}
						else{
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
							loadlock1_auto_step = 5020;
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
						Sleep(500);
						auto cmd = lk1->createMoveToSlotCommand(28);
						lk1->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(lk1->getName(), "移动到指定槽号", loadlock1_process_name, loadlock1_auto_step);
						}
						else{
							loadlock1_auto_step = 5030;
						}
					}
					else
					{
						logFailedNotNormal(lk1->getName(), loadlock1_process_name, loadlock1_auto_step);
					}
				}
				break;
				case 5030:
				{
					if (lk1->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto cmd = lk1->createCloseAngleValveCommand();
						lk1->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(lk1->getName(), "关闭角阀", loadlock1_process_name, loadlock1_auto_step);
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

					loadlock1_process_finished = true;
					loadlock1_auto_step = 6010;
				}
				break;
				case 6010:
				{
					loadlock1_auto_step = 100;

				}
				break;
				default:
					break;
				}
				loadlock1_step_once_finished = true;
				Sleep(5);
			}
			Sleep(100);
		}



	}

	void QSlotTransferAutoVTMWidgetPrivate::startLoadLock2Action(){

		std::shared_ptr<FortrendLoadLockSubsystem> lk2 = kernel->getKernelModule<FortrendLoadLockSubsystem>("LLB");
		while (thread_enabled)
		{
			while (running)
			{
				loadlock2_step_once_finished = false;
				switch (loadlock2_auto_step)
				{
				case 10:
				{
					loadlock2_process_finished = false;
					if (!lk2->hasBoxPresent())
					{
						loadlock2_auto_step = 20;
					}
					else
					{
						loadlock2_auto_step = 400;
					}
				}
				break;
				case 20:
				{
					if (!lk2->getExhaustVacuumValueReachesTheSetValue())
					{
						loadlock2_auto_step = 100;
						logInform(lk2->getName().c_str(), Poco::format("%s 步骤 = %d", lk2->getName(), loadlock2_auto_step).c_str());
					}
					else
					{
						loadlock2_auto_step = 310;
						logInform(lk2->getName().c_str(), Poco::format("%s 步骤 = %d", lk2->getName(), loadlock2_auto_step).c_str());
					}

				}
				break;

				/* ---------------- 破真空流程 -------------------- */
				case 100:
				{
					if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto cmd = lk2->createCloseAngleValveCommand();
						lk2->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(lk2->getName(), "关闭角阀", loadlock2_process_name, loadlock2_auto_step);
						}
						else{
							loadlock2_auto_step = 110;
						}

					}
					else{
						logFailedNotNormal(lk2->getName(), loadlock2_process_name, loadlock2_auto_step);
					}
				}
				break;
				//打开隔膜阀1 慢充
				case 110:
				{
					if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto cmd = lk2->createOpenDiaphragmValveCommand(LoadLockValveOpening::LoadLock_Slow);
						lk2->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(lk2->getName(), "打开隔膜阀慢充", loadlock2_process_name, loadlock2_auto_step);
						}
						else{
							loadlock2_auto_step = 120;
						}
					}
					else{
						logFailedNotNormal(lk2->getName(), loadlock2_process_name, loadlock2_auto_step);
					}
				}
				break;
				case 120:
				{
					if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						loadlock2_auto_step = 130;
					}
					else{
						logFailedNotNormal(lk2->getName(), loadlock2_process_name, loadlock2_auto_step);
					}
				}
				break;
				case 130:
				{
					if (lk2->getQuickInflationValueReachesTheSetValue())
					{

						loadlock2_auto_step = 140;
					}
					else{
						Sleep(100);
					}
				}
				break;
				case 140:
				{
					if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto cmd = lk2->createCloseDiaphragmValveCommand(LoadLockValveOpening::LoadLock_Slow);
						lk2->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(lk2->getName(), "关闭隔膜阀慢充", loadlock2_process_name, loadlock2_auto_step);
						}
						else{
							loadlock2_auto_step = 150;
						}
					}
					else{
						logFailedNotNormal(lk2->getName(), loadlock2_process_name, loadlock2_auto_step);
					}
				}
				break;
				//打开隔膜阀2 快充
				case 150:
				{
					if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto cmd = lk2->createOpenDiaphragmValveCommand(LoadLockValveOpening::LoadLock_Fast);
						lk2->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(lk2->getName(), "打开隔膜阀快充", loadlock2_process_name, loadlock2_auto_step);
						}
						else{
							loadlock2_auto_step = 160;
						}
					}
					else{
						logFailedNotNormal(lk2->getName(), loadlock2_process_name, loadlock2_auto_step);
					}
				}
				break;
				case 160:
				{
					if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						if (lk2->getExhaustVacuumValueReachesTheSetValue())
						{
							loadlock2_auto_step = 170;
						}
						else{
							Sleep(100);
						}

					}
					else{
						logFailedNotNormal(lk2->getName(), loadlock2_process_name, loadlock2_auto_step);
					}
				}
				break;
				case 170:
				{
					if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto cmd = lk2->createCloseDiaphragmValveCommand(LoadLockValveOpening::LoadLock_Fast);
						lk2->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(lk2->getName(), "关闭隔膜阀快充", loadlock2_process_name, loadlock2_auto_step);
						}
						else{
							loadlock2_auto_step = 180;
						}
					}
					else{
						logFailedNotNormal(lk2->getName(), loadlock2_process_name, loadlock2_auto_step);
					}
				}
				break;
				case 180:
				{
					if (lk2->getExhaustVacuumValueReachesTheSetValue() && (lk2->getVacuumPressureGageState() == 0))
					{
						loadlock2_auto_step = 300;
					}
					else{
						loadlock2_auto_step = 150;
					}
				}
				break;

				//开门放Cassette流程
				case 300:
				{
					if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{

						auto cmd = lk2->createOpenCassetteDoorCommand();
						lk2->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(lk2->getName(), "打开放晶圆盒门阀", loadlock2_process_name, loadlock2_auto_step);
						}
						else{
							loadlock2_auto_step = 310;
						}
					}
					else
					{
						logFailedNotNormal(lk2->getName(), loadlock2_process_name, loadlock2_auto_step);
					}
				}
				break;
				case 310:
				{
					if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						loadlock2_put_cassette_finished = false;
						if (loadlock2_process_finished)
						{

							loadlock2_auto_step = 320;
						}
						else{
							loadlock2_auto_step = 340;
						}

					}
					else
					{
						logFailedNotNormal(lk2->getName(), loadlock2_process_name, loadlock2_auto_step);
					}
				}
				break;
				//换Cassette
				case 320:
				{
					if (!lk2->hasBoxPresent())
					{
						loadlock2_process_finished = false;
						loadlock2_auto_step = 340;

					}
					else
					{
						Sleep(50);
					}
				}
				break;
				case 340:
				{
					if (lk2->hasBoxPresent())
					{
						ui->loadlock2_put_cassette_finished_pbt->setEnabled(true);
						loadlock2_auto_step = 350;
					}
					else
					{
						Sleep(50);
					}
				}
				break;
				case 350:
				{
					if (loadlock2_put_cassette_finished && lk2->hasBoxPresent())
					{
						ui->loadlock2_put_cassette_finished_pbt->setEnabled(false);
						loadlock2_auto_step = 400;
					}
					else
					{

						loadlock2_auto_step = 310;
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
						if (lk2->getVacuumEnable())
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
							loadlock2_auto_step = 520;
						}
						else
						{
							loadlock2_auto_step = 500;
						}
					}
					else{
						Sleep(100);
					}
				}
				break;
				case 520:
				{
					if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto cmd = lk2->createCloseAngleValveCommand();
						lk2->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(lk2->getName(), "关闭角阀", loadlock2_process_name, loadlock2_auto_step);
						}
						else{
							loadlock2_auto_step = 530;
						}

					}
					else{
						logFailedNotNormal(lk2->getName(), loadlock2_process_name, loadlock2_auto_step);
					}
				}
				break;
				case 530:
				{
					if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						loadlock2_auto_step = 800;

					}
					else{
						logFailedNotNormal(lk2->getName(), loadlock2_process_name, loadlock2_auto_step);
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
						loadlock2_auto_step = 900;
					}
					else
					{
						logFailedNotNormal(lk2->getName(), loadlock2_process_name, loadlock2_auto_step);
					}
				}
				break;
				case 900:
				{
					if (sequence_loadlock2_transfer_wafer.size() > 0)
					{
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
						loadlock2_get_vacuum = true;
						loadlock2_auto_step = 920;
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
						Sleep(100);
					}
				}
				break;
				case 950:
				{
					loadlock2_auto_step = 2000;
				}
				break;



				/* ---------------- 允许放晶圆流程 -------------------- */
				case 2000:
				{
					if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						loadlock2_move_slot_index = sequence_loadlock2_transfer_wafer[0];
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
						loadlock2_auto_step = 2060;
					}
					else
					{
						Sleep(20);
					}
				}
				break;
				case 2060:
				{
					if (sequence_loadlock2_transfer_wafer.size() > 0)
					{
						if (sequence_loadlock2_transfer_wafer.size() >= 2 &&
							sequence_loadlock2_transfer_wafer[0] != sequence_loadlock2_transfer_wafer[1])
						{
							loadlock2_auto_step = 2070;
						}

						sequence_loadlock2_transfer_wafer.erase(sequence_loadlock2_transfer_wafer.begin());
					}
					else
					{
						loadlock2_auto_step = 2070;
					}
				}
				break;
				case 2070:
				{
					if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
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
					else
					{
						logFailedNotNormal(lk2->getName(), loadlock2_process_name, loadlock2_auto_step);
					}
				}
				break;
				case 2080:
				{
					if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{

						if (sequence_loadlock2_transfer_wafer.size() > 0)
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
							loadlock2_auto_step = 5030;
						}
					}
					else
					{
						logFailedNotNormal(lk2->getName(), loadlock2_process_name, loadlock2_auto_step);
					}

				}
				break;

				case 5030:
				{
					if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						Sleep(500);
						auto cmd = lk2->createMoveToSlotCommand(28);
						lk2->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(lk2->getName(), "移动到指定槽号", loadlock2_process_name, loadlock2_auto_step);
						}
						else{
							loadlock2_auto_step = 5040;
						}
					}
					else
					{
						logFailedNotNormal(lk2->getName(), loadlock2_process_name, loadlock2_auto_step);
					}

				}
				break;
				case 5040:
				{
					if (lk2->getState() == IKernelSubSystem::State::SUB_NORMAL)
					{
						auto cmd = lk2->createCloseAngleValveCommand();
						lk2->startCommand(cmd);
						cmd->wait();
						if (cmd->hasError())
						{
							logFailedExcuteCommandHasError(lk2->getName(), "关闭角阀", loadlock2_process_name, loadlock2_auto_step);
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
					loadlock2_process_finished = true;
					loadlock2_auto_step = 6010;
				}
				break;
				case 6010:
				{
					if (loadlock1_process_finished && robot_process_finished)
					{
						onUpdateProcessEditEnabled(true);
					}
					loadlock2_auto_step = 100;
				}
				break;
				default:
					break;
				}
				loadlock2_step_once_finished = true;
				Sleep(10);
			}
			Sleep(100);
		}



	}

	void QSlotTransferAutoVTMWidgetPrivate::setRunning(const bool value){
		running = value;
		
	}

	void QSlotTransferAutoVTMWidgetPrivate::setLoadLock1PutCassetteFinished()
	{
		loadlock1_put_cassette_finished = true;
	}

	void QSlotTransferAutoVTMWidgetPrivate::setLoadLock2PutCassetteFinished(){
		loadlock2_put_cassette_finished = true;
	}

	void QSlotTransferAutoVTMWidgetPrivate::logFailed(const std::string station_name, const std::string log){
		Q_Q(QSlotTransferAutoVTMWidget);
		running = false;
		logError(station_name.c_str(), log.c_str());
		onUpdateControlEnabled("execute_pbt", true);
		onUpdateControlEnabled("reset_pbt", true);
	}

	void QSlotTransferAutoVTMWidgetPrivate::logFailedNotNormal(const std::string station_name, const std::string process_name, const int step){
		Q_Q(QSlotTransferAutoVTMWidget);
		logFailed(station_name, Poco::format("%s 未处于正常状态， %s：%d", station_name, process_name, step));
	}

	void QSlotTransferAutoVTMWidgetPrivate::logFailedExcuteCommandHasError(const std::string station_name, const std::string command_name, const std::string process_name, const int step){
		Q_Q(QSlotTransferAutoVTMWidget);
		logFailed(station_name, Poco::format("%s %s命令执行失败， %s：%d", station_name, command_name, process_name, step));
	}

	/**
	 * QSlotTransferAutoVTMWidget
	 */
	QSlotTransferAutoVTMWidget::QSlotTransferAutoVTMWidget(const std::shared_ptr<IKernel>& kernel, QWidget* parent)
		:QKernelModuleWidget(parent)
		, d_ptr(new QSlotTransferAutoVTMWidgetPrivate(this)){
		Q_D(QSlotTransferAutoVTMWidget);

		d->kernel = kernel;
		d->tower = kernel->getKernelModule<FortrendVTMSignalTower>();
		QHBoxLayout* main_layout = new QHBoxLayout(this);
		d->ui = new Ui::SlotTransferAutoVTMWidget;
		excel_helper = new ExcelAssistant();

		QWidget* opt_panel = new QWidget;
		d->ui->setupUi(opt_panel);
		main_layout->addWidget(opt_panel);
		d->ui->execute_pbt->setEnabled(false);
		d->ui->loadlock1_put_cassette_finished_pbt->setEnabled(false);
		d->ui->loadlock2_put_cassette_finished_pbt->setEnabled(false);

		d->ui->sequence_edit_tbw->setSelectionBehavior(QAbstractItemView::SelectRows);
		//d->ui->sequence_edit_tbw->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		//d->ui->sequence_edit_tbw->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
		//d->ui->sequence_edit_tbw->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
		//d->ui->sequence_edit_tbw->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
		//d->ui->sequence_edit_tbw->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
		//d->ui->sequence_edit_tbw->horizontalHeader()->setSectionsMovable(true);
		//d->ui->sequence_edit_tbw->horizontalScrollBar()->setValue(1);
		//d->ui->sequence_edit_tbw->setFr()->
		//d->ui->sequence_edit_tbw->horizontalHeader()->setStyleSheet("QHeaderView::section{background:#0000FF;}");
		//d->ui->sequence_edit_tbw->verticalHeader()->setStyleSheet("QHeaderView::section{background:#a7fffa;}");
		//d->ui->sequence_edit_tbw->setStyleSheet("QTableCornerButton::section{background:#0000FF;}");

		//connect
		connect(d->ui->add_an_item_pbt, &QPushButton::clicked, this, &QSlotTransferAutoVTMWidget::onAddAnItem);
		connect(d->ui->delete_the_selected_item_pbt, &QPushButton::clicked, this, &QSlotTransferAutoVTMWidget::onDeleteTheSelectedItem);
		connect(d->ui->clear_sequence_pbt, &QPushButton::clicked, this, &QSlotTransferAutoVTMWidget::onClearSequence);
		connect(d->ui->save_sequence_pbt, &QPushButton::clicked, this, &QSlotTransferAutoVTMWidget::onSaveSequence);
		connect(d->ui->load_sequence_pbt, &QPushButton::clicked, this, &QSlotTransferAutoVTMWidget::onLoadSequence);


		connect(d->ui->execute_pbt, &QPushButton::clicked, this, &QSlotTransferAutoVTMWidget::onStart);
		connect(d->ui->pause_pbt, &QPushButton::clicked, this, &QSlotTransferAutoVTMWidget::onPause);
		connect(d->ui->reset_pbt, &QPushButton::clicked, this, &QSlotTransferAutoVTMWidget::onReset);

		connect(d->ui->loadlock1_put_cassette_finished_pbt, &QPushButton::clicked, this, &QSlotTransferAutoVTMWidget::onLoadLock1PutCassetteFinished);
		connect(d->ui->loadlock2_put_cassette_finished_pbt, &QPushButton::clicked, this, &QSlotTransferAutoVTMWidget::onLoadLock2PutCassetteFinished);
		connect(d->ui->get_step_pbt, &QPushButton::clicked, this, &QSlotTransferAutoVTMWidget::onGetStep);

		connect(d->ui->save_target_site_pbt, &QPushButton::clicked, this, &QSlotTransferAutoVTMWidget::onSaveTargetSite);

		d->thread_enabled = true;
		/*d->thread_enabled = false;*/
		d->thd_show_message = std::thread([this] { this->threadShowAlarmMessage(); });
		d->thd_pm_cavity_process = std::thread([this] { this->threadGetPMCavityProcessData(); });
		d->thd_vacuum_auto = std::thread([this] { this->startVacuumAction(); });
		d->thd_robot_auto = std::thread([this] { this->startRobotAction(); });
		d->thd_loadlock1_auto = std::thread([this] { this->startLoadLock1Action(); });
		d->thd_loadlock2_auto = std::thread([this] { this->startLoadLock2Action(); });
		d->thd_vacuum_auto.detach();
		d->thd_robot_auto.detach();
		d->thd_loadlock1_auto.detach();
		d->thd_loadlock2_auto.detach();

		onLoadTargetSite();

		//d->ui->get_step_pbt->setVisible(false);
	}

	QSlotTransferAutoVTMWidget::~QSlotTransferAutoVTMWidget(){
		Q_D(QSlotTransferAutoVTMWidget);
		d->thread_enabled = false;
		if (d->thd_show_message.joinable())
		{
			d->thd_show_message.join();
		}
		if (d->thd_pm_cavity_process.joinable())
		{
			d->thd_pm_cavity_process.join();
		}
		if (d->thd_vacuum_auto.joinable())
		{
			d->thd_vacuum_auto.join();
		}
		if (d->thd_robot_auto.joinable())
		{
			d->thd_robot_auto.join();
		}
		if (d->thd_loadlock1_auto.joinable())
		{
			d->thd_loadlock1_auto.join();
		}
		if (d->thd_loadlock2_auto.joinable())
		{
			d->thd_loadlock2_auto.join();
		}
		delete d->ui;
		delete d;
	}

	void QSlotTransferAutoVTMWidget::onAddAnItem(){
		Q_D(QSlotTransferAutoVTMWidget);

		int row_count = d->ui->sequence_edit_tbw->rowCount();
		d->ui->sequence_edit_tbw->insertRow(row_count);

		QComboBox *loadlock_selected_cbx = new QComboBox();
		std::shared_ptr<FortrendLoadLockSubsystem> lk1 = d->kernel->getKernelModule<FortrendLoadLockSubsystem>("LLA");
		for (size_t i = 1; i <= lk1->getCassetteSlotCount(); i++)
		{
			std::string item = "槽号" + std::to_string(i);
			loadlock_selected_cbx->addItem(item.c_str());

		}

		d->ui->sequence_edit_tbw->setCellWidget(row_count, 0, loadlock_selected_cbx);

		QComboBox *pm_selected_cbx = new QComboBox();
		pm_selected_cbx->addItem("PM1");
		pm_selected_cbx->addItem("PM");
		pm_selected_cbx->addItem("PM3");
		d->ui->sequence_edit_tbw->setCellWidget(row_count, 1, pm_selected_cbx);

		addEditTableWidgetItemDoubleSpinBox(row_count, 2, 20.0, 650.0, 50, 1000);    //温度设定
		addEditTableWidgetItemDoubleSpinBox(row_count, 3, 0.0, 20.0, 1, 10);		 //粗抽压力
		addEditTableWidgetItemDoubleSpinBox(row_count, 4, 0.0001, 0.05, 0.005, 0.02, 4); //精抽压力
		addEditTableWidgetItemDoubleSpinBox(row_count, 5, 0.0001, 0.05, 0.005, 0.02, 4); //溅射压力
		addEditTableWidgetItemDoubleSpinBox(row_count, 6, 0.0, 200.0, 10, 50);		 //溅射流量1
		addEditTableWidgetItemDoubleSpinBox(row_count, 7, 0.0, 200.0, 10, 50);		 //溅射流量2
		addEditTableWidgetItemDoubleSpinBox(row_count, 8, 0.0, 200.0, 10, 50);		 //溅射流量3
		addEditTableWidgetItemDoubleSpinBox(row_count, 9, 0.0, 2000.0, 100, 1000);   //溅射功率1
		addEditTableWidgetItemDoubleSpinBox(row_count, 10, 1.0, 200.0, 10, 100);	 //溅射功率增速1
		addEditTableWidgetItemDoubleSpinBox(row_count, 11, 0.0, 2000.0, 100, 1000);  //溅射功率2
		addEditTableWidgetItemDoubleSpinBox(row_count, 12, 1.0, 200.0, 10, 100);	 //溅射功率增速2
		addEditTableWidgetItemDoubleSpinBox(row_count, 13, 0.0, 2000.0, 100, 1000);  //溅射功率3
		addEditTableWidgetItemDoubleSpinBox(row_count, 14, 1.0, 200.0, 10, 100);	 //溅射功率增速3
		addEditTableWidgetItemDoubleSpinBox(row_count, 15, 0.0, 5.0, 0.5, 3.0);		 //预溅射事件
		addEditTableWidgetItemDoubleSpinBox(row_count, 16, 0.0, 90.0, 10, 45.0);	 //工艺溅射旋转速度
		addEditTableWidgetItemDoubleSpinBox(row_count, 17, 0.0, 900.0, 100, 500);    //工艺溅射时间

		QComboBox *cathode_power_selection_1_cbx = new QComboBox();
		cathode_power_selection_1_cbx->addItem("无");
		cathode_power_selection_1_cbx->addItem("DC");
		cathode_power_selection_1_cbx->addItem("RF");
		d->ui->sequence_edit_tbw->setCellWidget(row_count, 18, cathode_power_selection_1_cbx);

		QComboBox *cathode_power_selection_2_cbx = new QComboBox();
		cathode_power_selection_2_cbx->addItem("无");
		cathode_power_selection_2_cbx->addItem("DC");
		cathode_power_selection_2_cbx->addItem("RF");
		d->ui->sequence_edit_tbw->setCellWidget(row_count, 19, cathode_power_selection_2_cbx);

		QComboBox *cathode_power_selection_3_cbx = new QComboBox();
		cathode_power_selection_3_cbx->addItem("无");
		cathode_power_selection_3_cbx->addItem("DC");
		cathode_power_selection_3_cbx->addItem("RF");
		d->ui->sequence_edit_tbw->setCellWidget(row_count, 20, cathode_power_selection_3_cbx);

	}

	void QSlotTransferAutoVTMWidget::onDeleteTheSelectedItem(){
		Q_D(QSlotTransferAutoVTMWidget);
		int row_count = d->ui->sequence_edit_tbw->rowCount();
		if (row_count > 0)
		{
			int selected_row = d->ui->sequence_edit_tbw->currentRow();
			d->ui->sequence_edit_tbw->removeRow(selected_row);
		}
	}

	void QSlotTransferAutoVTMWidget::onClearSequence(){
		Q_D(QSlotTransferAutoVTMWidget);
		int row_count = d->ui->sequence_edit_tbw->rowCount();
		if (row_count > 0)
		{
			d->ui->sequence_edit_tbw->setRowCount(0);
		}
	}

	void QSlotTransferAutoVTMWidget::onLoadSequence(){
		Q_D(QSlotTransferAutoVTMWidget);
		QString recipe_dir = QCoreApplication::applicationDirPath() + "/Recipes";
		QDir dir(recipe_dir);
		if (!dir.exists())
		{
			dir.mkpath(recipe_dir); // 使用mkpath创建文件夹，如果文件夹的父目录不存在也会一并创建
		}
		QString fileName = QFileDialog::getOpenFileName(this, "加载顺序", recipe_dir, "配置文件(*.xlsx)");
		if (fileName.isEmpty())
		{
			QMessageBox::warning(this, tr("警告信息"), tr("加载路径不能为空！"));
			return;
		}
		d->ui->load_sequence_pbt->setEnabled(false);
		QList<QList<QVariant>> load_data = excel_helper->openFile(fileName);
		logInform(d->module_name.c_str(), Poco::format("列数：%d", load_data[0].size()).c_str());
		if (load_data.size() > 1 && (load_data[0].size() == 21))
		{
			onClearSequence();
			//验证列名是否一致
			if (verificationLoadDataHeadName(load_data[0]))
			{

				for (int i = 1; i < load_data.size(); ++i) 
				{
					if (verificationLoadDataItem(load_data[i],i+1))
					{
						onAddAnItem();
						for (int j = 0; j < load_data[0].size(); ++j) {

							//Excel数据从1开始， QTableWidget从0开始
							QWidget *widget = d->ui->sequence_edit_tbw->cellWidget(i-1, j);
							if (j < 2 || j > 17)
							{
								QComboBox *combox = (QComboBox*)widget;
								combox->setCurrentText(load_data[i][j].toString());

							}
							else{
								QDoubleSpinBox *dsb = (QDoubleSpinBox*)widget;
								dsb->setValue(load_data[i][j].toDouble());
							}
						}
					}
					else
					{
						d->ui->load_sequence_pbt->setEnabled(true);
						return;
					}
				}
			}
			
		}
		else if (load_data.size() < 2)
		{
			QMessageBox::critical(this, tr("错误"), tr(QString("路径:%1配置文件行数错误").arg(fileName).toUtf8().constData()));
		}
		else 
		{
			QMessageBox::critical(this, tr("错误"), tr(QString("路径:%1配置文件列数不等于21").arg(fileName).toUtf8().constData()));
		}
		d->ui->load_sequence_pbt->setEnabled(true);
		
	}

	void QSlotTransferAutoVTMWidget::onSaveSequence(){
		Q_D(QSlotTransferAutoVTMWidget);
		
		if (d->ui->sequence_edit_tbw->rowCount()<1)
		{
			QMessageBox::warning(this, tr("警告信息"), tr("请先添加顺序！"));
			return;
		}
		QString recipe_dir = QCoreApplication::applicationDirPath() + "/Recipes";
		QDir dir(recipe_dir);
		if (!dir.exists()) 
		{
			dir.mkpath(recipe_dir); // 使用mkpath创建文件夹，如果文件夹的父目录不存在也会一并创建
		}
		QString fileName = QFileDialog::getSaveFileName(this, tr("保存顺序"), recipe_dir, tr("配置文件(*.xlsx)"));
		if (fileName.isEmpty())
		{
			QMessageBox::warning(this, tr("警告信息"), tr("保存路径不能为空！"));
			return;
		}
		d->ui->save_sequence_pbt->setEnabled(false);
		d->ui->sequence_edit_tbw->setEditTriggers(QAbstractItemView::NoEditTriggers);
		QList<QList<QVariant>> data_total;
		QList<QVariant> data_header;
		for (int i = 0; i< d->ui->sequence_edit_tbw->columnCount(); ++i)
		{

			QString columnName = d->ui->sequence_edit_tbw->horizontalHeaderItem(i)->text();
			data_header.append(QVariant::fromValue(columnName));
		}
		data_total.append(data_header);
		int rowCount = d->ui->sequence_edit_tbw->rowCount();
		int columnCount = d->ui->sequence_edit_tbw->columnCount();
		for (int i = 0; i < rowCount; ++i)
		{
			QList<QVariant> values;
			for (int j = 0; j < columnCount; ++j)
			{
				QString key = QString("row%1col%2").arg(i).arg(j);
				QWidget *widget = d->ui->sequence_edit_tbw->cellWidget(i, j);
				if (j < 2 || j > 17)
				{
					QComboBox *combox = (QComboBox*)widget;
					values.append(QVariant::fromValue(combox->currentText()));
				}
				else{
					QDoubleSpinBox *dsb = (QDoubleSpinBox*)widget;
					values.append(QVariant::fromValue(dsb->value()));
				}
			}
			data_total.append(values);
		}
		if (!excel_helper->saveFile(data_total, fileName)){
			QMessageBox::critical(this, tr("错误"), tr("保存失败！"));
		}
		d->ui->save_sequence_pbt->setEnabled(true);
		d->ui->sequence_edit_tbw->setEditTriggers(QAbstractItemView::AllEditTriggers);
	}

	//保存目标靶位
	void QSlotTransferAutoVTMWidget::onLoadTargetSite(){
		Q_D(QSlotTransferAutoVTMWidget);
		QString fileName = QDir::currentPath() + "/config/" + "TargetSite.ini";
		if (fileName.isEmpty())
		{
			logError("Auto", "加载目标靶位失败配置文件失败！");
			return;
		}

		QSettings settings(fileName, QSettings::IniFormat);

		/*d->ui->target_site_1_pm1_let->setText(settings.value("PM1TatgetSite1", "1").toString());
		d->ui->target_site_2_pm1_let->setText(settings.value("PM1TatgetSite2", "1").toString());
		d->ui->target_site_3_pm1_let->setText(settings.value("PM1TatgetSite3", "1").toString());*/

		d->ui->target_site_1_pm2_let->setText(settings.value("PM2TatgetSite1", "1").toString());
		d->ui->target_site_2_pm2_let->setText(settings.value("PM2TatgetSite2", "1").toString());
		d->ui->target_site_3_pm2_let->setText(settings.value("PM2TatgetSite3", "1").toString());

		/*d->ui->target_site_1_pm3_let->setText(settings.value("PM3TatgetSite1", "1").toString());
		d->ui->target_site_2_pm3_let->setText(settings.value("PM3TatgetSite2", "1").toString());
		d->ui->target_site_3_pm3_let->setText(settings.value("PM3TatgetSite3", "1").toString());*/
	}

	//保存目标靶位
	void QSlotTransferAutoVTMWidget::onSaveTargetSite(){
		Q_D(QSlotTransferAutoVTMWidget);
		QString fileName = QDir::currentPath() + "/config/" + "TargetSite.ini";
		if (fileName.isEmpty())
			return;
		QSettings settings(fileName, QSettings::IniFormat);

		/*settings.setValue("PM1TatgetSite1", d->ui->target_site_1_pm1_let->text());
		settings.setValue("PM1TatgetSite2", d->ui->target_site_2_pm1_let->text());
		settings.setValue("PM1TatgetSite3", d->ui->target_site_3_pm1_let->text());*/

		settings.setValue("PM2TatgetSite1", d->ui->target_site_1_pm2_let->text());
		settings.setValue("PM2TatgetSite2", d->ui->target_site_2_pm2_let->text());
		settings.setValue("PM2TatgetSite3", d->ui->target_site_3_pm2_let->text());

	/*	settings.setValue("PM3TatgetSite1", d->ui->target_site_1_pm3_let->text());
		settings.setValue("PM3TatgetSite2", d->ui->target_site_2_pm3_let->text());
		settings.setValue("PM3TatgetSite3", d->ui->target_site_3_pm3_let->text());*/

		logInform("Auto", "目标靶位保存成功.");
	}

	void QSlotTransferAutoVTMWidget::onStart(){
		Q_D(QSlotTransferAutoVTMWidget);

		if (d->sequence_loadlock1_transfer_wafer.size() == 0 &&
			d->sequence_loadlock1_transfer_wafer.size() == 0 &&
			d->sequence_robot_transfer_wafer.size() == 0)
		{
			if (d->setTransferSequence() == false)
			{
				logError("Auto", "流程配置错误.");
				return;
			}
		}
		d->process_edit_enabled = false;
		updateProcessEnabled();
		d->setRunning(true);
		d->ui->execute_pbt->setEnabled(false);
		d->ui->reset_pbt->setEnabled(false);
		d->tower->setOutput(FortrendVTMSignalTower::Output::YELLOW_LIGHT, false);
		d->tower->setOutput(FortrendVTMSignalTower::Output::GREEN_LIGHT, true);
	}

	void QSlotTransferAutoVTMWidget::onLoadLock1PutCassetteFinished(){
		Q_D(QSlotTransferAutoVTMWidget);

		d->setLoadLock1PutCassetteFinished();
	}

	void QSlotTransferAutoVTMWidget::onLoadLock2PutCassetteFinished(){
		Q_D(QSlotTransferAutoVTMWidget);
		d->setLoadLock2PutCassetteFinished();

	}

	void QSlotTransferAutoVTMWidget::startRobotAction(){
		Q_D(QSlotTransferAutoVTMWidget);
		d->startRobotAction();
	}

	void QSlotTransferAutoVTMWidget::startVacuumAction(){
		Q_D(QSlotTransferAutoVTMWidget);
		d->startVacuumAction();
	}

	void QSlotTransferAutoVTMWidget::startLoadLock1Action(){
		Q_D(QSlotTransferAutoVTMWidget);
		d->startLoadLock1Action();
	}

	void QSlotTransferAutoVTMWidget::startLoadLock2Action(){
		Q_D(QSlotTransferAutoVTMWidget);
		d->startLoadLock2Action();
	}

	void QSlotTransferAutoVTMWidget::onPause(){
		Q_D(QSlotTransferAutoVTMWidget);
		d->setRunning(false);
		d->ui->execute_pbt->setEnabled(true);
		d->ui->reset_pbt->setEnabled(true);
	}

	void QSlotTransferAutoVTMWidget::onReset(){
		Q_D(QSlotTransferAutoVTMWidget);
		d->setRunning(false);
		std::thread thread(&QSlotTransferAutoVTMWidget::resetAction, this);
		thread.detach();
	}

	void QSlotTransferAutoVTMWidget::resetAction(){
		Q_D(QSlotTransferAutoVTMWidget);
		d->resetAction();
	}

	void QSlotTransferAutoVTMWidget::updateAutoData(const QString& message)throw(KernelException){
		Q_D(QSlotTransferAutoVTMWidget);
		try{
			if (message == "PM1")
			{
				/*d->ui->coating_time_pm1_let->setText(QString::number(d->coating_time_pm1, 'f', 1));
				d->ui->processing_step_pm1_let->setText(QString::number(d->processing_step_pm1));*/
			}
			else if (message == "PM")
			{
				d->ui->coating_time_pm2_let->setText(QString::number(d->coating_time_pm2, 'f', 1));
				d->ui->processing_step_pm2_let->setText(QString::number(d->processing_step_pm2));
			}
			else if (message == "PM3")
			{
				/*d->ui->coating_time_pm3_let->setText(QString::number(d->coating_time_pm3, 'f', 1));
				d->ui->processing_step_pm3_let->setText(QString::number(d->processing_step_pm3));*/
			}
			else{

			}
		}
		catch (KernelException& e){
			logError(d->module_name.c_str(), e.what());
			//throw e;
		}

	}

	void QSlotTransferAutoVTMWidget::updateControlEnabled(const QString& name, const bool enabled) throw (KernelException){
		Q_D(QSlotTransferAutoVTMWidget);
		try{
			if (name == "execute_pbt")
			{
				d->ui->execute_pbt->setEnabled(enabled);
			}
			else if (name == "pause_pbt")
			{
				d->ui->pause_pbt->setEnabled(enabled);
			}
			else if (name == "reset_pbt")
			{
				d->ui->reset_pbt->setEnabled(enabled);
			}
			else if (name == "loadlock1_put_cassette_finished_pbt")
			{
				d->ui->loadlock1_put_cassette_finished_pbt->setEnabled(enabled);
			}
			else if (name == "loadlock2_put_cassette_finished_pbt")
			{
				d->ui->loadlock2_put_cassette_finished_pbt->setEnabled(enabled);
			}
			else{

			}
		}
		catch (KernelException& e){
			logError(d->module_name.c_str(), e.what());
			//throw e;
		}
	}

	void QSlotTransferAutoVTMWidget::updateProcessEnabled(){
		Q_D(QSlotTransferAutoVTMWidget);

		bool enabled = d->process_edit_enabled;
		if (enabled)
		{
			d->ui->sequence_edit_tbw->setEditTriggers(QAbstractItemView::AllEditTriggers);
			d->ui->sequence_edit_tbw->setSelectionMode(QAbstractItemView::SingleSelection);
		}
		else{
			d->ui->sequence_edit_tbw->setEditTriggers(QAbstractItemView::NoEditTriggers);
			d->ui->sequence_edit_tbw->setSelectionMode(QAbstractItemView::NoSelection); // 禁止选择单元格
		}
		//d->ui->sequence_edit_tbw->setEnabled(enabled);
		//d->ui->horizontalLayout->setEnabled(enabled);
		d->ui->add_an_item_pbt->setEnabled(enabled);
		d->ui->delete_the_selected_item_pbt->setEnabled(enabled);
		d->ui->clear_sequence_pbt->setEnabled(enabled);
		d->ui->save_sequence_pbt->setEnabled(enabled);
		d->ui->load_sequence_pbt->setEnabled(enabled);

	}

	void QSlotTransferAutoVTMWidget::onGetStep(){
		Q_D(QSlotTransferAutoVTMWidget);
		logInform("Auto", Poco::format("%s = %d", d->loadlock1_process_name, d->loadlock1_auto_step).c_str());
		logInform("Auto", Poco::format("%s = %d", d->loadlock2_process_name, d->loadlock2_auto_step).c_str());
		logInform("Auto", Poco::format("%s = %d", d->robot_process_name, d->robot_auto_step).c_str());
		logInform("Auto", Poco::format("%s = %d", d->vacuum_process_name, d->vacuum_auto_step).c_str());
		test_flag = !test_flag;
		if (test_flag)
		{
			d->ui->sequence_edit_tbw->setEditTriggers(QAbstractItemView::AllEditTriggers);
			d->ui->sequence_edit_tbw->setSelectionMode(QAbstractItemView::SingleSelection);
			
		}
		else{
			d->ui->sequence_edit_tbw->setSelectionMode(QAbstractItemView::NoSelection); // 禁止选择单元格
			d->ui->sequence_edit_tbw->setEditTriggers(QAbstractItemView::NoEditTriggers);
			
		}
	}



	void QSlotTransferAutoVTMWidget::showMessage(const QString& message) throw(KernelException){
		Q_D(QSlotTransferAutoVTMWidget);
		try{
			QMessageBox::information(this, "警告", message);
		}
		catch (KernelException& e){
			logError(d->module_name.c_str(), e.what());
			//throw e;
		}

	}

	void QSlotTransferAutoVTMWidget::threadShowAlarmMessage()
	{
		Q_D(QSlotTransferAutoVTMWidget);
		d->moniterPMCavityAlarmAction();
	}

	void QSlotTransferAutoVTMWidget::threadGetPMCavityProcessData()
	{
		Q_D(QSlotTransferAutoVTMWidget);
		d->moniterPMCavityProcessDataAction();
	}

	void QSlotTransferAutoVTMWidget::addEditTableWidgetItemDoubleSpinBox(int row, int column, double min_value, double max_value, double single_step, double value, int decimals_value){
		Q_D(QSlotTransferAutoVTMWidget);
		QDoubleSpinBox *dsb = new QDoubleSpinBox();
		dsb->setMaximum(max_value);
		dsb->setMinimum(min_value);
		dsb->setDecimals(decimals_value);
		dsb->setSingleStep(single_step);
		dsb->setValue(value);
		d->ui->sequence_edit_tbw->setCellWidget(row, column, dsb);
	}

	bool QSlotTransferAutoVTMWidget::verificationLoadDataHeadName(const QList<QVariant> &head_names){
		Q_D(QSlotTransferAutoVTMWidget);
		if (d->ui->sequence_edit_tbw->columnCount() != head_names.count())
		{
			QMessageBox::critical(this, tr("错误"), tr("配置文件中的列数量与软件规定数量不匹配"));
			return false;
		}
		for (int i = 0; i < d->ui->sequence_edit_tbw->columnCount(); i++)
		{
			QString columnName = d->ui->sequence_edit_tbw->horizontalHeaderItem(i)->text();
			if (columnName != head_names[i].toString())
			{
				QString message = QString("第%1列名配置文件中的名称【%2】与软件规定名称不匹配【%2】").arg(QString::number(i + 1).arg(head_names[i].toString()).arg(columnName));
				QMessageBox::critical(this, tr("错误"), tr(message.toUtf8().constData()));
				return false;
			}
		}
		return true;
	}

	bool QSlotTransferAutoVTMWidget::verificationLoadDataItem(const QList<QVariant> &items,const int row_index){
		Q_D(QSlotTransferAutoVTMWidget);
		if (d->ui->sequence_edit_tbw->columnCount() != items.count())
		{
			QMessageBox::critical(this, tr("错误"), tr("配置文件中的列数量与软件规定数量不匹配"));
			return false;
		}
		QRegExp reg("槽");
		int pos = 0;
		bool reg_result = false;
		//槽号1~13
		QString slot_name = items[0].toString();
		if (slot_name.size() == 4)
		{
			reg.setPattern("槽号1[0-3]");
			if ((pos = reg.indexIn(slot_name, pos)) != -1)
			{
				reg_result = true;
			}
			
		}
		else if (slot_name.size() == 3)
		{
			reg.setPattern("槽号[1-9]");
			if ((pos = reg.indexIn(slot_name, pos)) != -1)
			{
				reg_result = true;
			}
		}
		else
		{

		}
		if (!reg_result)
		{
			QString message = QString("第%1行，配置文件中的槽号【%2】与软件规定名称不匹配【槽1~13】").arg(row_index).arg(slot_name);
			QMessageBox::critical(this, tr("错误"), tr(message.toUtf8().constData()));
			return false;
		}
		//PM腔
		QString pm_name = items[1].toString();
		reg.setPattern("PM[1-3]");
		if ((pos = reg.indexIn(pm_name, pos)) == -1 || pm_name.size() != 3)
		{
			QString message = QString("第%1行，配置文件中的PM腔【%2】与软件规定名称不匹配【PM1~3】").arg(row_index).arg(pm_name);
			QMessageBox::critical(this, tr("错误"), tr(message.toUtf8().constData()));
			return false;

		}
		//温度
		if (!verificationDataInRange(items[2], 20 ,650))
		{
			QString message = QString("第%1行，配置文件中的温度(℃)【%2】与软件规定范围不匹配【20~650】").arg(row_index).arg(items[2].toString());
			QMessageBox::critical(this, tr("错误"), tr(message.toUtf8().constData()));
			return false;
		}
		//粗抽压力
		if (!verificationDataInRange(items[3], 0.00, 20.0))
		{
			QString message = QString("第%1行，配置文件中的粗抽压力(Pa)【%2】与软件规定范围不匹配【0.00~20.0】").arg(row_index).arg(items[3].toString());
			QMessageBox::critical(this, tr("错误"), tr(message.toUtf8().constData()));
			return false;
		}
		//精抽压力
		if (!verificationDataInRange(items[4], 0.0001, 0.05))
		{
			QString message = QString("第%1行，配置文件中的精抽压力(Pa)【%2】与软件规定范围不匹配【0.0001~0.05】").arg(row_index).arg(items[4].toString());
			QMessageBox::critical(this, tr("错误"), tr(message.toUtf8().constData()));
			return false;
		}
		//贱射压力
		if (!verificationDataInRange(items[5], 0.0001, 0.05))
		{
			QString message = QString("第%1行，配置文件中的溅射压力(Pa)【%2】与软件规定范围不匹配【0.0001~0.05】").arg(row_index).arg(items[5].toString());
			QMessageBox::critical(this, tr("错误"), tr(message.toUtf8().constData()));
			return false;
		}
		//溅射流量1
		if (!verificationDataInRange(items[6], 0.0, 200.0))
		{
			QString message = QString("第%1行，配置文件中的溅射流量1(sccm)【%2】与软件规定范围不匹配【0.0~200.0】").arg(row_index).arg(items[6].toString());
			QMessageBox::critical(this, tr("错误"), tr(message.toUtf8().constData()));
			return false;
		}
		//溅射流量2
		if (!verificationDataInRange(items[7], 0.0, 200.0))
		{
			QString message = QString("第%1行，配置文件中的溅射流量2(sccm)【%2】与软件规定范围不匹配【0.0~200.0】").arg(row_index).arg(items[7].toString());
			QMessageBox::critical(this, tr("错误"), tr(message.toUtf8().constData()));
			return false;
		}
		//溅射流量3
		if (!verificationDataInRange(items[8], 0.0, 200.0))
		{
			QString message = QString("第%1行，配置文件中的溅射流量3(sccm)【%2】与软件规定范围不匹配【0.0~200.0】").arg(row_index).arg(items[8].toString());
			QMessageBox::critical(this, tr("错误"), tr(message.toUtf8().constData()));
			return false;
		}
		//溅射功率1
		if (!verificationDataInRange(items[9], 0.0, 2000.0))
		{
			QString message = QString("第%1行，配置文件中的溅射功率1(W)【%2】与软件规定范围不匹配【0.0~2000.0】").arg(row_index).arg(items[9].toString());
			QMessageBox::critical(this, tr("错误"), tr(message.toUtf8().constData()));
			return false;
		}
		//溅射功率增速1
		if (!verificationDataInRange(items[10], 0.0, 200.0))
		{
			QString message = QString("第%1行，配置文件中的溅射功率增速1(W/S)【%2】与软件规定范围不匹配【0.0~200.0】").arg(row_index).arg(items[10].toString());
			QMessageBox::critical(this, tr("错误"), tr(message.toUtf8().constData()));
			return false;
		}
		//溅射功率2
		if (!verificationDataInRange(items[11], 0.0, 2000.0))
		{
			QString message = QString("第%1行，配置文件中的溅射功率2(W)【%2】与软件规定范围不匹配【0.0~2000.0】").arg(row_index).arg(items[11].toString());
			QMessageBox::critical(this, tr("错误"), tr(message.toUtf8().constData()));
			return false;
		}
		//溅射功率增速2
		if (!verificationDataInRange(items[12], 0.0, 200.0))
		{
			QString message = QString("第%1行，配置文件中的溅射功率增速2(W/S)【%2】与软件规定范围不匹配【0.0~200.0】").arg(row_index).arg(items[12].toString());
			QMessageBox::critical(this, tr("错误"), tr(message.toUtf8().constData()));
			return false;
		}
		//溅射功率3
		if (!verificationDataInRange(items[13], 0.0, 2000.0))
		{
			QString message = QString("第%1行，配置文件中的溅射功率3(W)【%2】与软件规定范围不匹配【0.0~2000.0】").arg(row_index).arg(items[13].toString());
			QMessageBox::critical(this, tr("错误"), tr(message.toUtf8().constData()));
			return false;
		}
		//溅射功率增速3
		if (!verificationDataInRange(items[14], 0.0, 200.0))
		{
			QString message = QString("第%1行，配置文件中的溅射功率增速3(W/S)【%2】与软件规定范围不匹配【0.0~200.0】").arg(row_index).arg(items[14].toString());
			QMessageBox::critical(this, tr("错误"), tr(message.toUtf8().constData()));
			return false;
		}
		//预溅射时间
		if (!verificationDataInRange(items[15], 0.0, 5.0))
		{
			QString message = QString("第%1行，配置文件中的预溅射时间(min)【%2】与软件规定范围不匹配【0.0~5.0】").arg(row_index).arg(items[15].toString());
			QMessageBox::critical(this, tr("错误"), tr(message.toUtf8().constData()));
			return false;
		}
		//工艺溅射旋转速度
		if (!verificationDataInRange(items[16], 0.0, 90.0))
		{
			QString message = QString("第%1行，配置文件中的工艺溅射旋转速度(°/S)【%2】与软件规定范围不匹配【0.0~90.0】").arg(row_index).arg(items[16].toString());
			QMessageBox::critical(this, tr("错误"), tr(message.toUtf8().constData()));
			return false;
		}
		//工艺溅射时间
		if (!verificationDataInRange(items[17], 0.0, 900.0))
		{
			QString message = QString("第%1行，配置文件中的工艺溅射时间(min)【%2】与软件规定范围不匹配【0.0~900.0】").arg(row_index).arg(items[17].toString());
			QMessageBox::critical(this, tr("错误"), tr(message.toUtf8().constData()));
			return false;
		}
		QString cathode_power_1 = items[18].toString();
		if (cathode_power_1 != "无" && cathode_power_1 != "DC" && cathode_power_1 != "RF")
		{
			QString message = QString("第%1行，配置文件中的1#阴极电源选择【%2】与软件规定范围不匹配【无，DC，RF】").arg(row_index).arg(items[18].toString());
			QMessageBox::critical(this, tr("错误"), tr(message.toUtf8().constData()));
			return false;
		}
		QString cathode_power_2 = items[19].toString();
		if (cathode_power_2 != "无" && cathode_power_2 != "DC" && cathode_power_2 != "RF")
		{
			QString message = QString("第%1行，配置文件中的2#阴极电源选择【%2】与软件规定范围不匹配【无，DC，RF】").arg(row_index).arg(items[19].toString());
			QMessageBox::critical(this, tr("错误"), tr(message.toUtf8().constData()));
			return false;
		}
		QString cathode_power_3 = items[20].toString();
		if (cathode_power_3 != "无" && cathode_power_3 != "DC" && cathode_power_3 != "RF")
		{
			QString message = QString("第%1行，配置文件中的1#阴极电源选择【%2】与软件规定范围不匹配【无，DC，RF】").arg(row_index).arg(items[20].toString());
			QMessageBox::critical(this, tr("错误"), tr(message.toUtf8().constData()));
			return false;
		}
		return true;
	}

	bool QSlotTransferAutoVTMWidget::verificationDataInRange(const QVariant &item, const double min, const double max){
		double value = item.toDouble();
		if (value >= min && value<= max)
		{
			return true;
		}
		return false;
	}
}
