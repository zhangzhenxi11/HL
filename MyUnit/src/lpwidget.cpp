#include "lpwidget.h"
#include "MyUnit/ui_lpwidget.h"
#include <QPainter>
LPWidget::LPWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LPWidget)
{
    ui->setupUi(this);
}

LPWidget::~LPWidget()
{
    delete ui;
}
void LPWidget::paintEvent(QPaintEvent *){
    QPainter painter(this);
    // 设置画笔颜色
    painter.setPen(Qt::green);
    // 设置画刷颜色
    //painter.setBrush(Qt::red);
    // 绘制一个圆，参数分别是圆心坐标、半径
    //painter.drawEllipse(0, 0, 300, 300);
    //painter.drawLine(QPoint(0,0),QPoint(100,100));
    //painter.drawPixmap(20,0,QPixmap("image/main.png"));
    // 绘制一个矩形，参数分别是矩形的左上角坐标、宽度和高度
    //painter.drawRect(0, 0, 120, 80);
//    int x = 50;
//    int y = 50;
//    int width = 100;
//    int height = 100;
//    int xRadius = 20; // 左右两边的圆角半径
//    int yRadius = height / 2; // 底部的圆角半径
//     painter.drawRoundedRect(x, y, width, height, 0, 20);
    // 创建一个 QPainterPath 对象，用于定义矩形的路径
               // path.quadTo(200, 100, 0, 150);
           QPainterPath path;
           path.moveTo(0, 0);     // 移动到 (0, 0) 点
           path.lineTo(150, 0);   // 添加一条直线到 (150, 0) 点
           path.lineTo(150, 150); // 添加一条直线到 (150, 150) 点
           path.quadTo(75, 200, 0, 150); // 添加一个二次贝塞尔曲线段，控制点为 (75, 150)，终点为 (0, 150) 点
           path.lineTo(0, 0);     // 再次添加一条直线到 (0, 0) 点，形成闭合的矩形路径
           // 设置画笔颜色
           painter.setPen(Qt::black);
           // 设置画刷颜色
           painter.setBrush(QColor(166, 166, 166)); // 设置填充颜色
           // 使用 QPainterPath 绘制路径
           painter.drawPath(path);
           QPainterPath path2;
           path2.moveTo(20, 20);     // 移动到 (20, 20) 点
           path2.lineTo(130, 20);   // 添加一条直线到 (150, 0) 点
           path2.lineTo(130, 130); // 添加一条直线到 (150, 150) 点
           path2.quadTo(75, 180, 20, 130); // 添加一个二次贝塞尔曲线段，控制点为 (75, 150)，终点为 (0, 150) 点
           path2.lineTo(20, 20);     // 再次添加一条直线到 (0, 0) 点，形成闭合的矩形路径
           painter.setPen(Qt::black);
           painter.setPen(Qt::DashLine);//设置虚线
           painter.setBrush(QColor(212, 216, 219)); // 设置填充颜色
           painter.drawPath(path2);

           painter.setPen(Qt::SolidLine);//设置实线
           painter.setBrush(Qt::red);
           painter.drawRect(70, 50, 20, 5);
           painter.setBrush(Qt::green);
           painter.drawEllipse(40, 60, 5, 5);
           painter.drawEllipse(110, 60, 5, 5);
           painter.drawEllipse(75, 120, 5, 5);
}
