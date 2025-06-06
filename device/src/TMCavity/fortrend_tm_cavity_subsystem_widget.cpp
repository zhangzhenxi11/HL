
/**
* @file            fortrend_tm_cavity_subsystem.h
* @brief           Fortrend TMCavity widget
* @author		   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/TMCavity


#include  "Kernel/kernel.h"
#include  "Kernel/kernel_subsystem_reset_command.h"
#include  "Kernel/kernel_subsystem_update_command.h"
#include  "Kernel/CoreUI/kernel_subsystem_status_widget.h"  
#include  "Kernel/FortrendUI/slot_widget.h"
#include  "Kernel/FortrendUI/cassette_widget.h"
#include  "Kernel/Fortrend/robot_wafer_abstract_subsystem.h"
#include  "Kernel/Fortrend/fortrend_cassette_manager.h"
#include  "Kernel/Fortrend/cassette.h"

#include  "device/ui_fortrend_tm_cavity_subsystem_widget.h"
#include  "TMCavity/fortrend_tm_cavity_subsystem_widget.h"
#include  "TMCavity/fortrend_tm_cavity_defined.h"

#include <QRadioButton>
#include <QSpacerItem>
#include <QMessageBox>
#include <QCheckBox>

#if _MSC_VER >1600
#pragma execution_character_set("utf-8")
#endif


namespace FC{
	/**
	* QTMCavitySubsystemWidgetPrivate
	**/
	class QTMCavitySubsystemWidgetPrivate{
		Q_DECLARE_PUBLIC(QTMCavitySubsystemWidget)
	public:
		QTMCavitySubsystemWidgetPrivate(QTMCavitySubsystemWidget* p);
	public:
		Ui::FortrendTMCavitySubsystem* ui;
		QTMCavitySubsystemWidget* q_ptr;

		//std::vector<QCheckBox*> arm_stat;
		std::vector<QCheckBox*> input_checkboxs;

		QCheckBox *slow_diaphragm_valve_ckb = 0, *fast_diaphragm_valve_ckb = 0, *ultrahigh_vacuum_baffle_valve_ckb = 0, *inserting_plate_value_ckb = 0, *angle_valve_ckb = 0, *door_lock_valve_ckb = 0;
		QCheckBox *pid_ckb = 0;
	};

	QTMCavitySubsystemWidgetPrivate::QTMCavitySubsystemWidgetPrivate(
		QTMCavitySubsystemWidget* p)
		:q_ptr(p){

	}






	/**
	* QTMCavitySubsystemWidget
	**/
	QTMCavitySubsystemWidget::QTMCavitySubsystemWidget(
		const std::shared_ptr<FortrendTMCavitySubsystem>& robot,
		QWidget*parent)
		: QAbstractSubsystemWidget<FortrendTMCavitySubsystem>(robot, parent)
		, d_ptr(new QTMCavitySubsystemWidgetPrivate(this)){

		Q_D(QTMCavitySubsystemWidget);
		d->ui = new Ui::FortrendTMCavitySubsystem;
		d->ui->setupUi(this);

		init();

		onAttributeUpdate();

		d->ultrahigh_vacuum_baffle_valve_ckb->setVisible(false);
		d->inserting_plate_value_ckb->setVisible(false);
		d->ui->open_height_vacuum_baffle_valve_btn->setVisible(false);
		d->ui->close_height_vacuum_baffle_valve_btn->setVisible(false);
		d->ui->open_inserting_plate_valve_btn->setVisible(false);
		d->ui->close_inserting_plate_valve_btn->setVisible(false);
		d->ui->open_pid_btn->setVisible(false);
		d->ui->close_pid_btn->setVisible(false);
		d->pid_ckb->setVisible(false);

		connect(d->ui->reset_btn, &QAbstractButton::clicked, this, &QTMCavitySubsystemWidget::onReset);
		connect(d->ui->generate_btn, &QAbstractButton::clicked, this, &QTMCavitySubsystemWidget::onGetStatus);
		connect(d->ui->open_diaphragm_valve_btn, &QAbstractButton::clicked, this, &QTMCavitySubsystemWidget::onOpenDiaphragmValve);
		connect(d->ui->close_diaphragm_valve_btn, &QAbstractButton::clicked, this, &QTMCavitySubsystemWidget::onCloseDiaphragmValve);
		//connect(d->ui->open_height_vacuum_baffle_valve_btn, &QAbstractButton::clicked, this, &QTMCavitySubsystemWidget::onOpenHeightVacuumBaffleValve);
		//connect(d->ui->close_height_vacuum_baffle_valve_btn, &QAbstractButton::clicked, this, &QTMCavitySubsystemWidget::onCloseHeightVacuumBaffleValve);
		//connect(d->ui->open_inserting_plate_valve_btn, &QAbstractButton::clicked, this, &QTMCavitySubsystemWidget::onOpenInsertingPlateValve);
		//connect(d->ui->close_inserting_plate_valve_btn, &QAbstractButton::clicked, this, &QTMCavitySubsystemWidget::onCloseInsertingPlateValve);

		connect(d->ui->open_angle_valve_btn, &QAbstractButton::clicked, this, &QTMCavitySubsystemWidget::onOpenAngleValve);
		connect(d->ui->close_angle_valve_btn, &QAbstractButton::clicked, this, &QTMCavitySubsystemWidget::onCloseAngleValve);
		//connect(d->ui->open_pid_btn, &QAbstractButton::clicked, this, &QTMCavitySubsystemWidget::onOpenPID);
		//connect(d->ui->close_pid_btn, &QAbstractButton::clicked, this, &QTMCavitySubsystemWidget::onClosePID);

	}

	QTMCavitySubsystemWidget::~QTMCavitySubsystemWidget(){

	}


	void QTMCavitySubsystemWidget::init(){
		Q_D(QTMCavitySubsystemWidget);

		//subsystem status
		QKernelSubsystemStatusWidget* status_widget = new QKernelSubsystemStatusWidget(getSubsystem());
		d->ui->right_verticalLayout->insertWidget(0, status_widget);
		//connect
		connect(status_widget, &QKernelSubsystemStatusWidget::onStatusUpdate, this, [=](){
			onAttributeUpdate();
		});

		
		d->ultrahigh_vacuum_baffle_valve_ckb = new QCheckBox("高真空挡板阀");
		d->inserting_plate_value_ckb = new QCheckBox("插板阀");
		d->angle_valve_ckb = new QCheckBox("角阀");
		d->slow_diaphragm_valve_ckb = new QCheckBox("慢充隔膜阀");
		d->fast_diaphragm_valve_ckb = new QCheckBox("快充隔膜阀");
		d->pid_ckb = new QCheckBox("PID调节");
		d->door_lock_valve_ckb = new QCheckBox("腔盖门锁");
		
		d->ultrahigh_vacuum_baffle_valve_ckb->setObjectName("io_object");
		d->inserting_plate_value_ckb->setObjectName("io_object");
		d->angle_valve_ckb->setObjectName("io_object");
		d->slow_diaphragm_valve_ckb->setObjectName("io_object");
		d->fast_diaphragm_valve_ckb->setObjectName("io_object");
		d->pid_ckb->setObjectName("io_object");
		d->door_lock_valve_ckb->setObjectName("io_object");
		
		d->ultrahigh_vacuum_baffle_valve_ckb->setEnabled(false);
		d->inserting_plate_value_ckb->setEnabled(false);
		d->angle_valve_ckb->setEnabled(false);
		d->slow_diaphragm_valve_ckb->setEnabled(false);
		d->fast_diaphragm_valve_ckb->setEnabled(false);
		d->pid_ckb->setEnabled(false);
		d->door_lock_valve_ckb->setEnabled(false);

		d->ui->vacuum_pressure_gage_ckb->setObjectName("io_object");
		d->ui->vacuum_pressure_gage_ckb->setEnabled(false);

		
		d->ui->operation_state_gridLayout->addWidget(d->ultrahigh_vacuum_baffle_valve_ckb, 0, 0);
		d->ui->operation_state_gridLayout->addWidget(d->angle_valve_ckb, 0, 1);
		d->ui->operation_state_gridLayout->addWidget(d->inserting_plate_value_ckb, 0, 2);
		d->ui->operation_state_gridLayout->addWidget(d->slow_diaphragm_valve_ckb, 1, 0);
		d->ui->operation_state_gridLayout->addWidget(d->fast_diaphragm_valve_ckb, 1, 1);
		d->ui->operation_state_gridLayout->addWidget(d->pid_ckb, 1, 2);
		d->ui->operation_state_gridLayout->addWidget(d->door_lock_valve_ckb, 2, 0);

	}

	void QTMCavitySubsystemWidget::onReset(){
		Q_D(QTMCavitySubsystemWidget);
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createResetCommand();
		executeCommand(getSubsystem(), cmd);
	}


	void QTMCavitySubsystemWidget::onGetStatus(){
		Q_D(QTMCavitySubsystemWidget);
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createUpdateCommand();
		executeCommand(getSubsystem(), cmd);
	}

	void QTMCavitySubsystemWidget::onOpenDiaphragmValve(){
		Q_D(QTMCavitySubsystemWidget);

		TMCavityValveOpening select = TMCavityValveOpening::TMCavity_Slow;
		if (d->ui->slow_charging_rbt->isChecked())
		{
			select = TMCavityValveOpening::TMCavity_Slow;
		}
		else if (d->ui->fast_charging_rbt->isChecked())
		{
			select = TMCavityValveOpening::TMCavity_Fast;
		}
		else{
			return;
		}
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createOpenDiaphragmValveCommand(select);
		executeCommand(getSubsystem(), cmd);
	}

	void QTMCavitySubsystemWidget::onCloseDiaphragmValve(){
		Q_D(QTMCavitySubsystemWidget);
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createCloseDiaphragmValveCommand(TMCavityValveOpening::TMCavity_Both);
		executeCommand(getSubsystem(), cmd);
	}

	void QTMCavitySubsystemWidget::onOpenHeightVacuumBaffleValve(){
		Q_D(QTMCavitySubsystemWidget);
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createOpenHeightVacuumBaffleValveCommand();
		executeCommand(getSubsystem(), cmd);
	}

	void QTMCavitySubsystemWidget::onCloseHeightVacuumBaffleValve(){
		Q_D(QTMCavitySubsystemWidget);
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createCloseHeightVacuumBaffleValveCommand();
		executeCommand(getSubsystem(), cmd);
	}
	void QTMCavitySubsystemWidget::onOpenInsertingPlateValve(){
		Q_D(QTMCavitySubsystemWidget);
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createOpenInsertingPlateValveCommand();
		executeCommand(getSubsystem(), cmd);
	}

	void QTMCavitySubsystemWidget::onCloseInsertingPlateValve(){
		Q_D(QTMCavitySubsystemWidget);
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createCloseInsertingPlateValveCommand();
		executeCommand(getSubsystem(), cmd);
	}


	void QTMCavitySubsystemWidget::onOpenAngleValve(){
		Q_D(QTMCavitySubsystemWidget);
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createOpenAngleValveCommand();
		executeCommand(getSubsystem(), cmd);
	}

	void QTMCavitySubsystemWidget::onCloseAngleValve(){
		Q_D(QTMCavitySubsystemWidget);
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createCloseAngleValveCommand();
		executeCommand(getSubsystem(), cmd);
	}

	

	void QTMCavitySubsystemWidget::onOpenFlowmeterDiaphragmValve(){
		Q_D(QTMCavitySubsystemWidget);
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createOpenFlowmeterDiaphragmValveCommand();
		executeCommand(getSubsystem(), cmd);
	}

	void QTMCavitySubsystemWidget::onCloseFlowmeterDiaphragmValve(){
		Q_D(QTMCavitySubsystemWidget);
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createCloseFlowmeterDiaphragmValveCommand();
		executeCommand(getSubsystem(), cmd);
	}

	void QTMCavitySubsystemWidget::onOpenPID(){
		Q_D(QTMCavitySubsystemWidget);
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createOpenPIDCommand();
		executeCommand(getSubsystem(), cmd);
	}
	void QTMCavitySubsystemWidget::onClosePID(){
		Q_D(QTMCavitySubsystemWidget);
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createClosePIDCommand();
		executeCommand(getSubsystem(), cmd);
	}


	void QTMCavitySubsystemWidget::onAttributeUpdate()throw(KernelException){
		Q_D(QTMCavitySubsystemWidget);
		try{

			//update input
			for (int i = 0; i < d->input_checkboxs.size(); i++){
				d->input_checkboxs.at(i)->setChecked(getSubsystem()->getInput(i));

			}

			//update object stat
			/*for (int i = 0; i < d->arm_stat.size(); i++){
			d->arm_stat.at(i)->setChecked(getSubsystem()->hasObject(i));
			}*/

			d->slow_diaphragm_valve_ckb->setChecked(getSubsystem()->getSlowDiaphragmValveOpend());
			d->fast_diaphragm_valve_ckb->setChecked(getSubsystem()->getFastDiaphragmValveOpend());
			d->ultrahigh_vacuum_baffle_valve_ckb->setChecked(getSubsystem()->getHeightVacuumBaffleValveOpend());
			d->inserting_plate_value_ckb->setChecked(getSubsystem()->getInsertingPlateValveOpend());
			d->angle_valve_ckb->setChecked(getSubsystem()->getAngleValveOpend());
			d->pid_ckb->setChecked(getSubsystem()->getPIDOpend());

			//QString str_vacuum = QString("%1Pa").arg(QString::number(getSubsystem()->getVacuumValue(), 'f', 6));
			//QString str_vacuum = QString::number(getSubsystem()->getVacuumValue(), 'e', 3).replace("e", "x10^(").append(")");

			d->door_lock_valve_ckb->setChecked(getSubsystem()->TMCavityCoverSafetyLock());

			d->ui->tm_cavity_current_vacuum_value_let->setText(QString::number(getSubsystem()->getTMCavityVacuumValue(), 'e', 3).append("Pa"));
			d->ui->molecule_pipeline_value_let->setText(QString::number(getSubsystem()->getMoleculePipelineVacuumValue(), 'e', 3).append("Pa"));
			d->ui->backing_pipeline_value_let->setText(QString::number(getSubsystem()->getBackingPipelineVacuumValue()).append("Pa"));
			d->ui->vacuum_pressure_gage_ckb->setChecked(getSubsystem()->getTMCavityVacuumPressureGageState() == 1);
			//backing_pipeline_value_let

		}
		catch (KernelException& e){
			logError(getSubsystem()->getName().c_str(), e.what());
			//throw e;
		}
		
	}

}
