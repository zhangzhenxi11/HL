// Library: VTM
// Package: VTM

#include "Poco/Format.h"
#include "Kernel/kernel_api.h"
#include "device\ui_control_mode_vtm_widget.h"
#include "control_mode_vtm_widget.h"

#include "VTMSignalTower/fortrend_vtm_signal_tower.h"
#include "Aligner/fortrend_aligner_subsystem.h"
#include "CoolingCavity/fortrend_cooling_cavity_subsystem.h"
#include "LoadLock/fortrend_loadlock_subsystem.h"
#include "PMCavity/fortrend_pm_cavity_subsystem.h"
#include "SunwayRobot/fortrend_sunwayrobot_subsystem.h"
#include "TMCavity/fortrend_tm_cavity_subsystem.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMap>
#include <QLabel>
#include <QCoreApplication>
#include <QDir>
#include <QSettings>
#include <QMessageBox>
#include "ScientificDoubleSpinBox.h"
#include <sstream>
#include <iomanip>

#include <QDateTime>
#include <qdebug.h>

#if _MSC_VER >1600
#pragma execution_character_set("utf-8")
#endif

#define color1 QColor(34, 163, 169)
#define color2 QColor(162, 121, 197)
#define color3 QColor(255, 107, 107)
#define color4 QColor(72, 103, 149)

namespace FC{
	static QMap<int, QString> enable_stat_map = {
		{ 0, "未启用" },
		{ 1, "启用" },
	};

	class QControlModeVTMWidgetPrivate {
	public:
		Q_DECLARE_PUBLIC(QControlModeVTMWidget)
		QControlModeVTMWidgetPrivate(QControlModeVTMWidget* p, const std::shared_ptr<IKernel>& kernel);
		~QControlModeVTMWidgetPrivate();
		void onAttributeChange();
		void setBuzzerEnable(const bool value);
		void setVacuumEnable(const bool value);
		void setWithWaferMode(const bool value);
		void setPMCavity(const int index,const bool value);
		void setLoadLockVacuumParameters(const int index);
		void setTMCavityVacuumParameters();
		void setPMCavityVacuumParameters(const int index);
	public:
		bool buzzer_enable = false;
		bool vacuum_enable = false;
		bool with_wafer_enable = false;
		bool pm1_enable = false;
		bool pm2_enable = false;
		bool pm3_enable = false;
		bool pm4_enable = false;

		double loadlock1_vacuum_upper_limit = 8;			 //LoadLock1真空上限值
		double loadlock1_vacuum_extraction = 5;				 //LoadLock1抽真空设定值
		double loadlock1_vacuum_fast_diaphragm_valve = 5000; //LoadLock1快充隔膜阀设定值
		double loadlock1_vacuum_fast_angle_valve = 80000;	 //LoadLock1快抽角阀设定值
		double loadlock2_vacuum_upper_limit = 8;			 //LoadLock2真空上限值
		double loadlock2_vacuum_extraction = 5;				 //LoadLock2抽真空设定值
		double loadlock2_vacuum_fast_diaphragm_valve = 5000; //LoadLock2快充隔膜阀设定值
		double loadlock2_vacuum_fast_angle_valve = 80000;	 //LoadLock2快抽角阀设定值
		double tm_vacuum_upper_limit = 0.8;					 //TM传输腔真空上限值
		double tm_vacuum_extraction = 0.5;					 //TM传输腔抽真空上限值
		double tm_vaccum_pid = 0.008;						 //TM传输腔PID设定值
		double tm_vacuum_molecular_open_vacuum = 5;			 //分子泵开启设定值
		double pm1_vacuum_setting = 0.08;					 //PM1真空值
		double pm1_vacuum_magnitude = 0.1;					 //PM1与传输腔数量级
		double pm2_vacuum_setting = 0.08;					 //PM2真空值
		double pm2_vacuum_magnitude = 0.1;					 //PM2与传输腔数量级
		double pm3_vacuum_setting = 0.08;					 //PM3真空值
		double pm3_vacuum_magnitude = 0.1;					 //PM3与传输腔数量级
		double rough_vacuum_set_value = 6;					 //TM传输腔抽真空上限值
		double loadlock1_vacuum_delay_time = 20;             //抽真空延迟时间
		double loadlock2_vacuum_delay_time = 20;
		double tm_vacuum_delay_time = 20;
	private:
		double getVacuumMagnitude(const double magnitude);
	private:
		Ui::ControlModeVTMWidget* ui;
		
		std::shared_ptr<IKernel> kernel;
		QControlModeVTMWidget* q_ptr;

		ScientificDoubleSpinBox *loadlock1_vacuum_upper_limit_dsb = 0, *loadlock1_vacuum_extraction_dsb = 0;
		ScientificDoubleSpinBox *loadlock1_vacuum_fast_diaphragm_valve_dsb = 0, *loadlock1_vacuum_fast_angle_valve_dsb = 0;
		ScientificDoubleSpinBox *loadlock2_vacuum_upper_limit_dsb = 0, *loadlock2_vacuum_extraction_dsb = 0;
		ScientificDoubleSpinBox *loadlock2_vacuum_fast_diaphragm_valve_dsb = 0, *loadlock2_vacuum_fast_angle_valve_dsb = 0;
		ScientificDoubleSpinBox *tm_cavity_vacuum_upper_limit_dsb = 0, *tm_cavity_vacuum_extraction_dsb = 0, *tm_cavity_vacuum_pid_dsb = 0;
		ScientificDoubleSpinBox *pm1_cavity_vacuum_upper_limit_dsb = 0, *pm2_cavity_vacuum_upper_limit_dsb = 0, *pm3_cavity_vacuum_upper_limit_dsb = 0;
		ScientificDoubleSpinBox * rough_vacuum_value= 0;
		ScientificDoubleSpinBox* loadlock1_vacuum_delay_time_dsb = 0, *loadlock2_vacuum_delay_time_dsb = 0, *tm_cavity_vacuum_delay_time_dsb = 0;
	};

	QControlModeVTMWidgetPrivate::QControlModeVTMWidgetPrivate(QControlModeVTMWidget* p, const std::shared_ptr<IKernel>& kernel)
		:q_ptr(p)
		, kernel(kernel)
		, ui(new Ui::ControlModeVTMWidget){

	}

	QControlModeVTMWidgetPrivate::~QControlModeVTMWidgetPrivate(){
		delete ui;
	}

	void QControlModeVTMWidgetPrivate::setBuzzerEnable(const bool value){
		buzzer_enable = value;
		std::thread   thread_buzzer([=]() {
			for (auto& tower : kernel->getKernelModules<FortrendVTMSignalTower>()) {
				tower->setEnabled(buzzer_enable);
			}
		});
		thread_buzzer.detach();
	}

