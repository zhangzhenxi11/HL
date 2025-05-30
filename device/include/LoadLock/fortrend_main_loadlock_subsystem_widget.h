/**
* @file            fortrend_loadlock_subsystem.h
* @brief           Fortrend LoadLock widget
* @author		   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/LoadLock


#ifndef _XLH_FORTREND_MAIN_LOADLOCK_SUBSYSTEM_WIDGET_INCLUDE_
#define _XLH_FORTREND_MAIN_LOADLOCK_SUBSYSTEM_WIDGET_INCLUDE_ 
#include  "kernel/FortrendUI/abstract_subsystem_widget.h"
#include  "fortrend_loadlock_subsystem.h"
#include  <QWidget>
#include  <vector>
#include  <memory>


namespace FC{

class QMainLoadLockSubsystemWidgetPrivate;
class  QMainLoadLockSubsystemWidget : public QAbstractSubsystemWidget<FortrendLoadLockSubsystem>{
	Q_OBJECT
public:
	QMainLoadLockSubsystemWidget(const std::shared_ptr<FortrendLoadLockSubsystem>& loadlock, QWidget* parent = NULL);
	~QMainLoadLockSubsystemWidget();
private slots:
	void onReset();
	void onGetStatus();
	void onOpenCassetteDoor();
	void onCloseCassetteDoor();
	void onMapping();
	void onMoveToSlot();
	void onOpenTMCavityDoor();
	void onCloseTMCavityDoor();
	void onOpenDiaphragmValve();
	void onCloseDiaphragmValve();
	void onOpenExhaustValve();
	void onCloseExhaustValve();
	void onOpenAngleValve();
	void onCloseAngleValve();
public:
	int getLoadLockValveOpening();
	std::string getName();
private:
	void init();
	void onAttributeUpdate();
	int getSelectedSlot()const;
private:
	Q_DECLARE_PRIVATE(QMainLoadLockSubsystemWidget)
	QMainLoadLockSubsystemWidgetPrivate *d_ptr;
	std::string name;
};

}

#endif
