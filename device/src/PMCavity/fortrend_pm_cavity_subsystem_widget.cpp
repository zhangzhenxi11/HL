
/**
* @file            fortrend_pm_cavity_subsystem.h
* @brief           Fortrend PMCavity widget
* @author		   xielonghua
*/

// Library: Fortrend
// Package: SubSystem//PMCavity

#include  "PMCavity/fortrend_pm_cavity_subsystem_widget.h"
#include  "Kernel/kernel.h"
#include  "Kernel/kernel_subsystem_reset_command.h"
#include  "Kernel/kernel_subsystem_update_command.h"
#include  "Kernel/CoreUI/kernel_subsystem_status_widget.h"  
#include  "Kernel/FortrendUI/slot_widget.h"
#include  "Kernel/FortrendUI/cassette_widget.h"
#include  "Kernel/Fortrend/robot_wafer_abstract_subsystem.h"
#include  "Kernel/Fortrend/fortrend_cassette_manager.h"
#include  "Kernel/Fortrend/cassette.h"

#include  "device/ui_fortrend_pm_cavity_subsystem_widget.h"
#include  "PMCavity/fortrend_pm_cavity_defined.h"



#include <QRadioButton>
#include <QSpacerItem>
#include <QMessageBox>
#include <QCheckBox>

#if _MSC_VER >1600
#pragma execution_character_set("utf-8")
#endif


namespace FC{
	/**
	* QPMCavitySubsystemWidgetPrivate
	**/
	class QPMCavitySubsystemWidgetPrivate{
		Q_DECLARE_PUBLIC(QPMCavitySubsystemWidget)
	public:
		QPMCavitySubsystemWidgetPrivate(QPMCavitySubsystemWidget* p);
	public:
		Ui::FortrendPMCavitySubsystem* ui;
		QPMCavitySubsystemWidget* q_ptr;

		//std::vector<QCheckBox*> arm_stat;
		std::vector<QCheckBox*> input_checkboxs;
		QCheckBox* tm_cavity_door_ckb = 0;

	};

	QPMCavitySubsystemWidgetPrivate::QPMCavitySubsystemWidgetPrivate(
		QPMCavitySubsystemWidget* p)
		:q_ptr(p){

	}

	/**
	* QPMCavitySubsystemWidget
	**/
	QPMCavitySubsystemWidget::QPMCavitySubsystemWidget(
		const std::shared_ptr<FortrendPMCavitySubsystem>& robot,
		QWidget*parent)
		: QAbstractSubsystemWidget<FortrendPMCavitySubsystem>(robot, parent)
		, d_ptr(new QPMCavitySubsystemWidgetPrivate(this)){

		Q_D(QPMCavitySubsystemWidget);
		d->ui = new Ui::FortrendPMCavitySubsystem;
		d->ui->setupUi(this);

		init();

		onAttributeUpdate();

		connect(d->ui->reset_btn, &QAbstractButton::clicked, this, &QPMCavitySubsystemWidget::onReset);
		connect(d->ui->generate_btn, &QAbstractButton::clicked, this, &QPMCavitySubsystemWidget::onGetStatus);
		connect(d->ui->open_tm_cavity_door_btn, &QAbstractButton::clicked, this, &QPMCavitySubsystemWidget::onOpenTMCavityDoor);
		connect(d->ui->close_tm_cavity_door_btn, &QAbstractButton::clicked, this, &QPMCavitySubsystemWidget::onCloseTMCavityDoor);
		connect(d->ui->get_finished_btn, &QAbstractButton::clicked, this, &QPMCavitySubsystemWidget::onGetFinished);
		connect(d->ui->upload_finished_btn, &QAbstractButton::clicked, this, &QPMCavitySubsystemWidget::onUplaodFinished);
		connect(d->ui->clear_pm_state_btn, &QAbstractButton::clicked, this, &QPMCavitySubsystemWidget::onClearState);
		connect(d->ui->read_process_parameters_btn, &QAbstractButton::clicked, this, &QPMCavitySubsystemWidget::onReadProcessParameters);
		connect(d->ui->write_process_parameters_btn, &QAbstractButton::clicked, this, &QPMCavitySubsystemWidget::onWriteProcessParameters);
		//connect(d->ui->control_inserting_plate_btn, &QAbstractButton::clicked, this, &QPMCavitySubsystemWidget::onInsertingPlateOpeningController);
		connect(d->ui->to_get_btn, &QAbstractButton::clicked, this, &QPMCavitySubsystemWidget::onToGetStation);
		connect(d->ui->to_put_btn, &QAbstractButton::clicked, this, &QPMCavitySubsystemWidget::onToPutStation);
		connect(d->ui->to_rotating_btn, &QAbstractButton::clicked, this, &QPMCavitySubsystemWidget::onToRotatingStation);
		connect(d->ui->to_target_pos_btn, &QAbstractButton::clicked, this, &QPMCavitySubsystemWidget::onToTargetPos);
		connect(d->ui->to_rotating_degree_btn, &QAbstractButton::clicked, this, &QPMCavitySubsystemWidget::onToRotatingDegreeStation);

		//connect(d->ui->lifting_axis_reset_btn, &QAbstractButton::clicked, this, &QPMCavitySubsystemWidget::onLiftingAxisRest);
		//connect(d->ui->rotating_axis_reset_btn, &QAbstractButton::clicked, this, &QPMCavitySubsystemWidget::onRotaingAxisRest);

		//d->ui->open_tm_cavity_door_btn->hide();
		//d->ui->close_tm_cavity_door_btn->hide();
		d->ui->upload_finished_btn->hide();
		d->ui->get_finished_btn->hide();
		//d->ui->to_get_btn->hide();
		//d->ui->to_put_btn->hide();
		d->ui->get_finished_btn->hide();
		d->ui->upload_finished_btn->hide();
		d->ui->label_50->hide();

	}

