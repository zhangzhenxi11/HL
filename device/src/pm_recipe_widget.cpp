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
#include <QComboBox>
#include <QPushButton>
#include <chrono>
#include <QSpinBox>
#include <QMap>


#if _MSC_VER >1600
#pragma execution_character_set("utf-8")
#endif

#define max(a, b) (((a) > (b)) ? (a) : (b))

#include <QSplitter>
#include <QLineEdit>
#include <QFileDialog>
#include <QTimer>

namespace FC {
	class QPmRecipeWidgetPrivate
	{
	public:
		Q_DECLARE_PUBLIC(QPmRecipeWidget)
		QPmRecipeWidgetPrivate(QPmRecipeWidget*p, const std::shared_ptr<IKernel>& kernel);
		~QPmRecipeWidgetPrivate();
	
		void initSequenceTable(QTableWidget* table);
		void initInnerTable(QTableWidget* table);
		
		QTableWidget* getSequenceTable(int index);
		QTableWidget* getInnerTable(int index);
		void runTimer(double second, int pmIndex);
		
		void highlightSequenceRow(int pmIndex, int row, QColor color);
		void highlightInnerColumn(int pmIndex, int col, QColor color);
		void clearHighlights(int pmIndex);

	private:
		Ui::QPmRecipeWidgetClass* ui;
		std::shared_ptr<IKernel> kernel;
		QPmRecipeWidget* q_ptr;
	
		// 4个PM的表格
		QTableWidget* sequenceTables[4];
		QTableWidget* innerTables[4];
		std::string currentRecipeNames[4]; // 序列表中当前选中的配方

		//pm统一的PM腔的工艺数据
		std::map<std::string, QPmRecipeWidget::PMRecipeConfig> pmRecipeConfigMap;

		std::chrono::steady_clock::time_point start_time;//开始时间点
		const std::chrono::hours timeout = std::chrono::hours(1); //超时时间

		std::string current_pm; //要执行测试的PM
		
		// 删除原有的共享线程控制变量
		// 已移动到QPmRecipeWidget::PMExecutionContext中

		friend class QPmRecipeWidget;
	};

	QPmRecipeWidgetPrivate::QPmRecipeWidgetPrivate(QPmRecipeWidget* p, const std::shared_ptr<IKernel>& kernel)
		:q_ptr(p)
		, kernel(kernel)
		, ui(new Ui::QPmRecipeWidgetClass)
	{
		for(int i=0; i<4; ++i) {
			sequenceTables[i] = nullptr;
			innerTables[i] = nullptr;
		}
	}
	
	QPmRecipeWidgetPrivate::~QPmRecipeWidgetPrivate()
	{
		// PMExecutionContext析构函数会自动处理线程清理
		delete ui;
	}
	
	void QPmRecipeWidgetPrivate::runTimer(double seconds, int pmIndex)
	{
		Q_Q(QPmRecipeWidget);
		if (pmIndex < 0 || pmIndex >= 4) return;
		
		auto& ctx = q->pmContexts[pmIndex];
		ctx.timerFinished.store(false);
		std::thread([&ctx, seconds]() {
			auto ms = std::chrono::milliseconds(static_cast<long long>(seconds * 1000.0));
			std::this_thread::sleep_for(ms);
			ctx.timerFinished.store(true);
			ctx.cycleCv.notify_all();
		}).detach();
	}

	void QPmRecipeWidgetPrivate::highlightSequenceRow(int pmIndex, int row, QColor color)
	{
		QTableWidget* table = getSequenceTable(pmIndex);
		if (!table) return;
		for (int c = 0; c < table->columnCount(); ++c) {
			if (auto item = table->item(row, c)) {
				item->setBackground(color);
			} else {
				// 部件（组合框）没有QTableWidgetItem背景。
				// 我们可以尝试设置部件的样式表。
				if (auto w = table->cellWidget(row, c)) {
					QString style = QString("background-color: %1").arg(color.name());
					QMetaObject::invokeMethod(w, "setStyleSheet", Q_ARG(QString, style));
				}
			}
		}
	}

	void QPmRecipeWidgetPrivate::highlightInnerColumn(int pmIndex, int col, QColor color)
	{
		QTableWidget* table = getInnerTable(pmIndex);
		if (!table) return;
		// 高亮表头
		if (auto item = table->horizontalHeaderItem(col)) {
		}
		
		for (int r = 0; r < table->rowCount(); ++r) {
			if (auto w = table->cellWidget(r, col)) {
				QString style = QString("background-color: %1").arg(color.name());
				QMetaObject::invokeMethod(w, "setStyleSheet", Q_ARG(QString, style));
			}
		}
	}

	void QPmRecipeWidgetPrivate::clearHighlights(int pmIndex)
	{
		QTableWidget* seqTable = getSequenceTable(pmIndex);
		if (seqTable) {
			for (int r = 0; r < seqTable->rowCount(); ++r) {
				highlightSequenceRow(pmIndex, r, Qt::white); // 恢复默认
			}
		}
		QTableWidget* innerTable = getInnerTable(pmIndex);
		if (innerTable) {
			for (int c = 0; c < innerTable->columnCount(); ++c) {
				highlightInnerColumn(pmIndex, c, Qt::white);
			}
		}
	}

	void QPmRecipeWidgetPrivate::initSequenceTable(QTableWidget* table)
	{
		if (!table) return;
		table->setAlternatingRowColors(true);
		table->verticalHeader()->setVisible(true);
		table->verticalHeader()->setDefaultSectionSize(30);
		table->setSelectionBehavior(QAbstractItemView::SelectRows);
		table->setSelectionMode(QAbstractItemView::SingleSelection);
		
		QFont font;
		font.setFamily("SimHei");
		table->horizontalHeader()->setFont(font);
		table->verticalHeader()->setFont(font);
		
		table->setColumnCount(3);
		QStringList headers = { "From", "To", "Recipe" };
		table->setHorizontalHeaderLabels(headers);
		table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	}

