
/**
* @file    fortrend_SunwayRobot_subsystem.h
* @brief   Fortrend SunwayRobot widget
* @author  xielonghua
*/

// Library: Fortrend
// Package: SubSystem/SunwayRobot


#include  "Kernel/kernel.h"
#include  "Kernel/FortrendUI/cassette_widget.h"
#include  "Kernel/FortrendUI/slot_widget.h"
#include  "Kernel/CoreUI/kernel_subsystem_status_widget.h"  
#include  "Kernel/Fortrend/fortrend_station.h"
#include  "Kernel/Fortrend/fortrend_cassette_manager.h"
#include  "Kernel/kernel_subsystem_reset_command.h"
#include  "Kernel/kernel_subsystem_update_command.h"
#include  "Kernel/Fortrend/abstract_output_command.h"

#include  "device/ui_fortrend_main_sunwayrobot_subsystem_widget.h" 


#include  "SunwayRobot/fortrend_main_sunwayrobot_subsystem_widget.h"


#include <QRadioButton>
#include <QSpacerItem>
#include <QMessageBox>
#include <QCheckBox>
#include <QDoubleSpinBox>
#if _MSC_VER >1600
#pragma execution_character_set("utf-8")
#endif


namespace FC{
	/**
	* QMainSunwayRobotSubsystemWidgetPrivate
	**/
	class QMainSunwayRobotSubsystemWidgetPrivate{
		Q_DECLARE_PUBLIC(QMainSunwayRobotSubsystemWidget)
	public:
		QMainSunwayRobotSubsystemWidgetPrivate(QMainSunwayRobotSubsystemWidget* p);
	public:
		Ui::SunwayMainRobotSubsystemWidget* ui;
		QMainSunwayRobotSubsystemWidget* q_ptr;

		std::vector<QCheckBox*> input_checkboxs;
		std::vector<QCheckBox*> arm_stat;
		std::vector<QRadioButton*> arm_select;
	};

	QMainSunwayRobotSubsystemWidgetPrivate::QMainSunwayRobotSubsystemWidgetPrivate(
		QMainSunwayRobotSubsystemWidget* p)
		:q_ptr(p){

	}



	/**
	* QMainSunwayRobotSubsystemWidget
	**/
	QMainSunwayRobotSubsystemWidget::QMainSunwayRobotSubsystemWidget(
		const std::shared_ptr<FortrendSunwayRobotSubsystem>& robot,
		QWidget*parent)
		: QAbstractSubsystemWidget<FortrendSunwayRobotSubsystem>(robot, parent)
		, d_ptr(new QMainSunwayRobotSubsystemWidgetPrivate(this)){

		Q_D(QMainSunwayRobotSubsystemWidget);
		d->ui = new Ui::SunwayMainRobotSubsystemWidget;
		d->ui->setupUi(this);
		//init samethong
		init();

		onAttributeUpdate();
	}

	QMainSunwayRobotSubsystemWidget::~QMainSunwayRobotSubsystemWidget(){

	}

