// Library: VTM
// Package: VTM

#include "kernel/kernel_subsystem.h"
#include "Kernel/Fortrend/fortrend_cassette_manager.h"
#include "Kernel/Fortrend/cassette.h"
#include "Kernel/kernel_log.h"

#include "device\ui_fortrend_station_status_vtm_widget.h"
#include "fortrend_station_status_vtm_widget.h"

#include "Aligner/fortrend_aligner_subsystem.h"
#include "CoolingCavity/fortrend_cooling_cavity_subsystem.h"
#include "LoadLock/fortrend_loadlock_subsystem.h"
#include "PMCavity/fortrend_pm_cavity_subsystem.h"
#include "SunwayRobot/fortrend_sunwayrobot_subsystem.h"
#include "kernel/FortrendUI/mapper_status_widget.h"
#include "kernel/kernel_abstract_subsystem.h"
#include "Kernel/kernel_event_module.h"
#include "Kernel/kernel_event_paramters.h"

#include "Kernel/FortrendUI/slot_widget.h"
#include  "Kernel/FortrendUI/cassette_widget.h"

#include  "SunwayRobot/fortrend_main_sunwayrobot_subsystem_widget.h"
#include  "TMCavity/fortrend_main_tm_cavity_subsystem_widget.h"
#include  "PMCavity/fortrend_main_pm_cavity_subsystem_widget.h"
#include  "LoadLock/fortrend_main_loadlock_subsystem_widget.h"
#include "SunwayRobot/fortrend_sunwayrobot_output_command.h"
#include "SunwayRobot/fortrend_sunwayrobot_reset_command.h"
#include "SunwayRobot/fortrend_sunwayrobot_update_command.h"
#include "SunwayRobot/fortrend_sunwayrobot_get_wafer_command.h"
#include "SunwayRobot/fortrend_sunwayrobot_put_wafer_command.h"
#include "SunwayRobot/fortrend_sunwayrobot_ready_get_wafer_command.h"
#include "SunwayRobot/fortrend_sunwayrobot_ready_put_wafer_command.h"
#include "SunwayRobot/fortrend_sunwayrobot_set_speed_command.h"
#include "SunwayRobot/fortrend_sunwayrobot_clear_error_command.h"

#include "LoadLock/fortrend_loadlock_reset_command.h"
#include "LoadLock/fortrend_loadlock_open_cassette_door_command.h"
#include "LoadLock/fortrend_loadlock_close_cassette_door_command.h"
#include "LoadLock/fortrend_loadlock_mapping_command.h"
#include "LoadLock/fortrend_loadlock_move_to_slot_command.h"
#include "LoadLock/fortrend_loadlock_close_tm_cavity_door_command.h"
#include "LoadLock/fortrend_loadlock_open_tm_cavity_door_command.h"
#include "LoadLock/fortrend_loadlock_close_exhaust_valve_command.h"
#include "LoadLock/fortrend_loadlock_open_exhaust_valve_command.h"

#include "LoadLock/fortrend_loadlock_cavity_close_height_vacuum_baffle_valve_command.h"
#include "LoadLock/fortrend_loadlock_cavity_close_inserting_plate_valve_command.h"
#include "LoadLock/fortrend_loadlock_cavity_open_height_vacuum_baffle_valve_command.h"
#include "LoadLock/fortrend_loadlock_cavity_open_inserting_plate_valve_command.h"

#include "LoadLock/fortrend_loadlock_close_diaphragm_valve_command.h"
#include "LoadLock/fortrend_loadlock_open_diaphragm_valve_command.h"
#include "LoadLock/fortrend_loadlock_close_angle_valve_command.h"
#include "LoadLock/fortrend_loadlock_open_angle_valve_command.h"
#include "LoadLock/fortrend_loadlock_clear_error_command.h"

#include "Aligner/fortrend_aligner_align_command.h"

#include "VTMSignalTower/fortrend_vtm_signal_tower.h"

#include  "PMCavity/fortrend_pm_cavity_to_get_station_command.h"
#include  "PMCavity/fortrend_pm_cavity_to_put_station_command.h"

#include "fortrend_device_kernel.h"
#include "vtmrobot.h"
#include "robotdialog.h"
#include "tm.h"

#include <thread>
#include <Windows.h>
#include <iostream>
#include <vector>

#include <qurl.h>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMap>
#include <QLabel>
#include <QPixmap>
#include <QPalette>
#include <QCoreApplication>
#include <QString>
#include <qdebug.h>
#include "QMessageBox"


#if _MSC_VER >1600
#pragma execution_character_set("utf-8")
#endif

using namespace Qt;

namespace FC{

	class QFortrendStationStatusVTMWidgetPrivate : public KernelListener<KernelAbstractSubSystem>, public FC::KernelEventListener, public FC::KernelListener<FC::Cassette>,
		public FC::KernelListener<FC::FortrendCassetteManager>{
	public:
		Q_DECLARE_PUBLIC(QFortrendStationStatusVTMWidget)
		QFortrendStationStatusVTMWidgetPrivate(QFortrendStationStatusVTMWidget* p, const std::shared_ptr<IKernel>& kernel);
		~QFortrendStationStatusVTMWidgetPrivate();
		void onAttributeChange();  
		virtual void onAttributeChange(const KernelAbstractSubSystem* arg);
		void onProcess();
		void onKernelEvent(FC::KernelEventModule* kernelModule, const std::shared_ptr<FC::IEventId>& eventId, FC::KernelEventParameter* context)override;
		void onAttributeChange(const FC::FortrendCassetteManager *arg) override;
		void onAttributeChange(const FC::Cassette *arg) override;
		void onRecipe();
		void onEfemReset();


	private:
		Ui::FortrendStationStatusVTMWidget* ui;
		//stackedWidget显示，就是模组手动界面
		QMainLoadLockSubsystemWidget* mainLoadlock1Widget;
		QMainLoadLockSubsystemWidget* mainLoadlock2Widget;
		QMainSunwayRobotSubsystemWidget* mainRobotWidget;
		QMainTMCavitySubsystemWidget* mainTMWidget;

		QMainPMCavitySubsystemWidget* mainPM1Widget;//add
		QMainPMCavitySubsystemWidget* mainPM2Widget;
		QMainPMCavitySubsystemWidget* mainPM3Widget;//add
		QMainPMCavitySubsystemWidget* mainPM4Widget;

		std::vector<QLabel*> loadlock1_cassete_slot_state;
		std::vector<QLabel*> loadlock2_cassete_slot_state;
		RobotDialog *robotdialog;
		QFortrendSlotWidget* slot_widgetA;
		QFortrendSlotWidget* slot_widgetB;
		//vtmrobot *robot;


		std::shared_ptr<IKernel> kernel;
		QFortrendStationStatusVTMWidget* q_ptr;

		//模组指针
		std::shared_ptr<FortrendLoadLockSubsystem> lk1;
		std::shared_ptr<FortrendLoadLockSubsystem> lk2;
		//PM
		std::shared_ptr<FortrendPMCavitySubsystem> pm1;
		std::shared_ptr<FortrendPMCavitySubsystem> pm2;
		std::shared_ptr<FortrendPMCavitySubsystem> pm3;
		std::shared_ptr<FortrendPMCavitySubsystem> pm4;

		std::shared_ptr<FortrendTMCavitySubsystem> tm;
		std::shared_ptr<FortrendAlignerSubsystem> aligner;
		std::shared_ptr<FortrendSunwayRobotSubsystem> wtr;
		//25-7-9 新增
		std::shared_ptr<FortrendVTMSignalTower> tower;

		std::shared_ptr<FortrendCassetteManager> cassManager;

		//执行指令的widget，继承了QKernelModuleWidget
		QAbstractSubsystemWidget<FortrendLoadLockSubsystem> *lk1widget;
		QAbstractSubsystemWidget<FortrendLoadLockSubsystem> *lk2widget;
		QAbstractSubsystemWidget<FortrendPMCavitySubsystem> *pm1widget;
		QAbstractSubsystemWidget<FortrendPMCavitySubsystem> *pm2widget;
		QAbstractSubsystemWidget<FortrendPMCavitySubsystem> *pm3widget;
		QAbstractSubsystemWidget<FortrendPMCavitySubsystem>* pm4widget;
		
		QAbstractSubsystemWidget<FortrendTMCavitySubsystem> *tmwidget;
		QAbstractSubsystemWidget<FortrendSunwayRobotSubsystem> *wtrwidget;


		int stationidlk1 = 1;
		int stationidpm1 = 2;
		int stationidpm2 = 3;
		int stationidpm3 = 4;
		int stationidpm4 = 5;
		int stationidlk2 = 6;
		int stationidaligner=5;
	
	};

	QFortrendStationStatusVTMWidgetPrivate::QFortrendStationStatusVTMWidgetPrivate(QFortrendStationStatusVTMWidget* p, const std::shared_ptr<IKernel>& kernel)
		:q_ptr(p)
		, kernel(kernel)
		, ui(new Ui::FortrendStationStatusVTMWidget){
		//setBackgroundImage("images/station_status.png");
		lk1 = kernel->getKernelModule<FortrendLoadLockSubsystem>("LLA");
		lk2 = kernel->getKernelModule<FortrendLoadLockSubsystem>("LLB");

		pm1 = kernel->getKernelModule<FortrendPMCavitySubsystem>("PM1");
		pm2 = kernel->getKernelModule<FortrendPMCavitySubsystem>("PM2");
		pm3 = kernel->getKernelModule<FortrendPMCavitySubsystem>("PM3");
		pm4 = kernel->getKernelModule<FortrendPMCavitySubsystem>("PM4");

		tm  = kernel->getKernelModule<FortrendTMCavitySubsystem>("TM");
		wtr = kernel->getKernelModule<FortrendSunwayRobotSubsystem>("WTR");
		aligner = kernel->getKernelModule<FortrendAlignerSubsystem>("Aligner");
		tower = kernel->getKernelModule<FortrendVTMSignalTower>("Tower");

		cassManager = lk2->getKernel()->getKernelModule<FortrendCassetteManager>();
		robotdialog = new RobotDialog(p);
		robotdialog->setWindowFlags(robotdialog->windowFlags() | Qt::WindowStaysOnTopHint);

		wtr->KernelAbstractSubSystem::addListener(this);
		stationidlk1 = lk1->getStationId(wtr->getName());
		stationidlk2 = lk2->getStationId(wtr->getName());
		stationidpm2 = pm2->getStationId(wtr->getName());
		//stationidaligner = aligner->getStationId(wtr->getName());
		
		//  订阅者是this：当前界面,界面触发信号，模组就执行指令
		lk1->addEventListener(this);
		lk2->addEventListener(this);
		pm1->addEventListener(this);
		pm2->addEventListener(this);
		pm3->addEventListener(this);
		pm4->addEventListener(this);
		tm->addEventListener(this);
		wtr->addEventListener(this);
		//aligner->addEventListener(this);
		cassManager->addListener(this);
		
		for (auto* station : cassManager->allStations()){
			//logInform(kernel->getName().c_str(), "station:%s", station->getName().c_str());
			auto cass = cassManager->getCassette(station);
			if (!cass){
				logInform(kernel->getName().c_str(), "station:%s no cass", station->getName().c_str());
				continue;
			}
			cass->addListener(this);
			//onAttributeChange(cass.get());
		}
		//添加工位和手爪配置
#pragma region 添加工位和手爪配置
#if 0
		QButtonGroup* stationGroup = new QButtonGroup;
		//create station select
		for (int i = 0; i < wtr->getWorkStations().size(); i++)
		{
			auto station = wtr->getWorkStations().at(i);
			QRadioButton* radioButton = new QRadioButton;
			radioButton->setText(QString::fromStdString(station->getName()) + QString("[%1]").arg(station->getStationId(wtr->getName())));
			radioButton->setProperty("index", i); //index
			robotdialog->AddStation(radioButton);
			stationGroup->addButton(radioButton, i);
			if (i == 0)
			{
				radioButton->setChecked(true);
			}

			// static_cast<void(QButtonGroup::*)(QAbstractButton*)>(&QButtonGroup::buttonClicked)

			QObject::connect(stationGroup, static_cast<void(QButtonGroup::*)(QAbstractButton*)>(&QButtonGroup::buttonClicked), this, [=]() {
			
				
				// 获取对话框的布局
				QLayout* slotsLayout = robotdialog->findChild<QLayout*>("slots_layout"); // 假设布局有对象名

				// 清除布局中的所有内容
				if (slotsLayout) {
					QLayoutItem* child;
					while ((child = slotsLayout->takeAt(0)) != nullptr) {
						if (child->widget()) {
							delete child->widget();
						}
						delete child;
					}
				}
				//recreate
				Cassette::Ptr cass = cassManager->getCassette(station.get());
				if (cass) {
					QFortrendSlotWidget* w = new QFortrendSlotWidget(cass, 15, 20); //max row count = 5
					w->canSelected(true, false);
					//d->ui->slots_layout->addWidget(w);
					//d->ui->slots_layout->addStretch();
				}
				radioButton->setText(QString::fromStdString(station->getName()) + QString("[%1]").arg(station->getStationId(wtr->getName())));
			});

		}
#endif		

		//create arm select
		for (int i = 0; i < wtr->armCount(); i++)
		{
			//select
			std::string name = wtr->getArmName(i);
			QRadioButton* selectBtn = new QRadioButton(QString::fromStdString(name));
			selectBtn->setProperty("index", i);
			robotdialog->AddArm(selectBtn);
			if (i == 0)
				selectBtn->setChecked(true);
		}
#pragma endregion
	}

