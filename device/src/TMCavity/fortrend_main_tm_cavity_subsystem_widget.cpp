
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

#include  "device/ui_fortrend_main_tm_cavity_subsystem_widget.h"
#include  "TMCavity/fortrend_main_tm_cavity_subsystem_widget.h"
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
	* QMainTMCavitySubsystemWidgetPrivate
	**/
	class QMainTMCavitySubsystemWidgetPrivate{
		Q_DECLARE_PUBLIC(QMainTMCavitySubsystemWidget)
	public:
		QMainTMCavitySubsystemWidgetPrivate(QMainTMCavitySubsystemWidget* p, const std::shared_ptr<FortrendAlignerSubsystem>& aligner);
	public:
		Ui::FortrendMainTMCavitySubsystem* ui;
		QMainTMCavitySubsystemWidget* q_ptr;
		std::shared_ptr<FortrendAlignerSubsystem> aligner;
		//std::vector<QCheckBox*> arm_stat;
		std::vector<QCheckBox*> input_checkboxs;

		QCheckBox *diaphragm_valve_ckb = 0, *ultrahigh_vacuum_baffle_valve_ckb = 0, *angle_valve_ckb = 0, *inserting_plate_value_ckb = 0 ,*door_lock_valve_ckb=0;

		QRadioButton *diaphragm_only1_rbt = 0, *diaphragm_only2_rbt = 0;
	};

	QMainTMCavitySubsystemWidgetPrivate::QMainTMCavitySubsystemWidgetPrivate(
		QMainTMCavitySubsystemWidget* p, const std::shared_ptr<FortrendAlignerSubsystem>& aligner)
		:q_ptr(p), aligner(aligner){

	}






	/**
	* QMainTMCavitySubsystemWidget
	**/
	QMainTMCavitySubsystemWidget::QMainTMCavitySubsystemWidget(
		const std::shared_ptr<FortrendTMCavitySubsystem>& robot, 
		const std::shared_ptr<FortrendAlignerSubsystem>& aligner,
		QWidget*parent)
		: QAbstractSubsystemWidget<FortrendTMCavitySubsystem>(robot, parent)
		, d_ptr(new QMainTMCavitySubsystemWidgetPrivate(this, aligner)){

		Q_D(QMainTMCavitySubsystemWidget);
		d->ui = new Ui::FortrendMainTMCavitySubsystem;
		d->ui->setupUi(this);
	
	/*	d->ui->open_flowmeter_diaphragm_valve_btn->setVisible(false);
		d->ui->close_flowmeter_diaphragm_valve_btn->setVisible(false);*/
		init();

		onAttributeUpdate();
		d->ui->states_group_2->hide();

	}

	QMainTMCavitySubsystemWidget::~QMainTMCavitySubsystemWidget(){

	}


	void QMainTMCavitySubsystemWidget::init(){
		Q_D(QMainTMCavitySubsystemWidget);

		//QKernelSubsystemStatusWidget* status_widget_aligner = new QKernelSubsystemStatusWidget(d->aligner);
		//QLabel *label_name_aligner = new QLabel();
		//QFont font = QFont();
		//font.setFamily("Arial");  //# 设置字体类型
		//font.setPointSize(14);   // # 设置字体大小
		//font.setBold(true);      //# 设置字体加粗
		////font.setItalic(true);    // # 设置字体斜体
		//label_name_aligner->setFont(font);    // # 应用字体样式到QLabel
		//label_name_aligner->setAlignment(Qt::AlignCenter);
		//label_name_aligner->setText(QString::fromStdString(d->aligner->getName()));
		//d->ui->status_layout_aligner->insertWidget(0, label_name_aligner);
		//d->ui->status_layout_aligner->insertWidget(1, status_widget_aligner);
		////connect
		//connect(status_widget_aligner, &QKernelSubsystemStatusWidget::onStatusUpdate, this, [=](){
		//	onAttributeUpdate();
		//});

		QKernelSubsystemStatusWidget* status_widget = new QKernelSubsystemStatusWidget(getSubsystem());
		QLabel *label_name = new QLabel();
		QFont font2 = QFont();
		font2.setFamily("Arial");  //# 设置字体类型
		font2.setPointSize(14);   // # 设置字体大小
		font2.setBold(true);      //# 设置字体加粗
		//font.setItalic(true);    // # 设置字体斜体
		label_name->setFont(font2);    // # 应用字体样式到QLabel

		label_name->setAlignment(Qt::AlignCenter);
		label_name->setText(QString::fromStdString(getSubsystem()->getName()));
		d->ui->status_layout->insertWidget(0, label_name);
		d->ui->status_layout->insertWidget(1, status_widget);

		//connect
		connect(status_widget, &QKernelSubsystemStatusWidget::onStatusUpdate, this, [=](){
			onAttributeUpdate();
		});

		//add virtual input
		for (int i = 0; i < getSubsystem()->inputCount(); i++){
			QCheckBox* button = new QCheckBox(QString::fromStdString(getSubsystem()->getInputName(i)));
			button->setObjectName("io_object");
			button->setEnabled(false);
			d->ui->inputs_layout->addWidget(button, i / 4, i % 4, 1, 1);
			d->input_checkboxs.push_back(button);
		}


		d->diaphragm_valve_ckb = new QCheckBox("隔膜阀");
		d->ultrahigh_vacuum_baffle_valve_ckb = new QCheckBox("高真空挡板阀");
		d->angle_valve_ckb = new QCheckBox("角阀");
		d->inserting_plate_value_ckb = new QCheckBox("插板阀");

		d->door_lock_valve_ckb = new QCheckBox("腔盖门锁");

		d->diaphragm_valve_ckb->setObjectName("io_object");
		d->ultrahigh_vacuum_baffle_valve_ckb->setObjectName("io_object");
		d->angle_valve_ckb->setObjectName("io_object");
		d->inserting_plate_value_ckb->setObjectName("io_object");
		d->door_lock_valve_ckb->setObjectName("io_object");

		d->diaphragm_valve_ckb->setEnabled(false);
		d->ultrahigh_vacuum_baffle_valve_ckb->setEnabled(false);
		d->angle_valve_ckb->setEnabled(false);
		d->inserting_plate_value_ckb->setEnabled(false);
		d->door_lock_valve_ckb->setEnabled(false);


	/*	d->ui->vacuum_pressure_gage_ckb->setObjectName("io_object");
		d->ui->vacuum_pressure_gage_ckb->setEnabled(false);*/

		d->ui->operation_state_gridLayout->addWidget(d->diaphragm_valve_ckb, 0, 0);
		d->ui->operation_state_gridLayout->addWidget(d->ultrahigh_vacuum_baffle_valve_ckb, 0, 1);
		d->ui->operation_state_gridLayout->addWidget(d->angle_valve_ckb, 0, 2);
		d->ui->operation_state_gridLayout->addWidget(d->inserting_plate_value_ckb, 0, 3);
		d->ui->operation_state_gridLayout->addWidget(d->door_lock_valve_ckb, 1, 0);

		d->diaphragm_only1_rbt = new QRadioButton;
		d->diaphragm_only1_rbt->setChecked(true);
		d->diaphragm_only1_rbt->setText(QString::fromStdString("慢充"));
		d->diaphragm_only1_rbt->setProperty("index", 0); //index
		d->ui->diaphragm_selected_gridLayout->addWidget(d->diaphragm_only1_rbt, 0, 0);

		d->diaphragm_only2_rbt = new QRadioButton;
		d->diaphragm_only2_rbt->setText(QString::fromStdString("快充"));
		d->diaphragm_only2_rbt->setProperty("index", 1); //index
		d->ui->diaphragm_selected_gridLayout->addWidget(d->diaphragm_only2_rbt, 0, 1);


	}

	void QMainTMCavitySubsystemWidget::onReset(){
		Q_D(QMainTMCavitySubsystemWidget);
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createResetCommand();
		executeCommand(getSubsystem(), cmd);
	}


	void QMainTMCavitySubsystemWidget::onGetStatus(){
		Q_D(QMainTMCavitySubsystemWidget);
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createUpdateCommand();
		executeCommand(getSubsystem(), cmd);
	}

	void QMainTMCavitySubsystemWidget::onOpenDiaphragmValve(){
		Q_D(QMainTMCavitySubsystemWidget);

		TMCavityValveOpening select = TMCavityValveOpening::TMCavity_Fast;
		if (d->diaphragm_only1_rbt->isChecked())
		{
			select = TMCavityValveOpening::TMCavity_Fast;
		}
		else if (d->diaphragm_only2_rbt->isChecked())
		{
			select = TMCavityValveOpening::TMCavity_Fast;
		}
		else{
			return;
		}
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createOpenDiaphragmValveCommand(select);
		executeCommand(getSubsystem(), cmd);
	}

	void QMainTMCavitySubsystemWidget::onCloseDiaphragmValve(){
		Q_D(QMainTMCavitySubsystemWidget);
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createCloseDiaphragmValveCommand(TMCavityValveOpening::TMCavity_Both);
		executeCommand(getSubsystem(), cmd);
	}

	void QMainTMCavitySubsystemWidget::onOpenHeightVacuumBaffleValve(){
		Q_D(QMainTMCavitySubsystemWidget);
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createOpenHeightVacuumBaffleValveCommand();
		executeCommand(getSubsystem(), cmd);
	}

	void QMainTMCavitySubsystemWidget::onCloseHeightVacuumBaffleValve(){
		Q_D(QMainTMCavitySubsystemWidget);
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createCloseHeightVacuumBaffleValveCommand();
		executeCommand(getSubsystem(), cmd);
	}
	void QMainTMCavitySubsystemWidget::onOpenAngleValve(){
		Q_D(QMainTMCavitySubsystemWidget);
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createOpenAngleValveCommand();
		executeCommand(getSubsystem(), cmd);
	}

	void QMainTMCavitySubsystemWidget::onCloseAngleValve(){
		Q_D(QMainTMCavitySubsystemWidget);
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createCloseAngleValveCommand();
		executeCommand(getSubsystem(), cmd);
	}

	void QMainTMCavitySubsystemWidget::onOpenInsertingPlateValve(){
		Q_D(QMainTMCavitySubsystemWidget);
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createOpenInsertingPlateValveCommand();
		executeCommand(getSubsystem(), cmd);
	}

	void QMainTMCavitySubsystemWidget::onCloseInsertingPlateValve(){
		Q_D(QMainTMCavitySubsystemWidget);
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createCloseInsertingPlateValveCommand();
		executeCommand(getSubsystem(), cmd);
	}

	void QMainTMCavitySubsystemWidget::onOpenFlowmeterDiaphragmValve(){
		Q_D(QMainTMCavitySubsystemWidget);
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createOpenFlowmeterDiaphragmValveCommand();
		executeCommand(getSubsystem(), cmd);
	}

	void QMainTMCavitySubsystemWidget::onCloseFlowmeterDiaphragmValve(){
		Q_D(QMainTMCavitySubsystemWidget);
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createCloseFlowmeterDiaphragmValveCommand();
		executeCommand(getSubsystem(), cmd);
	}


	void QMainTMCavitySubsystemWidget::onAttributeUpdate(){
		Q_D(QMainTMCavitySubsystemWidget);
		//update input
		for (int i = 0; i < d->input_checkboxs.size(); i++){
			d->input_checkboxs.at(i)->setChecked(getSubsystem()->getInput(i));
		}

		//update object stat
		/*for (int i = 0; i < d->arm_stat.size(); i++){
		d->arm_stat.at(i)->setChecked(getSubsystem()->hasObject(i));
		}*/

		//d->diaphragm_valve_ckb->setChecked(getSubsystem()->getDiaphragmValveOpend());
		d->ultrahigh_vacuum_baffle_valve_ckb->setChecked(getSubsystem()->getHeightVacuumBaffleValveOpend());
		d->angle_valve_ckb->setChecked(getSubsystem()->getAngleValveOpend());
		d->inserting_plate_value_ckb->setChecked(getSubsystem()->getInsertingPlateValveOpend());

		d->door_lock_valve_ckb->setChecked(getSubsystem()->TMCavityCoverSafetyLock());

		d->ui->tm_cavity_current_vacuum_value_let->setText(QString::number(getSubsystem()->getTMCavityVacuumValue(), 'e', 3).append("Pa"));
		d->ui->molecule_pipeline_value_let->setText(QString::number(getSubsystem()->getMoleculePipelineVacuumValue(), 'e', 3).append("Pa"));

		//bool res = false;
		//int length = 0;
		//int angle = 0;
		//d->aligner->getAlignResult(res, length, angle);
		//if (res)
		//{
		//	d->ui->align_result_let->setStyleSheet("background-color: lime;");
		//	d->ui->align_result_let->setText(QString::fromStdString("Sucess"));
		//}
		//else
		//{
		//	d->ui->align_result_let->setStyleSheet("background-color: red;");
		//	d->ui->align_result_let->setText(QString::fromStdString("Fail"));
		//}
		//d->ui->length_let->setText(QString::number(length));
		//d->ui->angle_let->setText(QString::number(angle));
	}

}
