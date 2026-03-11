#include "Data/fortrend_datahistory_subsystem_widget.h"
#include "device/ui_fortrend_datahistory_subsystem_widget.h"
#include "Data/fortrend_data_subsystem_helper.h"

#include "kernel/kernel_log.h"
#include "Kernel/kernel_exception.h"

#include "qcustomplot.h"

#include "Poco/Format.h"
#include <vector>
#include <memory>
#include <string>
#include <QDir>
#include <QDebug>
#include <QTimer>
#include <QList>
#include <QDateTimeEdit>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QVBoxLayout>
#include <QDateTime>
#include <QCoreApplication>
#include <qmessagebox.h>
#include <QMenu>
#include <QFileDialog>

namespace FC{

	class DataHistoryWidgetPrivate{
		Q_DECLARE_PUBLIC(DataHistoryWidget)
	public:
		DataHistoryWidgetPrivate(DataHistoryWidget* p);
		~DataHistoryWidgetPrivate();

	private:
		Ui::DataHistoryWidget *ui;

	public:
		DataHistoryWidget* q_ptr;
		std::shared_ptr<DataSubSystemHelper> datahelper;
		//database
		bool db_en = true;
		std::string db_file;
		
		// UI controls for history query
		QDateTimeEdit* startTimeEdit;
		QDateTimeEdit* endTimeEdit;
		QComboBox* pmComboBox;
	};
	DataHistoryWidgetPrivate::DataHistoryWidgetPrivate(DataHistoryWidget* p) :
		q_ptr(p),
		datahelper(new DataSubSystemHelper()){

	}
	DataHistoryWidgetPrivate::~DataHistoryWidgetPrivate(){
		
	}


	DataHistoryWidget::DataHistoryWidget(QWidget *parent) :
		QWidget(parent),
		d_ptr(new DataHistoryWidgetPrivate(this))
	{
		Q_D(DataHistoryWidget);
		
		d->ui = new Ui::DataHistoryWidget();
		d->ui->setupUi(this);
		
		// Initialize pmComboBox pointer
		d->pmComboBox = d->ui->dateType;

		// Layout Management
		QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(d->ui->verticalLayoutWidget->layout());
		if (!layout) {
			layout = new QVBoxLayout(d->ui->verticalLayoutWidget);
			layout->setContentsMargins(0,0,0,0);
		}

		d->ui->verticalLayoutWidget->setGeometry(50, 80, 1091, 551);

		// Ensure historyPlot is in the layout
		if (layout->indexOf(d->ui->historyPlot) == -1) {
			layout->addWidget(d->ui->historyPlot);
		}

		// Initialize QCustomPlot
		QCustomPlot* plot = d->ui->historyPlot;

		// Create 4 graphs with different colors and line styles
		// Velocity on left Y-axis
		plot->addGraph(plot->xAxis, plot->yAxis);
		plot->graph(0)->setName("Z-Axis Velocity");
		plot->graph(0)->setPen(QPen(Qt::blue, 2));

		plot->addGraph(plot->xAxis, plot->yAxis);
		plot->graph(1)->setName("R-Axis Velocity");
		plot->graph(1)->setPen(QPen(Qt::red, 2));

		// Position on right Y-axis
		plot->yAxis2->setVisible(true);
		
		plot->addGraph(plot->xAxis, plot->yAxis2);
		plot->graph(2)->setName("Z-Axis Position");
		plot->graph(2)->setPen(QPen(Qt::darkGreen, 2, Qt::DashLine));

		plot->addGraph(plot->xAxis, plot->yAxis2);
		plot->graph(3)->setName("R-Axis Position");
		plot->graph(3)->setPen(QPen(QColor(255, 165, 0), 2, Qt::DashLine));

		// Y-axis labels
		plot->yAxis->setLabel("Velocity");
		plot->yAxis2->setLabel("Position");

		// X-axis: time format for historical data
		QSharedPointer<QCPAxisTickerDateTime> timeTicker(new QCPAxisTickerDateTime);
		timeTicker->setDateTimeFormat("MM-dd HH:mm");
		plot->xAxis->setTicker(timeTicker);
		plot->xAxis->setTickLabelRotation(30);

		// Interactions: zoom + drag
		plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

		// Legend
		plot->legend->setVisible(true);
		plot->legend->setFont(QFont("sans", 8));
		plot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignTop | Qt::AlignRight);

		// Antialiasing optimization
		plot->setNoAntialiasingOnDrag(true);

