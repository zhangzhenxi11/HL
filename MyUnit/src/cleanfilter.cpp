#include "cleanfilter.h"
#include "MyUnit/ui_cleanfilter.h"

#include <QTransform>
CleanFilter::CleanFilter(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CleanFilter)
{
    ui->setupUi(this);
}

CleanFilter::~CleanFilter()
{
    delete ui;
}
void CleanFilter::paintEvent(QPaintEvent *){
    QPainter painter(this);
    int w=width();
    int h=height();
    QPixmap pixmap("image/CleanFilter.svg");

    // 创建一个QTransform对象
    QTransform transform;
    // 旋转图像
    transform.rotate(angle);
    // 应用变换到原始图像，创建旋转后的图像
    QPixmap rotatedPixmap = pixmap.transformed(transform);

    painter.drawPixmap(0,0,w,h,rotatedPixmap);
}