	QPMCavitySubsystemWidget::~QPMCavitySubsystemWidget(){

	}


	void QPMCavitySubsystemWidget::init(){
		Q_D(QPMCavitySubsystemWidget);

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

		d->tm_cavity_door_ckb = new QCheckBox(QString("取放料信号"));

		d->tm_cavity_door_ckb->setObjectName("io_object");

		d->tm_cavity_door_ckb->setEnabled(false);

		d->ui->operation_state_gridLayout->addWidget(d->tm_cavity_door_ckb, 0, 0);

		if (cass){
			QWidget* cassette_Widget = new QFortrendCassetteWidget(cass, cassManager, true, true, 25, this); //max row count = 25
			d->ui->center_layout->addWidget(cassette_Widget);

		}
	}

	void QPMCavitySubsystemWidget::onReset(){
		Q_D(QPMCavitySubsystemWidget);
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createResetCommand();
		executeCommand(getSubsystem(), cmd);
	}


	void QPMCavitySubsystemWidget::onGetStatus(){
		Q_D(QPMCavitySubsystemWidget);
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createUpdateCommand();
		executeCommand(getSubsystem(), cmd);
	}


	void QPMCavitySubsystemWidget::onOpenTMCavityDoor(){
		Q_D(QPMCavitySubsystemWidget);
		//TODO open box

		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createOpenTMCavityDoorCommand();
		executeCommand(getSubsystem(), cmd);

	}

	void QPMCavitySubsystemWidget::onCloseTMCavityDoor(){
		Q_D(QPMCavitySubsystemWidget);
		//TODO close box

		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createCloseTMCavityDoorCommand();
		executeCommand(getSubsystem(), cmd);
	}

	void QPMCavitySubsystemWidget::onGetFinished(){
		Q_D(QPMCavitySubsystemWidget);
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createGetFinishedCommand();
		executeCommand(getSubsystem(), cmd);
	}
	void  QPMCavitySubsystemWidget::onUplaodFinished(){
		Q_D(QPMCavitySubsystemWidget);
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createUploadFinishedCommand();
		executeCommand(getSubsystem(), cmd);
	}

	void QPMCavitySubsystemWidget::onToGetStation(){
		Q_D(QPMCavitySubsystemWidget);
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createToGetStationCommand();
		executeCommand(getSubsystem(), cmd);
	}
	void  QPMCavitySubsystemWidget::onToPutStation(){
		Q_D(QPMCavitySubsystemWidget);
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createToPutStationCommand();
		executeCommand(getSubsystem(), cmd);
	}

	void QPMCavitySubsystemWidget::onToRotatingStation()
	{
		Q_D(QPMCavitySubsystemWidget);
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createToRotatingStationCommand();
		executeCommand(getSubsystem(), cmd);
	}

	void QPMCavitySubsystemWidget::onToTargetPos()
	{
		Q_D(QPMCavitySubsystemWidget);
		double target_position = d->ui->lifting_axis_target_position_dsp->value();
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createLiftingActionCommand(target_position);
		executeCommand(getSubsystem(), cmd);
		
	}

	void QPMCavitySubsystemWidget::onToRotatingDegreeStation()
	{
		Q_D(QPMCavitySubsystemWidget);
		double  target_position =  d->ui->rotating_axis_target_position_dsp->value();
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createRotatingActionCommand(target_position);
		executeCommand(getSubsystem(), cmd);
	}

	void QPMCavitySubsystemWidget::onLiftingAxisRest()
	{
		Q_D(QPMCavitySubsystemWidget);
		getSubsystem()->setRotationHome(true);
	}

