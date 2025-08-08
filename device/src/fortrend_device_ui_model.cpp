/**
* @file            fortrend_device_ui_model.h
* @brief           device for fortrend for gui
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Main

#include "fortrend_device_ui_model.h"
#include "fortrend_device_kernel.h"
#include "driver_output_handler_file.h"
#include "Common\apphelper.hpp"

//kernel 
#include "Kernel/kernel_log.h"
#include "Kernel/kernel_default_block_manager.h"
#include "Kernel/kernel_action_subsystem.h"
#include "kernel/kernel_script_subsystem.h"
#include "Kernel/kernel_sim_iocontrol.h"
#include "Kernel/kernel_api.h"



//fortrend 
#include "Kernel/Fortrend/fortrend_cassette_manager.h"
#include "Kernel/Fortrend/fortrend_signal_tower.h"
#include "Kernel/FortrendUI/signal_tower_widget.h"
#include "Kernel/FortrendUI/fortrend_cassette_manager_widget.h" 
#include "Kernel/FortrendUI/aligner_status_widget.h" 

//VTM model
#include "LoadLock/fortrend_loadlock_subsystem.h"
#include "LoadLock/fortrend_loadlock_subsystem_widget.h"
#include "SunwayRobot/fortrend_sunwayrobot_subsystem.h"
#include "SunwayRobot/fortrend_sunwayrobot_subsystem_widget.h"
#include "Aligner/fortrend_aligner_subsystem.h"
#include "Aligner/fortrend_aligner_subsystem_widget.h"
#include "CoolingCavity/fortrend_cooling_cavity_subsystem.h"
#include "CoolingCavity/fortrend_cooling_cavity_subsystem_widget.h"
#include "TMCavity/fortrend_tm_cavity_subsystem.h"
#include "TMCavity/fortrend_tm_cavity_subsystem_widget.h"
#include "PMCavity/fortrend_pm_cavity_subsystem.h"
#include "PMCavity/fortrend_pm_cavity_subsystem_widget.h"
#include "Pump/fortrend_pump_subsystem.h"
#include "Pump/fortrend_pump_subsystem_widget.h"
#include  "VTMSignalTower/fortrend_vtm_signal_tower.h"

#include "vtm_system_control_hlayout_widget.h"
//core ui
//#include "kernel/CoreUI/kernel_system_control_hlayout_widget.h"
//#include "kernel/CoreUI/kernel_system_control_widget.h"
#include "kernel/CoreUI/kernel_api_control_widget.h"
#include "Kernel/CoreUI/kernel_api_status_widget.h"
#include "Kernel/CoreUI/kernel_io_control_widget.h"
#include "Kernel/CoreUI/kernel_blockmanager_widget.h"
#include "Kernel/CoreUI/kernel_action_subsystem_widget.h"
#include "kernel/CoreUI/kernel_script_subsystem_widget.h"
#include "Kernel/FortrendUI/slot_transfer_cycle_test_widget.h" 
#include "kernel/CoreUI/kernel_alarm_module_widget.h"

//myunit ui
#include  "Vacuumize/fortrend_vacuumize_subsystem_widget.h"
#include  "BreakVacuum/fortrend_breakvacuum_subsystem_widget.h"

#include "Data/fortrend_data_subsystem_widget.h"
#include "Data/fortrend_datahistory_subsystem_widget.h"

//VTM special
#include  "control_mode_vtm_widget.h"
#include  "fortrend_station_status_vtm_widget.h"
#include  "slot_transfer_cycle_vtm_widget.h"
#include  "slot_transfer_auto_vtm_widget.h"
#include  "slot_transfer_cycle_vtm_new_widget.h"
#include  "VTMSignalTower/vtm_signal_tower_widget.h"

#include "kernel/Fortrend/loadport_abstract_subsystem.h"
#include  "Kernel/FortrendUI/loadport_abstract_subsystem_widget.h" 
//EFEM
#include "EFEM/efem_wafer_robot_subsystem.h"
#include "EFEM/efem_robot_wafer_abstract_subsystem_widget.h"
#include "EFEM/efem_loadport_subsystem.h" 
#include  "EFEM/efem_loadport_abstract_subsystem_widget.h" 
#include "EFEM/efem_aligner_subsystem.h"
#include  "EFEM/efem_aligner_abstract_subsystem_widget.h" 

#include  "STATIONMODE/fortrend_StationMode_subsystem_widget.h"
//SMIF
//#include "Kernel/Fortrend/fortrend_SMIF_subsystem.h" 
//#include  "Kernel/FortrendUI/fortrend_SMIF_subsystem_widget.h"

//gui framework
#include "Core/modulemanager.hpp"
#include "CorePlugin/messagemanager.hpp"
#include "XuiPlugin/isubworkpanel.hpp"
#include "XuiPlugin/iheadpanel.hpp"
#include "Core/modulebase.hpp"

#include "GuiPlugin/iversionfactory.hpp"
#include "source_version.h"

#include <QTabWidget>
#include <iostream>
#include <QMessageBox>
#if _MSC_VER >1600
#pragma execution_character_set("utf-8")
#endif

//手动版本号管理
#define VERSION_NUMBER  "1.0.5.0805_alpha"
#define GIT_COMMIT  "18bed20358fd06578af5e6fb405c9af09daa7e7b"
#define GIT_BRANCH  "master"

using namespace FC;

class VersionFactory : public IVersionFactory{
public:
	virtual int priority() const { return 0; };
	virtual QString displayName() const { return "VTM"; }
	virtual QString versionText() const {
		return tr(
			"<h3>VTM v%1</h3>"
			"Author: Fortrend<br/>"
			"Built on %2 at %3<br />"
			"Commit time:%4<br/>"
			"Commit number:%5<br/>"
			"Source branch:%6<br/>"
			"<br/>"
			"Copyright fortrend. All rights reserved.<br/>"
			"<br/>")
			.arg(VERSION_NUMBER,
			QLatin1String(__DATE__),
			QLatin1String(__TIME__),
			COMPILE_DATATIME,
			GIT_COMMIT,
			GIT_BRANCH);

	}
};

class SubSystemWorkPanel : public ISubWorkPanel{
public:
	SubSystemWorkPanel(QWidget* panel, const QString& name, const QString& parantName = "Maintenance", int priority = 0, UserLevel level = FC::USER_ERR, QString path = ":/image/main_main.png")
	{
		this->w = panel;
		this->name = name;
		this->parantName = parantName;
		this->pri = priority;
		this->level = level;
		this->iconpath = path;
	}
	virtual QString displayName() const { return name; }
	virtual int priority() const{ return pri; }
	virtual QWidget* workWidget(QWidget* parent = 0){
		return w;
	}
	virtual QString iconPath() const { return iconpath; }
	virtual QString parentPane() const { return parantName; }
	virtual UserLevel access() const { return level; }
	
private:
	QWidget* w;
	QString name;
	QString parantName;
	UserLevel level;
	int pri;
	QString iconpath;
};


class HeadPanel : public IHeadPanel{
public:
	HeadPanel(QWidget* panel, int priority,Position pos = Position::HEAD_COMMON)
		:panel(panel)
		, pri(priority)
		, pos(pos){

	}
	virtual Position position() const { return pos; }
	virtual int priority() const{ return this->pri; }
	virtual QWidget* workWidget(QWidget* parent = 0){
		return panel;
	}
private:
	QWidget* panel;
	int pri;
	Position pos;
};



////driver log
//class   DriverOutputHandlerFile : public KernelLog::OutputHandler
//{
//public:
//
//	/** \brief The name of the file in which to save the message data */
//	DriverOutputHandlerFile(){
//		stdOut.reset(new KernelLog::OutputHandlerSTD);
//	}
//
//
//	virtual void log(const std::string &category, const std::string &text, KernelLog::VisibleLevel visible, KernelLog::LogLevel level, const char *filename, int line){
//		stdOut->log(category, text, visible,level, filename, line);
//		//save to framework
//		if (level > KernelLog::LogLevel::LOG_DEBUG){ //just push >DEBUG message
//			QString cat = QString::fromStdString(category);
//			MessageManager::instance()->push(cat + ":" + QString::fromStdString(text), "kernel_" + cat, (MsgType)level, true, false, visible==KernelLog::LOG_VISIBLE_LEVEL1);
//			//MessageManager::instance()->push(cat + ":" + QString::fromLocal8Bit(text.c_str()), "kernel_" + cat, (MsgType)level, true, false, visible == KernelLog::LOG_VISIBLE_LEVEL1);
//
//		}
//	}
//private:
//	std::shared_ptr<KernelLog::OutputHandlerSTD> stdOut;
//};


