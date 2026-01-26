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

#include <QVBoxLayout>
#include <QTabWidget>
#include <QDateTime>
#include <QTimer>
#include <QDebug>
#include <QCoreApplication>
#include <QLabel>

#define USE_WEBENGINE
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
        QWebEngineView* viewAcc;
        QWebEngineView* viewVel;
        QWebEngineView* viewPos;
        
        QTimer* timer;
        qint64 startTime;
        int pagesLoaded = 0;  // 跟踪所有页面加载状态
        bool isPageLoaded = false;
        
        // Data buffers
        QList<QString> timeList;
        QList<double> accZList, accRList;
        QList<double> velZList, velRList;
        QList<double> posZList, posRList;
        
        // Database helper
        std::shared_ptr<DataSubSystemHelper> dataHelper;
	};

	DataWidgetPrivate::DataWidgetPrivate(DataWidget* p, const std::shared_ptr<IKernel>& k) :
		q_ptr(p),
        kernel(k),
        tabWidget(nullptr),
        viewAcc(nullptr),
        viewVel(nullptr),
        viewPos(nullptr),
        currentSubsystem(nullptr),
        startTime(0),
        pagesLoaded(0)
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
        
        // Helper to create tab
        auto createTab = [&](const QString& title) -> QWebEngineView* {
#ifdef USE_WEBENGINE
            QWidget* page = new QWidget();
            QVBoxLayout* layout = new QVBoxLayout(page);
            layout->setContentsMargins(0,0,0,0);
            QWebEngineView* view = new QWebEngineView(page);
            
            // 设置焦点策略，确保能接收鼠标事件
            view->setFocusPolicy(Qt::StrongFocus);
            view->setAttribute(Qt::WA_AcceptTouchEvents, true);
            view->setAttribute(Qt::WA_NativeWindow, true);  // 确保OpenGL正确渲染
            
            layout->addWidget(view);
            d->tabWidget->addTab(page, title);
            
            // Load HTML
            QString appDirPath = QCoreApplication::applicationDirPath() + "/Echarts/line-stack.html";
            //qDebug() << "[DataWidget]" << title << "loading HTML from:" << appDirPath;
            //qDebug() << "  - File exists:" << QFile::exists(appDirPath);
            view->load(QUrl::fromLocalFile(appDirPath));
            
            //qDebug() << "[DataWidget]" << title << "QWebEngineView created";
            //qDebug() << "  - isEnabled:" << view->isEnabled();
            //qDebug() << "  - focusPolicy:" << view->focusPolicy();
            
            return view;
#else
            // 临时禁用WebEngine，显示占位符
            QWidget* page = new QWidget();
            QVBoxLayout* layout = new QVBoxLayout(page);
            QLabel* label = new QLabel("Chart disabled (WebEngine not available)", page);
            label->setAlignment(Qt::AlignCenter);
            layout->addWidget(label);
            d->tabWidget->addTab(page, title);
            return nullptr;
#endif
        };
        
        d->viewAcc = createTab("Acceleration");
        d->viewVel = createTab("Velocity");
        d->viewPos = createTab("Position");

#ifdef USE_WEBENGINE
        //C++ 等待 JS 环境就绪
        // Wait for page load - 检测所有页面都加载完成
        auto onPageLoaded = [d](bool ok){ 
            if(ok) {
                d->pagesLoaded++;
                //qDebug() << "[DataWidget] Page loaded successfully. Total:" << d->pagesLoaded << "/3";
                // 当所有三个页面都加载完成时才设置为true
                if(d->pagesLoaded >= 3) {
                    d->isPageLoaded = true;
                    //qDebug() << "[DataWidget] *** All WebEngine pages loaded successfully ***";
                }
            } else {
                //qDebug() << "[DataWidget] ERROR: Page load failed!";
            }
        };
        if(d->viewAcc) connect(d->viewAcc, &QWebEngineView::loadFinished, onPageLoaded);
        if(d->viewVel) connect(d->viewVel, &QWebEngineView::loadFinished, onPageLoaded);
        if(d->viewPos) connect(d->viewPos, &QWebEngineView::loadFinished, onPageLoaded);
        
        // 监听加载进度
        if(d->viewVel) {
            connect(d->viewVel, &QWebEngineView::loadProgress, [](int progress){
                if(progress % 20 == 0 || progress == 100) {
                    //qDebug() << "[DataWidget] Velocity view load progress:" << progress << "%";
                }
            });
        }
