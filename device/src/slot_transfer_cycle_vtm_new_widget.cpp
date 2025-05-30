/**
* @file            slot_transfer_cycle_test_widget.h
* @brief           slot transfer cycle test widget
* @author			xielonghua
*/

// Library: FortrendUI
// Package: VTM

#include <vector>
#include <string>
#include <thread>
#include <Windows.h>

#include "Poco/Format.h"

#include  "slot_transfer_cycle_vtm_new_widget.h" 
#include  "Kernel/FortrendUI/slot_transfer_editor_widget.h"
#include  "device/ui_slot_transfer_cycle_vtm_new_widget.h"
#include  "Kernel/kernel.h"
#include  "kernel/kernel_log.h"
#include  "Kernel/kernel_action_subsystem.h"
#include  "kernel/kernel_parallel_action.h"
#include  "kernel/kernel_subsystem.h"
#include  "Kernel/Fortrend/fortrend_cassette_manager.h"
#include  "Kernel/Fortrend/cassette.h"

#include  "Kernel/Fortrend/fortrend_station.h"
#include  "Kernel/Fortrend/aligner_abstract_subsystem.h"
#include  "Kernel/kernel_subsystem_update_command.h"
#include  "kernel/kernel_block_manager.h"

#include  "Aligner/fortrend_aligner_subsystem.h"
#include  "CoolingCavity/fortrend_cooling_cavity_subsystem.h"
#include  "LoadLock/fortrend_loadlock_subsystem.h"
#include  "PMCavity/fortrend_pm_cavity_subsystem.h"
#include  "TMCavity/fortrend_tm_cavity_subsystem.h"
#include  "SunwayRobot/fortrend_sunwayrobot_subsystem.h"
#include  "Pump/fortrend_pump_subsystem.h"

#include  "Aligner/fortrend_aligner_align_command.h"

#include  "CoolingCavity/fortrend_cooling_cavity_cooling_command.h"
#include  "CoolingCavity/fortrend_cooling_cavity_close_diaphragm_valve_command.h"
#include  "CoolingCavity/fortrend_cooling_cavity_open_diaphragm_valve_command.h"

#include  "LoadLock/fortrend_loadlock_close_angle_valve_command.h"
#include  "LoadLock/fortrend_loadlock_close_cassette_door_command.h"
#include  "LoadLock/fortrend_loadlock_close_diaphragm_valve_command.h"
#include  "LoadLock/fortrend_loadlock_close_exhaust_valve_command.h"
#include  "LoadLock/fortrend_loadlock_close_tm_cavity_door_command.h"
#include  "LoadLock/fortrend_loadlock_mapping_command.h"
#include  "LoadLock/fortrend_loadlock_move_to_slot_command.h"
#include  "LoadLock/fortrend_loadlock_open_angle_valve_command.h"
#include  "LoadLock/fortrend_loadlock_open_cassette_door_command.h"
#include  "LoadLock/fortrend_loadlock_open_diaphragm_valve_command.h"
#include  "LoadLock/fortrend_loadlock_open_exhaust_valve_command.h"
#include  "LoadLock/fortrend_loadlock_open_tm_cavity_door_command.h"

#include  "PMCavity/fortrend_pm_cavity_close_tm_cavity_door_command.h"
#include  "PMCavity/fortrend_pm_cavity_open_tm_cavity_door_command.h"
#include  "PMCavity/fortrend_pm_cavity_reset_command.h"

#include  "Pump/fortrend_pump_mechanical_open_command.h"
#include  "Pump/fortrend_pump_mechanical_close_command.h"
#include  "Pump/fortrend_pump_molecular_open_command.h"
#include  "Pump/fortrend_pump_molecular_close_command.h"

#include  <QVBoxLayout>
#include  <QHBoxLayout>
#include  <QTableWidget>
#include  <QMessageBox>
#include  <QDebug>
#include  <QDoubleSpinBox>
#include  <QFileDialog>
#include  <QSettings>
#include  <QComboBox>

#include <QListWidget>
#include <QTreeView>
#include <QTableWidget>
#include <QTableWidgetItem>

#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTreeView>
#include <QStandardItemModel>


#if _MSC_VER >1600
#pragma execution_character_set("utf-8")
#endif

namespace FC{

	/**
	* QSlotTransferCycleWTMWidgetPrivate
	*/
	class QSlotTransferCycleVTMNewWidgetPrivate : public KernelListener < IKernelCommand > {
	public:
		Q_DECLARE_PUBLIC(QSlotTransferCycleVTMNewWidget)
			QSlotTransferCycleVTMNewWidgetPrivate(QSlotTransferCycleVTMNewWidget*p);


		virtual void onAttributeChange(const IKernelCommand* cmd);



	private:
		QSlotTransferCycleVTMNewWidget * q_ptr;
		Ui::SlotTransferCycleVTMNewWidget *ui;
		std::shared_ptr<IKernel> kernel = 0;


	
	};

	QSlotTransferCycleVTMNewWidgetPrivate::QSlotTransferCycleVTMNewWidgetPrivate(QSlotTransferCycleVTMNewWidget* p)
		: q_ptr(p), ui(new Ui::SlotTransferCycleVTMNewWidget) {
		// Constructor body
		// Initialize any other members or state here
	}

