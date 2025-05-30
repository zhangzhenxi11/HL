/**
* @file            fortrend_tm_cavity_subsystem.h
* @brief           Fortrend tm cavity subsystem widget
* @author		   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/TMCavity


#ifndef _XLH_FORTREND_TM_CAVITY_SUBSYSTEM_WIDGET_INCLUDE_
#define _XLH_FORTREND_TM_CAVITY_SUBSYSTEM_WIDGET_INCLUDE_ 
#include  "kernel/kernel_exception.h"
#include  "kernel/FortrendUI/abstract_subsystem_widget.h"
#include  "fortrend_tm_cavity_subsystem.h"
#include  <QWidget>
#include  <vector>
#include  <memory>


namespace FC{

	class QTMCavitySubsystemWidgetPrivate;
	class  QTMCavitySubsystemWidget : public QAbstractSubsystemWidget < FortrendTMCavitySubsystem > {
		Q_OBJECT
	public:
		QTMCavitySubsystemWidget(const std::shared_ptr<FortrendTMCavitySubsystem>& tm_cavity, QWidget* parent = NULL);
		~QTMCavitySubsystemWidget();

	private slots:
		void onReset();
		void onGetStatus();
		void onOpenDiaphragmValve();
		void onCloseDiaphragmValve();
		void onOpenHeightVacuumBaffleValve();
		void onCloseHeightVacuumBaffleValve();
		void onOpenInsertingPlateValve();
		void onCloseInsertingPlateValve();
		void onOpenAngleValve();
		void onCloseAngleValve();
		void onOpenFlowmeterDiaphragmValve();
		void onCloseFlowmeterDiaphragmValve();
		void onOpenPID();
		void onClosePID();
		void onAttributeUpdate()throw(KernelException);
	private:
		void init();
	private:
		Q_DECLARE_PRIVATE(QTMCavitySubsystemWidget)
			QTMCavitySubsystemWidgetPrivate *d_ptr;

	};

}

#endif