	void QControlModeVTMWidgetPrivate::setVacuumEnable(const bool value)
	{
		vacuum_enable = value;
		for (auto& lk : kernel->getKernelModules<FortrendLoadLockSubsystem>()){
			lk->setVacuumEnable(vacuum_enable);
		}
		for (auto& col : kernel->getKernelModules<FortrendCoolingCavitySubsystem>()){
			col->setVacuumEnable(vacuum_enable);
		}
		for (auto& wtr : kernel->getKernelModules<FortrendSunwayRobotSubsystem>()){
			wtr->setVacuumEnable(vacuum_enable);
		}
		for (auto& aligner : kernel->getKernelModules<FortrendAlignerSubsystem>()){
			aligner->setVacuumEnable(vacuum_enable);
		}
		for (auto& pm : kernel->getKernelModules<FortrendPMCavitySubsystem>()){
			pm->setVacuumEnable(vacuum_enable);
		}
		
		for (auto& tm : kernel->getKernelModules<FortrendTMCavitySubsystem>()){
			tm->setVacuumEnable(vacuum_enable);
		}
	}

	void QControlModeVTMWidgetPrivate::setWithWaferMode(const bool value){
		with_wafer_enable = value;
		for (auto& lk : kernel->getKernelModules<FortrendLoadLockSubsystem>()){
			lk->setWithWaferModeEnable(with_wafer_enable);
		}
		for (auto& wtr : kernel->getKernelModules<FortrendSunwayRobotSubsystem>()){
			wtr->setWithWaferModeEnable(with_wafer_enable);
		}
		for (auto& aligner : kernel->getKernelModules<FortrendAlignerSubsystem>()){
			aligner->setWithWaferModeEnable(with_wafer_enable);
		}
		for (auto& pm : kernel->getKernelModules<FortrendPMCavitySubsystem>()){
			pm->setWithWaferModeEnable(with_wafer_enable);
		}
		for (auto& col : kernel->getKernelModules<FortrendCoolingCavitySubsystem>()){
			col->setWithWaferModeEnable(with_wafer_enable);
		}
		for (auto& tm : kernel->getKernelModules<FortrendTMCavitySubsystem>()){
			tm->setWithWaferModeEnable(with_wafer_enable);
		}
	}

	void  QControlModeVTMWidgetPrivate::setPMCavity(const int index,const bool value){
		std::string pm_name = "PM" + std::to_string(index);

		auto pm = kernel->getKernelModule<FortrendPMCavitySubsystem>(pm_name);
		if (!pm)
		{
			//QMessageBox::information(this, "警告", "子系统未找到");
			logInform("ControlMode","子系统未找到.");
			return;
		}
		pm->setPMCavityEnable(value);

		if (index == 1)
		{
			pm1_enable = value;
		}
		else if (index == 2)
		{
			pm2_enable = value;
		}
		else if (index == 3)
		{
			pm3_enable = value;
		}
		else{
			pm4_enable = value;
		}
	}

	void QControlModeVTMWidgetPrivate::setLoadLockVacuumParameters(const int index){
		std::string ll_name = "LLB";
		if (index == 1)ll_name = "LLA";
		auto lk = kernel->getKernelModule<FortrendLoadLockSubsystem>(ll_name);
		if (!lk)
		{
			logError("SaveConfig", "子系统未找到.");
			return;
		}
		double uppper = 0.0;
		double extraction = 0.0;
		double fast_diaphragm_valve = 0.0;
		double fast_angle_valve = 0.0;
		double delay_time = 0.0;
		if (index == 1)
		{
			loadlock1_vacuum_upper_limit = loadlock1_vacuum_upper_limit_dsb->value();
			loadlock1_vacuum_extraction = loadlock1_vacuum_extraction_dsb->value();
			loadlock1_vacuum_fast_diaphragm_valve = loadlock1_vacuum_fast_diaphragm_valve_dsb->value();
			loadlock1_vacuum_fast_angle_valve = loadlock1_vacuum_fast_angle_valve_dsb->value();
			loadlock1_vacuum_delay_time = loadlock1_vacuum_delay_time_dsb->value();
			delay_time = loadlock1_vacuum_delay_time;
			uppper = loadlock1_vacuum_upper_limit;
			extraction = loadlock1_vacuum_extraction;
			fast_diaphragm_valve = loadlock1_vacuum_fast_diaphragm_valve;
			fast_angle_valve = loadlock1_vacuum_fast_angle_valve;
		}
		else{
			loadlock2_vacuum_upper_limit = loadlock2_vacuum_upper_limit_dsb->value();
			loadlock2_vacuum_extraction = loadlock2_vacuum_extraction_dsb->value();
			loadlock2_vacuum_fast_diaphragm_valve = loadlock2_vacuum_fast_diaphragm_valve_dsb->value();
			loadlock2_vacuum_fast_angle_valve = loadlock2_vacuum_fast_angle_valve_dsb->value();
			loadlock2_vacuum_delay_time = loadlock2_vacuum_delay_time_dsb->value();
			delay_time = loadlock2_vacuum_delay_time;

			uppper = loadlock2_vacuum_upper_limit;
			extraction = loadlock2_vacuum_extraction;
			fast_diaphragm_valve = loadlock2_vacuum_fast_diaphragm_valve;
			fast_angle_valve = loadlock2_vacuum_fast_angle_valve;
		}
		lk->setVacuumUpperLimitAndExtractionValue(uppper, extraction);
		lk->setVacuumFastDiapgragmValueAndAngleValue(fast_diaphragm_valve, fast_angle_valve);
		lk->setVacuumPumpingDelayTime(delay_time);
		rough_vacuum_set_value = rough_vacuum_value->value();
		lk->setRoughVacuumValue(rough_vacuum_set_value);
	}

	

	void QControlModeVTMWidgetPrivate::setTMCavityVacuumParameters(){
		auto tm = kernel->getKernelModule<FortrendTMCavitySubsystem>("TM");
		if (!tm)
		{
			//QMessageBox::information(this, "警告", "子系统未找到");
			logError("SaveConfig", "子系统未找到.");
			return;
		}
		tm_vacuum_delay_time = tm_cavity_vacuum_delay_time_dsb->value();
		tm_vacuum_upper_limit = tm_cavity_vacuum_upper_limit_dsb->value();
		tm_vacuum_extraction = tm_cavity_vacuum_extraction_dsb->value();
		rough_vacuum_set_value = rough_vacuum_value->value();
		tm->setTMCavityVacuumUpperLimitAndExtractionValue(tm_vacuum_upper_limit, tm_vacuum_extraction);
		tm->setRoughVacuumValue(rough_vacuum_set_value);
		tm->setVacuumPumpingDelayTime(tm_vacuum_delay_time);
		//tm_vaccum_pid = tm_cavity_vacuum_pid_dsb->value();
		//std::ostringstream oss;
		//oss << std::fixed << std::setprecision(5) << tm_vaccum_pid;
		//std::string buff = oss.str();
		//tm_vaccum_pid = std::stod(oss.str());
		//tm->setTMCavityPIDValue(tm_vaccum_pid);
	}

