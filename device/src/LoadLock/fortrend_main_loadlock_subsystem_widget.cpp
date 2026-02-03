
/**
* @file            fortrend_loadlock_subsystem.h
* @brief           Fortrend LoadLock widget
* @author		   xielonghua
*/

// Library: Fortrend
// Package: SubSystem//LoadLock

#include  "LoadLock/fortrend_main_loadlock_subsystem_widget.h"
#include  "Kernel/kernel.h"
#include  "Kernel/kernel_subsystem_reset_command.h"
#include  "Kernel/kernel_subsystem_update_command.h"
#include  "Kernel/CoreUI/kernel_subsystem_status_widget.h"  
#include  "Kernel/FortrendUI/slot_widget.h"
#include  "Kernel/FortrendUI/cassette_widget.h"
#include  "Kernel/Fortrend/robot_wafer_abstract_subsystem.h"
#include  "Kernel/Fortrend/fortrend_cassette_manager.h"
#include  "Kernel/Fortrend/cassette.h"

#include  "device/ui_fortrend_main_loadlock_subsystem_widget.h"

#include  "LoadLock/fortrend_loadlock_defined.h"

#include <QLabel>
#include <QRadioButton>
#include <QSpacerItem>
#include <QMessageBox>
#include <QCheckBox>
#if _MSC_VER >1600
#pragma execution_character_set("utf-8")
#endif



namespace FC{
	/**
	* QMainLoadLockSubsystemWidgetPrivate
	**/
	class QMainLoadLockSubsystemWidgetPrivate{
		Q_DECLARE_PUBLIC(QMainLoadLockSubsystemWidget)
	public:
		QMainLoadLockSubsystemWidgetPrivate(QMainLoadLockSubsystemWidget* p);
	public:
		Ui::FortrendMainLoadLockSubsystem* ui;
		QMainLoadLockSubsystemWidget* q_ptr;

		//std::vector<QCheckBox*> arm_stat;
		std::vector<QCheckBox*> input_checkboxs;


		QCheckBox* cassette_door_ckb = 0, *tm_cavity_door_ckb = 0, *slow_diaphragm_valve_ckb = 0, *fast_diaphragm_valve_ckb = 0, *ultrahigh_vacuum_baffle_valve_ckb = 0, *inserting_plate_value_ckb = 0, *angle_valve_ckb = 0;
		QCheckBox* present_sensor_left_ckb = 0, *present_sensor_right_ckb = 0, *protruding_sensor_ckb = 0;
		QLineEdit *current_vacuumvalue_let = 0;
	};

	QMainLoadLockSubsystemWidgetPrivate::QMainLoadLockSubsystemWidgetPrivate(
		QMainLoadLockSubsystemWidget* p)

		:q_ptr(p){

	}






	/**
	* QMainLoadLockSubsystemWidget
	**/
	QMainLoadLockSubsystemWidget::QMainLoadLockSubsystemWidget(
		const std::shared_ptr<FortrendLoadLockSubsystem>& robot,
		QWidget*parent)
		: QAbstractSubsystemWidget<FortrendLoadLockSubsystem>(robot, parent)
		, d_ptr(new QMainLoadLockSubsystemWidgetPrivate(this)){

		Q_D(QMainLoadLockSubsystemWidget);
		d->ui = new Ui::FortrendMainLoadLockSubsystem;
		d->ui->setupUi(this);
	/*	d->ui->open_exhaust_valve_btn->setVisible(false);
		d->ui->close_exhaust_valve_btn->setVisible(false);*/
		init();
		name = robot->getName();
		onAttributeUpdate();

	
	}

	QMainLoadLockSubsystemWidget::~QMainLoadLockSubsystemWidget(){

	}


