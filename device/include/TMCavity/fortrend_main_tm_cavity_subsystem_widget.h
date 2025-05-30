/**
* @file            fortrend_tm_cavity_subsystem.h
* @brief           Fortrend tm cavity subsystem widget
* @author		   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/TMCavity


#ifndef _XLH_FORTREND_MAIN_TM_CAVITY_SUBSYSTEM_WIDGET_INCLUDE_
#define _XLH_FORTREND_MAIN_TM_CAVITY_SUBSYSTEM_WIDGET_INCLUDE_ 
#include  "kernel/FortrendUI/abstract_subsystem_widget.h"
#include  "fortrend_tm_cavity_subsystem.h"
#include  "Aligner/fortrend_aligner_subsystem.h"
#include  <QWidget>
#include  <vector>
#include  <memory>


namespace FC{

	class QMainTMCavitySubsystemWidgetPrivate;
	class  QMainTMCavitySubsystemWidget : public QAbstractSubsystemWidget < FortrendTMCavitySubsystem > {
		Q_OBJECT
	public:
		QMainTMCavitySubsystemWidget(const std::shared_ptr<FortrendTMCavitySubsystem>& tm_cavity, const std::shared_ptr<FortrendAlignerSubsystem>& aligner, QWidget* parent = NULL);
		~QMainTMCavitySubsystemWidget();

	private slots:
		void onReset();
		void onGetStatus();
		void onOpenDiaphragmValve();
		void onCloseDiaphragmValve();
		void onOpenHeightVacuumBaffleValve();
		void onCloseHeightVacuumBaffleValve();
		void onOpenAngleValve();
		void onCloseAngleValve();
		void onOpenInsertingPlateValve();
		void onCloseInsertingPlateValve();
		void onOpenFlowmeterDiaphragmValve();
		void onCloseFlowmeterDiaphragmValve();

	private:
		void init();
		void onAttributeUpdate();
	private:
		Q_DECLARE_PRIVATE(QMainTMCavitySubsystemWidget)
			QMainTMCavitySubsystemWidgetPrivate *d_ptr;

	};

}

#endif
