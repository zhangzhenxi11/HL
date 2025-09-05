#include "pipeLineWidget.h"
#include "MyUnit/ui_pipeLineWidget.h"
#include <QDebug>
#include <cmath>

#include "thread"
#include <QThread>
pipeLineWidget::pipeLineWidget(QWidget *parent,int width,int pipeDiameter,int angle,int WaterDirection) :
    QWidget(parent),
    rotationAngle(angle),
    waterPosition(0),
    waterDirection(0),
    waterSpeed(2.0) ,
    pipeWidth(width),
    pipeDiameter(pipeDiameter),
    waterWidth(pipeDiameter),
    currentWaterDirection(WaterDirection), // 初始化方向
    ui(new Ui::pipeLineWidget)
{

    ui->setupUi(this);

//    std::thread T(&pipeLineWidget::updateWaterPosition,this);
//    T.detach();

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &pipeLineWidget::updateWaterPosition);
    start(30);
}

pipeLineWidget::~pipeLineWidget()
{
    delete ui;
}

void pipeLineWidget::updateWaterPosition(){

//    while(true){
//        mutex.lock();
        waterPosition += waterSpeed; // 始终更新水流位置

        if(waterPosition > pipeWidth + waterWidth) {
            waterPosition = 0;
        }
       update(); // 触发重绘

//       QThread::msleep(20);
//    }

}

// 设置水流方向
void pipeLineWidget::setWaterDirection(int direction) {
      currentWaterDirection=direction;
}
pipeLineWidget& pipeLineWidget::getInstance()
{
    static pipeLineWidget instance;
    return instance;
}
void pipeLineWidget::stop()
{
    timer->stop(); // 每30毫秒更新一次位置
}
void pipeLineWidget::start(int timeOut)
{
    timer->start(30); // 每30毫秒更新一次位置
}
// 设置旋转角度
    void pipeLineWidget::setRotationAngle(int angle) {
        rotationAngle = angle;
        update(); // 触发重绘
    }
void pipeLineWidget::paintEvent(QPaintEvent * event)
{
    QPainter painter(this);
      pipeWidth = width();
      pipeDiameter = height();

      QPixmap pixmap("image/StraightLine.svg");
      painter.drawPixmap(0, 0, pipeWidth, pipeDiameter, pixmap);

      QPen pen2(QColor(0, 255, 0), 1);
      painter.setPen(pen2);
      painter.setBrush(QBrush(QColor(0, 255, 0)));
      if (waterDirection == 0) { // 水平方向
          int totalLength = pipeWidth + waterWidth; // 总长度包括管道和水滴宽度，以保证连续性
          int interval = waterWidth + 5; // 水滴和间隔的总长度
          qreal startPosition = fmod(waterPosition, interval); // 起始位置

          if(currentWaterDirection==0){
              //从左往右移动
              startPosition = interval - startPosition;
              for (int position = totalLength - startPosition; position >= -waterWidth; position -= interval) {
                  if (position <= pipeWidth && position > -waterWidth) { // 仅绘制在可视范围内的水滴
					  painter.drawRect(position - waterWidth, pipeDiameter / 4, pipeDiameter / 2, pipeDiameter / 2);
                  }
              }
          }else if(currentWaterDirection==1){
                for (int position = -startPosition; position < totalLength; position += interval) {
                    // 绘制水滴
                    if (position >= -waterWidth && position < pipeWidth) { // 仅绘制在可视范围内的水滴
						painter.drawRect(position, pipeDiameter / 4, pipeDiameter / 2, pipeDiameter / 2);
                    }
                }
		  }
      }
}