	void QPMCavitySubsystemWidget::onRotaingAxisRest()
	{
		Q_D(QPMCavitySubsystemWidget);
		getSubsystem()->setLiftingHome(true);
	}

	void QPMCavitySubsystemWidget::onReadProcessParameters(){
		Q_D(QPMCavitySubsystemWidget);
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createReadProcessParametersCommand();
		executeCommand(getSubsystem(), cmd);
	}

	void QPMCavitySubsystemWidget::onWriteProcessParameters(){
		Q_D(QPMCavitySubsystemWidget);
	
		PMCavityAxisSettingParameters param;
		param.lifting_axis_acce = d->ui->lifting_axis_acce_dsp->value();
		param.lifting_axis_dece = d->ui->lifting_axis_dece_dsp->value();
		param.lifting_axis_startup_speed = d->ui->lifting_axis_startup_speed_dsp->value();
		param.lifting_axis_inch_movement = d->ui->lifting_axis_inch_movement_dsp->value();
		param.lifting_axis_jog_speed = d->ui->lifting_axis_jog_speed_dsp->value();
		param.lifting_axis_target1_position = d->ui->lifting_axis_target1_position_dsp->value();
		param.lifting_axis_target2_position = d->ui->lifting_axis_target2_position_dsp->value();
		param.lifting_axis_target3_position = d->ui->lifting_axis_target3_position_dsp->value();
		param.lifting_axis_target_position = d->ui->lifting_axis_target_position_dsp->value();

		param.rotating_axis_acce = d->ui->rotating_axis_acce_dsp->value();
		param.rotating_axis_dece = d->ui->rotating_axis_dece_dsp->value();
		param.rotating_axis_inch_movement = d->ui->rotating_axis_inch_movement_dsp->value();
		param.rotating_axis_jog_speed = d->ui->rotating_axis_jog_speed_dsp->value();
		param.rotating_axis_startup_speed = d->ui->rotating_axis_startup_speed_dsp->value();
		param.rotating_axis_target_position = d->ui->rotating_axis_target_position_dsp->value();

		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createWriteProcessParametersCommand(param);
		executeCommand(getSubsystem(), cmd);
	}

	void QPMCavitySubsystemWidget::onClearState(){
		Q_D(QPMCavitySubsystemWidget);
		int button = QMessageBox::question(this, "警告", "确认清除腔室内部有片标记吗？", QMessageBox::StandardButton::Yes, QMessageBox::StandardButton::No);
		if (QMessageBox::StandardButton::Yes != button)
		{
			return;
		}
		logInform(getSubsystem()->getName().c_str(), "确定清除腔室内部有片标记");
		KernelSubsystemCommand::Ptr cmd = getSubsystem()->createClearStateCommand();
		executeCommand(getSubsystem(), cmd);
	}

	void QPMCavitySubsystemWidget::onInsertingPlateOpeningController(){
		Q_D(QPMCavitySubsystemWidget);
		/*KernelSubsystemCommand::Ptr cmd = getSubsystem()->createInsertingPlateOpeningControllerCommand(d->ui->valve_opening_value_dsb->value());
		executeCommand(getSubsystem(), cmd);*/
	}


