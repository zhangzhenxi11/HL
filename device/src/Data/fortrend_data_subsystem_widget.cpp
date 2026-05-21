#include "Data/fortrend_data_subsystem_widget.h"
#include "PMCavity/fortrend_pm_cavity_subsystem.h"
#include "Kernel/kernel_api.h"
#include  "Kernel/kernel.h"
#include  "kernel/kernel_log.h"
#include  "Kernel/kernel_action_subsystem.h"
#include  "kernel/kernel_parallel_action.h"
#include  "kernel/kernel_subsystem.h"
#include  "Kernel/Fortrend/fortrend_station.h"
#include "device/ui_fortrend_data_subsystem_widget.h"
#include "Data/fortrend_data_subsystem_helper.h"

#include "qcustomplot.h"

#include <QVBoxLayout>
#include <QTabWidget>
#include <QDateTime>
#include <QTimer>
#include <QDebug>
#include <QCoreApplication>
#include <QMenu>
#include <QFileDialog>

namespace FC{

	class DataWidgetPrivate{
		Q_DECLARE_PUBLIC(DataWidget)
	public:
		DataWidgetPrivate(DataWidget* p, const std::shared_ptr<IKernel>& k);
		~DataWidgetPrivate();
    private:
        Ui::DataWidget* ui;

	public:
		DataWidget* q_ptr;
        std::shared_ptr<IKernel> kernel = 0;
        std::shared_ptr<FortrendPMCavitySubsystem> currentSubsystem = nullptr;
        
        QTabWidget* tabWidget;
        QCustomPlot* plotAcc;
        QCustomPlot* plotVel;
        QCustomPlot* plotPos;
        
        QTimer* timer;
        qint64 startTime;
        
        // Data buffers
        QVector<double> timeKeys;
        QVector<double> accZVec, accRVec;
        QVector<double> velZVec, velRVec;
        QVector<double> posZVec, posRVec;
        
        // Database helper
        std::shared_ptr<DataSubSystemHelper> dataHelper;
	};

	DataWidgetPrivate::DataWidgetPrivate(DataWidget* p, const std::shared_ptr<IKernel>& k) :
		q_ptr(p),
        kernel(k),
        tabWidget(nullptr),
        plotAcc(nullptr),
        plotVel(nullptr),
        plotPos(nullptr),
        currentSubsystem(nullptr),
        startTime(0)
    {
        timer = new QTimer(p);
        // Initialize database helper
        dataHelper = std::make_shared<DataSubSystemHelper>();
        std::string db_file = "./realtime_data.db";
        dataHelper->opendb(db_file);
	}
    
	DataWidgetPrivate::~DataWidgetPrivate(){
		
	}

	DataWidget::DataWidget(const std::shared_ptr<IKernel>& kernel, QWidget *parent) :
		QWidget(parent),
		d_ptr(new DataWidgetPrivate(this, kernel))
	{
		Q_D(DataWidget);
        
        d->ui = new Ui::DataWidget();
        d->ui->setupUi(this);

        // Main Layout
        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(0,0,0,0);
        
        d->tabWidget = new QTabWidget(this);
        mainLayout->addWidget(d->tabWidget);
        
        // Helper to create tab with QCustomPlot
        auto createTab = [&](const QString& title, const QString& zName, const QString& rName) -> QCustomPlot* {
            QWidget* page = new QWidget();
            QVBoxLayout* layout = new QVBoxLayout(page);
            layout->setContentsMargins(0,0,0,0);
            QCustomPlot* plot = new QCustomPlot(page);
            layout->addWidget(plot);
            d->tabWidget->addTab(page, title);
            
            // Create 2 graphs: Z-axis and R-axis
            plot->addGraph();
            plot->graph(0)->setName(zName);
            plot->graph(0)->setPen(QPen(Qt::blue, 2));
            
            plot->addGraph();
            plot->graph(1)->setName(rName);
            plot->graph(1)->setPen(QPen(Qt::red, 2));
            
            // X-axis: time format
            QSharedPointer<QCPAxisTickerDateTime> timeTicker(new QCPAxisTickerDateTime);
            timeTicker->setDateTimeFormat("HH:mm:ss");
            plot->xAxis->setTicker(timeTicker);
            plot->xAxis->setTickLabelRotation(30);
            
            // Y-axis: auto range
            plot->yAxis->setLabel(title);
            
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
            
            return plot;
        };
        
        d->plotAcc = createTab("Acceleration", "Z-Acc", "R-Acc");
        d->plotVel = createTab("Velocity", "Z-Vel", "R-Vel");
        d->plotPos = createTab("Position", "Z-Pos", "R-Pos");

        // Only replot when switching tabs
        connect(d->tabWidget, &QTabWidget::currentChanged, [d](int index){
            QCustomPlot* plots[] = {d->plotAcc, d->plotVel, d->plotPos};
            if (index >= 0 && index < 3 && plots[index]) {
                plots[index]->replot();
            }
        });
        
        // Timer connection
        connect(d->timer, &QTimer::timeout, this, &DataWidget::onclick);
	}

    DataWidget::~DataWidget()
	{
        delete d_ptr;
	}

    void DataWidget::onCycleStart(const std::string& pmName)
    {
        Q_D(DataWidget);
        
        d->currentSubsystem = d->kernel->getKernelModule<FortrendPMCavitySubsystem>(pmName);

        if (!d->currentSubsystem) {
            return;
        }
        
        // Reset Data
        d->timeKeys.clear();
        d->accZVec.clear();
        d->accRVec.clear();
        d->velZVec.clear();
        d->velRVec.clear();
        d->posZVec.clear();
        d->posRVec.clear();
        
        d->startTime = QDateTime::currentMSecsSinceEpoch();
        d->timer->start(200); // 200ms interval
    }
    