	void QPmRecipeWidgetPrivate::initInnerTable(QTableWidget* table)
	{
		if (!table) return;
		table->setAlternatingRowColors(true);
		table->verticalHeader()->setVisible(true);
		table->verticalHeader()->setDefaultSectionSize(30);
		// table->setSelectionBehavior(QAbstractItemView::SelectColumns); // 选择列进行添加/删除？
		
		QFont font;
		font.setFamily("SimHei");
		table->horizontalHeader()->setFont(font);
		table->verticalHeader()->setFont(font);
		
		// 固定行：Z_ACC, Z_DEC, Z_JERK, Z_VEL, R_ACC, R_DEC, R_JERK, R_VEL
		table->setRowCount(8);
		QStringList headers = {
			"Z-Acc", "Z-Dec", "Z-Jerk", "Z-Vel",
			"R-Acc", "R-Dec", "R-Jerk", "R-Vel"
		};
		table->setVerticalHeaderLabels(headers);
	}

	QTableWidget* QPmRecipeWidgetPrivate::getSequenceTable(int index)
	{
		if (index >= 0 && index < 4) return sequenceTables[index];
		return nullptr;
	}

	QTableWidget* QPmRecipeWidgetPrivate::getInnerTable(int index)
	{
		if (index >= 0 && index < 4) return innerTables[index];
		return nullptr;
	}

	// 静态成员变量定义
	QPmRecipeWidget* QPmRecipeWidget::m_instance = nullptr;

