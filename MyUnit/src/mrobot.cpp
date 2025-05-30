#include "mrobot.h"
#include "MyUnit/ui_mrobot.h"
#include <QTimer>
#include <cmath>
mRobot::mRobot(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::mRobot)
{
    ui->setupUi(this);
    // 创建定时器
       timer = new QTimer(this);
       connect(timer, &QTimer::timeout, this, &mRobot::updateAngle);
       timer->start(30); // 设置定时器间隔，单位为毫秒
}

mRobot::~mRobot()
{
    delete ui;
}
void mRobot::paintEvent(QPaintEvent *){
    QPainter painter(this);
    // 设置画笔颜色
    painter.setPen(Qt::black);
    // 获取窗口的中心点
           int centerX = width() / 2;
           int centerY = height() / 2;

           // 绘制机械手的底座（圆形）
           int baseRadius = 50; // 圆形底座的半径
           painter.drawEllipse(centerX - baseRadius, centerY - baseRadius, 2 * baseRadius, 2 * baseRadius);

           // 绘制机械手的轴承
           int bearingRadius = 10; // 轴承的半径
           painter.drawEllipse(centerX - bearingRadius, centerY - bearingRadius, 2 * bearingRadius, 2 * bearingRadius);

           // 计算第一段手臂的位置和长度
           int arm1_x1 = centerX - bearingRadius;
           int arm1_x2 = centerX + bearingRadius;
           int arm1_y1 = centerY;
           int arm1Length = 50; // 第一段手臂的长度
           // 绘制第一段手臂
           painter.drawLine(arm1_x1, arm1_y1, arm1_x1, arm1_y1 - arm1Length);
           painter.drawLine(arm1_x2, arm1_y1, arm1_x2, arm1_y1 - arm1Length);


           // 计算第二段手臂的位置和长度
           int arm2_x1 = centerX + 5;
           int arm2_y1 = centerY;

           int arm2Length = 50; // 第二段手臂的长度

           // 计算轴承连接的位置
           int bearing_x = centerX;
           int bearing_y = centerY - arm1Length;
           int bearing_x1=bearing_x - bearingRadius;
           int bearing_y1=bearing_y - bearingRadius;
           // 绘制轴承连接
           painter.drawEllipse(bearing_x1,bearing_y1 , 2 * bearingRadius, 2 * bearingRadius);

           int arm2X1=bearing_x-bearingRadius-arm2Length/sqrt(2);//2手臂的终点位置X坐标和手臂长度公式：轴承X坐标-臂长/根号二
           int arm2Y1=bearing_y-arm2Length/sqrt(2);
           painter.drawLine(bearing_x-bearingRadius, bearing_y, arm2X1, arm2Y1);
           drawParallelLine(painter,bearing_x-bearingRadius,bearing_y,arm2X1,arm2Y1,bearingRadius);



           //drawArm(painter, bearing_x1, bearing_y1, arm2Length);
}

void mRobot::drawArm(QPainter &painter, int x, int y, int length) {
        // 保存当前的画笔状态
        painter.save();

        // 移动画笔到绘制手臂的起始位置
        painter.translate(x, y);

        // 旋转画笔
        painter.rotate(rotationAngle);

        // 绘制手臂
        painter.drawLine(0, 0, 0, -length);

        // 恢复画笔状态
        painter.restore();
    }

void mRobot::drawParallelLine(QPainter &painter, int x1, int y1, int x2, int y2, int X) {
    // 计算给定直线的斜率
    double slope = static_cast<double>(y2 - y1) / (x2 - x1);

    // 计算新直线的起点和终点坐标
    int newX1 = x1;
    int newY1 = y1 + X;
    int newX2 = x2;
    int newY2 = y2 + X;

    // 计算新直线的斜率
    // 如果给定直线是垂直的，则新直线斜率为无穷大
    double newSlope;
    if (x2 - x1 == 0) {
        newSlope = std::numeric_limits<double>::infinity();
    } else {
        newSlope = -1 / slope; // 两条平行线的斜率的乘积为 -1
    }

    // 根据新直线的斜率和起点绘制新直线
    // 如果新直线是垂直的，则直接绘制竖直线
    if (std::isinf(newSlope)) {
        painter.drawLine(newX1, newY1, newX1, newY2);
    } else {
        // 根据新直线的斜率和起点绘制新直线
        painter.drawLine(newX1, newY1, newX1 + 100, newY1 + 100 * newSlope);
        painter.drawLine(newX2, newY2, newX2 + 100, newY2 + 100 * newSlope);
    }
}


void mRobot::updateAngle()
{
    rotationAngle += 1;
    if (rotationAngle >= 360) {
        rotationAngle = 0;
    }
    update(); // 更新窗口，触发重绘
}
