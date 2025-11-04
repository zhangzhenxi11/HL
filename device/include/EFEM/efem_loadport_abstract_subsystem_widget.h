/**
 * @file            loadport_abstract_subsystem_widget.h
 * @brief           abstract loadport subsystem widget
 * @author			xielonghua
 */

// Library: FortrendUI
// Package: Subsystem/LoadPort



#ifndef _EFEM_LOADPORT_ABSTRACT_SUBSYSTEM_WIDGET_INCLUDE_
#define _EFEM_LOADPORT_ABSTRACT_SUBSYSTEM_WIDGET_INCLUDE_
#include  "Kernel/FortrendUI/fortrend_ui_macros.h"
#include  "Kernel/Fortrend/loadport_abstract_subsystem.h"
#include  "Kernel/FortrendUI/abstract_subsystem_widget.h"
#include  <QWidget>
#include  <vector>
#include  <memory>


namespace  KERNEL_NS {
class QEFEMLoadPortAbstractSubsystemWidgetPrivate;

/**
 * @brief abstract loadport subsystem widget
 */	
class  QEFEMLoadPortAbstractSubsystemWidget : public QAbstractSubsystemWidget<LoadPortAbstractSubsystem>{
Q_OBJECT
public:
	QEFEMLoadPortAbstractSubsystemWidget(const std::shared_ptr<LoadPortAbstractSubsystem>& loadport, QWidget* parent = NULL);
	~QEFEMLoadPortAbstractSubsystemWidget();
	virtual void insertWidget(QWidget*widget, int position = 0);

	/**
	* @brief  set button visible
	* @enum   Button
	*/
	enum Button{
		OPENBOX_BTN = 0x0001,
		CLOSEBOX_BTN = 0x0002,
		OPENDOOR_BTN = 0x0004,
		CLOSEDOOR_BTN = 0x0008,
		LOCK_BTN =	   0x0010,
		UNLOCK_BTN = 0x0020,
		RETMAP_BTN = 0x0040,
		RESET_BTN = 0x0080,
		UPDATE_BTN = 0x0100,
		CLEAR_BTN = 0x0200,
	};
	Q_DECLARE_FLAGS(ButtonFlag, Button)
	
	void setButtonVisible(ButtonFlag btns, bool visible);

private slots:
	void onClear();
	void onReset();
	void onGetStatus();
	void closeDoor();
	void openDoor();
	void closeBox();
	void openBox();
	void lockBox();
	void unlockBox();
	void getmap();
	void updateCassette();
	void updateStation();

private:
	void init();
private:
	Q_DECLARE_PRIVATE(QEFEMLoadPortAbstractSubsystemWidget)
	QEFEMLoadPortAbstractSubsystemWidgetPrivate *d_ptr;

};

Q_DECLARE_OPERATORS_FOR_FLAGS(QEFEMLoadPortAbstractSubsystemWidget::ButtonFlag)

}

#endif
