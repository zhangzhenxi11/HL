#include "loadlock.h"
#include "MyUnit/ui_loadlock.h"

#include <QMouseEvent>
#include <QMenu>
#include <QAction>

LoadLock::LoadLock(QWidget *parent) :
    QWidget(parent),
    IsLeft(true),
	Selectslot(0),
	currentslot(1),
    ui(new Ui::LoadLock)
{
    ui->setupUi(this);
}

LoadLock::~LoadLock()
{
    delete ui;
}
int LoadLock::GetCurrentSlot(){
	return currentslot;
}
void LoadLock::SetCurrentSlot(int slot){
	currentslot = slot;
	update();
}
int LoadLock::GetSelectSlot(){
	return Selectslot;
}
void LoadLock::SetSelectSlot(int slot){
	Selectslot = slot;
}

void LoadLock::SetName(std::string Name){
	name = Name;
}
std::string LoadLock::GetName(){
	return name;
}
void LoadLock::SetIsLeft(bool left){
    IsLeft=left;
   update();
}
void LoadLock::mousePressEvent(QMouseEvent *event){
	if (event->button() == Qt::RightButton) {
		QMenu menu(this);
		QAction *action1 = menu.addAction("打开盒门");
        QAction *action2 = menu.addAction("关闭盒门");
		QAction *action3 = menu.addAction("扫描晶圆");
		QAction *action4 = menu.addAction("移动到达选定槽号");//到,状态
		QAction *action5 = menu.addAction("打开传输腔门阀");
		QAction *action6 = menu.addAction("关闭传输腔门阀");
		QAction *action7 = menu.addAction("打开隔膜阀");
		QAction *action8 = menu.addAction("关闭隔膜阀");
	/*	QAction *action9 = menu.addAction("打开排气阀");
		QAction *action10 = menu.addAction("关闭排气阀");*/
		QAction *action11 = menu.addAction("打开角阀");
		QAction *action12 = menu.addAction("关闭角阀");
		QAction *action13 = menu.addAction("获取模组状况");
		QAction *action14 = menu.addAction("复位");
		QAction *action15 = menu.addAction("清除错误");

		connect(action1, &QAction::triggered, this, [this](){
			emit signalLoadLockOpenbox(name);
		});
		connect(action2, &QAction::triggered, this, [this]() {
			emit signalLoadLockClosebox(name);
		});
		connect(action3, &QAction::triggered, this, [this]() {
			emit signalLoadLockMapping(name);
		});
		connect(action4, &QAction::triggered, this, [this]() {
			emit signalLoadLockMoveToSlot(name,this);
		});
		connect(action5, &QAction::triggered, this, [this]() {
			emit signalLoadLockOpenTMCavityDoor(name);
		});
		connect(action6, &QAction::triggered, this, [this]() {
			emit signalLoadLockCloseTMCavityDoor(name);
		});
		connect(action7, &QAction::triggered, this, [this]() {
			emit signalLoadLockOpenDiaphragmValve(name);
		});
		connect(action8, &QAction::triggered, this, [this]() {
			emit signalLoadLockCloseDiaphragmValve(name);
		});
		/*connect(action9, &QAction::triggered, this, [this]() {
			emit signalLoadLockOpenExhaustValve(name);
		});
		connect(action10, &QAction::triggered, this, [this]() {
			emit signalLoadLockCloseExhaustValve(name);
		});*/
		connect(action11, &QAction::triggered, this, [this]() {
			emit signalLoadLockOpenAngleValve(name);
		});
		connect(action12, &QAction::triggered, this, [this]() {
			emit signalLoadLockCloseAngleValve(name);
		});
		connect(action13, &QAction::triggered, this, [this]() {
			emit signalLoadLockGetStatus(name);
		});
		connect(action14, &QAction::triggered, this, [this]() {
			emit signalLoadLockReset(name);
		});
		connect(action15, &QAction::triggered, this, [this]() {
			emit signalLoadLockClearError(name);
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
void LoadLock::paintEvent(QPaintEvent *event){
    QPainter painter(this);
    int w=width();
    int h=height();

    QPixmap originalPixmap("image/LoadLock.png");
    QImage mirroredImage = originalPixmap.toImage().mirrored(!IsLeft, false); // 先转换为 QImage，然后进行镜像,第二个参数设置为true（进行水平翻转
    QPixmap mirroredPixmap = QPixmap::fromImage(mirroredImage); // 再将 QImage 转换回 QPixmap
    // 现在使用镜像后的 QPixmap 绘制图片
    painter.drawPixmap(0, 0, w, h, mirroredPixmap);
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
