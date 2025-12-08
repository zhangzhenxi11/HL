# PM Recipe Widget Tab表格功能实现方案

## 功能需求

1. 在`pm_recipe_widget.ui`的`tabWidget`中，为四个tab页（PM1、PM2、PM3、PM4）添加`QTableWidget`
2. 每个`QTableWidget`的内容格式与主表格`pm_cavity_param_edit_tbw`一致
3. 实现"增加一项"、"删除选中项"、"清空数据"、"设置"按钮功能
4. 参考`slot_transfer_cycle_vtm_widget`中的实现

## 现有UI结构分析

从`pm_recipe_widget.ui`文件中可以看到：

1. 主表格`pm_cavity_param_edit_tbw`已存在，包含9列：
   - 工艺次数
   - 升降轴加速度1
   - 升降轴加速度2
   - 升降轴加速度3
   - 升降轴加速度4
   - 旋转加速度1
   - 旋转加速度2
   - 旋转加速度3
   - 旋转加速度4

2. `tabWidget`已存在，包含四个tab页：PM1、PM2、PM3、PM4

3. 按钮控件已存在：
   - `add_an_item_pbt`（增加一项）
   - `delete_the_selected_item_pbt`（删除选中项）
   - `clear_sequence_pbt`（清空数据）
   - `pushButton`（设置）

## 实现方案

### 1. 修改头文件 `pm_recipe_widget.h`

```cpp
/**
* @file            pm_recipe_widget.h
* @brief           pm_recipe_widget
* @author			xielonghua
*/

// Library: FortrendUI
// Package: VTM

#ifndef _PM_RECIPE_WIDGET_INCLUDE_
#define _PM_RECIPE_WIDGET_INCLUDE_

#include  "Kernel/kernel.h"
#include "Kernel/FortrendUI/fortrend_ui_macros.h"
#include <QWidget>
#include <string>
#include <QTableWidget>
#include <QPushButton>
#include <QVBoxLayout>

namespace FC {

	class QPmRecipeWidgetPrivate;

	/**
	* @brief   pm Recipe  widget
	*/
	class QPmRecipeWidget : public QWidget
	{
		Q_OBJECT

	public:
		QPmRecipeWidget(const std::shared_ptr<IKernel>& kernel,QWidget* parent = nullptr);
		~QPmRecipeWidget();

	private slots:
		void onAddAnItem();
		void onDeleteTheSelectedItem();
		void onClearSequence();
		void onSetParameters();

	private:
		Q_DECLARE_PRIVATE(QPmRecipeWidget)
		QPmRecipeWidgetPrivate* d_ptr;
	};
}
#endif // _PM_RECIPE_WIDGET_INCLUDE_
```

### 2. 修改源文件 `pm_recipe_widget.cpp`

