// Library: FortrendUI
// Package: Subsystem/Robot
//
// abstract loadport subsystem widget
//
// author xielonghua
//


#include  "EFEM/efem_robot_wafer_abstract_subsystem_widget.h"
#include  "Kernel/FortrendUI/mapper_status_widget.h"
#include  "Kernel/FortrendUI/slot_widget.h"

#include "Kernel/Fortrend/fortrend_cassette_manager.h"
#include "Kernel/kernel.h"

#include  "Kernel/Fortrend/robot_wafer_abstract_subsystem.h"
#include  "Kernel/Fortrend/fortrend_station.h"
#include  "Kernel/Fortrend/fortrend_cassette_manager.h"
#include  "Kernel/Fortrend/robot_getwafer_abstract_command.h"
#include  "Kernel/Fortrend/robot_putwafer_abstract_command.h"
#include  "Kernel/Fortrend/abstract_output_command.h"

#include "Kernel/Fortrend/fortrend_wafer_tool_station.h"

#include  "Kernel/kernel.h"
#include  "Kernel/kernel_subsystem_reset_command.h"
#include  "Kernel/kernel_subsystem_update_command.h"
#include  "Kernel/CoreUI/kernel_subsystem_status_widget.h"  

#include  "device/ui_efem_robot_wafer_abstract_subsystem_widget.h"
#include "Kernel/kernel_log.h"

#include "Poco/String.h"
#include "Poco/Format.h"
#include "Poco/StringTokenizer.h"
#include "Poco/NumberParser.h"

#include <QRadioButton>
#include <QSpacerItem>
#include <QMessageBox>
#include <QCheckBox>

 
KERNEL_NS_BEGIN
/**
* QEFEMRobotWaferAbstractSubsystemWidgetPrivate
**/
class QEFEMRobotWaferAbstractSubsystemWidgetPrivate :public KernelListener<FortrendCassetteManager>{
	Q_DECLARE_PUBLIC(QEFEMRobotWaferAbstractSubsystemWidget)
public:
	QEFEMRobotWaferAbstractSubsystemWidgetPrivate(QEFEMRobotWaferAbstractSubsystemWidget* p);

	void onAttributeChange(const FortrendCassetteManager* arg);
public:
	Ui::EFEMRobotWaferAbstractSubsystemWidget* ui;
	QEFEMRobotWaferAbstractSubsystemWidget* q_ptr;

	std::vector<QCheckBox*> input_checkboxs;
	std::vector<QCheckBox*> arm_stat;
	std::vector<QRadioButton*> arm_select;
	std::shared_ptr<FortrendStation> selected_station;
};

QEFEMRobotWaferAbstractSubsystemWidgetPrivate::QEFEMRobotWaferAbstractSubsystemWidgetPrivate(
	QEFEMRobotWaferAbstractSubsystemWidget* p)
:q_ptr(p){

}
void QEFEMRobotWaferAbstractSubsystemWidgetPrivate::onAttributeChange(const FortrendCassetteManager *cassManager) {
	QMetaObject::invokeMethod(q_ptr, "updateSlotsLayout", Qt::AutoConnection);
}

/**
* QEFEMRobotWaferAbstractSubsystemWidget
**/
QEFEMRobotWaferAbstractSubsystemWidget::QEFEMRobotWaferAbstractSubsystemWidget(
	const std::shared_ptr<WaferRobotAbstractSubsystem>& robot,
	QWidget*parent) 
	: QAbstractSubsystemWidget<WaferRobotAbstractSubsystem>(robot,parent)
	, d_ptr(new QEFEMRobotWaferAbstractSubsystemWidgetPrivate(this)){

	Q_D(QEFEMRobotWaferAbstractSubsystemWidget);
	d->ui = new Ui::EFEMRobotWaferAbstractSubsystemWidget;
	d->ui->setupUi(this);

	init();
	
	onAttributeUpdate();

	connect(d->ui->reset_btn, &QAbstractButton::clicked, this, &QEFEMRobotWaferAbstractSubsystemWidget::onReset);
	connect(d->ui->clear_btn, &QAbstractButton::clicked, this, &QEFEMRobotWaferAbstractSubsystemWidget::onClear);
	connect(d->ui->generate_btn, &QAbstractButton::clicked, this, &QEFEMRobotWaferAbstractSubsystemWidget::onGetStatus);
	connect(d->ui->put_btn, &QAbstractButton::clicked, this, &QEFEMRobotWaferAbstractSubsystemWidget::onPutObjet);
	connect(d->ui->get_btn, &QAbstractButton::clicked, this, &QEFEMRobotWaferAbstractSubsystemWidget::onGetObjet);
	connect(d->ui->out_on_btn, &QAbstractButton::clicked, this, &QEFEMRobotWaferAbstractSubsystemWidget::onOnOutput);
	connect(d->ui->out_off_btn, &QAbstractButton::clicked, this, &QEFEMRobotWaferAbstractSubsystemWidget::onOffOutput);
	connect(d->ui->setspeed_btn, &QAbstractButton::clicked, this, &QEFEMRobotWaferAbstractSubsystemWidget::onSetSpeed);

	//config ui
	configUI(robot->getConfigure());
}

