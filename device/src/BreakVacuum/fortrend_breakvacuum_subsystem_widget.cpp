
#include  "BreakVacuum/fortrend_breakvacuum_subsystem_widget.h"
#include  "device/ui_fortrend_breakvacuum_subsystem_widget.h"

//#include "Kernel/kernel_event_module.h"
//#include "Kernel/kernel_event_paramters.h"
#include  "Kernel/CoreUI/kernel_subsystem_status_widget.h" 
#include "TMCavity/fortrend_tm_cavity_defined.h"
#include "LoadLock/fortrend_loadlock_defined.h"

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

#include "Pump/fortrend_pump_subsystem.h"
#include "Pump/fortrend_pump_open_tm_cavity_auto_vacuum_command.h"

#include <QMessageBox>
#include <QCheckBox>

#include <iostream>

namespace FC{
	class QBreakVacuumSubsystemWidgetPrivate : public KernelListener<IKernelCommand> {
		Q_DECLARE_PUBLIC(QBreakVacuumSubsystemWidget)
	public:
		QBreakVacuumSubsystemWidgetPrivate(QBreakVacuumSubsystemWidget* p, const std::shared_ptr<IKernel>& kernel);
	public:
		Ui::BreakVacuumSubsystemWidget* ui;
		QBreakVacuumSubsystemWidget* q_ptr;
		std::shared_ptr<IKernel> kernel;
		std::shared_ptr<FortrendLoadLockSubsystem> lk1;
		std::shared_ptr<FortrendLoadLockSubsystem> lk2;
		std::shared_ptr<FortrendTMCavitySubsystem> tm;
		std::shared_ptr<FortrendPumpSubsystem> pump;

		QCheckBox *gmfk_tm_ckb = 0, *gmfm_tm_ckb = 0, *gmfk_lla_ckb = 0, *gmfm_lla_ckb = 0, *gmfk_llb_ckb = 0, *gmfm_llb_ckb = 0;

		virtual void onAttributeChange(const IKernelCommand* cmd){
			const KernelSubsystemCommand* subCmd = dynamic_cast<const KernelSubsystemCommand*>(cmd);
			if (subCmd && subCmd->hasError())
			{
				AlarmMessage::Ptr alarm = cmd->alarmMessage();
				std::cout << "message:%s" << alarm->message().c_str() << std::endl;
				logInform("AlarmMessage", "alarm type:%d,code:%d,message:%s,name:%s", alarm->type(), alarm->code(), alarm->message().c_str());
				QString msg;
				if (alarm == nullptr)
				{
					msg = QString("%1 %2: \n[T=%3][C=0x%4]%5")
						.arg(subCmd->getSubsystem()->getName().c_str())
						.arg(cmd->getName().c_str())
						.arg(1)
						.arg(1, 0, 16)
						.arg("未知报警");
				}
				else
				{
					msg = QString("%1 %2: \n[T=%3][C=0x%4]%5")
						.arg(subCmd->getSubsystem()->getName().c_str())
						.arg(cmd->getName().c_str())
						.arg(alarm->type())
						.arg(alarm->code(), 0, 16)
						.arg(alarm->message().c_str());
				}

				QMetaObject::invokeMethod(q_ptr, "showMessage", Qt::AutoConnection,
					Q_ARG(QString, msg));
				
			}
		}


	};

	QBreakVacuumSubsystemWidgetPrivate::QBreakVacuumSubsystemWidgetPrivate(QBreakVacuumSubsystemWidget* p, const std::shared_ptr<IKernel>& kernel)
		:q_ptr(p), kernel(kernel){

		lk1 = kernel->getKernelModule<FortrendLoadLockSubsystem>("LLA");
		lk2 = kernel->getKernelModule<FortrendLoadLockSubsystem>("LLB");
		tm = kernel->getKernelModule<FortrendTMCavitySubsystem>("TM");
		pump = kernel->getKernelModule<FortrendPumpSubsystem>("PUMP");
	}