	void QControlModeVTMWidgetPrivate::setPMCavityVacuumParameters(const int index){

		//std::string pm_name = "PM" + std::to_string(index);
		//auto pm = kernel->getKernelModule<FortrendPMCavitySubsystem>(pm_name);
		//if (!pm)
		//{
		//	//QMessageBox::information(this, "警告", "子系统未找到");
		//	logError("SaveConfig", "子系统未找到.");
		//	return;
		//}
		//double uppper = 0.0;
		//double magnitude = 0.0;
		//if (index == 1)
		//{
		//	pm1_vacuum_setting = pm1_cavity_vacuum_upper_limit_dsb->value();
		//	pm1_vacuum_magnitude = ui->pm1_vacuum_magnitude_dsb->value();
		//	
		//	uppper = pm1_vacuum_setting;
		//	magnitude = getVacuumMagnitude(pm1_vacuum_magnitude);
		//}
		//else if (index == 2)
		//{
		//	pm2_vacuum_setting = pm2_cavity_vacuum_upper_limit_dsb->value();
		//	pm2_vacuum_magnitude = ui->pm2_vacuum_magnitude_dsb->value();

		//	uppper = pm2_vacuum_setting;
		//	magnitude = getVacuumMagnitude(pm2_vacuum_magnitude);
		//	
		//}
		//else if (index == 3)
		//{
		//	pm3_vacuum_setting =pm3_cavity_vacuum_upper_limit_dsb->value();
		//	pm3_vacuum_magnitude = ui->pm3_vacuum_magnitude_dsb->value();
		//	
		//	uppper = pm3_vacuum_setting;
		//	magnitude = getVacuumMagnitude(pm3_vacuum_magnitude);
		//}
		//else{
		//	return;
		//}
		//pm->setVacuumSettingAndMagnitudeValue(uppper, magnitude);
		
	}


	void QControlModeVTMWidgetPrivate::onAttributeChange(){
		QMetaObject::invokeMethod(q_ptr, "updateState", Qt::AutoConnection);
	}
	
	//获取真空流量级（待优化）
	double QControlModeVTMWidgetPrivate::getVacuumMagnitude(const double magnitude){
		double result = 0.0;
		if (magnitude == 3.0)
		{
			result = 0.001;
		}
		else if (magnitude < 3.0 && magnitude > 2.0)
		{
			result = (3 - magnitude) * 0.01;
		}
		else if (magnitude == 2.0)
		{
			result = 0.01;
		}
		else if (magnitude < 2.0 && magnitude > 1.0)
		{
			result = (2.0 - magnitude) * 0.1;
		}
		else if (magnitude == 1.0)
		{
			result = 0.1;
		}
		else
		{
			result = 1.0 - magnitude;
		}
		return result;
	}

