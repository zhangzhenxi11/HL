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

		//数据加载
		QString appDirPath = QCoreApplication::applicationDirPath() + "/Echarts/history-line-stack.html";
		d->ui->qweb->load(QUrl(appDirPath));
		lineName.append("gate 01");
		lineName.append("gate 02");
		lineName.append("gate 03");

	

		std::string db_file = "./data.data";
		
		int rc = 0;
		rc = d->datahelper->opendb(db_file);
	}
	void DataHistoryWidget::onSelect(){
	
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
		//在QT中我们需要组成一个字符串将数据传过去
		QString jscode = "onDataReceived([";
		for (int i = 0; i < name.size(); i++)
		{
			jscode += QString("\"%1\"").arg(name[i]);
			if (i < name.size() - 1)
				jscode += ",";
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
		// 调用父类的resizeEvent
		QWidget::resizeEvent(event);

		//重新加载
		QString appDirPath = QCoreApplication::applicationDirPath() + "/Echarts/history-line-stack.html";
		d->ui->qweb->load(QUrl(appDirPath));


		// 获取窗体的大小
		int width = event->size().width();
		int height = event->size().height();
		// 将自定义控件的大小设为窗体的大小
		d->ui->qweb->setGeometry(0, 0, width, height);
	}
}