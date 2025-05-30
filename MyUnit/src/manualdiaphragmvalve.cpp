#include "manualdiaphragmvalve.h"
#include "MyUnit/ui_manualdiaphragmvalve.h"
#include <QTransform>
ManualDiaphragmValve::ManualDiaphragmValve(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ManualDiaphragmValve)
{
    ui->setupUi(this);
}

ManualDiaphragmValve::~ManualDiaphragmValve()
{
    delete ui;
}
void ManualDiaphragmValve::paintEvent(QPaintEvent *){
    QPainter painter(this);
    int w=width();
    int h=height();
    QPixmap pixmap("image/ManualDiaphragmValve.svg");

    // 创建一个QTransform对象
    QTransform transform;
    // 旋转图像
    transform.rotate(angle);
    // 应用变换到原始图像，创建旋转后的图像
    QPixmap rotatedPixmap = pixmap.transformed(transform);

    painter.drawPixmap(0,0,w,h,rotatedPixmap);
}
