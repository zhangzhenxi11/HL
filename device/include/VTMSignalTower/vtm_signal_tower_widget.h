/**
 * @file            signal_tower_widget.h
 * @brief           signal  tower  widget
 * @author			xielonghua
 */

// Library: FortrendUI
// Package: Modules


#ifndef _FORTREND_VTM_SIGNAL_TOWER_WIDGET_INCLUDE_
#define _FORTREND_VTM_SIGNAL_TOWER_WIDGET_INCLUDE_
#include "kernel/FortrendUI/fortrend_ui_macros.h"
#include <QWidget>
#include <memory>

namespace  KERNEL_NS {
class FortrendVTMSignalTower;
class QFortrendVTMSignalTowerlWidgetPrivate;

/**
 * @brief  signal  tower  widget
 */
class QFortrendVTMSignalTowerlWidget : public QWidget{
	Q_OBJECT
public:
	QFortrendVTMSignalTowerlWidget(const std::shared_ptr<FortrendVTMSignalTower>& tower, QWidget* parent = NULL);
	~QFortrendVTMSignalTowerlWidget();

protected:
	bool eventFilter(QObject *obj, QEvent *event);

Q_SIGNALS:
	void clicked();

private slots:
	void onUpdateSignals();
private:
	std::shared_ptr<FortrendVTMSignalTower> tower;
	std::shared_ptr<QFortrendVTMSignalTowerlWidgetPrivate> d;
};



}

#endif
