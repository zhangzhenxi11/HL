/**
 * @file            kernel_system_control_hlayout_widget.h
 * @brief           kernel system control widget
 * @author			xielonghua
 */
 
// Library: Core
// Package: Core

#ifndef _VTM_SYSTEM_CONTROL_HLAYOUT_WIDGET_INCLUDE_
#define _VTM_SYSTEM_CONTROL_HLAYOUT_WIDGET_INCLUDE_
#include "Kernel/CoreUI/kernel_ui_macros.h"
#include "Kernel/CoreUI/kernel_module_widget.h"
#include <QWidget>
#include "QCoreApplication"
#include <QPushButton>
#include "QTimer"

#include <fortrend_device_ui_model.h>
#include  "slot_transfer_cycle_vtm_widget.h" 
class QLabel;

namespace  FC {

class QVTMSystemControlHLayoutWidgetPrivate;
class IKernel;

class  QVTMSystemControlHLayoutWidget : public QKernelModuleWidget{
Q_OBJECT
public:
	QVTMSystemControlHLayoutWidget(const std::shared_ptr<IKernel>& kernel, QSlotTransferCycleVTMWidget* cycle, FortrendDeviceModel*model, QWidget* parent = Q_NULLPTR);
	~QVTMSystemControlHLayoutWidget();
	void btnIco(std::shared_ptr<IKernel> kernel);
	void btnUpdatePath();
	void btnPath(QPushButton *btn, QString t_png, QString btn_tool_txt);
private slots:
	void q_update_status();
	
private:
	Q_DECLARE_PRIVATE(QVTMSystemControlHLayoutWidget)
	QVTMSystemControlHLayoutWidgetPrivate *d_ptr;

};



}

#endif
