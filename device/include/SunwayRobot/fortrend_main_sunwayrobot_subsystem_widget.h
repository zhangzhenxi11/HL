
/**
* @file            fortrend_sunwayrobot_subsystem_widget.h
* @brief           Fortrend sunway robot widget
* @author           xielonghua
*/

// Library: Fortrend
// Package: SubSystem/SunwayRobot

#ifndef _XLH_FORTREND_MAIN_SUNWAYROBOT_SUBSYSTEM_WIDGET_INCLUDE_
#define _XLH_FORTREND_MAIN_SUNWAYROBOT_SUBSYSTEM_WIDGET_INCLUDE_ 
#include  "kernel/FortrendUI/abstract_subsystem_widget.h"
#include  "fortrend_sunwayrobot_subsystem.h"
#include  <QWidget>
#include  <vector>
#include  <memory>



namespace FC{

class QMainSunwayRobotSubsystemWidgetPrivate;
class QMainSunwayRobotSubsystemWidget : public QAbstractSubsystemWidget<FortrendSunwayRobotSubsystem>{
	Q_OBJECT
public:
	QMainSunwayRobotSubsystemWidget(const std::shared_ptr<FortrendSunwayRobotSubsystem>& fliper, QWidget* parent = NULL);
	~QMainSunwayRobotSubsystemWidget();
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

private:
	void init();
	void onAttributeUpdate();
	std::shared_ptr<FortrendStation> getSelectStation()const;
	int getSelectArmId()const;

private:
	Q_DECLARE_PRIVATE(QMainSunwayRobotSubsystemWidget)
		QMainSunwayRobotSubsystemWidgetPrivate *d_ptr;

};

}

#endif