	QBreakVacuumSubsystemWidget::QBreakVacuumSubsystemWidget(const std::shared_ptr<IKernel>& kernel, QWidget *parent) :
		QWidget(parent),
		d_ptr(new QBreakVacuumSubsystemWidgetPrivate(this, kernel))
	{
		Q_D(QBreakVacuumSubsystemWidget);
		d->ui = new Ui::BreakVacuumSubsystemWidget;
		d->ui->setupUi(this);

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

		//QObject::connect(d->ui->gmfk_tm, &SlideValveWidget::signalClicked, this, &QBreakVacuumSubsystemWidget::onTMGMFKClicked);
		QObject::connect(d->ui->gmfm_tm, &SlideValveWidget::signalClicked, this, &QBreakVacuumSubsystemWidget::onTMGMFMClicked);
		
		QObject::connect(d->ui->gmfk_lla, &SlideValveWidget::signalClicked, this, &QBreakVacuumSubsystemWidget::onLLAGMFKClicked);
		QObject::connect(d->ui->gmfm_lla, &SlideValveWidget::signalClicked, this, &QBreakVacuumSubsystemWidget::onLLAGMFMClicked);

		QObject::connect(d->ui->gmfk_llb, &SlideValveWidget::signalClicked, this, &QBreakVacuumSubsystemWidget::onLLBGMFKClicked);
		QObject::connect(d->ui->gmfm_llb, &SlideValveWidget::signalClicked, this, &QBreakVacuumSubsystemWidget::onLLBGMFMClicked);

		QObject::connect(d->ui->open_loadlock1_break_vacuum_btn, &QAbstractButton::clicked, this, &QBreakVacuumSubsystemWidget::onOpenLoadLock1AutoBreakVacuumCommand);//一键抽真空
		QObject::connect(d->ui->open_loadlock2_break_vacuum_btn, &QAbstractButton::clicked, this, &QBreakVacuumSubsystemWidget::onOpenLoadLock2AutoBreakVacuumCommand);//
		QObject::connect(d->ui->open_tm_cavity_break_vacuum_btn, &QAbstractButton::clicked, this, &QBreakVacuumSubsystemWidget::onOpenTMCavityAutoBreakVacuumCommand);//
		
		d->gmfk_tm_ckb = new QCheckBox("快充隔膜阀");
		d->gmfm_tm_ckb = new QCheckBox("慢充隔膜阀");
		d->gmfk_lla_ckb = new QCheckBox("快充隔膜阀");
		d->gmfm_lla_ckb = new QCheckBox("慢充隔膜阀");
		d->gmfk_llb_ckb = new QCheckBox("快充隔膜阀");
		d->gmfm_llb_ckb = new QCheckBox("慢充隔膜阀");

		d->gmfk_tm_ckb->setObjectName("io_object");
		d->gmfm_tm_ckb->setObjectName("io_object");
		d->gmfk_lla_ckb->setObjectName("io_object");
		d->gmfm_lla_ckb->setObjectName("io_object");
		d->gmfk_llb_ckb->setObjectName("io_object");
		d->gmfm_llb_ckb->setObjectName("io_object");

//		d->ui->gmfk_tm_layout->addWidget(d->gmfk_tm_ckb, 0, 0);
		d->ui->gmfm_tm_layout->addWidget(d->gmfm_tm_ckb, 0, 0);
		d->ui->gmfk_lla_layout->addWidget(d->gmfk_lla_ckb, 0, 0);
		d->ui->gmfm_lla_layout->addWidget(d->gmfm_lla_ckb, 0, 0);
		d->ui->gmfk_llb_layout->addWidget(d->gmfk_llb_ckb, 0, 0);
		d->ui->gmfm_llb_layout->addWidget(d->gmfm_llb_ckb, 0, 0);

		connect(d->gmfk_tm_ckb, &QCheckBox::clicked, this, [=](){
			if (d->gmfk_tm_ckb->checkState() == Qt::CheckState::Checked){
				onTMGMFKClicked(true);
			}
			else{
				onTMGMFKClicked(false);
			}
		});
		connect(d->gmfm_tm_ckb, &QCheckBox::clicked, this, [=](){
			if (d->gmfm_tm_ckb->checkState() == Qt::CheckState::Checked){
				onTMGMFMClicked(true);
			}
			else{
				onTMGMFMClicked(false);
			}
		});
		connect(d->gmfk_lla_ckb, &QCheckBox::clicked, this, [=](){
			if (d->gmfk_lla_ckb->checkState() == Qt::CheckState::Checked){
				onLLAGMFKClicked(true);
			}
			else{
				onLLAGMFKClicked(false);
			}
		});
		connect(d->gmfm_lla_ckb, &QCheckBox::clicked, this, [=](){
			if (d->gmfm_lla_ckb->checkState() == Qt::CheckState::Checked){
				onLLAGMFMClicked(true);
			}
			else{
				onLLAGMFMClicked(false);
			}
		});
		connect(d->gmfk_llb_ckb, &QCheckBox::clicked, this, [=](){
			if (d->gmfk_llb_ckb->checkState() == Qt::CheckState::Checked){
				onLLBGMFKClicked(true);
			}
			else{
				onLLBGMFKClicked(false);
			}
		});
		connect(d->gmfm_llb_ckb, &QCheckBox::clicked, this, [=](){
			if (d->gmfm_llb_ckb->checkState() == Qt::CheckState::Checked){
				onLLBGMFMClicked(true);
			}
			else{
				onLLBGMFMClicked(false);
			}
		});


		d->ui->widget_4->setWaterDirection(1);//设置从下往上流动
		d->ui->widget_13->setWaterDirection(1);//设置从下往上流动
		d->ui->widget_11->angle = 270;//设置从下往上流动
		d->ui->widget_17->setWaterDirection(1);//设置从下往上流动
		d->ui->widget_26->setWaterDirection(1);//设置从下往上流动
		d->ui->widget_29->setWaterDirection(1);//设置从下往上流动
		d->ui->widget_30->setWaterDirection(1);//设置从下往上流动
		d->ui->widget_31->setWaterDirection(1);//设置从下往上流动
		d->ui->widget_35->setWaterDirection(1);//设置从下往上流动
		d->ui->widget_36->setWaterDirection(1);//设置从下往上流动
		d->ui->widget_37->setWaterDirection(1);//设置从下往上流动
		d->ui->widget_41->setWaterDirection(1);//设置从下往上流动
		d->ui->widget_42->setWaterDirection(1);//设置从下往上流动
		d->ui->widget_43->setWaterDirection(1);//设置从下往上流动
//		d->ui->widget_44->setWaterDirection(1);//设置从下往上流动
//		d->ui->widget_46->setWaterDirection(1);//设置从下往上流动
		d->ui->widget_53->setWaterDirection(1);//设置从下往上流动
		d->ui->widget_55->setWaterDirection(1);//设置从下往上流动
		d->ui->widget_62->setWaterDirection(1);//设置从下往上流动
		d->ui->widget_64->setWaterDirection(1);//设置从下往上流动
		d->ui->widget_75->setWaterDirection(1);//设置从下往上流动
		d->ui->widget_78->setWaterDirection(1);//设置从下往上流动
		d->ui->widget_79->setWaterDirection(1);//设置从下往上流动

		d->ui->tm__lla_door->setWaterDirection(2);
		d->ui->tm__llb_door->setWaterDirection(2);


		//ui->widget_14->setWaterDirection(1);//设置从下往上流动
		//ui->widget_15->setWaterDirection(1);//设置从下往上流动
		//ui->widget_16->setWaterDirection(1);//设置从下往上流动
		//ui->widget_9 ->angle = 270;//设置从下往上流动
		//ui->widget_10->angle = 270;//设置从下往上流动
		//ui->widget_12->angle = 270;//设置从下往上流动
		//ui->widget_18->setWaterDirection(1);//设置从下往上流动
		//ui->widget_19->setWaterDirection(1);//设置从下往上流动
		//ui->widget_20->setWaterDirection(1);//设置从下往上流动
		// ui->widget_21->setWaterDirection(1);//设置从下往上流动
		//ui->widget_22->setWaterDirection(1);//设置从下往上流动
		// ui->widget_23->setWaterDirection(1);//设置从下往上流动
		//ui->widget_24->setWaterDirection(1);//设置从下往上流动
		//ui->widget_16->setWaterDirection(1);

		// 设置径向渐变
		QRadialGradient gradient(width() / 2, height() / 2, width() / 2 * 1.5); // 中心点和半径
		gradient.setColorAt(0, Qt::darkBlue); // 起始颜色（中心）
		gradient.setColorAt(1, Qt::black); // 结束颜色（边缘）


		// 设置QPalette并应用渐变
		QPalette palette;
		palette.setBrush(backgroundRole(), gradient);
		setPalette(palette);

		// 设置窗口属性，使背景色可以自动填充整个窗口
		setAutoFillBackground(true);
	}

