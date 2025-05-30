#include "tmwidgettow.h"
#include "MyUnit/ui_tmwidgettow.h"

tmwidgettow::tmwidgettow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::tmwidgettow)
{
    ui->setupUi(this);
    ui->widget_4->setWaterDirection(1);//设置从下往上流动
    ui->widget_13->setWaterDirection(1);//设置从下往上流动
    //ui->widget_14->setWaterDirection(1);//设置从下往上流动
    //ui->widget_15->setWaterDirection(1);//设置从下往上流动
    //ui->widget_16->setWaterDirection(1);//设置从下往上流动

    //ui->widget_9 ->angle = 270;//设置从下往上流动
    //ui->widget_10->angle = 270;//设置从下往上流动
    ui->widget_11->angle = 270;//设置从下往上流动
    //ui->widget_12->angle = 270;//设置从下往上流动


    ui->widget_17->setWaterDirection(1);//设置从下往上流动
    //ui->widget_18->setWaterDirection(1);//设置从下往上流动
    //ui->widget_19->setWaterDirection(1);//设置从下往上流动
    //ui->widget_20->setWaterDirection(1);//设置从下往上流动

   // ui->widget_21->setWaterDirection(1);//设置从下往上流动
    //ui->widget_22->setWaterDirection(1);//设置从下往上流动
   // ui->widget_23->setWaterDirection(1);//设置从下往上流动
    //ui->widget_24->setWaterDirection(1);//设置从下往上流动

    ui->widget_26->setWaterDirection(1);//设置从下往上流动

    ui->widget_29->setWaterDirection(1);//设置从下往上流动
    ui->widget_30->setWaterDirection(1);//设置从下往上流动
    ui->widget_31->setWaterDirection(1);//设置从下往上流动

    ui->widget_35->setWaterDirection(1);//设置从下往上流动
    ui->widget_36->setWaterDirection(1);//设置从下往上流动
    ui->widget_37->setWaterDirection(1);//设置从下往上流动

    ui->widget_41->setWaterDirection(1);//设置从下往上流动
    ui->widget_42->setWaterDirection(1);//设置从下往上流动
    ui->widget_43->setWaterDirection(1);//设置从下往上流动

    ui->widget_44->setWaterDirection(1);//设置从下往上流动
    ui->widget_46->setWaterDirection(1);//设置从下往上流动
    ui->widget_49->setWaterDirection(1);//设置从下往上流动
    ui->widget_50->setWaterDirection(1);//设置从下往上流动


    ui->widget_53->setWaterDirection(1);//设置从下往上流动
    ui->widget_55->setWaterDirection(1);//设置从下往上流动
    ui->widget_57->setWaterDirection(1);//设置从下往上流动
    ui->widget_61->setWaterDirection(1);//设置从下往上流动

    ui->widget_62->setWaterDirection(1);//设置从下往上流动
    ui->widget_64->setWaterDirection(1);//设置从下往上流动
    ui->widget_66->setWaterDirection(1);//设置从下往上流动
    ui->widget_70->setWaterDirection(1);//设置从下往上流动

    ui->widget_75->setWaterDirection(1);//设置从下往上流动
    ui->widget_78->setWaterDirection(1);//设置从下往上流动
    ui->widget_79->setWaterDirection(1);//设置从下往上流动

//    ui->widget_16->setWaterDirection(1);

    // 设置径向渐变
    QRadialGradient gradient(width() / 2, height() / 2, width()/2*1.5); // 中心点和半径
    gradient.setColorAt(0, Qt::darkBlue); // 起始颜色（中心）
    gradient.setColorAt(1, Qt::black); // 结束颜色（边缘）


    // 设置QPalette并应用渐变
    QPalette palette;
    palette.setBrush(backgroundRole(), gradient);
    setPalette(palette);

    // 设置窗口属性，使背景色可以自动填充整个窗口
    setAutoFillBackground(true);
}

tmwidgettow::~tmwidgettow()
{
    delete ui;
}
