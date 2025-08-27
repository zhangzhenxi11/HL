
/**
* @file            fortrend_loadlock_subsystem.h
* @brief           Fortrend LoadLock widget
* @author		   xielonghua
*/

// Library: Fortrend
// Package: SubSystem//LoadLock

#include  "LoadLock/fortrend_loadlock_subsystem_widget.h"
#include  "Kernel/kernel.h"
#include  "Kernel/kernel_subsystem_reset_command.h"
#include  "Kernel/kernel_subsystem_update_command.h"
#include  "Kernel/CoreUI/kernel_subsystem_status_widget.h"  
#include  "Kernel/FortrendUI/slot_widget.h"
#include  "Kernel/FortrendUI/cassette_widget.h"
#include  "Kernel/Fortrend/robot_wafer_abstract_subsystem.h"
#include  "Kernel/Fortrend/fortrend_cassette_manager.h"
#include  "Kernel/Fortrend/cassette.h"

#include  "device/ui_fortrend_loadlock_subsystem_widget.h"

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
	* QLoadLockSubsystemWidgetPrivate
	**/
	class QLoadLockSubsystemWidgetPrivate{
		Q_DECLARE_PUBLIC(QLoadLockSubsystemWidget)
	public:
		QLoadLockSubsystemWidgetPrivate(QLoadLockSubsystemWidget* p);
	public:
		Ui::FortrendLoadLockSubsystem* ui;
		QLoadLockSubsystemWidget* q_ptr;

		//std::vector<QCheckBox*> arm_stat;
		std::vector<QCheckBox*> input_checkboxs;


		QCheckBox* cassette_door_ckb = 0, *tm_cavity_door_ckb = 0, *slow_diaphragm_valve_ckb = 0, *fast_diaphragm_valve_ckb = 0, *ultrahigh_vacuum_baffle_valve_ckb = 0, *inserting_plate_value_ckb = 0, *angle_valve_ckb = 0;
		//QCheckBox* vacuum_pressure_gage_ckb = 0;
		//QCheckBox* present_sensor_left_ckb = 0, *present_sensor_right_ckb = 0, *protruding_sensor_ckb = 0;
		QLineEdit *current_vacuumvalue_let = 0;
	};

	QLoadLockSubsystemWidgetPrivate::QLoadLockSubsystemWidgetPrivate(
		QLoadLockSubsystemWidget* p)
		:q_ptr(p){

	}






	/**
	* QLoadLockSubsystemWidget
	**/
	QLoadLockSubsystemWidget::QLoadLockSubsystemWidget(
		const std::shared_ptr<FortrendLoadLockSubsystem>& robot,
		QWidget*parent)
		: QAbstractSubsystemWidget<FortrendLoadLockSubsystem>(robot, parent)
		, d_ptr(new QLoadLockSubsystemWidgetPrivate(this)){

		Q_D(QLoadLockSubsystemWidget);
		d->ui = new Ui::FortrendLoadLockSubsystem;
		d->ui->setupUi(this);
		init();

		onAttributeUpdate();

		connect(d->ui->reset_btn, &QAbstractButton::clicked, this, &QLoadLockSubsystemWidget::onReset);
		connect(d->ui->generate_btn, &QAbstractButton::clicked, this, &QLoadLockSubsystemWidget::onGetStatus);
		connect(d->ui->clear_error_btn, &QAbstractButton::clicked, this, &QLoadLockSubsystemWidget::onClearError);
		connect(d->ui->open_cassette_door_btn, &QAbstractButton::clicked, this, &QLoadLockSubsystemWidget::onOpenCassetteDoor);
		connect(d->ui->close_cassette_door_btn, &QAbstractButton::clicked, this, &QLoadLockSubsystemWidget::onCloseCassetteDoor);
		connect(d->ui->mapping_btn, &QAbstractButton::clicked, this, &QLoadLockSubsystemWidget::onMapping);
		//connect(d->ui->movetoslot_btn, &QAbstractButton::clicked, this, &QLoadLockSubsystemWidget::onMoveToSlot);

		connect(d->ui->open_tm_cavity_door_btn, &QAbstractButton::clicked, this, &QLoadLockSubsystemWidget::onOpenTMCavityDoor);
		connect(d->ui->close_tm_cavity_door_btn, &QAbstractButton::clicked, this, &QLoadLockSubsystemWidget::onCloseTMCavityDoor);

		connect(d->ui->open_diaphragm_valve_btn, &QAbstractButton::clicked, this, &QLoadLockSubsystemWidget::onOpenDiaphragmValve);
		connect(d->ui->close_diaphragm_valve_btn, &QAbstractButton::clicked, this, &QLoadLockSubsystemWidget::onCloseDiaphragmValve);

		connect(d->ui->open_angle_valve_btn, &QAbstractButton::clicked, this, &QLoadLockSubsystemWidget::onOpenAngleValve);
		connect(d->ui->close_angle_valve_btn, &QAbstractButton::clicked, this, &QLoadLockSubsystemWidget::onCloseAngleValve);

		/*connect(d->ui->open_height_vacuum_baffle_valve_btn, &QAbstractButton::clicked, this, &QLoadLockSubsystemWidget::onOpenHeightVacuumBaffleValve);
		connect(d->ui->close_height_vacuum_baffle_valve_btn, &QAbstractButton::clicked, this, &QLoadLockSubsystemWidget::onCloseHeightVacuumBaffleValve);
		connect(d->ui->open_inserting_plate_valve_btn, &QAbstractButton::clicked, this, &QLoadLockSubsystemWidget::onOpenInsertingPlateValve);
		connect(d->ui->close_inserting_plate_valve_btn, &QAbstractButton::clicked, this, &QLoadLockSubsystemWidget::onCloseInsertingPlateValve);
		connect(d->ui->moveorigin, &QAbstractButton::clicked, this, &QLoadLockSubsystemWidget::onMoveToOrigin);
		connect(d->ui->movetoslot_efem_btn, &QAbstractButton::clicked, this, &QLoadLockSubsystemWidget::onMoveToSlotEFEM);*/
		
		//d->ui->vacuum_pressure_gage_ckb->hide();
	}

	QLoadLockSubsystemWidget::~QLoadLockSubsystemWidget(){

	}


	void QLoadLockSubsystemWidget::init(){
		Q_D(QLoadLockSubsystemWidget);

		//subsystem status
		QKernelSubsystemStatusWidget* status_widget = new QKernelSubsystemStatusWidget(getSubsystem());
		d->ui->right_verticalLayout->insertWidget(0, status_widget);
		//connect
		connect(status_widget, &QKernelSubsystemStatusWidget::onStatusUpdate, this, [=](){
			onAttributeUpdate();
		});

		auto cassManager = getSubsystem()->getKernel()->getKernelModule<FortrendCassetteManager>();
		auto cass = cassManager->getCassette(getSubsystem().get());

		//create slot select
		//for (int i = 0; i < cass->slotCount(); i++)
		//{
		//	QRadioButton* radioButton = new QRadioButton;
		//	radioButton->setText(QString::fromStdString(std::to_string(i + 1)));
		//	radioButton->setProperty("index", i + 1); //index
		//	d->ui->slots_gridLayout->addWidget(radioButton, i % 10, i / 10);
		//	if (i == 0)
		//	{
		//		radioButton->setChecked(true);
		//	}
		//}

		


		
	/*	d->present_sensor_left_ckb = new QCheckBox(QString::fromStdString("左晶圆在位检测感应器"));
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
		d->input_checkboxs.push_back(d->protruding_sensor_ckb);*/

		//d->ultrahigh_vacuum_baffle_valve_ckb = new QCheckBox("高真空挡板阀");
		//d->inserting_plate_value_ckb = new QCheckBox("插板阀");

		d->cassette_door_ckb = new QCheckBox("晶圆盒门阀");
		d->tm_cavity_door_ckb = new QCheckBox("传输腔门阀");
		d->slow_diaphragm_valve_ckb = new QCheckBox("慢充隔膜阀");
		d->fast_diaphragm_valve_ckb = new QCheckBox("快充隔膜阀");
		d->angle_valve_ckb = new QCheckBox("角阀");
		//d->vacuum_pressure_gage_ckb = new QCheckBox("真空压力表");

		//d->ultrahigh_vacuum_baffle_valve_ckb->setObjectName("io_object");
		//d->inserting_plate_value_ckb->setObjectName("io_object");
		d->cassette_door_ckb->setObjectName("io_object");
		d->tm_cavity_door_ckb->setObjectName("io_object");
		d->slow_diaphragm_valve_ckb->setObjectName("io_object");
		d->fast_diaphragm_valve_ckb->setObjectName("io_object");
		d->angle_valve_ckb->setObjectName("io_object");
		//d->vacuum_pressure_gage_ckb->setObjectName("io_object");

		//d->ultrahigh_vacuum_baffle_valve_ckb->setEnabled(false);
		//d->inserting_plate_value_ckb->setEnabled(false);
		d->cassette_door_ckb->setEnabled(false);
		d->tm_cavity_door_ckb->setEnabled(false);
		d->slow_diaphragm_valve_ckb->setEnabled(false);
		d->fast_diaphragm_valve_ckb->setEnabled(false);
		d->angle_valve_ckb->setEnabled(false);
		//d->vacuum_pressure_gage_ckb->setEnabled(false);

		d->ui->vacuum_pressure_gage_ckb->setObjectName("io_object");
		d->ui->vacuum_pressure_gage_ckb->setEnabled(false);


		d->ui->operation_state_gridLayout->addWidget(d->cassette_door_ckb, 0, 0);
		d->ui->operation_state_gridLayout->addWidget(d->tm_cavity_door_ckb, 0, 1);
		d->ui->operation_state_gridLayout->addWidget(d->angle_valve_ckb, 0, 2);
		d->ui->operation_state_gridLayout->addWidget(d->slow_diaphragm_valve_ckb, 1, 0);
		d->ui->operation_state_gridLayout->addWidget(d->fast_diaphragm_valve_ckb, 1, 1);

		d->ui->operation_state_gridLayout->addWidget(d->ui->vacuum_pressure_gage_ckb, 2, 0);
		//d->ui->operation_state_gridLayout->addWidget(d->ultrahigh_vacuum_baffle_valve_ckb, 1, 2);
		//d->ui->operation_state_gridLayout->addWidget(d->inserting_plate_value_ckb, 2, 0);

		if (cass){
			QWidget* cassette_Widget = new QFortrendCassetteWidget(cass, cassManager, true, true, 25, this); //max row count = 25
			d->ui->center_layout->addWidget(cassette_Widget);

		}
	}

	int QLoadLockSubsystemWidget::getSelectedSlot()const{

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


	void QLoadLockSubsystemWidget::onReset(){
		Q_D(QLoadLockSubsystemWidget);
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createResetCommand();
		executeCommand(getSubsystem(), cmd);
	}


	void QLoadLockSubsystemWidget::onGetStatus(){
		Q_D(QLoadLockSubsystemWidget);
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createUpdateCommand();
		executeCommand(getSubsystem(), cmd);
	}

	void QLoadLockSubsystemWidget::onClearError(){
		Q_D(QLoadLockSubsystemWidget);
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createClearErrorCommand();
		executeCommand(getSubsystem(), cmd);
	}

	void QLoadLockSubsystemWidget::onOpenCassetteDoor(){
		Q_D(QLoadLockSubsystemWidget);
		//TODO open box

		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createOpenCassetteDoorCommand();
		executeCommand(getSubsystem(), cmd);

	}

	void QLoadLockSubsystemWidget::onCloseCassetteDoor(){
		Q_D(QLoadLockSubsystemWidget);
		//TODO close box

		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createCloseCassetteDoorCommand();
		executeCommand(getSubsystem(), cmd);
	}

	void QLoadLockSubsystemWidget::onMapping(){
		Q_D(QLoadLockSubsystemWidget);
		//TODO mapping
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createMappingCommand();
		executeCommand(getSubsystem(), cmd);
	}

	void QLoadLockSubsystemWidget::onMoveToSlot(){
		Q_D(QLoadLockSubsystemWidget);

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

	void QLoadLockSubsystemWidget::onMoveToSlotEFEM(){
		Q_D(QLoadLockSubsystemWidget);

		int slot = getSelectedSlot();
		if (slot == 0)
		{
			QMessageBox::information(this, "warn", "Please select slot");
			return;
		}
		slot += 30;
		//TODO move to slot
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createMoveToSlotCommand(slot);
		executeCommand(getSubsystem(), cmd);
	}

	void QLoadLockSubsystemWidget::onMoveToOrigin(){
		Q_D(QLoadLockSubsystemWidget);
		//TODO move to slot
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createMoveToSlotCommand(28);
		executeCommand(getSubsystem(), cmd);
	}

	void QLoadLockSubsystemWidget::onOpenTMCavityDoor(){
		Q_D(QLoadLockSubsystemWidget);

		//TODO open tm cavity door
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createOpenTMCavityDoorCommand();
		executeCommand(getSubsystem(), cmd);
	}

	void QLoadLockSubsystemWidget::onCloseTMCavityDoor(){
		//TODO close tm cavity door
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createCloseTMCavityDoorCommand();
		executeCommand(getSubsystem(), cmd);

	}

	void QLoadLockSubsystemWidget::onOpenHeightVacuumBaffleValve(){
		Q_D(QLoadLockSubsystemWidget);
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createOpenHeightVacuumBaffleValveCommand();
		executeCommand(getSubsystem(), cmd);
	}

	void QLoadLockSubsystemWidget::onCloseHeightVacuumBaffleValve(){
		Q_D(QLoadLockSubsystemWidget);
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createCloseHeightVacuumBaffleValveCommand();
		executeCommand(getSubsystem(), cmd);
	}
	void QLoadLockSubsystemWidget::onOpenInsertingPlateValve(){
		Q_D(QLoadLockSubsystemWidget);
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createOpenInsertingPlateValveCommand();
		executeCommand(getSubsystem(), cmd);
	}

	void QLoadLockSubsystemWidget::onCloseInsertingPlateValve(){
		Q_D(QLoadLockSubsystemWidget);
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createCloseInsertingPlateValveCommand();
		executeCommand(getSubsystem(), cmd);
	}

	void QLoadLockSubsystemWidget::onOpenDiaphragmValve(){
		Q_D(QLoadLockSubsystemWidget);
		LoadLockValveOpening select = LoadLockValveOpening::LoadLock_Both;
		if (d->ui->slow_charging_rbt->isChecked())
		{
			select = LoadLockValveOpening::LoadLock_Slow;
		}
		else if (d->ui->fast_charging_rbt->isChecked())
		{
			select = LoadLockValveOpening::LoadLock_Fast;
		}
		else{
			return;
		}
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createOpenDiaphragmValveCommand(select);
		executeCommand(getSubsystem(), cmd);
	}
	void QLoadLockSubsystemWidget::onCloseDiaphragmValve(){
		//TODO open vacuumize valve
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createCloseDiaphragmValveCommand(LoadLockValveOpening::LoadLock_Both);
		executeCommand(getSubsystem(), cmd);

	}

	void QLoadLockSubsystemWidget::onOpenExhaustValve(){
		Q_D(QLoadLockSubsystemWidget);

		//TODO open exhaust valve
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createOpenExhaustValveCommand();
		executeCommand(getSubsystem(), cmd);
	}
	void QLoadLockSubsystemWidget::onCloseExhaustValve(){
		//TODO close exhaust valve
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createCloseExhaustValveCommand();
		executeCommand(getSubsystem(), cmd);

	}

	void QLoadLockSubsystemWidget::onOpenAngleValve(){
		Q_D(QLoadLockSubsystemWidget);

		//TODO open exhaust valve
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createOpenAngleValveCommand();
		executeCommand(getSubsystem(), cmd);
	}
	void QLoadLockSubsystemWidget::onCloseAngleValve(){
		//TODO close exhaust valve
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createCloseAngleValveCommand();
		executeCommand(getSubsystem(), cmd);

	}

	void QLoadLockSubsystemWidget::onAttributeUpdate() throw(KernelException){
		Q_D(QLoadLockSubsystemWidget);
		
		auto cassManager = getSubsystem()->getKernel()->getKernelModule<FortrendCassetteManager>();
		auto cass = cassManager->getCassette(getSubsystem().get());

		//update object state
		/*for (int i = 0; i < d->arm_stat.size(); i++){
			d->arm_stat.at(i)->setChecked(getSubsystem()->hasObject(i));
			}*/

		//update door state
	/*	d->present_sensor_left_ckb->setChecked(getSubsystem()->getPresentSensorState(0));
		d->present_sensor_right_ckb->setChecked(getSubsystem()->getPresentSensorState(1));
		d->protruding_sensor_ckb->setChecked(getSubsystem()->getProtrudingSensorState());*/

		d->cassette_door_ckb->setChecked(getSubsystem()->getCassetteDoorOpend());
		d->tm_cavity_door_ckb->setChecked(getSubsystem()->getTMCavityDoorOpend());
		d->ui->current_vacuum_value_let->setText(QString::number(getSubsystem()->getVacuumValue(), 'e', 3).append("Pa"));
		d->ui->vacuum_pressure_gage_ckb->setChecked(getSubsystem()->getVacuumPressureGageState() == 1);

		d->slow_diaphragm_valve_ckb->setChecked(getSubsystem()->getSlowDiaphragmValveOpend());
		d->fast_diaphragm_valve_ckb->setChecked(getSubsystem()->getFastDiaphragmValveOpend());
		/*d->vacuum_pressure_gage_ckb->setChecked(getSubsystem()->getVacuumPressureGageState() == 1);*/

		//d->ultrahigh_vacuum_baffle_valve_ckb->setChecked(getSubsystem()->getHeightVacuumBaffleValveOpend());
		//d->inserting_plate_value_ckb->setChecked(getSubsystem()->getInsertingPlateValveOpend());
		d->angle_valve_ckb->setChecked(getSubsystem()->getAngleValveOpend());

		//update cassete status
		Cassette::Mapping FirstLayerMappingData = Cassette::Mapping::Unknown;
		Cassette::Mapping SecondLayerMappingData = Cassette::Mapping::Unknown;

		getSubsystem()->getFirstLayerMapping(FirstLayerMappingData);
		cass->setMapping(2, FirstLayerMappingData);

		getSubsystem()->getSecondLayerMapping(SecondLayerMappingData);
		cass->setMapping(1, SecondLayerMappingData);
	}

}
