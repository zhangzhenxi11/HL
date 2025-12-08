/**
* @file            fortrend_device_ui_model.h
* @brief           device for fortrend for gui
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Main

#ifndef _FORTREND_DEVICE_UI_MODEL_INCLUDE_
#define _FORTREND_DEVICE_UI_MODEL_INCLUDE_
#include <memory>
#include "Core/modulebase.hpp"
#include <Vacuumize/fortrend_vacuumize_subsystem_widget.h>
#include <qtabwidget.h>
#include <BreakVacuum/fortrend_breakvacuum_subsystem_widget.h>
#include <fortrend_station_status_vtm_widget.h>
#include <kernel/CoreUI/kernel_alarm_module_widget.h>
#include <pm_recipe_widget.h>
#include  "slot_transfer_cycle_vtm_widget.h" 

namespace FC{
class FortrendDeviceKernel;



/**
*  sorter device for fortrend
*/
class  FortrendDeviceModel :  public IModule
{
	Q_OBJECT
public:
	FortrendDeviceModel();
	~FortrendDeviceModel();
	virtual bool  initialize(const QStringList &arguments, QString *errorString)override;
	virtual void extensionsInitialized()override;
	virtual void aboutToShutdown()override;

	void StatusOpen();
	void StatusClose();
private:
	void addMainCompoments();
	void addManualCompoments();
	void addAutoCompoments();
	void addAlarmCompoments();
	void addHeadCompoments();

	QFortrendStationStatusVTMWidget* status_widget;//主界面
	QVacuumizeSubsystemWidget* tm_widget;//抽真空
	QBreakVacuumSubsystemWidget* tm_widget2;//破真空
	QSlotTransferCycleVTMWidget* slot_transfer_cycle_vtm_widget;// 循环 配方

	QTabWidget* efemmodule_tabWidget;//EFEM
	QWidget* control_widget;//设置
	QTabWidget* module_tabWidget;//手动
	
	QWidget* system_button_widget;//控制启动、暂停、停止、复位、手动、自动
	QKernelAlarmWidget* alarm;//报警

	QWidget* pmRecipe_widget;//pm配方界面


private:
	std::shared_ptr<FortrendDeviceKernel> kernel;
};

}
#endif