	//QControlModeVTMWidget
	QControlModeVTMWidget::QControlModeVTMWidget(const std::shared_ptr<IKernel>& kernel, QWidget* parent)
		:QWidget(parent)\
		, d_ptr(new QControlModeVTMWidgetPrivate(this, kernel)){

		Q_D(QControlModeVTMWidget);
		d->ui->setupUi(this);

		
	/*	for (int i = 0; i < d->ui->vacuum_hlyt->count(); i++) {
			QWidget* widget = d->ui->vacuum_hlyt->itemAt(i)->widget();
			if (widget) {
				widget->hide();
			}
		}
		for (int i = 0; i < d->ui->wafer_hlyt->count(); i++) {
			QWidget* widget = d->ui->wafer_hlyt->itemAt(i)->widget();
			if (widget) {
				widget->hide();
			}
		}

		for (int i = 0; i < d->ui->pm_hlyt->count(); i++) {
			QWidget* widget = d->ui->pm_hlyt->itemAt(i)->widget();
			if (widget) {
				widget->hide();
			}
		}*/


		d->loadlock1_vacuum_upper_limit_dsb = new ScientificDoubleSpinBox(d->ui->vacuum_setting_gbx);
		d->loadlock1_vacuum_upper_limit_dsb->setObjectName(QStringLiteral("loadlock1_vacuum_upper_limit_dsb"));
		d->loadlock1_vacuum_upper_limit_dsb->setDecimals(6);
		d->loadlock1_vacuum_upper_limit_dsb->setMinimum(0.000001);
		d->loadlock1_vacuum_upper_limit_dsb->setRange(1.000e-5, 3.0e+2);
		d->loadlock1_vacuum_upper_limit_dsb->setSingleStep(0.0001);
		d->loadlock1_vacuum_upper_limit_dsb->setValue(1.0e+1);
		d->ui->gridLayout_2->addWidget(d->loadlock1_vacuum_upper_limit_dsb, 1, 2, 1, 1);

		d->loadlock1_vacuum_extraction_dsb = new ScientificDoubleSpinBox(d->ui->vacuum_setting_gbx);
		d->loadlock1_vacuum_extraction_dsb->setObjectName(QStringLiteral("loadlock1_vacuum_extraction_dsb"));
		d->loadlock1_vacuum_extraction_dsb->setDecimals(6);
		d->loadlock1_vacuum_extraction_dsb->setMinimum(0.000001);
		d->loadlock1_vacuum_extraction_dsb->setMaximum(30);
		d->loadlock1_vacuum_extraction_dsb->setSingleStep(0.0001);
		d->loadlock1_vacuum_extraction_dsb->setValue(0.008);
		d->ui->gridLayout_2->addWidget(d->loadlock1_vacuum_extraction_dsb, 1, 5, 1, 1);

		d->loadlock1_vacuum_fast_diaphragm_valve_dsb = new ScientificDoubleSpinBox(d->ui->vacuum_setting_gbx);
		d->loadlock1_vacuum_fast_diaphragm_valve_dsb->setObjectName(QStringLiteral("loadlock1_vacuum_fast_diaphragm_valve_dsb"));
		d->loadlock1_vacuum_fast_diaphragm_valve_dsb->setDecimals(1);
		d->loadlock1_vacuum_fast_diaphragm_valve_dsb->setMinimum(100);
		d->loadlock1_vacuum_fast_diaphragm_valve_dsb->setMaximum(99000);
		d->loadlock1_vacuum_fast_diaphragm_valve_dsb->setSingleStep(100);
		d->loadlock1_vacuum_fast_diaphragm_valve_dsb->setValue(5000);
		//d->loadlock1_vacuum_fast_diaphragm_valve_dsb->setSuffix("Pa");
		d->ui->gridLayout_2->addWidget(d->loadlock1_vacuum_fast_diaphragm_valve_dsb, 1, 8, 1, 1);

		d->loadlock1_vacuum_fast_angle_valve_dsb = new ScientificDoubleSpinBox(d->ui->vacuum_setting_gbx);
		d->loadlock1_vacuum_fast_angle_valve_dsb->setObjectName(QStringLiteral("loadlock1_vacuum_fast_angle_valve_dsb"));
		d->loadlock1_vacuum_fast_angle_valve_dsb->setDecimals(1);
		d->loadlock1_vacuum_fast_angle_valve_dsb->setMinimum(100);
		d->loadlock1_vacuum_fast_angle_valve_dsb->setMaximum(99000);
		d->loadlock1_vacuum_fast_angle_valve_dsb->setSingleStep(1000);
		d->loadlock1_vacuum_fast_angle_valve_dsb->setValue(5000);
		//d->loadlock1_vacuum_fast_angle_valve_dsb->setSuffix("Pa");
		d->ui->gridLayout_2->addWidget(d->loadlock1_vacuum_fast_angle_valve_dsb, 1, 11, 1, 1);


		d->loadlock2_vacuum_upper_limit_dsb = new ScientificDoubleSpinBox(d->ui->vacuum_setting_gbx);
		d->loadlock2_vacuum_upper_limit_dsb->setObjectName(QStringLiteral("loadlock2_vacuum_upper_limit_dsb"));
		d->loadlock2_vacuum_upper_limit_dsb->setDecimals(6);
		d->loadlock2_vacuum_upper_limit_dsb->setMinimum(0.000001);
		d->loadlock2_vacuum_upper_limit_dsb->setRange(1.000e-5, 3.0e+2);
		d->loadlock2_vacuum_upper_limit_dsb->setSingleStep(0.0001);
		d->loadlock2_vacuum_upper_limit_dsb->setValue(1.0e+1);
		d->ui->gridLayout_2->addWidget(d->loadlock2_vacuum_upper_limit_dsb, 2, 2, 1, 1);

		d->loadlock2_vacuum_extraction_dsb = new ScientificDoubleSpinBox(d->ui->vacuum_setting_gbx);
		d->loadlock2_vacuum_extraction_dsb->setObjectName(QStringLiteral("loadlock2_vacuum_extraction_dsb"));
		d->loadlock2_vacuum_extraction_dsb->setDecimals(6);
		d->loadlock2_vacuum_extraction_dsb->setMinimum(0.000001);
		d->loadlock2_vacuum_extraction_dsb->setMaximum(30);
		d->loadlock2_vacuum_extraction_dsb->setSingleStep(0.0001);
		d->loadlock2_vacuum_extraction_dsb->setValue(0.008);
		d->ui->gridLayout_2->addWidget(d->loadlock2_vacuum_extraction_dsb, 2, 5, 1, 1);

		d->loadlock2_vacuum_fast_diaphragm_valve_dsb = new ScientificDoubleSpinBox(d->ui->vacuum_setting_gbx);
		d->loadlock2_vacuum_fast_diaphragm_valve_dsb->setObjectName(QStringLiteral("loadlock2_vacuum_fast_diaphragm_valve_dsb"));
		d->loadlock2_vacuum_fast_diaphragm_valve_dsb->setDecimals(1);
		d->loadlock2_vacuum_fast_diaphragm_valve_dsb->setMinimum(100);
		d->loadlock2_vacuum_fast_diaphragm_valve_dsb->setMaximum(99000);
		d->loadlock2_vacuum_fast_diaphragm_valve_dsb->setSingleStep(100);
		d->loadlock2_vacuum_fast_diaphragm_valve_dsb->setValue(5000);
		//d->loadlock2_vacuum_fast_diaphragm_valve_dsb->setSuffix("Pa");
		d->ui->gridLayout_2->addWidget(d->loadlock2_vacuum_fast_diaphragm_valve_dsb, 2, 8, 1, 1);

		d->loadlock2_vacuum_fast_angle_valve_dsb = new ScientificDoubleSpinBox(d->ui->vacuum_setting_gbx);
		d->loadlock2_vacuum_fast_angle_valve_dsb->setObjectName(QStringLiteral("loadlock2_vacuum_fast_angle_valve_dsb"));
		d->loadlock2_vacuum_fast_angle_valve_dsb->setDecimals(1);
		d->loadlock2_vacuum_fast_angle_valve_dsb->setMinimum(100);
		d->loadlock2_vacuum_fast_angle_valve_dsb->setMaximum(99000);
		d->loadlock2_vacuum_fast_angle_valve_dsb->setSingleStep(1000);
		d->loadlock2_vacuum_fast_angle_valve_dsb->setValue(5000);
		//d->loadlock2_vacuum_fast_angle_valve_dsb->setSuffix("Pa");
		d->ui->gridLayout_2->addWidget(d->loadlock2_vacuum_fast_angle_valve_dsb, 2, 11, 1, 1);

		d->tm_cavity_vacuum_upper_limit_dsb = new ScientificDoubleSpinBox(d->ui->vacuum_setting_gbx);
		d->tm_cavity_vacuum_upper_limit_dsb->setObjectName(QStringLiteral("tm_cavity_vacuum_upper_limit_dsb"));
		d->tm_cavity_vacuum_upper_limit_dsb->setDecimals(6);
		d->tm_cavity_vacuum_upper_limit_dsb->setMinimum(0.000001);
		d->tm_cavity_vacuum_upper_limit_dsb->setRange(1.000e-5, 3.0e+2);
		d->tm_cavity_vacuum_upper_limit_dsb->setSingleStep(0.0001);
		d->tm_cavity_vacuum_upper_limit_dsb->setValue(1.0e+1);
		d->ui->gridLayout_2->addWidget(d->tm_cavity_vacuum_upper_limit_dsb, 3, 2, 1, 1);

		d->loadlock1_vacuum_delay_time_dsb = new ScientificDoubleSpinBox(d->ui->vacuum_setting_gbx);
		d->loadlock1_vacuum_delay_time_dsb->setObjectName(QStringLiteral("loadlock1_vacuum_delay_time_dsb"));
		d->loadlock1_vacuum_delay_time_dsb->setDecimals(6);
		d->loadlock1_vacuum_delay_time_dsb->setMinimum(1);
		d->loadlock1_vacuum_delay_time_dsb->setRange(1, 1000);
		d->loadlock1_vacuum_delay_time_dsb->setSingleStep(1);
		d->loadlock1_vacuum_delay_time_dsb->setValue(1.0e+1);
		d->ui->gridLayout_2->addWidget(d->loadlock1_vacuum_delay_time_dsb, 1, 13, 1, 1);

		d->loadlock2_vacuum_delay_time_dsb = new ScientificDoubleSpinBox(d->ui->vacuum_setting_gbx);
		d->loadlock2_vacuum_delay_time_dsb->setObjectName(QStringLiteral("loadlock2_vacuum_delay_time_dsb"));
		d->loadlock2_vacuum_delay_time_dsb->setDecimals(6);
		d->loadlock2_vacuum_delay_time_dsb->setMinimum(1);
		d->loadlock2_vacuum_delay_time_dsb->setRange(1, 1000);
		d->loadlock2_vacuum_delay_time_dsb->setSingleStep(1);
		d->loadlock2_vacuum_delay_time_dsb->setValue(1.0e+1);
		d->ui->gridLayout_2->addWidget(d->loadlock2_vacuum_delay_time_dsb, 2, 13, 1, 1);


		d->tm_cavity_vacuum_delay_time_dsb = new ScientificDoubleSpinBox(d->ui->vacuum_setting_gbx);
		d->tm_cavity_vacuum_delay_time_dsb->setObjectName(QStringLiteral("tm_cavity_vacuum_delay_time_dsb"));
		d->tm_cavity_vacuum_delay_time_dsb->setDecimals(6);
		d->tm_cavity_vacuum_delay_time_dsb->setMinimum(1);
		d->tm_cavity_vacuum_delay_time_dsb->setRange(1, 1000);
		d->tm_cavity_vacuum_delay_time_dsb->setSingleStep(1);
		d->tm_cavity_vacuum_delay_time_dsb->setValue(1.0e+1);
		d->ui->gridLayout_2->addWidget(d->tm_cavity_vacuum_delay_time_dsb, 3, 13, 1, 1);

		d->tm_cavity_vacuum_extraction_dsb = new ScientificDoubleSpinBox(d->ui->vacuum_setting_gbx);
		d->tm_cavity_vacuum_extraction_dsb->setObjectName(QStringLiteral("tm_cavity_vacuum_extraction_dsb"));
		d->tm_cavity_vacuum_extraction_dsb->setDecimals(6);
		d->tm_cavity_vacuum_extraction_dsb->setMinimum(0.000001);
		d->tm_cavity_vacuum_extraction_dsb->setMaximum(30);
		d->tm_cavity_vacuum_extraction_dsb->setSingleStep(0.0001);
		d->tm_cavity_vacuum_extraction_dsb->setValue(0.008);
		d->ui->gridLayout_2->addWidget(d->tm_cavity_vacuum_extraction_dsb, 3, 5, 1, 1);

		d->rough_vacuum_value = new ScientificDoubleSpinBox(d->ui->vacuum_setting_gbx);
		d->rough_vacuum_value->setObjectName(QStringLiteral("rough_vacuum_value"));
		d->rough_vacuum_value->setDecimals(6);
		d->rough_vacuum_value->setMinimum(5);
		d->rough_vacuum_value->setMaximum(15);
		d->rough_vacuum_value->setSingleStep(0.1);
		d->rough_vacuum_value->setValue(6);
		d->ui->gridLayout_2->addWidget(d->rough_vacuum_value, 3, 8, 1, 1);

		

		//d->tm_cavity_vacuum_pid_dsb = new ScientificDoubleSpinBox(d->ui->vacuum_setting_gbx);
		//d->tm_cavity_vacuum_pid_dsb->setObjectName(QStringLiteral("tm_cavity_vacuum_pid_dsb"));
		//d->tm_cavity_vacuum_pid_dsb->setDecimals(5);
		//d->tm_cavity_vacuum_pid_dsb->setMinimum(0.006);
		//d->tm_cavity_vacuum_pid_dsb->setMaximum(0.1);
		//d->tm_cavity_vacuum_pid_dsb->setSingleStep(0.05);
		//d->tm_cavity_vacuum_pid_dsb->setValue(0.008);
		////d->tm_cavity_vacuum_pid_dsb->setSuffix("Pa");
		//d->ui->gridLayout_2->addWidget(d->tm_cavity_vacuum_pid_dsb, 3, 8, 1, 1);

		//d->pm1_cavity_vacuum_upper_limit_dsb = new ScientificDoubleSpinBox(d->ui->vacuum_setting_gbx);
		//d->pm1_cavity_vacuum_upper_limit_dsb->setObjectName(QStringLiteral("pm1_cavity_vacuum_upper_limit_dsb"));
		//d->pm1_cavity_vacuum_upper_limit_dsb->setDecimals(5);
		//d->pm1_cavity_vacuum_upper_limit_dsb->setMinimum(0.00001);
		//d->pm1_cavity_vacuum_upper_limit_dsb->setMaximum(30);
		//d->pm1_cavity_vacuum_upper_limit_dsb->setSingleStep(0.005);
		//d->pm1_cavity_vacuum_upper_limit_dsb->setValue(8);
		////d->pm1_cavity_vacuum_upper_limit_dsb->setSuffix("Pa");
		//d->ui->gridLayout_2->addWidget(d->pm1_cavity_vacuum_upper_limit_dsb, 4, 2, 1, 1);

		//d->pm2_cavity_vacuum_upper_limit_dsb = new ScientificDoubleSpinBox(d->ui->vacuum_setting_gbx);
		//d->pm2_cavity_vacuum_upper_limit_dsb->setObjectName(QStringLiteral("pm2_cavity_vacuum_upper_limit_dsb"));
		//d->pm2_cavity_vacuum_upper_limit_dsb->setDecimals(5);
		//d->pm2_cavity_vacuum_upper_limit_dsb->setMinimum(0.00001);
		//d->pm2_cavity_vacuum_upper_limit_dsb->setMaximum(30);
		//d->pm2_cavity_vacuum_upper_limit_dsb->setSingleStep(0.005);
		//d->pm2_cavity_vacuum_upper_limit_dsb->setValue(8);
		////d->pm2_cavity_vacuum_upper_limit_dsb->setSuffix("Pa");
		//d->ui->gridLayout_2->addWidget(d->pm2_cavity_vacuum_upper_limit_dsb, 4, 2, 1, 1);

		//d->pm3_cavity_vacuum_upper_limit_dsb = new ScientificDoubleSpinBox(d->ui->vacuum_setting_gbx);
		//d->pm3_cavity_vacuum_upper_limit_dsb->setObjectName(QStringLiteral("pm1_cavity_vacuum_upper_limit_dsb"));
		//d->pm3_cavity_vacuum_upper_limit_dsb->setDecimals(5);
		//d->pm3_cavity_vacuum_upper_limit_dsb->setMinimum(0.00001);
		//d->pm3_cavity_vacuum_upper_limit_dsb->setMaximum(30);
		//d->pm3_cavity_vacuum_upper_limit_dsb->setSingleStep(0.005);
		//d->pm3_cavity_vacuum_upper_limit_dsb->setValue(8);
		////d->pm3_cavity_vacuum_upper_limit_dsb->setSuffix("Pa");
		//d->ui->gridLayout_2->addWidget(d->pm3_cavity_vacuum_upper_limit_dsb, 6, 2, 1, 1);
		
		loadConfigFile();

		connect(d->ui->enable_buzzer_btn, &SwitchButton::checkedChanged, this, &QControlModeVTMWidget::onSetBuzzerEnableStatus,Qt::QueuedConnection);
		//connect(d->ui->disable_buzzer_btn, &QAbstractButton::clicked, this, &QControlModeVTMWidget::onSetBuzzerDisable);

		connect(d->ui->enable_vacuum_btn, &SwitchButton::checkedChanged, this, &QControlModeVTMWidget::onSetVacuumEnableStatus, Qt::QueuedConnection);
		//connect(d->ui->disable_vacuum_btn, &QAbstractButton::clicked, this, &QControlModeVTMWidget::onSetVacuumDisable);

		//connect(d->ui->enable_buzzer_btn, &SwitchButton::checkedChanged, this, &QControlModeVTMWidget::onSetVacuumEnableStatus);

		connect(d->ui->enable_with_wafer_btn, &SwitchButton::checkedChanged, this, &QControlModeVTMWidget::onSetWithWaferModeStatus, Qt::QueuedConnection);
		//connect(d->ui->disable_with_wafer_btn, &QAbstractButton::clicked, this, &QControlModeVTMWidget::onSetWithWaferModeDisable);

		connect(d->ui->enable_pm1_btn, &SwitchButton::checkedChanged, this, &QControlModeVTMWidget::onSetPM1Enabletatus, Qt::QueuedConnection);
		//connect(d->ui->disable_pm1_btn, &QAbstractButton::clicked, this, &QControlModeVTMWidget::onSetPM1Disable);
		connect(d->ui->enable_pm2_btn, &SwitchButton::checkedChanged, this, &QControlModeVTMWidget::onSetPM2Enabletatus, Qt::QueuedConnection);
		//connect(d->ui->disable_pm2_btn, &QAbstractButton::clicked, this, &QControlModeVTMWidget::onSetPM2Disable);
		connect(d->ui->enable_pm3_btn, &SwitchButton::checkedChanged, this, &QControlModeVTMWidget::onSetPM3Enabletatus, Qt::QueuedConnection);
		//connect(d->ui->disable_pm3_btn, &QAbstractButton::clicked, this, &QControlModeVTMWidget::onSetPM3Disable);
		connect(d->ui->enable_pm4_btn, &SwitchButton::checkedChanged, this, &QControlModeVTMWidget::onSetPM4Enabletatus, Qt::QueuedConnection);
		//connect(d->ui->disable_pm4_btn, &QAbstractButton::clicked, this, &QControlModeVTMWidget::onSetPM4Disable);
		//d->kernel->addListener(d);

		connect(d->ui->loadlock1_vacuum_set_pbt, &QAbstractButton::clicked, this, &QControlModeVTMWidget::onSetLoadLock1VacuumParameters);
		connect(d->ui->loadlock2_vacuum_set_pbt, &QAbstractButton::clicked, this, &QControlModeVTMWidget::onSetLoadLock2VacuumParameters);
		connect(d->ui->tm_cavity_vacuum_set_pbt, &QAbstractButton::clicked, this, &QControlModeVTMWidget::onSetTMCavityVacuumParamerers);
		//connect(d->ui->pm1_cavity_vacuum_set_pbt, &QAbstractButton::clicked, this, &QControlModeVTMWidget::onSetPM1CavityVacuumParameters);
		//connect(d->ui->pm2_cavity_vacuum_set_pbt, &QAbstractButton::clicked, this, &QControlModeVTMWidget::onSetPM2CavityVacuumParameters);
		//connect(d->ui->pm3_cavity_vacuum_set_pbt, &QAbstractButton::clicked, this, &QControlModeVTMWidget::onSetPM3CavityVacuumParameters);

		//调试模式注释，出场前放开
		//d->ui->vacuum_label->hide();
		//d->ui->enable_vacuum_btn->hide();
		//d->ui->disable_vacuum_btn->hide();
		//d->ui->label_2->hide();

		//d->ui->pm2_enable_label->hide();
		//d->ui->enable_pm2_btn->hide();
		//d->ui->disable_pm2_btn->hide();
		//d->ui->label_5->hide();

		//d->ui->with_wafer_label->hide();
		//d->ui->enable_with_wafer_btn->hide();
		//d->ui->disable_with_wafer_btn->hide();
		//d->ui->label_4->hide();
	}

