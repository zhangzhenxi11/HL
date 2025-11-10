#include "ERobotDialog.h"
#include "MyUnit/ui_ERobotDialog.h"
#include "Kernel/kernel_log.h"
ERobotDialog::ERobotDialog(QWidget *parent)
	: QDialog(parent),
	ui(new Ui::ERobotDialog)
{
	ui->setupUi(this);
	connect(ui->setspeed_btn, &QAbstractButton::clicked, this, &ERobotDialog::onSetRobotSpeed);
	connect(ui->put_btn, &QAbstractButton::clicked, this, &ERobotDialog::onPutWafer);
	connect(ui->get_btn, &QAbstractButton::clicked, this, &ERobotDialog::onGetWafer);
}

ERobotDialog::~ERobotDialog()
{

	delete ui;
}

void ERobotDialog::addStation(QRadioButton* selectBtn)
{
	ui->station_layout->addWidget(selectBtn);
}

void ERobotDialog::addArm(QRadioButton* armBtn)
{
	ui->arm_layout->addWidget(armBtn);
}

void ERobotDialog::addSlot(FC::QFortrendSlotWidget* selectSlot)
{
	ui->slots_layout->addWidget(selectSlot);
	ui->slots_layout->addStretch();
}

int ERobotDialog::getSelectEfemStation() const
{
	int index = -1;
	for (int i = 0; i < ui->station_layout->count(); i++) {
		QLayoutItem* child = ui->station_layout->itemAt(i);
		QRadioButton* radio_child = qobject_cast<QRadioButton*>(child->widget());
		if (radio_child && radio_child->isChecked()) {
			index = radio_child->property("index").toInt();
			return index;
		}
	}
	return -1;
}

int ERobotDialog::getSelectEfemArm() const
{
	int index = -1;
	for (int i = 0; i < ui->arm_layout->count(); i++) {
		QLayoutItem* child = ui->arm_layout->itemAt(i);
		QRadioButton* radio_child = qobject_cast<QRadioButton*>(child->widget());
		if (radio_child && radio_child->isChecked()) {
			index = radio_child->property("index").toInt()+1; //+1
			return index;
		}
	}
	return -1;
}

int ERobotDialog::getSelectEfemSlotId() const
{
	for (int i = 0; i < ui->slots_layout->count(); i++)
	{
		QLayoutItem* child = ui->slots_layout->itemAt(i);
		FC::QFortrendSlotWidget* w = qobject_cast<FC::QFortrendSlotWidget*>(child->widget());
		if (w) {
			return w->selected().size() > 0 ? w->selected().at(0) : -1;
		}
	}
	return -1;
}

void ERobotDialog::onPutWafer()
{
	int station = getSelectEfemStation();
	int arm = getSelectEfemArm();
	int slot = getSelectEfemSlotId();
	logInform("test", "station,arm,slot:%d,%d,%d", station, arm, slot);
	emit signalPut(station, arm, slot);
}

void ERobotDialog::onGetWafer()
{
	int station = getSelectEfemStation();
	int arm = getSelectEfemArm();
	int slot = getSelectEfemSlotId();
	logInform("test", "station,arm,slot:%d,%d,%d", station, arm, slot);
	emit signalGet(station, arm, slot);
}
void ERobotDialog::onSetRobotSpeed()
{
	emit signalSpeed(ui->speed_value_spx->value());
}