	void QPMCavitySubsystemWidget::onAttributeUpdate()throw(KernelException){
		Q_D(QPMCavitySubsystemWidget);
		try{

			d->tm_cavity_door_ckb->setChecked(getSubsystem()->getPMCavitySafeSignal());

			//update input
		/*	for (int i = 0; i < d->input_checkboxs.size(); i++){
				d->input_checkboxs.at(i)->setChecked(getSubsystem()->getInput(i));

			}*/

			//update object stat
			/*for (int i = 0; i < d->arm_stat.size(); i++){
				d->arm_stat.at(i)->setChecked(getSubsystem()->hasObject(i));
				}*/

			//update door state
			//d->tm_cavity_door_ckb->setChecked(getSubsystem()->hasDoorOpend());

			////update pm cavity moniter value
			//auto moniter_state = getSubsystem()->getPMCavityMonityValue();
			//d->ui->cavity_temperature_let->setText(QString::number(getSubsystem()->getTemperatureValue(), 'f', 2).append("℃"));
			////d->ui->cavity_vacuum_value_let->setText(std::to_string(moniter_state.vacuum_value).c_str());
			////d->ui->pre_stage_pippeline_vacuum_value_let->setText(std::to_string(moniter_state.pre_stage_pippeline_vacuum).c_str());

			////d->ui->cavity_vacuum_value_let->setText(QString::number(moniter_state.vacuum_value, 'e', 3).replace("e", "x10^(").append(")"));
			////d->ui->pre_stage_pippeline_vacuum_value_let->setText(QString::number(moniter_state.pre_stage_pippeline_vacuum, 'e', 3).replace("e", "x10^(").append(")"));

			//d->ui->cavity_vacuum_value_let->setText(QString::number(moniter_state.vacuum_value, 'e', 3).append("Pa"));
			//d->ui->pre_stage_pippeline_vacuum_value_let->setText(QString::number(moniter_state.pre_stage_pippeline_vacuum, 'e', 3).append("Pa"));
			//d->ui->sample_rotation_speed_let->setText(QString::number(moniter_state.sample_rotation_speed, 'f', 3).append("°/S"));
			//d->ui->flow_meter_1_let->setText(QString::number(moniter_state.flow_meter_1, 'f', 3).append("sccm"));
			//d->ui->flow_meter_2_let->setText(QString::number(moniter_state.flow_meter_2, 'f', 3).append("sccm"));
			//d->ui->flow_meter_3_let->setText(QString::number(moniter_state.flow_meter_3, 'f', 3).append("sccm"));
			//d->ui->voltage_1_let->setText(std::to_string(moniter_state.dc_vlotage_1).c_str());
			//d->ui->voltage_2_let->setText(std::to_string(moniter_state.dc_vlotage_2).c_str());
			//d->ui->voltage_3_let->setText(std::to_string(moniter_state.dc_vlotage_3).c_str());
			//d->ui->current_1_let->setText(std::to_string(moniter_state.dc_current_1).c_str());
			//d->ui->current_2_let->setText(std::to_string(moniter_state.dc_current_2).c_str());
			//d->ui->current_3_let->setText(std::to_string(moniter_state.dc_current_3).c_str());
			//d->ui->power_1_let->setText(std::to_string(moniter_state.dc_power_1).c_str());
			//d->ui->power_2_let->setText(std::to_string(moniter_state.dc_power_2).c_str());
			//d->ui->power_3_let->setText(std::to_string(moniter_state.dc_power_3).c_str());

			//auto film = getSubsystem()->getPMCavityFilmValue();
			//d->ui->film_frequency_let->setText(QString::number(film.frequency).append("Hz"));
			//d->ui->film_rate_let->setText(QString::number(film.rate).append("A/s"));
			//d->ui->film_accumulative_total_let->setText(QString::number(film.accumulative_total).append("A"));
			//d->ui->film_lifetime_let->setText(QString::number(film.lifetime).append("%"));


			//if (getSubsystem()->getPMCavityUpdateProcessParameters())
			//{
			//	auto process_param = getSubsystem()->getPMCavityProcessParameters();
			//	d->ui->heating_temperature_dsb->setValue(process_param.heating_temperature);
			//	d->ui->initial_extraction_pressure_dsb->setValue(process_param.initial_extraction_pressure);
			//	d->ui->purified_extraction_pressure_dsb->setValue(process_param.purified_extraction_pressure);
			//	d->ui->sputtering_pressure_dsb->setValue(process_param.sputtering_pressure);
			//	d->ui->sputtering_flow_rate1_dsb->setValue(process_param.sputtering_flow_rate1);
			//	d->ui->sputtering_flow_rate2_dsb->setValue(process_param.sputtering_flow_rate2);
			//	d->ui->sputtering_flow_rate3_dsb->setValue(process_param.sputtering_flow_rate3);

			//	d->ui->sputtering_power1_dsb->setValue(process_param.sputtering_power1);
			//	d->ui->sputtering_power_gear_up1_dsb->setValue(process_param.sputtering_power_gear_up1);
			//	d->ui->sputtering_power2_dsb->setValue(process_param.sputtering_power2);
			//	d->ui->sputtering_power_gear_up2_dsb->setValue(process_param.sputtering_power_gear_up2);
			//	d->ui->sputtering_power3_dsb->setValue(process_param.sputtering_power3);
			//	d->ui->sputtering_power_gear_up3_dsb->setValue(process_param.sputtering_power_gear_up3);
			//	d->ui->pre_sputtering_time_dsb->setValue(process_param.pre_sputtering_time);
			//	d->ui->substrate_speed_dsb->setValue(process_param.substrate_speed);
			//	d->ui->process_sputtering_time_dsb->setValue(process_param.process_sputtering_time);

			//	d->ui->cathode_power_selection_1_cbx->setCurrentIndex(process_param.cathode_power_selection_1);
			//	d->ui->cathode_power_selection_2_cbx->setCurrentIndex(process_param.cathode_power_selection_2);
			//	d->ui->cathode_power_selection_3_cbx->setCurrentIndex(process_param.cathode_power_selection_3);
			//}

		}
		catch (KernelException& e){
			logError(getSubsystem()->getName().c_str(), e.what());
			//throw e;
		}



	}

}