#else
        // WebEngine禁用时直接标记为已加载
        d->isPageLoaded = true;
#endif
        
        // Timer connection
        connect(d->timer, &QTimer::timeout, this, &DataWidget::onclick);
        //connect(d->timer, &QTimer::timeout, this, &DataWidget::onSimulateTest);
        //d->startTime = QDateTime::currentMSecsSinceEpoch();
        //Sleep(2000);
        //d->timer->start(2000);
	}

    DataWidget::~DataWidget()
	{
        delete d_ptr;
	}

    void DataWidget::onCycleStart(const std::string& pmName)
    {
        Q_D(DataWidget);
        //qDebug() << "[DataWidget] onCycleStart called for PM:" << QString::fromStdString(pmName);
        //qDebug() << "  - isPageLoaded:" << d->isPageLoaded;
        // Get Subsystem
        
        d->currentSubsystem = d->kernel->getKernelModule<FortrendPMCavitySubsystem>(pmName);

        if (!d->currentSubsystem) {
            //qDebug() << "DataWidget: PM Subsystem not found:" << QString::fromStdString(pmName);
            return;
        }
        
        // Reset Data
        d->timeList.clear();
        d->accZList.clear();
        d->accRList.clear();
        d->velZList.clear();
        d->velRList.clear();
        d->posZList.clear();
        d->posRList.clear();
        
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
        if (!d->isPageLoaded) return;
        
        d->currentSubsystem = d->kernel->getKernelModule<FortrendPMCavitySubsystem>("PM1");

        if (!d->currentSubsystem) {
            //qDebug() << "DataWidget: PM Subsystem not found:" << QString::fromStdString("PM1");
            return;
        }

        //qint64 now = QDateTime::currentMSecsSinceEpoch();
        //double t = (double)(now - d->startTime); // ms
        //QString tStr = QString::number(t, 'f', 0); // Display as string
        qint64 now = QDateTime::currentMSecsSinceEpoch();
        QString tStr = QDateTime::fromMSecsSinceEpoch(now).toString("HH:mm:ss:zzz");

        // Generate Random Data
        double accZ = (qrand() % 200) - 100; // -100 to 100
        double accR = (qrand() % 200) - 100;
        
        double velZ = qrand() % 500; // 0 to 500
        double velR = qrand() % 500;
        
        double posZ = qrand() % 100; // 0 to 100
        double posR = qrand() % 100;
        
        std::string pmName = d->currentSubsystem->getName();
        d->dataHelper->insertRealtimeData("PM1", accZ, accR, velZ, velR, posZ, posR);

        // Append
        d->timeList.append(tStr);
        d->accZList.append(accZ);
        d->accRList.append(accR);
        d->velZList.append(velZ);
        d->velRList.append(velR);
        d->posZList.append(posZ);
        d->posRList.append(posR);
        
        // Prune (Keep last 500 points)
        if (d->timeList.size() > 500) {
            d->timeList.removeFirst();
            d->accZList.removeFirst();
            d->accRList.removeFirst();
            d->velZList.removeFirst();
            d->velRList.removeFirst();
            d->posZList.removeFirst();
            d->posRList.removeFirst();
        }
        
        // Update Charts
        //httpUpdate(d->viewAcc, d->timeList, d->accZList, d->accRList, "Z-Acc", "R-Acc");
        httpUpdate(d->viewVel, d->timeList, d->velZList, d->velRList, "Z-Vel", "R-Vel");
        httpUpdate(d->viewPos, d->timeList, d->posZList, d->posRList, "Z-Pos", "R-Pos");
	}

	void DataWidget::onclick()
	{
		Q_D(DataWidget);
        if (!d->isPageLoaded) return;
        if (!d->currentSubsystem) return;
        
        qint64 now = QDateTime::currentMSecsSinceEpoch();
        QString tStr = QDateTime::fromMSecsSinceEpoch(now).toString("HH:mm:ss:zzz");
        
        // Fetch Data
        double accZ = d->currentSubsystem->getPMCavityZAxleAcc();
        double accR = d->currentSubsystem->getPMCavityRAxleAcc();
        
        double velZ = d->currentSubsystem->getPMCavityZAxleSpeed();
        double velR = d->currentSubsystem->getPMCavityRAxleSpeed();
        
        double posZ = d->currentSubsystem->getPMCavityZAxleLocation();
        double posR = d->currentSubsystem->getPMCavityRAxleLocation();
        
        // Save to database
        std::string pmName = d->currentSubsystem->getName();
        d->dataHelper->insertRealtimeData(pmName, accZ, accR, velZ, velR, posZ, posR);
        
        // Append
        d->timeList.append(tStr);
        d->accZList.append(accZ);
        d->accRList.append(accR);
        d->velZList.append(velZ);
        d->velRList.append(velR);
        d->posZList.append(posZ);
        d->posRList.append(posR);
        
        // Prune (Keep last 500 points)
        if (d->timeList.size() > 500) {
            d->timeList.removeFirst();
            d->accZList.removeFirst();
            d->accRList.removeFirst();
            d->velZList.removeFirst();
            d->velRList.removeFirst();
            d->posZList.removeFirst();
            d->posRList.removeFirst();
        }
        
        // Update Charts
        httpUpdate(d->viewAcc, d->timeList, d->accZList, d->accRList, "Z-Acc", "R-Acc");
        httpUpdate(d->viewVel, d->timeList, d->velZList, d->velRList, "Z-Vel", "R-Vel");
        httpUpdate(d->viewPos, d->timeList, d->posZList, d->posRList, "Z-Pos", "R-Pos");

	}

	//数据解析函数
	void DataWidget::httpUpdate(QWebEngineView* view, const QList<QString> &name, 
                                const QList<double> &dataZ, const QList<double> &dataR,
                                const QString& zName, const QString& rName)
	{
#ifdef USE_WEBENGINE
        if (!view) {
            //qDebug() << "[DataWidget] ERROR: httpUpdate called with null view!";
            return;
        }
        
        if (!view->page()) {
            //qDebug() << "[DataWidget] ERROR: QWebEngineView page is null!";
            return;
        }
        
        //qDebug() << "[DataWidget] httpUpdate for" << zName << "/" << rName;
        //qDebug() << "  - view isEnabled:" << view->isEnabled();
        //qDebug() << "  - view isVisible:" << view->isVisible();
        //qDebug() << "  - data points:" << name.size();
        
		//在QT中我们需要组成一个字符串将数据传过去
		QString jscode = "onDataReceived([";
		for (int i = 0; i < name.size(); i++)
		{
			jscode += QString("\"%1\"").arg(name[i]);
			if (i < name.size() - 1)
				jscode += ",";
		}
		jscode += "],";

        // Add legend update to match series names
        jscode += QString("['%1', '%2'],").arg(zName).arg(rName);

		//线条数据 拼接
        auto buildDataArray = [](const QList<double>& data) -> QString {
            QString str = "[";
            for (int i = 0; i < data.size(); i++) {
                str += QString::number(data[i]);
                if (i < data.size() - 1) str += ",";
            }
            str += "]";
            return str;
        };

		QString lineDataZ = buildDataArray(dataZ);
        QString lineDataR = buildDataArray(dataR);

		jscode += "[{name:'" + zName + "',data: " + lineDataZ + ",type: 'line',smooth: true}";//第一个线条
		jscode += ",{name:'" + rName + "',data: " + lineDataR + ",type: 'line',smooth: true}";//第二个线条
		jscode += "])";
        //jscode += "[{name:'" + zName + "',data: " + lineDataZ + ",type: 'line',stack: 'Total',smooth: true}";//第一个线条
        //jscode += ",{name:'" + rName + "',data: " + lineDataR + ",type: 'line',stack: 'Total',smooth: true}";//第二个线条
        //jscode += "])";

		view->page()->runJavaScript(jscode);
#else
        // WebEngine禁用时不执行
        Q_UNUSED(view);
        Q_UNUSED(name);
        Q_UNUSED(dataZ);
        Q_UNUSED(dataR);
        Q_UNUSED(zName);
        Q_UNUSED(rName);
#endif
	}

	//自适应窗体
	void DataWidget::resizeEvent(QResizeEvent *event){
        Q_D(DataWidget);
		/*QWidget::resizeEvent(event);*/
        // Views resize automatically with layout
	}
}
