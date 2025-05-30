/**
* @file            fortrend_cooling_cavity_subsystem.h
* @brief           Fortrend CoolingCavity widget
* @author		   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/CoolingCavity


#ifndef _XLH_FORTREND_COOLING_CAVITY_SUBSYSTEM_WIDGET_INCLUDE_
#define _XLH_FORTREND_COOLING_CAVITY_SUBSYSTEM_WIDGET_INCLUDE_ 
#include  "kernel/FortrendUI/abstract_subsystem_widget.h"
#include  "fortrend_cooling_cavity_subsystem.h"
#include  <QWidget>
#include  <vector>
#include  <memory>


namespace FC{

class QCoolingCavitySubsystemWidgetPrivate;
class  QCoolingCavitySubsystemWidget : public QAbstractSubsystemWidget<FortrendCoolingCavitySubsystem>{
	Q_OBJECT
public:
	QCoolingCavitySubsystemWidget(const std::shared_ptr<FortrendCoolingCavitySubsystem>& cooling_cavity, QWidget* parent = NULL);
	~QCoolingCavitySubsystemWidget();
private slots:
	void onReset();
	void onGetStatus();
	void onCooling();
private:
	void init();
	void onAttributeUpdate();
private:
	Q_DECLARE_PRIVATE(QCoolingCavitySubsystemWidget)
	QCoolingCavitySubsystemWidgetPrivate *d_ptr;

};

}

#endif
