
/**
* @file    fortrend_tmCartoon_subsystem.h
* @brief   Fortrend tmCartoon widget
* @author  xielonghua
*/

// Library: Fortrend
// Package: SubSystem/TMCARTOON

#include  "Vacuumize/fortrend_vacuumize_subsystem_widget.h"
#include  "Kernel/CoreUI/kernel_subsystem_status_widget.h"  
#include  "Kernel/kernel.h"
#include  "Kernel/kernel_subsystem_reset_command.h"
#include  "Kernel/kernel_subsystem_update_command.h"
#include  "Kernel/Fortrend/abstract_output_command.h"
#include  "device/ui_fortrend_vacuumize_subsystem_widget.h"
#include "Kernel/kernel_log.h"
#include "Kernel/kernel_subsystem.h"


//#include "Kernel/kernel_event_module.h"
//#include "Kernel/kernel_event_paramters.h"

#include "Pump/fortrend_pump_subsystem.h" 
#include "Pump/fortrend_pump_open_loadlock1_auto_vacuum_command.h"
#include "Pump/fortrend_pump_open_loadlock2_auto_vacuum_command.h"
#include "Pump/fortrend_pump_open_tm_cavity_auto_vacuum_command.h"
#include "Pump/fortrend_pump_mechanical_close_command.h"
#include "Pump/fortrend_pump_mechanical_open_command.h"
#include "Pump/fortrend_pump_molecular_close_command.h"
#include "Pump/fortrend_pump_molecular_open_command.h"

#include "TMCavity/fortrend_tm_cavity_subsystem.h" 
#include "TMCavity/fortrend_tm_cavity_open_angle_valve_command.h"
#include "TMCavity/fortrend_tm_cavity_close_angle_valve_command.h"
#include "TMCavity/fortrend_tm_cavity_open_inserting_plate_valve_command.h"
#include "TMCavity/fortrend_tm_cavity_close_inserting_plate_valve_command.h"
#include "TMCavity/fortrend_tm_cavity_open_height_vacuum_baffle_valve_command.h"
#include "TMCavity/fortrend_tm_cavity_close_height_vacuum_baffle_valve_command.h"

#include "LoadLock/fortrend_loadlock_subsystem.h" 
#include "LoadLock/fortrend_loadlock_close_angle_valve_command.h"
#include "LoadLock/fortrend_loadlock_open_angle_valve_command.h"
#include "LoadLock/fortrend_loadlock_cavity_open_inserting_plate_valve_command.h"
#include "LoadLock/fortrend_loadlock_cavity_close_inserting_plate_valve_command.h"
#include "LoadLock/fortrend_loadlock_cavity_open_height_vacuum_baffle_valve_command.h"
#include "LoadLock/fortrend_loadlock_cavity_close_height_vacuum_baffle_valve_command.h"


#include "slidevalvewidget.h"
#include "anglevalvewidget.h"
#include "roughextractionvalve.h"

#include "valveWheel.h"
#include <QPainter>

#include <QRadialGradient>
#include <QColor>
#include <QMessageBox>
#include <QCheckBox>

#if _MSC_VER >1600
#pragma execution_character_set("utf-8")
#endif
 
namespace FC{
/**
* QVacuumizeSubsystemWidgetPrivate
**/
	class QVacuumizeSubsystemWidgetPrivate :public KernelListener<IKernelCommand> {//,  public FC::KernelEventListener
    Q_DECLARE_PUBLIC(QVacuumizeSubsystemWidget)
public:
	QVacuumizeSubsystemWidgetPrivate(QVacuumizeSubsystemWidget* p, const std::shared_ptr<IKernel>& kernel);

public:
    Ui::VacuumizeSubsystemWidget* ui;
	QVacuumizeSubsystemWidget* q_ptr;
	std::shared_ptr<IKernel> kernel;
	std::shared_ptr<FortrendLoadLockSubsystem> lk1;
	std::shared_ptr<FortrendLoadLockSubsystem> lk2;
	std::shared_ptr<FortrendTMCavitySubsystem> tm;
	std::shared_ptr<FortrendPumpSubsystem> pump;
	QCheckBox *widget_tm_tmp_ckb = 0, *widget_lla_tmp_ckb = 0, *widget_llb_tmp_ckb = 0, *widget_vp_ckb = 0, *widget_pav_tm_ckb = 0, *widget_pav_lla_ckb = 0;
	QCheckBox *widget_pav_llb_ckb = 0, *widget_ppv_tm_ckb = 0, *widget_ppv_lla_ckb = 0, *widget_ppv_llb_ckb = 0, *widget_pbv_tm_ckb = 0, *widget_pbv_lla_ckb = 0, *widget_pbv_llb_ckb = 0;
	virtual void onAttributeChange(const IKernelCommand* cmd){
		const KernelSubsystemCommand* subCmd = dynamic_cast<const KernelSubsystemCommand*>(cmd);
		if (subCmd && subCmd->hasError()){
			AlarmMessage::Ptr alarm = cmd->alarmMessage();
			QString msg = QString("%1 %2: \n[T=%3][C=0x%4]%5")
				.arg(subCmd->getSubsystem()->getName().c_str())
				.arg(cmd->getName().c_str())
				.arg(alarm->type())
				.arg(alarm->code(), 0, 16)
				.arg(alarm->message().c_str());

			QMetaObject::invokeMethod(q_ptr, "showMessage", Qt::AutoConnection,
				Q_ARG(QString, msg));
		}
	}

};

