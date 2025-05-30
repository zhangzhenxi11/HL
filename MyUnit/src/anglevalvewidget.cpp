#include "anglevalvewidget.h"
#include "MyUnit/ui_anglevalvewidget.h"
#include <QMouseEvent>
#include <QMenu>

AngleValveWidget::AngleValveWidget(QWidget *parent) :
    QWidget(parent),
	openAction(tr("open"), this),
	closeAction(tr("close"), this),
    ui(new Ui::AngleValveWidget)
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

AngleValveWidget::~AngleValveWidget()
{
    delete ui;
}
void AngleValveWidget::open(){
	isImageOpen = true;
	update(); // 触发重绘
}
void AngleValveWidget::close(){
	isImageOpen = false;
	update(); // 触发重绘
}
void AngleValveWidget::paintEvent(QPaintEvent *){
    QPainter painter(this);
    int w=width();
    int h=height();
	QString imagestr = "image/AngleValve.svg";
	if (!isImageOpen) {
		imagestr = "image/AngleValveClose.svg";
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
void AngleValveWidget::mousePressEvent(QMouseEvent *event){
	if (event->button() == Qt::RightButton&&!isdisableclick) {
		QMenu menu(this);
		menu.addAction(&openAction);
		menu.addAction(&closeAction);
		
		// 在鼠标点击的位置显示菜单
		menu.exec(event->globalPos());
	}
	// 确保调用基类的实现
	QWidget::mousePressEvent(event);
}