	QBreakVacuumSubsystemWidget::~QBreakVacuumSubsystemWidget()
	{
	}

	void QBreakVacuumSubsystemWidget::onTMGMFKClicked(bool status){
		Q_D(QBreakVacuumSubsystemWidget);
		if (d->tm->getState() != IKernelSubSystem::State::SUB_NORMAL){
			logError(d->tm->getName().c_str(), "模组：%s状态异常，请先复位", d->tm->getName().c_str());
			return;
		}
		if (status){
			KernelSubsystemCommand::Ptr cmd = d->tm->createOpenDiaphragmValveCommand(TMCavityValveOpening::TMCavity_Fast);
			cmd->setOrigin("GUI");
			cmd->addListener(d);
			d->tm->startCommand(cmd);
		}
		else{
			KernelSubsystemCommand::Ptr cmd = d->tm->createCloseDiaphragmValveCommand(TMCavityValveOpening::TMCavity_Fast);
			cmd->setOrigin("GUI");
			cmd->addListener(d);
			d->tm->startCommand(cmd);
		}
	}
	void QBreakVacuumSubsystemWidget::onTMGMFMClicked(bool status){
		Q_D(QBreakVacuumSubsystemWidget);
		if (d->tm->getState() != IKernelSubSystem::State::SUB_NORMAL){
			logError(d->tm->getName().c_str(), "模组：%s状态异常，请先复位", d->tm->getName().c_str());
			return;
		}
		if (status){
			KernelSubsystemCommand::Ptr cmd = d->tm->createOpenDiaphragmValveCommand(TMCavityValveOpening::TMCavity_Slow);
			cmd->setOrigin("GUI");
			cmd->addListener(d);
			d->tm->startCommand(cmd);
		}
		else{
			KernelSubsystemCommand::Ptr cmd = d->tm->createCloseDiaphragmValveCommand(TMCavityValveOpening::TMCavity_Slow);
			cmd->setOrigin("GUI");
			cmd->addListener(d);
			d->tm->startCommand(cmd);
		}
	}
	void QBreakVacuumSubsystemWidget::onLLAGMFKClicked(bool status){
		Q_D(QBreakVacuumSubsystemWidget);
		if (d->lk1->getState() != IKernelSubSystem::State::SUB_NORMAL){
			logError(d->lk1->getName().c_str(), "模组：%s状态异常，请先复位", d->lk1->getName().c_str());
			return;
		}
		if (status){
			KernelSubsystemCommand::Ptr cmd = d->lk1->createOpenDiaphragmValveCommand(LoadLockValveOpening::LoadLock_Fast);
			cmd->setOrigin("GUI");
			cmd->addListener(d);
			d->lk1->startCommand(cmd);
		}
		else{
			KernelSubsystemCommand::Ptr cmd = d->lk1->createCloseDiaphragmValveCommand(LoadLockValveOpening::LoadLock_Fast);
			cmd->setOrigin("GUI");
			cmd->addListener(d);
			d->lk1->startCommand(cmd);
		}
	}
	void QBreakVacuumSubsystemWidget::onLLAGMFMClicked(bool status){
		Q_D(QBreakVacuumSubsystemWidget);
		if (d->lk1->getState() != IKernelSubSystem::State::SUB_NORMAL){
			logError(d->lk1->getName().c_str(), "模组：%s状态异常，请先复位", d->lk1->getName().c_str());
			return;
		}
		if (status){
			KernelSubsystemCommand::Ptr cmd = d->lk1->createOpenDiaphragmValveCommand(LoadLockValveOpening::LoadLock_Slow);
			cmd->setOrigin("GUI");
			cmd->addListener(d);
			d->lk1->startCommand(cmd);
		}
		else{
			KernelSubsystemCommand::Ptr cmd = d->lk1->createCloseDiaphragmValveCommand(LoadLockValveOpening::LoadLock_Slow);
			cmd->setOrigin("GUI");
			cmd->addListener(d);
			d->lk1->startCommand(cmd);
		}
	}
	void QBreakVacuumSubsystemWidget::onLLBGMFKClicked(bool status){
		Q_D(QBreakVacuumSubsystemWidget);
		if (d->lk2->getState() != IKernelSubSystem::State::SUB_NORMAL){
			logError(d->lk2->getName().c_str(), "模组：%s状态异常，请先复位", d->lk2->getName().c_str());
			return;
		}
		if (status){
			KernelSubsystemCommand::Ptr cmd = d->lk2->createOpenDiaphragmValveCommand(LoadLockValveOpening::LoadLock_Fast);
			cmd->setOrigin("GUI");
			cmd->addListener(d);
			d->lk2->startCommand(cmd);
		}
		else{
			KernelSubsystemCommand::Ptr cmd = d->lk2->createCloseDiaphragmValveCommand(LoadLockValveOpening::LoadLock_Fast);
			cmd->setOrigin("GUI");
			cmd->addListener(d);
			d->lk2->startCommand(cmd);
		}
	}
	void QBreakVacuumSubsystemWidget::onLLBGMFMClicked(bool status){
		Q_D(QBreakVacuumSubsystemWidget);
		if (d->lk2->getState() != IKernelSubSystem::State::SUB_NORMAL){
			logError(d->lk2->getName().c_str(), "模组：%s状态异常，请先复位", d->lk2->getName().c_str());
			return;
		}
		if (status){
			KernelSubsystemCommand::Ptr cmd = d->lk2->createOpenDiaphragmValveCommand(LoadLockValveOpening::LoadLock_Slow);
			cmd->setOrigin("GUI");
			cmd->addListener(d);
			d->lk2->startCommand(cmd);
		}
		else{
			KernelSubsystemCommand::Ptr cmd = d->lk2->createCloseDiaphragmValveCommand(LoadLockValveOpening::LoadLock_Slow);
			cmd->setOrigin("GUI");
			cmd->addListener(d);
			d->lk2->startCommand(cmd);
		}
	}

