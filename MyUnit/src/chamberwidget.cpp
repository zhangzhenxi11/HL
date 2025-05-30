#include "chamberwidget.h"
#include "MyUnit/ui_chamberwidget.h"
#include <QPainter>
#include <QTimer>
#include <QFontMetrics>

#include <QMouseEvent>
#include <QMenu>
#include <QAction>

chamberWidget::chamberWidget(QWidget *parent) :
    QWidget(parent),
    isbusy(false),
    isAlarm(true),
    rotationAngle(0),
	isWafer(false),
    ui(new Ui::chamberWidget)
{
   ui->setupUi(this);
    // 创建定时器
   timer = new QTimer(this);
   connect(timer, &QTimer::timeout, this, &chamberWidget::updateColor);
   timer->start(300); // 设置定时器间隔，单位为毫秒
}

chamberWidget::~chamberWidget()
{
    delete ui;
}
void chamberWidget::SetName(std::string Name){
	name = Name;
}
std::string chamberWidget::GetName(){
	return name;
}
void chamberWidget::mousePressEvent(QMouseEvent *event){
	if (event->button() == Qt::RightButton) {
		QMenu menu(this);
		QAction *action1 = menu.addAction("打开传输腔门阀");
		QAction *action2 = menu.addAction("关闭传输腔门阀");
		QAction *action3 = menu.addAction("上片完成");
		QAction *action4 = menu.addAction("取片完成");
		QAction *action5 = menu.addAction("获取获取模组状况");
		QAction *action6 = menu.addAction("复位");

		connect(action1, &QAction::triggered, this, [this](){
			emit signalPMOpenTMCavityDoor(name);
		});
		connect(action2, &QAction::triggered, this, [this]() {
			emit signalPMCloseTMCavityDoor(name);
		});
		connect(action3, &QAction::triggered, this, [this]() {
			emit signalPMGetFinished(name);
		});
		connect(action4, &QAction::triggered, this, [this]() {
			emit signalPMUplaodFinished(name);
		});
		connect(action5, &QAction::triggered, this, [this]() {
			emit signalPMGetStatus(name);
		});
		connect(action6, &QAction::triggered, this, [this]() {
			emit signalPMReset(name);
		});
		// 在鼠标点击的位置显示菜单
		menu.exec(event->globalPos());
	}
	else if (event->button() == Qt::LeftButton){
		emit signalRightClick();
	}
	// 确保调用基类的实现
	QWidget::mousePressEvent(event);
}

void chamberWidget::setRotationAngle(int angle){
    rotationAngle=angle;
	update();
}
void chamberWidget::setWafer(bool wafer){
    isWafer=wafer;
	update();
}
void chamberWidget::paintEvent(QPaintEvent *){
    QPainter painter(this);
    QPixmap pixmap("image/chamber.png");

    int centerX = pixmap.width() / 2;
    int centerY = pixmap.height() / 2;
    int statusX=centerX*0.31;
    int statusY=centerY*1.62;
    int TextX=centerX*0.6;
    int TextY=centerY*0.18;
    int RectX=centerX*0.68;
    int RectY=centerY*0.8;
    QString txt_slot="LoadLockA 1";
    QString txt_T="20°";
    QString txt_P="10";
    QColor Tcolor=Qt::green;
    QColor Pcolor=Qt::green;
    setStatus(true);
    if(rotationAngle==0){
        statusX=centerX*1.68;
        statusY=centerY*0.38;
         RectX=centerX*0.93;
         RectY=centerY*0.8;
         txt_slot="LoadLockB 3";
         txt_T="50°";
         Tcolor=Qt::red;
         Pcolor=Qt::green;
		 setStatus(true);
    }else if(rotationAngle==270){
        statusX=centerX*0.37;
        statusY=centerY*0.3;
        RectX=centerX*0.8;
        RectY=centerY*0.65;
        txt_slot="LoadLockA 2";
        txt_P="125.30";
        Tcolor=Qt::green;
        Pcolor=Qt::red;
		setStatus(true);
    }

    painter.save();
    painter.translate(centerX, centerY);
    // 旋转指定角度
    painter.rotate(rotationAngle); // 旋转45度，例如
    // 再将坐标原点移回
    painter.translate(-centerX, -centerY);

    // 设置画刷颜色
    painter.setBrush(Qt::red);
    painter.drawPixmap(0,0,pixmap);
    // 恢复到保存的画家状态，这时候的状态是未旋转的
    painter.restore();


    // 设置画笔颜色
    if(isWafer){
        painter.setPen(Qt::green);
        painter.setBrush(Qt::green);
        painter.drawRect(RectX, RectY, 50, 50);
    }

    // 设置字体大小
    QFont font = painter.font();
    font.setPointSize(11);
    font.setBold(true); // 设置字体为加粗
    painter.setFont(font);



   QFontMetrics metrics(font);
   QPointF textPosS(TextX, TextY);


   int widthDegree2 = metrics.width(txt_P);
   QPointF textPosP(TextX, TextY*2);
   QPointF textPosPa(TextX+widthDegree2+4, TextY*2);
    int widthDegree = metrics.width(txt_T);
    QPointF textPosT(TextX, TextY*3);
    QPointF textPosC(TextX+widthDegree, TextY*3);

    painter.setPen(Qt::green);

    painter.drawText(textPosS, txt_slot);

    painter.setPen(Tcolor);
    painter.drawText(textPosT, txt_T);
    painter.drawText(textPosC, "C");
    painter.setPen(Pcolor);
    painter.drawText(textPosP, txt_P);
    painter.drawText(textPosPa, "Pa");

    QPointF status(statusX,statusY );

    if(getStatus()){
    painter.setPen(Qt::green);
    painter.setBrush(Qt::green);
    painter.drawEllipse(status,15,15);
    }
    else{
        if(IsUpdate){
            painter.setPen(Qt::red);
            painter.setBrush(Qt::red);
            painter.drawEllipse(status,15,15);
        }
    }


}
int j=0;
void chamberWidget::updateColor()
{
    if(j==0){
      IsUpdate=true;
      j=1;
    }else{
       IsUpdate=false;
       j=0;
    }
    update(); // 更新窗口，触发重绘
}
void chamberWidget::setStatus(bool busy){
    isbusy=busy;
}
bool chamberWidget::getStatus(){
    return isbusy;
}