	void QMainLoadLockSubsystemWidget::init(){
		Q_D(QMainLoadLockSubsystemWidget);

		QKernelSubsystemStatusWidget* status_widget = new QKernelSubsystemStatusWidget(getSubsystem());
		QLabel *label_name = new QLabel();
		QFont font = QFont();
		font.setFamily("Arial");  //# 设置字体类型
		font.setPointSize(14);   // # 设置字体大小
		font.setBold(true);      //# 设置字体加粗
		//font.setItalic(true);    // # 设置字体斜体
		label_name->setFont(font);    // # 应用字体样式到QLabel

		label_name->setAlignment(Qt::AlignCenter);
		label_name->setText(QString::fromStdString(getSubsystem()->getName()));
		d->ui->status_layout->insertWidget(0, label_name);
		d->ui->status_layout->insertWidget(1, status_widget);
		//connect
		connect(status_widget, &QKernelSubsystemStatusWidget::onStatusUpdate, this, [=](){
			onAttributeUpdate();
		});

#if 0
		d->present_sensor_left_ckb = new QCheckBox(QString::fromStdString("左晶圆在位检测感应器"));
		d->present_sensor_left_ckb->setObjectName("io_object");
		d->present_sensor_left_ckb->setEnabled(false);
		d->ui->inputs_layout->addWidget(d->present_sensor_left_ckb, 0, 0, 1, 1);
		d->input_checkboxs.push_back(d->present_sensor_left_ckb);

		d->present_sensor_right_ckb = new QCheckBox(QString::fromStdString("右晶圆在位检测感应器"));
		d->present_sensor_right_ckb->setObjectName("io_object");
		d->present_sensor_right_ckb->setEnabled(false);
		d->ui->inputs_layout->addWidget(d->present_sensor_right_ckb, 0, 1, 1, 1);
		d->input_checkboxs.push_back(d->present_sensor_right_ckb);

		d->protruding_sensor_ckb = new QCheckBox(QString::fromStdString("晶圆突出检测感应器"));
		d->protruding_sensor_ckb->setObjectName("io_object");
		d->protruding_sensor_ckb->setEnabled(false);
		d->ui->inputs_layout->addWidget(d->protruding_sensor_ckb, 0, 2, 1, 1);
		d->input_checkboxs.push_back(d->protruding_sensor_ckb);
#endif
		//d->ultrahigh_vacuum_baffle_valve_ckb = new QCheckBox("高真空挡板阀");
		//d->inserting_plate_value_ckb = new QCheckBox("插板阀");

		d->cassette_door_ckb = new QCheckBox("晶圆盒门阀");
		d->tm_cavity_door_ckb = new QCheckBox("传输腔门阀");
		d->slow_diaphragm_valve_ckb = new QCheckBox("慢充隔膜阀");
		d->fast_diaphragm_valve_ckb = new QCheckBox("快充隔膜阀");
		d->angle_valve_ckb = new QCheckBox("角阀");

		//d->ultrahigh_vacuum_baffle_valve_ckb->setObjectName("io_object");
		//d->inserting_plate_value_ckb->setObjectName("io_object");
		d->cassette_door_ckb->setObjectName("io_object");
		d->tm_cavity_door_ckb->setObjectName("io_object");
		d->slow_diaphragm_valve_ckb->setObjectName("io_object");
		d->fast_diaphragm_valve_ckb->setObjectName("io_object");
		d->angle_valve_ckb->setObjectName("io_object");

		//d->ultrahigh_vacuum_baffle_valve_ckb->setEnabled(false);
		//d->inserting_plate_value_ckb->setEnabled(false);
		d->cassette_door_ckb->setEnabled(false);
		d->tm_cavity_door_ckb->setEnabled(false);
		d->slow_diaphragm_valve_ckb->setEnabled(false);
		d->fast_diaphragm_valve_ckb->setEnabled(false);
		d->angle_valve_ckb->setEnabled(false);

		d->ui->vacuum_pressure_gage_ckb->setObjectName("io_object");
		d->ui->vacuum_pressure_gage_ckb->setEnabled(false);

		d->ui->operation_state_gridLayout->addWidget(d->cassette_door_ckb, 0, 0);
		d->ui->operation_state_gridLayout->addWidget(d->tm_cavity_door_ckb, 0, 1);
		d->ui->operation_state_gridLayout->addWidget(d->angle_valve_ckb, 0, 2);
		d->ui->operation_state_gridLayout->addWidget(d->slow_diaphragm_valve_ckb, 1, 0);
		d->ui->operation_state_gridLayout->addWidget(d->fast_diaphragm_valve_ckb, 1, 1);
		//d->ui->operation_state_gridLayout->addWidget(d->ultrahigh_vacuum_baffle_valve_ckb, 1, 2);
		//d->ui->operation_state_gridLayout->addWidget(d->inserting_plate_value_ckb, 2, 0);

	}

	std::string QMainLoadLockSubsystemWidget::getName(){
		return name;
	}
	int QMainLoadLockSubsystemWidget::getLoadLockValveOpening(){
		Q_D(QMainLoadLockSubsystemWidget);
		return d->ui->fast_charging_rbt->isChecked() ? 2 : 1;
	}

	int QMainLoadLockSubsystemWidget::getSelectedSlot()const{

		int index = -1;
		//for (int i = 0; i < d_ptr->ui->slots_gridLayout->count(); i++){
		//	QLayoutItem *child = d_ptr->ui->slots_gridLayout->itemAt(i);
		//	QRadioButton* radio_child = qobject_cast<QRadioButton*>(child->widget());
		//	if (radio_child && radio_child->isChecked()){
		//		index = radio_child->property("index").toInt();
		//		//
		//		return index;
		//	}
		//}
		return 0;
	}


	void QMainLoadLockSubsystemWidget::onReset(){
		Q_D(QMainLoadLockSubsystemWidget);
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createResetCommand();
		executeCommand(getSubsystem(), cmd);
	}


	void QMainLoadLockSubsystemWidget::onGetStatus(){
		Q_D(QMainLoadLockSubsystemWidget);
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createUpdateCommand();
		executeCommand(getSubsystem(), cmd);
	}


