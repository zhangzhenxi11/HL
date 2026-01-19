#include "Data/fortrend_datahistory_subsystem_widget.h"
#include "device/ui_fortrend_datahistory_subsystem_widget.h"
#include "Data/fortrend_data_subsystem_helper.h"

#include "kernel/kernel_log.h"
#include "Kernel/kernel_exception.h"

#include "Poco/Format.h"

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
#include <QWebEngineView>
#include <QUrl>
#include <QDateTime>
#include <QCoreApplication>

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
		qDebug() << "[DataHistoryWidget] Constructor called";
		qDebug() << "  - parent:" << parent;
		
		d->ui = new Ui::DataHistoryWidget();
		d->ui->setupUi(this);
		
		// 监听窗口显示、隐藏事件
		installEventFilter(this);
		
		// Layout Management
		// Use existing layout from verticalLayoutWidget if available
		QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(d->ui->verticalLayoutWidget->layout());
		if (!layout) {
			layout = new QVBoxLayout(d->ui->verticalLayoutWidget);
			layout->setContentsMargins(0,0,0,0);
		}
		
		// // Create control panel for date/time selection
		// QWidget* controlPanel = new QWidget();
		// controlPanel->setFixedHeight(60);
		// QHBoxLayout* controlLayout = new QHBoxLayout(controlPanel);
		// controlLayout->setContentsMargins(5,5,5,5);
		
		
		// // Add date/time controls
		// QLabel* startLabel = new QLabel("Start Time:");
		// QLabel* endLabel = new QLabel("End Time:");
		// QLabel* pmLabel = new QLabel("PM Chamber:");
		
		// d->startTimeEdit = new QDateTimeEdit();
		// d->endTimeEdit = new QDateTimeEdit();
		// d->pmComboBox = new QComboBox();
		
		// // Set default datetime
		// QDateTime currentDateTime = QDateTime::currentDateTime();
		// d->startTimeEdit->setDateTime(currentDateTime.addDays(-1)); // Default to 1 day ago
		// d->endTimeEdit->setDateTime(currentDateTime);
		// d->startTimeEdit->setDisplayFormat("yyyy-MM-dd HH:mm:ss");
		// d->endTimeEdit->setDisplayFormat("yyyy-MM-dd HH:mm:ss");
		// d->startTimeEdit->setCalendarPopup(true);
		// d->endTimeEdit->setCalendarPopup(true);
		
		// QPushButton* queryButton = new QPushButton("Query History");
		// connect(queryButton, &QPushButton::clicked, this, &DataHistoryWidget::onQueryClicked);
		
		// // Add widgets to control layout
		// controlLayout->addWidget(startLabel);
		// controlLayout->addWidget(d->startTimeEdit);
		// controlLayout->addWidget(endLabel);
		// controlLayout->addWidget(d->endTimeEdit);
		// controlLayout->addWidget(pmLabel);
		// controlLayout->addWidget(d->pmComboBox);
		// controlLayout->addWidget(queryButton);
		// controlLayout->addStretch();
		
		// // Insert control panel at the top
		// layout->insertWidget(0, controlPanel);
		
		// Ensure qweb is in the layout
		if (layout->indexOf(d->ui->qweb) == -1) {
			layout->addWidget(d->ui->qweb);
		}
		
		// Re-setup main layout to ensure full screen
		if (this->layout() == nullptr) {
			QVBoxLayout* mainLayout = new QVBoxLayout(this);
			mainLayout->addWidget(d->ui->verticalLayoutWidget);
		}

		//数据加载 - 使用fromLocalFile确保本地文件正确加载
		QString appDirPath = QCoreApplication::applicationDirPath() + "/Echarts/history-line-stack.html";
		qDebug() << "[DataHistoryWidget] Loading HTML from:" << appDirPath;
		qDebug() << "[DataHistoryWidget] File exists:" << QFile::exists(appDirPath);
		
		d->ui->qweb->load(QUrl::fromLocalFile(appDirPath));
		
		// 设置焦点策略，确保能接收鼠标事件
		d->ui->qweb->setFocusPolicy(Qt::StrongFocus);
		d->ui->qweb->setAttribute(Qt::WA_AcceptTouchEvents, true);
		
		qDebug() << "[DataHistoryWidget] QWebEngineView properties:";
		qDebug() << "  - isEnabled:" << d->ui->qweb->isEnabled();
		qDebug() << "  - isVisible:" << d->ui->qweb->isVisible();
		qDebug() << "  - focusPolicy:" << d->ui->qweb->focusPolicy();
		qDebug() << "  - geometry:" << d->ui->qweb->geometry();
		
		// 监听页面加载完成
		connect(d->ui->qweb, &QWebEngineView::loadFinished, [](bool ok){
			qDebug() << "[DataHistoryWidget] Page load finished:" << ok;
		});
		
		// 监听页面加载进度
		connect(d->ui->qweb, &QWebEngineView::loadProgress, [](int progress){
			if(progress % 20 == 0 || progress == 100) {
				qDebug() << "[DataHistoryWidget] Page load progress:" << progress << "%";
			}
		});
		
		// Initialize line names for different data types
		lineName.clear();
		lineName.append("Z-Axis Velocity");
		lineName.append("R-Axis Velocity");
		lineName.append("Z-Axis Position");
		lineName.append("R-Axis Position");
		
		// Initialize database
		std::string db_file = "./realtime_data.db"; // Use same DB as real-time data
		d->datahelper->opendb(db_file);
		
		// Populate PM chamber combo box
		/*populatePMChambers();*/

		// 连接按钮点击事件
		connect(d->ui->select_btn, &QPushButton::clicked, this, &DataHistoryWidget::onQueryClicked);
		
		// 连接ComboBox信号 - 添加空指针检查和调试日志
		if (d->ui->dateType) {
			qDebug() << "[DataHistoryWidget] Connecting dateType signal, object name:" << d->ui->dateType->objectName();
			connect(d->ui->dateType, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), 
			        this, &DataHistoryWidget::populatePMChambers);
		} else {
			qDebug() << "[DataHistoryWidget] ERROR: dateType is nullptr!";
		}
		
		// 初始化pmComboBox指针
		d->pmComboBox = d->ui->dateType;
		if (!d->pmComboBox) {
			qDebug() << "[DataHistoryWidget] ERROR: pmComboBox assignment failed!";
		}
	}
	void DataHistoryWidget::onSelect(){
		
	}
	
	void DataHistoryWidget::onQueryClicked() {
	    Q_D(DataHistoryWidget);
	    
	    qDebug() << "[DataHistoryWidget] onQueryClicked() called";
	    qDebug() << "  - qweb isEnabled:" << d->ui->qweb->isEnabled();
	    qDebug() << "  - qweb isVisible:" << d->ui->qweb->isVisible();
	    qDebug() << "  - qweb hasFocus:" << d->ui->qweb->hasFocus();
	    
	    // Get selected time range and PM chamber
	    QString startTimeStr = d->ui->startDatetime->dateTime().toString("yyyy-MM-dd HH:mm");
	    QString endTimeStr = d->ui->endDatetime->dateTime().toString("yyyy-MM-dd HH:mm");
	    QString selectedPM = d->pmComboBox->currentText();
	    
	    if (selectedPM.isEmpty()) {

	        qDebug() << "Please select a PM chamber";
	  /*      return;*/
			selectedPM = "PM1";
	    }
	    
	    // Query historical data from database
	    std::vector<std::vector<std::string>> results = d->datahelper->queryHistoricalData(
	        selectedPM.toStdString(),
	        startTimeStr.toStdString(),
	        endTimeStr.toStdString()
	    );
	    
	    if (results.empty()) {
	        qDebug() << "No historical data found for the selected period";
	        return;
	    }
	    
	    // Clear existing data
	    httpname.clear();
	    httpdata.clear();
	    
	    // Prepare data for chart display
	    QList<QString> timestamps;
	    QList<double> accZData, accRData;
	    QList<double> velZData, velRData;
	    QList<double> posZData, posRData;
	    
	    for (const auto& row : results) {
	        if (row.size() >= 7) { // timestamp, acc_z, acc_r, vel_z, vel_r, pos_z, pos_r
	            timestamps.append(QString::fromStdString(row[0])); // timestamp
	            accZData.append(std::stod(row[1])); // acc_z
	            accRData.append(std::stod(row[2])); // acc_r
	            velZData.append(std::stod(row[3])); // vel_z
	            velRData.append(std::stod(row[4])); // vel_r
	            posZData.append(std::stod(row[5])); // pos_z
	            posRData.append(std::stod(row[6])); // pos_r
	        }
	    }
	    
	    // For demo purposes, let's display velocity data (you can modify to show other data)
	    // We'll show Z-axis velocity as primary data
	    QList<int> displayData;
	    for (double val : velZData) {
	        displayData.append(static_cast<int>(val));
	    }
	    
	    // Update chart with historical data
	    qDebug() << "[DataHistoryWidget] Updating chart with" << timestamps.size() << "data points";
	    httpUpdate(timestamps, displayData);
	}
	
	void DataHistoryWidget::populatePMChambers(int index) {
	    Q_D(DataHistoryWidget);
	    
	    // Query all available PM chambers from database
	    std::vector<std::vector<std::string>> results = d->datahelper->queryAllPMNames();
	    
	    d->pmComboBox->clear();
	    for (const auto& row : results) {
	        if (!row.empty()) {
	            d->pmComboBox->addItem(QString::fromStdString(row[0]));
	        }
	    }
	    
	    //if (d->pmComboBox->count() > 0) {
	    //    d->pmComboBox->setCurrentIndex(0);
	    //}

		if (index <= d->pmComboBox->count())
		{
			d->pmComboBox->setCurrentIndex(index);
		}
	}
	void DataHistoryWidget::onclick()
	{
		Q_D(DataHistoryWidget);
		QString dateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");//获取系统当前的时间
		httpname.append(dateTime);

		int randcount = 10;
		qsrand(QTime(0, 0, 0).secsTo(QTime::currentTime()));
		randcount += qrand() % 10 + 10;

		d->datahelper->exce(Poco::format(
			"INSERT OR REPLACE INTO test "
			"(value) "
			"VALUES(%d);"
			, randcount));

		httpdata.append(randcount);//随机数

		httpUpdate(httpname, httpdata);//更新html
		if (httpdata.count() > 500){
			httpdata.erase(httpdata.begin(), httpdata.begin() + 400);
		}
		if (httpname.count() > 500){
			httpname.erase(httpname.begin(), httpname.begin() + 400);
		}
	}

	DataHistoryWidget::~DataHistoryWidget()
	{
		Q_D(DataHistoryWidget);
		delete d->ui;
	}

	//数据解析函数
	void DataHistoryWidget::httpUpdate(const QList<QString> &name, const QList<int> &data)
	{
		Q_D(DataHistoryWidget);
		
		qDebug() << "[DataHistoryWidget] httpUpdate called with" << name.size() << "timestamps," << data.size() << "data points";
		qDebug() << "  - qweb page isNull:" << (d->ui->qweb->page() == nullptr);
		//在QT中我们需要组成一个字符串将数据传过去
		QString jscode = "onDataReceived([";
		for (int i = 0; i < name.size(); i++)
		{
			jscode += QString("\"%1\"").arg(name[i]);
			if (i < name.size() - 1)
				jscode += ",";
		}
		jscode += "],";

        // Add Legend Data
        jscode += "[";
        for (int i = 0; i < lineName.size(); i++) {
            jscode += QString("'%1'").arg(lineName[i]);
            if (i < lineName.size() - 1) jscode += ",";
        }
        jscode += "],";

		//线条数据 拼接
		QString lineData = "[";
		for (int i = 0; i < data.size(); i++)
		{
			lineData += QString::number(data[i]);
			if (i < data.size() - 1)
				lineData += ",";
		}
		lineData += "]";

		jscode += "[{name:'" + lineName[0] + "',data: " + lineData + ",type: 'line',stack: 'Total',smooth: true}";//第一个线条
		jscode += ",{name:'" + lineName[1] + "',data: " + lineData + ",type: 'line',stack: 'Total',smooth: true}";//第二个线条
		jscode += "])";
		d->ui->qweb->page()->runJavaScript(jscode);
	}




	//自适应窗体
	void DataHistoryWidget::resizeEvent(QResizeEvent *event){
		Q_D(DataHistoryWidget);
		qDebug() << "[DataHistoryWidget] resizeEvent:" << event->size();
		// 调用父类的resizeEvent
		QWidget::resizeEvent(event);
	}
	
	void DataHistoryWidget::showEvent(QShowEvent *event) {
		Q_D(DataHistoryWidget);
		qDebug() << "[DataHistoryWidget] *** showEvent - Widget is now visible ***";
		qDebug() << "  - this geometry:" << this->geometry();
		qDebug() << "  - this size:" << this->size();
		qDebug() << "  - qweb isEnabled:" << d->ui->qweb->isEnabled();
		qDebug() << "  - qweb isVisible:" << d->ui->qweb->isVisible();
		qDebug() << "  - qweb geometry:" << d->ui->qweb->geometry();
		qDebug() << "  - qweb size:" << d->ui->qweb->size();
		qDebug() << "  - verticalLayoutWidget geometry:" << d->ui->verticalLayoutWidget->geometry();
		
		// 强制更新布局
		this->updateGeometry();
		d->ui->qweb->updateGeometry();
		
		QWidget::showEvent(event);
		
		// 显示后再次检查
		QTimer::singleShot(100, this, [this, d](){
			qDebug() << "[DataHistoryWidget] 100ms after showEvent:";
			qDebug() << "  - qweb geometry:" << d->ui->qweb->geometry();
			qDebug() << "  - qweb isVisible:" << d->ui->qweb->isVisible();
		});
	}
	
	void DataHistoryWidget::hideEvent(QHideEvent *event) {
		qDebug() << "[DataHistoryWidget] hideEvent - Widget is now hidden";
		QWidget::hideEvent(event);
	}
	
	bool DataHistoryWidget::eventFilter(QObject *obj, QEvent *event) {
		if (event->type() == QEvent::FocusIn) {
			qDebug() << "[DataHistoryWidget] FocusIn event on" << obj->objectName();
		} else if (event->type() == QEvent::FocusOut) {
			qDebug() << "[DataHistoryWidget] FocusOut event on" << obj->objectName();
		} else if (event->type() == QEvent::MouseButtonPress) {
			qDebug() << "[DataHistoryWidget] MouseButtonPress event on" << obj->objectName();
		}
		return QWidget::eventFilter(obj, event);
	}
}