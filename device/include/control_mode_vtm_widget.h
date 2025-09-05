/**
 * @file            control_mode_vtm_widget.h
 * @brief           control mode vtm widget
 * @author			xielonghua
 */

// Library: FortrendUI
// Package: VTM

#ifndef _CONTROL_MODE_VTM_WIDGET_INCLUDE_
#define _CONTROL_MODE_VTM_WIDGET_INCLUDE_


#include  "Kernel/kernel.h"
#include "Kernel/FortrendUI/fortrend_ui_macros.h"
#include <QWidget>
#include <string>

namespace FC {

	class QControlModeVTMWidgetPrivate;

	/**
	* @brief   slot transfer cycle test widget
	*/
	class  QControlModeVTMWidget : public QWidget{
		Q_OBJECT
	public:
		QControlModeVTMWidget(const std::shared_ptr<IKernel>& kernel, QWidget* parent = NULL);
		~QControlModeVTMWidget();
	private slots:
		void onSetBuzzerEnable();
		void onSetBuzzerDisable();
		void onSetBuzzerEnableStatus(bool checked);

		void onSetVacuumEnable();
		void onSetVacuumDisable();

		void onSetVacuumEnableStatus(bool checked);

		void onSetWithWaferModeEnable();
		void onSetWithWaferModeDisable();

		void onSetWithWaferModeStatus(bool checked);

		void onSetPM1Enable();
		void onSetPM1Disable();

		void onSetPM1Enabletatus(bool checked);

		void onSetPM2Enable();
		void onSetPM2Disable();
		void onSetPM2Enabletatus(bool checked);

		void onSetPM3Enable();
		void onSetPM3Disable();
		void onSetPM3Enabletatus(bool checked);

		void onSetPM4Enable();
		void onSetPM4Disable();
		void onSetPM4Enabletatus(bool checked);

		void onSetLoadLock1VacuumParameters();
		void onSetLoadLock2VacuumParameters();
		void onSetTMCavityVacuumParamerers();
		void onSetPM1CavityVacuumParameters();
		void onSetPM2CavityVacuumParameters();
		void onSetPM3CavityVacuumParameters();

		void updateState();

		void InitUi();

	private:
		void loadConfigFile();
		void saveConfigFile();
	private:
		Q_DECLARE_PRIVATE(QControlModeVTMWidget)
			QControlModeVTMWidgetPrivate *d_ptr;

	};



}

#endif
