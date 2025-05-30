#include "robotwidget.h"
#include "MyUnit/ui_robotwidget.h"
#include <QPaintEvent>
#include <QPainterPath>
robotWidget::robotWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::robotWidget),
    m_rotationAngle(0.0)
{
    ui->setupUi(this);
    // 创建动画
            QPropertyAnimation *animation = new QPropertyAnimation(this, "rotationAngle");
            animation->setDuration(2000); // 动画持续时间
            animation->setStartValue(0); // 起始角度
            animation->setEndValue(360); // 结束角度
            animation->setLoopCount(-1); // 无限循环
            animation->start(); // 启动动画
}

robotWidget::~robotWidget()
{
    delete ui;
}
void robotWidget::paintEvent(QPaintEvent *event){
    QPainter painter(this);
            painter.setRenderHint(QPainter::Antialiasing);

            int centerX = width() / 2;
            int centerY = height() / 2;
            // 圆形基座的中心位置
                   QPointF baseCenter(centerX, centerY-100);
                   qreal baseRadius = 70;
                   // 绘制圆形基座
                    painter.setBrush(QColor(166, 166, 166)); // 设置填充颜色
                   painter.drawEllipse(baseCenter, baseRadius, baseRadius); // 绘制圆形基座
                   // 第一个圆形轴承的位置与基座圆心相同
                   QPointF bearing1Center = baseCenter;
                   qreal bearingRadius = 20; // 轴承的半径
                   qreal bearingDiameter = bearingRadius*2;
                   // 绘制第一个圆形轴承
                   painter.setBrush(QColor(166, 166, 166)); // 设置填充颜色
                   painter.drawEllipse(bearing1Center, bearingRadius, bearingRadius); // 绘制圆形轴承

                   // 计算第一段机械手臂的起始位置
                   QPointF arm1Start(bearing1Center.x(), bearing1Center.y());

                   painter.translate(bearing1Center); // 平移坐标系到轴承中心
                   painter.rotate(m_rotationAngle); // 应用旋转
                   painter.translate(-bearing1Center); // 平移回原始坐标系

                   // 第一段机械手臂尺寸和位置
                   qreal arm1Height = 80;
                   // 绘制第一段机械手臂
                   QPainterPath arm1Path;
                   arm1Path.moveTo(arm1Start + QPointF(-bearingDiameter / 2, 0)); // 上边左点
                   arm1Path.arcTo(QRectF(arm1Start.x() - bearingDiameter / 2, arm1Start.y() - bearingDiameter / 2, bearingDiameter, bearingDiameter), 180, 180);
                   arm1Path.lineTo(arm1Start + QPointF(bearingDiameter / 2, arm1Height)); // 下边右点
                   arm1Path.arcTo(QRectF(arm1Start.x()-bearingDiameter / 2, arm1Start.y()+arm1Height-bearingDiameter/2, bearingDiameter, bearingDiameter), 0, 180);
                   arm1Path.closeSubpath();
                   painter.setBrush(QColor(185, 185, 210)); // 设置填充颜色
                   painter.drawPath(arm1Path);

                   // 计算第二个圆形轴承的位置
                   QPointF bearing2Center = QPointF(arm1Start.x(), arm1Start.y() + arm1Height);

                   painter.translate(bearing2Center); // 平移坐标系到轴承中心
                   painter.rotate(m_rotationAngle); // 应用旋转
                   painter.translate(-bearing2Center); // 平移回原始坐标系

                   painter.setBrush(QColor(166, 166, 166)); // 设置填充颜色
                   // 绘制第二个圆形轴承
                   painter.drawEllipse(bearing2Center, bearingRadius, bearingRadius); // 绘制圆形轴承

                   // 计算第二段机械手臂的起始位置
                   QPointF arm2Start = QPointF(bearing2Center.x(), bearing2Center.y());
                   // 第二段机械手臂尺寸和位置
                   qreal arm2Height = 80;
                   // 绘制第二段机械手臂
                   QPainterPath arm2Path;
                   arm2Path.moveTo(arm2Start + QPointF(-bearingDiameter / 2, 0)); // 上边左点
                   arm2Path.arcTo(QRectF(arm2Start.x() - bearingDiameter / 2, arm2Start.y() - bearingDiameter / 2, bearingDiameter, bearingDiameter), 180, 180);
                   arm2Path.lineTo(arm2Start + QPointF(bearingDiameter / 2, arm2Height)); // 下边右点
                   arm2Path.arcTo(QRectF(arm2Start.x()-bearingDiameter / 2, arm2Start.y()+arm2Height-bearingDiameter/2, bearingDiameter, bearingDiameter), 0, 180);
                   arm2Path.closeSubpath();
                   painter.setBrush(QColor(185, 185, 210)); // 设置填充颜色
                   painter.drawPath(arm2Path);

                   // 计算第三个圆形轴承的位置
                   QPointF bearing3Center = QPointF(arm2Start.x(), arm2Start.y() + arm2Height);
                   painter.translate(bearing3Center); // 平移坐标系到轴承中心
                   painter.rotate(m_rotationAngle); // 应用旋转
                   painter.translate(-bearing3Center); // 平移回原始坐标系


                   painter.setBrush(QColor(166, 166, 166)); // 设置填充颜色
                   // 绘制第三个圆形轴承
                   painter.drawEllipse(bearing3Center, bearingRadius, bearingRadius); // 绘制圆形轴承
                      QPointF bearingCenter(arm2Start.x(), arm2Start.y() + arm2Height); // 第三个轴承的中心位置

                      // 梯形的顶部中心位置
                      QPointF trapezoidTopCenter(bearingCenter.x(), bearingCenter.y());

                      // 梯形尺寸和位置
                      qreal trapezoidHeight = 40.0;
                      qreal trapezoidTopWidth = bearingDiameter; // 短边，等于轴承直径
                      qreal trapezoidBottomWidth = 1.2 * bearingDiameter; // 长边

                      // 绘制梯形
                      QPainterPath trapezoidPath;
                      trapezoidPath.moveTo(trapezoidTopCenter + QPointF(-trapezoidTopWidth / 2, 0)); // 上边左点
                      trapezoidPath.arcTo(QRectF(trapezoidTopCenter.x() - bearingDiameter / 2, trapezoidTopCenter.y() - bearingDiameter / 2, bearingDiameter, bearingDiameter), 180, 180);
                      trapezoidPath.lineTo(trapezoidTopCenter + QPointF(trapezoidBottomWidth / 2, trapezoidHeight)); // 下边右点
                      trapezoidPath.lineTo(trapezoidTopCenter + QPointF(-trapezoidBottomWidth / 2, trapezoidHeight)); // 下边左点
                      trapezoidPath.closeSubpath();

                      // 绘制三角形
                      QPointF triangleBaseLeft = trapezoidTopCenter + QPointF(-trapezoidBottomWidth / 2, trapezoidHeight);
                      QPointF triangleBaseRight = trapezoidTopCenter + QPointF(trapezoidBottomWidth / 2, trapezoidHeight);
                      qreal triangleHeight = 60; // 三角形高，可调整

                      QPainterPath trianglePath;
                      // 左侧三角形
                      trianglePath.moveTo(triangleBaseLeft);
                      trianglePath.lineTo(triangleBaseLeft + QPointF(-trapezoidBottomWidth / 4, triangleHeight));
                      trianglePath.lineTo(triangleBaseLeft + QPointF(trapezoidBottomWidth / 2, 0));
                      trianglePath.closeSubpath();

                      // 右侧三角形
                      trianglePath.moveTo(triangleBaseRight);
                      trianglePath.lineTo(triangleBaseRight + QPointF(trapezoidBottomWidth / 4, triangleHeight));
                      trianglePath.lineTo(triangleBaseRight - QPointF(trapezoidBottomWidth / 2, 0));
                      trianglePath.closeSubpath();
                      painter.setBrush(QColor(185, 185, 210)); // 设置填充颜色
                      // 绘制路径


                      painter.drawPath(trapezoidPath);
                      painter.drawPath(trianglePath);
}