	void QMainSunwayRobotSubsystemWidget::init(){
		Q_D(QMainSunwayRobotSubsystemWidget);

		auto cassManager = getSubsystem()->getKernel()->getKernelModule<FortrendCassetteManager>();

		QKernelSubsystemStatusWidget* status_widget = new QKernelSubsystemStatusWidget(getSubsystem());
		QLabel *label_name = new QLabel();
		QFont font = QFont();
		font.setFamily("Arial");  //# 设置字体类型
		font.setPointSize(14);   // # 设置字体大小
		font.setBold(true);      //# 设置字体加粗
		//font.setItalic(true);    // # 设置字体斜体
		label_name->setFont(font);    // # 应用字体样式到QLabel

		label_name->setAlignment(Qt::AlignCenter);
		label_name->setText(QString::fromStdString(getSubsystem()->getName()));
		d->ui->status_layout->insertWidget(0, label_name);
		d->ui->status_layout->insertWidget(1, status_widget);

		//connect
		connect(status_widget, &QKernelSubsystemStatusWidget::onStatusUpdate, this, [=](){
			onAttributeUpdate();
		});


		//create station select
		for (int i = 0; i < getSubsystem()->getWorkStations().size(); i++){
			auto station = getSubsystem()->getWorkStations().at(i);
			QRadioButton* radioButton = new QRadioButton;
			radioButton->setText(QString::fromStdString(station->getName()) + QString("[%1]").arg(station->getStationId(getSubsystem()->getName())));
			radioButton->setProperty("index", i); //index
			//d->ui->station_layout->addWidget(radioButton);
			if (i == 0)
				radioButton->setChecked(true);
		}

		//d->ui->station_layout->addStretch();

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
			//d->ui->arm_layout->addLayout(layout);
			//save stat widget
			d->arm_stat.push_back(stat);
			d->arm_select.push_back(selectBtn);
			if (i == 0)
				selectBtn->setChecked(true);
		}
		//d->ui->arm_layout->addStretch();
		//d->ui->horizontalLayout_2->addStretch();

		//add virtual input
		/*for (int i = 0; i < getSubsystem()->inputCount(); i++){
			QCheckBox* button = new QCheckBox(QString::fromStdString(getSubsystem()->getInputName(i)));
			button->setObjectName("io_object");
			button->setEnabled(false);
			d->ui->inputs_layout->addWidget(button, i / 4, i % 4, 1, 1);
			d->input_checkboxs.push_back(button);
			}*/

		//d->ui->speed_combo->addItem(QString::number(1));
		//d->ui->speed_combo->addItem(QString::number(5));
		//speed
		/*for (int i = 1; i <= 10; i++){
			d->ui->speed_combo->addItem(QString::number(i * 10));
		}*/

		//check modules
		//auto cass = cassManager->getCassette(getSubsystem().get());
		//if (cass){
		//	QWidget* cassette_Widget = new QFortrendCassetteWidget(cass, cassManager, true, true, 2, this); //max row count = 25
		//	d->ui->center_layout->addWidget(cassette_Widget);