QEFEMRobotWaferAbstractSubsystemWidget::~QEFEMRobotWaferAbstractSubsystemWidget(){

}
void QEFEMRobotWaferAbstractSubsystemWidget::updateSlotsLayout() {
	Q_D(QEFEMRobotWaferAbstractSubsystemWidget);

	auto cassManager = getSubsystem()->getKernel()->getKernelModule<FortrendCassetteManager>();
	Cassette::Ptr cass = cassManager->getCassette(d->selected_station.get());
	if (!cass){
		//recreate
		QLayoutItem *child;
		//delete all child
		while ((child = d->ui->slots_layout->takeAt(0)) != 0) {
			if (child->widget()){
				child->widget()->setParent(NULL);
				delete child->widget();
			}
			delete child;
		}
	}
}

void QEFEMRobotWaferAbstractSubsystemWidget::init(){
	Q_D(QEFEMRobotWaferAbstractSubsystemWidget);

	auto cassManager = getSubsystem()->getKernel()->getKernelModule<FortrendCassetteManager>();
	cassManager->addListener(d);
	//subsystem status
	QKernelSubsystemStatusWidget* status_widget = new QKernelSubsystemStatusWidget(getSubsystem());
	d->ui->right_verticalLayout->insertWidget(0, status_widget);
	//connect
	connect(status_widget, &QKernelSubsystemStatusWidget::onStatusUpdate, this, [=](){
		onAttributeUpdate();
	});


	//create station select
	for (int i = 0; i < getSubsystem()->getWorkStations().size();i++){
		auto station = getSubsystem()->getWorkStations().at(i);
		std::string types = station->getStationPodTypes();
		if (types == ""){
			QRadioButton* radioButton = new QRadioButton;
			radioButton->setText(QString::fromStdString(station->getName()) + QString("[%1]").arg(station->getStationId(getSubsystem()->getName())));
			radioButton->setProperty("index", i); //index
			d->ui->station_layout->addWidget(radioButton);

			connect(radioButton, &QRadioButton::clicked, this, [=](){
				d->selected_station = station;
				QLayoutItem *child;
				//delete all child
				while ((child = d->ui->slots_layout->takeAt(0)) != 0) {
					if (child->widget()){
						child->widget()->setParent(NULL);
						delete child->widget();//
					}
					delete child;
				}
				//recreate
				Cassette::Ptr cass = cassManager->getCassette(station.get());
				if (cass){
					QFortrendSlotWidget* w = new QFortrendSlotWidget(cass, 15, 20); //max row count = 5
					w->canSelected(true, false);
					d->ui->slots_layout->addWidget(w);
					d->ui->slots_layout->addStretch();
				}
				radioButton->setText(QString::fromStdString(station->getName()) + QString("[%1]").arg(station->getStationId(getSubsystem()->getName())));
			});
		}
		else{
			Poco::StringTokenizer token(types, ",", Poco::StringTokenizer::TOK_IGNORE_EMPTY | Poco::StringTokenizer::TOK_TRIM);
			for (int j = 0; j < token.count(); j++){
				QRadioButton* radioButton = new QRadioButton;
				int podsize =std::stoi(token[j]);
				radioButton->setText(QString::fromStdString(station->getName() + "-" + token[j]) + QString("[%1]").arg(station->getStationId(getSubsystem()->getName(), podsize)));
				radioButton->setProperty("index", i); //index
				d->ui->station_layout->addWidget(radioButton);

				connect(radioButton, &QRadioButton::clicked, this, [=](){
					d->selected_station = station;
					QLayoutItem *child;
					//delete all child
					while ((child = d->ui->slots_layout->takeAt(0)) != 0) {
						if (child->widget()){
							child->widget()->setParent(NULL);
							delete child->widget();//
						}
						delete child;
					}
					int stationid = station->getStationId(getSubsystem()->getName(), podsize);
					station->setStationId(station->getName(),stationid);
					//recreate
					Cassette::Ptr cass = cassManager->getCassette(station.get());
					if (cass){
						QFortrendSlotWidget* w = new QFortrendSlotWidget(cass, 15, 20); //max row count = 5
						w->canSelected(true, false);
						d->ui->slots_layout->addWidget(w);
						d->ui->slots_layout->addStretch();
					}
					radioButton->setText(QString::fromStdString(station->getName() + "-" + std::to_string(podsize)) + QString("[%1]").arg(stationid));
				});
			}
		}
		
		
	}

	d->ui->station_layout->addStretch();

	//create arm select
	for (int i = 0; i < getSubsystem()->armCount(); i++){
		//select
		std::string name = getSubsystem()->getArmName(i);
		QRadioButton* selectBtn = new QRadioButton(QString::fromStdString(name));
		selectBtn->setProperty("index", i);
		//status 
		QCheckBox* stat = new QCheckBox;
		stat->setObjectName("io_object");
		stat->setEnabled(false);

		QHBoxLayout* layout = new QHBoxLayout;
		layout->addWidget(selectBtn);
		layout->addWidget(stat);
		d->ui->arm_layout->addLayout(layout);
		//save stat widget
		d->arm_stat.push_back(stat);
		d->arm_select.push_back(selectBtn);
	}
	d->ui->arm_layout->addStretch();

	//add status widget
	d->ui->horizontalLayout_2->addWidget(new QFortrendMapperStatusWidget);
	d->ui->horizontalLayout_2->addStretch();

	//add virtual input
	if(0 == getSubsystem()->inputCount()){
		d->ui->virtual_inputs_group->hide();
	}
	for (int i = 0; i < getSubsystem()->inputCount(); i++){
		QCheckBox* button = new QCheckBox(QString::fromStdString(getSubsystem()->getInputName(i)));
		button->setObjectName("io_object");
		button->setEnabled(false);
		d->ui->inputs_layout->addWidget(button, i / 4, i % 4, 1, 1);
		d->input_checkboxs.push_back(button);
	}

	//add virtual output
	if(0 == getSubsystem()->outputCount()){
		d->ui->virtual_outputs_group->hide();
	}
	for (int i = 0; i < getSubsystem()->outputCount(); i++){
		d->ui->output_combo->addItem(QString::fromStdString(getSubsystem()->getOutputName(i)));
	}
	//speed
	for (int i = 1; i <= 10; i++){
		d->ui->speed_combo->addItem(QString::number(i*10));
	}

}


