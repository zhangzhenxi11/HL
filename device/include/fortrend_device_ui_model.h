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

	QFortrendStationStatusVTMWidget* status_widget;
	QVacuumizeSubsystemWidget* tm_widget;
	QBreakVacuumSubsystemWidget* tm_widget2;
	QSlotTransferCycleVTMWidget* slot_transfer_cycle_vtm_widget;

	QTabWidget* efemmodule_tabWidget;
	QWidget* control_widget;
	QTabWidget* module_tabWidget;
	
	QWidget* system_button_widget;
	QKernelAlarmWidget* alarm;

	QWidget* pmRecipe_widget;

	QWidget* data_widget;
	QWidget* datahistory_widget;

private:
	std::shared_ptr<FortrendDeviceKernel> kernel;
};

}
#endif
