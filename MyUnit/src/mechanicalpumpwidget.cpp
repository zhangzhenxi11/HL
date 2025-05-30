#include "mechanicalpumpwidget.h"
#include "MyUnit/ui_mechanicalpumpwidget.h"

MechanicalPumpWidget::MechanicalPumpWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MechanicalPumpWidget)
{
    ui->setupUi(this);
}

MechanicalPumpWidget::~MechanicalPumpWidget()
{
    delete ui;
}

void MechanicalPumpWidget::paintEvent(QPaintEvent *){
    QPainter painter(this);
    int w=width();
    int h=height();
    painter.drawPixmap(0,0,w,h,QPixmap("image/MechanicalPump.svg"));
}