	QFortrendStationStatusVTMWidgetPrivate::~QFortrendStationStatusVTMWidgetPrivate(){
		delete ui;
		auto cass_manager = kernel->getKernelModule<FC::FortrendCassetteManager>();
		cass_manager->removeListener(this);
		for (auto* station : cass_manager->allStations()){
			auto cass = cass_manager->getCassette(station);
			if (cass){
				cass->removeListener(this);
			}
		}
	}

	void QFortrendStationStatusVTMWidgetPrivate::onAttributeChange(const FC::Cassette *arg) {
		auto station = arg->getStation();
		if (!station){
			return;
		}
		//logInform("Test", "name=%s isopened=%d %s", station->getName(), arg->isBoxOpened(), wtr->getName());

		QMetaObject::invokeMethod(q_ptr, "updateCassetteAnimation", 
			Qt::AutoConnection, 
			Q_ARG(std::string, station->getName()),
			Q_ARG(bool, arg->isBoxOpened()), 
			Q_ARG(std::vector<Cassette::Mapping>, arg->getAllMapping()));
	}

	void QFortrendStationStatusVTMWidgetPrivate::onAttributeChange(const FC::FortrendCassetteManager *arg) {
		auto cass_manager = kernel->getKernelModule<FC::FortrendCassetteManager>();
		auto stations = cass_manager->allStations();
		for (auto* station : cass_manager->allStations()){
			//			logInform(kernel->getName().c_str(), "station:%s", station->getName().c_str());
			auto cass = cass_manager->getCassette(station);
			if (!cass){
				//logInform(kernel->getName().c_str(), "station:%s no cass", station->getName().c_str());
				continue;
			}
			cass->addListener(this);
		}
	}
	
	

	//监控命令执行调整动画
	void QFortrendStationStatusVTMWidgetPrivate::onKernelEvent(FC::KernelEventModule* kernelModule, const std::shared_ptr<FC::IEventId>& eventId, FC::KernelEventParameter* context){
		//logInform("EventTest", "onKernelEvent");
		if (std::dynamic_pointer_cast<EVENT_ALARM_CREATE>(eventId)){
				auto sub_system = dynamic_cast<KernelAbstractSubSystem*>(kernelModule);
				auto name = QString::fromStdString(sub_system->getName());
				//logInform("EventTest", "EVENT_ALARM_CREATE name:%s", name);
				//报警暂停动画
				QMetaObject::invokeMethod(q_ptr, "pauseAnimation", Qt::AutoConnection);
				return;
			}
		if (std::dynamic_pointer_cast<EVENT_STATE_CHANGED>(eventId)){
			auto sub_system = dynamic_cast<KernelAbstractSubSystem*>(kernelModule);
			auto status = sub_system->getState();
			auto name = QString::fromStdString(sub_system->getName());

			//logInform("EventTest", "EVENT_STATE_CHANGED name:%s", name);

			return;
		}
		if (std::dynamic_pointer_cast<EVENT_COMMAND_END>(eventId)){
			auto x = dynamic_cast<KernelCommandParameter*>(context);
			if (!x){
				return;
			}
			
			auto command = x->value();
			if (!command){
				return;
			}
			auto name = kernelModule->getName();

			//LoadLock复位命令完成
			auto reset_command = std::dynamic_pointer_cast<KernelSubsystemResetCommand>(command);
			if (reset_command){
				if (name == "LLA"){//LoadLock1模块复位完成,
					ui->loadlockA_widget->SetCurrentSlot(0);
				}
				else if (name == "LLB"){
					ui->loadlockB_widget->SetCurrentSlot(0);
				}
				return;
			}

			//LoadLock移动槽命令完成
			auto movetoslot_command = std::dynamic_pointer_cast<LoadLockMoveToSlotCommand>(command);
			if (movetoslot_command){
				int slot = movetoslot_command->getSlot();
				if (name == "LLA"){
					ui->loadlockA_widget->SetCurrentSlot(slot);
				}
				else if (name == "LLB"){
					ui->loadlockB_widget->SetCurrentSlot(slot);
				}
			}
			auto setspeed = std::dynamic_pointer_cast<SunwayRobotSetSpeedCommand>(command);
			if (setspeed){
				int speed = setspeed->getSpeed();
				//logInform("EventTest", "SunwayRobotSetSpeedCommand End speed:%d", speed);
				if (speed <=5 && speed>0){
					ui->robot_widget->setBaseSpeed(5000);//圆的旋转速度
					ui->robot_widget->setArmSpeed(4500);//收缩的速度
				}
				else if (speed<=15&&speed>5){
					ui->robot_widget->setBaseSpeed(4000);
					ui->robot_widget->setArmSpeed(3000);
				}
				else if (speed <= 25 && speed>15){
					ui->robot_widget->setBaseSpeed(2500);
					ui->robot_widget->setArmSpeed(2000);
				}
				else if (speed <= 35&&speed>25){
					ui->robot_widget->setBaseSpeed(2200);
					ui->robot_widget->setArmSpeed(2000);
				}
				else if (speed <= 45&&speed >35){
					ui->robot_widget->setBaseSpeed(1700);
					ui->robot_widget->setArmSpeed(1600);
				}
				else if (speed <= 55&&speed >45){
					ui->robot_widget->setBaseSpeed(1600);
					ui->robot_widget->setArmSpeed(1500);
				}
				else if (speed <= 65 && speed >55){
					ui->robot_widget->setBaseSpeed(1500);
					ui->robot_widget->setArmSpeed(1400);
				}
				else if (speed <= 75 && speed >65){
					ui->robot_widget->setBaseSpeed(1500);
					ui->robot_widget->setArmSpeed(1300);
				}
				else if (speed <= 85 && speed >75){
					ui->robot_widget->setBaseSpeed(1400);
					ui->robot_widget->setArmSpeed(1300);
				}
				else{
					ui->robot_widget->setBaseSpeed(1300);
					ui->robot_widget->setArmSpeed(1300);
				}
			}
			return;
		}

		//if (std::dynamic_pointer_cast<EVENT_COMMAND_START>(eventId)){
		if (std::dynamic_pointer_cast<EVENT_COMMAND_RUNNING>(eventId)){
			auto x = dynamic_cast<KernelCommandParameter*>(context);
			if (!x){
				return;
			}
			auto command = x->value();
			if (!command){
				return;
			}
			auto name = kernelModule->getName();
			//过滤命令
			auto get_command = std::dynamic_pointer_cast<SunwayRobotGetWaferCommand>(command);
			if (get_command){
				int station = get_command->getStation()->getStationId(wtr->getName());
				int arm = get_command->getArm();
				ui->robot_widget->setCurrentStation(station);
				ui->robot_widget->setCurrentArm(arm);
				QMetaObject::invokeMethod(q_ptr, "Animation", Qt::AutoConnection, Q_ARG(int, station), Q_ARG(int, arm), Q_ARG(QString, "get"));
				//logInform(name.c_str(), "get Animation Start stationid=%d arm=%d", station, arm);
				return;
			}
			//过滤命令
			auto put_command = std::dynamic_pointer_cast<SunwayRobotPutWaferCommand>(command);
			if (put_command){
				int station = put_command->getStation()->getStationId(wtr->getName());
				int arm = put_command->getArm();
				ui->robot_widget->setCurrentStation(station);
				ui->robot_widget->setCurrentArm(arm);
				QMetaObject::invokeMethod(q_ptr, "Animation", Qt::AutoConnection, Q_ARG(int, station), Q_ARG(int, arm), Q_ARG(QString, "put"));
				//logInform(name.c_str(), "put Animation Start stationid=%d arm=%d", station, arm);
				return;
			}
			//过滤命令
			auto aligner_command = std::dynamic_pointer_cast<AlignerAlignCommand>(command);
			if (aligner_command){
				QMetaObject::invokeMethod(q_ptr, "alignerAnimation", Qt::AutoConnection);
				//logInform(name.c_str(), "Aligner Animation Start");
				return;
			}
			//过滤命令
			auto toput_command = std::dynamic_pointer_cast<PMCavityToPutStationCommand>(command);
			if (toput_command){
				int station = 0;
				int speed = 0;
				double AxleSpeed = pm2->getPMCavityAxleSpeed();
				double AxleLocation = pm2->getPMCavityAxleLocation();
				if (toput_command->stationid==1){
					station = 220;
					if (AxleLocation < 0.1){
						speed = 2000;
					}
					else{
						speed = (int)(std::fabs(AxleLocation - 39.8) * 10 / AxleSpeed * 1000);
					}
					
				}
				else{//取料位2
					station = 20;
					if (AxleLocation < 0.1){
						speed = 2000;
					}
					else{
						speed = (int)(std::fabs(AxleLocation - 1) * 10 / AxleSpeed * 1000);
					}
				}

				QMetaObject::invokeMethod(q_ptr, "pmAnimation", Qt::AutoConnection, Q_ARG(int, station), Q_ARG(int, speed));
				//logInform(name.c_str(), "Aligner Animation Start");
				return;
			}

			//过滤命令
			auto toget_command = std::dynamic_pointer_cast<PMCavityToGetStationCommand>(command);
			if (toget_command){
				int station = 220;
				int speed = 0;
				double AxleSpeed = pm2->getPMCavityAxleSpeed();
				double AxleLocation = pm2->getPMCavityAxleLocation();
				if (AxleLocation < 0.1){
					speed = 2000;
				}
				else{
					speed = (int)(std::fabs(AxleLocation - 39.8) * 10 / AxleSpeed * 1000);
				}

				QMetaObject::invokeMethod(q_ptr, "pmAnimation", Qt::AutoConnection, Q_ARG(int, station), Q_ARG(int, speed));
				//logInform(name.c_str(), "Aligner Animation Start");
				return;
			}

			//过滤命令
			auto reset_command = std::dynamic_pointer_cast<PMCavityResetCommand>(command);
			if (reset_command){
				int station = 70;
				int speed = 0;
				double AxleSpeed = pm2->getPMCavityAxleSpeed();
				double AxleLocation = pm2->getPMCavityAxleLocation();
				if (AxleLocation < 0.1){
					speed = 2000;
				}
				else{
					speed = (int)(std::fabs(AxleLocation - 10) * 10 / AxleSpeed * 1000);
				}
				QMetaObject::invokeMethod(q_ptr, "pmAnimation", Qt::AutoConnection, Q_ARG(int, station), Q_ARG(int, speed));
				//logInform(name.c_str(), "Aligner Animation Start");
				return;
			}
			
			return;
		}
	}