void QEFEMRobotWaferAbstractSubsystemWidget::insertWidget(QWidget*widget, int position){
	Q_D(QEFEMRobotWaferAbstractSubsystemWidget);
	d->ui->extern_layout->insertWidget(position, widget);
}


std::shared_ptr<FortrendStation>  QEFEMRobotWaferAbstractSubsystemWidget::getSelectStation()const{
	int index = -1;
	for (int i = 0; i < d_ptr->ui->station_layout->count(); i++){
		QLayoutItem *child = d_ptr->ui->station_layout->itemAt(i);
		QRadioButton* radio_child = qobject_cast<QRadioButton*>(child->widget());
		if (radio_child && radio_child->isChecked()){
			index = radio_child->property("index").toInt();
			std::shared_ptr<FortrendStation>station= getSubsystem()->getWorkStations().at(index);
			return station;
		}
	}

	return 0;
}

int QEFEMRobotWaferAbstractSubsystemWidget::getSelectArmId()const{
	for (int i = 0; i < d_ptr->arm_select.size(); i++){
		if (d_ptr->arm_select.at(i)->isChecked()){
			return i;
		}
	}
}

int QEFEMRobotWaferAbstractSubsystemWidget::getSelectSlotId()const{
	for (int i = 0; i < d_ptr->ui->slots_layout->count(); i++){
		QLayoutItem *child = d_ptr->ui->slots_layout->itemAt(i);
		QFortrendSlotWidget* w = qobject_cast<QFortrendSlotWidget*>(child->widget());
		if (w){
			return w->selected().size() > 0 ? w->selected().at(0) : -1;
		}
	}
	return -1;
}

