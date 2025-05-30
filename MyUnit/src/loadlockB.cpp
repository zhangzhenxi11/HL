#include "loadlockB.h"
#include "MyUnit/ui_loadlockB.h"

#include <QMouseEvent>
#include <QMenu>
#include <QAction>

LoadLockB::LoadLockB(QWidget *parent) :
    QWidget(parent),
    IsLeft(true),
	Selectslot(0),
	currentslot(1),
    ui(new Ui::LoadLockB)
{
    ui->setupUi(this);
}

LoadLockB::~LoadLockB()
{
    delete ui;
}
int LoadLockB::GetCurrentSlot(){
	return currentslot;
}
void LoadLockB::SetCurrentSlot(int slot){
	currentslot = slot;
	update();
}
int LoadLockB::GetSelectSlot(){
	return Selectslot;
}
void LoadLockB::SetSelectSlot(int slot){
	Selectslot = slot;
}

void LoadLockB::SetName(std::string Name){
	name = Name;
}
std::string LoadLockB::GetName(){
	return name;
}
void LoadLockB::SetIsLeft(bool left){
    IsLeft=left;
   update();
}
void LoadLockB::mousePressEvent(QMouseEvent *event){
	if (event->button() == Qt::RightButton) {
		QMenu menu(this);
		QAction *action1 = menu.addAction("打开盒门");
        QAction *action2 = menu.addAction("关闭盒门");
		QAction *action3 = menu.addAction("扫描晶圆");
		QAction *action4 = menu.addAction("移动到达选定槽号");//到,状态
		QAction *action21 = menu.addAction("移动到达取放盒位");//到,状态移动至SMIF取放位
		QAction *action5 = menu.addAction("打开传输腔门阀");
		QAction *action6 = menu.addAction("关闭传输腔门阀");
		QAction *action7 = menu.addAction("打开隔膜阀");
		QAction *action8 = menu.addAction("关闭隔膜阀");
	/*	QAction *action9 = menu.addAction("打开排气阀");
		QAction *action10 = menu.addAction("关闭排气阀");*/
		QAction *action11 = menu.addAction("打开角阀");
		QAction *action12 = menu.addAction("关闭角阀");
		QAction *action16 = menu.addAction("打开插板阀");
		QAction *action17 = menu.addAction("关闭插板阀");
		QAction *action18 = menu.addAction("打开高真空挡板阀");
		QAction *action19 = menu.addAction("关闭高真空挡板阀");
		QAction *action13 = menu.addAction("获取模组状况");
		QAction *action14 = menu.addAction("复位");
		QAction *action15 = menu.addAction("清除错误");

		connect(action1, &QAction::triggered, this, [this](){
			emit signalLoadLockBOpenbox(name);
		});
		connect(action2, &QAction::triggered, this, [this]() {
			emit signalLoadLockBClosebox(name);
		});
		connect(action3, &QAction::triggered, this, [this]() {
			emit signalLoadLockBMapping(name);
		});
		connect(action4, &QAction::triggered, this, [this]() {
			emit signalLoadLockBMoveToSlot(name,this);
		});
		connect(action5, &QAction::triggered, this, [this]() {
			emit signalLoadLockBOpenTMCavityDoor(name);
		});
		connect(action6, &QAction::triggered, this, [this]() {
			emit signalLoadLockBCloseTMCavityDoor(name);
		});
		connect(action7, &QAction::triggered, this, [this]() {
			emit signalLoadLockBOpenDiaphragmValve(name);
		});
		connect(action8, &QAction::triggered, this, [this]() {
			emit signalLoadLockBCloseDiaphragmValve(name);
		});
		/*connect(action9, &QAction::triggered, this, [this]() {
			emit signalLoadLockBOpenExhaustValve(name);
		});
		connect(action10, &QAction::triggered, this, [this]() {
			emit signalLoadLockBCloseExhaustValve(name);
		});*/
		connect(action11, &QAction::triggered, this, [this]() {
			emit signalLoadLockBOpenAngleValve(name);
		});
		connect(action12, &QAction::triggered, this, [this]() {
			emit signalLoadLockBCloseAngleValve(name);
		});
		connect(action13, &QAction::triggered, this, [this]() {
			emit signalLoadLockBGetStatus(name);
		});
		connect(action14, &QAction::triggered, this, [this]() {
			emit signalLoadLockBReset(name);
		});
		connect(action15, &QAction::triggered, this, [this]() {
			emit signalLoadLockBClearError(name);
		});

		connect(action16, &QAction::triggered, this, [this]() {
			emit signalLoaLockBOpenInsertingPlateValve(name);
		});
		connect(action17, &QAction::triggered, this, [this]() {
			emit signalLoaLockBCloseInsertingPlateValve(name);
		});
		connect(action18, &QAction::triggered, this, [this]() {
			emit signalLoaLockBOpenHeightVacuumBaffleValve(name);
		});
		connect(action19, &QAction::triggered, this, [this]() {
			emit signalLoaLockBCloseHeightVacuumBaffleValve(name);
		});
		connect(action21, &QAction::triggered, this, [this]() {
			emit signalLoadLockAMoveToOrigin(name);
		});
		// 在鼠标点击的位置显示菜单
		menu.exec(event->globalPos());
	}
	else if (event->button() == Qt::LeftButton){
		emit signalRightClick();
	}
	// 确保调用基类的实现
	QWidget::mousePressEvent(event);
}
void LoadLockB::paintEvent(QPaintEvent *event){
    QPainter painter(this);
    int w=width();
    int h=height();

    QPixmap originalPixmap("image/LLB.png");
    //QImage mirroredImage = originalPixmap.toImage().mirrored(!IsLeft, false); // 先转换为 QImage，然后进行镜像,第二个参数设置为true（进行水平翻转
    //QPixmap mirroredPixmap = QPixmap::fromImage(mirroredImage); // 再将 QImage 转换回 QPixmap
    // 现在使用镜像后的 QPixmap 绘制图片
	painter.drawPixmap(0, 0, w, h, originalPixmap);
	// 设置字体大小
	QFont font = painter.font();
	font.setPointSize(15);
	painter.setFont(font);
	painter.setPen(Qt::green);
	int textX = w*0.66;
	int textY = h*0.86;
	if (!IsLeft){
		textX = w*0.2;
	}
	QPointF textPosSlot(textX, textY);
	QString slot = QString::number(currentslot);
	painter.drawText(textPosSlot, slot);
	
}