	QControlModeVTMWidget::~QControlModeVTMWidget(){
		Q_D(QControlModeVTMWidget);
		//d->api->removeListener(d);
		delete d_ptr;
	}
	void QControlModeVTMWidget::onSetBuzzerEnable(){
		Q_D(QControlModeVTMWidget);

		d->setBuzzerEnable(true);
		updateState();
		saveConfigFile();
	}
	void QControlModeVTMWidget::onSetBuzzerDisable(){
		Q_D(QControlModeVTMWidget);

		d->setBuzzerEnable(false);
		updateState();
		saveConfigFile();
	}

	void QControlModeVTMWidget::onSetBuzzerEnableStatus(bool checked)
	{
		//qint64 start = QDateTime::currentMSecsSinceEpoch(); // 开始时间
		Q_D(QControlModeVTMWidget);
		if (checked)
		{
			d->setBuzzerEnable(true);
			updateState();
			saveConfigFile();
		}
		else
		{
			d->setBuzzerEnable(false);
			updateState();
			saveConfigFile();
		}
		//qint64 end = QDateTime::currentMSecsSinceEpoch(); // 结束时间
		//qDebug() << "槽函数执行耗时：" << (end - start) << "ms"; // 打印耗时
	}

	void QControlModeVTMWidget::onSetVacuumEnable(){

		qint64 start = QDateTime::currentMSecsSinceEpoch(); // 开始时间
		Q_D(QControlModeVTMWidget);

		d->setVacuumEnable(true);
		updateState();
		saveConfigFile();

		qint64 end = QDateTime::currentMSecsSinceEpoch(); // 结束时间
		qDebug() << "槽函数执行耗时：" << (end - start) << "ms"; // 打印耗时
	}
	void QControlModeVTMWidget::onSetVacuumDisable(){
		Q_D(QControlModeVTMWidget);

		d->setVacuumEnable(false);
		updateState();
		saveConfigFile();
	}

