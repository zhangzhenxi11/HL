/**
* @file            fortrend_pump_subsystem.h
* @brief           Fortrend Pump widget
* @author		   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/Pump


#ifndef _XLH_FORTREND_PUMP_SUBSYSTEM_WIDGET_INCLUDE_
#define _XLH_FORTREND_PUMP_SUBSYSTEM_WIDGET_INCLUDE_ 
#include  "kernel/FortrendUI/abstract_subsystem_widget.h"
#include  "fortrend_pump_subsystem.h"
#include  <QWidget>
#include  <vector>
#include  <memory>


namespace FC{

class QPumpSubsystemWidgetPrivate;
class  QPumpSubsystemWidget : public QAbstractSubsystemWidget<FortrendPumpSubsystem>{
	Q_OBJECT
public:
	QPumpSubsystemWidget(const std::shared_ptr<FortrendPumpSubsystem>& pump, QWidget* parent = NULL);
	~QPumpSubsystemWidget();
private slots:
	void onReset();
	void onGetStatus();
	void onMechanicalOpen();
	void onMechanicalClose();
	void onMolecularOpenLLA();
	void onMolecularCloseLLA();
	void onMolecularOpenLLB();
	void onMolecularCloseLLB();
	void onMolecularOpenTM();
	void onMolecularCloseTM();
	void onOpenLoadLock1AutoVacuum();
	void onOpenLoadLock2AutoVacuum();
	void onOpenTMCavityAutoVacuum();
	void onClearError();
	void onAttributeUpdate()throw(KernelException);
private:
	void init();
	
private:
	Q_DECLARE_PRIVATE(QPumpSubsystemWidget)
	QPumpSubsystemWidgetPrivate *d_ptr;

};

}

#endif