	void QFortrendStationStatusVTMWidgetPrivate::onAttributeChange(const KernelAbstractSubSystem* arg){
		if (wtr->getRunningStatus() == "pause"){
			printf("onAttributeChange pause\n");
			ui->robot_widget->animationPause();
		}
		else if (wtr->getRunningStatus() == "normal"){
			printf("onAttributeChange normal\n");
			ui->robot_widget->animationResume();
		}
		else if (wtr->getRunningStatus() == "abort"){
			printf("onAttributeChange abort\n");
			ui->robot_widget->animationAbort();
		}
	}


	void QFortrendStationStatusVTMWidgetPrivate::onAttributeChange(){
		QMetaObject::invokeMethod(q_ptr, "updateState", Qt::AutoConnection);
	}

	void QFortrendStationStatusVTMWidgetPrivate::onProcess(){
		
	}

	/*
	 切换配方功能
	*/
	void QFortrendStationStatusVTMWidgetPrivate::onRecipe(){
		//0=A模式 1=B模式 2=先A后B模式 3=先B后A模式
		emit q_ptr->signalUpdateRecipe(ui->recipe_box->currentIndex());
	}

	void QFortrendStationStatusVTMWidgetPrivate::onEfemReset()
	{
		tower->EfemResetAll();
	}

	

