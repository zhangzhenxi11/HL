/**
 * @file            fortrend_station_status_vtm_widget.h
 * @brief           fortrend_station_status vtm widget
 * @author			xielonghua
 */

// Library: FortrendUI
// Package: VTM

#ifndef _FORTREND_STATION_STATUS_VTM_WIDGET_INCLUDE_
#define _FORTREND_STATION_STATUS_VTM_WIDGET_INCLUDE_

#include  "kernel/FortrendUI/abstract_subsystem_widget.h"
#include  "Kernel/kernel.h"
#include "Kernel/FortrendUI/fortrend_ui_macros.h"
#include "TMCavity/fortrend_tm_cavity_subsystem.h"
#include "SunwayRobot/fortrend_sunwayrobot_subsystem.h"
#include "loadlockA.h"
#include "loadlockB.h"
#include "tm.h"
#include "chamberwidget.h"
#include "gdt_pmwidget.h"

#include "ProgressColor.h" //进度条
#include <QWidget>
#include "qcoreapplication.h"
#include "qimage.h"
#include <qurl.h>
#include "qpainter.h"
#include <QPainter>


namespace FC {

	class QFortrendStationStatusVTMWidgetPrivate;

	/**
	* @brief   fortrend station status vtm widget
	*/
	class  QFortrendStationStatusVTMWidget : public QWidget{
		Q_OBJECT
	public:
		QFortrendStationStatusVTMWidget(const std::shared_ptr<IKernel>& kernel, QWidget* parent = NULL);
		~QFortrendStationStatusVTMWidget();
		void paintEvent(QPaintEvent *event)override;
		void setAxislocation(double location);
	public:
		Q_INVOKABLE void Animation(int station, int arm, QString action);
		Q_INVOKABLE void updateCassetteAnimation(const std::string &name, bool isBoxOpened, const std::vector<Cassette::Mapping> &mapping);
	signals :
		void signalUpdateRecipe(int model); //切换配方信号
		void signalSelectTransferMode(int model);
	private slots:
		void updateState();
		void pauseAnimation();
		void alignerAnimation();
		void pmAnimation(int station,int speed);
		//void Animation(int station, int arm, QString action);

		void onRobotDialog();
		void onRobotReset();
		void onRobotGetStatus();
		void onRobotClearError();
		//void onRobotData();
		//void onRobotStatusDialog();
		void onRobotGet(int station, int arm, int slot);
		void onRobotPut(int station, int arm, int slot);
		void onRobotSpeed(int speed);
		void onRobotZSpeed(int speed);
		void onRobotGetFinished();
		void onRobotPutFinished();

		void onLoadLockOpenbox(std::string name);
		void onLoadLockClosebox(std::string name);
		void onLoadLockMapping(std::string name);
		void onLoadLockMoveToSlot(std::string name);
		void onLoadLockOpenTMCavityDoor(std::string name);
		void onLoadLockCloseTMCavityDoor(std::string name);
		void onLoadLockOpenDiaphragmValve(std::string name);
		void onLoadLockCloseDiaphragmValve(std::string name);
		void onLoadLockOpenExhaustValve(std::string name);
		void onLoadLockCloseExhaustValve(std::string name);
		void onLoadLockOpenAngleValve(std::string name);
		void onLoadLockCloseAngleValve(std::string name);
		void onOpenInsertingPlateValve(std::string name);
		void onCloseInsertingPlateValve(std::string name);
		void onOpenHeightVacuumBaffleValve(std::string name);
		void onCloseHeightVacuumBaffleValve(std::string name);
		void onLoadLockMoveToOrigin(std::string name);

		void onLoadLockGetStatus(std::string name);
		void onLoadLockReset(std::string name);
		void onLoadLockClearError(std::string name);

		void onTMOpenDiaphragmValve(std::string name);
		void onTMCloseDiaphragmValve(std::string name);
		void onTMOpenHeightVacuumBaffleValve(std::string name);
		void onTMCloseHeightVacuumBaffleValve(std::string name);
		void onTMOpenAngleValve(std::string name);
		void onTMCloseAngleValve(std::string name);
		void onTMOpenInsertingPlateValve(std::string name);
		void onTMCloseInsertingPlateValve(std::string name);
		void onTMOpenFlowmeterDiaphragmValve(std::string name);
		void onTMCloseFlowmeterDiaphragmValve(std::string name);
		void onTMReset(std::string name);
		void onTMGetStatus(std::string name);

		void onAlignerReset(std::string name);
		void onAlignerAlign(std::string name);

		void onPMOpenTMCavityDoor(std::string name);
		void onPMCloseTMCavityDoor(std::string name);
		void onPMGetFinished(std::string name);
		void onPMUplaodFinished(std::string name);
		void onPMRotatePosFinished(std::string name);
		void onPMRotatingDegreeFinished(std::string name);
		void onPMZaxisReset(std::string name);
		void onPMRaxisReset(std::string name);
		void onPMClearZaxisError(std::string name);
		void onPMClearRaxisError(std::string name);

		void onPMGetStatus(std::string name);
		void onPMReset(std::string name);

		void _onSelectASlot(int solt);
		void _onSelectBSlot(int solt);
		void onRecipe();
		void onModel();

		void onEfemReset();

	private:
		virtual std::string getName()const { return "station_Status"; }
		void initTipsUI();
		void process();

	private:
		Q_DECLARE_PRIVATE(QFortrendStationStatusVTMWidget)
			QFortrendStationStatusVTMWidgetPrivate *d_ptr;

	};



}

#endif
