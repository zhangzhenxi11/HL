#include "loadlockA.h"
#include "MyUnit/ui_loadlockA.h"

#include <QMouseEvent>
#include <QMenu>
#include <QAction>

LoadLockA::LoadLockA(QWidget *parent) :
    QWidget(parent),
    IsLeft(true),
	Selectslot(0),
	currentslot(1),
    ui(new Ui::LoadLockA)
{
    ui->setupUi(this);
}

LoadLockA::~LoadLockA()
{
    delete ui;
}
int LoadLockA::GetCurrentSlot(){
	return currentslot;
}
void LoadLockA::SetCurrentSlot(int slot){
	currentslot = slot;
	update();
}
int LoadLockA::GetSelectSlot(){
	return Selectslot;
}
void LoadLockA::SetSelectSlot(int slot){
	Selectslot = slot;
}

void LoadLockA::SetName(std::string Name){
	name = Name;
}
std::string LoadLockA::GetName(){
	return name;
}
void LoadLockA::SetIsLeft(bool left){
    IsLeft=left;
   update();
}
void LoadLockA::mousePressEvent(QMouseEvent *event){
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
			emit signalLoadLockAOpenbox(name);
		});
		connect(action2, &QAction::triggered, this, [this]() {
			emit signalLoadLockAClosebox(name);
		});
		connect(action3, &QAction::triggered, this, [this]() {
			emit signalLoadLockAMapping(name);
		});
		connect(action4, &QAction::triggered, this, [this]() {
			emit signalLoadLockAMoveToSlot(name,this);
		});
		connect(action5, &QAction::triggered, this, [this]() {
			emit signalLoadLockAOpenTMCavityDoor(name);
		});
		connect(action6, &QAction::triggered, this, [this]() {
			emit signalLoadLockACloseTMCavityDoor(name);
		});
		connect(action7, &QAction::triggered, this, [this]() {
			emit signalLoadLockAOpenDiaphragmValve(name);
		});
		connect(action8, &QAction::triggered, this, [this]() {
			emit signalLoadLockACloseDiaphragmValve(name);
		});
		/*connect(action9, &QAction::triggered, this, [this]() {
			emit signalLoadLockAOpenExhaustValve(name);
		});
		connect(action10, &QAction::triggered, this, [this]() {
			emit signalLoadLockACloseExhaustValve(name);
		});*/
		connect(action11, &QAction::triggered, this, [this]() {
			emit signalLoadLockAOpenAngleValve(name);
		});
		connect(action12, &QAction::triggered, this, [this]() {
			emit signalLoadLockACloseAngleValve(name);
		});
		connect(action13, &QAction::triggered, this, [this]() {
			emit signalLoadLockAGetStatus(name);
		});
		connect(action14, &QAction::triggered, this, [this]() {
			emit signalLoadLockAReset(name);
		});
		connect(action15, &QAction::triggered, this, [this]() {
			emit signalLoadLockAClearError(name);
		});

		connect(action16, &QAction::triggered, this, [this]() {
			emit signalLoaLockAOpenInsertingPlateValve(name);
		});
		connect(action17, &QAction::triggered, this, [this]() {
			emit signalLoaLockACloseInsertingPlateValve(name);
		});
		connect(action18, &QAction::triggered, this, [this]() {
			emit signalLoaLockAOpenHeightVacuumBaffleValve(name);
		});
		connect(action19, &QAction::triggered, this, [this]() {
			emit signalLoaLockACloseHeightVacuumBaffleValve(name);
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
void LoadLockA::paintEvent(QPaintEvent *event){
    QPainter painter(this);
    int w=width();
    int h=height();

    QPixmap originalPixmap("image/LLA.png");
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
