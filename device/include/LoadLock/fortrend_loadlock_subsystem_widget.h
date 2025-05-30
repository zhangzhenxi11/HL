/**
* @file            fortrend_loadlock_subsystem.h
* @brief           Fortrend LoadLock widget
* @author		   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/LoadLock


#ifndef _XLH_FORTREND_LOADLOCK_SUBSYSTEM_WIDGET_INCLUDE_
#define _XLH_FORTREND_LOADLOCK_SUBSYSTEM_WIDGET_INCLUDE_ 
#include  "kernel/FortrendUI/abstract_subsystem_widget.h"
#include  "fortrend_loadlock_subsystem.h"
#include  <QWidget>
#include  <vector>
#include  <memory>


namespace FC{

class QLoadLockSubsystemWidgetPrivate;
class  QLoadLockSubsystemWidget : public QAbstractSubsystemWidget<FortrendLoadLockSubsystem>{
	Q_OBJECT
public:
	QLoadLockSubsystemWidget(const std::shared_ptr<FortrendLoadLockSubsystem>& loadlock, QWidget* parent = NULL);
	~QLoadLockSubsystemWidget();
private slots:
	void onReset();
	void onGetStatus();
	void onClearError();
	void onOpenCassetteDoor();
	void onCloseCassetteDoor();
	void onMapping();

	void onMoveToSlot();
	void onMoveToSlotEFEM();
	void onMoveToOrigin();

	void onOpenTMCavityDoor();
	void onCloseTMCavityDoor();

	void onOpenDiaphragmValve();
	void onCloseDiaphragmValve();

	void onOpenHeightVacuumBaffleValve();//取消
	void onCloseHeightVacuumBaffleValve();//取消

	void onOpenInsertingPlateValve();//取消
	void onCloseInsertingPlateValve();//取消

	void onOpenExhaustValve();//取消
	void onCloseExhaustValve();//取消

	void onOpenAngleValve();
	void onCloseAngleValve();

	void onAttributeUpdate()throw(KernelException);
private:
	void init();
	int getSelectedSlot()const;
private:
	Q_DECLARE_PRIVATE(QLoadLockSubsystemWidget)
	QLoadLockSubsystemWidgetPrivate *d_ptr;

};

}

#endif