FortrendDeviceModel::FortrendDeviceModel(){
	kernel.reset(new FortrendDeviceKernel);
}


FortrendDeviceModel::~FortrendDeviceModel(){
	kernel->unInitialize();
}

void FortrendDeviceModel::addHeadCompoments(){
	//status
	//QWidget* status_widget = new QKernelApiStatusWidget(kernel->getKernelModule<KernelApi>());
	//addAutoReleasedObject(new HeadPanel(status_widget, 0));

	//control button
	//QWidget* system_button_widget = new QKernelSystemControlWidget(kernel);
	//system_button_widget->setMaximumWidth(100);
	//system_button_widget->setObjectName("SystemButtonWidget"); 
	//system_button_widget->setMaximumWidth(400);
	//addAutoReleasedObject(new HeadPanel(system_button_widget, 2, IHeadPanel::HEAD_CENTER));

	//control button
	/*QWidget* system_button_widget = new QKernelSystemControlHLayoutWidget(kernel);*/
	

	//tower
	QFortrendVTMSignalTowerlWidget* tower_widget = new QFortrendVTMSignalTowerlWidget(kernel->getKernelModule<FortrendVTMSignalTower>());
	addAutoReleasedObject(new HeadPanel(tower_widget, 10));
}


void FortrendDeviceModel::addMainCompoments(){
	
	status_widget = new QFortrendStationStatusVTMWidget(kernel);

    addAutoReleasedObject(new SubSystemWorkPanel(status_widget, "主界面", "主界面", 0, UserLevel::USER_MANAGER, ":/Imageblack/main_homepage.png"));

	tm_widget = new QVacuumizeSubsystemWidget(kernel);
	addAutoReleasedObject(new SubSystemWorkPanel(tm_widget, "抽真空", "主界面", 0, UserLevel::USER_MANAGER, ":/Imageblack/main_Breakthevacuum.png"));

	tm_widget2 = new QBreakVacuumSubsystemWidget(kernel);
	//tm_widget2->widthMM();
	addAutoReleasedObject(new SubSystemWorkPanel(tm_widget2, "破真空", "主界面", 0, UserLevel::USER_MANAGER, ":/Imageblack/main_vacuum.png"));
	
	slot_transfer_cycle_vtm_widget = new QSlotTransferCycleVTMWidget(kernel, status_widget);
	addAutoReleasedObject(new SubSystemWorkPanel(slot_transfer_cycle_vtm_widget, "循环", "配方", 0, UserLevel::USER_ADMIN, ":/Imageblack/main_while.png"));

	QWidget* system_button_widget = new QVTMSystemControlHLayoutWidget(kernel, slot_transfer_cycle_vtm_widget,this);
	system_button_widget->setMaximumWidth(450);
	addAutoReleasedObject(new HeadPanel(system_button_widget, 2, IHeadPanel::HEAD_CENTER));
	/*DataWidget* data_widget = new DataWidget();

	addAutoReleasedObject(new SubSystemWorkPanel(data_widget, "实时数据", "历史曲线", 0, UserLevel::USER_MANAGER, ":/Imageblack/main_historicalcurve.png"));

	DataHistoryWidget* datahistoey_widget = new DataHistoryWidget();

	addAutoReleasedObject(new SubSystemWorkPanel(datahistoey_widget, "历史数据", "历史曲线", 0, UserLevel::USER_MANAGER, ":/Imageblack/main_historicalcurve.png"));*/
}

