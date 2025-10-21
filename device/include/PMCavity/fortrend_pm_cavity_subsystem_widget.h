/**
* @file            fortrend_pm_cavity_subsystem.h
* @brief           Fortrend PMCavity widget
* @author		   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/PMCavity


#ifndef _XLH_FORTREND_PM_CAVITY_SUBSYSTEM_WIDGET_INCLUDE_
#define _XLH_FORTREND_PM_CAVITY_SUBSYSTEM_WIDGET_INCLUDE_ 
#include  "kernel/kernel_exception.h"
#include  "kernel/FortrendUI/abstract_subsystem_widget.h"
#include  "fortrend_pm_cavity_subsystem.h"
#include  <QWidget>
#include  <vector>
#include  <memory>


namespace FC{

class QPMCavitySubsystemWidgetPrivate;
class  QPMCavitySubsystemWidget : public QAbstractSubsystemWidget<FortrendPMCavitySubsystem>{
	Q_OBJECT
public:
	QPMCavitySubsystemWidget(const std::shared_ptr<FortrendPMCavitySubsystem>& pm_cavity, QWidget* parent = NULL);
	~QPMCavitySubsystemWidget();
private slots:
	void onReset();
	void onGetStatus();
	void onOpenTMCavityDoor();
	void onCloseTMCavityDoor();
	void onGetFinished();
	void onUplaodFinished();
	void onReadProcessParameters();
	void onWriteProcessParameters();
	void onClearState();
	void onInsertingPlateOpeningController();
	void onAttributeUpdate() throw (KernelException);
	void onToGetStation();
	void onToPutStation();
	void onToRotatingStation();
	void onToTargetPos();
	void onToRotatingDegreeStation();
	void onLiftingAxisRest();
	void onRotaingAxisRest();
private:
	void init();
	
private:
	Q_DECLARE_PRIVATE(QPMCavitySubsystemWidget)
	QPMCavitySubsystemWidgetPrivate *d_ptr;

};

}

#endif