	void QSlotTransferCycleVTMNewWidgetPrivate::onAttributeChange(const IKernelCommand* cmd){
	
	}
	/**
	 * QSlotTransferCycleVTMNewWidget
	 */
	QSlotTransferCycleVTMNewWidget::QSlotTransferCycleVTMNewWidget(const std::shared_ptr<IKernel>& kernel, QWidget* parent)
		:QKernelModuleWidget(parent)
		, d_ptr(new QSlotTransferCycleVTMNewWidgetPrivate(this)){
		Q_D(QSlotTransferCycleVTMNewWidget);

		d->kernel = kernel;

		d->ui = new Ui::SlotTransferCycleVTMNewWidget;
		d->ui->setupUi(this);

		d->ui->commandTableWidget->setColumnCount(5); // 设置列数为3
		d->ui->commandTableWidget->setRowCount(5); // 设置行数为5
		QStringList headers{ "模组", "命令名称", "参数1", "参数2", "参数3" };
		d->ui->commandTableWidget->setHorizontalHeaderLabels(headers);
		
		// 添加数据到每一个单元格
		for (int row = 0; row < 5; ++row) {
			for (int col = 0; col < 5; ++col) {
				QTableWidgetItem *item = new QTableWidgetItem(QString("行%1, 列%2").arg(row + 1).arg(col + 1));
				d->ui->commandTableWidget->setItem(row, col, item);
			}
		}
		// 创建模型
		QStandardItemModel *model = new QStandardItemModel();
		model->setHorizontalHeaderLabels(QStringList() << "名称" << "参数1" << "参数2" << "参数3");

		// 添加数据
		QStandardItem *parentItem = model->invisibleRootItem();
		QStandardItem *childItem;

		// 添加第一级节点
		QStandardItem *firstLevel = new QStandardItem("WTR");
		auto item = new QStandardItem("Get");
		firstLevel->appendRow(item);
		firstLevel->appendRow(new QStandardItem("Put"));
		firstLevel->appendRow(new QStandardItem("Reset"));
		parentItem->appendRow(firstLevel);

		// 添加第二级节点
		QStandardItem *secondLevel = new QStandardItem("LoadLockA");
		secondLevel->appendRow(new QStandardItem("Open"));
		secondLevel->appendRow(new QStandardItem("move slot"));
		secondLevel->appendRow(new QStandardItem("reset"));
		parentItem->appendRow(secondLevel);
		
		d->ui->commandListWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);//禁止编辑
		d->ui->commandListWidget->setModel(model);
		d->ui->commandListWidget->expandAll(); // 展开所有节点

		d->ui->commandListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
		QObject::connect(d->ui->commandListWidget, &QWidget::customContextMenuRequested, [=](const QPoint &pos) {

			QModelIndex index = d->ui->commandListWidget->indexAt(pos);
			if (index.isValid()) {
				QMenu contextMenu;
				QAction *addAction = contextMenu.addAction("Add");
				QObject::connect(addAction, &QAction::triggered, this, [this, index]() {
					this->onAddTask(index);
				});
				contextMenu.exec(d->ui->commandListWidget->viewport()->mapToGlobal(pos));
			}
		});

		d->ui->taskQueueWidget->setContextMenuPolicy(Qt::CustomContextMenu);
		QObject::connect(d->ui->taskQueueWidget, &QWidget::customContextMenuRequested, [=](const QPoint &pos) {
			QListWidgetItem *item = d->ui->taskQueueWidget->itemAt(pos);
			if (item) {
				QMenu contextMenu;
				QAction *removeAction = contextMenu.addAction("Remove");
				QObject::connect(removeAction, &QAction::triggered, [this, item]() {
					this->onRemoveTask(item);
				});
				contextMenu.exec(d->ui->taskQueueWidget->viewport()->mapToGlobal(pos));
			}
		});

		QObject::connect(d->ui->btnUp, &QPushButton::clicked, [this]() {
			Q_D(QSlotTransferCycleVTMNewWidget);
			int currentIndex = d->ui->taskQueueWidget->currentRow();
			if (currentIndex > 0) {
				QListWidgetItem *currentItem = d->ui->taskQueueWidget->takeItem(currentIndex);
				d->ui->taskQueueWidget->insertItem(currentIndex - 1, currentItem);
				d->ui->taskQueueWidget->setCurrentItem(currentItem);
			}
		});

		QObject::connect(d->ui->btnDown, &QPushButton::clicked, [this]() {
			Q_D(QSlotTransferCycleVTMNewWidget);
			int currentIndex = d->ui->taskQueueWidget->currentRow();
			if (currentIndex < d->ui->taskQueueWidget->count() - 1 && currentIndex >= 0) {
				QListWidgetItem *currentItem = d->ui->taskQueueWidget->takeItem(currentIndex);
				d->ui->taskQueueWidget->insertItem(currentIndex + 1, currentItem);
				d->ui->taskQueueWidget->setCurrentItem(currentItem);
			}
		});

		//QListWidgetItem *newItem = new QListWidgetItem(item);
		//// 将 QListWidgetItem 添加到 QListWidget
		//d->ui->taskQueueWidget->addItem(newItem);
		
	}

	QSlotTransferCycleVTMNewWidget::~QSlotTransferCycleVTMNewWidget(){
		Q_D(QSlotTransferCycleVTMNewWidget);
		delete d->ui;
		delete d;
	}

	void QSlotTransferCycleVTMNewWidget::onAddTask(QModelIndex index){
		Q_D(QSlotTransferCycleVTMNewWidget);
			auto model = qobject_cast<QStandardItemModel*>(d->ui->commandListWidget->model());
			QString command = model->itemFromIndex(index)->text();
			QListWidgetItem *newItem = new QListWidgetItem(command);
			d->ui->taskQueueWidget->addItem(newItem);
		
	}
	void QSlotTransferCycleVTMNewWidget::onRemoveTask(QListWidgetItem *item){
		Q_D(QSlotTransferCycleVTMNewWidget);
		delete d->ui->taskQueueWidget->takeItem(d->ui->taskQueueWidget->row(item));
	}


}