		// Right-click context menu for export
		plot->setContextMenuPolicy(Qt::CustomContextMenu);
		QObject::connect(plot, &QWidget::customContextMenuRequested, [plot](QPoint pos){
			QMenu menu;
			menu.addAction("Save as PNG", [plot]{
				QString path = QFileDialog::getSaveFileName(plot, "Save Chart", "", "PNG (*.png)");
				if(!path.isEmpty()) plot->savePng(path, 0, 0, 1.0, -1, 300);
			});
			menu.addAction("Save as JPG", [plot]{
				QString path = QFileDialog::getSaveFileName(plot, "Save Chart", "", "JPG (*.jpg)");
				if(!path.isEmpty()) plot->saveJpg(path, 0, 0, 1.0, -1, 300);
			});
			menu.exec(plot->mapToGlobal(pos));
		});

		// Initialize database
		std::string db_file = "./realtime_data.db";
		d->datahelper->opendb(db_file);
		
		// Populate PM chamber combo box
		populatePMChambers();

		// Connect query button
		connect(d->ui->select_btn, &QPushButton::clicked, this, &DataHistoryWidget::onQueryClicked);
	}

	void DataHistoryWidget::onQueryClicked() {
	    Q_D(DataHistoryWidget);
	    
	    // Get selected time range and PM chamber
	    QString startTimeStr = d->ui->startDatetime->dateTime().toString("yyyy-MM-dd HH:mm");
	    QString endTimeStr = d->ui->endDatetime->dateTime().toString("yyyy-MM-dd HH:mm");
	    QString selectedPM = d->pmComboBox->currentText();
	    
	    if (selectedPM.isEmpty()) {
	        return;
	    }
	    
	    // Query historical data from database
	    std::vector<std::vector<std::string>> results = d->datahelper->queryHistoricalData(
	        selectedPM.toStdString(),
	        startTimeStr.toStdString(),
	        endTimeStr.toStdString()
	    );
	    
	    if (results.empty()) {
			QMessageBox::warning(this, "No Data", "No historical data found for the selected period.");
	        return;
	    }
	    
	    // Prepare data for chart display
	    QVector<double> timeKeys;
	    QVector<double> velZData, velRData;
	    QVector<double> posZData, posRData;
	    
	    for (const auto& row : results) {
	        if (row.size() >= 7) {
	            // Parse timestamp to double key
	            QDateTime dt = QDateTime::fromString(QString::fromStdString(row[0]), "yyyy-MM-dd HH:mm:ss");
	            if (dt.isValid()) {
	                double key = QCPAxisTickerDateTime::dateTimeToKey(dt);
	                timeKeys.append(key);
	                velZData.append(std::stod(row[3]));  // vel_z
	                velRData.append(std::stod(row[4]));  // vel_r
	                posZData.append(std::stod(row[5]));  // pos_z
	                posRData.append(std::stod(row[6]));  // pos_r
	            }
	        }
	    }
	    
	    if (timeKeys.isEmpty()) {
	        QMessageBox::warning(this, "No Data", "No valid data found for the selected period.");
	        return;
	    }
	    
	    // Update chart with 4 series
	    updateHistoryChart(timeKeys, velZData, velRData, posZData, posRData);
	}
	
	void DataHistoryWidget::populatePMChambers(int index) {
	    Q_D(DataHistoryWidget);

		if (index <= d->pmComboBox->count())
		{
			d->pmComboBox->setCurrentIndex(index);
		}
	}

	void DataHistoryWidget::populatePMChambers()
	{
		Q_D(DataHistoryWidget);

		// Query all available PM chambers from database
		std::vector<std::vector<std::string>> results = d->datahelper->queryAllPMNames();
		d->ui->dateType->clear();

		if (!results.empty())
		{
			for (const auto& row : results) 
			{
				if (!row.empty()) {
					d->pmComboBox->addItem(QString::fromStdString(row[0]));
				}
			}
		}
		
		if (d->pmComboBox->count() > 0) {
		    d->pmComboBox->setCurrentIndex(0);
		}
	}

	DataHistoryWidget::~DataHistoryWidget()
	{
		Q_D(DataHistoryWidget);
		delete d->ui;
	}

	void DataHistoryWidget::updateHistoryChart(const QVector<double> &timeKeys,
	                                           const QVector<double> &velZ, const QVector<double> &velR,
	                                           const QVector<double> &posZ, const QVector<double> &posR)
	{
		Q_D(DataHistoryWidget);
		QCustomPlot* plot = d->ui->historyPlot;
		
		plot->graph(0)->setData(timeKeys, velZ);
		plot->graph(1)->setData(timeKeys, velR);
		plot->graph(2)->setData(timeKeys, posZ);
		plot->graph(3)->setData(timeKeys, posR);
		
		plot->rescaleAxes();
		plot->replot();
	}

	void DataHistoryWidget::resizeEvent(QResizeEvent *event){
		QWidget::resizeEvent(event);
	}
	
	void DataHistoryWidget::showEvent(QShowEvent *event) {
		QWidget::showEvent(event);
	}
	
	void DataHistoryWidget::hideEvent(QHideEvent *event) {
		QWidget::hideEvent(event);
	}
}