	void QControlModeVTMWidget::onSetVacuumEnableStatus(bool checked)
	{
		Q_D(QControlModeVTMWidget);

		if (checked)
		{
			d->setVacuumEnable(true);
			updateState();
			saveConfigFile();
		}
		else
		{
			d->setVacuumEnable(false);
			updateState();
			saveConfigFile();
		}
		
	}

	void QControlModeVTMWidget::onSetWithWaferModeEnable(){
		Q_D(QControlModeVTMWidget);
		d->setWithWaferMode(true);
		updateState();
		saveConfigFile();
	}

	void QControlModeVTMWidget::onSetWithWaferModeDisable(){
		Q_D(QControlModeVTMWidget);
		d->setWithWaferMode(false);
		updateState();
		saveConfigFile();
	}

	void QControlModeVTMWidget::onSetWithWaferModeStatus(bool checked)
	{
		Q_D(QControlModeVTMWidget);
		if (checked)
		{
			d->setWithWaferMode(true);
			updateState();
			saveConfigFile();
		}
		else
		{
			d->setWithWaferMode(false);
			updateState();
			saveConfigFile();
		}

	}

	void QControlModeVTMWidget::onSetPM1Enable(){
		Q_D(QControlModeVTMWidget);
		d->setPMCavity(1, true);
		updateState();
		saveConfigFile();
	}
	void QControlModeVTMWidget::onSetPM1Disable(){
		Q_D(QControlModeVTMWidget);
		d->setPMCavity(1, false);
		updateState();
		saveConfigFile();
	}
	void QControlModeVTMWidget::onSetPM1Enabletatus(bool checked)
	{
		Q_D(QControlModeVTMWidget);
		if (checked)
		{
			d->setPMCavity(1, true);
			updateState();
			saveConfigFile();
		}
		else
		{
			d->setPMCavity(1, false);
			updateState();
			saveConfigFile();
		}
	}
	void QControlModeVTMWidget::onSetPM2Enable(){
		Q_D(QControlModeVTMWidget);
		d->setPMCavity(2, true);
		updateState();
		saveConfigFile();
	}
	void QControlModeVTMWidget::onSetPM2Disable(){
		Q_D(QControlModeVTMWidget);
		d->setPMCavity(2, false);
		updateState();
		saveConfigFile();
	}
	void QControlModeVTMWidget::onSetPM2Enabletatus(bool checked)
	{
		Q_D(QControlModeVTMWidget);
		if (checked)
		{
			d->setPMCavity(2, true);
			updateState();
			saveConfigFile();
		}
		else
		{
			d->setPMCavity(2, false);
			updateState();
			saveConfigFile();
		}
	}
	void QControlModeVTMWidget::onSetPM3Enable(){
		Q_D(QControlModeVTMWidget);
		d->setPMCavity(3, true);
		updateState();
		saveConfigFile();
	}
	void QControlModeVTMWidget::onSetPM3Disable(){
		Q_D(QControlModeVTMWidget);
		d->setPMCavity(3, false);
		updateState();
		saveConfigFile();
	}

