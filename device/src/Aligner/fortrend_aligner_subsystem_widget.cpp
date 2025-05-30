
/**
* @file            fortrend_aligner_subsystem.h
* @brief           Fortrend Aligner widget
* @author		   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/Aligner

#include  "Aligner/fortrend_aligner_subsystem_widget.h"
#include  "Kernel/kernel.h"
#include  "Kernel/kernel_subsystem_reset_command.h"
#include  "Kernel/kernel_subsystem_update_command.h"
#include  "Kernel/CoreUI/kernel_subsystem_status_widget.h"  
#include  "Kernel/FortrendUI/slot_widget.h"
#include  "Kernel/FortrendUI/cassette_widget.h"
#include  "Kernel/Fortrend/fortrend_cassette_manager.h"
#include  "Kernel/Fortrend/cassette.h"

#include  "Kernel/FortrendUI/aligner_status_widget.h"
#include  "Kernel/FortrendUI/mapper_status_widget.h"

#include  "device/ui_fortrend_aligner_subsystem_widget.h"

#include <QRadioButton>
#include <QSpacerItem>
#include <QMessageBox>
#include <QCheckBox>
#if _MSC_VER >1600
#pragma execution_character_set("utf-8")
#endif
 
namespace FC{
/**
* QAlignerSubsystemWidgetPrivate
**/
class QAlignerSubsystemWidgetPrivate{
	Q_DECLARE_PUBLIC(QAlignerSubsystemWidget)
public:
	QAlignerSubsystemWidgetPrivate(QAlignerSubsystemWidget* p);
public:
	Ui::FortrendAlignerSubsystem* ui;
	QAlignerSubsystemWidget* q_ptr;

	//std::vector<QCheckBox*> arm_stat;
	std::vector<QCheckBox*> input_checkboxs;
};

QAlignerSubsystemWidgetPrivate::QAlignerSubsystemWidgetPrivate(
	QAlignerSubsystemWidget* p)
:q_ptr(p){

}






/**
* QAlignerSubsystemWidget
**/
QAlignerSubsystemWidget::QAlignerSubsystemWidget(
	const std::shared_ptr<FortrendAlignerSubsystem>& robot,
	QWidget*parent) 
	: QAbstractSubsystemWidget<FortrendAlignerSubsystem>(robot, parent)
	, d_ptr(new QAlignerSubsystemWidgetPrivate(this)){

	Q_D(QAlignerSubsystemWidget);
	d->ui = new Ui::FortrendAlignerSubsystem;
	d->ui->setupUi(this);

	//add aligner status widget
	/*d->ui->wafer_state_layout->addWidget(new QFortrendAlignerStatusWidget(robot, robot->getKernel(), 200));
	QList<Cassette::Mapping> showMaps = {
		Cassette::Mapping::Present,
		Cassette::Mapping::Empty,
		Cassette::Mapping::Unknown
	};
	d->ui->wafer_state_layout->addWidget(new QFortrendMapperStatusWidget(showMaps));
	d->ui->wafer_state_layout->addStretch();*/

	init();
	
	onAttributeUpdate();

	connect(d->ui->reset_btn, &QAbstractButton::clicked, this, &QAlignerSubsystemWidget::onReset);
	connect(d->ui->generate_btn, &QAbstractButton::clicked, this, &QAlignerSubsystemWidget::onGetStatus);
	connect(d->ui->align_btn, &QAbstractButton::clicked, this, &QAlignerSubsystemWidget::onAlign);

}

QAlignerSubsystemWidget::~QAlignerSubsystemWidget(){

}


void QAlignerSubsystemWidget::init(){
	Q_D(QAlignerSubsystemWidget);

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


}

void QAlignerSubsystemWidget::onReset(){
	Q_D(QAlignerSubsystemWidget);
	KernelSubsystemCommand::Ptr cmd = getSubsystem()->createResetCommand();
	executeCommand(getSubsystem(), cmd);
}


void QAlignerSubsystemWidget::onGetStatus(){
	Q_D(QAlignerSubsystemWidget);
	KernelSubsystemCommand::Ptr cmd = getSubsystem()->createUpdateCommand();
	executeCommand(getSubsystem(), cmd);
}


void QAlignerSubsystemWidget::onAlign(){
	Q_D(QAlignerSubsystemWidget);
	//TODO open box
	
	KernelSubsystemCommand::Ptr cmd = getSubsystem()->createAlignCommand();
	executeCommand(getSubsystem(), cmd);
	
}

void QAlignerSubsystemWidget::onAttributeUpdate(){
	Q_D(QAlignerSubsystemWidget);
	//update input
	for (int i = 0; i < d->input_checkboxs.size(); i++){
		d->input_checkboxs.at(i)->setChecked(getSubsystem()->getInput(i));
		
	}
	bool res = false;
	int length = 0;
	int angle =0;
	getSubsystem()->getAlignResult(res, length, angle);
	if (res)
	{
		d->ui->align_result_let->setStyleSheet("background-color: lime;");
		d->ui->align_result_let->setText(QString::fromStdString("Sucess"));
	}
	else
	{
		d->ui->align_result_let->setStyleSheet("background-color: red;");
		d->ui->align_result_let->setText(QString::fromStdString("Fail"));
	}
	d->ui->length_let->setText(QString::number(length));
	d->ui->angle_let->setText(QString::number(angle));
	//update object stat
	/*for (int i = 0; i < d->arm_stat.size(); i++){
		d->arm_stat.at(i)->setChecked(getSubsystem()->hasObject(i));
	}*/
}

}