void FortrendDeviceModel::addManualCompoments(){
	//create module ui
	module_tabWidget = new QTabWidget;
	module_tabWidget->setStyleSheet("QTabWidget::pane { border: 0; }");
	
	//load lock
	for (auto& sub : kernel->getKernelModules<FortrendLoadLockSubsystem>()){
		QWidget* w = new QLoadLockSubsystemWidget(sub);
		module_tabWidget->addTab(w, QString::fromStdString(sub->getName()));
	}

	//wtr
	for (auto& sub : kernel->getKernelModules<FortrendSunwayRobotSubsystem>()){
		//QWidget* w = new QSunwayRobotSubsystemWidget(kernel,sub);
		QWidget* w = new QSunwayRobotSubsystemWidget(sub);
		module_tabWidget->addTab(w, QString::fromStdString(sub->getName()));
	}

	//aligner
	//for (auto& sub : kernel->getKernelModules<FortrendAlignerSubsystem>()){
	//	QWidget* w = new QAlignerSubsystemWidget(sub);
	//	module_tabWidget->addTab(w, QString::fromStdString(sub->getName()));
	//}

	//tm
	for (auto& sub : kernel->getKernelModules<FortrendTMCavitySubsystem>())
	{
		QWidget* w = new QTMCavitySubsystemWidget(sub);
		module_tabWidget->addTab(w, QString::fromStdString(sub->getName()));
	}

	//pm
	for (auto& sub : kernel->getKernelModules<FortrendPMCavitySubsystem>()){
		QWidget* w = new QPMCavitySubsystemWidget(sub);
		module_tabWidget->addTab(w, QString::fromStdString(sub->getName()));
	}

	//mode 
	for (auto& sub : kernel->getKernelModules<FortrendSTATIONMODESubsystem>()) {
		QWidget* w = new QSTATIONMODESubsystemWidget(sub);
		module_tabWidget->addTab(w, QString::fromStdString(sub->getName()));
	}

	//cooling
	//for (auto& sub : kernel->getKernelModules<FortrendCoolingCavitySubsystem>()){
	//	QWidget* w = new QCoolingCavitySubsystemWidget(sub);
	//	module_tabWidget->addTab(w, QString::fromStdString(sub->getName()));
	//}

	//pump
	for (auto& sub : kernel->getKernelModules<FortrendPumpSubsystem>()){
		QWidget* w = new QPumpSubsystemWidget(sub);
		module_tabWidget->addTab(w, QString::fromStdString(sub->getName()));
	}

	//create io board ui
	//QTabWidget* io_tabWidget = new QTabWidget;
	//io_tabWidget->setStyleSheet("QTabWidget::pane { border: 0; }");
	//for (auto& io : kernel->getKernelModules<IKernelIOControl>()){
	//	QWidget* w = new QKernelIOControlWidget(io, 5);
	//	io_tabWidget->addTab(w, QString::fromStdString(io->getName()));
	//}

	//control widget

	efemmodule_tabWidget = new QTabWidget;
	efemmodule_tabWidget->setStyleSheet("QTabWidget::pane { border: 0; }");

	//EFEM LP
	for (auto& sub : kernel->getKernelModules<EFEMLPSubsystem>()){
		QWidget* w = new QEFEMLoadPortAbstractSubsystemWidget(sub);
		efemmodule_tabWidget->addTab(w, QString::fromStdString(sub->getName()));
	}

	//EFEM robot
	for (auto& sub : kernel->getKernelModules<EFEMWaferRobotSubsystem>()){
		QWidget* w = new QEFEMRobotWaferAbstractSubsystemWidget(sub);
		efemmodule_tabWidget->addTab(w, QString::fromStdString(sub->getName()));
	}

	//EFEM aligner
	for (auto& sub : kernel->getKernelModules<EFEMAlignerSubsystem>())
	{
		QWidget* w = new QEFEMAlignerAbstractSubsystemWidget(sub);
		efemmodule_tabWidget->addTab(w, QString::fromStdString(sub->getName()));
	}

	//QTabWidget* smifmodule_tabWidget = new QTabWidget;
	//smifmodule_tabWidget->setStyleSheet("QTabWidget::pane { border: 0; }");

	//SMIF
	/*for (auto& sub : kernel->getKernelModules<FortrendSMIFSubsystem>()){
		QWidget* w = new QSMIFSubsystemWidget(sub);
		smifmodule_tabWidget->addTab(w, QString::fromStdString(sub->getName()));
	}*/

	addAutoReleasedObject(new SubSystemWorkPanel(efemmodule_tabWidget, "EFEM", "主界面", 0, UserLevel::USER_MANAGER, ":/Imageblack/main_manual.png"));
	//addAutoReleasedObject(new SubSystemWorkPanel(smifmodule_tabWidget, "SMIF", "主界面", 0, UserLevel::USER_MANAGER, ":/Imageblack/main_manual.png"));

	control_widget = new QControlModeVTMWidget(kernel);
	addAutoReleasedObject(new SubSystemWorkPanel(control_widget, "设置", "设置", 0, UserLevel::USER_MANAGER, ":/Imageblack/main_controlsettin.png"));
	
	addAutoReleasedObject(new SubSystemWorkPanel(module_tabWidget, "手动", "主界面", 0, UserLevel::USER_MANAGER, ":/Imageblack/main_manual.png"));

}

