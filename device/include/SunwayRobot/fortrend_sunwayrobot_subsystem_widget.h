
/**
* @file            fortrend_sunwayrobot_subsystem_widget.h
* @brief           Fortrend sunway robot widget
* @author           xielonghua
*/

// Library: Fortrend
// Package: SubSystem/SunwayRobot

#ifndef _XLH_FORTREND_SUNWAYROBOT_SUBSYSTEM_WIDGET_INCLUDE_
#define _XLH_FORTREND_SUNWAYROBOT_SUBSYSTEM_WIDGET_INCLUDE_ 
#include  "kernel/FortrendUI/abstract_subsystem_widget.h"
#include  "fortrend_sunwayrobot_subsystem.h"

#include "LoadLock/fortrend_loadlock_subsystem.h" 

#include  <QWidget>
#include  <vector>
#include  <memory>



namespace FC{

class QSunwayRobotSubsystemWidgetPrivate;
class QSunwayRobotSubsystemWidget : public QAbstractSubsystemWidget<FortrendSunwayRobotSubsystem>{
	Q_OBJECT
public:
	QSunwayRobotSubsystemWidget(const std::shared_ptr<IKernel>& kernel, const std::shared_ptr<FortrendSunwayRobotSubsystem>& fliper, QWidget* parent = NULL);
	~QSunwayRobotSubsystemWidget();
private slots:
	void onReset();
	void onGetStatus();
	void onOutput();
	//special commands
	void onGetWaferCommand();
	void onPutWaferCommand();
	void onReadyGetWaferCommand();
	void onReadyPutWaferCommand();
	void onSetSpeedCommand();
	void onSetAxisZSpeedCommand();
	void onCheckLoadCommand();
	void onClearErrorCommand();
	void onHomeCommand();
	void onAttributeUpdate()throw(KernelException);
private:
	void init();
	std::shared_ptr<IKernel> kernel = 0;
	std::shared_ptr<FortrendStation> getSelectStation()const;
	int getSelectArmId()const;

private:
	Q_DECLARE_PRIVATE(QSunwayRobotSubsystemWidget)
		QSunwayRobotSubsystemWidgetPrivate *d_ptr;

};

}

#endif
