// Library: VTM
// Library: VTM
// Package: VTM

#include "Kernel/kernel_api.h"
#include "Poco/Format.h"
#include "pm_recipe_widget.h"
#include "device/ui_pm_recipe_widget.h"
#include "PMCavity/fortrend_pm_cavity_defined.h"
#include "PMCavity/fortrend_pm_cavity_subsystem.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDir>
#include <QMessageBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <thread>
#include <atomic>
#include <QComboBox>
#include <QPushButton>
#include <chrono>
#include <QSpinBox>
#include <QMap>
#include <thread>
#include <mutex>
#include <condition_variable>

#if _MSC_VER >1600
#pragma execution_character_set("utf-8")
#endif

#define max(a, b) (((a) > (b)) ? (a) : (b))

namespace FC {

	class QPmRecipeWidgetPrivate
	{
	public:
		Q_DECLARE_PUBLIC(QPmRecipeWidget)
		QPmRecipeWidgetPrivate(QPmRecipeWidget*p, const std::shared_ptr<IKernel>& kernel);
		~QPmRecipeWidgetPrivate();
	
		void initTableWidget(QTableWidget* tableWidget);
		QTableWidget* getCurrentTableWidget();
		QTableWidget* getIndexTableWidget(int index);
		void runTimer(double minutes);
		
	private:
		Ui::QPmRecipeWidgetClass* ui;
		std::shared_ptr<IKernel> kernel;
		QPmRecipeWidget* q_ptr;
	
		// 四个tab页对应的表格
		QTableWidget* pm1TableWidget;
		QTableWidget* pm2TableWidget;
		QTableWidget* pm3TableWidget;
		QTableWidget* pm4TableWidget;

		// 存储所有PM腔的工艺数据---废弃
		std::map<std::string, std::vector<PMMotionProcessParameters>> pmMotionProcessData;

		//pm统一的PM腔的工艺数据
		std::map<std::string, QPmRecipeWidget::PMRecipeConfig> pmRecipeConfigMap;

		std::chrono::steady_clock::time_point start_time;//开始时间点
		const std::chrono::hours timeout = std::chrono::hours(1); //超时时间
		double processElapsed = 0.0; //工艺耗时（单位：分钟）

		std::string current_pm; //要执行测试的PM
		std::thread cycleThread;
		std::atomic<bool> isRunning;
		std::atomic<bool> stopRequested;
		std::atomic<int> currentCycleCount;
		std::atomic<bool> timerFinished{ false };
		std::condition_variable cycleCv;
		std::mutex cycleMutex;

		friend class QPmRecipeWidget;
	};

	QPmRecipeWidgetPrivate::QPmRecipeWidgetPrivate(QPmRecipeWidget* p, const std::shared_ptr<IKernel>& kernel)
		:q_ptr(p)
		, kernel(kernel)
		, ui(new Ui::QPmRecipeWidgetClass)
		, pm1TableWidget(nullptr)
		, pm2TableWidget(nullptr)
		, pm3TableWidget(nullptr)
		, pm4TableWidget(nullptr) 
		, isRunning(false)
		, stopRequested(false)
		, currentCycleCount(0)
	{

	}
	
	QPmRecipeWidgetPrivate::~QPmRecipeWidgetPrivate()
	{
		if (isRunning) {
			stopRequested = true;
			if (cycleThread.joinable()) {
				cycleThread.join();
			}
		}
		delete ui;
	}
	
	void QPmRecipeWidgetPrivate::runTimer(double minutes)
	{
		timerFinished.store(false);
		std::thread([this, minutes]() {
			auto ms = std::chrono::milliseconds(static_cast<long long>(minutes * 60000.0));
			std::this_thread::sleep_for(ms);
			timerFinished.store(true);
			cycleCv.notify_all();
		}).detach();
	}

	void QPmRecipeWidgetPrivate::initTableWidget(QTableWidget* tableWidget)
	{
		if (!tableWidget)
			return;
		
		// 设置表格属性，与主表格一致
		tableWidget->setAlternatingRowColors(true);
		// 显示垂直表头（自动生成行号）
		tableWidget->verticalHeader()->setVisible(true);
		// 设置垂直表头宽度
		tableWidget->verticalHeader()->setDefaultSectionSize(30);
		tableWidget->setMinimumSize(600, 200);
		tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);

