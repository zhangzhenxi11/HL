#include "ltwidget.h"
#include "MyUnit/ui_ltwidget.h"

LTWidget::LTWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LTWidget)
{
    ui->setupUi(this);
}

LTWidget::~LTWidget()
{
    delete ui;
}
void LTWidget::paintEvent(QPaintEvent *){
    QPainter painter(this);
    int w=width();
    int h=height();
    painter.drawPixmap(0,0,w,h,QPixmap("image/LT.svg"));
}