	//QFortrendStationStatusVTMWidget
	QFortrendStationStatusVTMWidget::QFortrendStationStatusVTMWidget(const std::shared_ptr<IKernel>& kernel, QWidget* parent)
		:QWidget(parent)
		, d_ptr(new QFortrendStationStatusVTMWidgetPrivate(this, kernel)){
		Q_D(QFortrendStationStatusVTMWidget);

		
		d->ui = new Ui::FortrendStationStatusVTMWidget;
		d->ui->setupUi(this);
		d->ui->pm1_widget->setRotationAngle(0);
		d->ui->pm2_widget->setRotationAngle(60);
		d->ui->pm3_widget->setRotationAngle(120);
		d->ui->pm4_widget->setRotationAngle(180);
		d->ui->loadlockB_widget->SetIsLeft(false);

		d->ui->pm1_widget->setStatus(true);
		d->ui->pm2_widget->setStatus(true);
		d->ui->pm3_widget->setStatus(true);
		d->ui->pm4_widget->setStatus(true);

		d->ui->loadlockB_widget->SetName("LLB");
		d->ui->loadlockA_widget->SetName("LLA");
		d->ui->robot_widget->SetName("WTR");
		d->ui->pm1_widget->SetName("PM1");
		d->ui->pm2_widget->SetName("PM2");
		d->ui->pm3_widget->SetName("PM3");
		d->ui->pm4_widget->SetName("PM4");

		d->ui->tm_widget->SetName("TM");

		d->lk1widget = new QAbstractSubsystemWidget<FortrendLoadLockSubsystem>(d->lk1, this);
		d->lk2widget = new QAbstractSubsystemWidget<FortrendLoadLockSubsystem>(d->lk2, this);

		d->pm1widget = new QAbstractSubsystemWidget<FortrendPMCavitySubsystem>(d->pm1, this);
		d->pm2widget = new QAbstractSubsystemWidget<FortrendPMCavitySubsystem>(d->pm2, this);
		d->pm3widget = new QAbstractSubsystemWidget<FortrendPMCavitySubsystem>(d->pm3, this);
		d->pm4widget = new QAbstractSubsystemWidget<FortrendPMCavitySubsystem>(d->pm4, this);

		d->tmwidget = new QAbstractSubsystemWidget<FortrendTMCavitySubsystem>(d->tm, this);
		d->wtrwidget = new QAbstractSubsystemWidget<FortrendSunwayRobotSubsystem>(d->wtr, this);
	
		d->mainLoadlock1Widget = new QMainLoadLockSubsystemWidget(d->lk1,this);
		d->mainLoadlock2Widget = new QMainLoadLockSubsystemWidget(d->lk2, this);

		d->mainPM1Widget = new QMainPMCavitySubsystemWidget(d->pm1, this);
		d->mainPM2Widget = new QMainPMCavitySubsystemWidget(d->pm2, this);
		d->mainPM3Widget = new QMainPMCavitySubsystemWidget(d->pm3, this);
		d->mainPM4Widget = new QMainPMCavitySubsystemWidget(d->pm4, this);

		d->mainTMWidget = new QMainTMCavitySubsystemWidget(d->tm,d->aligner, this);
		d->mainRobotWidget = new QMainSunwayRobotSubsystemWidget(d->wtr, this);

		//加入到stackedWidget中
	    d->ui->stackedWidget->addWidget(d->mainLoadlock1Widget);
		d->ui->stackedWidget->addWidget(d->mainPM1Widget);
		d->ui->stackedWidget->addWidget(d->mainPM2Widget);
		d->ui->stackedWidget->addWidget(d->mainPM3Widget);
		d->ui->stackedWidget->addWidget(d->mainPM4Widget);
		d->ui->stackedWidget->addWidget(d->mainLoadlock2Widget);
		d->ui->stackedWidget->addWidget(d->mainTMWidget);
		d->ui->stackedWidget->addWidget(d->mainRobotWidget);
		initTipsUI();
		

		auto  casslk1 = d->cassManager->getCassette(d->lk1.get());
		if (casslk1){
			casslk1->setBoxId("A");
			d->slot_widgetA = new  QFortrendSlotWidget(casslk1);
			d->slot_widgetA->canSelected(true, false);
			d->slot_widgetA->setName(QString::fromStdString("LLA"));
			d->ui->gridLayoutA->addWidget(d->slot_widgetA, 0, 0, Qt::AlignLeft);
			
			QWidget* cassette_Widget = new QFortrendCassetteWidget(casslk1, d->cassManager, true, true, 25, this); //max row count = 25
			d->ui->center_layout->addWidget(cassette_Widget);
		}
		auto  casspm1 = d->cassManager->getCassette(d->pm1.get());
		if (casspm1){
			QWidget* cassette_Widget = new QFortrendCassetteWidget(casspm1, d->cassManager, true, true, 25, this); //max row count = 25
			d->ui->center_layout->addWidget(cassette_Widget);
		}
		auto  casspm2 = d->cassManager->getCassette(d->pm2.get());
		if (casspm2){
			if (casspm2->getMapping(1) == Cassette::Present){
				d->ui->pm2_widget->setWafer(true);
			}
			QWidget* cassette_Widget = new QFortrendCassetteWidget(casspm2, d->cassManager, true, true, 25, this); //max row count = 25
			d->ui->center_layout->addWidget(cassette_Widget);
		}
		auto  casspm3 = d->cassManager->getCassette(d->pm3.get());
		if (casspm3){
			QWidget* cassette_Widget = new QFortrendCassetteWidget(casspm3, d->cassManager, true, true, 25, this); //max row count = 25
			d->ui->center_layout->addWidget(cassette_Widget);
		}
		auto  casspm4 = d->cassManager->getCassette(d->pm4.get());
		if (casspm4) {
			QWidget* cassette_Widget = new QFortrendCassetteWidget(casspm4, d->cassManager, true, true, 25, this); //max row count = 25
			d->ui->center_layout->addWidget(cassette_Widget);
		}

		auto  casslk2 = d->cassManager->getCassette(d->lk2.get());
		if (casslk2){
			casslk2->setBoxId("B");
			d->slot_widgetB = new  QFortrendSlotWidget(casslk2);
			d->slot_widgetB->canSelected(true, false);
			d->slot_widgetB->setName(QString::fromStdString("LLB"));
			d->ui->gridLayoutB->addWidget(d->slot_widgetB, 0, 0, Qt::AlignLeft);
			QObject::connect(d->slot_widgetB, SIGNAL(clickd(int)), this, SLOT(_onSelectBSlot(int)));
			QWidget* cassette_Widget = new QFortrendCassetteWidget(casslk2, d->cassManager, true, true, 25, this); //max row count = 25
			d->ui->center_layout->addWidget(cassette_Widget);
		}

		//auto  cassaligner = d->cassManager->getCassette(d->aligner.get());
		//if (cassaligner){
		//	if (cassaligner->getMapping(1) == Cassette::Present){
		//		if (!d_ptr->ui->tm_widget->getIsWaferAligner()){
		//			d->ui->tm_widget->setIsWaferAligner(true);
		//		}
		//	}
		//	QWidget* cassette_Widget = new QFortrendCassetteWidget(cassaligner, d->cassManager, true, true, 25, this); //max row count = 25
		//	d->ui->center_layout->addWidget(cassette_Widget);
		//}

		auto  casswtr = d->cassManager->getCassette(d->wtr.get());
		if (casswtr){


			//std::string mapResult = casswtr->toString();
			//logInform(d->wtr->getName().c_str(),"mapResult:%s", mapResult);
			//qDebug() << "mapResult:" << mapResult.c_str() << endl;
			//casswtr->setMapping(1, Cassette::Empty);

			if (casswtr->getMapping(1) == Cassette::Present){
				if (!d_ptr->ui->robot_widget->getIsWaferArm1()){
					d_ptr->ui->robot_widget->setIsWaferArm1(true);
				}
			}

			if (casswtr->getMapping(2) == Cassette::Present){
				if (!d_ptr->ui->robot_widget->getIsWaferArm2()){
					d_ptr->ui->robot_widget->setIsWaferArm2(true);
				}
			}
			QWidget* cassette_Widget = new QFortrendCassetteWidget(casswtr, d->cassManager, true, true, 25, this); //max row count = 25
			d->ui->center_layout->addWidget(cassette_Widget);
		}

		//create station select
		for (int i = 0; i < d->wtr->getWorkStations().size(); i++)
		{
			auto station = d->wtr->getWorkStations().at(i);
			QRadioButton* radioButton = new QRadioButton;
			radioButton->setText(QString::fromStdString(station->getName()) + QString("[%1]").arg(station->getStationId(d->wtr->getName())));
			radioButton->setProperty("index", i); //index
			d->robotdialog->AddStation(radioButton);
			//stationGroup->addButton(radioButton, i);
			if (i == 0)
			{
				radioButton->setChecked(true);
			}
			
			//不能加在private类中，因为私有类不是Qt的作用域，找不到Qt
			QObject::connect(radioButton, &QRadioButton::clicked, this, [=]() {

				// 获取对话框的布局
				QLayout* slotsLayout = d->robotdialog->findChild<QLayout*>("slots_layout"); // 假设布局有对象名

				// 清除布局中的所有内容
				if (slotsLayout) {
					QLayoutItem* child;
					while ((child = slotsLayout->takeAt(0)) != nullptr) {
						if (child->widget()) {
							delete child->widget();
						}
						delete child;
					}
				}
				//recreate
				Cassette::Ptr cass = d->cassManager->getCassette(station.get());
				if (cass) {
					QFortrendSlotWidget* w = new QFortrendSlotWidget(cass, 15, 20); //max row count = 5
					w->canSelected(true, false);
					d->robotdialog->AddSlot(w);
				}
				radioButton->setText(QString::fromStdString(station->getName()) + QString("[%1]").arg(station->getStationId(d->wtr->getName())));
			});
		}

		
		//lambada 表达式做槽函数
		d->ui->stackedWidget->setCurrentIndex(7);
		connect(d->ui->loadlockA_widget, &LoadLockA::signalRightClick, this, [this]() {Q_D(QFortrendStationStatusVTMWidget); d->ui->stackedWidget->setCurrentIndex(2); d->ui->center_layout->setCurrentIndex(2);});
		
		connect(d->ui->pm1_widget, &PMGDTWidget::signalRightClick, this, [this]() {Q_D(QFortrendStationStatusVTMWidget); d->ui->stackedWidget->setCurrentIndex(3); d->ui->center_layout->setCurrentIndex(3);});
		
		connect(d->ui->pm2_widget, &PMGDTWidget::signalRightClick, this, [this]() {Q_D(QFortrendStationStatusVTMWidget); d->ui->stackedWidget->setCurrentIndex(4); d->ui->center_layout->setCurrentIndex(4);});
		
		connect(d->ui->pm3_widget, &PMGDTWidget::signalRightClick, this, [this]() {Q_D(QFortrendStationStatusVTMWidget); d->ui->stackedWidget->setCurrentIndex(5); d->ui->center_layout->setCurrentIndex(5);});
		
		connect(d->ui->pm4_widget, &PMGDTWidget::signalRightClick, this, [this]() {Q_D(QFortrendStationStatusVTMWidget); d->ui->stackedWidget->setCurrentIndex(6); d->ui->center_layout->setCurrentIndex(6); });

		connect(d->ui->loadlockB_widget, &LoadLockB::signalRightClick, this, [this]() {Q_D(QFortrendStationStatusVTMWidget); d->ui->stackedWidget->setCurrentIndex(7); d->ui->center_layout->setCurrentIndex(7);});

		connect(d->ui->tm_widget, &TM::signalRightClick, this, [this]() {Q_D(QFortrendStationStatusVTMWidget); d->ui->stackedWidget->setCurrentIndex(8); d->ui->center_layout->setCurrentIndex(8);});
		
		connect(d->ui->robot_widget, &vtmrobot::signalRightClick, this, [this]() {Q_D(QFortrendStationStatusVTMWidget); d->ui->stackedWidget->setCurrentIndex(9); d->ui->center_layout->setCurrentIndex(9);});
		
		connect(d->ui->recipe_btn, &QAbstractButton::clicked, this, &QFortrendStationStatusVTMWidget::onRecipe);

		connect(d->ui->reset_all_btn, &QAbstractButton::clicked, this, &QFortrendStationStatusVTMWidget::onEfemReset);
		

		d->ui->recipe_box->addItem("单上下片模式");
		d->ui->recipe_box->addItem("双上双下模式");
		//d->ui->recipe_box->addItem("先A后B模式");
		//d->ui->recipe_box->addItem("先B后A模式");
		if (d->pm2->getState() == IKernelSubSystem::SUB_NORMAL || d->pm2->getState() == IKernelSubSystem::SUB_IDEL){
			double AxleLocation = d->pm2->getPMCavityAxleLocation();//PM横移轴位置初始化
			printf("getPMCavityAxleLocation %d \r\n", AxleLocation);
			if (AxleLocation>35){
				d->ui->pm2_widget->setYOffset(220);//上下料位
			}
			else if (AxleLocation>25 && AxleLocation <= 35){
				d->ui->pm2_widget->setYOffset(220);//工艺位1
			}
			else if (AxleLocation>5 && AxleLocation <= 15){
				d->ui->pm2_widget->setYOffset(67);//原点位
			}
			else if (AxleLocation>0.1 && AxleLocation <= 5){
				d->ui->pm2_widget->setYOffset(10);//工艺位2
			}
			else{
				d->ui->pm2_widget->setYOffset(67);//原点位
			}
		}
#pragma region 机械手模块信号槽绑定
		QObject::connect(d->ui->robot_widget, &vtmrobot::signalRobotDialog, this, &QFortrendStationStatusVTMWidget::onRobotDialog);
		QObject::connect(d->ui->robot_widget, &vtmrobot::signalRobotReset, this, &QFortrendStationStatusVTMWidget::onRobotReset);
		QObject::connect(d->ui->robot_widget, &vtmrobot::signalRobotGetStatus, this, &QFortrendStationStatusVTMWidget::onRobotGetStatus);
		QObject::connect(d->ui->robot_widget, &vtmrobot::signalRobotClearError, this, &QFortrendStationStatusVTMWidget::onRobotClearError);

		QObject::connect(d->robotdialog, &RobotDialog::signalspeed, this, &QFortrendStationStatusVTMWidget::onRobotSpeed);
		QObject::connect(d->robotdialog, &RobotDialog::signalzspeed, this, &QFortrendStationStatusVTMWidget::onRobotZSpeed);
		//QObject::connect(d->ui->robot_widget, &vtmrobot::signalRobotData, this, &QFortrendStationStatusVTMWidget::onRobotData);
		//QObject::connect(d->ui->robot_widget, &vtmrobot::signalRobotStatusDialog, this, &QFortrendStationStatusVTMWidget::onRobotStatusDialog);
		QObject::connect(d->robotdialog, &RobotDialog::signalget, this, &QFortrendStationStatusVTMWidget::onRobotGet);
		QObject::connect(d->robotdialog, &RobotDialog::signalput, this, &QFortrendStationStatusVTMWidget::onRobotPut);
		QObject::connect(d->ui->robot_widget, &vtmrobot::extendAnimationGetFinished, this, &QFortrendStationStatusVTMWidget::onRobotGetFinished);
		QObject::connect(d->ui->robot_widget, &vtmrobot::extendAnimationPutFinished, this, &QFortrendStationStatusVTMWidget::onRobotPutFinished);
		connect(d->robotdialog, &QDialog::rejected, this, [this](){
			Q_D(QFortrendStationStatusVTMWidget);
			d->ui->tm_widget->IseventFilter = true;
		});//Dialog关闭误触发其他窗体点击事件，添加过滤标志
#pragma endregion
		
#pragma region LoadLock模块信号槽绑定
		QObject::connect(d->ui->loadlockA_widget, &LoadLockA::signalLoadLockAOpenbox, this, &QFortrendStationStatusVTMWidget::onLoadLockOpenbox);
		QObject::connect(d->ui->loadlockA_widget, &LoadLockA::signalLoadLockAClosebox, this, &QFortrendStationStatusVTMWidget::onLoadLockClosebox);
		QObject::connect(d->ui->loadlockA_widget, &LoadLockA::signalLoadLockAMapping, this, &QFortrendStationStatusVTMWidget::onLoadLockMapping);
		//QObject::connect(d->ui->loadlockA_widget, &LoadLockA::signalLoadLockAMoveToSlot, this, &QFortrendStationStatusVTMWidget::onLoadLockMoveToSlot);
		QObject::connect(d->ui->loadlockA_widget, &LoadLockA::signalLoadLockAOpenTMCavityDoor, this, &QFortrendStationStatusVTMWidget::onLoadLockOpenTMCavityDoor);
		QObject::connect(d->ui->loadlockA_widget, &LoadLockA::signalLoadLockACloseTMCavityDoor, this, &QFortrendStationStatusVTMWidget::onLoadLockCloseTMCavityDoor);
		QObject::connect(d->ui->loadlockA_widget, &LoadLockA::signalLoadLockAOpenDiaphragmValve, this, &QFortrendStationStatusVTMWidget::onLoadLockOpenDiaphragmValve);
		QObject::connect(d->ui->loadlockA_widget, &LoadLockA::signalLoadLockACloseDiaphragmValve, this, &QFortrendStationStatusVTMWidget::onLoadLockCloseDiaphragmValve);
		//QObject::connect(d->ui->loadlockA_widget, &LoadLockA::signalLoadLockAOpenExhaustValve, this, &QFortrendStationStatusVTMWidget::onLoadLockOpenExhaustValve);
		//QObject::connect(d->ui->loadlockA_widget, &LoadLockA::signalLoadLockACloseExhaustValve, this, &QFortrendStationStatusVTMWidget::onLoadLockCloseExhaustValve);

		//QObject::connect(d->ui->loadlockA_widget, &LoadLockA::signalLoaLockAOpenInsertingPlateValve, this, &QFortrendStationStatusVTMWidget::onOpenInsertingPlateValve);
		//QObject::connect(d->ui->loadlockA_widget, &LoadLockA::signalLoaLockACloseInsertingPlateValve, this, &QFortrendStationStatusVTMWidget::onCloseInsertingPlateValve);
		//QObject::connect(d->ui->loadlockA_widget, &LoadLockA::signalLoaLockAOpenHeightVacuumBaffleValve, this, &QFortrendStationStatusVTMWidget::onOpenHeightVacuumBaffleValve);
		//QObject::connect(d->ui->loadlockA_widget, &LoadLockA::signalLoaLockACloseHeightVacuumBaffleValve, this, &QFortrendStationStatusVTMWidget::onCloseHeightVacuumBaffleValve);

		QObject::connect(d->ui->loadlockA_widget, &LoadLockA::signalLoadLockAOpenAngleValve, this, &QFortrendStationStatusVTMWidget::onLoadLockOpenAngleValve);
		QObject::connect(d->ui->loadlockA_widget, &LoadLockA::signalLoadLockACloseAngleValve, this, &QFortrendStationStatusVTMWidget::onLoadLockCloseAngleValve);
		QObject::connect(d->ui->loadlockA_widget, &LoadLockA::signalLoadLockAGetStatus, this, &QFortrendStationStatusVTMWidget::onLoadLockGetStatus);
		QObject::connect(d->ui->loadlockA_widget, &LoadLockA::signalLoadLockAReset, this, &QFortrendStationStatusVTMWidget::onLoadLockReset);
		QObject::connect(d->ui->loadlockA_widget, &LoadLockA::signalLoadLockAClearError, this, &QFortrendStationStatusVTMWidget::onLoadLockClearError);
		QObject::connect(d->ui->loadlockA_widget, &LoadLockA::signalLoadLockAMoveToOrigin, this, &QFortrendStationStatusVTMWidget::onLoadLockMoveToOrigin);
		

		QObject::connect(d->ui->loadlockB_widget, &LoadLockB::signalLoadLockBOpenbox, this, &QFortrendStationStatusVTMWidget::onLoadLockOpenbox);
		QObject::connect(d->ui->loadlockB_widget, &LoadLockB::signalLoadLockBClosebox, this, &QFortrendStationStatusVTMWidget::onLoadLockClosebox);
		QObject::connect(d->ui->loadlockB_widget, &LoadLockB::signalLoadLockBMapping, this, &QFortrendStationStatusVTMWidget::onLoadLockMapping);
		//QObject::connect(d->ui->loadlockB_widget, &LoadLockB::signalLoadLockBMoveToSlot, this, &QFortrendStationStatusVTMWidget::onLoadLockMoveToSlot);
		QObject::connect(d->ui->loadlockB_widget, &LoadLockB::signalLoadLockBOpenTMCavityDoor, this, &QFortrendStationStatusVTMWidget::onLoadLockOpenTMCavityDoor);
		QObject::connect(d->ui->loadlockB_widget, &LoadLockB::signalLoadLockBCloseTMCavityDoor, this, &QFortrendStationStatusVTMWidget::onLoadLockCloseTMCavityDoor);
		QObject::connect(d->ui->loadlockB_widget, &LoadLockB::signalLoadLockBOpenDiaphragmValve, this, &QFortrendStationStatusVTMWidget::onLoadLockOpenDiaphragmValve);
		QObject::connect(d->ui->loadlockB_widget, &LoadLockB::signalLoadLockBCloseDiaphragmValve, this, &QFortrendStationStatusVTMWidget::onLoadLockCloseDiaphragmValve);
		//QObject::connect(d->ui->loadlockB_widget, &LoadLockB::signalLoadLockBOpenExhaustValve, this, &QFortrendStationStatusVTMWidget::onLoadLockOpenExhaustValve);
		//QObject::connect(d->ui->loadlockB_widget, &LoadLockB::signalLoadLockBCloseExhaustValve, this, &QFortrendStationStatusVTMWidget::onLoadLockCloseExhaustValve);

		//QObject::connect(d->ui->loadlockB_widget, &LoadLockB::signalLoaLockBOpenInsertingPlateValve, this, &QFortrendStationStatusVTMWidget::onOpenInsertingPlateValve);
		//QObject::connect(d->ui->loadlockB_widget, &LoadLockB::signalLoaLockBCloseInsertingPlateValve, this, &QFortrendStationStatusVTMWidget::onCloseInsertingPlateValve);
		//QObject::connect(d->ui->loadlockB_widget, &LoadLockB::signalLoaLockBOpenHeightVacuumBaffleValve, this, &QFortrendStationStatusVTMWidget::onOpenHeightVacuumBaffleValve);
		//QObject::connect(d->ui->loadlockB_widget, &LoadLockB::signalLoaLockBCloseHeightVacuumBaffleValve, this, &QFortrendStationStatusVTMWidget::onCloseHeightVacuumBaffleValve);

		QObject::connect(d->ui->loadlockB_widget, &LoadLockB::signalLoadLockBOpenAngleValve, this, &QFortrendStationStatusVTMWidget::onLoadLockOpenAngleValve);
		QObject::connect(d->ui->loadlockB_widget, &LoadLockB::signalLoadLockBCloseAngleValve, this, &QFortrendStationStatusVTMWidget::onLoadLockCloseAngleValve);
		QObject::connect(d->ui->loadlockB_widget, &LoadLockB::signalLoadLockBGetStatus, this, &QFortrendStationStatusVTMWidget::onLoadLockGetStatus);
		QObject::connect(d->ui->loadlockB_widget, &LoadLockB::signalLoadLockBReset, this, &QFortrendStationStatusVTMWidget::onLoadLockReset);
		QObject::connect(d->ui->loadlockB_widget, &LoadLockB::signalLoadLockBClearError, this, &QFortrendStationStatusVTMWidget::onLoadLockClearError);
		QObject::connect(d->ui->loadlockB_widget, &LoadLockB::signalLoadLockAMoveToOrigin, this, &QFortrendStationStatusVTMWidget::onLoadLockMoveToOrigin);
#pragma endregion
	
#pragma region TM模块信号槽绑定
		QObject::connect(d->ui->tm_widget, &TM::signalTMOpenDiaphragmValve, this, &QFortrendStationStatusVTMWidget::onTMOpenDiaphragmValve);
		QObject::connect(d->ui->tm_widget, &TM::signalTMCloseDiaphragmValve, this, &QFortrendStationStatusVTMWidget::onTMCloseDiaphragmValve);
		//QObject::connect(d->ui->tm_widget, &TM::signalTMOpenHeightVacuumBaffleValve, this, &QFortrendStationStatusVTMWidget::onTMOpenHeightVacuumBaffleValve);
		//QObject::connect(d->ui->tm_widget, &TM::signalTMCloseHeightVacuumBaffleValve, this, &QFortrendStationStatusVTMWidget::onTMCloseHeightVacuumBaffleValve);
		QObject::connect(d->ui->tm_widget, &TM::signalTMOpenAngleValve, this, &QFortrendStationStatusVTMWidget::onTMOpenAngleValve);
		QObject::connect(d->ui->tm_widget, &TM::signalTMCloseAngleValve, this, &QFortrendStationStatusVTMWidget::onTMCloseAngleValve);
		//QObject::connect(d->ui->tm_widget, &TM::signalTMOpenInsertingPlateValve, this, &QFortrendStationStatusVTMWidget::onTMOpenInsertingPlateValve);
		//QObject::connect(d->ui->tm_widget, &TM::signalTMCloseInsertingPlateValve, this, &QFortrendStationStatusVTMWidget::onTMCloseInsertingPlateValve);
		QObject::connect(d->ui->tm_widget, &TM::signalTMOpenFlowmeterDiaphragmValve, this, &QFortrendStationStatusVTMWidget::onTMOpenFlowmeterDiaphragmValve);
		QObject::connect(d->ui->tm_widget, &TM::signalTMCloseFlowmeterDiaphragmValve, this, &QFortrendStationStatusVTMWidget::onTMCloseFlowmeterDiaphragmValve);
		QObject::connect(d->ui->tm_widget, &TM::signalTMReset, this, &QFortrendStationStatusVTMWidget::onTMReset);
		QObject::connect(d->ui->tm_widget, &TM::signalTMGetStatus, this, &QFortrendStationStatusVTMWidget::onTMGetStatus);

		QObject::connect(d->ui->tm_widget, &TM::signalAlignerReset, this, &QFortrendStationStatusVTMWidget::onAlignerReset);
		QObject::connect(d->ui->tm_widget, &TM::signalAlignerAlign, this, &QFortrendStationStatusVTMWidget::onAlignerAlign);
#pragma endregion

#pragma region PM模块信号槽绑定
		QObject::connect(d->ui->pm1_widget, &PMGDTWidget::signalPMOpenTMCavityDoor, this, &QFortrendStationStatusVTMWidget::onPMOpenTMCavityDoor);
		QObject::connect(d->ui->pm1_widget, &PMGDTWidget::signalPMCloseTMCavityDoor, this, &QFortrendStationStatusVTMWidget::onPMCloseTMCavityDoor);
		QObject::connect(d->ui->pm1_widget, &PMGDTWidget::signalPMGetFinished, this, &QFortrendStationStatusVTMWidget::onPMGetFinished);
		QObject::connect(d->ui->pm1_widget, &PMGDTWidget::signalPMUplaodFinished, this, &QFortrendStationStatusVTMWidget::onPMUplaodFinished);
		QObject::connect(d->ui->pm1_widget, &PMGDTWidget::signalPMGetStatus, this, &QFortrendStationStatusVTMWidget::onPMGetStatus);
		QObject::connect(d->ui->pm1_widget, &PMGDTWidget::signalPMReset, this, &QFortrendStationStatusVTMWidget::onPMReset);

		QObject::connect(d->ui->pm2_widget, &PMGDTWidget::signalPMOpenTMCavityDoor, this, &QFortrendStationStatusVTMWidget::onPMOpenTMCavityDoor);
		QObject::connect(d->ui->pm2_widget, &PMGDTWidget::signalPMCloseTMCavityDoor, this, &QFortrendStationStatusVTMWidget::onPMCloseTMCavityDoor);
		QObject::connect(d->ui->pm2_widget, &PMGDTWidget::signalPMGetFinished, this, &QFortrendStationStatusVTMWidget::onPMGetFinished);
		QObject::connect(d->ui->pm2_widget, &PMGDTWidget::signalPMUplaodFinished, this, &QFortrendStationStatusVTMWidget::onPMUplaodFinished);
		QObject::connect(d->ui->pm2_widget, &PMGDTWidget::signalPMGetStatus, this, &QFortrendStationStatusVTMWidget::onPMGetStatus);
		QObject::connect(d->ui->pm2_widget, &PMGDTWidget::signalPMReset, this, &QFortrendStationStatusVTMWidget::onPMReset);

		QObject::connect(d->ui->pm3_widget, &PMGDTWidget::signalPMOpenTMCavityDoor, this, &QFortrendStationStatusVTMWidget::onPMOpenTMCavityDoor);
		QObject::connect(d->ui->pm3_widget, &PMGDTWidget::signalPMCloseTMCavityDoor, this, &QFortrendStationStatusVTMWidget::onPMCloseTMCavityDoor);
		QObject::connect(d->ui->pm3_widget, &PMGDTWidget::signalPMGetFinished, this, &QFortrendStationStatusVTMWidget::onPMGetFinished);
		QObject::connect(d->ui->pm3_widget, &PMGDTWidget::signalPMUplaodFinished, this, &QFortrendStationStatusVTMWidget::onPMUplaodFinished);
		QObject::connect(d->ui->pm3_widget, &PMGDTWidget::signalPMGetStatus, this, &QFortrendStationStatusVTMWidget::onPMGetStatus);
		QObject::connect(d->ui->pm3_widget, &PMGDTWidget::signalPMReset, this, &QFortrendStationStatusVTMWidget::onPMReset);


		QObject::connect(d->ui->pm4_widget, &PMGDTWidget::signalPMOpenTMCavityDoor, this, &QFortrendStationStatusVTMWidget::onPMOpenTMCavityDoor);
		QObject::connect(d->ui->pm4_widget, &PMGDTWidget::signalPMCloseTMCavityDoor, this, &QFortrendStationStatusVTMWidget::onPMCloseTMCavityDoor);
		QObject::connect(d->ui->pm4_widget, &PMGDTWidget::signalPMGetFinished, this, &QFortrendStationStatusVTMWidget::onPMGetFinished);
		QObject::connect(d->ui->pm4_widget, &PMGDTWidget::signalPMUplaodFinished, this, &QFortrendStationStatusVTMWidget::onPMUplaodFinished);
		QObject::connect(d->ui->pm4_widget, &PMGDTWidget::signalPMGetStatus, this, &QFortrendStationStatusVTMWidget::onPMGetStatus);
		QObject::connect(d->ui->pm4_widget, &PMGDTWidget::signalPMReset, this, &QFortrendStationStatusVTMWidget::onPMReset);

#pragma endregion

	}