	void QBreakVacuumSubsystemWidget::onAttributeUpdate()throw(KernelException){
		Q_D(QBreakVacuumSubsystemWidget);
		d->ui->tm_cavity_current_vacuum_value_let->setText(QString::number(d->tm->getTMCavityVacuumValue(), 'e', 3).append("Pa"));
		d->ui->lla_current_vacuum_value_let->setText(QString::number(d->lk1->getVacuumValue(), 'e', 3).append("Pa"));
		d->ui->llb_current_vacuum_value_let->setText(QString::number(d->lk2->getVacuumValue(), 'e', 3).append("Pa"));

		if (d->tm->getFastDiaphragmValveOpend()){
//			d->ui->gmfk_tm_w_1->setWaterDirection(1);//流动方向,0从左往右，1从右往左，其他停止流动
//			d->ui->gmfk_tm_h_1->setWaterDirection(0);
			d->ui->gmfm_tm_w_1->setWaterDirection(1);
//			d->ui->gmfk_tm->open();
			d->gmfk_tm_ckb->setChecked(true);
		}
		else{
//			d->ui->gmfk_tm_w_1->setWaterDirection(2);
//			d->ui->gmfk_tm_h_1->setWaterDirection(2);
//			d->ui->gmfk_tm->close();
			d->gmfk_tm_ckb->setChecked(false);
		}

		if (d->tm->getSlowDiaphragmValveOpend()){
			d->ui->gmfm_tm_w_1->setWaterDirection(1);
			d->ui->gmfm_tm->open();
			d->gmfm_tm_ckb->setChecked(true);
		}
		else{
			d->ui->gmfm_tm_w_1->setWaterDirection(2);
			d->ui->gmfm_tm->close();
			d->gmfm_tm_ckb->setChecked(false);
		}

		


		if (d->lk1->getFastDiaphragmValveOpend()){
			d->ui->gmfk_lla_w_1->setWaterDirection(1);
			d->ui->gmfk_lla_h_1->setWaterDirection(0);
			d->ui->gmfm_lla_w_1->setWaterDirection(1);
			d->ui->gmfk_lla->open();
			d->gmfk_lla_ckb->setChecked(true);
		}
		else{
			d->ui->gmfk_lla_w_1->setWaterDirection(2);
			d->ui->gmfk_lla_h_1->setWaterDirection(2);
			d->ui->gmfk_lla->close();
			d->gmfk_lla_ckb->setChecked(false);
		}

		if (d->lk1->getSlowDiaphragmValveOpend()){
			d->ui->gmfm_lla_w_1->setWaterDirection(1);
			d->ui->gmfm_lla->open();
			d->gmfm_lla_ckb->setChecked(true);
		}
		else{
			d->ui->gmfm_lla_w_1->setWaterDirection(2);
			d->ui->gmfm_lla->close();
			d->gmfm_lla_ckb->setChecked(false);
		}


		if (d->lk2->getFastDiaphragmValveOpend()){
			d->ui->gmfk_llb_w_1->setWaterDirection(1);
			d->ui->gmfk_llb_h_1->setWaterDirection(0);
			d->ui->gmfm_llb_w_1->setWaterDirection(1);
			d->ui->gmfk_llb->open();
			d->gmfk_llb_ckb->setChecked(true);
		}
		else{
			d->ui->gmfk_llb_w_1->setWaterDirection(2);
			d->ui->gmfk_llb_h_1->setWaterDirection(2);
			d->ui->gmfk_llb->close();
			d->gmfk_llb_ckb->setChecked(false);
		}

		if (d->lk2->getSlowDiaphragmValveOpend()){
			d->ui->gmfm_llb_w_1->setWaterDirection(1);
			d->ui->gmfm_llb->open();
			d->gmfm_llb_ckb->setChecked(true);
		}
		else{
			d->ui->gmfm_llb_w_1->setWaterDirection(2);
			d->ui->gmfm_llb->close();
			d->gmfm_llb_ckb->setChecked(false);
		}
		
	}

