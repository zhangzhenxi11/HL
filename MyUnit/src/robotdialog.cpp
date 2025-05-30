#include "robotdialog.h"
#include "MyUnit/ui_robotdialog.h"

RobotDialog::RobotDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RobotDialog)
{
    ui->setupUi(this);
	
	connect(ui->setspeed_btn, &QAbstractButton::clicked, this, &RobotDialog::onSetSpeed);
	connect(ui->set_speed_btn_z, &QAbstractButton::clicked, this, &RobotDialog::onSetZSpeed);
	connect(ui->put_btn, &QAbstractButton::clicked, this, &RobotDialog::onPut);
	connect(ui->get_btn, &QAbstractButton::clicked, this, &RobotDialog::onGet);

	/*ui->speed_combo->addItem(QString::number(1));
	ui->speed_combo->addItem(QString::number(5));*/
	//speed
	/*for (int i = 1; i <= 10; i++){
		ui->speed_combo->addItem(QString::number(i * 10));
	}*/

	setStyleSheet("background-color: #FFFFFF;");
	QString style = "QAbstractButton{ background-color: #F3F3F4; } QAbstractButton:hover{ background-color: #C9F1DF; } ";
	ui->setspeed_btn->setStyleSheet(style);
	ui->put_btn->setStyleSheet(style);
	ui->get_btn->setStyleSheet(style);
	//ui->widget->setStyleSheet("background-color: #FFFFFF;");
}
void RobotDialog::AddStation(QRadioButton* selectBtn){
	ui->station_layout->addWidget(selectBtn);
}
void RobotDialog::AddArm(QRadioButton* armBtn){
	ui->arm_layout->addWidget(armBtn);
}

RobotDialog::~RobotDialog()
{
    delete ui;
}

int  RobotDialog::getSelectStation()const{
	int index = -1;
	for (int i = 0; i < ui->station_layout->count(); i++){
		QLayoutItem *child = ui->station_layout->itemAt(i);
		QRadioButton* radio_child = qobject_cast<QRadioButton*>(child->widget());
		if (radio_child && radio_child->isChecked()){
			index = radio_child->property("index").toInt();
			return index;
		}
	}
	return -1;
}

int  RobotDialog::getSelectArm()const{
	int index = -1;
	for (int i = 0; i < ui->arm_layout->count(); i++){
		QLayoutItem *child = ui->arm_layout->itemAt(i);
		QRadioButton* radio_child = qobject_cast<QRadioButton*>(child->widget());
		if (radio_child && radio_child->isChecked()){
			index = radio_child->property("index").toInt();
			return index;
		}
	}
	return -1;
}

void RobotDialog::onGet(){
	int station = getSelectStation();
	int arm = getSelectArm();
	emit signalget(station,arm);
}

void RobotDialog::onPut(){
	int station = getSelectStation();
	int arm = getSelectArm();
	emit signalput(station, arm);
}

void RobotDialog::onSetSpeed(){
	emit signalspeed(ui->speed_value_spx->value());
}

void RobotDialog::onSetZSpeed(){
	emit signalzspeed(ui->speed_value_spx_z->value());
}