    void DataWidget::onCycleStop()
    {
        Q_D(DataWidget);
        d->timer->stop();
    }

	void DataWidget::onSimulateTest()
	{
		Q_D(DataWidget);
        
        d->currentSubsystem = d->kernel->getKernelModule<FortrendPMCavitySubsystem>("PM1");

        if (!d->currentSubsystem) {
            return;
        }

        qint64 now = QDateTime::currentMSecsSinceEpoch();
        double key = QCPAxisTickerDateTime::dateTimeToKey(QDateTime::currentDateTime());

        // Generate Random Data
        double accZ = (qrand() % 200) - 100;
        double accR = (qrand() % 200) - 100;
        double velZ = qrand() % 500;
        double velR = qrand() % 500;
        double posZ = qrand() % 100;
        double posR = qrand() % 100;
        
        // Save to database
        QString timestampStr = QDateTime::fromMSecsSinceEpoch(now).toString("yyyy-MM-dd HH:mm:ss");
        d->dataHelper->insertRealtimeDataWithTimestamp("PM1", accZ, accR, velZ, velR, posZ, posR, timestampStr.toStdString());

        // Append
        d->timeKeys.append(key);
        d->accZVec.append(accZ);
        d->accRVec.append(accR);
        d->velZVec.append(velZ);
        d->velRVec.append(velR);
        d->posZVec.append(posZ);
        d->posRVec.append(posR);
        
        // Prune (Keep last 500 points)
        if (d->timeKeys.size() > 500) {
            d->timeKeys.remove(0, 1);
            d->accZVec.remove(0, 1);
            d->accRVec.remove(0, 1);
            d->velZVec.remove(0, 1);
            d->velRVec.remove(0, 1);
            d->posZVec.remove(0, 1);
            d->posRVec.remove(0, 1);
        }
        
        // Update Charts
        updateChart(d->plotVel, d->timeKeys, d->velZVec, d->velRVec);
        updateChart(d->plotPos, d->timeKeys, d->posZVec, d->posRVec);
	}

	void DataWidget::onclick()
	{
		Q_D(DataWidget);
        if (!d->currentSubsystem) return;
        
        qint64 now = QDateTime::currentMSecsSinceEpoch();
        double key = QCPAxisTickerDateTime::dateTimeToKey(QDateTime::currentDateTime());
        
        // Fetch Data
        double accZ = double(d->currentSubsystem->getPMCavityZAxleAcc());
        double accR = double(d->currentSubsystem->getPMCavityRAxleAcc());
        
        double velZ = double(d->currentSubsystem->getPMCavityZAxleSpeed());
        double velR = double(d->currentSubsystem->getPMCavityRAxleSpeed());
        
        double posZ = double(d->currentSubsystem->getPMCavityZAxleLocation());
        double posR = double(d->currentSubsystem->getPMCavityRAxleLocation());
        
        // Save to database
        QString timestampStr = QDateTime::fromMSecsSinceEpoch(now).toString("yyyy-MM-dd HH:mm:ss");
        std::string pmName = d->currentSubsystem->getName();
        d->dataHelper->insertRealtimeDataWithTimestamp(pmName, accZ, accR, velZ, velR, posZ, posR, timestampStr.toStdString());
        
        // Append
        d->timeKeys.append(key);
        d->accZVec.append(accZ);
        d->accRVec.append(accR);
        d->velZVec.append(velZ);
        d->velRVec.append(velR);
        d->posZVec.append(posZ);
        d->posRVec.append(posR);
        
        // Prune (Keep last 1000 points)
        if (d->timeKeys.size() > 1000) {
            d->timeKeys.remove(0, 1);
            d->accZVec.remove(0, 1);
            d->accRVec.remove(0, 1);
            d->velZVec.remove(0, 1);
            d->velRVec.remove(0, 1);
            d->posZVec.remove(0, 1);
            d->posRVec.remove(0, 1);
        }
        
        // Update Charts
        updateChart(d->plotAcc, d->timeKeys, d->accZVec, d->accRVec);
        updateChart(d->plotVel, d->timeKeys, d->velZVec, d->velRVec);
        updateChart(d->plotPos, d->timeKeys, d->posZVec, d->posRVec);
	}

	void DataWidget::updateChart(QCustomPlot* plot, const QVector<double> &timeKeys, 
                                 const QVector<double> &dataZ, const QVector<double> &dataR)
	{
        if (!plot || timeKeys.isEmpty()) return;
        
        plot->graph(0)->setData(timeKeys, dataZ);
        plot->graph(1)->setData(timeKeys, dataR);
        
        // X-axis: show recent 60-second window
        plot->xAxis->setRange(timeKeys.last(), 60, Qt::AlignRight);
        
        // Y-axis: auto-fit
        plot->graph(0)->rescaleValueAxis(false);
        plot->graph(1)->rescaleValueAxis(true);
        
        // Only replot if this plot's tab is currently visible
        Q_D(DataWidget);
        int currentTab = d->tabWidget->currentIndex();
        QCustomPlot* plots[] = {d->plotAcc, d->plotVel, d->plotPos};
        if (currentTab >= 0 && currentTab < 3 && plots[currentTab] == plot) {
            plot->replot(QCustomPlot::rpQueuedReplot);
        }
	}

	void DataWidget::resizeEvent(QResizeEvent *event){
        Q_D(DataWidget);
        QWidget::resizeEvent(event);
	}
}