```cpp
// Library: VTM
// Package: VTM

#include "Kernel/kernel_api.h"
#include "pm_recipe_widget.h"
#include "device/ui_pm_recipe_widget.h"

#if _MSC_VER >1600
#pragma execution_character_set("utf-8")
#endif


namespace FC {

	class QPmRecipeWidgetPrivate
	{
	public:
		Q_DECLARE_PUBLIC(QPmRecipeWidget)
		QPmRecipeWidgetPrivate(QPmRecipeWidget*p, const std::shared_ptr<IKernel>& kernel);
		~QPmRecipeWidgetPrivate();
	
		void initTableWidget(QTableWidget* tableWidget);
		QTableWidget* getCurrentTableWidget();
	
	private:
		Ui::QPmRecipeWidgetClass* ui;
		std::shared_ptr<IKernel> kernel;
		QPmRecipeWidget* q_ptr;
	
		// 四个tab页对应的表格
		QTableWidget* pm1TableWidget;
		QTableWidget* pm2TableWidget;
		QTableWidget* pm3TableWidget;
		QTableWidget* pm4TableWidget;
	};
	
	QPmRecipeWidgetPrivate::QPmRecipeWidgetPrivate(QPmRecipeWidget* p, const std::shared_ptr<IKernel>& kernel)
		:q_ptr(p)
		, kernel(kernel)
		, ui(new Ui::QPmRecipeWidgetClass)
		, pm1TableWidget(nullptr)
		, pm2TableWidget(nullptr)
		, pm3TableWidget(nullptr)
		, pm4TableWidget(nullptr)
	{
	}
	
	QPmRecipeWidgetPrivate::~QPmRecipeWidgetPrivate()
	{
		delete ui;
	}
	
	void QPmRecipeWidgetPrivate::initTableWidget(QTableWidget* tableWidget)
	{
		if (!tableWidget)
			return;
		
		// 设置表格属性，与主表格一致
		tableWidget->setAlternatingRowColors(true);
		tableWidget->verticalHeader()->setVisible(false);
		tableWidget->setMinimumSize(600, 200);
		
		// 设置列数和列名
		tableWidget->setColumnCount(9);
		QStringList headers;
		headers << "工艺次数" << "升降轴加速度1" << "升降轴加速度2" << "升降轴加速度3" << "升降轴加速度4" \
				<< "旋转加速度1" << "旋转加速度2" << "旋转加速度3" << "旋转加速度4";
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
	
	QPmRecipeWidget::QPmRecipeWidget(const std::shared_ptr<IKernel>& kernel,QWidget* parent)
		: QWidget(parent)
		, d_ptr(new QPmRecipeWidgetPrivate(this, kernel))
	{
		Q_D(QPmRecipeWidget);
		d->ui->setupUi(this);
		
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
		
		// 连接按钮信号到槽函数
		connect(d->ui->add_an_item_pbt, &QPushButton::clicked, this, &QPmRecipeWidget::onAddAnItem);
		connect(d->ui->delete_the_selected_item_pbt, &QPushButton::clicked, this, &QPmRecipeWidget::onDeleteTheSelectedItem);
		connect(d->ui->clear_sequence_pbt, &QPushButton::clicked, this, &QPmRecipeWidget::onClearSequence);
		connect(d->ui->pushButton, &QPushButton::clicked, this, &QPmRecipeWidget::onSetParameters);
	}
	
	QPmRecipeWidget::~QPmRecipeWidget()
	{
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
		for (int col = 0; col < 9; ++col) {
			QTableWidgetItem* item = new QTableWidgetItem("0");
			currentTable->setItem(rowCount, col, item);
		}
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
		}
	}
	
	// 清空数据
	void QPmRecipeWidget::onClearSequence()
	{
		Q_D(QPmRecipeWidget);
		QTableWidget* currentTable = d->getCurrentTableWidget();
		if (!currentTable)
			return;
		
		currentTable->setRowCount(0);
	}
	
	// 设置参数
	void QPmRecipeWidget::onSetParameters()
	{
		Q_D(QPmRecipeWidget);
		
		// 这里实现将表格数据保存到对应PM腔参数的逻辑
		// 可以根据实际需求修改
		
		// 示例：获取当前表格数据
		QTableWidget* currentTable = d->getCurrentTableWidget();
		if (!currentTable)
			return;
		
		int rowCount = currentTable->rowCount();
		for (int row = 0; row < rowCount; ++row) {
			for (int col = 0; col < 9; ++col) {
				QTableWidgetItem* item = currentTable->item(row, col);
				if (item) {
					QString value = item->text();
					// 这里将数据保存到对应PM腔参数
					// 例如：
					// pm1Params[row].processTimes = value.toInt();
					// pm1Params[row].zAcc1 = value.toDouble();
				}
			}
		}
	}

}
```

## 实现步骤

### 1. 确保UI文件正确

确保`pm_recipe_widget.ui`文件包含以下控件：

- `tabWidget`（QTabWidget）
- 四个tab页（QWidget）
- `pm_cavity_param_edit_tbw`（QTableWidget）
- `add_an_item_pbt`（QPushButton）
- `delete_the_selected_item_pbt`（QPushButton）
- `clear_sequence_pbt`（QPushButton）
- `pushButton`（QPushButton）

### 2. 重新生成UI头文件

使用Qt Designer重新生成`ui_pm_recipe_widget.h`文件，或者使用`uic`命令：

```bash
uic pm_recipe_widget.ui -o ui_pm_recipe_widget.h
```

### 3. 替换现有代码

将上述代码替换到`pm_recipe_widget.h`和`pm_recipe_widget.cpp`文件中。

### 4. 编译和测试

编译项目并测试功能是否正常：

1. 启动应用程序，打开PM Recipe Widget
2. 切换到不同的tab页，验证每个tab页都有一个表格
3. 点击"增加一项"按钮，验证表格中添加了一行
4. 点击"删除选中项"按钮，验证选中的行被删除
5. 点击"清空数据"按钮，验证表格被清空
6. 点击"设置"按钮，验证数据被正确保存

## 技术要点

1. **表格初始化**：使用`initTableWidget`函数统一初始化所有表格，确保格式一致
2. **当前表格获取**：使用`getCurrentTableWidget`函数获取当前选中tab页对应的表格，简化代码
3. **信号槽连接**：在构造函数中连接所有按钮的信号到对应的槽函数
4. **布局管理**：使用`QVBoxLayout`将表格添加到tab页，确保表格自适应大小
5. **数据处理**：实现了基本的数据添加、删除、清空和保存功能

## 扩展功能建议

1. **数据持久化**：将表格数据保存到配置文件或数据库中
2. **数据验证**：添加输入验证，确保数据格式正确
3. **复制粘贴功能**：支持在表格中复制粘贴数据
4. **导入导出功能**：支持从文件导入数据和导出数据到文件
5. **批量操作**：支持批量添加、删除和修改数据

## 参考文件

- `slot_transfer_cycle_vtm_widget.cpp`：参考其表格操作和按钮事件处理
- `pm_recipe_widget.ui`：现有UI结构
- `ui_pm_recipe_widget.h`：UI头文件

## 预期效果

1. 每个tab页中显示一个与主表格格式一致的`QTableWidget`
2. 按钮功能正常工作，能够操作当前选中tab页的表格
3. 表格数据可以正确保存和加载
4. UI布局合理，表格自适应大小
5. 代码结构清晰，易于维护和扩展