	void QBreakVacuumSubsystemWidget::showMessage(const QString& message){
		QMessageBox::information(this, "warn", message);
	}

	void QBreakVacuumSubsystemWidget::onOpenTMCavityAutoBreakVacuumCommand(){
		Q_D(QBreakVacuumSubsystemWidget);
		if (d->tm->getState() != IKernelSubSystem::State::SUB_NORMAL){
			logError(d->tm->getName().c_str(), "模组：%s状态异常，请先复位", d->tm->getName().c_str());
			return;
		}
		KernelSubsystemCommand::Ptr cmd = d->pump->createOpenTMCavityAutoVacuumCommand();
		cmd->setOrigin("GUI");
		cmd->addListener(d);
		d->pump->startCommand(cmd);
	}
	void QBreakVacuumSubsystemWidget::onOpenLoadLock1AutoBreakVacuumCommand(){
		Q_D(QBreakVacuumSubsystemWidget);
		if (d->lk1->getState() != IKernelSubSystem::State::SUB_NORMAL){
			logError(d->lk1->getName().c_str(), "模组：%s状态异常，请先复位", d->lk1->getName().c_str());
			return;
		}
		KernelSubsystemCommand::Ptr cmd = d->lk1->createAutoBreakVacuumCommand();
		cmd->setOrigin("GUI");
		cmd->addListener(d);
		d->lk1->startCommand(cmd);
	}
	void QBreakVacuumSubsystemWidget::onOpenLoadLock2AutoBreakVacuumCommand(){
		Q_D(QBreakVacuumSubsystemWidget);
		if (d->lk2->getState() != IKernelSubSystem::State::SUB_NORMAL){
			logError(d->lk2->getName().c_str(), "模组：%s状态异常，请先复位", d->lk2->getName().c_str());
			return;
		}
		KernelSubsystemCommand::Ptr cmd = d->lk2->createAutoBreakVacuumCommand();
		cmd->setOrigin("GUI");
		cmd->addListener(d);
		d->lk2->startCommand(cmd);
	}
}