	void QControlModeVTMWidget::onSetPM3Enabletatus(bool checked)
	{
		Q_D(QControlModeVTMWidget);
		if (checked)
		{
			d->setPMCavity(3, true);
			updateState();
			saveConfigFile();
		}
		else {
			d->setPMCavity(3, false);
			updateState();
			saveConfigFile();
		}

	}

	void QControlModeVTMWidget::onSetPM4Enable()
	{
		Q_D(QControlModeVTMWidget);
		d->setPMCavity(4, true);
		updateState();
		saveConfigFile();
	}

	void QControlModeVTMWidget::onSetPM4Disable()
	{
		Q_D(QControlModeVTMWidget);
		d->setPMCavity(4, false);
		updateState();
		saveConfigFile();
	}

	void QControlModeVTMWidget::onSetPM4Enabletatus(bool checked)
	{
		Q_D(QControlModeVTMWidget);
		if (checked)
		{
			d->setPMCavity(4, true);
			updateState();
			saveConfigFile();
		}
		else
		{
			d->setPMCavity(4, false);
			updateState();
			saveConfigFile();
		}

	}

	void QControlModeVTMWidget::onSetLoadLock1VacuumParameters(){
		Q_D(QControlModeVTMWidget);
		d->setLoadLockVacuumParameters(1);
		saveConfigFile();
	}

	void QControlModeVTMWidget::onSetLoadLock2VacuumParameters(){
		Q_D(QControlModeVTMWidget);
		d->setLoadLockVacuumParameters(2);
		saveConfigFile();
	}

	void QControlModeVTMWidget::onSetTMCavityVacuumParamerers(){
		Q_D(QControlModeVTMWidget);
		d->setTMCavityVacuumParameters();
		saveConfigFile();
	}
	void QControlModeVTMWidget::onSetPM1CavityVacuumParameters(){
		Q_D(QControlModeVTMWidget);
		d->setPMCavityVacuumParameters(1);
		saveConfigFile();
	}

	void QControlModeVTMWidget::onSetPM2CavityVacuumParameters(){
		Q_D(QControlModeVTMWidget);
		d->setPMCavityVacuumParameters(2);
		saveConfigFile();
	}

	void QControlModeVTMWidget::onSetPM3CavityVacuumParameters(){
		Q_D(QControlModeVTMWidget);
		d->setPMCavityVacuumParameters(3);
		saveConfigFile();
	}

	void QControlModeVTMWidget::updateState(){
		Q_D(QControlModeVTMWidget);
		d->ui->buzzer_label->setText(enable_stat_map[d->buzzer_enable]);
		d->ui->vacuum_label->setText(enable_stat_map[d->vacuum_enable]);
		d->ui->with_wafer_label->setText(enable_stat_map[d->with_wafer_enable]);
		d->ui->pm1_enable_label->setText(enable_stat_map[d->pm1_enable]);
		d->ui->pm2_enable_label->setText(enable_stat_map[d->pm2_enable]);
		d->ui->pm3_enable_label->setText(enable_stat_map[d->pm3_enable]);
		d->ui->pm4_enable_label->setText(enable_stat_map[d->pm4_enable]);
		
		d->loadlock1_vacuum_upper_limit_dsb->setValue(d->loadlock1_vacuum_upper_limit);
		d->loadlock1_vacuum_extraction_dsb->setValue(d->loadlock1_vacuum_extraction);
		d->loadlock1_vacuum_fast_diaphragm_valve_dsb->setValue(d->loadlock1_vacuum_fast_diaphragm_valve);
		d->loadlock1_vacuum_fast_angle_valve_dsb->setValue(d->loadlock1_vacuum_fast_angle_valve);

		d->loadlock2_vacuum_upper_limit_dsb->setValue(d->loadlock2_vacuum_upper_limit);
		d->loadlock2_vacuum_extraction_dsb->setValue(d->loadlock2_vacuum_extraction);
		d->loadlock2_vacuum_fast_diaphragm_valve_dsb->setValue(d->loadlock2_vacuum_fast_diaphragm_valve);
		d->loadlock2_vacuum_fast_angle_valve_dsb->setValue(d->loadlock2_vacuum_fast_angle_valve);

		d->tm_cavity_vacuum_upper_limit_dsb->setValue(d->tm_vacuum_upper_limit);
		d->tm_cavity_vacuum_extraction_dsb->setValue(d->tm_vacuum_extraction);
		d->rough_vacuum_value->setValue(d->rough_vacuum_set_value);
		d->loadlock1_vacuum_delay_time_dsb->setValue(d->loadlock1_vacuum_delay_time);
		d->loadlock2_vacuum_delay_time_dsb->setValue(d->loadlock2_vacuum_delay_time);
		d->tm_cavity_vacuum_delay_time_dsb->setValue(d->tm_vacuum_delay_time);

	}

	void QControlModeVTMWidget::InitUi()
	{
		Q_D(QControlModeVTMWidget);
		d->ui->enable_buzzer_btn->setBgColorOn(color1);
		d->ui->enable_buzzer_btn->setShowText(true);
		d->ui->enable_buzzer_btn->setTextOff("禁用");
		d->ui->enable_buzzer_btn->setTextOn("启用");

		d->ui->enable_vacuum_btn->setShowText(true);
		d->ui->enable_vacuum_btn->setBgColorOn(color2);
		d->ui->enable_vacuum_btn->setTextOff("禁用");
		d->ui->enable_vacuum_btn->setTextOn("启用");

		d->ui->enable_pm1_btn->setShowText(true);
		d->ui->enable_pm1_btn->setBgColorOn(color2);
		d->ui->enable_pm1_btn->setTextOff("禁用");
		d->ui->enable_pm1_btn->setTextOn("启用");


		d->ui->enable_pm2_btn->setShowText(true);
		d->ui->enable_pm2_btn->setBgColorOn(color3);
		d->ui->enable_pm2_btn->setTextOff("禁用");
		d->ui->enable_pm2_btn->setTextOn("启用");


		d->ui->enable_pm3_btn->setShowText(true);
		d->ui->enable_pm3_btn->setBgColorOn(color4);
		d->ui->enable_pm3_btn->setTextOff("禁用");
		d->ui->enable_pm3_btn->setTextOn("启用");
						
		d->ui->enable_pm4_btn->setShowText(true);
		d->ui->enable_pm4_btn->setBgColorOn(color1);
		d->ui->enable_pm4_btn->setTextOff("禁用");
		d->ui->enable_pm4_btn->setTextOn("启用");

		d->ui->enable_with_wafer_btn->setShowText(true);

		d->ui->enable_with_wafer_btn->setBgColorOn(color1);
		d->ui->enable_with_wafer_btn->setTextOff("禁用");
		d->ui->enable_with_wafer_btn->setTextOn("启用");
	}

