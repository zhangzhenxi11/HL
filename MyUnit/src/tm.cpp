#include "tm.h"
#include "MyUnit/ui_tm.h"
#include "Kernel/kernel_log.h"
#include <QPainter>
#include <QPolygon>
#include <QTimer>
#include <QTransform>
#include <QtMath>

#include <QMouseEvent>
#include <QMenu>
#include <QAction>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

TM::TM(QWidget *parent) :
QWidget(parent), 
IseventFilter(false),
 ui(new Ui::TM)
{
	timer1 = new QTimer(this);
	connect(timer1, &QTimer::timeout, this, &TM::timeFS);
	animation = new QPropertyAnimation(this, "rotationAngle");
	animation->setDuration(3000); // 动画持续时间
	animation->setStartValue(0);
	animation->setEndValue(360); // 一圈
	animation->setLoopCount(-1); // 无限循环

    ui->setupUi(this);
	installEventFilter(this);
}

TM::~TM()
{
    delete ui;
}

qreal TM::rotationAngle() const {
	return m_rotationAngle; 
}
void TM::setRotationAngle(qreal angle) {
	m_rotationAngle = angle;
	update(); // 更新绘制
}
int TM::getIsWaferAligner() const{
	return IsWaferAligner;
}
void TM::setIsWaferAligner(int wafer){
	IsWaferAligner = wafer;
	update(); // 重绘
}
void TM::SetName(std::string Name){
	name = Name;
}
std::string TM::GetName(){
	return name;
}

bool TM::eventFilter(QObject *watched, QEvent *event) {
	if (watched == this && event->type() == QEvent::MouseButtonPress) {
		if (IseventFilter){
			IseventFilter = false;
			return true;
		}
		
	}
	return QWidget::eventFilter(watched, event);
}

void TM::mousePressEvent(QMouseEvent *event){
	QPoint circleCenter = QPoint(width() / 2, height() / 2);
	int distance = (event->pos() - circleCenter).manhattanLength();
	// 如果距离小于半径，则认为点击在圆内
	if (distance > 128*1.2) {
		if (event->button() == Qt::RightButton) {
			QMenu menu(this);
		/*	QAction *actionAlignerAlign = menu.addAction("寻边");
			QAction *actionAlignerReset = menu.addAction("寻边复位");*/
			QAction *action1 = menu.addAction("打开隔膜阀");
			QAction *action2 = menu.addAction("关闭隔膜阀");
			QAction *action3 = menu.addAction("打开高真空挡板阀");
			QAction *action4 = menu.addAction("关闭高真空挡板阀");
			QAction *action5 = menu.addAction("打开角阀");
			QAction *action6 = menu.addAction("关闭角阀");
			QAction *action7 = menu.addAction("打开插板阀");
			QAction *action8 = menu.addAction("关闭插板阀");
			QAction *action9 = menu.addAction("打开流量隔膜阀");
			QAction *action10 = menu.addAction("关闭流量隔膜阀");
			QAction *action11 = menu.addAction("获取模组状况");
			QAction *action12 = menu.addAction("TM复位");

			/*connect(actionAlignerReset, &QAction::triggered, this, [this](){
				emit signalAlignerReset(name);
			});
			connect(actionAlignerAlign, &QAction::triggered, this, [this]() {
				emit signalAlignerAlign(name);
			});*/

			connect(action1, &QAction::triggered, this, [this](){
				emit signalTMOpenDiaphragmValve(name);
			});
			connect(action2, &QAction::triggered, this, [this]() {
				emit signalTMCloseDiaphragmValve(name);
			});
			connect(action3, &QAction::triggered, this, [this]() {
				emit signalTMOpenHeightVacuumBaffleValve(name);
			});
			connect(action4, &QAction::triggered, this, [this]() {
				emit signalTMCloseHeightVacuumBaffleValve(name);
			});
			connect(action5, &QAction::triggered, this, [this]() {
				emit signalTMOpenAngleValve(name);
			});
			connect(action6, &QAction::triggered, this, [this]() {
				emit signalTMCloseAngleValve(name);
			});
			connect(action7, &QAction::triggered, this, [this](){
				emit signalTMOpenInsertingPlateValve(name);
			});
			connect(action8, &QAction::triggered, this, [this]() {
				emit signalTMCloseInsertingPlateValve(name);
			});
			connect(action9, &QAction::triggered, this, [this]() {
				emit signalTMOpenFlowmeterDiaphragmValve(name);
			});
			connect(action10, &QAction::triggered, this, [this]() {
				emit signalTMCloseFlowmeterDiaphragmValve(name);
			});
			connect(action11, &QAction::triggered, this, [this]() {
				emit signalTMGetStatus(name);
			});
			connect(action12, &QAction::triggered, this, [this]() {
				emit signalTMReset(name);
			});
			// 在鼠标点击的位置显示菜单
			menu.exec(event->globalPos());
		}
		else if (event->button() == Qt::LeftButton){
			emit signalRightClick();
		}
	}
	
	// 确保调用基类的实现
	QWidget::mousePressEvent(event);
}

void TM::startRotationAnimation(int numRotations) {
	animation->setEndValue(360 * numRotations); // 设置旋转圈数
	animation->start();

}

void TM::paintEvent(QPaintEvent *){
    QPainter painter(this);
    int centerX = width() / 3.16;
    int centerY = height() / 1.265;
    QPixmap pixmap("image/TM.png");
    painter.drawPixmap(0,0,pixmap);
	if (IsWaferAligner){
		// 设置旋转
		int notchWidth = 6;
		int notchDepth = 8;
		int diameter = 52;
		int radius = diameter / 2;
		QPointF center(centerX, centerY);
		painter.setPen(Qt::NoPen);
		painter.setBrush(QColor(0, 255, 0));

		painter.drawEllipse(center, radius, radius);

		double angle = m_rotationAngle * M_PI / 180.0;
		QPointF notchStart = center + QPointF(radius * cos(angle), radius * sin(angle));

		QPointF notchLeft = notchStart + QPointF(notchWidth / 2 * cos(angle + M_PI_2), notchWidth / 2 * sin(angle + M_PI_2));
		QPointF notchRight = notchStart + QPointF(notchWidth / 2 * cos(angle - M_PI_2), notchWidth / 2 * sin(angle - M_PI_2));
		QPointF notchTip = notchStart + QPointF(-notchDepth * cos(angle), -notchDepth * sin(angle));

		QPolygonF notch;
		notch << notchLeft << notchTip << notchRight << notchStart;

		painter.setBrush(Qt::white);
		painter.drawPolygon(notch);
		
		

	}
	

}

void TM::timeFS()
{
	m_rotationAngle += 5; // 每次增加2度来模拟旋转
	if (m_rotationAngle >= 360) {
		m_rotationAngle = 0; // 重置角度
	}
	//更新ui
	update();
}