	QFortrendStationStatusVTMWidget::~QFortrendStationStatusVTMWidget(){
		Q_D(QFortrendStationStatusVTMWidget);
		delete d_ptr;
		
	}

	void QFortrendStationStatusVTMWidget::paintEvent(QPaintEvent *event){
	
	}

	void QFortrendStationStatusVTMWidget::pauseAnimation(){
		Q_D(QFortrendStationStatusVTMWidget);
		d->ui->robot_widget->pauseAnimation();
	}

	void QFortrendStationStatusVTMWidget::Animation(int station, int arm, QString action){
		Q_D(QFortrendStationStatusVTMWidget);
		int rotationangle = 40;
		int armangle = station == d->stationidaligner ? 20 : 70;
		int currentRotationAngle = d->ui->robot_widget->getCurrentRotationAngle();

		if (station == d->stationidlk1 && arm == 0){
			if (currentRotationAngle <= -100 && currentRotationAngle >= -297){
				rotationangle == rotationangle ? rotationangle = -293 : rotationangle = -293 - currentRotationAngle;
			}
			else{
				rotationangle == rotationangle ? rotationangle = 67 : rotationangle = 67 - currentRotationAngle;
			}
		}
		else if (station == d->stationidaligner && arm == 0){
			if (currentRotationAngle <= -100 && currentRotationAngle >= -297){
				rotationangle == rotationangle ? rotationangle = -297 : rotationangle = -297 - currentRotationAngle;
			}
			else{
				rotationangle == rotationangle ? rotationangle = 63 : rotationangle = 63 - currentRotationAngle;
			}
		}
		else if (station == d->stationidpm1 && arm == 0){
			if (currentRotationAngle <= -180 && currentRotationAngle > -360){
				rotationangle == rotationangle ? rotationangle = -360 : rotationangle = -360 - currentRotationAngle;
			}
			else{
				rotationangle == rotationangle ? rotationangle = 0 : rotationangle = 0 - currentRotationAngle;
			}
		}
		else if (station == d->stationidpm2 && arm == 0){
			armangle = 30;
			if (currentRotationAngle >= 114 && currentRotationAngle <= -246){
				rotationangle == rotationangle ? rotationangle = 270 : rotationangle = 270 - currentRotationAngle;
			}
			/*else if (currentRotationAngle == -297){
				rotationangle = -450;
			}*/
			else{
				rotationangle == rotationangle ? rotationangle = -90 : rotationangle = -90 - currentRotationAngle;
			}
		}
		else if (station == d->stationidpm3 && arm == 0){
			if (currentRotationAngle >= -180 && currentRotationAngle <= 0){
				rotationangle == rotationangle ? rotationangle = -180 : rotationangle = -180 - currentRotationAngle;
			}
			else{//其他情况逆时针旋转到PM3
				rotationangle == rotationangle ? rotationangle = 180 : rotationangle = 180 - currentRotationAngle;
			}
		}
		else if (station == d->stationidlk2 && arm == 0){
			if (currentRotationAngle >= -246 && currentRotationAngle <= -90){
				rotationangle == rotationangle ? rotationangle = -246 : rotationangle = -246 - currentRotationAngle;
			}
			else{
				rotationangle == rotationangle ? rotationangle = 114 : rotationangle = 114 - currentRotationAngle;
			}
		}
		else if (station == d->stationidlk1 && arm == 1){
			if (currentRotationAngle >= 90 && currentRotationAngle <= 246){
				rotationangle == rotationangle ? rotationangle = 246 : rotationangle = 246 - currentRotationAngle;
			}
			else{
				rotationangle == rotationangle ? rotationangle = -114 : rotationangle = -114 - currentRotationAngle;
			}
		}
		else if (station == d->stationidaligner && arm == 1){
			if (currentRotationAngle >= 90 && currentRotationAngle <= 242){
				rotationangle == rotationangle ? rotationangle = 242 : rotationangle = 242 - currentRotationAngle;
			}
			else{
				rotationangle == rotationangle ? rotationangle = -118 : rotationangle = -118 - currentRotationAngle;
			}
		}
		else if (station == d->stationidpm1 && arm == 1){
			if (currentRotationAngle >= -180 && currentRotationAngle <= 0){
				rotationangle == rotationangle ? rotationangle = -180 : rotationangle = -180 - currentRotationAngle;
			}
			else{//其他情况逆时针旋转到PM3
				rotationangle == rotationangle ? rotationangle = 180 : rotationangle = 180 - currentRotationAngle;
			}

		}
		else if (station == d->stationidpm2 && arm == 1){
			armangle = 30;
			if (currentRotationAngle >= -270 && currentRotationAngle <= -114){
				rotationangle == rotationangle ? rotationangle = -270 : rotationangle = -270 - currentRotationAngle;
			}
			else{
				rotationangle == rotationangle ? rotationangle = 90 : rotationangle = 90 - currentRotationAngle;
			}
		}
		else if (station == d->stationidpm3 && arm == 1){
			if (currentRotationAngle >= -360 && currentRotationAngle <= -180){
				rotationangle == rotationangle ? rotationangle = -360 : rotationangle = -360 - currentRotationAngle;
			}
			else{
				rotationangle == rotationangle ? rotationangle = 0 : rotationangle = 0 - currentRotationAngle;
			}
		}
		else if (station == d->stationidlk2 && arm == 1){
			if (currentRotationAngle >= 120 && currentRotationAngle <= 292){
				rotationangle == rotationangle ? rotationangle = 292 : rotationangle = 292 - currentRotationAngle;
			}
			else{
				rotationangle == rotationangle ? rotationangle = -68 : rotationangle = -68 - currentRotationAngle;
			}
		}

		if (action == "get"){
			//printf("Get currentRotationAngle:%d  rotationangle: %d \n", currentRotationAngle, rotationangle);
			//logInform(d->wtr->getName().c_str(), "Get currentRotationAngle:%d  rotationangle: %d station=%d stationidlk1=%d", currentRotationAngle, rotationangle, station, d->stationidlk1);
			d->ui->robot_widget->get(arm, rotationangle, armangle);
		}
		else{
			//printf("Put currentRotationAngle:%d  rotationangle: %d \n", currentRotationAngle, rotationangle);
			//logInform(d->wtr->getName().c_str(), "Put currentRotationAngle:%d  rotationangle: %d station=%d stationidlk1=%d", currentRotationAngle, rotationangle, station, d->stationidlk1);
			d->ui->robot_widget->put(arm, rotationangle, armangle);
		}

	}

#pragma region 机械手模块指令
	void QFortrendStationStatusVTMWidget::onRobotDialog(){
		Q_D(QFortrendStationStatusVTMWidget);
		//d->robotdialog->exec();
		d->robotdialog->show();//打开可操作其他控件
	};
	void QFortrendStationStatusVTMWidget::onRobotReset(){
		Q_D(QFortrendStationStatusVTMWidget);
		//d->ui->robot_widget->reset();
		KernelSubsystemCommand::Ptr cmd = d->wtr->createResetCommand();
		d->wtrwidget->executeCommand(d->wtr, cmd);
	};

