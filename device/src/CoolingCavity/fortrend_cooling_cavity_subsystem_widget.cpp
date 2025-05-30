
/**
* @file            fortrend_cooling_cavity_subsystem.h
* @brief           Fortrend CoolingCavity widget
* @author		   xielonghua
*/

// Library: Fortrend
// Package: SubSystem//CoolingCavity

#include  "CoolingCavity/fortrend_cooling_cavity_subsystem_widget.h"
#include  "Kernel/kernel.h"
#include  "Kernel/kernel_subsystem_reset_command.h"
#include  "Kernel/kernel_subsystem_update_command.h"
#include  "Kernel/CoreUI/kernel_subsystem_status_widget.h"  
#include  "Kernel/FortrendUI/slot_widget.h"
#include  "Kernel/FortrendUI/cassette_widget.h"
#include  "Kernel/Fortrend/robot_wafer_abstract_subsystem.h"
#include  "Kernel/Fortrend/fortrend_cassette_manager.h"
#include  "Kernel/Fortrend/cassette.h"

#include  "device/ui_fortrend_cooling_cavity_subsystem_widget.h"

#include <QRadioButton>
#include <QSpacerItem>
#include <QMessageBox>
#include <QCheckBox>

 
namespace FC{
/**
* QCoolingCavitySubsystemWidgetPrivate
**/
class QCoolingCavitySubsystemWidgetPrivate{
	Q_DECLARE_PUBLIC(QCoolingCavitySubsystemWidget)
public:
	QCoolingCavitySubsystemWidgetPrivate(QCoolingCavitySubsystemWidget* p);
public:
	Ui::FortrendCoolingCavitySubsystem* ui;
	QCoolingCavitySubsystemWidget* q_ptr;

	//std::vector<QCheckBox*> arm_stat;
	std::vector<QCheckBox*> input_checkboxs;
};

QCoolingCavitySubsystemWidgetPrivate::QCoolingCavitySubsystemWidgetPrivate(
	QCoolingCavitySubsystemWidget* p)
:q_ptr(p){

}






/**
* QCoolingCavitySubsystemWidget
**/
QCoolingCavitySubsystemWidget::QCoolingCavitySubsystemWidget(
	const std::shared_ptr<FortrendCoolingCavitySubsystem>& robot,
	QWidget*parent) 
	: QAbstractSubsystemWidget<FortrendCoolingCavitySubsystem>(robot, parent)
	, d_ptr(new QCoolingCavitySubsystemWidgetPrivate(this)){

	Q_D(QCoolingCavitySubsystemWidget);
	d->ui = new Ui::FortrendCoolingCavitySubsystem;
	d->ui->setupUi(this);

	init();
	
	onAttributeUpdate();

	connect(d->ui->reset_btn, &QAbstractButton::clicked, this, &QCoolingCavitySubsystemWidget::onReset);
	connect(d->ui->generate_btn, &QAbstractButton::clicked, this, &QCoolingCavitySubsystemWidget::onGetStatus);
	connect(d->ui->cooling_btn, &QAbstractButton::clicked, this, &QCoolingCavitySubsystemWidget::onCooling);
}

QCoolingCavitySubsystemWidget::~QCoolingCavitySubsystemWidget(){

}


void QCoolingCavitySubsystemWidget::init(){
	Q_D(QCoolingCavitySubsystemWidget);

	//subsystem status
	QKernelSubsystemStatusWidget* status_widget = new QKernelSubsystemStatusWidget(getSubsystem());
	d->ui->right_verticalLayout->insertWidget(0, status_widget);
	//connect
	connect(status_widget, &QKernelSubsystemStatusWidget::onStatusUpdate, this, [=](){
		onAttributeUpdate();
	});

	auto cassManager = getSubsystem()->getKernel()->getKernelModule<FortrendCassetteManager>();
	auto cass = cassManager->getCassette(getSubsystem().get());


	//add virtual input
	for (int i = 0; i < getSubsystem()->inputCount(); i++){
		QCheckBox* button = new QCheckBox(QString::fromStdString(getSubsystem()->getInputName(i)));
		button->setObjectName("io_object");
		button->setEnabled(false);
		d->ui->inputs_layout->addWidget(button, i / 4, i % 4, 1, 1);
		d->input_checkboxs.push_back(button);
	}

	if (cass){
		QWidget* cassette_Widget = new QFortrendCassetteWidget(cass, cassManager, true, true, 25, this); //max row count = 25
		d->ui->center_layout->addWidget(cassette_Widget);
		
	}
}


void QCoolingCavitySubsystemWidget::onReset(){
	Q_D(QCoolingCavitySubsystemWidget);
	KernelSubsystemCommand::Ptr cmd = getSubsystem()->createResetCommand();
	executeCommand(getSubsystem(), cmd);
}


void QCoolingCavitySubsystemWidget::onGetStatus(){
	Q_D(QCoolingCavitySubsystemWidget);
	KernelSubsystemCommand::Ptr cmd = getSubsystem()->createUpdateCommand();
	executeCommand(getSubsystem(), cmd);
}


void QCoolingCavitySubsystemWidget::onCooling(){
	Q_D(QCoolingCavitySubsystemWidget);
	//TODO open box
	
	KernelSubsystemCommand::Ptr cmd = getSubsystem()->createCoolingCommand();
	executeCommand(getSubsystem(), cmd);
	
}

void QCoolingCavitySubsystemWidget::onAttributeUpdate(){
	Q_D(QCoolingCavitySubsystemWidget);
	//update input
	for (int i = 0; i < d->input_checkboxs.size(); i++){
		d->input_checkboxs.at(i)->setChecked(getSubsystem()->getInput(i));
		
	}

	//update object stat
	/*for (int i = 0; i < d->arm_stat.size(); i++){
		d->arm_stat.at(i)->setChecked(getSubsystem()->hasObject(i));
	}*/
}

}