		//}

	}



	std::shared_ptr<FortrendStation>  QMainSunwayRobotSubsystemWidget::getSelectStation()const{

		int index = -1;
		//for (int i = 0; i < d_ptr->ui->station_layout->count(); i++){
		//	QLayoutItem *child = d_ptr->ui->station_layout->itemAt(i);
		//	QRadioButton* radio_child = qobject_cast<QRadioButton*>(child->widget());
		//	if (radio_child && radio_child->isChecked()){
		//		index = radio_child->property("index").toInt();
		//		//
		//		return getSubsystem()->getWorkStations().at(index);
		//	}
		//}

		return 0;
	}

	int QMainSunwayRobotSubsystemWidget::getSelectArmId()const{
		for (int i = 0; i < d_ptr->arm_select.size(); i++){
			if (d_ptr->arm_select.at(i)->isChecked()){
				return i;
			}
		}
	}

	void QMainSunwayRobotSubsystemWidget::onReset(){
		Q_D(QMainSunwayRobotSubsystemWidget);
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createResetCommand();
		executeCommand(getSubsystem(), cmd);
	}

	void QMainSunwayRobotSubsystemWidget::onGetStatus(){
		Q_D(QMainSunwayRobotSubsystemWidget);
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createUpdateCommand();
		executeCommand(getSubsystem(), cmd);
	}

	void QMainSunwayRobotSubsystemWidget::onOutput(){
		Q_D(QMainSunwayRobotSubsystemWidget);
		int index = -1; //outout index
		bool outStat = false;  //output status
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createOutputCommand(index, outStat);
		executeCommand(getSubsystem(), cmd);
	}

	//all special commands

	void QMainSunwayRobotSubsystemWidget::onGetWaferCommand(){
		Q_D(QMainSunwayRobotSubsystemWidget);
		int arm = getSelectArmId();
		std::shared_ptr<FortrendStation> station = getSelectStation();

		if (!station){
			QMessageBox::information(this, "warn", "Please select station");
			return;
		}

		if (arm < 0){
			QMessageBox::information(this, "warn", "Please select arm");
			return;
		}
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createGetCommand(station, arm, 1);
		executeCommand(getSubsystem(), cmd);
	}

	void QMainSunwayRobotSubsystemWidget::onPutWaferCommand(){
		Q_D(QMainSunwayRobotSubsystemWidget);
		int arm = getSelectArmId();
		std::shared_ptr<FortrendStation> station = getSelectStation();

		if (!station){
			QMessageBox::information(this, "warn", "Please select station");
			return;
		}

		if (arm < 0){
			QMessageBox::information(this, "warn", "Please select arm");
			return;
		}
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createPutCommand(station, arm, 1);
		executeCommand(getSubsystem(), cmd);
	}

	void QMainSunwayRobotSubsystemWidget::onReadyGetWaferCommand(){
		Q_D(QMainSunwayRobotSubsystemWidget);
		int arm = getSelectArmId();
		std::shared_ptr<FortrendStation> station = getSelectStation();

		if (!station){
			QMessageBox::information(this, "warn", "Please select station");
			return;
		}

		if (arm < 0){
			QMessageBox::information(this, "warn", "Please select arm");
			return;
		}
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createReadyGetCommand(station, arm, 1);
		executeCommand(getSubsystem(), cmd);
	}

	void QMainSunwayRobotSubsystemWidget::onReadyPutWaferCommand(){
		Q_D(QMainSunwayRobotSubsystemWidget);
		int arm = getSelectArmId();
		std::shared_ptr<FortrendStation> station = getSelectStation();

		if (!station){
			QMessageBox::information(this, "warn", "Please select station");
			return;
		}

		if (arm < 0){
			QMessageBox::information(this, "warn", "Please select arm");
			return;
		}
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createReadyPutCommand(station, arm, 1);
		executeCommand(getSubsystem(), cmd);
	}

	void QMainSunwayRobotSubsystemWidget::onSetSpeedCommand(){
		Q_D(QMainSunwayRobotSubsystemWidget);
		//KernelSubsystemCommand::Ptr cmd = getSubsystem()->createSetSpeedCommand(d->ui->speed_combo->currentText().toInt());
		//executeCommand(getSubsystem(), cmd);
	}


	void QMainSunwayRobotSubsystemWidget::onAttributeUpdate(){
		Q_D(QMainSunwayRobotSubsystemWidget);
		//update input
		for (int i = 0; i < d->input_checkboxs.size(); i++){
			d->input_checkboxs.at(i)->setChecked(getSubsystem()->getInput(i));
		}

		//update object state
		for (int i = 0; i < d->arm_stat.size(); i++){
			d->arm_stat.at(i)->setChecked(getSubsystem()->hasObject(i));
		}

		//update awc record data
		auto data = getSubsystem()->getAWCRecordData(1);
		/*d->ui->pm1_awc_r_dsb->setValue(data.R);
		d->ui->pm1_awc_t_dsb->setValue(data.T);
		d->ui->pm1_awc_x_dsb->setValue(data.X);
		d->ui->pm1_awc_y_dsb->setValue(data.Y);

		data = getSubsystem()->getAWCRecordData(1);*/
		d->ui->pm2_awc_r_dsb->setValue(data.R);
		d->ui->pm2_awc_t_dsb->setValue(data.T);
		d->ui->pm2_awc_x_dsb->setValue(data.X);
		d->ui->pm2_awc_y_dsb->setValue(data.Y);

		/*data = getSubsystem()->getAWCRecordData(2);
		d->ui->pm3_awc_r_dsb->setValue(data.R);
		d->ui->pm3_awc_t_dsb->setValue(data.T);
		d->ui->pm3_awc_x_dsb->setValue(data.X);
		d->ui->pm3_awc_y_dsb->setValue(data.Y);*/

		
	}
}