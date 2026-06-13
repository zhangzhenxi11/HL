#include "gdt_pmwidget.h"
#include "MyUnit/ui_gdt_pmwidget.h"
#include <QTimer>
#include <QFontMetrics>
#include <QMouseEvent>
#include <QMenu>
#include <QAction>
#if _MSC_VER >1600
#pragma execution_character_set("utf-8")
#endif

PMGDTWidget::PMGDTWidget(QWidget *parent) :
    QWidget(parent),
    isbusy(false),
    isAlarm(true),
    rotationAngle(0),
	isWafer(false),
	yOffsetValue(70),
	ui(new Ui::PMGDTWidget)
{
   ui->setupUi(this);
   // 初始化动画
   animationGroup = new QSequentialAnimationGroup(this);
   //timer = new QTimer(this);
   //connect(timer, &QTimer::timeout, this, &PMGDTWidget::updateColor);
   //timer->start(100); // 设置定时器间隔，单位为毫秒
}

PMGDTWidget::~PMGDTWidget()
{
    delete ui;
}
void PMGDTWidget::SetName(std::string Name){
	name = Name;
}
std::string PMGDTWidget::GetName(){
	return name;
}
void PMGDTWidget::mousePressEvent(QMouseEvent *event){
	if (event->button() == Qt::RightButton) {
		QMenu menu(this);
		//QAction *action1 = menu.addAction("打开传输腔门阀");
		//QAction *action2 = menu.addAction("关闭传输腔门阀");
		QAction *action3 = menu.addAction("移动到上下料位 ");
		QAction *action4 = menu.addAction("移动到工艺位");
		QAction *action5 = menu.addAction("移动到旋转位");
		QAction *action6 = menu.addAction("旋转到目标角度");
		QAction* action7 = menu.addAction("复位");
		QAction* action8 = menu.addAction("Z轴回原");
		QAction* action9 = menu.addAction("R轴回原");
		QAction* action10 = menu.addAction("清除Z轴报警");
		QAction* action11 = menu.addAction("清除R轴报警");
		QAction* action12 = menu.addAction("清除有片标记");

		/*connect(action1, &QAction::triggered, this, [this](){
			emit signalPMOpenTMCavityDoor(name);
		});
		connect(action2, &QAction::triggered, this, [this]() {
			emit signalPMCloseTMCavityDoor(name);
		});*/
		connect(action3, &QAction::triggered, this, [this]() {
			emit signalPMGetFinished(name);
		});
		connect(action4, &QAction::triggered, this, [this]() {
			emit signalPMUplaodFinished(name);
		});
		connect(action5, &QAction::triggered, this, [this]() {
			emit signalPMRotatePosFinished(name);
			//emit signalPMGetStatus(name);
		});
		connect(action6, &QAction::triggered, this, [this]() {
			emit signalPMRotatingDegreeFinished(name);
			});
		connect(action7, &QAction::triggered, this, [this]() {
			emit signalPMReset(name);
		});

		connect(action8, &QAction::triggered, this, [this]() {
			emit signalPMZaxisReset(name);
			});
		connect(action9, &QAction::triggered, this, [this]() {
			emit signalPMRaxisReset(name);
			});
		connect(action10, &QAction::triggered, this, [this]() {
			emit signalPMClearZaxisError(name);
			});
		connect(action11, &QAction::triggered, this, [this]() {
			emit signalPMClearRaxisError(name);
			});
		connect(action12, &QAction::triggered, this, [this]() {
			emit signalPMClearState(name);
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




void PMGDTWidget::setRotationAngle(int angle){
    rotationAngle=angle;
	update();
}
void PMGDTWidget::setWafer(bool wafer){
    isWafer=wafer;
	update();
}


int PMGDTWidget::yOffset() const { return yOffsetValue; }

void PMGDTWidget::setYOffset(int offset) {
	//centerY*0.8;
	//int X_A = 1, Y_A = 20;
	//int X_B = 30, Y_B = 170;
	//int X_C = 40, Y_C = 220;

	//int yOffsetValue = 0; // Resulting Y coordinate

	//// Interpolating based on the range of the actual position
	//if (offset >= X_B && offset <= X_C) {
	//	yOffsetValue = Y_C + (Y_B - Y_C) * (offset - X_C) / (X_B - X_C);
	//}
	//else if (offset >= X_A && offset < X_B) {
	//	yOffsetValue = Y_B + (Y_A - Y_B) * (offset - X_B) / (X_A - X_B);
	//}
	yOffsetValue = offset;
	update(); // 重绘
}

// 外部调用此方法来设置新Y位置，并启动动画

//实现平滑移动效果
void PMGDTWidget::animateToYOffset(int newYOffset,int speed) {
	printf("newYOffset %d \r\n", newYOffset);
	QPropertyAnimation* extendAnimation = new QPropertyAnimation(this, "yOffset");
	//extendAnimation->setStartValue(yOffsetValue);
	extendAnimation->setDuration(speed);
	extendAnimation->setEndValue(newYOffset);
	animationGroup->clear();
	animationGroup->addAnimation(extendAnimation);
	animationGroup->start();
}

void PMGDTWidget::paintEvent(QPaintEvent *){
    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform); // 2025-10-29: 启用平滑缩放
    
    QPixmap pixmap("image/image/PM1.png");

	// 创建一个QTransform对象
	QTransform transform;
	// 旋转图像
	transform.rotate(rotationAngle);
	// 应用变换到原始图像，创建旋转后的图像
	QPixmap rotatedPixmap = pixmap.transformed(transform, Qt::SmoothTransformation);

    // 2025-10-29: 缩放图片以适应控件大小
    QPixmap scaledPixmap = rotatedPixmap.scaled(this->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    
    int centerX = scaledPixmap.width() / 2;
    int centerY = scaledPixmap.height() / 2;
	int RectX = centerX*0.87;
	//int RectY = centerY*0.8;
	//double RectY = 20;
	//double RectY = 170;
	//double RectY = 220;
	double RectY = yOffsetValue;
	//printf("RectY %.2f \r\n", centerY*0.8);

	int TextX = centerX*0.6;
	int TextY = centerY*0.18;
	int statusX = centerX*0.31;
	int statusY = centerY*1.62;
	int lineLength = rotatedPixmap.width()*0.7;
	int lineSpacing = 50;
    painter.save();

    // 2025-10-29: 绘制缩放后的图片，居中显示
    int x = (this->width() - scaledPixmap.width()) / 2;
    int y = (this->height() - scaledPixmap.height()) / 2;
    painter.drawPixmap(x, y, scaledPixmap);
    
    // 恢复到保存的画家状态，这时候的状态是未旋转的
    painter.restore();

    // 设置画笔颜色
    if(isWafer){
        painter.setPen(Qt::green);
        painter.setBrush(Qt::green);
		painter.drawEllipse(RectX, RectY, 50, 50);
    }

	painter.setRenderHint(QPainter::Antialiasing);

	painter.setPen(QPen(Qt::green, 2));
	painter.setBrush(Qt::green);

	// 2025-10-29: 根据缩放比例调整线条参数
	double scaleRatio = (double)this->width() / rotatedPixmap.width();
	int widgetWidth = width();
	int y1 = RectY;
	int y2 = y1 + lineSpacing * scaleRatio;

	int x1 = (widgetWidth - lineLength * scaleRatio) / 2;
	int x2 = x1 + lineLength * scaleRatio;

	//painter.drawLine(x1, y1, x2, y1);  // 绘制第一条线
	//painter.drawLine(x1, y2, x2, y2);  // 绘制第二条线


	painter.setPen(QPen(Qt::red, 8 * scaleRatio));
	painter.setBrush(Qt::red);

	int y3 = 140 * scaleRatio;
	//painter.drawLine(x1, y3, x2, y3);  // 绘制第一条线


    // 设置字体大小
   // QFont font = painter.font();
   // font.setPointSize(11);
   // font.setBold(true); // 设置字体为加粗
   // painter.setFont(font);

   //QFontMetrics metrics(font);
   //QPointF textPosS(TextX, TextY);
   // QPointF status(statusX,statusY );


}

void PMGDTWidget::updateColor()
{
	update();  // 更新绘制
}

void PMGDTWidget::setStatus(bool busy){
    isbusy=busy;
}
bool PMGDTWidget::getStatus(){
    return isbusy;
}