void QEFEMRobotWaferAbstractSubsystemWidget::onGetObjet(){ 
	Q_D(QEFEMRobotWaferAbstractSubsystemWidget);
	int arm = getSelectArmId()+1;
	char slot = getSelectSlotId();
	std::shared_ptr<FortrendStation> station = getSelectStation();

	if (!station){
		QMessageBox::information(this, "warn",  "Please select station");
		return;
	}
	if (slot < 0){
		QMessageBox::information(this, "warn", "Please select slot");
		return;
	}

	if (arm < 0){
		QMessageBox::information(this, "warn", "Please select arm");
		return;
	}
	

	KernelSubsystemCommand::Ptr cmd = getSubsystem()->createGetCommand(station, arm, slot);
	executeCommand(getSubsystem(),cmd);
}

void QEFEMRobotWaferAbstractSubsystemWidget::onPutObjet(){
	Q_D(QEFEMRobotWaferAbstractSubsystemWidget);
	int arm = getSelectArmId()+1;
	char slot = getSelectSlotId();
	std::shared_ptr<FortrendStation> station = getSelectStation();

	if (!station){
		QMessageBox::information(this, "warn", "Please select station");
		return;
	}
	if (slot < 0){
		QMessageBox::information(this, "warn", "Please select slot");
		return;
	}

	if (arm < 0){
		QMessageBox::information(this, "warn", "Please select arm");
		return;
	}


	KernelSubsystemCommand::Ptr cmd = getSubsystem()->createPutCommand(station, arm,slot);
	executeCommand(getSubsystem(), cmd);
}


void QEFEMRobotWaferAbstractSubsystemWidget::onReset(){
	Q_D(QEFEMRobotWaferAbstractSubsystemWidget);
	KernelSubsystemCommand::Ptr cmd = getSubsystem()->createResetCommand();
	executeCommand(getSubsystem(), cmd);
}

void QEFEMRobotWaferAbstractSubsystemWidget::onClear(){
	Q_D(QEFEMRobotWaferAbstractSubsystemWidget);
	KernelSubsystemCommand::Ptr cmd = getSubsystem()->createResetCommand();
	executeCommand(getSubsystem(), cmd);
}


void QEFEMRobotWaferAbstractSubsystemWidget::onGetStatus(){
	Q_D(QEFEMRobotWaferAbstractSubsystemWidget);
	KernelSubsystemCommand::Ptr cmd = getSubsystem()->createUpdateCommand();
	executeCommand(getSubsystem(), cmd);
}



void QEFEMRobotWaferAbstractSubsystemWidget::onOnOutput(){
	Q_D(QEFEMRobotWaferAbstractSubsystemWidget);
	KernelSubsystemCommand::Ptr cmd = getSubsystem()->createOutputCommand(d->ui->output_combo->currentIndex(), true);
	executeCommand(getSubsystem(), cmd);
}


void QEFEMRobotWaferAbstractSubsystemWidget::onOffOutput(){
	Q_D(QEFEMRobotWaferAbstractSubsystemWidget);
	KernelSubsystemCommand::Ptr cmd = getSubsystem()->createOutputCommand(d->ui->output_combo->currentIndex(), false);
	executeCommand(getSubsystem(), cmd);
}

void QEFEMRobotWaferAbstractSubsystemWidget::onSetSpeed(){
	Q_D(QEFEMRobotWaferAbstractSubsystemWidget);
	KernelSubsystemCommand::Ptr cmd = getSubsystem()->createSetSpeedCommand(d->ui->speed_combo->currentText().toInt());  //speed_combo
	executeCommand(getSubsystem(), cmd);
}

void QEFEMRobotWaferAbstractSubsystemWidget::onSetSize(){
	Q_D(QEFEMRobotWaferAbstractSubsystemWidget);

	
}
void QEFEMRobotWaferAbstractSubsystemWidget::onSetToolSize(){
	Q_D(QEFEMRobotWaferAbstractSubsystemWidget);
	
}

void QEFEMRobotWaferAbstractSubsystemWidget::onAttributeUpdate(){
	Q_D(QEFEMRobotWaferAbstractSubsystemWidget);
	//update input
	for (int i = 0; i < d->input_checkboxs.size(); i++){
		d->input_checkboxs.at(i)->setChecked(getSubsystem()->getInput(i));
	}

	//update object stat
	for (int i = 0; i < d->arm_stat.size(); i++){
		d->arm_stat.at(i)->setChecked(getSubsystem()->hasObject(i));
	}
}

KERNEL_NS_END
