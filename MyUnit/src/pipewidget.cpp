#include "pipewidget.h"
#include "MyUnit/ui_pipewidget.h"
#include <QDebug>


pipeWidget::pipeWidget(QWidget *parent,int width,int height,int pipeDiameter,int angle) :
    QWidget(parent),
    rotationAngle(angle),
    waterPosition(0),
    waterDirection(0),
    waterSpeed(2.0) ,
    pipeWidth(width),
    pipeHeight(height),
    pipeDiameter(pipeDiameter),
    ui(new Ui::pipeWidget)
{
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &pipeWidget::updateWaterPosition);
    timer->start(30); // 每30毫秒更新一次位置
    ui->setupUi(this);
}

pipeWidget::~pipeWidget()
{
    delete ui;
}

void pipeWidget::updateWaterPosition(){
       // 根据流动速度和方向更新位置
       waterPosition += waterSpeed;

       if(waterDirection == 0 && waterPosition > pipeWidth){
           setWaterDirection(1);
       }
       if (waterDirection == 1 && waterPosition > pipeHeight) {
           setWaterDirection(0); // 重置位置，模拟循环流动
       }
       update(); // 触发重绘
}

// 设置水流方向
void pipeWidget::setWaterDirection(int direction) {
       waterDirection = direction;
       waterPosition = 0; // 每次改变方向时重置水流位置
}
// 设置旋转角度
    void pipeWidget::setRotationAngle(int angle) {
        rotationAngle = angle;
        update(); // 触发重绘
    }
void pipeWidget::paintEvent(QPaintEvent * event)
{
           QPainter painter(this);
           painter.setRenderHint(QPainter::Antialiasing, true);

           // 应用旋转变换
           QTransform transform;
           transform.translate(width() / 2, height() / 2); // 将旋转中心移动到widget的中心
           transform.rotate(rotationAngle); // 应用旋转角度
           transform.translate(-width() / 2, -height() / 2); // 将绘图原点移回
           painter.setTransform(transform);

           // 设置管道的颜色和样式
           QPen pen(QColor(33, 83, 141), 5);
           painter.setPen(pen);
           painter.setBrush(QBrush(QColor(33, 83, 141)));

//           QRect rect1(0, 0, pipeWidth, pipeDiameter);
//           QRect rect2(pipeWidth, 0, pipeDiameter, pipeHeight); // 垂直部分
//           drawRectEdges(painter, rect1, rect2);
//           drawRectEdges(painter, rect2, rect1);

           QPainterPath path;
           path.moveTo(0, 0);     // 移动到 (0, 0) 点
           path.lineTo(pipeWidth+pipeDiameter, 0);   // 添加一条直线
           path.lineTo(pipeWidth+pipeDiameter, pipeHeight); // 添加一条直线
           path.lineTo(pipeWidth, pipeHeight); // 添加一条直线
           path.lineTo(pipeWidth, pipeDiameter);     // 添加一条直线
           path.lineTo(0, pipeDiameter);
           path.lineTo(0, 0);
           // 使用 QPainterPath 绘制路径
           painter.drawPath(path);

           QPen pen2(QColor(68, 160, 250), 1);
           painter.setPen(pen2);
           // 绘制方向
          painter.setBrush(QBrush(QColor(68, 160, 250)));
          if (waterDirection == 0) { // 水平方向
              painter.drawRect(waterPosition, 0, pipeDiameter, pipeDiameter);
          } else { // 垂直方向
              painter.drawRect(pipeWidth, pipeDiameter + waterPosition, pipeDiameter, pipeDiameter);
          }
}

void pipeWidget::drawRectEdges(QPainter &painter, const QRect &rect, const QRect &otherRect) {
        drawPartialEdge(painter, rect.topLeft(), rect.topRight(), otherRect);//左上角点，右上角点 上边
        drawPartialEdge(painter, rect.topRight(), rect.bottomRight(), otherRect);//右上角点 右下角点 右边
        drawPartialEdge(painter, rect.bottomLeft(), rect.bottomRight(), otherRect); //左下角点 右下角点  底边
        drawPartialEdge(painter, rect.topLeft(), rect.bottomLeft(), otherRect);//左上角点 左下角点  左边
}

void pipeWidget::drawPartialEdge(QPainter &painter, const QPoint &start, const QPoint &end, const QRect &otherRect) {

    // 检查线段是否水平
        if (start.y() == end.y()) {
            if (start.x() < otherRect.left()) {
                QPoint newEnd(qMin(end.x(), otherRect.left()), start.y());
                painter.drawLine(start, newEnd);
            }
            if (end.x() > otherRect.right()) {
                QPoint newStart(qMax(start.x(), otherRect.right()), end.y());
                painter.drawLine(newStart, end);
            }
        } else if (start.x() == end.x()) { // 检查线段是否垂直
            // 如果线段在otherRect的左边或右边
            //if (start.x() < otherRect.left() || start.x() > otherRect.right()) {
            if ((otherRect.left()-start.x())>1 || (start.x()-otherRect.right())>1) {//1为线宽
                painter.drawLine(start, end); // 直接绘制整条线
//                qDebug()<<"startX:" << start.x();
//                qDebug()<<"startY:" << start.y();
//                qDebug()<<"endX:" << end.x();
//                qDebug()<<"endY:" << end.y();
            } else {
                // 线段可能与otherRect垂直重合的部分
                if (start.y() < otherRect.top()) {
                    painter.drawLine(start.x(), start.y(), start.x(), qMin(end.y(), otherRect.top()));
                }
                if (end.y() > otherRect.bottom()) {
                    painter.drawLine(end.x(), qMax(start.y(), otherRect.bottom()), end.x(), end.y());
                }

            }
        }
}