	QPmRecipeWidget::QPmRecipeWidget(const std::shared_ptr<IKernel>& kernel,QWidget* parent)
		: QWidget(parent)
		, d_ptr(new QPmRecipeWidgetPrivate(this, kernel))
	{
		Q_D(QPmRecipeWidget);
		d->ui->setupUi(this);
		
		initPMCavityParamEdieTableWidget();

		// Create layouts for each tab
		QWidget* tabs[] = { d->ui->tab, d->ui->tab_2, d->ui->tab_3, d->ui->tab_4 };
		
		for(int i=0; i<4; ++i) {
			QVBoxLayout* layout = new QVBoxLayout(tabs[i]);
			QSplitter* splitter = new QSplitter(Qt::Vertical, tabs[i]);
			layout->addWidget(splitter);

			// 序列表（外层）
			QWidget* seqWidget = new QWidget(splitter);
			QVBoxLayout* seqLayout = new QVBoxLayout(seqWidget);
			d->sequenceTables[i] = new QTableWidget(seqWidget);
			d->initSequenceTable(d->sequenceTables[i]);
			seqLayout->addWidget(new QLabel("Sequence Flow:", seqWidget));
			seqLayout->addWidget(d->sequenceTables[i]);
			splitter->addWidget(seqWidget);

			// 内表（内层）
			QWidget* innerWidget = new QWidget(splitter);
			QVBoxLayout* innerLayout = new QVBoxLayout(innerWidget);
			d->innerTables[i] = new QTableWidget(innerWidget);
			d->initInnerTable(d->innerTables[i]);

			// 内表按钮
			QHBoxLayout* btnLayout = new QHBoxLayout();
			QPushButton* addColBtn = new QPushButton("Add Step", innerWidget);
			QPushButton* delColBtn = new QPushButton("Del Step", innerWidget);
			btnLayout->addWidget(new QLabel("Motor Parameters:", innerWidget));
			btnLayout->addStretch();
			btnLayout->addWidget(addColBtn);
			btnLayout->addWidget(delColBtn);
			innerLayout->addLayout(btnLayout);
			innerLayout->addWidget(d->innerTables[i]);
			splitter->addWidget(innerWidget);

			// 连接信号
			connect(d->sequenceTables[i], &QTableWidget::cellClicked, [this, i](int row, int col) {
				loadRecipeToInnerTable(i, row);
			});
			
			connect(addColBtn, &QPushButton::clicked, [this, i]() {
				addInnerTableColumn(i);
			});
			connect(delColBtn, &QPushButton::clicked, [this, i]() {
				deleteInnerTableColumn(i);
			});
		}

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

	QPmRecipeWidget* QPmRecipeWidget::instance(const std::shared_ptr<IKernel>& kernel, QWidget* parent)
	{
		if (!m_instance) {
			m_instance = new QPmRecipeWidget(kernel, parent);
		}
		return m_instance;
	}
	
	std::map<std::string, QPmRecipeWidget::PMRecipeConfig>& QPmRecipeWidget::getPMRecipeConfigMap()
	{
		Q_D(QPmRecipeWidget);
		return d->pmRecipeConfigMap;
	}

	// 删除选中项 (Sequence Table)
	void QPmRecipeWidget::onDeleteTheSelectedItem()
	{
		Q_D(QPmRecipeWidget);
		QTableWidget* currentTable = d->getSequenceTable(d->ui->tabWidget->currentIndex());
		if (!currentTable) return;
		
		int selectedRow = currentTable->currentRow();
		if (selectedRow >= 0) {
			currentTable->removeRow(selectedRow);
			// 更新数据模型在onSetParameters或显式保存中处理
		}
	}
	
	// 增加一项 (Sequence Table)
	void QPmRecipeWidget::onAddAnItem()
	{
		Q_D(QPmRecipeWidget);
		QTableWidget* currentTable = d->getSequenceTable(d->ui->tabWidget->currentIndex());
		if (!currentTable) return;
		
		int rowCount = currentTable->rowCount();
		currentTable->insertRow(rowCount);
		updateSequenceTableRow(d->ui->tabWidget->currentIndex(), rowCount);
	}
	
	void QPmRecipeWidget::updateSequenceTableRow(int pmIndex, int row)
	{
		Q_D(QPmRecipeWidget);
		QTableWidget* table = d->getSequenceTable(pmIndex);
		if(!table) return;

		// From/To 组合框
		auto createPosCombo = []() {
			QComboBox* cb = new QComboBox();
			cb->addItems({"Transfer", "LiftPin", "Rotate", "Process"});
			return cb;
		};
		
		table->setCellWidget(row, 0, createPosCombo());
		table->setCellWidget(row, 1, createPosCombo());
		
		// 配方名称（可编辑）
		// 使用 QLineEdit 或 QTableWidgetItem
		QTableWidgetItem* item = new QTableWidgetItem("new_recipe");
		table->setItem(row, 2, item);
	}

	void QPmRecipeWidget::loadRecipeToInnerTable(int pmIndex, int row)
	{
		Q_D(QPmRecipeWidget);
		QTableWidget* seqTable = d->getSequenceTable(pmIndex);
		QTableWidget* innerTable = d->getInnerTable(pmIndex);
		if(!seqTable || !innerTable) return;

		// 切换前将当前内表保存到内存？
		// 理想情况下是的，但假设用户点击“保存”以持久化到磁盘。
		// 然而，切换行可能应该将上一行的编辑保存到内存。
		if (!d->currentRecipeNames[pmIndex].empty()) {
			saveInnerTableToRecipe(pmIndex);
		}

		// 获取配方名称
		QTableWidgetItem* item = seqTable->item(row, 2);
		std::string recipeName = item ? item->text().toStdString() : "";
		if (recipeName.empty()) return;

		d->currentRecipeNames[pmIndex] = recipeName;
		std::string pmName = "PM" + std::to_string(pmIndex + 1);
		
		// 查找配方
		auto& recipes = d->pmRecipeConfigMap[pmName].recipes;
		if (recipes.find(recipeName) == recipes.end()) {
			// 如果不存在则创建新配方
			PMRecipeDetails details;
			details.process_count = 1;
			details.motors.resize(1); // 默认1步
			recipes[recipeName] = details;
		}
		
		const auto& details = recipes[recipeName];
		
		// 设置内表列
		innerTable->setColumnCount(details.process_count);
		QStringList headers;
		for(int i=0; i<details.process_count; ++i) headers << QString("Step %1").arg(i+1);
		innerTable->setHorizontalHeaderLabels(headers);
		
		// 填充数据
		for(int c=0; c<details.process_count; ++c) {
			const auto& m = details.motors[c];
			addTableWidgetItemDoubleSpinBox(0, c, 0, 100, 1, m.lifting_acc, 3, innerTable);
			addTableWidgetItemDoubleSpinBox(1, c, 0, 100, 1, m.lifting_dec, 3, innerTable);
			addTableWidgetItemDoubleSpinBox(2, c, 0, 100, 1, m.lifting_jerk, 3, innerTable);
			addTableWidgetItemDoubleSpinBox(3, c, 0, 500, 1, m.lifting_vel, 3, innerTable);
			addTableWidgetItemDoubleSpinBox(4, c, 0, 1000, 1, m.rotating_acc, 3, innerTable);
			addTableWidgetItemDoubleSpinBox(5, c, 0, 1000, 1, m.rotating_dec, 3, innerTable);
			addTableWidgetItemDoubleSpinBox(6, c, 0, 1000, 1, m.rotating_jerk, 3, innerTable);
			addTableWidgetItemDoubleSpinBox(7, c, 0, 360, 1, m.rotating_vel, 3, innerTable);
		}
	}

	void QPmRecipeWidget::saveInnerTableToRecipe(int pmIndex)
	{
		Q_D(QPmRecipeWidget);
		std::string pmName = "PM" + std::to_string(pmIndex + 1);
		std::string recipeName = d->currentRecipeNames[pmIndex];
		if (recipeName.empty()) return;
		
		QTableWidget* table = d->getInnerTable(pmIndex);
		if(!table) return;

		PMRecipeDetails details;
		details.process_count = table->columnCount();
		
		for(int c=0; c<details.process_count; ++c) {
			PMMotorRow row;
			auto getVal = [&](int r) {
				QDoubleSpinBox* dsb = qobject_cast<QDoubleSpinBox*>(table->cellWidget(r, c));
				return dsb ? dsb->value() : 0.0;
			};
			row.lifting_acc = getVal(0);
			row.lifting_dec = getVal(1);
			row.lifting_jerk = getVal(2);
			row.lifting_vel = getVal(3);
			row.rotating_acc = getVal(4);
			row.rotating_dec = getVal(5);
			row.rotating_jerk = getVal(6);
			row.rotating_vel = getVal(7);
			details.motors.push_back(row);
		}
		
		d->pmRecipeConfigMap[pmName].recipes[recipeName] = details;
	}

	void QPmRecipeWidget::addInnerTableColumn(int pmIndex)
	{
		Q_D(QPmRecipeWidget);
		QTableWidget* table = d->getInnerTable(pmIndex);
		if(!table) return;
		
		int col = table->columnCount();
		table->insertColumn(col);
		QString header = QString("Step %1").arg(col + 1);
		table->setHorizontalHeaderItem(col, new QTableWidgetItem(header));
		
		for(int r=0; r<8; ++r) {
			addTableWidgetItemDoubleSpinBox(r, col, 0, 1000, 1, 10, 3, table);
		}
		
		// 立即更新映射？还是等待保存？
		// 最好更新内存结构
		saveInnerTableToRecipe(pmIndex);
	}

	void QPmRecipeWidget::deleteInnerTableColumn(int pmIndex)
	{
		Q_D(QPmRecipeWidget);
		QTableWidget* table = d->getInnerTable(pmIndex);
		if(!table) return;
		
		int col = table->currentColumn();
		if (col >= 0) {
			table->removeColumn(col);
			saveInnerTableToRecipe(pmIndex);
		}
	}

		// 清空数据
	void QPmRecipeWidget::onClearParameters()
	{
		Q_D(QPmRecipeWidget);
		for (int i = 0; i < 4; ++i) {
			if (auto table = d->getSequenceTable(i)) table->setRowCount(0);
			if (auto table = d->getInnerTable(i)) table->setColumnCount(0);
			d->currentRecipeNames[i].clear();
			
			// 清空全局参数
			if (auto dsb = qobject_cast<QDoubleSpinBox*>(d->ui->pm_cavity_param_edit_tbw->cellWidget(i, 1))) dsb->setValue(0.0);
			// ... (清空其他)
		}
	}
	
	void QPmRecipeWidget::onLoadParameters()
	{
		Q_D(QPmRecipeWidget);

		// 打开JSON文件
		QString fileName = QFileDialog::getOpenFileName(this, QStringLiteral("Load Parameters"), "./config", QStringLiteral("JSON Files (*.json)"));
		if (fileName.isEmpty()) return;

		QFile file(fileName);
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

		// 新格式
		QJsonObject rootObj = doc.object();
		d->pmRecipeConfigMap.clear();
		for (int i = 0; i < 4; ++i) 
		{
			std::string pmName = "PM" + std::to_string(i + 1);
			QPmRecipeWidget::PMRecipeConfig cfg;
			
			if (rootObj.contains(QString::fromStdString(pmName)) && rootObj[QString::fromStdString(pmName)].isObject()) 
			{
				QJsonObject pmObj = rootObj[QString::fromStdString(pmName)].toObject();
				
				// 参数
				if (pmObj.contains("params") && pmObj["params"].isObject()) {
					QJsonObject p = pmObj["params"].toObject();
					cfg.params.take_position_mm = p["take_position_mm"].toDouble();
					cfg.params.rotation_angle_deg = p["rotation_angle_deg"].toInt();
					cfg.params.process_count = p["process_count"].toInt();
					cfg.params.rotate_position_mm = p["rotate_position_mm"].toDouble();
					cfg.params.process_position_mm = p["process_position_mm"].toDouble();
					cfg.params.process_time_min = p["process_time_min"].toDouble();
					cfg.params.last_process_time_s = p["last_process_time_s"].toDouble();
					cfg.params.lift_pin_position_mm = p["lift_pin_position_mm"].toDouble();
				}
				
				// 步骤
				if (pmObj.contains("steps") && pmObj["steps"].isArray()) {
					QJsonArray arr = pmObj["steps"].toArray();
					for(const auto& val : arr) {
						QJsonObject obj = val.toObject();
						PMStep step;
						step.from_pos = obj["from"].toString().toStdString();
						step.to_pos = obj["to"].toString().toStdString();
						step.recipe_name = obj["recipe"].toString().toStdString();
						cfg.steps.push_back(step);
					}
				}
				
				// 配方
				if (pmObj.contains("recipes") && pmObj["recipes"].isObject()) {
					QJsonObject recipesObj = pmObj["recipes"].toObject();
					for(auto it = recipesObj.begin(); it != recipesObj.end(); ++it) {
						std::string rName = it.key().toStdString();
						QJsonObject rObj = it.value().toObject();
						PMRecipeDetails details;
						details.process_count = rObj["process_count"].toInt();
						
						if (rObj.contains("motors") && rObj["motors"].isArray()) {
							QJsonArray mArr = rObj["motors"].toArray();
							for(const auto& mVal : mArr) {
								QJsonObject mObj = mVal.toObject();
								PMMotorRow row;
								row.lifting_acc = mObj["lifting_acc"].toDouble();
								row.lifting_dec = mObj["lifting_dec"].toDouble();
								row.lifting_jerk = mObj["lifting_jerk"].toDouble();
								row.lifting_vel = mObj["lifting_vel"].toDouble();
								row.rotating_acc = mObj["rotating_acc"].toDouble();
								row.rotating_dec = mObj["rotating_dec"].toDouble();
								row.rotating_jerk = mObj["rotating_jerk"].toDouble();
								row.rotating_vel = mObj["rotating_vel"].toDouble();
								details.motors.push_back(row);
							}
						}
						if (details.process_count > (int)details.motors.size()) {
							details.motors.resize(details.process_count);
						}
						cfg.recipes[rName] = details;
					}
				}
			}

			d->pmRecipeConfigMap[pmName] = cfg;
			
			// 更新UI全局参数
			if (auto dsb1 = qobject_cast<QDoubleSpinBox*>(d->ui->pm_cavity_param_edit_tbw->cellWidget(i, 1))) dsb1->setValue(cfg.params.take_position_mm);
			if (auto dsb2 = qobject_cast<QDoubleSpinBox*>(d->ui->pm_cavity_param_edit_tbw->cellWidget(i, 2))) dsb2->setValue(cfg.params.lift_pin_position_mm);
			if (auto cbx = qobject_cast<QComboBox*>(d->ui->pm_cavity_param_edit_tbw->cellWidget(i, 3))) {
				int idx = cbx->findData(cfg.params.rotation_angle_deg);
				cbx->setCurrentIndex(idx >= 0 ? idx : 0);
			}
			if (auto spb = qobject_cast<QSpinBox*>(d->ui->pm_cavity_param_edit_tbw->cellWidget(i, 4))) spb->setValue(cfg.params.process_count);
			if (auto dsb5 = qobject_cast<QDoubleSpinBox*>(d->ui->pm_cavity_param_edit_tbw->cellWidget(i, 5))) dsb5->setValue(cfg.params.rotate_position_mm);
			if (auto dsb6 = qobject_cast<QDoubleSpinBox*>(d->ui->pm_cavity_param_edit_tbw->cellWidget(i, 6))) dsb6->setValue(cfg.params.process_position_mm);
			if (auto dsb7 = qobject_cast<QDoubleSpinBox*>(d->ui->pm_cavity_param_edit_tbw->cellWidget(i, 7))) dsb7->setValue(cfg.params.process_time_min);
						
			// 触发计算
			updateProcessTimeDistribution(i);

			// 更新序列表
			if (auto table = d->getSequenceTable(i)) {
				table->setRowCount(0);
				for(size_t s=0; s<cfg.steps.size(); ++s) {
					table->insertRow(s);
					updateSequenceTableRow(i, s);
					
					auto setCombo = [&](int col, const std::string& val) {
						QComboBox* cb = qobject_cast<QComboBox*>(table->cellWidget(s, col));
						if(cb) cb->setCurrentText(QString::fromStdString(val));
					};
					setCombo(0, cfg.steps[s].from_pos);
					setCombo(1, cfg.steps[s].to_pos);
					
					if(table->item(s, 2)) table->item(s, 2)->setText(QString::fromStdString(cfg.steps[s].recipe_name));
				}
			}
			
			// 清空内表
			if (auto table = d->getInnerTable(i)) {
				table->setColumnCount(0);
				d->currentRecipeNames[i].clear();
			}
		}
		QMessageBox::information(nullptr, QStringLiteral("load success"), QStringLiteral("PM recipe loaded (New Format)."));
	}

	// 设置参数
	void QPmRecipeWidget::onSetParameters()
	{
		Q_D(QPmRecipeWidget);
		QJsonObject rootObj;
		for (int i = 0; i < 4; ++i) {
			std::string pmName = "PM" + std::to_string(i + 1);
			
			// 先保存当前内表
			if (!d->currentRecipeNames[i].empty()) {
				saveInnerTableToRecipe(i);
			}
			
			auto& cfg = d->pmRecipeConfigMap[pmName];
			
			if (auto dsb1 = qobject_cast<QDoubleSpinBox*>(d->ui->pm_cavity_param_edit_tbw->cellWidget(i, 1))) cfg.params.take_position_mm = dsb1->value();
			if (auto dsb7 = qobject_cast<QDoubleSpinBox*>(d->ui->pm_cavity_param_edit_tbw->cellWidget(i, 2))) cfg.params.lift_pin_position_mm = dsb7->value();
			if (auto cbx = qobject_cast<QComboBox*>(d->ui->pm_cavity_param_edit_tbw->cellWidget(i, 3))) cfg.params.rotation_angle_deg = cbx->currentData().toInt();
			if (auto spb = qobject_cast<QSpinBox*>(d->ui->pm_cavity_param_edit_tbw->cellWidget(i, 4))) cfg.params.process_count = spb->value();
			if (auto dsb4 = qobject_cast<QDoubleSpinBox*>(d->ui->pm_cavity_param_edit_tbw->cellWidget(i, 5))) cfg.params.rotate_position_mm = dsb4->value();
			if (auto dsb5 = qobject_cast<QDoubleSpinBox*>(d->ui->pm_cavity_param_edit_tbw->cellWidget(i, 6))) cfg.params.process_position_mm = dsb5->value();
			if (auto dsb6 = qobject_cast<QDoubleSpinBox*>(d->ui->pm_cavity_param_edit_tbw->cellWidget(i, 7))) cfg.params.process_time_min = dsb6->value();
			if (auto dsb8 = qobject_cast<QDoubleSpinBox*>(d->ui->pm_cavity_param_edit_tbw->cellWidget(i, 8))) cfg.params.last_process_time_s = dsb8->value();

			// 步骤
			cfg.steps.clear();
			if (auto table = d->getSequenceTable(i)) {
				for(int r=0; r<table->rowCount(); ++r) {
					PMStep step;
					if(auto cb = qobject_cast<QComboBox*>(table->cellWidget(r, 0))) step.from_pos = cb->currentText().toStdString();
					if(auto cb = qobject_cast<QComboBox*>(table->cellWidget(r, 1))) step.to_pos = cb->currentText().toStdString();
					if(table->item(r, 2)) step.recipe_name = table->item(r, 2)->text().toStdString();
					
					// 校验：From == To
					if (step.from_pos == step.to_pos && step.from_pos != "Rotate") {
						QMessageBox::warning(this, "Warning", QString("Row %1: 'From' and 'To' cannot be same (except Rotate).").arg(r + 1));
						return;
					}

					// 校验：Process -> Rotate (检查工艺次数一致性)
					if (step.from_pos == "Process" && step.to_pos == "Rotate") {
						if (cfg.recipes.count(step.recipe_name)) {
							int recipeCols = cfg.recipes[step.recipe_name].motors.size();
							if (recipeCols != cfg.params.process_count) {
								QMessageBox::warning(this, "Warning", 
									QString("Row %1 (Process->Rotate): Recipe '%2' steps (%3) must match Global Process Count (%4).")
									.arg(r + 1).arg(QString::fromStdString(step.recipe_name)).arg(recipeCols).arg(cfg.params.process_count));
								return;
							}
						}
					}

					cfg.steps.push_back(step);
				}
			}

			QJsonObject pmObj;
			
			QJsonObject params;
			params["take_position_mm"] = cfg.params.take_position_mm;
			params["rotation_angle_deg"] = cfg.params.rotation_angle_deg;
			params["process_count"] = cfg.params.process_count;
			params["rotate_position_mm"] = cfg.params.rotate_position_mm;
			params["process_position_mm"] = cfg.params.process_position_mm;
			params["process_time_min"] = cfg.params.process_time_min;
			params["last_process_time_s"] = cfg.params.last_process_time_s;
			params["lift_pin_position_mm"] = cfg.params.lift_pin_position_mm;
			pmObj["params"] = params;
			
			QJsonArray stepsArr;
			for(const auto& s : cfg.steps) {
				QJsonObject sObj;
				sObj["from"] = QString::fromStdString(s.from_pos);
				sObj["to"] = QString::fromStdString(s.to_pos);
				sObj["recipe"] = QString::fromStdString(s.recipe_name);
				stepsArr.append(sObj);
			}
			pmObj["steps"] = stepsArr;
			
			QJsonObject recipesObj;
			for(auto it = cfg.recipes.begin(); it != cfg.recipes.end(); ++it) {
				QJsonObject rObj;
				rObj["process_count"] = it->second.process_count;
				QJsonArray mArr;
				for(const auto& m : it->second.motors) {
					QJsonObject mObj;
					mObj["lifting_acc"] = m.lifting_acc;
					mObj["lifting_dec"] = m.lifting_dec;
					mObj["lifting_jerk"] = m.lifting_jerk;
					mObj["lifting_vel"] = m.lifting_vel;
					mObj["rotating_acc"] = m.rotating_acc;
					mObj["rotating_dec"] = m.rotating_dec;
					mObj["rotating_jerk"] = m.rotating_jerk;
					mObj["rotating_vel"] = m.rotating_vel;
					mArr.append(mObj);
				}
				rObj["motors"] = mArr;
				recipesObj[QString::fromStdString(it->first)] = rObj;
			}
			pmObj["recipes"] = recipesObj;
			
			rootObj[QString::fromStdString(pmName)] = pmObj;
		}
		
		QString fileName = QFileDialog::getSaveFileName(this, QStringLiteral("Save Parameters"), "./config/pm_recipe_parameters.json", QStringLiteral("JSON Files (*.json)"));
		if (fileName.isEmpty()) return;

		QJsonDocument doc(rootObj);
		QFile file(fileName);
		if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
			QByteArray out = doc.toJson(QJsonDocument::Indented);
			out = out.replace("\n", "\r\n");
			file.write(out);
			file.close();
			QMessageBox::information(nullptr, QStringLiteral("save success"), QStringLiteral("PM recipe parameters saved (New Format)."));
		} else {
			QMessageBox::warning(nullptr, QStringLiteral("save failed"), QStringLiteral("Unable to open file for saving."));
		}
	}

