
/**
* @file            fortrend_pm_cavity_subsystem.h
* @brief           Fortrend PMCavity widget
* @author		   xielonghua
*/

// Library: Fortrend
// Package: SubSystem//PMCavity

#include "fortrend_station_status_vtm_widget.h"
#include  "PMCavity/fortrend_main_pm_cavity_subsystem_widget.h"
#include  "Kernel/kernel.h"
#include  "Kernel/kernel_subsystem_reset_command.h"
#include  "Kernel/kernel_subsystem_update_command.h"
#include  "Kernel/CoreUI/kernel_subsystem_status_widget.h"  
#include  "Kernel/FortrendUI/slot_widget.h"
#include  "Kernel/FortrendUI/cassette_widget.h"
#include  "Kernel/Fortrend/robot_wafer_abstract_subsystem.h"
#include  "Kernel/Fortrend/fortrend_cassette_manager.h"
#include  "Kernel/Fortrend/cassette.h"

#include  "device/ui_fortrend_main_pm_cavity_subsystem_widget.h"
#include  "PMCavity/fortrend_pm_cavity_defined.h"

#include <QRadioButton>
#include <QSpacerItem>
#include <QMessageBox>
#include <QCheckBox>
#include <QDir>
#include <QSettings>

#if _MSC_VER >1600
#pragma execution_character_set("utf-8")
#endif

 
namespace FC{
/**
* QMainPMCavitySubsystemWidgetPrivate
**/
class QMainPMCavitySubsystemWidgetPrivate{
	Q_DECLARE_PUBLIC(QMainPMCavitySubsystemWidget)
public:
	QMainPMCavitySubsystemWidgetPrivate(QMainPMCavitySubsystemWidget* p);
public:
	Ui::FortrendMainPMCavitySubsystem* ui;
	QMainPMCavitySubsystemWidget* q_ptr;
	//QFortrendStationStatusVTMWidget * s_ptr;
	//std::vector<QCheckBox*> arm_stat;
	std::vector<QCheckBox*> input_checkboxs;
	QCheckBox* tm_cavity_door_ckb = 0;
	QCheckBox* pm_cavity_motor_home_ckb, *pm_cavity_motor_running_ckb, *pm_cavity_motor_forward_ckb = 0;
	
};

QMainPMCavitySubsystemWidgetPrivate::QMainPMCavitySubsystemWidgetPrivate(
	QMainPMCavitySubsystemWidget* p)
:q_ptr(p){

}






/**
* QMainPMCavitySubsystemWidget
**/
QMainPMCavitySubsystemWidget::QMainPMCavitySubsystemWidget(
	const std::shared_ptr<FortrendPMCavitySubsystem>& robot,
	QWidget*parent) 
	: QAbstractSubsystemWidget<FortrendPMCavitySubsystem>(robot, parent)
	, d_ptr(new QMainPMCavitySubsystemWidgetPrivate(this)){
	Q_D(QMainPMCavitySubsystemWidget);
	d->ui = new Ui::FortrendMainPMCavitySubsystem;
	d->ui->setupUi(this);
	init();
	//d_ptr->s_ptr = static_cast<QFortrendStationStatusVTMWidget*>(parent);
	//onAttributeUpdate(); generate_state_group

}

QMainPMCavitySubsystemWidget::~QMainPMCavitySubsystemWidget(){

}


void QMainPMCavitySubsystemWidget::init(){
	Q_D(QMainPMCavitySubsystemWidget);

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
	connect(d->ui->set_speed_btn_z, &QAbstractButton::clicked, this, &QMainPMCavitySubsystemWidget::onSetSpeed);

	connect(d->ui->set_crft_count, &QAbstractButton::clicked, this, &QMainPMCavitySubsystemWidget::onSetCrft);
	connect(d->ui->forward_btn, &QPushButton::pressed, this, &QMainPMCavitySubsystemWidget::onStartForward);
	connect(d->ui->forward_btn, &QPushButton::released, this, &QMainPMCavitySubsystemWidget::onStopForward);
	connect(d->ui->backward_btn, &QPushButton::pressed, this, &QMainPMCavitySubsystemWidget::onStartBackward);
	connect(d->ui->backward_btn, &QPushButton::released, this, &QMainPMCavitySubsystemWidget::onStopBackward);
	connect(d->ui->set_turn_speed_btn, &QAbstractButton::clicked, this, &QMainPMCavitySubsystemWidget::onSetTurnSpeed);
	
	loadConfigFile();


	d->tm_cavity_door_ckb = new QCheckBox(QString("取放料信号"));

	d->tm_cavity_door_ckb->setObjectName("io_object");

	d->tm_cavity_door_ckb->setEnabled(false);

	d->pm_cavity_motor_forward_ckb = new QCheckBox(QString("电机前进到位信号"));
	d->pm_cavity_motor_forward_ckb->setObjectName("io_object");
	d->pm_cavity_motor_forward_ckb->setEnabled(false);

	d->pm_cavity_motor_home_ckb = new QCheckBox(QString("电机后退到位信号"));
	d->pm_cavity_motor_home_ckb->setObjectName("io_object");
	d->pm_cavity_motor_home_ckb->setEnabled(false);

	d->pm_cavity_motor_running_ckb = new QCheckBox(QString("电机运动中"));
	d->pm_cavity_motor_running_ckb->setObjectName("io_object");
	d->pm_cavity_motor_running_ckb->setEnabled(false);
	d->ui->operation_state_gridLayout->addWidget(d->tm_cavity_door_ckb, 0, 0);
	d->ui->operation_state_gridLayout->addWidget(d->pm_cavity_motor_home_ckb, 0, 1);
	d->ui->operation_state_gridLayout->addWidget(d->pm_cavity_motor_forward_ckb, 1, 0);
	d->ui->operation_state_gridLayout->addWidget(d->pm_cavity_motor_running_ckb, 1, 1);

	d->ui->label_50->hide();
}

void QMainPMCavitySubsystemWidget::onReset(){
	Q_D(QMainPMCavitySubsystemWidget);
	KernelSubsystemCommand::Ptr cmd = getSubsystem()->createResetCommand();
	executeCommand(getSubsystem(), cmd);
}


void QMainPMCavitySubsystemWidget::onGetStatus(){
	Q_D(QMainPMCavitySubsystemWidget);
	KernelSubsystemCommand::Ptr cmd = getSubsystem()->createUpdateCommand();
	executeCommand(getSubsystem(), cmd);
}


void QMainPMCavitySubsystemWidget::onOpenTMCavityDoor(){
	Q_D(QMainPMCavitySubsystemWidget);
	//TODO open box
	KernelSubsystemCommand::Ptr cmd = getSubsystem()->createOpenTMCavityDoorCommand();
	executeCommand(getSubsystem(), cmd);
	
}


void QMainPMCavitySubsystemWidget::onSetSpeed(){
	Q_D(QMainPMCavitySubsystemWidget);
	//TODO close box
	getSubsystem()->setPMCavityAxleSpeed(d->ui->speed_value_spx_z->value());
}
void QMainPMCavitySubsystemWidget::onSetCrft(){
	Q_D(QMainPMCavitySubsystemWidget);
	saveConfigFile();
}

void QMainPMCavitySubsystemWidget::onSetTurnSpeed(){
	Q_D(QMainPMCavitySubsystemWidget);
	//TODO close box
	getSubsystem()->setPMCavityTurnSpeed(d->ui->speed_value_turn->value());
}

void QMainPMCavitySubsystemWidget::onStartForward(){
	Q_D(QMainPMCavitySubsystemWidget);
	//TODO close box
	getSubsystem()->setPMCavityForward(true);
}

void QMainPMCavitySubsystemWidget::onStopForward(){
	Q_D(QMainPMCavitySubsystemWidget);
	//TODO close box
	getSubsystem()->setPMCavityForward(false);
}
void QMainPMCavitySubsystemWidget::onStartBackward(){
	Q_D(QMainPMCavitySubsystemWidget);
	//TODO close box
	getSubsystem()->setPMCavityBackward(true);
}
void QMainPMCavitySubsystemWidget::onStopBackward(){
	Q_D(QMainPMCavitySubsystemWidget);
	//TODO close box
	getSubsystem()->setPMCavityBackward(false);
}

void QMainPMCavitySubsystemWidget::onCloseTMCavityDoor(){
	Q_D(QMainPMCavitySubsystemWidget);
	//TODO close box
	KernelSubsystemCommand::Ptr cmd = getSubsystem()->createCloseTMCavityDoorCommand();
	executeCommand(getSubsystem(), cmd);
}

void QMainPMCavitySubsystemWidget::onGetFinished(){
	Q_D(QMainPMCavitySubsystemWidget);
	KernelSubsystemCommand::Ptr cmd = getSubsystem()->createGetFinishedCommand();
	executeCommand(getSubsystem(), cmd);
}
void  QMainPMCavitySubsystemWidget::onUplaodFinished(){
	Q_D(QMainPMCavitySubsystemWidget);
	KernelSubsystemCommand::Ptr cmd = getSubsystem()->createUploadFinishedCommand();
	executeCommand(getSubsystem(), cmd);
}

void QMainPMCavitySubsystemWidget::onReadProcessParameters(){
	Q_D(QMainPMCavitySubsystemWidget);
	KernelSubsystemCommand::Ptr cmd = getSubsystem()->createReadProcessParametersCommand();
	executeCommand(getSubsystem(), cmd);
}

void QMainPMCavitySubsystemWidget::onWriteProcessParameters(){
	Q_D(QMainPMCavitySubsystemWidget);

}

void QMainPMCavitySubsystemWidget::onAttributeUpdate(){
	Q_D(QMainPMCavitySubsystemWidget);
	d->ui->speed_value_spx_z->setValue(getSubsystem()->getPMCavityAxleSpeed());

	d->ui->axis_location->setText(QString::number(getSubsystem()->getPMCavityAxleLocation()));

	//d->s_ptr->setAxislocation(getSubsystem()->getPMCavityAxleLocation());
	//update door state
	d->tm_cavity_door_ckb->setChecked(getSubsystem()->getPMCavitySafeSignal());
	d->pm_cavity_motor_home_ckb->setChecked(getSubsystem()->getPMCavityMotorHomeSignal());
	d->pm_cavity_motor_forward_ckb->setChecked(getSubsystem()->getPMCavityMotorForwardSignal());
	d->ui->speed_value_turn->setValue(getSubsystem()->getPMCavityMotorSpeed());
	d->pm_cavity_motor_running_ckb->setChecked(getSubsystem()->getPMCavityMotorRunSignal());
	
}

void QMainPMCavitySubsystemWidget::loadConfigFile(){
	Q_D(QMainPMCavitySubsystemWidget);
	QString fileName = QDir::currentPath() + "/config/" + "config_pm.ini";
	if (fileName.isEmpty())
		return;
	QSettings settings(fileName, QSettings::IniFormat);
	int counta = settings.value("CrftCountLLA", 1).toInt();
	int countb = settings.value("CrftCountLLB", 1).toInt();

	d->ui->crft_count_lla->setValue(counta);
	d->ui->crft_count_llb->setValue(countb);

	getSubsystem()->setPMCavityCrftCountLLA(counta);
	getSubsystem()->setPMCavityCrftCountLLB(countb);
	
}

void QMainPMCavitySubsystemWidget::saveConfigFile(){
	Q_D(QMainPMCavitySubsystemWidget);
	logInform("Test", "value1 %d value2 %d", d->ui->crft_count_lla->value(), d->ui->crft_count_llb->value());

	getSubsystem()->setPMCavityCrftCountLLA(d->ui->crft_count_lla->value());
	getSubsystem()->setPMCavityCrftCountLLB(d->ui->crft_count_llb->value());
	QString fileName = QDir::currentPath() + "/config/" + "config_pm.ini";
	if (fileName.isEmpty())
		return;
	QSettings settings(fileName, QSettings::IniFormat);
	settings.setValue("CrftCountLLA", d->ui->crft_count_lla->value());
	settings.setValue("CrftCountLLB", d->ui->crft_count_llb->value());

}

}
