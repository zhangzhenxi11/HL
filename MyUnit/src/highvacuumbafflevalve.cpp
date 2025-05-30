#include "highvacuumbafflevalve.h"
#include "MyUnit/ui_highvacuumbafflevalve.h"

#include <QTransform>

HighVacuumBaffleValve::HighVacuumBaffleValve(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HighVacuumBaffleValve)
{
    ui->setupUi(this);
}

HighVacuumBaffleValve::~HighVacuumBaffleValve()
{
    delete ui;
}
void HighVacuumBaffleValve::paintEvent(QPaintEvent *){
    QPainter painter(this);
    int w=width();
    int h=height();
    QPixmap pixmap("image/HighVacuumBaffleValve.svg");

    // 创建一个QTransform对象
    QTransform transform;
    // 旋转图像
    transform.rotate(angle);
    // 应用变换到原始图像，创建旋转后的图像
    QPixmap rotatedPixmap = pixmap.transformed(transform);

    painter.drawPixmap(0,0,w,h,rotatedPixmap);
}