	void QPmRecipeWidget::addTableWidgetItemDoubleSpinBox(int row, int column, double min_value, double max_value, 
		double single_step, double value, int decimals_value,QTableWidget* table)
	{
		Q_D(QPmRecipeWidget);
		QTableWidget* target = table;
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

	void QPmRecipeWidget::updateSequenceRowHighlight(int pmIndex, int row, QColor color)
	{
		Q_D(QPmRecipeWidget);
		d->highlightSequenceRow(pmIndex, row, color);
	}

	void QPmRecipeWidget::updateInnerColumnHighlight(int pmIndex, int col, QColor color)
	{
		Q_D(QPmRecipeWidget);
		d->highlightInnerColumn(pmIndex, col, color);
	}

	void QPmRecipeWidget::doLoadRecipeToInnerTable(int pmIndex, int row)
	{
		loadRecipeToInnerTable(pmIndex, row);
	}

	void QPmRecipeWidget::onCycleStoppedState()
	{
		Q_D(QPmRecipeWidget);
		d->ui->start_pm_pbt->setEnabled(true);
		d->ui->stop_pm_pbt->setEnabled(false);
	}

	void QPmRecipeWidget::addTableWidgetItemSpinBox(int row, int column, int min_value, int max_value,
		int single_step, int value, QTableWidget* table)
	{

		Q_D(QPmRecipeWidget);
		QTableWidget* target = table;
		if (!target)
			return;
		QSpinBox* dsb = new QSpinBox();
		dsb->setMaximum(max_value);
		dsb->setMinimum(min_value);
		dsb->setSingleStep(single_step);
		dsb->setValue(value);
		target->setCellWidget(row, column, dsb);
	}



	void QPmRecipeWidget::onStartCycle()
	{
		Q_D(QPmRecipeWidget);
		int pmIndex = d->ui->pm_selecte_cbx->currentIndex();
		
		// 检查该PM是否正在运行
		if (pmContexts[pmIndex].isRunning) {
			QMessageBox::warning(this, "Warning", "This PM cycle is already running!");
			return;
		}

		startPmMotorRun(pmIndex);
	}

	void QPmRecipeWidget::startPmMotorRun(int pmIndex)
	{
		Q_D(QPmRecipeWidget);
		
		if (pmIndex < 0 || pmIndex >= 4) {
			QMessageBox::warning(this, "Warning", "Invalid PM Index");
			return;
		}

		std::string pmName = "PM" + std::to_string(pmIndex + 1);

		if (d->pmRecipeConfigMap.find(pmName) == d->pmRecipeConfigMap.end())
		{
			QMessageBox::warning(this, "Warning", "No configuration for " + QString::fromStdString(pmName));
			return;
		}

		const auto& cfg = d->pmRecipeConfigMap[pmName];
		if (cfg.steps.empty()) {
			QMessageBox::warning(this, "Warning", "No steps defined for " + QString::fromStdString(pmName));
			return;
		}

		// 获取该PM的独立执行上下文
		auto& ctx = pmContexts[pmIndex];
		
		// 再次检查是否已经在运行
		if (ctx.isRunning) {
			QMessageBox::warning(this, "Warning", QString::fromStdString(pmName) + " is already running!");
			return;
		}

		ctx.isRunning = true;
		ctx.stopRequested = false;
		d->ui->start_pm_pbt->setEnabled(false);
		d->ui->stop_pm_pbt->setEnabled(true);

		// 清除高亮
		d->clearHighlights(pmIndex);

		emit cycleStarted(pmName);

		ctx.cycleThread = std::thread([=, &ctx]() {
			auto pmSubsystem = d->kernel->getKernelModule<FortrendPMCavitySubsystem>(pmName);
			if (!pmSubsystem) {
				ctx.isRunning = false;
				QMetaObject::invokeMethod(this, "cycleStopped");
				return;
			}

			// 定义位置
			std::map<std::string, double> posMap;
			posMap["Transfer"] = cfg.params.take_position_mm;
			posMap["LiftPin"] = cfg.params.lift_pin_position_mm;
			posMap["Rotate"] = cfg.params.rotate_position_mm;
			posMap["Process"] = cfg.params.process_position_mm;

			int rotation_angle_deg = cfg.params.rotation_angle_deg;

			// 计算时间分配
			double totalTime = cfg.params.process_time_min; // 现在单位为秒
			double lastTime = cfg.params.last_process_time_s;
			int processCount = cfg.params.process_count;
			double avgTime = 0.0;
			if (processCount > 1) {
				avgTime = (totalTime - lastTime) / (double)(processCount - 1);
				if (avgTime < 0) avgTime = 0;
			}
			else {
				avgTime = totalTime; // 或者仅作为最后一次时间处理
			}

			try {
				// 初始移动到序列开始？
				// 或者直接开始执行步骤。

				for (size_t sIdx = 0; sIdx < cfg.steps.size(); ++sIdx) {
					if (ctx.stopRequested) break;

					QMetaObject::invokeMethod(this, "updateSequenceRowHighlight", Qt::QueuedConnection, Q_ARG(int, pmIndex),
						Q_ARG(int, sIdx), Q_ARG(QColor, Qt::green));

					const auto& step = cfg.steps[sIdx];
					std::string from = step.from_pos;
					std::string to = step.to_pos;
					std::string rName = step.recipe_name;

					// 运行时校验（阻塞）
					if (from == to && from != "Rotate") {
						logFailed(pmSubsystem->getName(), "Invalid Step: From == To (except Rotate)");

						QMetaObject::invokeMethod(this, "updateSequenceRowHighlight", Qt::QueuedConnection, Q_ARG(int, pmIndex),
							Q_ARG(int, sIdx), Q_ARG(QColor, Qt::red));
						break;
					}

					// 查找配方
					auto rIt = cfg.recipes.find(rName);
					if (rIt == cfg.recipes.end()) {
						logFailed(pmSubsystem->getName(), "Recipe not found: " + rName);
						QMetaObject::invokeMethod(this, "updateSequenceRowHighlight", Qt::QueuedConnection, Q_ARG(int, pmIndex), Q_ARG(int, sIdx), Q_ARG(QColor, Qt::red));
						break;
					}
					const auto& recipe = rIt->second;

					// 加载内表到UI以显示进度（可选但有助于视觉效果）
					QTimer::singleShot(0, this, [=]() {
						loadRecipeToInnerTable(pmIndex, sIdx);
						});

					double targetPos = posMap.count(to) ? posMap[to] : 0.0; //存在取其值，否则是0

					// 记录步骤
					logInform(pmSubsystem->getName().c_str(), "Step %d: %s -> %s (%s)", sIdx + 1, from.c_str(), to.c_str(), rName.c_str());

					// 执行循环（内表列）
					int loopCount = recipe.motors.size();
					for (int i = 0; i < loopCount; ++i) {
						if (ctx.stopRequested) break;

						QMetaObject::invokeMethod(this, "updateInnerColumnHighlight", Qt::QueuedConnection, Q_ARG(int, pmIndex), Q_ARG(int, sIdx), Q_ARG(QColor, Qt::green));
						if (i > 0)
						{
							QMetaObject::invokeMethod(this, "updateInnerColumnHighlight", Qt::QueuedConnection, Q_ARG(int, pmIndex), Q_ARG(int, i - 1), Q_ARG(QColor, Qt::white));
						}
						// 更新UI进度（可选）
						QMetaObject::invokeMethod(d->ui->pm1_spx, "setValue", Q_ARG(int, i + 1));

						const auto& m = recipe.motors[i];

						// 设置电机参数
						pmSubsystem->setPMCavityZAxleAcc(m.lifting_acc);
						pmSubsystem->setPMCavityZAxleDcc(m.lifting_dec);
						pmSubsystem->setPMCavityZAxleJerk((uint32_t)m.lifting_jerk);
						pmSubsystem->setPMCavityAxleSpeed(m.lifting_vel);

						pmSubsystem->setPMCavityRAxleAcc(m.rotating_acc);
						pmSubsystem->setPMCavityRAxleDcc(m.rotating_dec);
						pmSubsystem->setPMCavityRAxleJerk((uint32_t)m.rotating_jerk);
						pmSubsystem->setPMCavityRAxleSpeed(m.rotating_vel);

						// 1. Z轴移动（标准 From!=To）
						if (from != to)
						{
							auto cmd = pmSubsystem->createLiftingActionCommand(targetPos);
							pmSubsystem->startCommand(cmd);
							cmd->wait();
							if (cmd->hasError() || ctx.stopRequested) {
								logFailedExcuteCommandHasError(pmSubsystem->getName(), "Move Z Failed", to.c_str());

								QMetaObject::invokeMethod(this, "updateSequenceRowHighlight", Qt::QueuedConnection, Q_ARG(int, pmIndex), Q_ARG(int, sIdx), Q_ARG(QColor, Qt::red));
								QMetaObject::invokeMethod(this, "updateInnerColumnHighlight", Qt::QueuedConnection, Q_ARG(int, pmIndex), Q_ARG(int, i), Q_ARG(QColor, Qt::red));
								throw std::runtime_error("Move Z Failed");
							}
						}

						// 2. Rotate Action
						//到达旋转面时且是from：Process to:Rotate

						if (from == "Process" && to == "Rotate")
						{
							//if (pmSubsystem->getRotatingimumPlaneLevelSignal()) //检测是否达到了旋转面
							{
								//先旋转一定角度
								auto cmd = pmSubsystem->createRotatingActionCommand(rotation_angle_deg);
								pmSubsystem->startCommand(cmd);
								cmd->wait();
								if (cmd->hasError() || ctx.stopRequested) {
									logFailedExcuteCommandHasError(pmSubsystem->getName(), "Rotate Failed", "");
									QMetaObject::invokeMethod(this, "updateSequenceRowHighlight", Qt::QueuedConnection, Q_ARG(int, pmIndex), Q_ARG(int, sIdx), Q_ARG(QColor, Qt::red));
									QMetaObject::invokeMethod(this, "updateInnerColumnHighlight", Qt::QueuedConnection, Q_ARG(int, pmIndex), Q_ARG(int, i), Q_ARG(QColor, Qt::red));
									throw std::runtime_error("Rotate Failed");
								}
								else
								{
									//再Move Z Process面
									double targetPos = posMap["Process"];
									auto cmd = pmSubsystem->createLiftingActionCommand(targetPos);
									pmSubsystem->startCommand(cmd);
									cmd->wait();
									if (cmd->hasError() || ctx.stopRequested) {
										logFailedExcuteCommandHasError(pmSubsystem->getName(), "Move Z Failed", to.c_str());
										QMetaObject::invokeMethod(this, "updateSequenceRowHighlight", Qt::QueuedConnection, Q_ARG(int, pmIndex), Q_ARG(int, sIdx), Q_ARG(QColor, Qt::red));
										QMetaObject::invokeMethod(this, "updateInnerColumnHighlight", Qt::QueuedConnection, Q_ARG(int, pmIndex), Q_ARG(int, i), Q_ARG(QColor, Qt::red));
										throw std::runtime_error("Move Z Failed");
									}
								}
							}
						}


						// 3. Process Wait  只有from == "Process" && to == "Rotate"的配方且真实达到了Process面，才去做工艺
						if (from == "Process" && to == "Rotate")
						{
							//if (pmSubsystem->getMaximumPlaneLevelSignal())//检测是否达到了工艺面
							{
								double waitTime = 0.0;
								if (i < processCount - 1)
								{
									waitTime = avgTime;
								}
								else
								{
									waitTime = lastTime;
								}

								logInform(pmSubsystem->getName().c_str(), "Process Wait: %.3f s (Step %d/%d)", waitTime, i + 1, loopCount);

								d->runTimer(waitTime, pmIndex);
								{
									std::unique_lock<std::mutex> lk(ctx.cycleMutex);
									ctx.cycleCv.wait(lk, [&]() {
										return ctx.timerFinished.load() || ctx.stopRequested.load();
										});
								}
							}
						}

						QMetaObject::invokeMethod(this, "updateInnerColumnHighlight", Qt::QueuedConnection, Q_ARG(int, pmIndex), Q_ARG(int, i), Q_ARG(QColor, Qt::white));

					}
					QMetaObject::invokeMethod(this, "updateSequenceRowHighlight", Qt::QueuedConnection, Q_ARG(int, pmIndex), Q_ARG(int, sIdx), Q_ARG(QColor, Qt::white));

				}

				logInform(pmSubsystem->getName().c_str(), "Sequence Completed");

			}
			catch (const std::exception& e) {
				// Logged already
			}
			catch (...) {
				logFailed(pmSubsystem->getName(), "Unknown Exception");
			}

			ctx.isRunning = false;
			QMetaObject::invokeMethod(d->ui->start_pm_pbt, "setEnabled", Q_ARG(bool, true));
			QMetaObject::invokeMethod(d->ui->stop_pm_pbt, "setEnabled", Q_ARG(bool, false));

			QMetaObject::invokeMethod(this, "cycleStopped");
			});
		ctx.cycleThread.detach();


	}

	void QPmRecipeWidget::stopPmMotor(int pmIndex)
	{
		if (pmIndex < 0 || pmIndex >= 4) return;
		
		auto& ctx = pmContexts[pmIndex];
		if (ctx.isRunning) {
			ctx.stopRequested = true;
			ctx.cycleCv.notify_all();
		}
	}

	void QPmRecipeWidget::onStopCycle()
	{
		Q_D(QPmRecipeWidget);
		int pmIndex = d->ui->pm_selecte_cbx->currentIndex();
		stopPmMotor(pmIndex);
		
		d->ui->stop_pm_pbt->setEnabled(false); // 防止多次点击
		emit cycleStopped();
	}
	
	void QPmRecipeWidget::logFailed(const std::string station_name, const std::string log) {
		Q_D(QPmRecipeWidget);
		logError(station_name.c_str(), log.c_str());
		//pmIndex范围: 0-3对应PM1-PM4
		if(station_name == "PM1")
		{
			auto& ctx = pmContexts[0];
			if (ctx.isRunning) {
				ctx.stopRequested = true;
			}
		}
		else if(station_name == "PM2")
		{
			auto& ctx = pmContexts[1];
			if (ctx.isRunning) {
				ctx.stopRequested = true;
			}
		}
		else if (station_name == "PM3")
		{
			auto& ctx = pmContexts[2];
			if (ctx.isRunning) {
				ctx.stopRequested = true;
			}
		}
		else
		{
			auto& ctx = pmContexts[3];
			if (ctx.isRunning) {
				ctx.stopRequested = true;
			}
		}
		d->ui->stop_pm_pbt->setEnabled(false);

	}

	void QPmRecipeWidget::logFailedExcuteCommandHasError(const std::string station_name, const std::string command_name, const std::string process_name)
	{
		Q_D(QPmRecipeWidget);
		QMetaObject::invokeMethod(d->ui->start_pm_pbt, "setEnabled", Q_ARG(bool, true));
		QMetaObject::invokeMethod(d->ui->stop_pm_pbt, "setEnabled", Q_ARG(bool, false));

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
		addEditTableWidgetItemDoubleSpinBox(row_count, 1, 0.0, 300.0, 1, 100); //电机取放片位置
		addEditTableWidgetItemDoubleSpinBox(row_count, 2, 0.0, 300.0, 1, 0.0);	//顶针位置
		addEditTableWidgetItemComboBox(row_count, 3, 1);						//电机旋转角度/°

		QSpinBox* Rotation_count_spx = new QSpinBox();
		Rotation_count_spx->setMinimum(0);
		Rotation_count_spx->setMaximum(6);
		Rotation_count_spx->setSingleStep(1);
		d->ui->pm_cavity_param_edit_tbw->setCellWidget(row_count, 4, Rotation_count_spx);//旋转次数

		addEditTableWidgetItemDoubleSpinBox(row_count, 5, 0.0, 300.0, 1, 100); //电机旋转位置
		addEditTableWidgetItemDoubleSpinBox(row_count, 6, 0.0, 300.0, 1, 120); //电机工艺位置/mm   max height:100mm
		addEditTableWidgetItemDoubleSpinBox(row_count, 7, 0, 10000.0, 1, 15.0);	//总工艺时间
		
		addEditTableWidgetItemDoubleSpinBox(row_count, 8, 0.0, 10000.0, 1, 0.0);	//最后一次时间
		addEditTableWidgetItemDoubleSpinBox(row_count, 9, 0.0, 10000.0, 1, 0.0);	//前n-1次时间
		
		// Disable Avg Time Editing
		if(auto dsb = qobject_cast<QDoubleSpinBox*>(d->ui->pm_cavity_param_edit_tbw->cellWidget(row_count, 9))) {
			dsb->setEnabled(false);
		}

		// Connect Signals for Time Calculation
		// Row is row_count.
		// Cols: 3 (Count), 6 (Total Time), 8 (Last Time)
		auto connectSignal = [&](int col) {
			if(auto widget = d->ui->pm_cavity_param_edit_tbw->cellWidget(row_count, col)) {
				if(auto spb = qobject_cast<QSpinBox*>(widget)) {
					connect(spb, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), [this, row_count](int){
						updateProcessTimeDistribution(row_count);
					});
				} else if(auto dsb = qobject_cast<QDoubleSpinBox*>(widget)) {
					connect(dsb, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [this, row_count](double){
						updateProcessTimeDistribution(row_count);
					});
				}
			}
		};
		connectSignal(4);
		connectSignal(7);
		connectSignal(8);
	}

