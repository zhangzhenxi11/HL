
/**
* @file            fortrend_pump_subsystem.h
* @brief           Fortrend Pump widget
* @author		   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/Pump

#include  "Pump/fortrend_pump_subsystem_widget.h"
#include  "Kernel/kernel.h"
#include  "Kernel/kernel_subsystem_reset_command.h"
#include  "Kernel/kernel_subsystem_update_command.h"
#include  "Kernel/CoreUI/kernel_subsystem_status_widget.h"  
#include  "Kernel/FortrendUI/slot_widget.h"
#include  "Kernel/FortrendUI/cassette_widget.h"
#include  "Kernel/Fortrend/fortrend_cassette_manager.h"
#include  "Kernel/Fortrend/cassette.h"


#include  "device/ui_fortrend_pump_subsystem_widget.h"

#include <QRadioButton>
#include <QSpacerItem>
#include <QMessageBox>
#include <QCheckBox>

#include <qdebug>
#if _MSC_VER >1600
#pragma execution_character_set("utf-8")
#endif

namespace FC{
	/**
	* QPumpSubsystemWidgetPrivate
	**/
	class QPumpSubsystemWidgetPrivate{
		Q_DECLARE_PUBLIC(QPumpSubsystemWidget)
	public:
		QPumpSubsystemWidgetPrivate(QPumpSubsystemWidget* p);
	public:
		Ui::FortrendPumpSubsystem* ui;
		QPumpSubsystemWidget* q_ptr;

		//std::vector<QCheckBox*> arm_stat;
		std::vector<QCheckBox*> input_checkboxs;
		QCheckBox* mechanical_pump_ckb = 0;
		QCheckBox* molecular_pump_ckb_tm = 0;
		QCheckBox* molecular_pump_reach_speed_ckb_tm = 0;
		QCheckBox* molecular_pump_ckb_lla = 0;
		QCheckBox* molecular_pump_reach_speed_ckb_lla = 0;
		QCheckBox* molecular_pump_ckb_llb = 0;
		QCheckBox* molecular_pump_reach_speed_ckb_llb = 0;
		QCheckBox* mechanical_pump_alarm_ckb = 0;
		QCheckBox* mechanical_pump_warn_ckb = 0;
		QCheckBox* mechanical_pump_Acc_ckb = 0;
		QCheckBox* mechanical_pump_running_ckb = 0;

	};

	QPumpSubsystemWidgetPrivate::QPumpSubsystemWidgetPrivate(
		QPumpSubsystemWidget* p)
		:q_ptr(p){

	}






	/**
	* QPumpSubsystemWidget
	**/
	QPumpSubsystemWidget::QPumpSubsystemWidget(
		const std::shared_ptr<FortrendPumpSubsystem>& robot,
		QWidget*parent)
		: QAbstractSubsystemWidget<FortrendPumpSubsystem>(robot, parent)
		, d_ptr(new QPumpSubsystemWidgetPrivate(this)){

		Q_D(QPumpSubsystemWidget);
		d->ui = new Ui::FortrendPumpSubsystem;
		d->ui->setupUi(this);

		//add pump status widget
		/*d->ui->wafer_state_layout->addWidget(new QFortrendPumpStatusWidget(robot, robot->getKernel(), 200));
		QList<Cassette::Mapping> showMaps = {
		Cassette::Mapping::Present,
		Cassette::Mapping::Empty,
		Cassette::Mapping::Unknown
		};
		d->ui->wafer_state_layout->addWidget(new QFortrendMapperStatusWidget(showMaps));
		d->ui->wafer_state_layout->addStretch();*/

		init();
		d->ui->molecular_open_lla_btn->setVisible(false);
		d->ui->molecular_close_lla_btn->setVisible(false);
		d->ui->molecular_open_llb_btn->setVisible(false);
		d->ui->molecular_close_llb_btn->setVisible(false);
		d->ui->molecular_open_tm_btn->setVisible(false);
		d->ui->molecular_close_tm_btn->setVisible(false);
		d->ui->generate_btn->setVisible(false);

		onAttributeUpdate();
		//connect(d->ui->generate_btn, &QAbstractButton::clicked, this, &QPumpSubsystemWidget::onGetStatus);
		connect(d->ui->mechanical_open_btn, &QAbstractButton::clicked, this, &QPumpSubsystemWidget::onMechanicalOpen);
		connect(d->ui->mechanical_close_btn, &QAbstractButton::clicked, this, &QPumpSubsystemWidget::onMechanicalClose);

	/*	connect(d->ui->molecular_open_lla_btn, &QAbstractButton::clicked, this, &QPumpSubsystemWidget::onMolecularOpenLLA);
		connect(d->ui->molecular_close_lla_btn, &QAbstractButton::clicked, this, &QPumpSubsystemWidget::onMolecularCloseLLA);

		connect(d->ui->molecular_open_llb_btn, &QAbstractButton::clicked, this, &QPumpSubsystemWidget::onMolecularOpenLLB);
		connect(d->ui->molecular_close_llb_btn, &QAbstractButton::clicked, this, &QPumpSubsystemWidget::onMolecularCloseLLB);

		connect(d->ui->molecular_open_tm_btn, &QAbstractButton::clicked, this, &QPumpSubsystemWidget::onMolecularOpenTM);
		connect(d->ui->molecular_close_tm_btn, &QAbstractButton::clicked, this, &QPumpSubsystemWidget::onMolecularCloseTM);*/


		connect(d->ui->open_loadlock1_vacuum_btn, &QAbstractButton::clicked, this, &QPumpSubsystemWidget::onOpenLoadLock1AutoVacuum);
		connect(d->ui->open_loadlock2_vacuum_btn, &QAbstractButton::clicked, this, &QPumpSubsystemWidget::onOpenLoadLock2AutoVacuum);
		connect(d->ui->open_tm_cavity_vacuum_btn, &QAbstractButton::clicked, this, &QPumpSubsystemWidget::onOpenTMCavityAutoVacuum);
		connect(d->ui->clear_error_btn, &QAbstractButton::clicked, this, &QPumpSubsystemWidget::onClearError);
	}

	QPumpSubsystemWidget::~QPumpSubsystemWidget(){

	}


	void QPumpSubsystemWidget::init(){
		Q_D(QPumpSubsystemWidget);

		//subsystem status
		QKernelSubsystemStatusWidget* status_widget = new QKernelSubsystemStatusWidget(getSubsystem());
		d->ui->right_verticalLayout->insertWidget(0, status_widget);
		//connect
		connect(status_widget, &QKernelSubsystemStatusWidget::onStatusUpdate, this, [=](){
			onAttributeUpdate();
		});

		auto cassManager = getSubsystem()->getKernel()->getKernelModule<FortrendCassetteManager>();
		auto cass = cassManager->getCassette(getSubsystem().get());

		if (cass){
			QWidget* cassette_Widget = new QFortrendCassetteWidget(cass, cassManager, true, true, 1, this);
			d->ui->center_layout->addWidget(cassette_Widget);

		}
		//add virtual input
		for (int i = 0; i < getSubsystem()->inputCount(); i++){
			QCheckBox* button = new QCheckBox(QString::fromStdString(getSubsystem()->getInputName(i)));
			button->setObjectName("io_object");
			button->setEnabled(false);
			d->ui->inputs_layout->addWidget(button, i / 4, i % 4, 1, 1);
			d->input_checkboxs.push_back(button);
		}

		d->mechanical_pump_ckb = new QCheckBox(QString("干泵打开"));
		d->mechanical_pump_ckb->setObjectName("io_object");
		d->mechanical_pump_ckb->setEnabled(false);
		d->ui->operation_state_gridLayout->addWidget(d->mechanical_pump_ckb, 0, 0);

		d->molecular_pump_ckb_tm = new QCheckBox(QString("TM分子泵打开"));
		d->molecular_pump_ckb_tm->setObjectName("io_object");
		d->molecular_pump_ckb_tm->setEnabled(false);
		//d->ui->operation_state_gridLayout->addWidget(d->molecular_pump_ckb_tm, 0, 1);

		d->molecular_pump_reach_speed_ckb_tm = new QCheckBox(QString("TM分子泵达到转速"));
		d->molecular_pump_reach_speed_ckb_tm->setObjectName("io_object");
		d->molecular_pump_reach_speed_ckb_tm->setEnabled(false);
		//d->ui->operation_state_gridLayout->addWidget(d->molecular_pump_reach_speed_ckb_tm, 0, 2);

		d->molecular_pump_ckb_lla = new QCheckBox(QString("LLA分子泵打开"));
		d->molecular_pump_ckb_lla->setObjectName("io_object");
		d->molecular_pump_ckb_lla->setEnabled(false);
		//d->ui->operation_state_gridLayout->addWidget(d->molecular_pump_ckb_lla, 1, 0);

		d->molecular_pump_reach_speed_ckb_lla = new QCheckBox(QString("LLA分子泵达到转速"));
		d->molecular_pump_reach_speed_ckb_lla->setObjectName("io_object");
		d->molecular_pump_reach_speed_ckb_lla->setEnabled(false);
		//d->ui->operation_state_gridLayout->addWidget(d->molecular_pump_reach_speed_ckb_lla, 1, 1);

		d->molecular_pump_ckb_llb = new QCheckBox(QString("LLB分子泵打开"));
		d->molecular_pump_ckb_llb->setObjectName("io_object");
		d->molecular_pump_ckb_llb->setEnabled(false);
		//d->ui->operation_state_gridLayout->addWidget(d->molecular_pump_ckb_llb, 1, 2);

		d->molecular_pump_reach_speed_ckb_llb = new QCheckBox(QString("LLB分子泵达到转速"));
		d->molecular_pump_reach_speed_ckb_llb->setObjectName("io_object");
		d->molecular_pump_reach_speed_ckb_llb->setEnabled(false);
		//d->ui->operation_state_gridLayout->addWidget(d->molecular_pump_reach_speed_ckb_llb, 2, 0);
		
		d->mechanical_pump_alarm_ckb = new QCheckBox(QString("干泵报警"));
		d->mechanical_pump_alarm_ckb->setObjectName("io_object");
		d->mechanical_pump_alarm_ckb->setEnabled(false);
		//d->ui->operation_state_gridLayout->addWidget(d->mechanical_pump_alarm_ckb,0,1);


		d->mechanical_pump_warn_ckb = new QCheckBox(QString("干泵警告"));
		d->mechanical_pump_warn_ckb->setObjectName("io_object");
		d->mechanical_pump_warn_ckb->setEnabled(false);
		d->ui->operation_state_gridLayout->addWidget(d->mechanical_pump_warn_ckb, 0, 1);

		d->mechanical_pump_Acc_ckb = new QCheckBox(QString("ACC信号"));
		d->mechanical_pump_Acc_ckb->setObjectName("io_object");
		d->mechanical_pump_Acc_ckb->setEnabled(false);
		d->ui->operation_state_gridLayout->addWidget(d->mechanical_pump_Acc_ckb, 1, 0);

		d->mechanical_pump_running_ckb = new QCheckBox(QString("干泵运行中"));
		d->mechanical_pump_running_ckb->setObjectName("io_object");
		d->mechanical_pump_running_ckb->setEnabled(false);
		d->ui->operation_state_gridLayout->addWidget(d->mechanical_pump_running_ckb,1,1);

	}

	void QPumpSubsystemWidget::onReset(){
		Q_D(QPumpSubsystemWidget);
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createResetCommand();
		executeCommand(getSubsystem(), cmd);
	}


	void QPumpSubsystemWidget::onGetStatus(){
		Q_D(QPumpSubsystemWidget);
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createUpdateCommand();
		executeCommand(getSubsystem(), cmd);
	}


	void QPumpSubsystemWidget::onMechanicalOpen(){
		Q_D(QPumpSubsystemWidget);
		//TODO open box
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createMechanicalOpenCommand();
		executeCommand(getSubsystem(), cmd);

	}

	void QPumpSubsystemWidget::onMechanicalClose(){
		Q_D(QPumpSubsystemWidget);
		//TODO open box
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createMechanicalCloseCommand();
		executeCommand(getSubsystem(), cmd);

	}

	void QPumpSubsystemWidget::onMolecularOpenLLA(){
		Q_D(QPumpSubsystemWidget);
		//TODO open box
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createMolecularOpenCommand("LLA");
		executeCommand(getSubsystem(), cmd);
	}

	void QPumpSubsystemWidget::onMolecularCloseLLA(){
		Q_D(QPumpSubsystemWidget);
		//TODO open box
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createMolecularCloseCommand("LLA");
		executeCommand(getSubsystem(), cmd);
	}

	void QPumpSubsystemWidget::onMolecularOpenLLB(){
		Q_D(QPumpSubsystemWidget);
		//TODO open box
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createMolecularOpenCommand("LLB");
		executeCommand(getSubsystem(), cmd);
	}

	void QPumpSubsystemWidget::onMolecularCloseLLB(){
		Q_D(QPumpSubsystemWidget);
		//TODO open box
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createMolecularCloseCommand("LLB");
		executeCommand(getSubsystem(), cmd);
	}

	void QPumpSubsystemWidget::onMolecularOpenTM(){
		Q_D(QPumpSubsystemWidget);
		//TODO open box
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createMolecularOpenCommand("TM");
		executeCommand(getSubsystem(), cmd);
	}

	void QPumpSubsystemWidget::onMolecularCloseTM(){
		Q_D(QPumpSubsystemWidget);
		//TODO open box
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createMolecularCloseCommand("TM");
		executeCommand(getSubsystem(), cmd);
	}

	void QPumpSubsystemWidget::onOpenLoadLock1AutoVacuum(){
		Q_D(QPumpSubsystemWidget);
		//TODO open 
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createOpenLoadLock1AutoVacuumCommand();
		executeCommand(getSubsystem(), cmd);

	}

	void QPumpSubsystemWidget::onOpenLoadLock2AutoVacuum(){
		Q_D(QPumpSubsystemWidget);
		//TODO open 
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createOpenLoadLock2AutoVacuumCommand();
		executeCommand(getSubsystem(), cmd);

	}

	void QPumpSubsystemWidget::onOpenTMCavityAutoVacuum(){
		Q_D(QPumpSubsystemWidget);
		//TODO open 
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createOpenTMCavityAutoVacuumCommand();
		executeCommand(getSubsystem(), cmd);

	}

	void QPumpSubsystemWidget::onClearError(){
		Q_D(QPumpSubsystemWidget);
		//TODO open box
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createResetCommand();
		executeCommand(getSubsystem(), cmd);

	}


	void QPumpSubsystemWidget::onAttributeUpdate() throw(KernelException){
		Q_D(QPumpSubsystemWidget);
		try{

			d->mechanical_pump_ckb->setChecked(getSubsystem()->getMechanicalPumpOpened());
			//d->ui->molecular_speed_let->setText(QString::fromStdString(std::to_string(getSubsystem()->getMolecularPumpRev())));
			//d->molecular_pump_ckb_lla->setChecked(getSubsystem()->getMolecularPumpOpenedLLA());
			//d->molecular_pump_reach_speed_ckb_lla->setChecked(getSubsystem()->getMolecularPumpReachSpeedLLA());
			//d->molecular_pump_ckb_llb->setChecked(getSubsystem()->getMolecularPumpOpenedLLB());
			//d->molecular_pump_reach_speed_ckb_llb->setChecked(getSubsystem()->getMolecularPumpReachSpeedLLB());
			//d->molecular_pump_ckb_tm->setChecked(getSubsystem()->getMolecularPumpOpenedTM());
			//d->molecular_pump_reach_speed_ckb_tm->setChecked(getSubsystem()->getMolecularPumpReachSpeedTM());
			/*d->mechanical_pump_Acc_ckb->setChecked(getSubsystem()->getMechanicalPumpAcc());*/
			//d->mechanical_pump_alarm_ckb->setChecked(getSubsystem()->getMechanicalPumpHasAlarm());

			d->mechanical_pump_running_ckb->setChecked(getSubsystem()->getMechanicalPumpRunningState());
			d->mechanical_pump_warn_ckb->setChecked(getSubsystem()->getMechanicalPumpHasWarn());

		}
		catch (KernelException& e){
			logError(getSubsystem()->getName().c_str(), e.what());
			//throw e;
		}

	}

}
