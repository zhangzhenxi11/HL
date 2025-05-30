#include "slidevalvewidget.h"
#include "MyUnit/ui_slidevalvewidget.h"
#include <QMouseEvent>
#include <QMenu>
#include <QAction>

#include <QTransform>
SlideValveWidget::SlideValveWidget(QWidget *parent) :
    QWidget(parent),
	openAction(tr("打开"), this),
	closeAction(tr("关闭"), this),
    ui(new Ui::SlideValveWidget)
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

SlideValveWidget::~SlideValveWidget()
{
    delete ui;
}
void SlideValveWidget::open(){
	isImageOpen = true;
	update(); // 触发重绘
}
void SlideValveWidget::close(){
	isImageOpen = false;
	update(); // 触发重绘
}
void SlideValveWidget::paintEvent(QPaintEvent *){
    QPainter painter(this);

    int w=width();
    int h=height();
	QString imagestr = "image/SlideValve.svg";

	if (!isImageOpen){
		imagestr = "image/SlideValveClose.svg";
	}

	QPixmap pixmap(imagestr);
    // 创建一个QTransform对象
    QTransform transform;
    // 旋转图像
    transform.rotate(angle);
    // 应用变换到原始图像，创建旋转后的图像
    QPixmap rotatedPixmap = pixmap.transformed(transform);

    painter.drawPixmap(0,0,w,h,rotatedPixmap);

}

void SlideValveWidget::mousePressEvent(QMouseEvent *event){
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


