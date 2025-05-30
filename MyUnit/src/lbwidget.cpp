#include "lbwidget.h"
#include "MyUnit/ui_lbwidget.h"


LBWidget::LBWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LBWidget)
{
    ui->setupUi(this);
}

LBWidget::~LBWidget()
{
    delete ui;
}
void LBWidget::paintEvent(QPaintEvent *){
    QPainter painter(this);
    int w=width();
    int h=height();
    painter.drawPixmap(0,0,w,h,QPixmap("image/LB.svg"));
}
