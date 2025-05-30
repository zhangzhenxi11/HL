#include "straightlinewidget.h"
#include "MyUnit/ui_straightlinewidget.h"

StraightLineWidget::StraightLineWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StraightLineWidget)
{
    ui->setupUi(this);
}

StraightLineWidget::~StraightLineWidget()
{
    delete ui;
}
void StraightLineWidget::paintEvent(QPaintEvent *){
    QPainter painter(this);
    int w=width();
    int h=height();
    painter.drawPixmap(0,0,w,h,QPixmap("image/StraightLine.svg"));
}
