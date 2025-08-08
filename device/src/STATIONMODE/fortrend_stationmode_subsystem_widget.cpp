
/**
* @file    fortrend_StationMode_subsystem.h
* @brief   Fortrend StationMode widget
* @author  xielonghua
*/

// Library: Fortrend
// Package: SubSystem/STATIONMODE

#include  "STATIONMODE/fortrend_StationMode_subsystem_widget.h"
#include  "Kernel/kernel.h"
#include  "Kernel/kernel_subsystem_reset_command.h"
#include  "Kernel/kernel_subsystem_update_command.h"
#include  "Kernel/CoreUI/kernel_subsystem_status_widget.h"  
#include  "Kernel/FortrendUI/slot_widget.h"
#include  "Kernel/FortrendUI/cassette_widget.h"
#include  "Kernel/Fortrend/robot_wafer_abstract_subsystem.h"
#include  "Kernel/Fortrend/fortrend_cassette_manager.h"
#include  "Kernel/Fortrend/cassette.h"
#include  "Kernel/Fortrend/abstract_output_command.h"

#include  "device/ui_fortrend_StationMode_subsystem_widget.h"

 
namespace FC{
/**
* QSTATIONMODESubsystemWidgetPrivate
**/
class QSTATIONMODESubsystemWidgetPrivate{
    Q_DECLARE_PUBLIC(QSTATIONMODESubsystemWidget)
public:
    QSTATIONMODESubsystemWidgetPrivate(QSTATIONMODESubsystemWidget* p);
public:
    Ui::STATIONMODESubsystemWidget* ui;
    QSTATIONMODESubsystemWidget* q_ptr;
};

QSTATIONMODESubsystemWidgetPrivate::QSTATIONMODESubsystemWidgetPrivate(
    QSTATIONMODESubsystemWidget* p)
:q_ptr(p){

}



/**
* QSTATIONMODESubsystemWidget
**/
QSTATIONMODESubsystemWidget::QSTATIONMODESubsystemWidget(
    const std::shared_ptr<FortrendSTATIONMODESubsystem>& robot,
    QWidget*parent) 
    : QAbstractSubsystemWidget<FortrendSTATIONMODESubsystem>(robot, parent)
    , d_ptr(new QSTATIONMODESubsystemWidgetPrivate(this)){

    Q_D(QSTATIONMODESubsystemWidget);
    d->ui = new Ui::STATIONMODESubsystemWidget;
    d->ui->setupUi(this);

    //subsystem status
    QKernelSubsystemStatusWidget* status_widget = new QKernelSubsystemStatusWidget(getSubsystem());
    d->ui->right_verticalLayout->insertWidget(0, status_widget);

    //connect
    connect(status_widget, &QKernelSubsystemStatusWidget::onStatusUpdate, this, [=]() {
        //onAttributeUpdate();
    });

    auto cassManager = getSubsystem()->getKernel()->getKernelModule<FortrendCassetteManager>();
    auto cass = cassManager->getCassette(getSubsystem().get());

    if (cass) {
        QWidget* cassette_Widget = new QFortrendCassetteWidget(cass, cassManager, true, true, 25, this); //max row count = 25
        d->ui->center_layout->addWidget(cassette_Widget);

    }
}

QSTATIONMODESubsystemWidget::~QSTATIONMODESubsystemWidget(){

}



void QSTATIONMODESubsystemWidget::onReset(){
    Q_D(QSTATIONMODESubsystemWidget);
    KernelSubsystemCommand::Ptr cmd = getSubsystem()->createResetCommand();
    executeCommand(getSubsystem(), cmd);
}


void QSTATIONMODESubsystemWidget::onGetStatus(){
    Q_D(QSTATIONMODESubsystemWidget);
    KernelSubsystemCommand::Ptr cmd = getSubsystem()->createUpdateCommand();
    executeCommand(getSubsystem(), cmd);
}


void QSTATIONMODESubsystemWidget::onOutput(){
    Q_D(QSTATIONMODESubsystemWidget);
    int index = -1; //outout index
    bool outStat = false;  //output status
    KernelSubsystemCommand::Ptr cmd = getSubsystem()->createOutputCommand(index,outStat);
    executeCommand(getSubsystem(), cmd);
}



//all special commands

void QSTATIONMODESubsystemWidget::onResetCommand(){
    Q_D(QSTATIONMODESubsystemWidget);
    KernelSubsystemCommand::Ptr cmd = getSubsystem()->createResetCommand();
    executeCommand(getSubsystem(), cmd);
}
 

}