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
		void onAddAnItem();
		void onDeleteTheSelectedItem();
		void onClearParameters();
		void onLoadParameters();
		void onSetParameters();
		void addTableWidgetItemDoubleSpinBox(int row, int column, double min_value,
			double max_value, double single_step, double value, int decimals_value = 3);
	private:
		Q_DECLARE_PRIVATE(QPmRecipeWidget)
		QPmRecipeWidgetPrivate* d_ptr;
	};
}
#endif // _PM_RECIPE_WIDGET_INCLUDE_