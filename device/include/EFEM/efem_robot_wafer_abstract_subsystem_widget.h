/**
 * @file            efem_robot_wafer_abstract_subsystem_widget.h
 * @brief           abstract wafer robot subsystem widget
 * @author			xielonghua
 */

// Library: FortrendUI
// Package: Subsystem/Robot



#ifndef _EFEM_ROBOT2_WAFER_ABSTRACT_SUBSYSTEM_WIDGET_INCLUDE_
#define _EFEM_ROBOT2_WAFER_ABSTRACT_SUBSYSTEM_WIDGET_INCLUDE_
#include  "Kernel/FortrendUI/fortrend_ui_macros.h"
#include  "Kernel/Fortrend/robot_wafer_abstract_subsystem.h"
#include  "Kernel/FortrendUI/abstract_subsystem_widget.h"
#include  <QWidget>
#include  <vector>
#include  <memory>


namespace  KERNEL_NS {
class QEFEMRobotWaferAbstractSubsystemWidgetPrivate;

/**
 * @brief   abstract wafer robot subsystem widget
 */
class QEFEMRobotWaferAbstractSubsystemWidget : public QAbstractSubsystemWidget<WaferRobotAbstractSubsystem>{
	Q_OBJECT
public:
	QEFEMRobotWaferAbstractSubsystemWidget(const std::shared_ptr<WaferRobotAbstractSubsystem>& robot, QWidget* parent = NULL);
	~QEFEMRobotWaferAbstractSubsystemWidget();
	virtual void insertWidget(QWidget*widget, int position = 0);
private slots:
	void onGetObjet();
	void onPutObjet();
	void onReset();
	void onClear();
	void onGetStatus();
	void onOnOutput();
	void onOffOutput();
	void onSetSpeed();
	void onSetSize();
	void onSetToolSize();
	void updateSlotsLayout();

private:
	void init();
	void onAttributeUpdate();
	std::shared_ptr<FortrendStation> getSelectStation()const;
	int getSelectArmId()const;
	int getSelectSlotId()const;
private:
	Q_DECLARE_PRIVATE(QEFEMRobotWaferAbstractSubsystemWidget)
	QEFEMRobotWaferAbstractSubsystemWidgetPrivate *d_ptr;

};



}

#endif