	void QMainLoadLockSubsystemWidget::onOpenCassetteDoor(){
		Q_D(QMainLoadLockSubsystemWidget);
		//TODO open box

		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createOpenCassetteDoorCommand();
		executeCommand(getSubsystem(), cmd);

	}

	void QMainLoadLockSubsystemWidget::onCloseCassetteDoor(){
		Q_D(QMainLoadLockSubsystemWidget);
		//TODO close box

		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createCloseCassetteDoorCommand();
		executeCommand(getSubsystem(), cmd);
	}

	void QMainLoadLockSubsystemWidget::onMapping(){
		Q_D(QMainLoadLockSubsystemWidget);
		//TODO mapping
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createMappingCommand();
		executeCommand(getSubsystem(), cmd);
	}

	void QMainLoadLockSubsystemWidget::onMoveToSlot(){
		Q_D(QMainLoadLockSubsystemWidget);

		int slot = getSelectedSlot();
		if (slot == 0)
		{
			QMessageBox::information(this, "warn", "Please select slot");
			return;
		}

		//TODO move to slot
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createMoveToSlotCommand(slot);
		executeCommand(getSubsystem(), cmd);
	}

	void QMainLoadLockSubsystemWidget::onOpenTMCavityDoor(){
		Q_D(QMainLoadLockSubsystemWidget);

		//TODO open tm cavity door
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createOpenTMCavityDoorCommand();
		executeCommand(getSubsystem(), cmd);
	}
	void QMainLoadLockSubsystemWidget::onCloseTMCavityDoor(){
		//TODO close tm cavity door
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createCloseTMCavityDoorCommand();
		executeCommand(getSubsystem(), cmd);

	}

	void QMainLoadLockSubsystemWidget::onOpenDiaphragmValve(){
		Q_D(QMainLoadLockSubsystemWidget);

		//TODO open vacuumize valve
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createOpenDiaphragmValveCommand(LoadLockValveOpening::LoadLock_Both);
		executeCommand(getSubsystem(), cmd);
	}
	void QMainLoadLockSubsystemWidget::onCloseDiaphragmValve(){
		//TODO open vacuumize valve
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createCloseDiaphragmValveCommand(LoadLockValveOpening::LoadLock_Both);
		executeCommand(getSubsystem(), cmd);

	}

	void QMainLoadLockSubsystemWidget::onOpenExhaustValve(){
		Q_D(QMainLoadLockSubsystemWidget);

		//TODO open exhaust valve
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createOpenExhaustValveCommand();
		executeCommand(getSubsystem(), cmd);
	}
	void QMainLoadLockSubsystemWidget::onCloseExhaustValve(){
		//TODO close exhaust valve
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createCloseExhaustValveCommand();
		executeCommand(getSubsystem(), cmd);

	}

	void QMainLoadLockSubsystemWidget::onOpenAngleValve(){
		Q_D(QMainLoadLockSubsystemWidget);

		//TODO open exhaust valve
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createOpenAngleValveCommand();
		executeCommand(getSubsystem(), cmd);
	}
	void QMainLoadLockSubsystemWidget::onCloseAngleValve(){
		//TODO close exhaust valve
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createCloseAngleValveCommand();
		executeCommand(getSubsystem(), cmd);

	}

	void QMainLoadLockSubsystemWidget::onAttributeUpdate(){
		Q_D(QMainLoadLockSubsystemWidget);
		
		//update object state
		/*for (int i = 0; i < d->arm_stat.size(); i++){
			d->arm_stat.at(i)->setChecked(getSubsystem()->hasObject(i));
			}*/
		
		//update door state
		//d->present_sensor_left_ckb->setChecked(getSubsystem()->getPresentSensorState(0));
		//d->present_sensor_right_ckb->setChecked(getSubsystem()->getPresentSensorState(1));
		//d->protruding_sensor_ckb->setChecked(getSubsystem()->getProtrudingSensorState());

		d->cassette_door_ckb->setChecked(getSubsystem()->getCassetteDoorOpend());
		d->tm_cavity_door_ckb->setChecked(getSubsystem()->getTMCavityDoorOpend());
		d->ui->current_vacuum_value_let->setText(QString::number(getSubsystem()->getVacuumValue(), 'e', 3).append("Pa"));
		d->ui->vacuum_pressure_gage_ckb->setChecked(getSubsystem()->getVacuumPressureGageState() == 1);

		d->slow_diaphragm_valve_ckb->setChecked(getSubsystem()->getSlowDiaphragmValveOpend());
		d->fast_diaphragm_valve_ckb->setChecked(getSubsystem()->getFastDiaphragmValveOpend());

		//d->ultrahigh_vacuum_baffle_valve_ckb->setChecked(getSubsystem()->getHeightVacuumBaffleValveOpend());
		//d->inserting_plate_value_ckb->setChecked(getSubsystem()->getInsertingPlateValveOpend());
		d->angle_valve_ckb->setChecked(getSubsystem()->getAngleValveOpend());
	}

}
