// Library: FortrendUI
// Package: Subsystem/Aligner
//
// abstract aligner subsystem widget
//
// author xielonghua
//

#include  "EFEM/efem_aligner_abstract_subsystem_widget.h" 
#include  "Kernel/FortrendUI/aligner_status_widget.h"
#include  "Kernel/FortrendUI/mapper_status_widget.h"

#include  "Kernel/CoreUI/kernel_subsystem_status_widget.h"  
#include  "device/ui_efem_aligner_abstract_subsystem_widget.h"

#include  "Kernel/Fortrend/aligner_abstract_command.h"
#include  "Kernel/kernel_listener.h"
#include  "Kernel/kernel_subsystem_reset_command.h"
#include  "Kernel/kernel_subsystem_update_command.h"


#include <QMessageBox>
 
KERNEL_NS_BEGIN
/**
* QEFEMAlignerAbstractSubsystemWidgetPrivate
*/
class QEFEMAlignerAbstractSubsystemWidgetPrivate {
	Q_DECLARE_PUBLIC(QEFEMAlignerAbstractSubsystemWidget)
public:
	QEFEMAlignerAbstractSubsystemWidgetPrivate(QEFEMAlignerAbstractSubsystemWidget* p);

private:
	Ui::EFEMAlignerAbstractSubsystemWidget* ui;
	QEFEMAlignerAbstractSubsystemWidget* q_ptr;

};


QEFEMAlignerAbstractSubsystemWidgetPrivate::QEFEMAlignerAbstractSubsystemWidgetPrivate(
	QEFEMAlignerAbstractSubsystemWidget* p)
	:q_ptr(p){

}



/**
* QEFEMAlignerAbstractSubsystemWidget
*/
QEFEMAlignerAbstractSubsystemWidget::QEFEMAlignerAbstractSubsystemWidget(const std::shared_ptr<AlignerAbstractSubsystem>& aligner, QWidget* parent)
:d_ptr(new QEFEMAlignerAbstractSubsystemWidgetPrivate(this))
,QAbstractSubsystemWidget<AlignerAbstractSubsystem>(aligner,parent){
	Q_D(QEFEMAlignerAbstractSubsystemWidget);
	d->ui = new Ui::EFEMAlignerAbstractSubsystemWidget;
	d->ui->setupUi(this);

	//subsystem status
	d->ui->right_verticalLayout->insertWidget(0, new QKernelSubsystemStatusWidget(aligner));

	//add aligner status widget
	d->ui->horizontalLayout_2->addWidget(new QFortrendAlignerStatusWidget(aligner, aligner->getKernel(), 200));
	//add status widget
	QList<Cassette::Mapping> showMaps = {
		Cassette::Mapping::Present,
		Cassette::Mapping::Empty,
		Cassette::Mapping::Unknown
	};

	d->ui->horizontalLayout_2->addWidget(new QFortrendMapperStatusWidget(showMaps));
	d->ui->horizontalLayout_2->addStretch();
	auto precision = aligner->getPrecision();//¾«¶È
	d->ui->angle_spinBox->setMinimum(0);
	d->ui->angle_spinBox->setMaximum(360 - precision);
	d->ui->angle_spinBox->setDecimals([precision](){
		int decimals = 0;
		auto tmp = precision;
		while(tmp<1){
			decimals += 1;
			tmp *= 10;
		}
		return decimals;
	}());
	d->ui->angle_spinBox->setSingleStep(precision);


	//connect
	connect(d->ui->reset_btn, &QAbstractButton::clicked, this, &QEFEMAlignerAbstractSubsystemWidget::reset);
	connect(d->ui->align_btn, &QAbstractButton::clicked, this, &QEFEMAlignerAbstractSubsystemWidget::align);
	connect(d->ui->generate_btn, &QAbstractButton::clicked, this, &QEFEMAlignerAbstractSubsystemWidget::generate);
	//connect(d->ui->movstart_btn, &QAbstractButton::clicked, this, &QEFEMAlignerAbstractSubsystemWidget::moveStartPos);
	connect(d->ui->vaccum_on_btn, &QAbstractButton::clicked, this, &QEFEMAlignerAbstractSubsystemWidget::vaccum_on);
	connect(d->ui->vaccum_off_btn, &QAbstractButton::clicked, this, &QEFEMAlignerAbstractSubsystemWidget::vaccum_off);
	connect(d->ui->rotate_btn, &QAbstractButton::clicked, this, &QEFEMAlignerAbstractSubsystemWidget::rotate);
	 
	//config ui
	configUI(getSubsystem()->getConfigure());
}
QEFEMAlignerAbstractSubsystemWidget::~QEFEMAlignerAbstractSubsystemWidget(){ 
	Q_D(QEFEMAlignerAbstractSubsystemWidget);
	delete d->ui;
}
 

void QEFEMAlignerAbstractSubsystemWidget::generate(){
	Q_D(QEFEMAlignerAbstractSubsystemWidget);

	KernelSubsystemCommand::Ptr cmd = getSubsystem()->createUpdateCommand();
	executeCommand(getSubsystem(),cmd);
}

void QEFEMAlignerAbstractSubsystemWidget::reset(){
	Q_D(QEFEMAlignerAbstractSubsystemWidget);
	KernelSubsystemCommand::Ptr cmd = getSubsystem()->createResetCommand();
	executeCommand(getSubsystem(), cmd);
}

void QEFEMAlignerAbstractSubsystemWidget::align(){
	Q_D(QEFEMAlignerAbstractSubsystemWidget);
	KernelSubsystemCommand::Ptr cmd = getSubsystem()->createAlignCommand();
	executeCommand(getSubsystem(), cmd);
}

void QEFEMAlignerAbstractSubsystemWidget::moveStartPos(){
	Q_D(QEFEMAlignerAbstractSubsystemWidget);
	//RNDCommand::Ptr cmd(new RNDBaseCommand("Move To Start Pos","RST","RST"));
	//executeCommand(getSubsystem(), cmd);
}

void QEFEMAlignerAbstractSubsystemWidget::vaccum_on(){
	Q_D(QEFEMAlignerAbstractSubsystemWidget);
	//RNDCommand::Ptr cmd(new RNDBaseCommand("Vaccum ON", "VVN", "VVN"));
	executeCommand(getSubsystem(), getSubsystem()->createVaccOnCommand());
}

void QEFEMAlignerAbstractSubsystemWidget::vaccum_off(){
	Q_D(QEFEMAlignerAbstractSubsystemWidget);
	//RNDCommand::Ptr cmd(new RNDBaseCommand("Vaccum OFF", "VVF", "VVF"));
	executeCommand(getSubsystem(), getSubsystem()->createVaccOffCommand());
}

void QEFEMAlignerAbstractSubsystemWidget::rotate(){
	Q_D(QEFEMAlignerAbstractSubsystemWidget);
	KernelSubsystemCommand::Ptr cmd = getSubsystem()->createRotateCommand(d->ui->angle_spinBox->value());
	executeCommand(getSubsystem(), cmd);

}

KERNEL_NS_END