	void QFortrendStationStatusVTMWidget::onRobotSpeed(int speed){
		Q_D(QFortrendStationStatusVTMWidget);
		//d->ui->robot_widget->reset();
		KernelSubsystemCommand::Ptr cmd = d->wtr->createSetSpeedCommand(speed);
		d->wtrwidget->executeCommand(d->wtr, cmd);
	};

	void QFortrendStationStatusVTMWidget::onRobotZSpeed(int speed){
		Q_D(QFortrendStationStatusVTMWidget);
		//d->ui->robot_widget->reset();
		KernelSubsystemCommand::Ptr cmd = d->wtr->createSetAxisZSpeedCommand(speed);
		d->wtrwidget->executeCommand(d->wtr, cmd);
	};

	void QFortrendStationStatusVTMWidget::onRobotGetStatus(){
		Q_D(QFortrendStationStatusVTMWidget);
		KernelSubsystemCommand::Ptr cmd = d->wtr->createUpdateCommand();
		d->wtrwidget->executeCommand(d->wtr, cmd);
	};
	void QFortrendStationStatusVTMWidget::onRobotClearError(){
		Q_D(QFortrendStationStatusVTMWidget);
		KernelSubsystemCommand::Ptr cmd = d->wtr->createClearErrorCommand();
		d->wtrwidget->executeCommand(d->wtr, cmd);
	};
	void QFortrendStationStatusVTMWidget::onRobotGet(int station,int arm, int slot){
		Q_D(QFortrendStationStatusVTMWidget);
		if (station == -1){
			//QMessageBox::information(d->robotdialog, "warn", "请选择取料工位");
			logWarn(d->wtr->getName().c_str(), "请选择取料工位!");
			return;
		} 
		if (arm == -1){
			//QMessageBox::information(d->robotdialog, "warn", "请选择取料手爪");
			logWarn(d->wtr->getName().c_str(), "请选择取料手爪!");
			return;
		}

		auto robotcass = d->cassManager->getCassette(d->wtr.get());
		bool armwafer = robotcass->getMapping(arm+1) == Cassette::Present;
		if (armwafer){
			QString armstr = arm == 0 ? "A" : "B";
			//QMessageBox::information(d->robotdialog, "warn", QString("手爪%1已有料，禁止取料，请重新检查!").arg(armstr));
			logWarn(d->wtr->getName().c_str(), "工位%s已有料，禁止取料，请重新检查!", armstr);
			return;
		}
		std::shared_ptr<FortrendStation> stationptr  = d->wtr->getWorkStations().at(station);
		auto stationcass = d->cassManager->getCassette(stationptr.get());
		bool stationIsWafer = false;
		
		if (station == 0){
			stationIsWafer = stationcass->getMapping(d->ui->loadlockA_widget->GetCurrentSlot()) == Cassette::Present;
			station = d->stationidlk1;
		}
		else if (station == 6){
			stationIsWafer = stationcass->getMapping(d->ui->loadlockB_widget->GetCurrentSlot()) == Cassette::Present;
			station = d->stationidlk2;
		}
		else{
			stationIsWafer = stationcass->getMapping(1) == Cassette::Present;
			//station == 1 ? station = d->stationidpm2 : station = d->stationidaligner;
		}
		if (!stationIsWafer){
			//QMessageBox::information(d->robotdialog, "warn", QString("工位%1没有料，禁止取料，请重新检查!").arg(stationptr->getName().c_str()));
			logWarn(d->wtr->getName().c_str(), "工位%s没有料，禁止取料，请重新检查!", stationptr->getName());
			return;
		}
		d->ui->robot_widget->setCurrentStation(station);
		d->ui->robot_widget->setCurrentArm(arm);
		KernelSubsystemCommand::Ptr cmd = d->wtr->createGetCommand(stationptr, arm, slot);
		d->wtrwidget->executeCommand(d->wtr, cmd);
		//Animation(station,arm,"get");
		
	};
	void QFortrendStationStatusVTMWidget::onRobotPut(int station, int arm, int slot){
		Q_D(QFortrendStationStatusVTMWidget);
		if (station == -1){
			//QMessageBox::information(d->robotdialog, "warn", "请选择放料工位!");
			logWarn(d->wtr->getName().c_str(), "请选择放料工位!");
			return;
		}
		if (arm == -1){
			//QMessageBox::information(d->robotdialog, "warn", "请选择放料手爪!");
			logWarn(d->wtr->getName().c_str(), "请选择放料手爪!");
			return;
		}

		auto robotcass = d->cassManager->getCassette(d->wtr.get());
		bool armwafer = robotcass->getMapping(arm+1) == Cassette::Empty;
		if (armwafer){
			QString armstr = arm == 0 ? "A" : "B";
			//QMessageBox::information(d->robotdialog, "warn", QString("手爪%1没有料，禁止放料，请重新检查!").arg(armstr));
			logWarn(d->wtr->getName().c_str(), "工位%s没有料，禁止放料，请重新检查!", armstr);
			return;
		}

		std::shared_ptr<FortrendStation> stationptr = d->wtr->getWorkStations().at(station);
		auto stationcass = d->cassManager->getCassette(stationptr.get());
		bool stationIsWafer = false;
		if (station == 0){
			stationIsWafer = stationcass->getMapping(d->ui->loadlockA_widget->GetCurrentSlot()) == Cassette::Present;
			station = d->stationidlk1;
		}
		else if (station ==2){
			stationIsWafer = stationcass->getMapping(d->ui->loadlockB_widget->GetCurrentSlot()) == Cassette::Present;
			station = d->stationidlk2;
		}
		else{
			stationIsWafer = stationcass->getMapping(1) == Cassette::Present;//Mapping从1开始算
			//station == 1 ? station = d->stationidpm2 : station = d->stationidaligner;
		}
		if (stationIsWafer){
			//QMessageBox::information(d->robotdialog, "warn", QString("工位%1有料，禁止放料，请重新检查!").arg(stationptr->getName().c_str()));
			logWarn(d->wtr->getName().c_str(), "工位%s有料，禁止放料，请重新检查!", stationptr->getName());
			return;
		}

		d->ui->robot_widget->setCurrentStation(station);
		d->ui->robot_widget->setCurrentArm(arm);
		KernelSubsystemCommand::Ptr cmd = d->wtr->createPutCommand(stationptr, arm, slot);
		d->wtrwidget->executeCommand(d->wtr, cmd);
		//d->Animation(station, arm,"put");
		
	};
	void QFortrendStationStatusVTMWidget::onRobotGetFinished(){
		Q_D(QFortrendStationStatusVTMWidget);
		//int arm = d->ui->robot_widget->getCurrentArm();
		//auto robotcass = d->cassManager->getCassette(d->wtr.get());
		//robotcass->setMapping(arm + 1, Cassette::Present);

		int station = d->ui->robot_widget->getCurrentStation();
		if (station == d->stationidlk1){
		/*int slot = d->ui->loadlockA_widget->GetCurrentSlot();
			auto cassA = d->cassManager->getCassette(d->lk1.get());
			cassA->setMapping(slot, Cassette::Empty);*/
		}
		else if (station == d->stationidpm1){
			d->ui->pm1_widget->setWafer(false);
			auto pm1cass = d->cassManager->getCassette(d->pm1.get());
			pm1cass->setMapping(1, Cassette::Empty);
		}
		else if (station == d->stationidpm2){
			d->ui->pm2_widget->setWafer(false);
			auto pm2cass = d->cassManager->getCassette(d->pm2.get());
			pm2cass->setMapping(1, Cassette::Empty);
		}
		else if (station == d->stationidpm3){
			d->ui->pm3_widget->setWafer(false);
			auto pm3cass = d->cassManager->getCassette(d->pm3.get());
			pm3cass->setMapping(1, Cassette::Empty);
		}
		else if (station == d->stationidpm4)
		{
			d->ui->pm4_widget->setWafer(false);
			auto pm4cass = d->cassManager->getCassette(d->pm4.get());
			pm4cass->setMapping(1, Cassette::Empty);
		}
		else if (station == d->stationidlk2){
		/*int slot = d->ui->loadlockB_widget->GetCurrentSlot();
			auto cassB = d->cassManager->getCassette(d->lk2.get());
			cassB->setMapping(slot, Cassette::Empty);*/
		}

	}
	void QFortrendStationStatusVTMWidget::onRobotPutFinished(){
		Q_D(QFortrendStationStatusVTMWidget);
		/*int arm = d->ui->robot_widget->getCurrentArm();
		auto robotcass = d->cassManager->getCassette(d->wtr.get());
		robotcass->setMapping(arm + 1, Cassette::Empty);*/
		int station = d->ui->robot_widget->getCurrentStation();
		if (station == d->stationidlk1){
			/*int slot = d->ui->loadlockA_widget->GetCurrentSlot();
			auto cassA = d->cassManager->getCassette(d->lk1.get());
			cassA->setMapping(slot, Cassette::Present);*/
		}
		else if (station == d->stationidpm1){
			d->ui->pm1_widget->setWafer(true);
			auto pm1cass = d->cassManager->getCassette(d->pm1.get());
			pm1cass->setMapping(1, Cassette::Present);
		}
		else if (station == d->stationidpm2){
			d->ui->pm2_widget->setWafer(true);
			auto pm2cass = d->cassManager->getCassette(d->pm2.get());
			pm2cass->setMapping(1, Cassette::Present);
		}
		else if (station == d->stationidpm3){
			d->ui->pm3_widget->setWafer(true);
			auto pm3cass = d->cassManager->getCassette(d->pm3.get());
			pm3cass->setMapping(1, Cassette::Present);
		}
		else if (station == d->stationidpm4)
		{
			d->ui->pm4_widget->setWafer(true);
			auto pm4cass = d->cassManager->getCassette(d->pm4.get());
			pm4cass->setMapping(1, Cassette::Present);
		}
		else if (station == d->stationidlk2){
			/*int slot = d->ui->loadlockB_widget->GetCurrentSlot();
			auto cassB = d->cassManager->getCassette(d->lk2.get());
			cassB->setMapping(slot, Cassette::Present);*/
		}
	}

#pragma endregion

#pragma region LoadLock模块指令
	void QFortrendStationStatusVTMWidget::onLoadLockOpenbox(std::string name){
		Q_D(QFortrendStationStatusVTMWidget);
		auto lk = d->kernel->getKernelModule<FortrendLoadLockSubsystem>(name);
		KernelSubsystemCommand::Ptr cmd = lk->createOpenCassetteDoorCommand();
		d->lk1widget->executeCommand(lk, cmd);
	};
	void QFortrendStationStatusVTMWidget::onLoadLockClosebox(std::string name){
		Q_D(QFortrendStationStatusVTMWidget);
		auto lk = d->kernel->getKernelModule<FortrendLoadLockSubsystem>(name);
		KernelSubsystemCommand::Ptr cmd = lk->createCloseCassetteDoorCommand();
		d->lk1widget->executeCommand(lk, cmd);
	};
	void QFortrendStationStatusVTMWidget::onLoadLockMapping(std::string name){
		Q_D(QFortrendStationStatusVTMWidget);
		auto lk = d->kernel->getKernelModule<FortrendLoadLockSubsystem>(name);
		KernelSubsystemCommand::Ptr cmd = lk->createMappingCommand();
		d->lk1widget->executeCommand(lk, cmd);
	};
	void QFortrendStationStatusVTMWidget::onLoadLockMoveToSlot(std::string name){
			Q_D(QFortrendStationStatusVTMWidget);
			
			if (name == "LLA"){
				int slot = d->ui->loadlockA_widget->GetSelectSlot();
				if (slot == 0){
					//QMessageBox::information(d->robotdialog, "warn", "请在旁边选择槽号!");
					logWarn(name.c_str(), "请在旁边选择槽号!");
					return;
				}
				auto lk = d->kernel->getKernelModule<FortrendLoadLockSubsystem>(name);
				KernelSubsystemCommand::Ptr cmd = lk->createMoveToSlotCommand(slot);
				d->wtrwidget->executeCommand(lk, cmd);
				d->ui->loadlockA_widget->SetCurrentSlot(slot);
			}
			else{
				int slot = d->ui->loadlockB_widget->GetSelectSlot();
				if (slot == 0){
					//QMessageBox::information(d->robotdialog, "warn", "请在旁边选择槽号!");
					logWarn(name.c_str(), "请在旁边选择槽号!");
					return;
				}
				auto lk = d->kernel->getKernelModule<FortrendLoadLockSubsystem>(name);
				KernelSubsystemCommand::Ptr cmd = lk->createMoveToSlotCommand(slot);
				d->wtrwidget->executeCommand(lk, cmd);
				d->ui->loadlockB_widget->SetCurrentSlot(slot);
			}

	};

