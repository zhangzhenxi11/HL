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

#include <mutex>

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
			QFortrendStationStatusVTMWidget* ptr,
			QWidget* parent = NULL);
		~QSlotTransferCycleVTMWidget();
		//更新配方
		void onUpdateRecipe(int model);
		void onSelectTransferMode(int model);
		void clickStart();
		void clickPause();
		void clickReset();
		void clickAbort();
		void clickContinue();
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
		void onContinue();
		void onLoadLock1PutCassetteFinished();
		void onLoadLock2PutCassetteFinished();
		void onGetStep();
		void update_cycle_data();
		void updateProcessControlEnabled(const bool enabled);
		void updateControlEnabled(const QString control,const bool enabled);
		void updateLightButtonStatus(const QString control, const int color);

		// WPH测试相关槽函数
		void onWphTestStart();
		void onWphTestStop();
		void updateWphCompletedCount(int count);

		//新增 流程启动
		void startProcessingThreads();

		void initializeThreads();

	private:
		void startVacuumAction();
		void resetAction();
		void onProcess();

		//新增
		void executeEFEMTransfer();

		void executeLLATransfer();

		void executeLLBTransfer();

		void executePM1Transfer();

		void executePM2Transfer();

		void executePM3Transfer();

		void executePM4Transfer();

		void executeTMTransfer();

		void executeUpdateTransferStatus();

		// WPH测试执行函数
		void executeWphTest();
		
	protected:
		Q_DECLARE_PRIVATE(QSlotTransferCycleVTMWidget)
		QSlotTransferCycleVTMWidgetPrivate *d_ptr;
		QFortrendStationStatusVTMWidget* s_ptr;

	};

}

#endif
