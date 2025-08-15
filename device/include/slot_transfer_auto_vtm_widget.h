/**
 * @file            slot_transfer_auto_test_widget.h
 * @brief           slot transfer auto test widget
 * @author			xielonghua
 */

// Library: FortrendUI
// Package: VTM

#ifndef _SLOT_TRANSFER_AUTO_VTM_WIDGET_INCLUDE_
#define _SLOT_TRANSFER_AUTO_VTM_WIDGET_INCLUDE_
#include  "kernel/kernel_exception.h"
#include  "Kernel/FortrendUI/fortrend_ui_macros.h"
#include  "Kernel/CoreUI/kernel_module_widget.h" 
#include  "ExcelAssistant/excel_assistant.h"

#include  <QWidget>
#include  <QVariant>
#include  <QVariantList>
#include  <vector>
#include  <memory>

class QTableWidget;

namespace  FC {
	class IKernel;
	class QSlotTransferAutoVTMWidgetPrivate;

	/**
	 * @brief   slot transfer cycle test widget
	 */
	class  QSlotTransferAutoVTMWidget : public QKernelModuleWidget{
		Q_OBJECT
	public:
		QSlotTransferAutoVTMWidget(const std::shared_ptr<IKernel>& kernel, QWidget* parent = NULL);
		~QSlotTransferAutoVTMWidget();

	private slots:
		void onAddAnItem();
		void onDeleteTheSelectedItem();
		void onClearSequence();
		void onSaveSequence();
		void onLoadSequence();

		void onLoadTargetSite();
		void onSaveTargetSite();

		void onStart();
		void onPause();
		void onReset();
		void onLoadLock1PutCassetteFinished();
		void onLoadLock2PutCassetteFinished();
		void updateAutoData(const QString& message) throw (KernelException);
		void updateControlEnabled(const QString& name,const bool enabled) throw (KernelException);
		void updateProcessEnabled();
		void onGetStep();

		void showMessage(const QString& message)throw (KernelException);
		
	private:
		void startRobotAction();
		void startVacuumAction();
		void startLoadLock1Action();
		void startLoadLock2Action();
		void resetAction();
		void threadShowAlarmMessage();
		void threadGetPMCavityProcessData();

		void addEditTableWidgetItemDoubleSpinBox(int row, int column, double min_value, double max_value, double single_step, double value, int decimals_value =3);

		bool verificationLoadDataHeadName(const QList<QVariant> &head_names);
		bool verificationLoadDataItem(const QList<QVariant> &items, const int row_index);
		bool verificationDataInRange(const QVariant &item, const double min,const double max);
	private:
		ExcelAssistant * excel_helper;
		bool test_flag = true;
	protected:
		Q_DECLARE_PRIVATE(QSlotTransferAutoVTMWidget)
			QSlotTransferAutoVTMWidgetPrivate *d_ptr;

	};



}

#endif