void FortrendDeviceModel::addAutoCompoments(){
	//script for job
	/*QTabWidget* script_tabWidget = new QTabWidget;
	script_tabWidget->setStyleSheet("QTabWidget::pane { border: 0; }");
	for (auto& sub : kernel->getKernelModules<KernelScriptSubsystem>()){
		QWidget* w = new QKernelScriptSubsystemWidget(sub);
		script_tabWidget->addTab(w, QString::fromStdString(sub->getName()));
	}*/

	//slot transfer cycle widget
	

	//QSlotTransferCycleVTMNewWidget* slot_transfer_cycle_vtm_widget2 = new QSlotTransferCycleVTMNewWidget(kernel);

	//slot transfer auto widget
	//QSlotTransferAutoVTMWidget* slot_transfer_auto_vtm_widget = new QSlotTransferAutoVTMWidget(kernel);

	//QSlotTransferCycleVTMWidget* slot_transfer_cycle_vtm_widget = new QSlotTransferCycleVTMWidget(kernel);
	//addAutoReleasedObject(new SubSystemWorkPanel(slot_transfer_cycle_vtm_widget, "循环", "配方", 0, UserLevel::USER_ADMIN,":/Imageblack/main_while.png"));
	//addAutoReleasedObject(new SubSystemWorkPanel(slot_transfer_auto_vtm_widget, "传输", "配方", 0, UserLevel::USER_ADMIN, ":/Imageblack/main_transmit.png"));
	//addAutoReleasedObject(new SubSystemWorkPanel(slot_transfer_cycle_vtm_widget2, "cycle", "配方", 0, UserLevel::USER_ADMIN, ":/Imageblack/main_while.png"));
}


