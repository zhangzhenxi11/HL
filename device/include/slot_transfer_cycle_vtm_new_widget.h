/**
 * @file            slot_transfer_cycle_test_widget.h
 * @brief           slot transfer cycle test widget
 * @author			xielonghua
 */

// Library: FortrendUI
// Package: VTM

#ifndef _SLOT_TRANSFER_CYCLE_VTM_NEW_WIDGET_INCLUDE_
#define _SLOT_TRANSFER_CYCLE_VTM_NEW_WIDGET_INCLUDE_
#include  "Kernel/FortrendUI/fortrend_ui_macros.h"
#include  "Kernel/CoreUI/kernel_module_widget.h" 
#include  <QWidget>
#include  <vector>
#include  <memory>
#include <QListWidget>

class QTableWidget;

namespace  FC {
	class IKernel;
	class QSlotTransferCycleVTMNewWidgetPrivate;

	/**
	 * @brief   slot transfer cycle test widget
	 */
	class  QSlotTransferCycleVTMNewWidget : public QKernelModuleWidget{
		Q_OBJECT
	public:
		QSlotTransferCycleVTMNewWidget(const std::shared_ptr<IKernel>& kernel, QWidget* parent = NULL);
		~QSlotTransferCycleVTMNewWidget();

	private slots:
	void onAddTask(QModelIndex index);
	void onRemoveTask(QListWidgetItem *item);
	/*public:
		void update_cycle_data();
	private slots:
		void onAddAnItem();
		void onDeleteTheSelectedItem();
		void onClearSequence();
		void onSaveSequence();
		void onLoadSequence();

		void onStart();
		void onPause();
		void onReset();
		void onLoadLock1PutCassetteFinished();
		void onLoadLock2PutCassetteFinished();
		void onGetStep();
	
	private:
		void startVacuumAction();
		void startVacuumMoniterAction();
		void startRobotAction();
		void startLoadLock1Action();
		void startLoadLock2Action();
		void resetAction();

		void addEditTableWidgetItemDoubleSpinBox(int row, int column, double min_value, double max_value,double single_step,double value);*/
		//QString setStyleSheetColor(QString Control);
	protected:
		Q_DECLARE_PRIVATE(QSlotTransferCycleVTMNewWidget)
			QSlotTransferCycleVTMNewWidgetPrivate *d_ptr;

	};



}

#endif
