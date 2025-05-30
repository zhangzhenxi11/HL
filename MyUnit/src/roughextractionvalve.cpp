#include "roughextractionvalve.h"
#include "MyUnit/ui_roughextractionvalve.h"
#include <QMouseEvent>
#include <QMenu>


#include <QTransform>
RoughExtractionValve::RoughExtractionValve(QWidget *parent) :
    QWidget(parent),
	openAction(tr("打开"), this),
	closeAction(tr("关闭"), this),
    ui(new Ui::RoughExtractionValve)
{
	connect(&openAction, &QAction::triggered, this, [this]() {
		//open(); // 调用 open 方法
		emit signalClicked(true); // 发送打开信号
	});
	connect(&closeAction, &QAction::triggered, this, [this]() {
		//close(); // 调用 close 方法
		emit signalClicked(false);
	});

    ui->setupUi(this);
}

RoughExtractionValve::~RoughExtractionValve()
{
    delete ui;
}
void RoughExtractionValve::open(){
	isImageOpen = true;
	update(); // 触发重绘
}
void RoughExtractionValve::close(){
	isImageOpen = false;
	update(); // 触发重绘
}
void RoughExtractionValve::paintEvent(QPaintEvent *){
    QPainter painter(this);
    int w=width();
    int h=height();
	QString imagestr = "image/RoughExtractionValve.svg";
	if (!isImageOpen) {
		imagestr = "image/RoughExtractionValveClose.svg";
	}
	QPixmap pixmap(imagestr);

	// 创建一个QTransform对象
	QTransform transform;
	// 旋转图像
	transform.rotate(angle);
	// 应用变换到原始图像，创建旋转后的图像
	QPixmap rotatedPixmap = pixmap.transformed(transform);
	painter.drawPixmap(0, 0, w, h, rotatedPixmap);
}

void RoughExtractionValve::mousePressEvent(QMouseEvent *event){
	if (event->button() == Qt::RightButton&&!isdisableclick) {
		QMenu menu(this);
		menu.addAction(&openAction);
		menu.addAction(&closeAction);
		//connect(action1, &QAction::triggered, this, [this](){
		//	isImageOpen = true; // 设置为打开状态
		//	update(); // 触发重绘
		//	emit signalClicked(true); //发送打开信号
		//});
		//connect(action2, &QAction::triggered, this, [this]() {
		//	isImageOpen = false; // 设置为关闭状态
		//	update(); // 触发重绘
		//	emit signalClicked(false);
		//});
		// 在鼠标点击的位置显示菜单
		menu.exec(event->globalPos());
	}
	// 确保调用基类的实现
	QWidget::mousePressEvent(event);
}