void FortrendDeviceModel::addAlarmCompoments(){
	//slot transfer  cycle widget
	alarm = new QKernelAlarmWidget(kernel);

	addAutoReleasedObject(new SubSystemWorkPanel(alarm, "报警", "报警信息",0, UserLevel::USER_ADMIN, ":/Imageblack/main_alert.png"));
}



bool FortrendDeviceModel::initialize(const QStringList &arguments,QString *errorString){
	try{
		KernelLog::useOutputHandler(new DriverOutputHandlerFile);
		KernelLog::setLogLevel(KernelLog::LOG_DEBUG);
		AppHelper::setAppFont(16);
		//AppHelper::setAppCode();
		kernel->configure("./kernel.xml");
		kernel->initialize();

		addAutoReleasedObject(new VersionFactory);
		addMainCompoments();
		addManualCompoments();
		addAutoCompoments();
		addAlarmCompoments();
		addHeadCompoments();
		
	}
	catch (std::exception&e){
		*errorString = e.what();
		return false;
	}
	return true;	

}

/**
* extensionsInitialized
*/
void FortrendDeviceModel::extensionsInitialized(){
	kernel->process();
}

void FortrendDeviceModel::aboutToShutdown(){

}

/// <summary>
/// 启用
/// </summary>
void FC::FortrendDeviceModel::StatusOpen()
{
	status_widget->setEnabled(true);//主界面
	tm_widget->setEnabled(true);//抽真空
	tm_widget2->setEnabled(true);//破真空
	slot_transfer_cycle_vtm_widget->setEnabled(true);// 循环 配方
	efemmodule_tabWidget->setEnabled(true);//EFEM
	control_widget->setEnabled(true);//设置
	module_tabWidget->setEnabled(true);//手动
}

/// <summary>
/// 禁用
/// </summary>
void FC::FortrendDeviceModel::StatusClose()
{
	status_widget->setEnabled(false);//主界面
	tm_widget->setEnabled(false);//抽真空
	tm_widget2->setEnabled(false);//破真空
	slot_transfer_cycle_vtm_widget->setEnabled(false);// 循环 配方
	efemmodule_tabWidget->setEnabled(false);//EFEM
	control_widget->setEnabled(false);//设置
	module_tabWidget->setEnabled(false);//手动
}


MODULE_BEGIN
MODULE_CLASS(FortrendDeviceModel)
MODULE_END


// optional set up and clean up functions
void xlabInitializeLibrary()
{
	std::cout << "device  dll initializing" << std::endl;
}
void xlabUninitializeLibrary()
{
	std::cout << "device  dll  uninitializing" << std::endl;
}