	void QFortrendStationStatusVTMWidget::onLoadLockMoveToOrigin(std::string name){
		Q_D(QFortrendStationStatusVTMWidget);

		if (name == "LLA"){
			
			auto lk = d->kernel->getKernelModule<FortrendLoadLockSubsystem>(name);
			KernelSubsystemCommand::Ptr cmd = lk->createMoveToSlotCommand(28);
			d->wtrwidget->executeCommand(lk, cmd);
		}
		else{
			auto lk = d->kernel->getKernelModule<FortrendLoadLockSubsystem>(name);
			KernelSubsystemCommand::Ptr cmd = lk->createMoveToSlotCommand(28);
			d->wtrwidget->executeCommand(lk, cmd);
		}

	};
	void QFortrendStationStatusVTMWidget::onLoadLockOpenTMCavityDoor(std::string name){
		Q_D(QFortrendStationStatusVTMWidget);
		auto lk = d->kernel->getKernelModule<FortrendLoadLockSubsystem>(name);
		KernelSubsystemCommand::Ptr cmd = lk->createOpenTMCavityDoorCommand();
		d->wtrwidget->executeCommand(lk, cmd);
	};
	void QFortrendStationStatusVTMWidget::onLoadLockCloseTMCavityDoor(std::string name){
		Q_D(QFortrendStationStatusVTMWidget);
		auto lk = d->kernel->getKernelModule<FortrendLoadLockSubsystem>(name);
		KernelSubsystemCommand::Ptr cmd = lk->createCloseTMCavityDoorCommand();
		d->lk1widget->executeCommand(lk, cmd);
	};
	void QFortrendStationStatusVTMWidget::onLoadLockOpenDiaphragmValve(std::string name){
		Q_D(QFortrendStationStatusVTMWidget);
		auto lk = d->kernel->getKernelModule<FortrendLoadLockSubsystem>(name);
		QMainLoadLockSubsystemWidget* mainLoadlockWidget = d->mainLoadlock1Widget->getName() == name ? d->mainLoadlock1Widget : d->mainLoadlock2Widget;//待优化
		LoadLockValveOpening valve = mainLoadlockWidget->getLoadLockValveOpening() == 1 ? LoadLock_Slow : LoadLock_Fast;
		KernelSubsystemCommand::Ptr cmd = lk->createOpenDiaphragmValveCommand(valve);
		d->lk1widget->executeCommand(lk, cmd);
	};
	void QFortrendStationStatusVTMWidget::onLoadLockCloseDiaphragmValve(std::string name){
		Q_D(QFortrendStationStatusVTMWidget);
		auto lk = d->kernel->getKernelModule<FortrendLoadLockSubsystem>(name);
		QMainLoadLockSubsystemWidget* mainLoadlockWidget = d->mainLoadlock1Widget->getName() == name ? d->mainLoadlock1Widget : d->mainLoadlock2Widget;//待优化
		LoadLockValveOpening valve = mainLoadlockWidget->getLoadLockValveOpening() == 1 ? LoadLock_Slow : LoadLock_Fast;
		KernelSubsystemCommand::Ptr cmd = lk->createCloseDiaphragmValveCommand(valve);
		d->lk1widget->executeCommand(lk, cmd);
	};
	void QFortrendStationStatusVTMWidget::onLoadLockOpenExhaustValve(std::string name){
		Q_D(QFortrendStationStatusVTMWidget);
		auto lk = d->kernel->getKernelModule<FortrendLoadLockSubsystem>(name);
		KernelSubsystemCommand::Ptr cmd = lk->createOpenExhaustValveCommand();
		d->lk1widget->executeCommand(lk, cmd);
	};
	void QFortrendStationStatusVTMWidget::onLoadLockCloseExhaustValve(std::string name){
		Q_D(QFortrendStationStatusVTMWidget);
		auto lk = d->kernel->getKernelModule<FortrendLoadLockSubsystem>(name);
		KernelSubsystemCommand::Ptr cmd = lk->createCloseExhaustValveCommand();
		d->lk1widget->executeCommand(lk, cmd);
	};
	void QFortrendStationStatusVTMWidget::onLoadLockOpenAngleValve(std::string name){
		Q_D(QFortrendStationStatusVTMWidget);
		auto lk = d->kernel->getKernelModule<FortrendLoadLockSubsystem>(name);
		KernelSubsystemCommand::Ptr cmd = lk->createOpenAngleValveCommand();
		d->lk1widget->executeCommand(lk, cmd);
	};
	void QFortrendStationStatusVTMWidget::onLoadLockCloseAngleValve(std::string name){
		Q_D(QFortrendStationStatusVTMWidget);
		auto lk = d->kernel->getKernelModule<FortrendLoadLockSubsystem>(name);
		KernelSubsystemCommand::Ptr cmd = lk->createCloseAngleValveCommand();
		d->lk1widget->executeCommand(lk, cmd);
	};

	void QFortrendStationStatusVTMWidget::onOpenInsertingPlateValve(std::string name){
		Q_D(QFortrendStationStatusVTMWidget);
		auto lk = d->kernel->getKernelModule<FortrendLoadLockSubsystem>(name);
		KernelSubsystemCommand::Ptr cmd = lk->createOpenInsertingPlateValveCommand();
		d->lk1widget->executeCommand(lk, cmd);
	};
	void QFortrendStationStatusVTMWidget::onCloseInsertingPlateValve(std::string name){
		Q_D(QFortrendStationStatusVTMWidget);
		auto lk = d->kernel->getKernelModule<FortrendLoadLockSubsystem>(name);
		KernelSubsystemCommand::Ptr cmd = lk->createCloseInsertingPlateValveCommand();
		d->lk1widget->executeCommand(lk, cmd);
	};
	void QFortrendStationStatusVTMWidget::onOpenHeightVacuumBaffleValve(std::string name){
		Q_D(QFortrendStationStatusVTMWidget);
		auto lk = d->kernel->getKernelModule<FortrendLoadLockSubsystem>(name);
		KernelSubsystemCommand::Ptr cmd = lk->createOpenHeightVacuumBaffleValveCommand();
		d->lk1widget->executeCommand(lk, cmd);
	};
	void QFortrendStationStatusVTMWidget::onCloseHeightVacuumBaffleValve(std::string name){
		Q_D(QFortrendStationStatusVTMWidget);
		auto lk = d->kernel->getKernelModule<FortrendLoadLockSubsystem>(name);
		KernelSubsystemCommand::Ptr cmd = lk->createCloseHeightVacuumBaffleValveCommand();
		d->lk1widget->executeCommand(lk, cmd);
	};

