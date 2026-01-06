
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
	QCheckBox* pm_cavity_motor_home_ckb,
		* pm_cavity_motor_running_ckb, 
		* pm_cavity_motor_forward_ckb = 0,
		* pm_cavity_raxis_motor_running_ckb,
		* pm_cavity_raxis_motor_home_ckb;
	QCheckBox* pm_min_plane_Level_ckb,
		* pm_max_plane_Level_ckb,
		* pm_rotating_plane_Level_ckb,
		* pm_liftting_alarm_ckb,
		* pm_rotating_alarm_ckb;

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
	connect(d->ui->set_speed_btn_r, &QAbstractButton::clicked, this, &QMainPMCavitySubsystemWidget::onSetRAxisSpeed);
	connect(d->ui->set_crft_count, &QAbstractButton::clicked, this, &QMainPMCavitySubsystemWidget::onSetCrft);
	connect(d->ui->forward_btn, &QPushButton::pressed, this, &QMainPMCavitySubsystemWidget::onStartForward);
	connect(d->ui->forward_btn, &QPushButton::released, this, &QMainPMCavitySubsystemWidget::onStopForward);
	connect(d->ui->backward_btn, &QPushButton::pressed, this, &QMainPMCavitySubsystemWidget::onStartBackward);
	connect(d->ui->backward_btn, &QPushButton::released, this, &QMainPMCavitySubsystemWidget::onStopBackward);
	connect(d->ui->positive_btn, &QPushButton::pressed, this, &QMainPMCavitySubsystemWidget::onStartPositive);
	connect(d->ui->positive_btn, &QPushButton::released, this, &QMainPMCavitySubsystemWidget::onStopPositive);
	connect(d->ui->negative_btn, &QPushButton::pressed, this, &QMainPMCavitySubsystemWidget::onStartNegative);
	connect(d->ui->negative_btn, &QPushButton::released, this, &QMainPMCavitySubsystemWidget::onStopNegative);
	connect(d->ui->set_turn_speed_btn, &QAbstractButton::clicked, this, &QMainPMCavitySubsystemWidget::onSetTurnSpeed);
	connect(d->ui->set_r_axis_turn_speed_btn, &QAbstractButton::clicked, this, &QMainPMCavitySubsystemWidget::onSetRAxisTurnSpeed);
	loadConfigFile();


	d->tm_cavity_door_ckb = new QCheckBox(QString("取放料信号"));
	d->tm_cavity_door_ckb->setObjectName("io_object");
	d->tm_cavity_door_ckb->setEnabled(false);

	//d->pm_cavity_motor_forward_ckb = new QCheckBox(QString("电机前进到位信号"));
	//d->pm_cavity_motor_forward_ckb->setObjectName("io_object");
	//d->pm_cavity_motor_forward_ckb->setEnabled(false);

	d->pm_cavity_motor_home_ckb = new QCheckBox(QString("Z轴回原到位信号"));
	d->pm_cavity_motor_home_ckb->setObjectName("io_object");
	d->pm_cavity_motor_home_ckb->setEnabled(false);

	d->pm_cavity_motor_running_ckb = new QCheckBox(QString("Z轴运动中"));
	d->pm_cavity_motor_running_ckb->setObjectName("io_object");
	d->pm_cavity_motor_running_ckb->setEnabled(false);

	d->pm_cavity_raxis_motor_home_ckb = new QCheckBox(QString("R轴回原到位信号"));
	d->pm_cavity_raxis_motor_home_ckb->setObjectName("io_object");
	d->pm_cavity_raxis_motor_home_ckb->setEnabled(false);

	d->pm_cavity_raxis_motor_running_ckb = new QCheckBox(QString("R轴运动中"));
	d->pm_cavity_raxis_motor_running_ckb->setObjectName("io_object");
	d->pm_cavity_raxis_motor_running_ckb->setEnabled(false);


	d->pm_min_plane_Level_ckb = new QCheckBox(QString("最低面位检测信号"));
	d->pm_min_plane_Level_ckb->setObjectName("io_object");
	d->pm_min_plane_Level_ckb->setEnabled(false);

	d->pm_max_plane_Level_ckb = new QCheckBox(QString("最高面位检测信号"));
	d->pm_max_plane_Level_ckb->setObjectName("io_object");
	d->pm_max_plane_Level_ckb->setEnabled(false);

	d->pm_rotating_plane_Level_ckb = new QCheckBox(QString("中间旋转面位检测信号"));
	d->pm_rotating_plane_Level_ckb->setObjectName("io_object");
	d->pm_rotating_plane_Level_ckb->setEnabled(false);

	d->pm_liftting_alarm_ckb = new QCheckBox(QString("Z轴报警"));
	d->pm_liftting_alarm_ckb->setObjectName("io_object");
	d->pm_liftting_alarm_ckb->setEnabled(false);

	d->pm_rotating_alarm_ckb = new QCheckBox(QString("R轴报警"));
	d->pm_rotating_alarm_ckb->setObjectName("io_object");
	d->pm_rotating_alarm_ckb->setEnabled(false);

	d->ui->operation_state_gridLayout->addWidget(d->tm_cavity_door_ckb, 0, 0);
	d->ui->operation_state_gridLayout->addWidget(d->pm_cavity_motor_home_ckb, 0, 1);
	//d->ui->operation_state_gridLayout->addWidget(d->pm_cavity_motor_forward_ckb, 1, 0);
	d->ui->operation_state_gridLayout->addWidget(d->pm_cavity_motor_running_ckb, 1, 0);
	d->ui->operation_state_gridLayout->addWidget(d->pm_cavity_raxis_motor_running_ckb, 1, 1);
	d->ui->operation_state_gridLayout->addWidget(d->pm_cavity_raxis_motor_home_ckb, 1, 2);
	d->ui->operation_state_gridLayout->addWidget(d->pm_liftting_alarm_ckb,1,3);

	d->ui->operation_state_gridLayout->addWidget(d->pm_min_plane_Level_ckb, 2, 0);
	d->ui->operation_state_gridLayout->addWidget(d->pm_max_plane_Level_ckb, 2, 1);
	d->ui->operation_state_gridLayout->addWidget(d->pm_rotating_plane_Level_ckb, 2, 2);
	d->ui->operation_state_gridLayout->addWidget(d->pm_rotating_alarm_ckb, 2, 3);

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
void QMainPMCavitySubsystemWidget::onSetRAxisSpeed()
{
	Q_D(QMainPMCavitySubsystemWidget);

	getSubsystem()->setPMCavityRAxleSpeed(d->ui->speed_value_spx_r->value());
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

void QMainPMCavitySubsystemWidget::onSetRAxisTurnSpeed()
{
	Q_D(QMainPMCavitySubsystemWidget);
	getSubsystem()->setsetPMCavityRAxleTurnSpeed(d->ui->speed_value_r_axis_turn->value());
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

void QMainPMCavitySubsystemWidget::onStartPositive()
{
	Q_D(QMainPMCavitySubsystemWidget);
	getSubsystem()->setPMCavityRAxlePositive(true);
}

void QMainPMCavitySubsystemWidget::onStopPositive()
{
	Q_D(QMainPMCavitySubsystemWidget);
	getSubsystem()->setPMCavityRAxlePositive(false);
}

void QMainPMCavitySubsystemWidget::onStartNegative()
{
	Q_D(QMainPMCavitySubsystemWidget);
	getSubsystem()->setPMCavityRAxleNegative(true);
}

void QMainPMCavitySubsystemWidget::onStopNegative()
{
	Q_D(QMainPMCavitySubsystemWidget);
	getSubsystem()->setPMCavityRAxleNegative(false);
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

	d->ui->speed_realTime_speed_z->setText(QString::number(getSubsystem()->getPMCavityZAxleSpeed()));
	d->ui->axis_location->setText(QString::number(getSubsystem()->getPMCavityZAxleLocation()));

	d->ui->axis_r_location->setText(QString::number(getSubsystem()->getPMCavityRAxleLocation()));
	d->ui->speed_realTime_speed_r->setText(QString::number(getSubsystem()->getPMCavityRAxleSpeed()));

	//update door state
	d->ui->speed_realTime_Jog_speed_z->setText(QString::number(getSubsystem()->getPMCavityMotorSpeed()));
	d->ui->speed_realTime_Jog_speed_r->setText(QString::number(getSubsystem()->getPMCavityRAxisMotorSpeed()));

	d->tm_cavity_door_ckb->setChecked(getSubsystem()->getPMCavitySafeSignal());

	d->pm_cavity_motor_home_ckb->setChecked(getSubsystem()->getLiftingHomeDone());
	d->pm_min_plane_Level_ckb->setChecked(getSubsystem()->getMinimumPlaneLevelSignal());
	d->pm_max_plane_Level_ckb->setChecked(getSubsystem()->getMaximumPlaneLevelSignal());
	d->pm_rotating_plane_Level_ckb->setChecked(getSubsystem()->getRotatingimumPlaneLevelSignal());
	d->pm_cavity_motor_running_ckb->setChecked(getSubsystem()->getZAxleJogRunning());

	d->pm_cavity_raxis_motor_home_ckb->setChecked(getSubsystem()->getRotationHomeDone());
	d->pm_cavity_raxis_motor_running_ckb->setChecked(getSubsystem()->getRAxleJogRunning());

	d->pm_liftting_alarm_ckb->setChecked(getSubsystem()->getZAxisAlarm());
	d->pm_rotating_alarm_ckb->setChecked(getSubsystem()->getRAxisAlarm());
	
}

void QMainPMCavitySubsystemWidget::loadConfigFile(){
	Q_D(QMainPMCavitySubsystemWidget);
	QString fileName = QDir::currentPath() + "/config/" + "config_pm.ini";
	if (fileName.isEmpty())
		return;
	QSettings settings(fileName, QSettings::IniFormat);
	int count = 0;
	std::string pmName = getSubsystem()->getName();
	if(pmName == "PM1")
	{
		count = settings.value("CrftPM1Count", 1).toInt();
	}
	else if (pmName == "PM2")
	{
		count = settings.value("CrftPM2Count", 1).toInt();
	}
	else if (pmName == "PM3")
	{
		count = settings.value("CrftPM3Count", 1).toInt();
	}
	else 
	{
		count = settings.value("CrftPM4Count", 1).toInt();
	}


	d->ui->crft_count_pm->setValue(count);
	getSubsystem()->setPMCavityCraftCount(count);

	
}

void QMainPMCavitySubsystemWidget::saveConfigFile(){
	Q_D(QMainPMCavitySubsystemWidget);
	logInform("Test", "value1 %d ", d->ui->crft_count_pm->value());

	getSubsystem()->setPMCavityCraftCount(d->ui->crft_count_pm->value());

	QString fileName = QDir::currentPath() + "/config/" + "config_pm.ini";
	if (fileName.isEmpty())
		return;
	QSettings settings(fileName, QSettings::IniFormat);

	 
	std::string _key = "Crft" + getSubsystem()->getName() + "Count";//CrftPM1Count
	const QString& key = QString::fromStdString(_key);
	settings.setValue(key, d->ui->crft_count_pm->value());

}

}
