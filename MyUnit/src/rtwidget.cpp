#include "rtwidget.h"
#include "MyUnit/ui_rtwidget.h"

RTWidget::RTWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RTWidget)
{
    ui->setupUi(this);
}

RTWidget::~RTWidget()
{
    delete ui;
}

void RTWidget::paintEvent(QPaintEvent *){
    QPainter painter(this);
    int w=width();
    int h=height();
    painter.drawPixmap(0,0,w,h,QPixmap("image/RT.svg"));
}
