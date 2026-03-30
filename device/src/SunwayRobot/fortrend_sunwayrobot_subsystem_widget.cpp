
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
#include "Kernel/kernel_log.h"
#include  "device/ui_fortrend_sunwayrobot_subsystem_widget.h" 
#include  "SunwayRobot/fortrend_sunwayrobot_subsystem_widget.h"
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
	* QSunwayRobotSubsystemWidgetPrivate
	**/
	class QSunwayRobotSubsystemWidgetPrivate{
		Q_DECLARE_PUBLIC(QSunwayRobotSubsystemWidget)
	public:
		QSunwayRobotSubsystemWidgetPrivate(QSunwayRobotSubsystemWidget* p);
	public:
		Ui::SunwayRobotSubsystemWidget* ui;
		QSunwayRobotSubsystemWidget* q_ptr;

		std::vector<QCheckBox*> input_checkboxs;
		std::vector<QCheckBox*> arm_stat;
		std::vector<QRadioButton*> arm_select;
		std::shared_ptr<FortrendStation> selected_station;
		//std::shared_ptr<IKernel> kernel = 0;
		std::shared_ptr<FortrendLoadLockSubsystem> lk1 = nullptr;
		std::shared_ptr<FortrendLoadLockSubsystem> lk2 = nullptr;

		//ewtr = kernel->getKernelModule<EFEMWaferRobotSubsystem>("EWTR");

	};

	QSunwayRobotSubsystemWidgetPrivate::QSunwayRobotSubsystemWidgetPrivate(
		QSunwayRobotSubsystemWidget* p)
		:q_ptr(p){

	}



	/**
	* QSunwayRobotSubsystemWidget
	**/
	QSunwayRobotSubsystemWidget::QSunwayRobotSubsystemWidget(
		const std::shared_ptr<FortrendSunwayRobotSubsystem>& robot,
		QWidget*parent)
		: QAbstractSubsystemWidget<FortrendSunwayRobotSubsystem>(robot, parent)
		, d_ptr(new QSunwayRobotSubsystemWidgetPrivate(this))
	    {
		Q_D(QSunwayRobotSubsystemWidget);
		d->ui = new Ui::SunwayRobotSubsystemWidget;
		d->ui->setupUi(this);
		//d->kernel = kernel;
		/*	d->lk1 = kernel->getKernelModule<FortrendLoadLockSubsystem>("LLA");
		d->lk2 = kernel->getKernelModule<FortrendLoadLockSubsystem>("LLB");*/

		//d->robot_sub = std::dynamic_pointer_cast<FortrendSunwayRobotSubsystem>(robot);
		//init samethong
		init();

		onAttributeUpdate();
	

		connect(d->ui->reset_btn, &QAbstractButton::clicked, this, &QSunwayRobotSubsystemWidget::onReset);
		connect(d->ui->generate_btn, &QAbstractButton::clicked, this, &QSunwayRobotSubsystemWidget::onGetStatus);
		connect(d->ui->put_btn, &QAbstractButton::clicked, this, &QSunwayRobotSubsystemWidget::onPutWaferCommand);
		connect(d->ui->get_btn, &QAbstractButton::clicked, this, &QSunwayRobotSubsystemWidget::onGetWaferCommand);
		connect(d->ui->check_load_btn, &QAbstractButton::clicked, this, &QSunwayRobotSubsystemWidget::onCheckLoadCommand);
		connect(d->ui->clear_error_btn, &QAbstractButton::clicked, this, &QSunwayRobotSubsystemWidget::onClearErrorCommand);
		//connect(d->ui->ready_get_btn, &QAbstractButton::clicked, this, &QSunwayRobotSubsystemWidget::onReadyGetWaferCommand);
		//connect(d->ui->ready_put_btn, &QAbstractButton::clicked, this, &QSunwayRobotSubsystemWidget::onReadyPutWaferCommand);
		connect(d->ui->set_speed_btn, &QAbstractButton::clicked, this, &QSunwayRobotSubsystemWidget::onSetSpeedCommand);;
		connect(d->ui->set_speed_btn_z, &QAbstractButton::clicked, this, &QSunwayRobotSubsystemWidget::onSetAxisZSpeedCommand);
		connect(d->ui->home_btn, &QAbstractButton::clicked, this, &QSunwayRobotSubsystemWidget::onHomeCommand);

		//for (int i = 0; i < d->arm_stat.size(); i++)
		//{
		//	connect(d->arm_stat.at(i), &QCheckBox::stateChanged, this, [this, i](int state) {
		//		onSetLoadCommand(i, state);
		//	});
		//}

		//设置手指有无片按钮
		connect(d->ui->set_has_wafer_btn, &QAbstractButton::clicked, this, [this]() {
			int arm = d_ptr->ui->arm_a_radio->isChecked() ? 0 : 1;
			onSetLoadCommand(arm, 1);  //state=1 有片
		});
		connect(d->ui->set_no_wafer_btn, &QAbstractButton::clicked, this, [this]() {
			int arm = d_ptr->ui->arm_a_radio->isChecked() ? 0 : 1;
			onSetLoadCommand(arm, 0);  //state=0 无片
		});


	}

	QSunwayRobotSubsystemWidget::~QSunwayRobotSubsystemWidget(){

	}

	void QSunwayRobotSubsystemWidget::init(){
		Q_D(QSunwayRobotSubsystemWidget);

		auto cassManager = getSubsystem()->getKernel()->getKernelModule<FortrendCassetteManager>();

		//subsystem status
		QKernelSubsystemStatusWidget* status_widget = new QKernelSubsystemStatusWidget(getSubsystem());
		d->ui->right_verticalLayout->insertWidget(0, status_widget);
		////connect
		connect(status_widget, &QKernelSubsystemStatusWidget::onStatusUpdate, this, [=](){
			onAttributeUpdate();
		});


		//create station select
		for (int i = 0; i < getSubsystem()->getWorkStations().size(); i++)
		{
			auto station = getSubsystem()->getWorkStations().at(i);
			QRadioButton* radioButton = new QRadioButton;
			radioButton->setText(QString::fromStdString(station->getName()) + QString("[%1]").arg(station->getStationId(getSubsystem()->getName())));
			radioButton->setProperty("index", i); //index
			d->ui->station_layout->addWidget(radioButton);
			if (i == 0)
			{
				radioButton->setChecked(true);
			}
			connect(radioButton, &QRadioButton::clicked, this, [=]() {
				d->selected_station = station;
				QLayoutItem* child;
				//delete all child
				while ((child = d->ui->slots_layout->takeAt(0)) != 0) {
					if (child->widget()) {
						child->widget()->setParent(NULL);
						delete child->widget();//
					}
					delete child;
				}
				//recreate
				Cassette::Ptr cass = cassManager->getCassette(station.get());
				if (cass) {
					QFortrendSlotWidget* w = new QFortrendSlotWidget(cass, 15, 20); //max row count = 5
					w->canSelected(true, false);
					d->ui->slots_layout->addWidget(w);
					d->ui->slots_layout->addStretch();
				}
				radioButton->setText(QString::fromStdString(station->getName()) + QString("[%1]").arg(station->getStationId(getSubsystem()->getName())));
			});
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
			//QString armObj = "arm_" + QString::number(i);
			stat->setObjectName("io_object");
			stat->setEnabled(false);

			QHBoxLayout* layout = new QHBoxLayout;
			layout->addWidget(selectBtn);
			layout->addWidget(stat);
			d->ui->arm_layout->addLayout(layout);
			//save stat widget
			d->arm_stat.push_back(stat);
			d->arm_select.push_back(selectBtn);
			if (i == 0)
				selectBtn->setChecked(true);
		}
		d->ui->arm_layout->addStretch();
		d->ui->horizontalLayout_2->addStretch();

		//add virtual input
		/*for (int i = 0; i < getSubsystem()->inputCount(); i++){
			QCheckBox* button = new QCheckBox(QString::fromStdString(getSubsystem()->getInputName(i)));
			button->setObjectName("io_object");
			button->setEnabled(false);
			d->ui->inputs_layout->addWidget(button, i / 4, i % 4, 1, 1);
			d->input_checkboxs.push_back(button);
			}*/


		//check modules
		auto cass = cassManager->getCassette(getSubsystem().get());
		if (cass){
			QWidget* cassette_Widget = new QFortrendCassetteWidget(cass, cassManager, true, true, 2, this); //max row count = 25
			d->ui->center_layout->addWidget(cassette_Widget);
		}

	}



	std::shared_ptr<FortrendStation>  QSunwayRobotSubsystemWidget::getSelectStation()const{

		int index = -1;
		for (int i = 0; i < d_ptr->ui->station_layout->count(); i++){
			QLayoutItem *child = d_ptr->ui->station_layout->itemAt(i);
			QRadioButton* radio_child = qobject_cast<QRadioButton*>(child->widget());
			if (radio_child && radio_child->isChecked()){
				index = radio_child->property("index").toInt();
				//
				return getSubsystem()->getWorkStations().at(index);
			}
		}

		return 0;
	}

	int QSunwayRobotSubsystemWidget::getSelectArmId()const{
		for (int i = 0; i < d_ptr->arm_select.size(); i++)
		{
			if (d_ptr->arm_select.at(i)->isChecked())
			{
				return i;
			}
		}
	}

	int QSunwayRobotSubsystemWidget::getSelectSlotId() const
	{
		for (int i = 0; i < d_ptr->ui->slots_layout->count(); i++)
		{
			QLayoutItem* child = d_ptr->ui->slots_layout->itemAt(i);
			QFortrendSlotWidget* w = qobject_cast<QFortrendSlotWidget*>(child->widget());
			if (w) {
				return w->selected().size() > 0 ? w->selected().at(0) : -1;
			}
		}
		return -1;
	}

	void QSunwayRobotSubsystemWidget::onReset(){
		Q_D(QSunwayRobotSubsystemWidget);
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createResetCommand();
		executeCommand(getSubsystem(), cmd);
	}

	void QSunwayRobotSubsystemWidget::onGetStatus(){
		Q_D(QSunwayRobotSubsystemWidget);
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createUpdateCommand();
		executeCommand(getSubsystem(), cmd);
	}

	void QSunwayRobotSubsystemWidget::onOutput(){
		Q_D(QSunwayRobotSubsystemWidget);
		int index = -1; //outout index
		bool outStat = false;  //output status
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createOutputCommand(index, outStat);
		executeCommand(getSubsystem(), cmd);
	}

	//all special commands

	void QSunwayRobotSubsystemWidget::onGetWaferCommand(){
		Q_D(QSunwayRobotSubsystemWidget);
		int arm = getSelectArmId();
		/*arm = (arm == 0) ? 1 : 0;*/
		logInform(getSubsystem()->getName().c_str(), "arm:%d", arm);

		std::shared_ptr<FortrendStation> station = getSelectStation();
		int slot = getSelectSlotId();

		if (!station){
			QMessageBox::information(this, "警告", "请选择工位");
			return;
		}
		if (slot < 0) {
			QMessageBox::information(this, "警告", "请选择槽号");
			return;
		}
		if (arm < 0){
			QMessageBox::information(this, "警告", "请选择手臂");
			return;
		}
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createGetCommand(station, arm, slot);
		executeCommand(getSubsystem(), cmd);
	}

	void QSunwayRobotSubsystemWidget::onPutWaferCommand(){
		Q_D(QSunwayRobotSubsystemWidget);
		int arm = getSelectArmId();
		//arm = (arm == 0) ? 1 : 0;
		logInform(getSubsystem()->getName().c_str(), "arm:%d", arm);

		std::shared_ptr<FortrendStation> station = getSelectStation();
		int slot = getSelectSlotId();

		if (!station){
			QMessageBox::information(this, "警告", "请选择工位");
			return;
		}
		if (slot < 0) {
			QMessageBox::information(this, "警告", "请选择槽号");
			return;
		}
		if (arm < 0){
			QMessageBox::information(this, "警告", "请选择手臂");
			return;
		}
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createPutCommand(station, arm, slot);
		executeCommand(getSubsystem(), cmd);
	}

	void QSunwayRobotSubsystemWidget::onReadyGetWaferCommand(){
		Q_D(QSunwayRobotSubsystemWidget);
		int arm = getSelectArmId();
		arm = (arm == 0) ? 1 : 0;
		logInform(getSubsystem()->getName().c_str(), "arm:%s", arm == 0 ? "B" : "A");
		std::shared_ptr<FortrendStation> station = getSelectStation();
		int slot = getSelectSlotId();
		if (!station){
			QMessageBox::information(this, "警告", "请选择工位");
			return;
		}
		if (slot < 0) {
			QMessageBox::information(this, "警告", "请选择槽号");
			return;
		}
		if (arm < 0){
			QMessageBox::information(this, "警告", "请选择手臂");
			return;
		}
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createReadyGetCommand(station, arm, slot);
		executeCommand(getSubsystem(), cmd);
	}

	void QSunwayRobotSubsystemWidget::onReadyPutWaferCommand(){
		Q_D(QSunwayRobotSubsystemWidget);
		int arm = getSelectArmId();
		arm = (arm == 0) ? 1 : 0;
		logInform(getSubsystem()->getName().c_str(), "arm:%s", arm == 0 ? "B" : "A");
		std::shared_ptr<FortrendStation> station = getSelectStation();
		int slot = getSelectSlotId();
		if (!station){
			QMessageBox::information(this, "警告", "请选择工位");
			return;
		}
		if (slot < 0) {
			QMessageBox::information(this, "警告", "请选择槽号");
			return;
		}
		if (arm < 0){
			QMessageBox::information(this, "警告", "请选择手臂");
			return;
		}
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createReadyPutCommand(station, arm, slot);
		executeCommand(getSubsystem(), cmd);
	}

	void QSunwayRobotSubsystemWidget::onSetAxisZSpeedCommand(){
		Q_D(QSunwayRobotSubsystemWidget);
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createSetAxisZSpeedCommand(d->ui->speed_value_spx_z->value());
		executeCommand(getSubsystem(), cmd);
	}

	void QSunwayRobotSubsystemWidget::onSetSpeedCommand(){
		Q_D(QSunwayRobotSubsystemWidget);
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createSetSpeedCommand(d->ui->speed_value_spx->value());
		executeCommand(getSubsystem(), cmd);
	}

	void QSunwayRobotSubsystemWidget::onCheckLoadCommand(){
		Q_D(QSunwayRobotSubsystemWidget);
		int arm = getSelectArmId();
		arm = (arm == 0) ? 1 : 0;
#if  0	
		std::shared_ptr<FortrendStation> station = getSelectStation();

		/*if (!station){
			QMessageBox::information(this, "warn", "Please select station");
			return;
		}*/

		if (arm < 0){
			QMessageBox::information(this, "警告", "请选择手臂");
			return;
		}
		int station_id = 1;

		if (station->getName() == "LLA")
		{
			station_id = 1;
		}
		else if (station->getName() == "LLB")
		{
			station_id = 6;
		}
		else if (station->getName() == "PM1")
		{
			station_id = 2;
		}
		else if (station->getName() == "PM2")
		{
			station_id = 3;
		}
		else if (station->getName() == "PM3")
		{
			station_id = 4;
		}
		else if (station->getName() == "PM5")
		{
			station_id = 6;
		}
		else{
			QMessageBox::information(this, "错误", "查询手指有无晶圆输入工位错误");
			return;
		}
#endif
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createRQLoadCommand(arm);
		executeCommand(getSubsystem(), cmd);

	}

	void QSunwayRobotSubsystemWidget::onClearErrorCommand(){
		Q_D(QSunwayRobotSubsystemWidget);
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createClearErrorCommand();
		executeCommand(getSubsystem(), cmd);
	}

	void QSunwayRobotSubsystemWidget::onHomeCommand()
	{
		Q_D(QSunwayRobotSubsystemWidget);
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createHomeCommand();
		executeCommand(getSubsystem(), cmd);
	}

	void QSunwayRobotSubsystemWidget::onSetLoadCommand(int arm, int state)
	{
		Q_D(QSunwayRobotSubsystemWidget);
		arm = (arm == 0) ? 1 : 0;
		logInform(getSubsystem()->getName().c_str(), "arm:%s,state:%d", arm == 0 ? "B" : "A", state);

		if (getSubsystem()->getState() == IKernelSubSystem::State::SUB_NORMAL)
		{
			if (state == 0 || state == 1)
			{
				KernelSubsystemCommand::Ptr cmd = getSubsystem()->createSetLoadCommand(arm, state);

				executeCommand(getSubsystem(), cmd);
			}
		}
	}

	void QSunwayRobotSubsystemWidget::onAttributeUpdate()throw(KernelException){
		Q_D(QSunwayRobotSubsystemWidget);
		try{

			//update input
			for (int i = 0; i < d->input_checkboxs.size(); i++){
				d->input_checkboxs.at(i)->setChecked(getSubsystem()->getInput(i));
			}

			//update object state
			for (int i = 0; i < d->arm_stat.size(); i++)
			{
				//d->arm_stat.at(i)->blockSignals(true);
				d->arm_stat.at(i)->setChecked(getSubsystem()->hasObject(i));
				//d->arm_stat.at(i)->blockSignals(false);
			}

			//update awc record data
			auto data = getSubsystem()->getAWCRecordData();
			d->ui->pm2_awc_r_dsb->setValue(data.R);
			d->ui->pm2_awc_t_dsb->setValue(data.T);

#if 0
			auto cassManager = getSubsystem()->getKernel()->getKernelModule<FortrendCassetteManager>();
			auto cass = cassManager->getCassette(getSubsystem().get());

			//update cassete status
			Cassette::Mapping mappingData = Cassette::Mapping::Unknown;

			std::shared_ptr<FortrendStation> station = getSelectStation();
			if (station->getName() == "LLA")
			{
				d->lk1->getFirstLayerMapping(mappingData);
				cass->setMapping(2, mappingData);

				d->lk1->getSecondLayerMapping(mappingData);
				cass->setMapping(1, mappingData);
			}
			else if(station->getName() == "LLB")
			{
				d->lk2->getFirstLayerMapping(mappingData);
				cass->setMapping(2, mappingData);

				d->lk2->getSecondLayerMapping(mappingData);
				cass->setMapping(1, mappingData);
			}
#endif
		}
		catch (KernelException& e){
			logError(getSubsystem()->getName().c_str(), e.what());
			//throw e;
		}
		

		
	}


}