	void QControlModeVTMWidget::loadConfigFile(){
		Q_D(QControlModeVTMWidget);
		QString fileName = QDir::currentPath() + "/config/" + "config.ini";
		if (fileName.isEmpty())
			return;
		QSettings settings(fileName, QSettings::IniFormat);
		bool buzzer = settings.value("BuzzerEnable", true).toBool();
		bool vacuum = settings.value("VacuumEnable", true).toBool();
		bool with_wafer = settings.value("WithWaferMode", true).toBool();
		d->setBuzzerEnable(buzzer);
		d->setVacuumEnable(vacuum);
		d->setWithWaferMode(with_wafer);

		bool pm1 = settings.value("PM1Enable", true).toBool();
		bool pm2 = settings.value("PM2Enable", true).toBool();
		bool pm3 = settings.value("PM3Enable", true).toBool();
		bool pm4 = settings.value("PM4Enable", true).toBool();
		d->setPMCavity(1, pm1);
		d->setPMCavity(2, pm2);
		d->setPMCavity(3, pm3);
		d->setPMCavity(4, pm4);

		d->loadlock1_vacuum_upper_limit = settings.value("LoadLock1VacuumUpperLimit", true).toDouble();
		d->loadlock1_vacuum_extraction = settings.value("LoadLock1VacuumExtraction", true).toDouble();
		d->loadlock1_vacuum_fast_diaphragm_valve = settings.value("LoadLock1VacuumFastDiaphragmValve", true).toDouble();
		d->loadlock1_vacuum_fast_angle_valve = settings.value("LoadLock1VacuumFastAngleValve", true).toDouble();

		d->loadlock2_vacuum_upper_limit = settings.value("LoadLock2VacuumUpperLimit", true).toDouble();
		d->loadlock2_vacuum_extraction = settings.value("LoadLock2VacuumExtraction", true).toDouble();
		d->loadlock2_vacuum_fast_diaphragm_valve = settings.value("LoadLock2VacuumFastDiaphragmValve", true).toDouble();
		d->loadlock2_vacuum_fast_angle_valve = settings.value("LoadLock2VacuumFastAngleValve", true).toDouble();

		d->tm_vacuum_upper_limit = settings.value("TMCavityVacuumUpperLimit", true).toDouble();
		d->tm_vacuum_extraction = settings.value("TMCavityVacuumExtraction", true).toDouble();
		d->rough_vacuum_set_value = settings.value("RoughVacuumSetting", true).toDouble();
		//d->tm_vaccum_pid = settings.value("TMCavityVacuumPID", true).toDouble();

		d->pm1_vacuum_setting = settings.value("PM1CavityVacuumSetting", true).toDouble();
		d->pm1_vacuum_magnitude = settings.value("PM1CavityVacuumMagnitude", true).toDouble();

		d->pm2_vacuum_setting = settings.value("PM2CavityVacuumSetting", true).toDouble();
		d->pm2_vacuum_magnitude = settings.value("PM2CavityVacuumMagnitude", true).toDouble();

		d->pm3_vacuum_setting = settings.value("PM3CavityVacuumSetting", true).toDouble();
		d->pm3_vacuum_magnitude = settings.value("PM3CavityVacuumMagnitude", true).toDouble();

		d->loadlock1_vacuum_delay_time = settings.value("Loadlock1VacuumDelayTime", true).toDouble();
		d->loadlock2_vacuum_delay_time = settings.value("Loadlock2VacuumDelayTime", true).toDouble();
		d->tm_vacuum_delay_time = settings.value("TmVacuumDelayTime", true).toDouble();

		updateState();

		d->setLoadLockVacuumParameters(1);
		d->setLoadLockVacuumParameters(2);
		d->setTMCavityVacuumParameters();
		d->setPMCavityVacuumParameters(1);
		d->setPMCavityVacuumParameters(2);
		d->setPMCavityVacuumParameters(3);
	}

	void QControlModeVTMWidget::saveConfigFile(){
		Q_D(QControlModeVTMWidget);
		QString fileName = QDir::currentPath() + "/config/" + "config.ini";
		if (fileName.isEmpty())
			return;
		QSettings settings(fileName, QSettings::IniFormat);
		settings.setValue("BuzzerEnable", d->buzzer_enable);
		settings.setValue("VacuumEnable", d->vacuum_enable);
		settings.setValue("WithWaferMode", d->with_wafer_enable);
		settings.setValue("PM1Enable", d->pm1_enable);
		settings.setValue("PM2Enable", d->pm2_enable);
		settings.setValue("PM3Enable", d->pm3_enable);

		settings.setValue("LoadLock1VacuumUpperLimit", d->loadlock1_vacuum_upper_limit);
		settings.setValue("LoadLock1VacuumExtraction", d->loadlock1_vacuum_extraction);
		settings.setValue("LoadLock1VacuumFastDiaphragmValve", d->loadlock1_vacuum_fast_diaphragm_valve);
		settings.setValue("LoadLock1VacuumFastAngleValve", d->loadlock1_vacuum_fast_angle_valve);

		settings.setValue("LoadLock2VacuumUpperLimit", d->loadlock2_vacuum_upper_limit);
		settings.setValue("LoadLock2VacuumExtraction", d->loadlock2_vacuum_extraction);
		settings.setValue("LoadLock2VacuumFastDiaphragmValve", d->loadlock2_vacuum_fast_diaphragm_valve);
		settings.setValue("LoadLock2VacuumFastAngleValve", d->loadlock2_vacuum_fast_angle_valve);

		settings.setValue("TMCavityVacuumUpperLimit", d->tm_vacuum_upper_limit);
		settings.setValue("TMCavityVacuumExtraction", d->tm_vacuum_extraction);
		settings.setValue("TMCavityVacuumPID", d->tm_vaccum_pid);
		settings.setValue("PM1CavityVacuumSetting", d->pm1_vacuum_setting);
		settings.setValue("PM1CavityVacuumMagnitude", d->pm1_vacuum_magnitude);
		settings.setValue("PM2CavityVacuumSetting", d->pm2_vacuum_setting);
		settings.setValue("PM2CavityVacuumMagnitude", d->pm2_vacuum_magnitude);
		settings.setValue("PM3CavityVacuumSetting", d->pm3_vacuum_setting);
		settings.setValue("PM3CavityVacuumMagnitude", d->pm3_vacuum_magnitude);
		settings.setValue("RoughVacuumSetting", d->rough_vacuum_set_value);

		settings.setValue("Loadlock1VacuumDelayTime", d->loadlock1_vacuum_delay_time);
		settings.setValue("Loadlock2VacuumDelayTime", d->loadlock2_vacuum_delay_time);
		settings.setValue("TmVacuumDelayTime", d->tm_vacuum_delay_time);
	}
}