	void QPmRecipeWidget::updateProcessTimeDistribution(int pmIndex)
	{
		Q_D(QPmRecipeWidget);
		// Get Inputs
		int count = 0;
		double totalTime = 0.0;
		double lastTime = 0.0;

		if(auto spb = qobject_cast<QSpinBox*>(d->ui->pm_cavity_param_edit_tbw->cellWidget(pmIndex, 4))) count = spb->value();
		if(auto dsb = qobject_cast<QDoubleSpinBox*>(d->ui->pm_cavity_param_edit_tbw->cellWidget(pmIndex, 7))) totalTime = dsb->value();
		if(auto dsb = qobject_cast<QDoubleSpinBox*>(d->ui->pm_cavity_param_edit_tbw->cellWidget(pmIndex, 8))) lastTime = dsb->value();

		double avgTime = 0.0;
		if (count > 1) {
			avgTime = (totalTime - lastTime) / (double)(count - 1);
			if (avgTime < 0) avgTime = 0;
		} else if (count == 1) {
			// 如果计数为 1，那么“上次时间”理论上应等于“总时间”，或者“总时间”就是唯一的“时间”。
			// 逻辑：如果用户设定了“总时间”，那么“上次时间”是否应该更新？反之亦然？
			// 要求说明：“总时间”是从 TCP 中获取的……“上次时间”是由用户设定的。
			// 如果计数为 1，就没有“n-1”了。平均时间 = 0。
             avgTime = 0.0;
        }

		// Update Avg Time Display
		if(auto dsb = qobject_cast<QDoubleSpinBox*>(d->ui->pm_cavity_param_edit_tbw->cellWidget(pmIndex, 9))) {
			dsb->setValue(avgTime);
		}
	}
	// 增加一项
	// Removed duplicate onAddAnItem (duplicate definition)

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
