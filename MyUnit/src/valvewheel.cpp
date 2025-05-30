#include "valvewheel.h"
#include "MyUnit/ui_valvewheel.h"
#include <QMouseEvent>
#include <QMenu>
#include <QAction>


ValveWheel::ValveWheel(QWidget *parent) :
    QWidget(parent),
	openAction(tr("打开"), this),
	closeAction(tr("关闭"), this),
    ui(new Ui::ValveWheel)
{

	timer1 = new QTimer(this);
	connect(timer1, &QTimer::timeout, this, &ValveWheel::timeFS);
	connect(&openAction, &QAction::triggered, this, [this](){
		timer1->start(10);
		emit signalClicked(true); //发送打开信号
	});
	connect(&closeAction, &QAction::triggered, this, [this]() {
		timer1->stop();
		emit signalClicked(false);
	});
	ui->setupUi(this);
    //timer1->start(10); // 每30毫秒更新一次位置
}
void ValveWheel::open(){
	timer1->start(10);
}
void ValveWheel::close(){
	timer1->stop();
}
void ValveWheel::isDisableClick(bool disable){
	isdisableclick = disable;
}
void ValveWheel::mousePressEvent(QMouseEvent *event){
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


ValveWheel::~ValveWheel()
{
    delete ui;
}
void ValveWheel::paintEvent(QPaintEvent *event) {
     QPainter painter(this);
     painter.setRenderHint(QPainter::Antialiasing);
     // 设置颜色和线宽
     painter.setPen(QPen(colorLine, thicknessLine));

     // 绘制风扇中心
     painter.drawEllipse(QPoint(width() / 2, height() / 2), width() / 3, width() / 3);

     // 绘制风扇叶片（这里只绘制一个叶片作为示例）
     painter.save();
     painter.translate(width() / 2, height() / 2); // 移动到中心点
     painter.rotate(rotationAngle); // 应用旋转

     painter.setRenderHint(QPainter::HighQualityAntialiasing); // 抗锯齿


     // 计算十字架的中心点
     int centerX = 0;
     int centerY = 0;

     // 计算十字架的长度（这里假设为部件宽度的一半）
     int crossLength = width() / 3;

     // 绘制十字架的横线和竖线
     painter.drawLine(centerX - crossLength, centerY, centerX + crossLength, centerY); // 横线
     painter.drawLine(centerX, centerY - crossLength, centerX, centerY + crossLength); // 竖线

     painter.restore();
 }
void ValveWheel::timeFS()
{
    if(statr){
        if(AngleDirection){//旋转方向控制
            rotationAngle += 5; // 每次增加2度来模拟旋转
            if (rotationAngle >= 360) {
                 rotationAngle = 0; // 重置角度
            }
        }else{
            rotationAngle -= 5; // 每次增加2度来模拟旋转
            if (rotationAngle >= 0) {
                 rotationAngle = 360; // 重置角度
            }
        }
        //更新ui
        update();

    }else{
        QThread::sleep(1);//关闭时 每次 1 秒更新一次

    }
}