	void QFortrendStationStatusVTMWidget::onLoadLockGetStatus(std::string name){
		Q_D(QFortrendStationStatusVTMWidget);
		auto lk = d->kernel->getKernelModule<FortrendLoadLockSubsystem>(name);
		KernelSubsystemCommand::Ptr cmd = lk->createUpdateCommand();
		d->lk1widget->executeCommand(lk, cmd);
	};
	void QFortrendStationStatusVTMWidget::onLoadLockReset(std::string name){
		Q_D(QFortrendStationStatusVTMWidget);
		auto lk = d->kernel->getKernelModule<FortrendLoadLockSubsystem>(name);
		KernelSubsystemCommand::Ptr cmd = lk->createResetCommand();
		d->lk1widget->executeCommand(lk, cmd);
	};
	void QFortrendStationStatusVTMWidget::onLoadLockClearError(std::string name){
		Q_D(QFortrendStationStatusVTMWidget);
		auto lk = d->kernel->getKernelModule<FortrendLoadLockSubsystem>(name);
		KernelSubsystemCommand::Ptr cmd = lk->createClearErrorCommand();
		d->lk1widget->executeCommand(lk, cmd);
	};
	void QFortrendStationStatusVTMWidget::_onSelectASlot(int slot){
		Q_D(QFortrendStationStatusVTMWidget);
		if (d->slot_widgetA->isSelected(slot)){
			d->ui->loadlockA_widget->SetSelectSlot(slot);
			printf("SelectSolt %d \r\n", slot);
		}
		else{
			d->ui->loadlockA_widget->SetSelectSlot(0);
			printf("SelectSolt %d \r\n", 0);
		}
	}
	void QFortrendStationStatusVTMWidget::_onSelectBSlot(int slot){
		Q_D(QFortrendStationStatusVTMWidget);
		if (d->slot_widgetB->isSelected(slot)){
			d->ui->loadlockB_widget->SetSelectSlot(slot);
			printf("SelectSoltB %d \r\n", slot);
		}
		else{
			d->ui->loadlockB_widget->SetSelectSlot(0);
			printf("SelectSoltB %d \r\n", 0);
		}
	}
#pragma endregion

#pragma region TM模块指令
	void QFortrendStationStatusVTMWidget::onTMOpenDiaphragmValve(std::string name){
		Q_D(QFortrendStationStatusVTMWidget);
		KernelSubsystemCommand::Ptr cmd = d->tm->createOpenDiaphragmValveCommand(TMCavityValveOpening::TMCavity_Both);
		d->tmwidget->executeCommand(d->tm, cmd);
	};
	void QFortrendStationStatusVTMWidget::onTMCloseDiaphragmValve(std::string name){
		Q_D(QFortrendStationStatusVTMWidget);
		KernelSubsystemCommand::Ptr cmd = d->tm->createCloseDiaphragmValveCommand(TMCavityValveOpening::TMCavity_Both);
		d->tmwidget->executeCommand(d->tm, cmd);
	};
	void QFortrendStationStatusVTMWidget::onTMOpenHeightVacuumBaffleValve(std::string name){
		Q_D(QFortrendStationStatusVTMWidget);
		KernelSubsystemCommand::Ptr cmd = d->tm->createOpenHeightVacuumBaffleValveCommand();
		d->tmwidget->executeCommand(d->tm, cmd);
	};
	void QFortrendStationStatusVTMWidget::onTMCloseHeightVacuumBaffleValve(std::string name){
		Q_D(QFortrendStationStatusVTMWidget);
		KernelSubsystemCommand::Ptr cmd = d->tm->createCloseHeightVacuumBaffleValveCommand();
		d->tmwidget->executeCommand(d->tm, cmd);
	};
	void QFortrendStationStatusVTMWidget::onTMOpenAngleValve(std::string name){
		Q_D(QFortrendStationStatusVTMWidget);
		KernelSubsystemCommand::Ptr cmd = d->tm->createOpenAngleValveCommand();
		d->tmwidget->executeCommand(d->tm, cmd);
	};
	void QFortrendStationStatusVTMWidget::onTMCloseAngleValve(std::string name){
		Q_D(QFortrendStationStatusVTMWidget);
		KernelSubsystemCommand::Ptr cmd = d->tm->createCloseAngleValveCommand();
		d->tmwidget->executeCommand(d->tm, cmd);
	};
	void QFortrendStationStatusVTMWidget::onTMOpenInsertingPlateValve(std::string name){
		Q_D(QFortrendStationStatusVTMWidget);
		KernelSubsystemCommand::Ptr cmd = d->tm->createOpenInsertingPlateValveCommand();
		d->tmwidget->executeCommand(d->tm, cmd);
	};
	void QFortrendStationStatusVTMWidget::onTMCloseInsertingPlateValve(std::string name){
		Q_D(QFortrendStationStatusVTMWidget);
		KernelSubsystemCommand::Ptr cmd = d->tm->createCloseInsertingPlateValveCommand();
		d->tmwidget->executeCommand(d->tm, cmd);
	};
	void QFortrendStationStatusVTMWidget::onTMOpenFlowmeterDiaphragmValve(std::string name){
		Q_D(QFortrendStationStatusVTMWidget);
		KernelSubsystemCommand::Ptr cmd = d->tm->createOpenFlowmeterDiaphragmValveCommand();
		d->tmwidget->executeCommand(d->tm, cmd);
	};
	void QFortrendStationStatusVTMWidget::onTMCloseFlowmeterDiaphragmValve(std::string name){
		Q_D(QFortrendStationStatusVTMWidget);
		KernelSubsystemCommand::Ptr cmd = d->tm->createCloseFlowmeterDiaphragmValveCommand();
		d->tmwidget->executeCommand(d->tm, cmd);
	};
	void QFortrendStationStatusVTMWidget::onTMReset(std::string name){
		Q_D(QFortrendStationStatusVTMWidget);
		KernelSubsystemCommand::Ptr cmd = d->tm->createResetCommand();
		d->tmwidget->executeCommand(d->tm, cmd);
	};
	void QFortrendStationStatusVTMWidget::onTMGetStatus(std::string name){};

	void QFortrendStationStatusVTMWidget::onAlignerReset(std::string name){
		Q_D(QFortrendStationStatusVTMWidget);
		KernelSubsystemCommand::Ptr cmd = d->aligner->createResetCommand();
		d->tmwidget->executeCommand(d->aligner, cmd);
	}
	void QFortrendStationStatusVTMWidget::onAlignerAlign(std::string name){
		Q_D(QFortrendStationStatusVTMWidget);
		KernelSubsystemCommand::Ptr cmd = d->aligner->createAlignCommand();
		d->tmwidget->executeCommand(d->aligner, cmd);
	}

#pragma endregion

#pragma region PM模块指令
	void QFortrendStationStatusVTMWidget::onPMOpenTMCavityDoor(std::string name){
		Q_D(QFortrendStationStatusVTMWidget);
		//KernelSubsystemCommand::Ptr cmd = d->pm2->createOpenTMCavityDoorCommand();
		//d->pm2widget->executeCommand(d->pm2, cmd);
	};
	void QFortrendStationStatusVTMWidget::onPMCloseTMCavityDoor(std::string name){
		Q_D(QFortrendStationStatusVTMWidget);
		/*KernelSubsystemCommand::Ptr cmd = d->pm2->createCloseTMCavityDoorCommand();
		d->pm2widget->executeCommand(d->pm2, cmd);*/
	};
	void QFortrendStationStatusVTMWidget::onPMGetFinished(std::string name){//移到取料位
		Q_D(QFortrendStationStatusVTMWidget);
		KernelSubsystemCommand::Ptr cmd = d->pm2->createToGetStationCommand();
		d->pm2widget->executeCommand(d->pm2, cmd);
		/*KernelSubsystemCommand::Ptr cmd = d->pm2->createGetFinishedCommand();
		d->pm2widget->executeCommand(d->pm2, cmd);*/
	};
	void QFortrendStationStatusVTMWidget::onPMUplaodFinished(std::string name){//移到放料位
		Q_D(QFortrendStationStatusVTMWidget);
		KernelSubsystemCommand::Ptr cmd = d->pm2->createToPutStationCommand();
		d->pm2widget->executeCommand(d->pm2, cmd);
		/*KernelSubsystemCommand::Ptr cmd = d->pm2->createUploadFinishedCommand();
		d->pm2widget->executeCommand(d->pm2, cmd);*/
	};
	void QFortrendStationStatusVTMWidget::onPMGetStatus(std::string name){
		Q_D(QFortrendStationStatusVTMWidget);
		/*KernelSubsystemCommand::Ptr cmd = d->pm2->createUpdateCommand();
		d->pm2widget->executeCommand(d->pm2, cmd);*/
		KernelSubsystemCommand::Ptr cmd = d->pm2->createToPutStationCommand(2);
		d->pm2widget->executeCommand(d->pm2, cmd);
	};
	void QFortrendStationStatusVTMWidget::onPMReset(std::string name){
		Q_D(QFortrendStationStatusVTMWidget);
		KernelSubsystemCommand::Ptr cmd = d->pm2->createResetCommand();
		d->pm2widget->executeCommand(d->pm2, cmd);
	};
#pragma endregion

	void QFortrendStationStatusVTMWidget::process(){
		Q_D(QFortrendStationStatusVTMWidget);
		d->onProcess();
	}

	void QFortrendStationStatusVTMWidget::initTipsUI(){
		Q_D(QFortrendStationStatusVTMWidget);
		//d->ui->gmfm_lla_progress->setTextColor(QColor(250, 250, 250));
		//d->ui->gmfm_lla_progress->setBarBgColor(QColor(30, 30, 30));
		//d->ui->gmfm_lla_progress->setRange(0, 100);

		//d->ui->gmfm_llb_progress->setBarColor(QColor(255, 107, 107));
		//d->ui->gmfm_llb_progress->setPrecision(1);
		//d->ui->gmfm_llb_progress->setRange(0, 100);

		//d->ui->gmfm_tm_progress->setTextColor(QColor(250, 250, 250));
		//d->ui->gmfm_tm_progress->setBarBgColor(QColor(80, 80, 80));
		//d->ui->gmfm_tm_progress->setBarColor(QColor(24, 189, 155));
		//d->ui->gmfm_tm_progress->setRange(0, 100);
	}

	void QFortrendStationStatusVTMWidget::updateState(){
		Q_D(QFortrendStationStatusVTMWidget);

	}

	void QFortrendStationStatusVTMWidget::updateCassetteAnimation(const std::string &name, bool isBoxOpened, const std::vector<Cassette::Mapping> &mapping) {
		Q_D(QFortrendStationStatusVTMWidget);
		if (name == d->wtr->getName())
		{
			if (mapping.size()>=1 && mapping[0] == Cassette::Present){
			logInform(d->wtr->getName().c_str(),"updateCassetteAnimation wtr 1 Present");
			d_ptr->ui->robot_widget->setIsWaferArm1(true);
			d->wtr->setObject(1,true);
			}
			else if (mapping.size()>=1 && mapping[0] == Cassette::Empty){
			logInform(d->wtr->getName().c_str(), "updateCassetteAnimation wtr 1 Empty");
			d_ptr->ui->robot_widget->setIsWaferArm1(false);
			d->wtr->setObject(1, false);
			}

			if (mapping.size()>=2 && mapping[1] == Cassette::Present){
			logInform(d->wtr->getName().c_str(), "updateCassetteAnimation wtr 2 Present");
			d_ptr->ui->robot_widget->setIsWaferArm2(true);
			d->wtr->setObject(2, true);
			}
			else if (mapping.size()>=2 && mapping[1] == Cassette::Empty){
			logInform(d->wtr->getName().c_str(), "updateCassetteAnimation wtr 2 Empty");
			d_ptr->ui->robot_widget->setIsWaferArm2(false);
			d->wtr->setObject(2, false);
			}
			
		}
		else if (name == d->pm2->getName()){
			if (mapping.empty())return;

			if ( mapping[0] == Cassette::Present){
			     d->ui->pm2_widget->setWafer(true);
			}
			else{
			     d->ui->pm2_widget->setWafer(false);
			}
		}
		else if (name == d->pm1->getName()) {
			if (mapping.empty())return;

			if (mapping[0] == Cassette::Present) {
				d->ui->pm1_widget->setWafer(true);
			}
			else {
				d->ui->pm1_widget->setWafer(false);
			}
		}
		else if (name == d->pm3->getName()) {
			if (mapping.empty())return;

			if (mapping[0] == Cassette::Present) {
				d->ui->pm3_widget->setWafer(true);
			}
			else {
				d->ui->pm3_widget->setWafer(false);
			}
		}
		else if (name == d->pm4->getName()) {
			if (mapping.empty())return;

			if (mapping[0] == Cassette::Present) {
				d->ui->pm4_widget->setWafer(true);
			}
			else {
				d->ui->pm4_widget->setWafer(false);
			}
		}

	}

	void QFortrendStationStatusVTMWidget::alignerAnimation(){
		Q_D(QFortrendStationStatusVTMWidget);
		d->ui->tm_widget->startRotationAnimation(3);
	}

	void QFortrendStationStatusVTMWidget::pmAnimation(int station,int speed){
		Q_D(QFortrendStationStatusVTMWidget);
		d->ui->pm2_widget->animateToYOffset(station,speed);
	}
	void QFortrendStationStatusVTMWidget::onRecipe(){
		Q_D(QFortrendStationStatusVTMWidget);
		d->onRecipe();
	}

	void QFortrendStationStatusVTMWidget::onEfemReset()
	{
		Q_D(QFortrendStationStatusVTMWidget);

		d->onEfemReset();
	}

	void QFortrendStationStatusVTMWidget::setAxislocation(double location){
		Q_D(QFortrendStationStatusVTMWidget);
		d->ui->pm2_widget->setYOffset(location);
	}
}
