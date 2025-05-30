/**
* @file            fortrend_pm_cavity_subsystem.h
* @brief           Fortrend PMCavity widget
* @author		   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/PMCavity


#ifndef _XLH_FORTREND_MAIN_PM_CAVITY_SUBSYSTEM_WIDGET_INCLUDE_
#define _XLH_FORTREND_MAIN_PM_CAVITY_SUBSYSTEM_WIDGET_INCLUDE_ 
#include  "kernel/FortrendUI/abstract_subsystem_widget.h"
#include  "fortrend_pm_cavity_subsystem.h"
#include  <QWidget>
#include  <vector>
#include  <memory>


namespace FC{

class QMainPMCavitySubsystemWidgetPrivate;
class  QMainPMCavitySubsystemWidget : public QAbstractSubsystemWidget<FortrendPMCavitySubsystem>{
	Q_OBJECT
public:
	QMainPMCavitySubsystemWidget(const std::shared_ptr<FortrendPMCavitySubsystem>& pm_cavity, QWidget* parent = NULL);
	~QMainPMCavitySubsystemWidget();
private slots:
	void onReset();
	void onGetStatus();
	void onOpenTMCavityDoor();
	void onCloseTMCavityDoor();
	void onGetFinished();
	void onUplaodFinished();
	void onReadProcessParameters();
	void onWriteProcessParameters();
	void onSetSpeed();
	void onSetCrft();
	void onSetTurnSpeed();
	void onStartForward();
	void onStopForward();
	void onStartBackward();
	void onStopBackward();
private:
	void init();
	void onAttributeUpdate();
	void loadConfigFile();
	void saveConfigFile();
private:
	Q_DECLARE_PRIVATE(QMainPMCavitySubsystemWidget)
	QMainPMCavitySubsystemWidgetPrivate *d_ptr;

};

}

#endif
