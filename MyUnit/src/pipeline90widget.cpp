#include "pipeLine90Widget.h"
#include "MyUnit/ui_pipeLine90Widget.h"
#include <QDebug>
#include <QPainter>

#include <QThread>
#include "thread"
pipeLine90Widget::pipeLine90Widget(QWidget *parent) :
    QWidget(parent),
    waterPosition(0),
    pipeHeight(100),
    waterDirection(1),
    waterSpeed(2.0) ,
    waterHeight(20),
    currentWaterDirection(0),
    ui(new Ui::pipeLine90Widget)
{

    ui->setupUi(this);
        timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &pipeLine90Widget::updateWaterPosition);
        timer->start(20); // 每30毫秒更新一次位置

//    std::thread T(&pipeLine90Widget::updateWaterPosition,this);
//    T.detach();
}

pipeLine90Widget::~pipeLine90Widget()
{
    delete ui;
}

void pipeLine90Widget::updateWaterPosition(){

//    while(true){

        waterPosition += waterSpeed; // 始终更新水流位置

        if(waterPosition > pipeHeight + waterHeight) {
            waterPosition = 0;
        }
       update(); // 触发重绘
//       QThread::msleep(1);
//    }

}

// 设置水流方向,0是从上往下，1是从下往上
void pipeLine90Widget::setWaterDirection(int direction) {
     currentWaterDirection=direction;
}

void pipeLine90Widget::paintEvent(QPaintEvent * event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
     pipeHeight=height();
     pipeDiameter=width();
    //begin直线
    QPixmap pixmap("image/StraightLine.svg");
    // 创建一个QTransform对象
    QTransform transform;
    // 旋转图像
    transform.rotate(90);
    // 应用变换到原始图像，创建旋转后的图像
    QPixmap rotatedPixmap = pixmap.transformed(transform);
    painter.drawPixmap(0,0,width(),height(),rotatedPixmap);


    QPen pen2(QColor(0, 255, 0), 1);
    painter.setPen(pen2);
    // 绘制方向
    painter.setBrush(QBrush(QColor(0, 255, 0)));
    if (waterDirection == 1) {

        int totalLength = pipeHeight + waterHeight; //总长度包括管道和水滴高度，以保证连续性
        int interval = waterHeight + 5; // 水滴和间隔的总长度
        qreal startPosition = fmod(waterPosition, interval); // 起始位置

        if(currentWaterDirection==0){
            //从上往下
            startPosition = interval - startPosition;
            for (int position = totalLength - startPosition; position >= -waterHeight; position -= interval) {
                if (position <= pipeHeight && position > -waterHeight) { // 仅绘制在可视范围内的水滴
//                           painter.drawRect(0, position - waterHeight,pipeDiameter,waterHeight);
					painter.drawRect(pipeDiameter / 4, position - waterHeight, pipeDiameter / 2, pipeDiameter / 2);
                }
            }
        }else if(currentWaterDirection==1){
            for (int position = -startPosition; position < totalLength; position += interval) {
                // 绘制水滴
                if (position >= -waterHeight && position < pipeHeight) { // 仅绘制在可视范围内的水滴
//                           painter.drawRect(0,position ,pipeDiameter ,waterHeight );
					painter.drawRect(pipeDiameter / 4, position, pipeDiameter / 2, pipeDiameter / 2);
                }
            }
        }
    }
 }

