/**
 * @file            slot_transfer_cycle_test_widget.h
 * @brief           slot transfer cycle test widget
 * @author			xielonghua
 */

// Library: FortrendUI
// Package: VTM

#ifndef _SLOT_TRANSFER_CYCLE_VTM_WIDGET_INCLUDE_
#define _SLOT_TRANSFER_CYCLE_VTM_WIDGET_INCLUDE_
#include  "Kernel/FortrendUI/fortrend_ui_macros.h"
#include  "Kernel/CoreUI/kernel_module_widget.h" 
#include "fortrend_station_status_vtm_widget.h"
#include  <QWidget>
#include  <vector>
#include  <memory>

#define SIM_CYCLE_MODE 1
class QTableWidget;

namespace  FC {
	class IKernel;
	class QSlotTransferCycleVTMWidgetPrivate;

	/**
	 * @brief   slot transfer cycle test widget
	 */
	class  QSlotTransferCycleVTMWidget : public QKernelModuleWidget{
		Q_OBJECT
	public:
		QSlotTransferCycleVTMWidget(const std::shared_ptr<IKernel>& kernel, 
			QFortrendStationStatusVTMWidget* ptr, QWidget* parent = NULL);
		~QSlotTransferCycleVTMWidget();
		void onUpdateRecipe(int model);
		void clickStart();
		void clickPause();
		void clickReset();
		void clickAbort();
		void clickplcAuto(bool isauto);
		bool isAuto();

		bool isEnabledStart();
		bool isEnabledPause();
		bool isEnabledReset();
		bool isEnabledAbort();
		bool isEnabledplcAuto();
	
	private slots:
		void onAddAnItem();
		void onDeleteTheSelectedItem();
		void onClearSequence();
		void onSaveSequence();
		void onLoadSequence();

		void onStart();
		void onPause();
		void onReset();
		void onAbort();
		void onLoadLock1PutCassetteFinished();
		void onLoadLock2PutCassetteFinished();
		void onGetStep();
		void update_cycle_data();
		void updateProcessControlEnabled(const bool enabled);
		void updateControlEnabled(const QString control,const bool enabled);

		void addEditTableWidgetItemDoubleSpinBox(int row, int column, double min_value, 
			double max_value, double single_step, double value, int decimals_value = 3);

		void addEditTableWidgetItemComboBox(int row, int column,int value);
		//新增 流程启动
		void startProcessingThreads();

	private:
		void startVacuumAction();
		void startVacuumMoniterAction();
		void startRobotAction();
		void startPMAction();
		void startLoadLock1Action();
		void startLoadLock2Action();
		void startUpdateStatusAction();
		void startEFEMAction();
		void resetAction();
		void onProcess();
		void initPMCavityParamEdieTableWidget();
		void addAnPMItem(const QString name);


	protected:
		Q_DECLARE_PRIVATE(QSlotTransferCycleVTMWidget)
		QSlotTransferCycleVTMWidgetPrivate *d_ptr;
		QFortrendStationStatusVTMWidget* s_ptr;

	};



}

#endif
