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
#include "PMCavity/fortrend_pm_cavity_defined.h"
#include <QWidget>
#include <string>
#include <QTableWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <map>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

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
		//单例模式
		static QPmRecipeWidget* instance(const std::shared_ptr<IKernel>& kernel,QWidget* parent = nullptr);
		static QPmRecipeWidget* m_instance;
		
		// 新增：统一 PM 配方数据访问
		struct PMMotorRow {
			double lifting_acc = 0.0;
			double lifting_dec = 0.0;
			double lifting_jerk = 0.0;
			double lifting_vel = 0.0;
			double rotating_acc = 0.0;
			double rotating_dec = 0.0;
			double rotating_jerk = 0.0;
			double rotating_vel = 0.0;
		};

		struct PMRecipeDetails {
			int process_count = 0;
			std::vector<PMMotorRow> motors;
		};

		struct PMStep {
			std::string from_pos;
			std::string to_pos;
			std::string recipe_name;
		};

		struct PMParams {
			double take_position_mm = 0.0;
			double lift_pin_position_mm = 0.0; // Added LiftPin Position
			int process_count = 0; // Added Process Count
			int rotation_angle_deg = 0;
			double rotate_position_mm = 0.0;
			double process_position_mm = 0.0;
			double process_time_min = 0.0;
			double last_process_time_s = 0.0; // Added Last Process Time
		};

		struct PMRecipeConfig {
			PMParams params;
			std::vector<PMStep> steps;
			std::map<std::string, PMRecipeDetails> recipes;
		};

		std::map<std::string, PMRecipeConfig>& getPMRecipeConfigMap();

	signals:
		void cycleStarted(const std::string& pmName);
		void cycleStopped();


	public:
		void startPmMotorRun(int pmIndex);
		void stopPmMotor(int pmIndex); // 修改：支持指定PM索引停止

	private slots:
		void onStartCycle();
		void onStopCycle();
		void onAddAnItem();
		void onDeleteTheSelectedItem();
		void onClearParameters();
		void onLoadParameters();
		void onSetParameters();
		void onSelectPMChanged(int index);

		//初始化PM腔界面
		void initPMCavityParamEdieTableWidget();

		//增加pm项
		void addAnPMItem(const QString name);

		void addTableWidgetItemDoubleSpinBox(int row, int column, double min_value,
			double max_value, double single_step, double value, int decimals_value = 3, QTableWidget* table = nullptr);

		void addTableWidgetItemSpinBox(int row, int column, int min_value,
			int max_value, int single_step, int value, QTableWidget* table = nullptr);

		//重载函数
		void addEditTableWidgetItemDoubleSpinBox(int row, int column, double min_value, 
			double max_value, double single_step, double value, int decimals_value = 3);

		void addEditTableWidgetItemComboBox(int row, int column,int value);

		void logFailed(const std::string station_name, const std::string log);

		void logFailedExcuteCommandHasError(const std::string station_name, const std::string command_name, const std::string process_name);

		// Helpers for new UI logic
		void loadRecipeToInnerTable(int pmIndex, int row);
		void saveInnerTableToRecipe(int pmIndex);
		void addInnerTableColumn(int pmIndex);
		void deleteInnerTableColumn(int pmIndex);
		void copyInnerTableColumn(int pmIndex);      // 新增：复制列
		void pasteInnerTableColumnAsNew(int pmIndex); // 新增：粘贴为新列
		void updateSequenceTableRow(int pmIndex, int row);
		void updateProcessTimeDistribution(int pmIndex);

	private slots:
		void updateSequenceRowHighlight(int pmIndex, int row, QColor color);
		void updateInnerColumnHighlight(int pmIndex, int col, QColor color);
		void doLoadRecipeToInnerTable(int pmIndex, int row);
		void onCycleStoppedState();

	private:
		// PM执行上下文结构体 - 每个PM独立
		struct PMExecutionContext {
			std::thread cycleThread;
			std::atomic<bool> isRunning{false};
			std::atomic<bool> stopRequested{false};
			std::atomic<bool> timerFinished{false};
			std::mutex cycleMutex;
			std::condition_variable cycleCv;
			std::string currentRecipeName;
			
			PMExecutionContext() = default;
			~PMExecutionContext() {
				if (isRunning) {
					stopRequested = true;
					cycleCv.notify_all();
					if (cycleThread.joinable()) {
						cycleThread.join();
					}
				}
			}
			
			// 禁止拷贝
			PMExecutionContext(const PMExecutionContext&) = delete;
			PMExecutionContext& operator=(const PMExecutionContext&) = delete;
		};

		// 列数据剪贴板结构 - 用于复制粘贴功能
		struct ColumnClipboard {
			bool hasData = false;
			PMMotorRow motorData;
		} columnClipboards[4]; // 每个PM独立的剪贴板

	private:
		Q_DECLARE_PRIVATE(QPmRecipeWidget)
		QPmRecipeWidgetPrivate* d_ptr;
		
		// 4个PM的独立执行上下文
		PMExecutionContext pmContexts[4];
	};
}
#endif // _PM_RECIPE_WIDGET_INCLUDE_
