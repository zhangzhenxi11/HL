#include "rbwidget.h"
#include "MyUnit/ui_rbwidget.h"

RBWidget::RBWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RBWidget)
{
    ui->setupUi(this);
}

RBWidget::~RBWidget()
{
    delete ui;
}
void RBWidget::paintEvent(QPaintEvent *){
    QPainter painter(this);
    int w=width();
    int h=height();
    painter.drawPixmap(0,0,w,h,QPixmap("image/RB.svg"));
}