		for (size_t i = 0; i < tableWidget->columnCount(); i++)
		{
			tableWidget->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Stretch);
		}
		// 设置中文字体
		QFont font;
		font.setFamily("SimHei"); // 黑体
		tableWidget->horizontalHeader()->setFont(font);
		tableWidget->verticalHeader()->setFont(font);
		
		// 设置列数和列名
		tableWidget->setColumnCount(8);
		QStringList headers = {
			"liftingAcce1",
			"liftingAcce2",
			"liftingAcce3",
			"liftingAcce4",
			"rotatingAcce1",
			"rotatingAcce2",
			"rotatingAcce3",
			"rotatingAcce4"
		};
		tableWidget->setHorizontalHeaderLabels(headers);
	}

	QTableWidget* QPmRecipeWidgetPrivate::getCurrentTableWidget()
	{
		int currentIndex = ui->tabWidget->currentIndex();
		switch (currentIndex) {
		case 0:
			return pm1TableWidget;
		case 1:
			return pm2TableWidget;
		case 2:
			return pm3TableWidget;
		case 3:
			return pm4TableWidget;
		default:
			return nullptr;
		}
	}

	QTableWidget* QPmRecipeWidgetPrivate::getIndexTableWidget(int index)
	{
		switch (index) {
		case 0:
			return pm1TableWidget;
		case 1:
			return pm2TableWidget;
		case 2:
			return pm3TableWidget;
		case 3:
			return pm4TableWidget;
		default:
			return nullptr;
		}
	}

	QPmRecipeWidget::QPmRecipeWidget(const std::shared_ptr<IKernel>& kernel,QWidget* parent)
		: QWidget(parent)
		, d_ptr(new QPmRecipeWidgetPrivate(this, kernel))
	{
		Q_D(QPmRecipeWidget);
		d->ui->setupUi(this);
		
		initPMCavityParamEdieTableWidget();

		// 为每个tab页创建并配置QTableWidget
		d->pm1TableWidget = new QTableWidget(d->ui->tab);
		d->pm2TableWidget = new QTableWidget(d->ui->tab_2);
		d->pm3TableWidget = new QTableWidget(d->ui->tab_3);
		d->pm4TableWidget = new QTableWidget(d->ui->tab_4);
		
		// 初始化表格
		d->initTableWidget(d->pm1TableWidget);
		d->initTableWidget(d->pm2TableWidget);
		d->initTableWidget(d->pm3TableWidget);
		d->initTableWidget(d->pm4TableWidget);
		
		// 创建布局并将表格添加到tab页
		QVBoxLayout* layout1 = new QVBoxLayout(d->ui->tab);
		layout1->addWidget(d->pm1TableWidget);
		
		QVBoxLayout* layout2 = new QVBoxLayout(d->ui->tab_2);
		layout2->addWidget(d->pm2TableWidget);
		
		QVBoxLayout* layout3 = new QVBoxLayout(d->ui->tab_3);
		layout3->addWidget(d->pm3TableWidget);
		
		QVBoxLayout* layout4 = new QVBoxLayout(d->ui->tab_4);
		layout4->addWidget(d->pm4TableWidget);


		connect(d->ui->add_an_item_pbt, &QPushButton::clicked, this, &QPmRecipeWidget::onAddAnItem);

		connect(d->ui->delete_the_selected_item_pbt, &QPushButton::clicked, this, &QPmRecipeWidget::onDeleteTheSelectedItem);

		connect(d->ui->load_prams_pbt, &QPushButton::clicked, this, &QPmRecipeWidget::onLoadParameters);

		connect(d->ui->clear_prams_pbt, &QPushButton::clicked, this, &QPmRecipeWidget::onClearParameters);

		connect(d->ui->save_prams_pbt, &QPushButton::clicked, this, &QPmRecipeWidget::onSetParameters);

		connect(d->ui->start_pm_pbt, &QPushButton::clicked, this, &QPmRecipeWidget::onStartCycle);

		connect(d->ui->stop_pm_pbt, &QPushButton::clicked, this, &QPmRecipeWidget::onStopCycle);

		d->ui->pm_selecte_cbx->clear();
		d->ui->pm_selecte_cbx->addItem("PM1", 0);
		d->ui->pm_selecte_cbx->addItem("PM2", 1);
		d->ui->pm_selecte_cbx->addItem("PM3", 2);
		d->ui->pm_selecte_cbx->addItem("PM4", 3);

		d->ui->pm_selecte_cbx->setCurrentIndex(d->ui->tabWidget->currentIndex());

		connect(d->ui->pm_selecte_cbx, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &QPmRecipeWidget::onSelectPMChanged);


	}

	QPmRecipeWidget::~QPmRecipeWidget()
	{

	}
	
	// 获取pmMotionProcessData的公共方法实现
	std::map<std::string, std::vector<PMMotionProcessParameters>>& QPmRecipeWidget::getPMMotionProcessData()
	{
		Q_D(QPmRecipeWidget);
		return d->pmMotionProcessData;
	}
	std::map<std::string, QPmRecipeWidget::PMRecipeConfig>& QPmRecipeWidget::getPMRecipeConfigMap()
	{
		Q_D(QPmRecipeWidget);
		return d->pmRecipeConfigMap;
	}

	// 删除选中项
	void QPmRecipeWidget::onDeleteTheSelectedItem()
	{
		Q_D(QPmRecipeWidget);
		QTableWidget* currentTable = d->getCurrentTableWidget();
		if (!currentTable)
			return;
		
		int selectedRow = currentTable->currentRow();
		if (selectedRow >= 0) {
			currentTable->removeRow(selectedRow);
			int pmIndex = d->ui->tabWidget->currentIndex();
			if (auto spb = qobject_cast<QSpinBox*>(d->ui->pm_cavity_param_edit_tbw->cellWidget(pmIndex, 3))) {
				spb->setValue(currentTable->rowCount());
			}
		}
	}
	
		// 清空数据
	void QPmRecipeWidget::onClearParameters()
	{
		Q_D(QPmRecipeWidget);
		for (int i = 0; i < 4; ++i) {
			if (auto table = d->getIndexTableWidget(i)) {
				table->setRowCount(0);
			}
			if (auto dsb = qobject_cast<QDoubleSpinBox*>(d->ui->pm_cavity_param_edit_tbw->cellWidget(i, 1))) dsb->setValue(0.0);
			if (auto cbx = qobject_cast<QComboBox*>(d->ui->pm_cavity_param_edit_tbw->cellWidget(i, 2))) cbx->setCurrentIndex(0);
			if (auto spb = qobject_cast<QSpinBox*>(d->ui->pm_cavity_param_edit_tbw->cellWidget(i, 3))) spb->setValue(0);
			if (auto dsb4 = qobject_cast<QDoubleSpinBox*>(d->ui->pm_cavity_param_edit_tbw->cellWidget(i, 4))) dsb4->setValue(0.0);
			if (auto dsb5 = qobject_cast<QDoubleSpinBox*>(d->ui->pm_cavity_param_edit_tbw->cellWidget(i, 5))) dsb5->setValue(0.0);
			if (auto dsb6 = qobject_cast<QDoubleSpinBox*>(d->ui->pm_cavity_param_edit_tbw->cellWidget(i, 6))) dsb6->setValue(0.0);
		}
	}
	
	void QPmRecipeWidget::onLoadParameters()
	{
		Q_D(QPmRecipeWidget);

		// 打开JSON文件
		QFile file("./config/pm_recipe_parameters.json");
		if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
			QMessageBox::warning(nullptr, QStringLiteral("load failed"), QStringLiteral("Unable to open file for loading"));
			return;
		}

		// 读取JSON数据
		QByteArray jsonData = file.readAll();
		file.close();

		// 解析JSON文档
		QJsonDocument doc = QJsonDocument::fromJson(jsonData);
		if (doc.isNull() || !doc.isObject()) {
			QMessageBox::warning(nullptr, QStringLiteral("load failed"), QStringLiteral("JSON file format error"));
			return;
		}

		// 新格式：params + motors
		QJsonObject rootObj = doc.object();
		d->pmRecipeConfigMap.clear();
		for (int i = 0; i < 4; ++i) 
		{
			std::string pmName = "PM" + std::to_string(i + 1);
			QString pmNameStr = QString::fromStdString(pmName);
			QPmRecipeWidget::PMRecipeConfig cfg;
			if (rootObj.contains(pmNameStr) && rootObj[pmNameStr].isObject()) 
			{
				QJsonObject pmObj = rootObj[pmNameStr].toObject();
				if (pmObj.contains("params") && pmObj["params"].isObject()) {
					QJsonObject p = pmObj["params"].toObject();
					cfg.params.take_position_mm = p["take_position_mm"].toDouble();
					cfg.params.rotation_angle_deg = p["rotation_angle_deg"].toInt();
					cfg.params.process_count = p["process_count"].toInt();
					cfg.params.rotate_position_mm = p["rotate_position_mm"].toDouble();
					cfg.params.process_position_mm = p["process_position_mm"].toDouble();
					cfg.params.process_time_min = p["process_time_min"].toDouble();
				}
				if (pmObj.contains("motors") && pmObj["motors"].isArray())
				{
					QJsonArray arr = pmObj["motors"].toArray();
					for (int k = 0; k < arr.size(); ++k) {
						auto obj = arr[k].toObject();
						QPmRecipeWidget::PMMotorRow row;
						row.lifting_axis_acce1 = obj["lifting_axis_acce1"].toDouble();
						row.lifting_axis_acce2 = obj["lifting_axis_acce2"].toDouble();
						row.lifting_axis_acce3 = obj["lifting_axis_acce3"].toDouble();
						row.lifting_axis_acce4 = obj["lifting_axis_acce4"].toDouble();
						row.rotating_axis_acce1 = obj["rotating_axis_acce1"].toDouble();
						row.rotating_axis_acce2 = obj["rotating_axis_acce2"].toDouble();
						row.rotating_axis_acce3 = obj["rotating_axis_acce3"].toDouble();
						row.rotating_axis_acce4 = obj["rotating_axis_acce4"].toDouble();
						cfg.motors.push_back(row);
					}
				}
			}
		// 对齐 motors 行数到 process_count
		if (cfg.params.process_count > (int)cfg.motors.size()) {
			while ((int)cfg.motors.size() < cfg.params.process_count) {
				cfg.motors.push_back(QPmRecipeWidget::PMMotorRow{});
			}
		} else if (cfg.params.process_count < (int)cfg.motors.size()) {
			cfg.motors.resize(cfg.params.process_count);
		}
		// 写入界面：PM腔参数
		if (auto dsb1 = qobject_cast<QDoubleSpinBox*>(d->ui->pm_cavity_param_edit_tbw->cellWidget(i, 1))) dsb1->setValue(cfg.params.take_position_mm);
		if (auto cbx = qobject_cast<QComboBox*>(d->ui->pm_cavity_param_edit_tbw->cellWidget(i, 2))) {
			int idx = cbx->findData(cfg.params.rotation_angle_deg);
			cbx->setCurrentIndex(idx >= 0 ? idx : 0);
		}
		if (auto spb = qobject_cast<QSpinBox*>(d->ui->pm_cavity_param_edit_tbw->cellWidget(i, 3))) spb->setValue(cfg.params.process_count);
		if (auto dsb4 = qobject_cast<QDoubleSpinBox*>(d->ui->pm_cavity_param_edit_tbw->cellWidget(i, 4))) dsb4->setValue(cfg.params.rotate_position_mm);
		if (auto dsb5 = qobject_cast<QDoubleSpinBox*>(d->ui->pm_cavity_param_edit_tbw->cellWidget(i, 5))) dsb5->setValue(cfg.params.process_position_mm);
		if (auto dsb6 = qobject_cast<QDoubleSpinBox*>(d->ui->pm_cavity_param_edit_tbw->cellWidget(i, 6))) dsb6->setValue(cfg.params.process_time_min);

		// 写入界面：电机参数表
		if (auto table = d->getIndexTableWidget(i)) {
			table->setRowCount(0);
			for (int r = 0; r < (int)cfg.motors.size(); ++r) {
				table->insertRow(r);
				addTableWidgetItemDoubleSpinBox(r, 0, 0.0, 100.0, 1, cfg.motors[r].lifting_axis_acce1, 3, table);//[0,100]
				addTableWidgetItemDoubleSpinBox(r, 1, 0.0, 100.0, 1, cfg.motors[r].lifting_axis_acce2, 3, table);
				addTableWidgetItemDoubleSpinBox(r, 2, 0.0, 100.0, 1, cfg.motors[r].lifting_axis_acce3, 3, table);
				addTableWidgetItemDoubleSpinBox(r, 3, 0.0, 100.0, 1, cfg.motors[r].lifting_axis_acce4, 3, table);
				addTableWidgetItemDoubleSpinBox(r, 4, -50.0, 100.0, 1, cfg.motors[r].rotating_axis_acce1, 3, table);//[-50,100]
				addTableWidgetItemDoubleSpinBox(r, 5, -50.0, 100.0, 1, cfg.motors[r].rotating_axis_acce2, 3, table);
				addTableWidgetItemDoubleSpinBox(r, 6, -50.0, 100.0, 1, cfg.motors[r].rotating_axis_acce3, 3, table);
				addTableWidgetItemDoubleSpinBox(r, 7, -50.0, 100.0, 1, cfg.motors[r].rotating_axis_acce4, 3, table);
			}
		}
		d->pmRecipeConfigMap[pmName] = cfg;
	}
		QMessageBox::information(nullptr, QStringLiteral("load success"), QStringLiteral("PM recipe loaded (params + motors)."));
	}

	// 设置参数
	void QPmRecipeWidget::onSetParameters()
	{
		Q_D(QPmRecipeWidget);
		QJsonObject rootObj;
		for (int i = 0; i < 4; ++i) {
			std::string pmName = "PM" + std::to_string(i + 1);
			QJsonObject pmObj;
			QJsonObject params;
			double takePos = 0.0, rotatePos = 0.0, processPos = 0.0, processTime = 0.0;
			int angleDeg = 0, processCount = 0;
			if (auto dsb1 = qobject_cast<QDoubleSpinBox*>(d->ui->pm_cavity_param_edit_tbw->cellWidget(i, 1))) takePos = dsb1->value();
			if (auto cbx = qobject_cast<QComboBox*>(d->ui->pm_cavity_param_edit_tbw->cellWidget(i, 2))) angleDeg = cbx->currentData().toInt();
			if (auto spb = qobject_cast<QSpinBox*>(d->ui->pm_cavity_param_edit_tbw->cellWidget(i, 3))) processCount = spb->value();
			if (auto dsb4 = qobject_cast<QDoubleSpinBox*>(d->ui->pm_cavity_param_edit_tbw->cellWidget(i, 4))) rotatePos = dsb4->value();
			if (auto dsb5 = qobject_cast<QDoubleSpinBox*>(d->ui->pm_cavity_param_edit_tbw->cellWidget(i, 5))) processPos = dsb5->value();
			if (auto dsb6 = qobject_cast<QDoubleSpinBox*>(d->ui->pm_cavity_param_edit_tbw->cellWidget(i, 6))) processTime = dsb6->value();
			params["take_position_mm"] = takePos;
			params["rotation_angle_deg"] = angleDeg;
			params["process_count"] = processCount;
			params["rotate_position_mm"] = rotatePos;
			params["process_position_mm"] = processPos;
			params["process_time_min"] = processTime;
			pmObj["params"] = params;
			QJsonArray motors;
			auto table = d->getIndexTableWidget(i);
			int useRows = max(0, processCount);
			for (int r = 0; r < useRows; ++r) {
				QJsonObject obj;
				auto getVal = [&](int col) -> double {
					if (!table) return 0.0;
					QDoubleSpinBox* dsb = qobject_cast<QDoubleSpinBox*>(table->cellWidget(r, col));
					return dsb ? dsb->value() : 0.0;
				};
				obj["lifting_axis_acce1"] = getVal(0);
				obj["lifting_axis_acce2"] = getVal(1);
				obj["lifting_axis_acce3"] = getVal(2);
				obj["lifting_axis_acce4"] = getVal(3);
				obj["rotating_axis_acce1"] = getVal(4);
				obj["rotating_axis_acce2"] = getVal(5);
				obj["rotating_axis_acce3"] = getVal(6);
				obj["rotating_axis_acce4"] = getVal(7);
				motors.append(obj);
			}
			pmObj["motors"] = motors;
			rootObj[QString::fromStdString(pmName)] = pmObj;
		}
		QJsonDocument doc(rootObj);
		QFile file("./config/pm_recipe_parameters.json");
		if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
			QByteArray out = doc.toJson(QJsonDocument::Indented);
			out = out.replace("\n", "\r\n");
			file.write(out);
			file.close();
			QMessageBox::information(nullptr, QStringLiteral("save success"), QStringLiteral("PM recipe parameters saved (CRLF)."));
		} else {
			QMessageBox::warning(nullptr, QStringLiteral("save failed"), QStringLiteral("Unable to open file for saving."));
		}
	}

	void QPmRecipeWidget::addTableWidgetItemDoubleSpinBox(int row, int column, double min_value, double max_value, double single_step, 
		double value, int decimals_value,QTableWidget* table)
	{
		Q_D(QPmRecipeWidget);
		QTableWidget* target = table ? table : d->getCurrentTableWidget();
		if (!target)
			return;
		QDoubleSpinBox* dsb = new QDoubleSpinBox();
		dsb->setMaximum(max_value);
		dsb->setMinimum(min_value);
		dsb->setDecimals(decimals_value);
		dsb->setSingleStep(single_step);
		dsb->setValue(value);
		target->setCellWidget(row, column, dsb);
	}

	void QPmRecipeWidget::onStartCycle()
	{
		Q_D(QPmRecipeWidget);
		if (d->isRunning) {
			QMessageBox::warning(this, "Warning", "Cycle is already running!");
			return;
		}

		// Save current parameters first to ensure we have latest data
		onSetParameters();

		int selectedIndex = d->ui->pm_selecte_cbx->currentIndex();
		std::string pmName = "PM" + std::to_string(selectedIndex + 1);

		if (d->pmRecipeConfigMap.find(pmName) == d->pmRecipeConfigMap.end() || d->pmRecipeConfigMap[pmName].params.process_count <= 0) 
		{
			QMessageBox::warning(this, "Warning", "No process parameters for " + QString::fromStdString(pmName));
			return;
		}

		d->isRunning = true;
		d->stopRequested = false;
		d->currentCycleCount = 0;
		d->ui->start_pm_pbt->setEnabled(false);
		d->ui->stop_pm_pbt->setEnabled(true);


		// Sequence: Z3 -> 执行process_count次（Z2 -> Rotate -> Z1 -> 停留一段工艺时间）-> 执行完总工艺时间->  Z3

		d->cycleThread = std::thread([=](){
			auto pmSubsystem = d->kernel->getKernelModule<FortrendPMCavitySubsystem>(pmName);
			if (!pmSubsystem) {
				// Log error?
				d->isRunning = false;
				return;
			}
			const auto& cfg = d->pmRecipeConfigMap[pmName];
			double z1 = cfg.params.process_position_mm; //工艺面
			double z2 = cfg.params.rotate_position_mm;  //旋转面
			double z3 = cfg.params.take_position_mm;    //取放片面
			int rotation_angle_deg = cfg.params.rotation_angle_deg;

			double atProcessPosMinutes = cfg.params.process_time_min / max(1, cfg.params.process_count); //eg:  15/6 = 2.5min

			if (!pmSubsystem->getMinimumPlaneLevelSignal())
			{
				auto cmdToZ3 = pmSubsystem->createLiftingActionCommand(z3);
				pmSubsystem->startCommand(cmdToZ3);
				cmdToZ3->wait();
				if (cmdToZ3->hasError() || (d->stopRequested))
				{
					/*logFailedExcuteCommandHasError(pmSubsystem->getName(), "移动到取放片面","测试pm流程");*/
					return;
				}
			}

			try {
				for (int idx = 0; idx < cfg.params.process_count; ++idx) 
				{
					if (d->stopRequested) break;
					
					// Update UI
					QMetaObject::invokeMethod(d->ui->pm1_spx, "setValue", Q_ARG(int, idx + 1));

					if (!pmSubsystem->getRotatingimumPlaneLevelSignal())
					{
						// Move to Z2
						auto cmdToZ2 = pmSubsystem->createLiftingActionCommand(z2);
						pmSubsystem->startCommand(cmdToZ2);
						cmdToZ2->wait();
						if (cmdToZ2->hasError() || (d->stopRequested))
						{
							logFailedExcuteCommandHasError(pmSubsystem->getName(), "移动到旋转面", "测试pm流程");
							break;
						}
						logInform(pmSubsystem->getName().c_str(), "第:%d次，移动到旋转面---------------", idx);
					}
						
					// Rotate at Z2
					auto cmdRotate = pmSubsystem->createRotatingActionCommand(rotation_angle_deg);
					pmSubsystem->startCommand(cmdRotate);
					cmdRotate->wait();

					if (cmdRotate->hasError() || (d->stopRequested))
					{
						logFailedExcuteCommandHasError(pmSubsystem->getName(), "执行旋转", "测试pm流程");
						break;
					}
					logInform(pmSubsystem->getName().c_str(), "第:%d次，执行旋转中---------------", idx);

					// Move to Z1 (Process)

					if (!pmSubsystem->getMaximumPlaneLevelSignal())
					{
						auto cmdToZ1 = pmSubsystem->createLiftingActionCommand(z1);
						pmSubsystem->startCommand(cmdToZ1);
						cmdToZ1->wait();
						if (cmdToZ1->hasError() || (d->stopRequested))
						{
							logFailedExcuteCommandHasError(pmSubsystem->getName(), "上升到工艺面", "测试pm流程");
							break;
						}
					}

					logInform(pmSubsystem->getName().c_str(), "第:%d次，执行上升到工艺面完成---------------", idx);

					logInform(pmSubsystem->getName().c_str(), "第:%d次，执行工艺开始---------------", idx);
					// Wait (Process Time at Z1) — 计时线程唤醒/停止唤醒
					d->runTimer(atProcessPosMinutes);
					{
						std::unique_lock<std::mutex> lk(d->cycleMutex);
						d->cycleCv.wait(lk, [&]() {
							return d->timerFinished.load() || d->stopRequested.load();
						});
						if (d->stopRequested) break;
					}
					logInform(pmSubsystem->getName().c_str(), "第:%d次，执行工艺结束---------------", idx);

					/*d->processElapsed += atProcessPosMinutes;

					if (d->processElapsed >= cfg.params.process_time_min)
					{
						logInform(pmSubsystem->getName().c_str(), "超过工艺总时间，停止循环");
						break;
					}*/
				}

				// Move back to Z3 (End of cycle)

				if (!pmSubsystem->getMinimumPlaneLevelSignal())
				{
					auto cmdToZ3 = pmSubsystem->createLiftingActionCommand(z3);
					pmSubsystem->startCommand(cmdToZ3);
					cmdToZ3->wait();
					if (cmdToZ3->hasError() || (d->stopRequested))
					{
						//logFailedExcuteCommandHasError(pmSubsystem->getName(), "移动到取放片面", "测试pm流程");
						return;
					}
					else
					{
						logInform(pmSubsystem->getName().c_str(), "整体工艺完成");
					}
				}
			} 
			catch (...) {
				// Handle exceptions
			}

			d->isRunning = false;
			// Restore UI state
			QMetaObject::invokeMethod(d->ui->start_pm_pbt, "setEnabled", Q_ARG(bool, true));
			QMetaObject::invokeMethod(d->ui->stop_pm_pbt, "setEnabled", Q_ARG(bool, false));
		});
		d->cycleThread.detach(); // Detach to let it run independently
	}

	void QPmRecipeWidget::onStopCycle()
	{
		Q_D(QPmRecipeWidget);
		if (d->isRunning) {
			d->stopRequested = true;
			d->cycleCv.notify_all();
			d->ui->stop_pm_pbt->setEnabled(false); // Prevent multiple clicks

			//if (d->processElapsed > 0)
			//{
			//	d->processElapsed = 0.0;
			//}
		}
	}
	
	void QPmRecipeWidget::logFailed(const std::string station_name, const std::string log) {
		Q_D(QPmRecipeWidget);
		logError(station_name.c_str(), log.c_str());
		d->stopRequested = true;
		d->ui->stop_pm_pbt->setEnabled(false);
	}

	void QPmRecipeWidget::logFailedExcuteCommandHasError(const std::string station_name, const std::string command_name, const std::string process_name)
	{
		logFailed(station_name, Poco::format("%s %s命令执行失败， %s", station_name, command_name, process_name));
	}

	void QPmRecipeWidget::onSelectPMChanged(int index)
	{
		Q_D(QPmRecipeWidget);
		d->ui->tabWidget->setCurrentIndex(index);
		d->ui->pm1_spx->setValue(0);
	}


	void QPmRecipeWidget::initPMCavityParamEdieTableWidget()
	{
		Q_D(QPmRecipeWidget);
		addAnPMItem("PM1");
		addAnPMItem("PM2");
		addAnPMItem("PM3");
		addAnPMItem("PM4");
		for (size_t i = 0; i < d->ui->pm_cavity_param_edit_tbw->columnCount(); i++)
		{
			d->ui->pm_cavity_param_edit_tbw->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Stretch);
		}

		d->ui->pm_cavity_param_edit_tbw->setSelectionBehavior(QAbstractItemView::SelectRows);
	}


	void QPmRecipeWidget::addAnPMItem(const QString name)
	{
		Q_D(QPmRecipeWidget);

		int row_count = d->ui->pm_cavity_param_edit_tbw->rowCount();
		d->ui->pm_cavity_param_edit_tbw->insertRow(row_count);

		QTableWidgetItem* item = new QTableWidgetItem;
		item->setText(name);
		item->setFlags(item->flags() & ~Qt::ItemIsEditable);
		d->ui->pm_cavity_param_edit_tbw->setItem(row_count, 0, item);
		addEditTableWidgetItemDoubleSpinBox(row_count, 1, 0.0, 100.0, 1, 100); //电机取放片位置
		addEditTableWidgetItemComboBox(row_count, 2, 1);						//电机旋转角度/°

		QSpinBox* Rotation_count_spx = new QSpinBox();
		Rotation_count_spx->setMinimum(0);
		Rotation_count_spx->setMaximum(6);
		Rotation_count_spx->setSingleStep(1);
		d->ui->pm_cavity_param_edit_tbw->setCellWidget(row_count, 3, Rotation_count_spx);//旋转次数

		addEditTableWidgetItemDoubleSpinBox(row_count, 4, 0.0, 100.0, 1, 100); //电机旋转位置
		addEditTableWidgetItemDoubleSpinBox(row_count, 5, 0.0, 100.0, 1, 120); //电机工艺位置/mm   max height:100mm
		addEditTableWidgetItemDoubleSpinBox(row_count, 6, 0, 20.0, 1, 15.0);	//工艺时间		  max:20min

	}
	// 增加一项
	void QPmRecipeWidget::onAddAnItem()
	{
		Q_D(QPmRecipeWidget);
		QTableWidget* currentTable = d->getCurrentTableWidget();
		if (!currentTable)
			return;
		
		int rowCount = currentTable->rowCount();
		currentTable->insertRow(rowCount);
		// 为新行添加默认值
		for (int col = 0; col < 8; ++col)
		{
			addTableWidgetItemDoubleSpinBox(rowCount, col, 0.0, 100.0, 1, 100);
		}
		int pmIndex = d->ui->tabWidget->currentIndex();
		if (auto spb = qobject_cast<QSpinBox*>(d->ui->pm_cavity_param_edit_tbw->cellWidget(pmIndex, 3))) {
			spb->setValue(currentTable->rowCount());
		}
	}

	void QPmRecipeWidget::addEditTableWidgetItemComboBox(int row, int column, int value)
	{
		Q_D(QPmRecipeWidget);
		QComboBox* dcb = new QComboBox();
		
		QMap<QString, int> AngleMap;
		AngleMap.insert("0",0);
		AngleMap.insert("60", 60);
		AngleMap.insert("120", 120);
		AngleMap.insert("180", 180);
		AngleMap.insert("240", 240);
		AngleMap.insert("300", 300);
		AngleMap.insert("360", 360);

		for (const auto& str : AngleMap.keys()) 
		{
			dcb->addItem(str, AngleMap.value(str));
		}

		dcb->setCurrentIndex(value);

		d->ui->pm_cavity_param_edit_tbw->setCellWidget(row,column,dcb);
	}

	void QPmRecipeWidget::addEditTableWidgetItemDoubleSpinBox(int row, int column, double min_value, double max_value, 
		double single_step, double value, int decimals_value)
	{
		Q_D(QPmRecipeWidget);
		QDoubleSpinBox* dsb = new QDoubleSpinBox();
		dsb->setMaximum(max_value);
		dsb->setMinimum(min_value);
		dsb->setDecimals(decimals_value);
		dsb->setSingleStep(single_step);
		dsb->setValue(value);
		d->ui->pm_cavity_param_edit_tbw->setCellWidget(row, column, dsb);
	}

}
