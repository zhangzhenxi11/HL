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
#include <map>
#include <vector>
#include "PMCavity/fortrend_pm_cavity_defined.h"
#pragma execution_character_set("utf-8")

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
		
		// 公共方法，用于获取pmMotionProcessData
		std::map<std::string, std::vector<PMMotionProcessParameters>>& getPMMotionProcessData();
	private slots:
		void onStartCycle();
		void onStopCycle();
		void onAddAnItem();
		void onDeleteTheSelectedItem();
		void onClearParameters();
		void onLoadParameters();
		void onSetParameters();

		//初始化PM腔界面
		void initPMCavityParamEdieTableWidget();

		//增加pm项
		void addAnPMItem(const QString name);

		void addTableWidgetItemDoubleSpinBox(int row, int column, double min_value,
			double max_value, double single_step, double value, int decimals_value = 3, QTableWidget* table = nullptr);

		//重载函数
		void addEditTableWidgetItemDoubleSpinBox(int row, int column, double min_value, 
			double max_value, double single_step, double value, int decimals_value = 3);

		void addEditTableWidgetItemComboBox(int row, int column,int value);



	private:
		Q_DECLARE_PRIVATE(QPmRecipeWidget)
		QPmRecipeWidgetPrivate* d_ptr;
	};
}
#endif // _PM_RECIPE_WIDGET_INCLUDE_