	QVacuumizeSubsystemWidgetPrivate::QVacuumizeSubsystemWidgetPrivate(QVacuumizeSubsystemWidget* p, const std::shared_ptr<IKernel>& kernel)
		:q_ptr(p), kernel(kernel){

		lk1 = kernel->getKernelModule<FortrendLoadLockSubsystem>("LLA");
		lk2 = kernel->getKernelModule<FortrendLoadLockSubsystem>("LLB");
		tm = kernel->getKernelModule<FortrendTMCavitySubsystem>("TM");
		pump = kernel->getKernelModule<FortrendPumpSubsystem>("PUMP");
}

/**
* QVacuumizeSubsystemWidget
**/
	QVacuumizeSubsystemWidget::QVacuumizeSubsystemWidget(const std::shared_ptr<IKernel>& kernel, QWidget*parent)
	:QWidget(parent),
	d_ptr(new QVacuumizeSubsystemWidgetPrivate(this, kernel))
{
    Q_D(QVacuumizeSubsystemWidget);
	//pump->addEventListener(d_ptr);
    d->ui = new Ui::VacuumizeSubsystemWidget;
    d->ui->setupUi(this);

	//d->ui->widget_18->angle = 90;//90度旋转
	d->ui->widget_pbv_llb->angle = 90;//90度旋转
	d->ui->widget_21->angle = 90;//90度旋转
	d->ui->widget_24->angle = 90;//90度旋转
	d->ui->widget_46->angle = 90;//90度旋转
	d->ui->widget_72->angle = 90;//90度旋转
	d->ui->widget_54->angle = 90;//90度旋转
    //init samethong
	//d->ui->widget_13->setWaterDirection(0);//设置从下往上流动
	//d->ui->widget_53->setWaterDirection(1);//设置从右往左流动

	QKernelSubsystemStatusWidget* status_widget_tm = new QKernelSubsystemStatusWidget(d->tm);

	connect(status_widget_tm, &QKernelSubsystemStatusWidget::onStatusUpdate, this, [=](){
		onAttributeUpdate();
	});

	QKernelSubsystemStatusWidget* status_widget_lla = new QKernelSubsystemStatusWidget(d->lk1);
	connect(status_widget_lla, &QKernelSubsystemStatusWidget::onStatusUpdate, this, [=](){
		onAttributeUpdate();
	});

	QKernelSubsystemStatusWidget* status_widget_llb = new QKernelSubsystemStatusWidget(d->lk2);
	connect(status_widget_llb, &QKernelSubsystemStatusWidget::onStatusUpdate, this, [=](){
		onAttributeUpdate();
	});

	QKernelSubsystemStatusWidget* status_widget_pump = new QKernelSubsystemStatusWidget(d->pump);
	connect(status_widget_pump, &QKernelSubsystemStatusWidget::onStatusUpdate, this, [=](){
		onAttributeUpdate();
	});

	d->ui->verticalLayout->insertWidget(0, status_widget_pump);

	//d->ui->widget_13->setWaterDirection(2);//设置从下往上流动
	//d->ui->widget_4->setWaterDirection(2);//设置从下往上流动

	
	d->ui->widget_tm_tmp->colorLine = QColor(153, 194, 255);
	d->ui->widget_lla_tmp->colorLine = QColor(153, 194, 255);
	d->ui->widget_llb_tmp->colorLine = QColor(153, 194, 255);
	d->ui->widget_vp->colorLine = QColor(153, 194, 255);
	
	//d->ui->widget_vp->open();
	//d->ui->widget_vp->isDisableClick(true);//禁用点击事件

	QRadialGradient gradient(width() / 2, height() / 2, width() / 2 * 1.5); // 中心点和半径
	gradient.setColorAt(0, Qt::darkBlue); // 起始颜色（中心）
	gradient.setColorAt(1, Qt::black); // 结束颜色（边缘）


	// 设置QPalette并应用渐变
	QPalette palette;
	palette.setBrush(backgroundRole(), gradient);
	setPalette(palette);

	// 设置窗口属性，使背景色可以自动填充整个窗口
	setAutoFillBackground(true);

	QObject::connect(d->ui->widget_tm_tmp, &ValveWheel::signalClicked, this, &QVacuumizeSubsystemWidget::onTMPTMClicked);//TM腔分子泵
	QObject::connect(d->ui->widget_lla_tmp, &ValveWheel::signalClicked, this, &QVacuumizeSubsystemWidget::onTMPLLAClicked);//LoadLockA腔分子泵
	QObject::connect(d->ui->widget_llb_tmp, &ValveWheel::signalClicked, this, &QVacuumizeSubsystemWidget::onTMPLLBClicked);//LoadLockB腔分子泵
	QObject::connect(d->ui->widget_vp, &ValveWheel::signalClicked, this, &QVacuumizeSubsystemWidget::onVPClicked);//干泵

	QObject::connect(d->ui->widget_pav_tm, &RoughExtractionValve::signalClicked, this, &QVacuumizeSubsystemWidget::onPAVTMClicked);//气动角阀
	QObject::connect(d->ui->widget_pav_lla, &RoughExtractionValve::signalClicked, this, &QVacuumizeSubsystemWidget::onPAVLLAClicked);//
	QObject::connect(d->ui->widget_pav_llb, &RoughExtractionValve::signalClicked, this, &QVacuumizeSubsystemWidget::onPAVLLBClicked);//
	
	QObject::connect(d->ui->widget_ppv_tm, &AngleValveWidget::signalClicked, this, &QVacuumizeSubsystemWidget::onPPVTMClicked);//插板阀
	QObject::connect(d->ui->widget_ppv_lla, &AngleValveWidget::signalClicked, this, &QVacuumizeSubsystemWidget::onPPVLLAClicked);//
	QObject::connect(d->ui->widget_ppv_llb, &AngleValveWidget::signalClicked, this, &QVacuumizeSubsystemWidget::onPPVLLBClicked);//

	QObject::connect(d->ui->widget_pbv_tm, &RoughExtractionValve::signalClicked, this, &QVacuumizeSubsystemWidget::onPBVTMClicked);//高真空挡板阀
	QObject::connect(d->ui->widget_pbv_lla, &RoughExtractionValve::signalClicked, this, &QVacuumizeSubsystemWidget::onPBVLLAClicked);//
	QObject::connect(d->ui->widget_pbv_llb, &SlideValveWidget::signalClicked, this, &QVacuumizeSubsystemWidget::onPBVLLBClicked);//

	QObject::connect(d->ui->open_loadlock1_vacuum_btn, &QAbstractButton::clicked, this, &QVacuumizeSubsystemWidget::onOpenLoadLock1AutoVacuumCommand);//一键抽真空（打开真空）
	QObject::connect(d->ui->open_loadlock2_vacuum_btn, &QAbstractButton::clicked, this, &QVacuumizeSubsystemWidget::onOpenLoadLock2AutoVacuumCommand);//
	QObject::connect(d->ui->open_tm_cavity_vacuum_btn, &QAbstractButton::clicked, this, &QVacuumizeSubsystemWidget::onOpenTMCavityAutoVacuumCommand);//


	d->widget_tm_tmp_ckb = new QCheckBox("TM分子泵");
	d->widget_lla_tmp_ckb = new QCheckBox("LLA分子泵");
	d->widget_llb_tmp_ckb = new QCheckBox("LLB分子泵");
	d->widget_vp_ckb = new QCheckBox("干泵");
	d->widget_pav_tm_ckb = new QCheckBox("TM角阀");
	d->widget_pav_lla_ckb = new QCheckBox("LLA角阀");
	d->widget_pav_llb_ckb = new QCheckBox("LLB角阀");
	d->widget_ppv_tm_ckb = new QCheckBox("TM插板阀");
	d->widget_ppv_lla_ckb = new QCheckBox("LLA插板阀");
	d->widget_ppv_llb_ckb = new QCheckBox("LLB插板阀");
	d->widget_pbv_tm_ckb = new QCheckBox("TM挡板阀");
	d->widget_pbv_lla_ckb = new QCheckBox("LLA挡板阀");
	d->widget_pbv_llb_ckb = new QCheckBox("LLB挡板阀");

	d->widget_tm_tmp_ckb->setObjectName("io_object");
	d->widget_lla_tmp_ckb->setObjectName("io_object");
	d->widget_llb_tmp_ckb->setObjectName("io_object");
	d->widget_vp_ckb->setObjectName("io_object");
	d->widget_pav_tm_ckb->setObjectName("io_object");
	d->widget_pav_lla_ckb->setObjectName("io_object");
	d->widget_pav_llb_ckb->setObjectName("io_object");
	d->widget_ppv_tm_ckb->setObjectName("io_object");
	d->widget_ppv_lla_ckb->setObjectName("io_object");
	d->widget_ppv_llb_ckb->setObjectName("io_object");
	d->widget_pbv_tm_ckb->setObjectName("io_object");
	d->widget_pbv_lla_ckb->setObjectName("io_object");
	d->widget_pbv_llb_ckb->setObjectName("io_object");

	d->ui->widget_tm_tmp_layout->addWidget(d->widget_tm_tmp_ckb, 0, 0);
	d->ui->widget_lla_tmp_layout->addWidget(d->widget_lla_tmp_ckb, 0, 0);
	d->ui->widget_llb_tmp_layout->addWidget(d->widget_llb_tmp_ckb, 0, 0);
	d->ui->widget_vp_layout->addWidget(d->widget_vp_ckb, 0, 0);
	d->ui->widget_pav_tm_layout->addWidget(d->widget_pav_tm_ckb, 0, 0);
	d->ui->widget_pav_lla_layout->addWidget(d->widget_pav_lla_ckb, 0, 0);
	d->ui->widget_pav_llb_layout->addWidget(d->widget_pav_llb_ckb, 0, 0);
	d->ui->widget_ppv_tm_layout->addWidget(d->widget_ppv_tm_ckb, 0, 0);
	d->ui->widget_ppv_lla_layout->addWidget(d->widget_ppv_lla_ckb, 0, 0);
	d->ui->widget_ppv_llb_layout->addWidget(d->widget_ppv_llb_ckb, 0, 0);
	d->ui->widget_pbv_tm_layout->addWidget(d->widget_pbv_tm_ckb, 0, 0);
	d->ui->widget_pbv_lla_layout->addWidget(d->widget_pbv_lla_ckb, 0, 0);
	d->ui->widget_pbv_llb_layout->addWidget(d->widget_pbv_llb_ckb, 0, 0);

	connect(d->widget_tm_tmp_ckb, &QCheckBox::clicked, this, [=](){
		if (d->widget_tm_tmp_ckb->checkState() == Qt::CheckState::Checked){
			onTMPTMClicked(true);
		}
		else{
			onTMPTMClicked(false);
		}
	});
	connect(d->widget_lla_tmp_ckb, &QCheckBox::clicked, this, [=](){
		if (d->widget_lla_tmp_ckb->checkState() == Qt::CheckState::Checked){
			onTMPLLAClicked(true);
		}
		else{
			onTMPLLAClicked(false);
		}
	});
	connect(d->widget_llb_tmp_ckb, &QCheckBox::clicked, this, [=](){
		if (d->widget_llb_tmp_ckb->checkState() == Qt::CheckState::Checked){
			onTMPLLBClicked(true);
		}
		else{
			onTMPLLBClicked(false);
		}
	});
	connect(d->widget_vp_ckb, &QCheckBox::clicked, this, [=](){
		if (d->widget_vp_ckb->checkState() == Qt::CheckState::Checked){
			onVPClicked(true);
		}
		else{
			onVPClicked(false);
		}
	});
	connect(d->widget_pav_tm_ckb, &QCheckBox::clicked, this, [=](){
		if (d->widget_pav_tm_ckb->checkState() == Qt::CheckState::Checked){
			onPAVTMClicked(true);
		}
		else{
			onPAVTMClicked(false);
		}
	});
	connect(d->widget_pav_lla_ckb, &QCheckBox::clicked, this, [=](){
		if (d->widget_pav_lla_ckb->checkState() == Qt::CheckState::Checked){
			onPAVLLAClicked(true);
		}
		else{
			onPAVLLAClicked(false);
		}
	});
	connect(d->widget_pav_llb_ckb, &QCheckBox::clicked, this, [=](){
		if (d->widget_pav_llb_ckb->checkState() == Qt::CheckState::Checked){
			onPAVLLBClicked(true);
		}
		else{
			onPAVLLBClicked(false);
		}
	});
	connect(d->widget_ppv_tm_ckb, &QCheckBox::clicked, this, [=](){
		if (d->widget_ppv_tm_ckb->checkState() == Qt::CheckState::Checked){
			onPPVTMClicked(true);
		}
		else{
			onPPVTMClicked(false);
		}
	});
	connect(d->widget_ppv_lla_ckb, &QCheckBox::clicked, this, [=](){
		if (d->widget_ppv_lla_ckb->checkState() == Qt::CheckState::Checked){
			onPPVLLAClicked(true);
		}
		else{
			onPPVLLAClicked(false);
		}
	});
	connect(d->widget_ppv_llb_ckb, &QCheckBox::clicked, this, [=](){
		if (d->widget_ppv_llb_ckb->checkState() == Qt::CheckState::Checked){
			onPPVLLBClicked(true);
		}
		else{
			onPPVLLBClicked(false);
		}
	});
	connect(d->widget_pbv_tm_ckb, &QCheckBox::clicked, this, [=](){
		if (d->widget_pbv_tm_ckb->checkState() == Qt::CheckState::Checked){
			onPBVTMClicked(true);
		}
		else{
			onPBVTMClicked(false);
		}
	});
	connect(d->widget_pbv_lla_ckb, &QCheckBox::clicked, this, [=](){
		if (d->widget_pbv_lla_ckb->checkState() == Qt::CheckState::Checked){
			onPBVLLAClicked(true);
		}
		else{
			onPBVLLAClicked(false);
		}
	});
	connect(d->widget_pbv_llb_ckb, &QCheckBox::clicked, this, [=](){
		if (d->widget_pbv_llb_ckb->checkState() == Qt::CheckState::Checked){
			onPBVLLBClicked(true);
		}
		else{
			onPBVLLBClicked(false);
		}
	});

	//connect(d->widget_pav_tm_ckb, &QCheckBox::clicked, this, [=](){
	//	if (d->widget_pav_tm_ckb->checkState() == Qt::CheckState::Checked){
	//		onPAVTMClicked(true);
	//	}
	//	else{
	//		onPAVTMClicked(false);
	//	}
	//	//logInform("Test", "widget_pav_tm_ckb %d", d->widget_pav_tm_ckb->checkState() == Qt::CheckState::Checked);
	//});

	/*d->ui->widget_pav_tm->close();
	d->ui->widget_pav_lla->close();
	d->ui->widget_pav_llb->close();
	d->ui->widget_ppv_tm->close();
	d->ui->widget_ppv_lla->close();
	d->ui->widget_ppv_llb->close();
	d->ui->widget_pbv_tm->close();
	d->ui->widget_pbv_lla->close();
	d->ui->widget_pbv_llb->close();*/
}

QVacuumizeSubsystemWidget::~QVacuumizeSubsystemWidget(){

}

void QVacuumizeSubsystemWidget::onOpenTMCavityAutoVacuumCommand(){
	Q_D(QVacuumizeSubsystemWidget);
	if (d->pump->getState() != IKernelSubSystem::State::SUB_NORMAL){
		logError(d->pump->getName().c_str(), "模组：%s状态异常，请先复位", d->pump->getName().c_str());
		return;
	}
	KernelSubsystemCommand::Ptr cmd = d->pump->createOpenTMCavityAutoVacuumCommand();
	cmd->setOrigin("GUI");
	cmd->addListener(d);
	d->pump->startCommand(cmd);
}
void QVacuumizeSubsystemWidget::onOpenLoadLock1AutoVacuumCommand(){
	Q_D(QVacuumizeSubsystemWidget);
	if (d->pump->getState() != IKernelSubSystem::State::SUB_NORMAL){
		logError(d->pump->getName().c_str(), "模组：%s状态异常，请先复位", d->pump->getName().c_str());
		return;
	}
	KernelSubsystemCommand::Ptr cmd = d->pump->createOpenLoadLock1AutoVacuumCommand();
	cmd->setOrigin("GUI");
	cmd->addListener(d);
	d->pump->startCommand(cmd);
}
void QVacuumizeSubsystemWidget::onOpenLoadLock2AutoVacuumCommand(){
	Q_D(QVacuumizeSubsystemWidget);
	if (d->pump->getState() != IKernelSubSystem::State::SUB_NORMAL){
		logError(d->pump->getName().c_str(), "模组：%s状态异常，请先复位", d->pump->getName().c_str());
		return;
	}
	KernelSubsystemCommand::Ptr cmd = d->pump->createOpenLoadLock2AutoVacuumCommand();
	cmd->setOrigin("GUI");
	cmd->addListener(d);
	d->pump->startCommand(cmd);
}

void QVacuumizeSubsystemWidget::onReset(){
    Q_D(QVacuumizeSubsystemWidget);
   
}

void QVacuumizeSubsystemWidget::onAttributeUpdate()throw(KernelException){
	Q_D(QVacuumizeSubsystemWidget);

	d->ui->tm_cavity_current_vacuum_value_let->setText(QString::number(d->tm->getTMCavityVacuumValue(), 'e', 3).append("Pa"));
	d->ui->molecule_pipeline_value_let->setText(QString::number(d->tm->getMoleculePipelineVacuumValue(), 'e', 3).append("Pa"));
	d->ui->lla_current_vacuum_value_let->setText(QString::number(d->lk1->getVacuumValue(), 'e', 3).append("Pa"));
	d->ui->llb_current_vacuum_value_let->setText(QString::number(d->lk2->getVacuumValue(), 'e', 3).append("Pa"));
	//初始化管道的流动状态
	if (d->pump->getMechanicalPumpOpened()){
		//干泵开启状态
		d->ui->widget_vp->open();
		d->widget_vp_ckb->setChecked(true);
		d->ui->vp_h_1->setWaterDirection(0);//干泵
		d->ui->vp_h_2->setWaterDirection(0);
		d->ui->vp_h_3->setWaterDirection(0);
		d->ui->vp_w_1->setWaterDirection(0);
		d->ui->vp_w_2->setWaterDirection(0);
		d->ui->vp_w_3->setWaterDirection(1);//1从右往左，从下往上，0从左往右，从上往下
		d->ui->vp_w_4->setWaterDirection(1);
		d->ui->vp_w_5->setWaterDirection(1);
		d->ui->vp_w_6->setWaterDirection(1);
		d->ui->vp_w_7->setWaterDirection(1);
		d->ui->vp_w_8->setWaterDirection(1);
	}
	else{
		d->ui->widget_vp->close();
		d->widget_vp_ckb->setChecked(false);
		d->ui->vp_h_1->setWaterDirection(2);//干泵关闭
		d->ui->vp_h_2->setWaterDirection(2);
		d->ui->vp_h_3->setWaterDirection(2);
		d->ui->vp_w_1->setWaterDirection(2);
		d->ui->vp_w_2->setWaterDirection(2);
		d->ui->vp_w_3->setWaterDirection(2);//1从右往左，从下往上，0从左往右，从上往下
		d->ui->vp_w_4->setWaterDirection(2);
		d->ui->vp_w_5->setWaterDirection(2);
		d->ui->vp_w_6->setWaterDirection(2);
		d->ui->vp_w_7->setWaterDirection(2);
		d->ui->vp_w_8->setWaterDirection(2);
	}

	//分子泵开启状态
	if (d->pump->getMolecularPumpOpenedLLA()){
		d->ui->widget_lla_tmp->open();
		d->widget_lla_tmp_ckb->setChecked(true);
		d->ui->tmp_lla_w_1->setWaterDirection(1);
	}
	else{
		d->ui->widget_lla_tmp->close();
		d->widget_lla_tmp_ckb->setChecked(false);
		d->ui->tmp_lla_w_1->setWaterDirection(2);
	}

	if (d->pump->getMolecularPumpOpenedLLB()){
		d->ui->widget_llb_tmp->open();
		d->widget_llb_tmp_ckb->setChecked(true);
		d->ui->tmp_llb_w_1->setWaterDirection(1);
	}
	else{
		d->ui->widget_llb_tmp->close();
		d->widget_llb_tmp_ckb->setChecked(false);
		d->ui->tmp_llb_w_1->setWaterDirection(2);
	}


	if (d->pump->getMolecularPumpOpenedTM()){
		d->ui->widget_tm_tmp->open();
		d->widget_tm_tmp_ckb->setChecked(true);
		d->ui->tmp_tm_w_1->setWaterDirection(1);
	}
	else{
		d->ui->widget_tm_tmp->close();
		d->widget_tm_tmp_ckb->setChecked(false);
		d->ui->tmp_tm_w_1->setWaterDirection(2);
	}




	if (d->tm->getHeightVacuumBaffleValveOpend()){
		d->ui->pbv_tm_w_1->setWaterDirection(1);//挡板阀
		d->ui->widget_pbv_tm->open();
		d->widget_pbv_tm_ckb->setChecked(true);
	}
	else{
		d->ui->pbv_tm_w_1->setWaterDirection(2);//挡板阀
		d->ui->widget_pbv_tm->close();
		d->widget_pbv_tm_ckb->setChecked(false);
	}

	if (d->tm->getInsertingPlateValveOpend()){
		d->ui->widget_ppv_tm->open();
		d->widget_ppv_tm_ckb->setChecked(true);
		d->ui->ppv_tm_w_1->setWaterDirection(1);//插板阀
	}
	else{
		d->ui->widget_ppv_tm->close();
		d->widget_ppv_tm_ckb->setChecked(false);
		d->ui->ppv_tm_w_1->setWaterDirection(2);//插板阀
	}

	if (d->tm->getAngleValveOpend()){
		d->ui->widget_pav_tm->open();
		d->widget_pav_tm_ckb->setChecked(true);
		d->ui->pav_tm_w_1->setWaterDirection(1);//角阀
	}
	else{
		d->ui->widget_pav_tm->close();
		d->widget_pav_tm_ckb->setChecked(false);
		d->ui->pav_tm_w_1->setWaterDirection(2);//角阀
	}


	if (d->lk1->getHeightVacuumBaffleValveOpend()){
		//d->ui->widget_ppv_lla->open();
		d->ui->pbv_lla_w_1->setWaterDirection(1);//挡板阀
		d->ui->widget_pbv_lla->open();
		d->widget_pbv_lla_ckb->setChecked(true);
	}
	else{
		d->ui->widget_pbv_lla->close();
		d->widget_pbv_lla_ckb->setChecked(false);
		d->ui->pbv_lla_w_1->setWaterDirection(2);
	}

	if (d->lk1->getInsertingPlateValveOpend()){
		d->ui->widget_ppv_lla->open();
		d->widget_ppv_lla_ckb->setChecked(true);
		d->ui->ppv_lla_w_1->setWaterDirection(1);//插板阀
	}
	else{
		d->ui->widget_ppv_lla->close();
		d->widget_ppv_lla_ckb->setChecked(false);
		d->ui->ppv_lla_w_1->setWaterDirection(2);
	}

	if (d->lk1->getAngleValveOpend()){
		d->ui->widget_pav_lla->open();
		d->widget_pav_lla_ckb->setChecked(true);
		d->ui->pav_lla_w_1->setWaterDirection(1);//角阀
	}
	else{
		d->ui->widget_pav_lla->close();
		d->widget_pav_lla_ckb->setChecked(false);
		d->ui->pav_lla_w_1->setWaterDirection(2);
	}

	if (d->lk2->getHeightVacuumBaffleValveOpend()){
		d->ui->widget_pbv_llb->open();
		d->widget_pbv_llb_ckb->setChecked(true);
		d->ui->pbv_llb_w_1->setWaterDirection(0);//挡板阀
		d->ui->pbv_llb_h_1->setWaterDirection(0);
	}
	else{
		d->ui->widget_pbv_llb->close();
		d->widget_pbv_llb_ckb->setChecked(false);
		d->ui->pbv_llb_w_1->setWaterDirection(2);
		d->ui->pbv_llb_h_1->setWaterDirection(2);
	}

	if (d->lk2->getInsertingPlateValveOpend()){
		d->ui->widget_ppv_llb->open();
		d->widget_ppv_llb_ckb->setChecked(true);
		d->ui->ppv_llb_w_1->setWaterDirection(1);//插板阀
		d->ui->ppv_llb_h_1->setWaterDirection(0);
	}
	else{
		d->ui->widget_ppv_llb->close();
		d->widget_ppv_llb_ckb->setChecked(false);
		d->ui->ppv_llb_w_1->setWaterDirection(2);
		d->ui->ppv_llb_h_1->setWaterDirection(2);
	}

	if (d->lk2->getAngleValveOpend()){
		d->ui->widget_pav_llb->open();
		d->widget_pav_llb_ckb->setChecked(true);
		d->ui->pav_llb_w_1->setWaterDirection(1);//角阀
	}
	else{
		d->ui->widget_pav_llb->close();
		d->widget_pav_llb_ckb->setChecked(false);
		d->ui->pav_llb_w_1->setWaterDirection(2);
	}

	if (d->lk1->getTMCavityDoorOpend()){
		d->ui->tm__lla_door->setWaterDirection(1);
	}
	else{
		d->ui->tm__lla_door->setWaterDirection(2);
	}

	if (d->lk2->getTMCavityDoorOpend()){
		d->ui->tm__llb_door->setWaterDirection(1);
	}
	else{
		d->ui->tm__llb_door->setWaterDirection(2);
	}
	

	d->ui->widget_10->setWaterDirection(2);
	d->ui->widget_13->setWaterDirection(2);
	d->ui->widget_53->setWaterDirection(2);





}

void QVacuumizeSubsystemWidget::onGetStatus(){
    Q_D(QVacuumizeSubsystemWidget);
  
}


void QVacuumizeSubsystemWidget::onOutput(){
    Q_D(QVacuumizeSubsystemWidget);
   
}
void QVacuumizeSubsystemWidget::onTMPTMClicked(bool status){
	Q_D(QVacuumizeSubsystemWidget);
	if (d->pump->getState() != IKernelSubSystem::State::SUB_NORMAL){
		logError(d->pump->getName().c_str(), "模组：%s状态异常，请先复位", d->pump->getName().c_str());
		return;
	}
		if (status){
			KernelSubsystemCommand::Ptr cmd = d->pump->createMolecularOpenCommand("TM");
			cmd->setOrigin("GUI");
			cmd->addListener(d);
			d->pump->startCommand(cmd);
			d->ui->tmp_tm_w_1->setWaterDirection(1);//分子泵
			d->ui->widget_tm_tmp->open();
		}
		else{
			KernelSubsystemCommand::Ptr cmd = d->pump->createMolecularCloseCommand("TM");
			cmd->setOrigin("GUI");
			cmd->addListener(d);
			d->pump->startCommand(cmd);
			d->ui->tmp_tm_w_1->setWaterDirection(2);//关闭分子泵
			d->ui->widget_tm_tmp->close();
		}
}
void QVacuumizeSubsystemWidget::onTMPLLAClicked(bool status){
	Q_D(QVacuumizeSubsystemWidget);
	if (d->pump->getState() != IKernelSubSystem::State::SUB_NORMAL){
		logError(d->pump->getName().c_str(), "模组：%s状态异常，请先复位", d->pump->getName().c_str());
		return;
	}
		if (status){
			KernelSubsystemCommand::Ptr cmd = d->pump->createMolecularOpenCommand("LLA");
			cmd->setOrigin("GUI");
			cmd->addListener(d);
			d->pump->startCommand(cmd);
			d->ui->tmp_lla_w_1->setWaterDirection(1);
			d->ui->widget_lla_tmp->open();
		}
		else{
			KernelSubsystemCommand::Ptr cmd = d->pump->createMolecularCloseCommand("LLA");
			cmd->setOrigin("GUI");
			cmd->addListener(d);
			d->pump->startCommand(cmd);
			d->ui->tmp_lla_w_1->setWaterDirection(2);
			d->ui->widget_lla_tmp->close();
		}
}
void QVacuumizeSubsystemWidget::onTMPLLBClicked(bool status){
	Q_D(QVacuumizeSubsystemWidget);
	if (d->pump->getState() != IKernelSubSystem::State::SUB_NORMAL){
		logError(d->pump->getName().c_str(), "模组：%s状态异常，请先复位", d->pump->getName().c_str());
		return;
	}
		if (status){
			KernelSubsystemCommand::Ptr cmd = d->pump->createMolecularOpenCommand("LLB");
			cmd->setOrigin("GUI");
			cmd->addListener(d);
			d->pump->startCommand(cmd);
			d->ui->tmp_llb_w_1->setWaterDirection(1);
			d->ui->widget_llb_tmp->open();
		}
		else{
			KernelSubsystemCommand::Ptr cmd = d->pump->createMolecularCloseCommand("LLB");
			cmd->setOrigin("GUI");
			cmd->addListener(d);
			d->pump->startCommand(cmd);
			d->ui->tmp_llb_w_1->setWaterDirection(2);
			d->ui->widget_llb_tmp->close();
		}
}
void QVacuumizeSubsystemWidget::onVPClicked(bool status){
	Q_D(QVacuumizeSubsystemWidget);
	if (d->pump->getState() != IKernelSubSystem::State::SUB_NORMAL){
		logError(d->pump->getName().c_str(), "模组：%s状态异常，请先复位", d->pump->getName().c_str());
		return;
	}
	if (status){
		KernelSubsystemCommand::Ptr cmd = d->pump->createMechanicalOpenCommand();
		cmd->setOrigin("GUI");
		cmd->addListener(d);
		d->pump->startCommand(cmd);
		d->ui->widget_vp->open();
	}
	else{
		KernelSubsystemCommand::Ptr cmd = d->pump->createMechanicalCloseCommand();
		cmd->setOrigin("GUI");
		cmd->addListener(d);
		d->pump->startCommand(cmd);
		d->ui->widget_vp->close();
	}
}
//角阀
void QVacuumizeSubsystemWidget::onPAVTMClicked(bool status){
	Q_D(QVacuumizeSubsystemWidget);
	if (d->tm->getState() != IKernelSubSystem::State::SUB_NORMAL){
		logError(d->tm->getName().c_str(), "模组：%s状态异常，请先复位", d->tm->getName().c_str());
		return;
	}
		if (status){
			KernelSubsystemCommand::Ptr cmd = d->tm->createOpenAngleValveCommand();
			cmd->setOrigin("GUI");
			cmd->addListener(d);
			d->tm->startCommand(cmd);
		}
		else{
			KernelSubsystemCommand::Ptr cmd = d->tm->createCloseAngleValveCommand();
			cmd->setOrigin("GUI");
			cmd->addListener(d);
			d->tm->startCommand(cmd);
		}

}
void QVacuumizeSubsystemWidget::onPAVLLAClicked(bool status){
	Q_D(QVacuumizeSubsystemWidget);
	if (d->lk1->getState() != IKernelSubSystem::State::SUB_NORMAL){
		logError(d->lk1->getName().c_str(), "模组：%s状态异常，请先复位", d->lk1->getName().c_str());
		return;
	}
	if (status){
		KernelSubsystemCommand::Ptr cmd = d->lk1->createOpenAngleValveCommand();
		cmd->setOrigin("GUI");
		cmd->addListener(d);
		d->lk1->startCommand(cmd);
	}
	else{
		KernelSubsystemCommand::Ptr cmd = d->lk1->createCloseAngleValveCommand();
		cmd->setOrigin("GUI");
		cmd->addListener(d);
		d->lk1->startCommand(cmd);
	}
}
void QVacuumizeSubsystemWidget::onPAVLLBClicked(bool status){
	Q_D(QVacuumizeSubsystemWidget);
	if (d->lk2->getState() != IKernelSubSystem::State::SUB_NORMAL){
		logError(d->lk2->getName().c_str(), "模组：%s状态异常，请先复位", d->lk2->getName().c_str());
		return;
	}
	if (status){
		KernelSubsystemCommand::Ptr cmd = d->lk2->createOpenAngleValveCommand();
		cmd->setOrigin("GUI");
		cmd->addListener(d);
		d->lk2->startCommand(cmd);
	}
	else{
		KernelSubsystemCommand::Ptr cmd = d->lk2->createCloseAngleValveCommand();
		cmd->setOrigin("GUI");
		cmd->addListener(d);
		d->lk2->startCommand(cmd);
	}
}
//插板阀
void QVacuumizeSubsystemWidget::onPPVTMClicked(bool status){
	Q_D(QVacuumizeSubsystemWidget);
	if (d->tm->getState() != IKernelSubSystem::State::SUB_NORMAL){
		logError(d->tm->getName().c_str(), "模组：%s状态异常，请先复位", d->tm->getName().c_str());
		return;
	}
	if (status){
		KernelSubsystemCommand::Ptr cmd = d->tm->createOpenInsertingPlateValveCommand();
		cmd->setOrigin("GUI");
		cmd->addListener(d);
		d->tm->startCommand(cmd);
	}
	else{
		KernelSubsystemCommand::Ptr cmd = d->tm->createCloseInsertingPlateValveCommand();
		cmd->setOrigin("GUI");
		cmd->addListener(d);
		d->tm->startCommand(cmd);
	}
}
void QVacuumizeSubsystemWidget::onPPVLLAClicked(bool status){
	Q_D(QVacuumizeSubsystemWidget);
	if (d->lk1->getState() != IKernelSubSystem::State::SUB_NORMAL){
		logError(d->lk1->getName().c_str(), "模组：%s状态异常，请先复位", d->lk1->getName().c_str());
		return;
	}
	if (status){
		KernelSubsystemCommand::Ptr cmd = d->lk1->createOpenInsertingPlateValveCommand();
		cmd->setOrigin("GUI");
		cmd->addListener(d);
		d->lk1->startCommand(cmd);
	}
	else{
		KernelSubsystemCommand::Ptr cmd = d->lk1->createCloseInsertingPlateValveCommand();
		cmd->setOrigin("GUI");
		cmd->addListener(d);
		d->lk1->startCommand(cmd);
	}
}
void QVacuumizeSubsystemWidget::onPPVLLBClicked(bool status){
	Q_D(QVacuumizeSubsystemWidget);
	if (d->lk2->getState() != IKernelSubSystem::State::SUB_NORMAL){
		logError(d->lk2->getName().c_str(), "模组：%s状态异常，请先复位", d->lk2->getName().c_str());
		return;
	}
	if (status){
		KernelSubsystemCommand::Ptr cmd = d->lk2->createOpenInsertingPlateValveCommand();
		cmd->setOrigin("GUI");
		cmd->addListener(d);
		d->lk2->startCommand(cmd);
	}
	else{
		KernelSubsystemCommand::Ptr cmd = d->lk2->createCloseInsertingPlateValveCommand();
		cmd->setOrigin("GUI");
		cmd->addListener(d);
		d->lk2->startCommand(cmd);
	}
}
//高真空挡板阀
void QVacuumizeSubsystemWidget::onPBVTMClicked(bool status){
	Q_D(QVacuumizeSubsystemWidget);
	if (d->tm->getState() != IKernelSubSystem::State::SUB_NORMAL){
		logError(d->tm->getName().c_str(), "模组：%s状态异常，请先复位", d->tm->getName().c_str());
		return;
	}

	if (status){
		KernelSubsystemCommand::Ptr cmd = d->tm->createOpenHeightVacuumBaffleValveCommand();
		cmd->setOrigin("GUI");
		cmd->addListener(d);
		d->tm->startCommand(cmd);
	}
	else{
		KernelSubsystemCommand::Ptr cmd = d->tm->createCloseHeightVacuumBaffleValveCommand();
		cmd->setOrigin("GUI");
		cmd->addListener(d);
		d->tm->startCommand(cmd);
	}
}
void QVacuumizeSubsystemWidget::onPBVLLAClicked(bool status){
	Q_D(QVacuumizeSubsystemWidget);
	if (d->lk1->getState() != IKernelSubSystem::State::SUB_NORMAL){
		logError(d->lk1->getName().c_str(), "模组：%s状态异常，请先复位", d->lk1->getName().c_str());
		return;
	}
	if (status){
		KernelSubsystemCommand::Ptr cmd = d->lk1->createOpenHeightVacuumBaffleValveCommand();
		cmd->setOrigin("GUI");
		cmd->addListener(d);
		d->lk1->startCommand(cmd);
	}
	else{
		KernelSubsystemCommand::Ptr cmd = d->lk1->createCloseHeightVacuumBaffleValveCommand();
		cmd->setOrigin("GUI");
		cmd->addListener(d);
		d->lk1->startCommand(cmd);
	}
}
void QVacuumizeSubsystemWidget::onPBVLLBClicked(bool status){
	Q_D(QVacuumizeSubsystemWidget);
	if (d->lk2->getState() != IKernelSubSystem::State::SUB_NORMAL){
		logError(d->lk2->getName().c_str(), "模组：%s状态异常，请先复位", d->lk2->getName().c_str());
		return;
	}
	if (status){
		KernelSubsystemCommand::Ptr cmd = d->lk2->createOpenHeightVacuumBaffleValveCommand();
		cmd->setOrigin("GUI");
		cmd->addListener(d);
		d->lk2->startCommand(cmd);
	}
	else{
		KernelSubsystemCommand::Ptr cmd = d->lk2->createCloseHeightVacuumBaffleValveCommand();
		cmd->setOrigin("GUI");
		cmd->addListener(d);
		d->lk2->startCommand(cmd);
	}
}

void QVacuumizeSubsystemWidget::showMessage(const QString& message){
	QMessageBox::information(this, "warn", message);
}
 

}