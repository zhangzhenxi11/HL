// Library: FortrendUI
// Package: Subsystem/Aligner
//
// abstract aligner subsystem widget
//
// author xielonghua
//
#include  "EFEM/efem_aligner_subsystem.h"
#include  "EFEM/efem_aligner_abstract_subsystem_widget.h" 
#include "Kernel/kernel_api.h"
#include  "Kernel/kernel.h"
#include  "Kernel/kernel_listener.h"
#include  "Kernel/kernel_subsystem_reset_command.h"
#include  "Kernel/kernel_subsystem_update_command.h"
#include  "Kernel/FortrendUI/aligner_status_widget.h"
#include  "Kernel/FortrendUI/mapper_status_widget.h"
#include  "Kernel/CoreUI/kernel_subsystem_status_widget.h"  
#include  "device/ui_efem_aligner_abstract_subsystem_widget.h"
#include  "Kernel/Fortrend/aligner_abstract_command.h"
#include <QComboBox>
#include <QMessageBox>

//#include <QOverload>
#if _MSC_VER >1600
#pragma execution_character_set("utf-8")
#endif

KERNEL_NS_BEGIN
/**
* QEFEMAlignerAbstractSubsystemWidgetPrivate
*/
class QEFEMAlignerAbstractSubsystemWidgetPrivate {
	Q_DECLARE_PUBLIC(QEFEMAlignerAbstractSubsystemWidget)
public:
	QEFEMAlignerAbstractSubsystemWidgetPrivate(QEFEMAlignerAbstractSubsystemWidget* p);
	virtual void onAttributeChange(const FortrendStation* arg);

private:
	Ui::EFEMAlignerAbstractSubsystemWidget* ui;
	QEFEMAlignerAbstractSubsystemWidget* q_ptr;
	std::shared_ptr<EFEMAlignerSubsystem> aligner; //模组指针，要是基类指针还是找不到成员函数
	

};


QEFEMAlignerAbstractSubsystemWidgetPrivate::QEFEMAlignerAbstractSubsystemWidgetPrivate(
	QEFEMAlignerAbstractSubsystemWidget* p)
	:q_ptr(p){

}

void QEFEMAlignerAbstractSubsystemWidgetPrivate::onAttributeChange(const FortrendStation* arg)
{
	QMetaObject::invokeMethod(q_ptr, "updateOcrInfo", Qt::AutoConnection);
}



/**
* QEFEMAlignerAbstractSubsystemWidget
*/
QEFEMAlignerAbstractSubsystemWidget::QEFEMAlignerAbstractSubsystemWidget(
	const std::shared_ptr<AlignerAbstractSubsystem>& aligner,
	QWidget* parent)
	:d_ptr(new QEFEMAlignerAbstractSubsystemWidgetPrivate(this))
	,QAbstractSubsystemWidget<AlignerAbstractSubsystem>(aligner,parent)
{
	Q_D(QEFEMAlignerAbstractSubsystemWidget);
	d->ui = new Ui::EFEMAlignerAbstractSubsystemWidget;
	d->ui->setupUi(this);

	d->aligner = std::dynamic_pointer_cast<EFEMAlignerSubsystem>(aligner);//模组指针 ，下行转换

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
	auto precision = aligner->getPrecision();//精度
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

	//connect(d->ui->dirct_cbx, QOverload<int>::of(&QComboBox::currentIndexChanged), this,&QEFEMAlignerAbstractSubsystemWidget::readOcr);

	connect(d->ui->dirct_cbx,
		static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
		this,
		&QEFEMAlignerAbstractSubsystemWidget::readOcr);


	//connect(d->ui->read_ocr_btn, &QAbstractButton::clicked, this, [=]()
	//{
	//	d->aligner->GetOCRCommand(1);
	//});
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

void QEFEMAlignerAbstractSubsystemWidget::readOcr(int index)
{
	Q_D(QEFEMAlignerAbstractSubsystemWidget);
	KernelSubsystemCommand::Ptr cmd = d->aligner->createOcrCommand(index);
	executeCommand(getSubsystem(), cmd);
}

void KERNEL_NS::QEFEMAlignerAbstractSubsystemWidget::updateOcrInfo()
{
	Q_D(QEFEMAlignerAbstractSubsystemWidget);

	d->ui->ocr_lineEdit->setText(QString(d->aligner->getWaferID().c_str()));
}


KERNEL_NS_END
