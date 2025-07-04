#include "vtmrobot.h"
#include "MyUnit/ui_vtmrobot.h"
#include "Kernel/kernel_log.h"

#include <QTransform>
#include <QtMath>

#include <QMouseEvent>
#include <QMenu>
#include <QAction>
#include "QMessageBox"


vtmrobot::vtmrobot(QWidget *parent) :
    QWidget(parent),
    armWidth(6),
    armLength(100),
    armAngle(83),
    arm2Angle(83),
    baseRotationAngle(0),
	armSpeed(1500),
	baseSpeed(1500),
    isWaferarm1(0),
    isWaferarm2(0),
	currentRotationAngle(0),
	currentStation(-1),
    ui(new Ui::vtmrobot)
{
	animationGroupReset = new QSequentialAnimationGroup(this);
	animationGroup = new QSequentialAnimationGroup(this);
	//rotateAnimation = new QPropertyAnimation(this, "baseRotationAngle");
}

void vtmrobot::mousePressEvent(QMouseEvent *event){
	QPoint circleCenter = QPoint(width() / 2, height() / 2);
	int distance = (event->pos() - circleCenter).manhattanLength();
	// 如果距离小于半径，则认为点击在圆内
	if (distance <= armLength*1.2) {
		//printf("armLength %d \n", armLength);
		if (event->button() == Qt::RightButton) {
				QMenu menu(this);
				QAction *action1 = menu.addAction("动作控制");
				QAction *action2 = menu.addAction("获取状况");
				QAction *action3 = menu.addAction("复位");
				QAction *action4 = menu.addAction("清除错误");
				//QAction *action5 = menu.addAction("AWC数据记录");
				//QAction *action6 = menu.addAction("手臂晶圆设置");

				connect(action1, &QAction::triggered, this, [this](){
					emit signalRobotDialog(); //发送机械手弹窗信号
				});
				connect(action2, &QAction::triggered, this, [this]() {
					emit signalRobotGetStatus();
				});
				connect(action3, &QAction::triggered, this, [this]() {
					emit signalRobotReset();
				});
				connect(action4, &QAction::triggered, this, [this]() {
					emit signalRobotClearError();
				});
				/*connect(action5, &QAction::triggered, this, [this]() {
					emit signalRobotData();
				});
				connect(action6, &QAction::triggered, this, [this]() {
					emit signalRobotStatusDialog();
				});*/
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

int vtmrobot::getBaseRotationAngle() const {
        return baseRotationAngle;
    }
int vtmrobot::getCurrentRotationAngle() const{
	return currentRotationAngle;
}

void vtmrobot::setBaseRotationAngle(int angle) {
        baseRotationAngle = angle;
        update(); // 重绘
}
int vtmrobot::getArmAngle() const{
    return armAngle;
}
void vtmrobot::setArmAngle(int angle){
    armAngle = angle;
    update(); // 重绘
}
int vtmrobot::getArm2Angle() const{
    return arm2Angle;
}
void vtmrobot::setArm2Angle(int angle){
    arm2Angle = angle;
    update(); // 重绘
}

int vtmrobot::getIsWaferArm1() const{
    return isWaferarm1;
}
void vtmrobot::setIsWaferArm1(int wafer){
    isWaferarm1 = wafer;
    update(); // 重绘
}
int vtmrobot::getIsWaferArm2() const{
    return isWaferarm2;
}
void vtmrobot::setIsWaferArm2(int wafer){
    isWaferarm2 = wafer;
    update(); // 重绘
}

int vtmrobot::getCurrentStation() const{
	return currentStation;
}
void vtmrobot::setCurrentStation(int station){
	currentStation = station;
}
int vtmrobot::getCurrentArm() const{
	return currentArm;
}
void vtmrobot::setCurrentArm(int arm){
	currentArm = arm;
}

void vtmrobot::setArmSpeed(int speed){
	armSpeed = speed;
}
void vtmrobot::setBaseSpeed(int speed){
	baseSpeed = speed;
}
int vtmrobot::getArmSpeed(){
	return armSpeed;
}
int vtmrobot::getBaseSpeed(){
	return baseSpeed;
}
void vtmrobot::SetName(std::string Name){
	name = Name;
}
std::string vtmrobot::GetName(){
	return name;
}

void vtmrobot::animationAbort(){
	if (animationGroup->state() == QAbstractAnimation::Paused){
		animationGroup->stop();
		animationGroup->clear();
	}
	if (animationGroupReset->state() == QAbstractAnimation::Paused || animationGroupReset->state() == QAbstractAnimation::Running){
		animationGroupReset->stop();
		animationGroupReset->clear();
	}
}
void vtmrobot::animationPause(){
	if (animationGroup->state() == QAbstractAnimation::Running){
		animationGroup->pause();
	}
	if (animationGroupReset->state() == QAbstractAnimation::Running){
		animationGroupReset->pause();
	}
}
void vtmrobot::animationResume(){
	
	if (animationGroup->state() == QAbstractAnimation::Paused){
		animationGroup->resume();
	}
	if (animationGroupReset->state() == QAbstractAnimation::Paused){
		animationGroupReset->resume();
	}
}
void vtmrobot::reset(){
	if (animationGroupReset->state() == QAbstractAnimation::Running){
		logWarn(name.c_str(), "复位中，操作重复!");
		return;
	}
	if (animationGroup->state() != QAbstractAnimation::Stopped){
		logWarn(name.c_str(), "请先中止当前动作后再进行复位!");
		return;
	}
	animationGroupReset->clear();
	if (armAngle != 83){
		QPropertyAnimation *resetArm1 = new QPropertyAnimation(this, "armAngle");
		resetArm1->setDuration(3000);//复位速度单独控制
		resetArm1->setEndValue(83);
		animationGroupReset->addAnimation(resetArm1);
	}
	if (arm2Angle != 83){
		QPropertyAnimation *resetArm2 = new QPropertyAnimation(this, "arm2Angle");
		resetArm2->setDuration(3000);
		resetArm2->setEndValue(83);
		animationGroupReset->addAnimation(resetArm2);
	}
	if (baseRotationAngle != 0){
		QPropertyAnimation *resetbase = new QPropertyAnimation(this, "baseRotationAngle");
		resetbase->setDuration(2000);
		resetbase->setEndValue(0);
		animationGroupReset->addAnimation(resetbase);
		connect(resetbase, &QPropertyAnimation::finished, this, [this](){currentRotationAngle = 0; currentStation=-1; });
	}
	animationGroupReset->start();
}

//void vtmrobot::get(int arm, int rotationangle, int armangle) {
//	if (animationGroupGet->state() == QAbstractAnimation::Running){
//		//QMessageBox::information(this, "warn", "重复操作!");
//		logWarn(name.c_str(), "取料中，操作重复!");
//		return;
//	}
//	QPropertyAnimation *extendAnimation = 0;
//	QPropertyAnimation *waferAnimation = 0;
//	QPropertyAnimation *rotateBackAnimation = 0;
//	if (armangle == 20)armSpeed -= 200;
//	if (arm == 0){
//		//if (armAngle!=83){
//		//	//QMessageBox::information(this, "warn", "手爪A状态异常，请先复位!");
//		//	logWarn(name.c_str(), "手爪A状态异常，请先复位后再放料!");
//		//	return;
//		//}
//		extendAnimation = new QPropertyAnimation(this, "armAngle");
//		extendAnimation->setDuration(armSpeed);
//		extendAnimation->setEndValue(armAngle - armangle);
//
//		waferAnimation = new QPropertyAnimation(this, "isWaferarm1");
//		waferAnimation->setDuration(200);
//		waferAnimation->setEndValue(1);
//
//		rotateBackAnimation = new QPropertyAnimation(this, "armAngle");
//		int armSpeed2 = armSpeed >= 2500 ? armSpeed - 500 : armSpeed;
//		rotateBackAnimation->setDuration(armSpeed);
//		rotateBackAnimation->setEndValue(armAngle);
//	}
//	else{
//		//if (arm2Angle != 83){
//		//	//QMessageBox::information(this, "warn", "手爪B状态异常，请先复位!");
//		//	logWarn(name.c_str(), "手爪B状态异常，请先复位后再放料!");
//		//	return;
//		//}
//		extendAnimation = new QPropertyAnimation(this, "arm2Angle");
//		extendAnimation->setDuration(armSpeed);
//		extendAnimation->setEndValue(arm2Angle - armangle);
//
//		waferAnimation = new QPropertyAnimation(this, "isWaferarm2");
//		waferAnimation->setDuration(200);
//		waferAnimation->setEndValue(1);
//
//		rotateBackAnimation = new QPropertyAnimation(this, "arm2Angle");
//		int armSpeed2 = armSpeed >= 2500 ? armSpeed - 500 : armSpeed;
//		rotateBackAnimation->setDuration(armSpeed);
//		rotateBackAnimation->setEndValue(arm2Angle);
//	}
//	animationGroupGet->clear();
//	
//	logInform("Test", "Get currentRotationAngle:%d  rotationangle: %d", currentRotationAngle, rotationangle);
//	int shortestRotation = calculateShortestRotation(currentRotationAngle, rotationangle);
//	logInform("Test", "Get currentRotationAngle:%d  shortestRotation: %d", currentRotationAngle, shortestRotation);
//	if (shortestRotation != currentRotationAngle) {
//		//QPropertyAnimation *rotateAnimation = new QPropertyAnimation(this, "baseRotationAngle");
//		rotateAnimation->setDuration(baseSpeed);
//		rotateAnimation->setEndValue(shortestRotation);
//		animationGroupGet->addAnimation(rotateAnimation);
//		currentRotationAngle = rotationangle;
//	}
//	animationGroupGet->addAnimation(extendAnimation);
//	animationGroupGet->addAnimation(waferAnimation);
//	animationGroupGet->addAnimation(rotateBackAnimation);
//
//	connect(waferAnimation, &QPropertyAnimation::finished, this, [this](){emit extendAnimationGetFinished(); });
//	animationGroupGet->start();
//
//}
//
//void vtmrobot::put(int arm, int rotationangle, int armangle) {
//	if (animationGroupPut->state() == QAbstractAnimation::Running){
//		//QMessageBox::information(this, "warn", "重复操作!");
//		logWarn(name.c_str(), "放料中，操作重复!");
//		return;
//	}
//
//	QPropertyAnimation *retractAnimation = 0;
//	QPropertyAnimation *waferAnimation = 0;
//	QPropertyAnimation *retractAnimation2 = 0;
//	if (arm == 0){
//		//if (armAngle != 83){//初始化角度
//		//	//QMessageBox::information(this, "warn", "手爪A状态异常，请先复位!");
//		//	logWarn(name.c_str(), "手爪A状态异常，请先复位后再放料!");
//		//	return;
//		//}
//		retractAnimation = new QPropertyAnimation(this, "armAngle");
//		retractAnimation->setDuration(armSpeed); // 手臂收回动画持续时间
//		retractAnimation->setEndValue(armAngle - armangle); // 手臂回到原始长度
//
//		waferAnimation = new QPropertyAnimation(this, "isWaferarm1");
//		waferAnimation->setDuration(200); // 手臂伸出动画持续时间
//		waferAnimation->setEndValue(0); // 假设抓取时手臂伸出20单位长度
//
//		retractAnimation2 = new QPropertyAnimation(this, "armAngle");
//		int armSpeed2 = armSpeed >= 2500 ? armSpeed - 500 : armSpeed;
//		retractAnimation2->setDuration(armSpeed2); // 手臂收回动画持续时间
//		retractAnimation2->setEndValue(armAngle); // 手臂回到原始长度
//	}
//	else{
//		//if (arm2Angle != 83){
//		//	//QMessageBox::information(this, "warn", "手爪B状态异常，请先复位!");
//		//	logWarn(name.c_str(), "手爪B状态异常，请先复位后再放料!");
//		//	return;
//		//}
//		retractAnimation = new QPropertyAnimation(this, "arm2Angle");
//		retractAnimation->setDuration(armSpeed); // 手臂收回动画持续时间
//		retractAnimation->setEndValue(arm2Angle - armangle); // 手臂回到原始长度
//
//		waferAnimation = new QPropertyAnimation(this, "isWaferarm2");
//		waferAnimation->setDuration(100); // 手臂伸出动画持续时间
//		waferAnimation->setEndValue(0); // 假设抓取时手臂伸出20单位长度
//
//		retractAnimation2 = new QPropertyAnimation(this, "arm2Angle");
//		int armSpeed2 = armSpeed >= 2500 ? armSpeed - 500 : armSpeed;
//		retractAnimation2->setDuration(armSpeed2); // 手臂收回动画持续时间
//		retractAnimation2->setEndValue(arm2Angle); // 手臂回到原始长度
//	}
//	animationGroupPut->clear();
//
//	logInform("Test", "Put currentRotationAngle:%d  rotationangle: %d", currentRotationAngle, rotationangle);
//	int shortestRotation = calculateShortestRotation(currentRotationAngle, rotationangle);
//	logInform("Test", "Put currentRotationAngle:%d  shortestRotation: %d", currentRotationAngle, shortestRotation);
//
//	if (shortestRotation != currentRotationAngle) {
//		//QPropertyAnimation *rotateBackAnimation = new QPropertyAnimation(this, "baseRotationAngle");
//
//	/*	rotateBackAnimation->setDuration(baseSpeed);
//		rotateBackAnimation->setEndValue(shortestRotation);
//		animationGroupPut->addAnimation(rotateBackAnimation);*/
//		rotateAnimation->setDuration(baseSpeed);
//		rotateAnimation->setEndValue(shortestRotation);
//		animationGroupPut->addAnimation(rotateAnimation);
//		currentRotationAngle = rotationangle;
//	}
//	animationGroupPut->addAnimation(retractAnimation);
//	animationGroupPut->addAnimation(waferAnimation);
//	animationGroupPut->addAnimation(retractAnimation2);
//
//	connect(waferAnimation, &QPropertyAnimation::finished, this, [this](){emit extendAnimationPutFinished(); });
//	animationGroupPut->start();
//
//}


void vtmrobot::get(int arm, int rotationangle, int armangle) {
	//if (animationGroup->state() == QAbstractAnimation::Running){
	//	//QMessageBox::information(this, "warn", "重复操作!");
	//	logWarn(name.c_str(), "取料中，操作重复!");
	//	return;
	//}
	QPropertyAnimation *extendAnimation = 0;
	QPropertyAnimation *waferAnimation = 0;
	QPropertyAnimation *rotateBackAnimation = 0;
	if (armangle == 20)armSpeed -= 200;
	if (arm == 0){
		//if (armAngle!=83){
		//	//QMessageBox::information(this, "warn", "手爪A状态异常，请先复位!");
		//	logWarn(name.c_str(), "手爪A状态异常，请先复位后再放料!");
		//	return;
		//}
		extendAnimation = new QPropertyAnimation(this, "armAngle");
		extendAnimation->setDuration(armSpeed);
		extendAnimation->setEndValue(armAngle - armangle);

		waferAnimation = new QPropertyAnimation(this, "isWaferarm1");
		waferAnimation->setDuration(200);
		waferAnimation->setEndValue(1);

		rotateBackAnimation = new QPropertyAnimation(this, "armAngle");
		int armSpeed2 = armSpeed >= 2500 ? armSpeed - 500 : armSpeed;
		rotateBackAnimation->setDuration(armSpeed);
		rotateBackAnimation->setEndValue(armAngle);
	}
	else{
		//if (arm2Angle != 83){
		//	//QMessageBox::information(this, "warn", "手爪B状态异常，请先复位!");
		//	logWarn(name.c_str(), "手爪B状态异常，请先复位后再放料!");
		//	return;
		//}
		extendAnimation = new QPropertyAnimation(this, "arm2Angle");
		extendAnimation->setDuration(armSpeed);
		extendAnimation->setEndValue(arm2Angle - armangle);

		waferAnimation = new QPropertyAnimation(this, "isWaferarm2");
		waferAnimation->setDuration(200);
		waferAnimation->setEndValue(1);

		rotateBackAnimation = new QPropertyAnimation(this, "arm2Angle");
		int armSpeed2 = armSpeed >= 2500 ? armSpeed - 500 : armSpeed;
		rotateBackAnimation->setDuration(armSpeed);
		rotateBackAnimation->setEndValue(arm2Angle);
	}
	animationGroup->clear();

	int shortestRotation = calculateShortestRotation(currentRotationAngle, rotationangle);
	logInform("Test", "Get currentRotationAngle:%d  shortestRotation: %d baseRotationAngle:%d", currentRotationAngle, shortestRotation, baseRotationAngle);

	if (baseRotationAngle == -270 && currentRotationAngle == 90){
		baseRotationAngle = currentRotationAngle;
	}
	if (baseRotationAngle == 270 && currentRotationAngle == -90){
		baseRotationAngle = currentRotationAngle;
	}
	if (baseRotationAngle == -450 && currentRotationAngle == -90){
		baseRotationAngle = currentRotationAngle;
	}
	if (shortestRotation != currentRotationAngle) {
		QPropertyAnimation *rotateAnimation = new QPropertyAnimation(this, "baseRotationAngle");
		rotateAnimation->setDuration(baseSpeed);
		rotateAnimation->setEndValue(shortestRotation);
		animationGroup->addAnimation(rotateAnimation);
		currentRotationAngle = rotationangle;
	}
	else{
		baseRotationAngle = currentRotationAngle;
	}
	animationGroup->addAnimation(extendAnimation);
	animationGroup->addAnimation(waferAnimation);
	animationGroup->addAnimation(rotateBackAnimation);

	connect(waferAnimation, &QPropertyAnimation::finished, this, [this](){emit extendAnimationGetFinished(); });
	startAnimationGroup();
}

void vtmrobot::put(int arm, int rotationangle, int armangle) {
	//if (animationGroup->state() == QAbstractAnimation::Running){
	//	//QMessageBox::information(this, "warn", "重复操作!");
	//	logWarn(name.c_str(), "放料中，操作重复!");
	//	return;
	//}

	QPropertyAnimation *retractAnimation = 0;
	QPropertyAnimation *waferAnimation = 0;
	QPropertyAnimation *retractAnimation2 = 0;
	if (arm == 0){
		//if (armAngle != 83){//初始化角度
		//	//QMessageBox::information(this, "warn", "手爪A状态异常，请先复位!");
		//	logWarn(name.c_str(), "手爪A状态异常，请先复位后再放料!");
		//	return;
		//}
		retractAnimation = new QPropertyAnimation(this, "armAngle");
		retractAnimation->setDuration(armSpeed); // 手臂收回动画持续时间
		retractAnimation->setEndValue(armAngle - armangle); // 手臂回到原始长度

		waferAnimation = new QPropertyAnimation(this, "isWaferarm1");
		waferAnimation->setDuration(200); // 手臂伸出动画持续时间
		waferAnimation->setEndValue(0); // 假设抓取时手臂伸出20单位长度

		retractAnimation2 = new QPropertyAnimation(this, "armAngle");
		int armSpeed2 = armSpeed >= 2500 ? armSpeed - 500 : armSpeed;
		retractAnimation2->setDuration(armSpeed2); // 手臂收回动画持续时间
		retractAnimation2->setEndValue(armAngle); // 手臂回到原始长度
	}
	else{
		//if (arm2Angle != 83){
		//	//QMessageBox::information(this, "warn", "手爪B状态异常，请先复位!");
		//	logWarn(name.c_str(), "手爪B状态异常，请先复位后再放料!");
		//	return;
		//}
		retractAnimation = new QPropertyAnimation(this, "arm2Angle");
		retractAnimation->setDuration(armSpeed); // 手臂收回动画持续时间
		retractAnimation->setEndValue(arm2Angle - armangle); // 手臂回到原始长度

		waferAnimation = new QPropertyAnimation(this, "isWaferarm2");
		waferAnimation->setDuration(100); // 手臂伸出动画持续时间
		waferAnimation->setEndValue(0); // 假设抓取时手臂伸出20单位长度

		retractAnimation2 = new QPropertyAnimation(this, "arm2Angle");
		int armSpeed2 = armSpeed >= 2500 ? armSpeed - 500 : armSpeed;
		retractAnimation2->setDuration(armSpeed2); // 手臂收回动画持续时间
		retractAnimation2->setEndValue(arm2Angle); // 手臂回到原始长度
	}
	animationGroup->clear();

	int shortestRotation = calculateShortestRotation(currentRotationAngle, rotationangle);
	logInform("Test", "Put currentRotationAngle:%d  shortestRotation: %d baseRotationAngle:%d", currentRotationAngle, shortestRotation, baseRotationAngle);
	
	if (baseRotationAngle == -270 && currentRotationAngle == 90){
		baseRotationAngle = currentRotationAngle;
	}
	if (baseRotationAngle == 270 && currentRotationAngle == -90){
		baseRotationAngle = currentRotationAngle;
	}
	if (baseRotationAngle == -450 && currentRotationAngle == -90){
		baseRotationAngle = currentRotationAngle;
	}
	if (shortestRotation != currentRotationAngle) {
		QPropertyAnimation *rotateBackAnimation = new QPropertyAnimation(this, "baseRotationAngle");
		rotateBackAnimation->setDuration(baseSpeed);
		rotateBackAnimation->setEndValue(shortestRotation);
		animationGroup->addAnimation(rotateBackAnimation);
		currentRotationAngle = rotationangle;
	}
	else{
		baseRotationAngle = currentRotationAngle;
	}
	
	animationGroup->addAnimation(retractAnimation);
	animationGroup->addAnimation(waferAnimation);
	animationGroup->addAnimation(retractAnimation2);

	connect(waferAnimation, &QPropertyAnimation::finished, this, [this](){emit extendAnimationPutFinished(); });
	startAnimationGroup();

}

void vtmrobot::startAnimationGroup() {
	if (animationGroup->state() == QAbstractAnimation::Running) {
		logWarn(name.c_str(), "操作重复!");
		return;
	}
	animationGroup->start();
}

void vtmrobot::pauseAnimation(){
	animationGroup->stop();
	animationGroup->clear();
	reset();
}
void vtmrobot::clearAnimation(){
	
}

vtmrobot::~vtmrobot()
{
    delete ui;
}

void vtmrobot::paintEvent(QPaintEvent *event){
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        // 中心点坐标
        int centerX = width() / 2;
        int centerY = height() / 2;
		armLength = width() / 2.5;

        // 底座圆轴
        int baseRadius = 13;
		int clipRadius = qMin(width(), height()) / 2.00;

		QPainterPath clipPath;
		clipPath.addEllipse(QPointF(centerX, centerY), clipRadius, clipRadius);
		painter.setClipPath(clipPath);


        painter.setPen(QPen(QColor(0, 0, 0), 1));
        painter.setBrush(QColor(246, 246, 246)); // 设置画刷颜色

        painter.drawEllipse(QPointF(centerX, centerY), baseRadius, baseRadius);

        painter.setPen(QPen(QColor(0, 0, 0), 1));
        painter.setBrush(QColor(179, 185, 211)); // 设置画刷颜色
        painter.drawEllipse(QPointF(centerX, centerY), 9, 9);
		drawArm(painter, baseRotationAngle + 180, 1);
        drawArm(painter,baseRotationAngle,2);

}

void vtmrobot::drawArm(QPainter &painter, int baseRotationAngle,int arm){

    int armAngle = getArmAngle();

    int isWafer = getIsWaferArm1();

	//qDebug() << "isWafer:"<< isWafer? "present": "absence";

    if(arm==2)
	{
		armAngle = getArm2Angle();
		isWafer = getIsWaferArm2();
    }
    // 中心点坐标
    int centerX = width() / 2;
    int centerY = height() / 2;

    // 根据底座旋转角度调整手臂1和手臂2的位置
    double baseAngleRad = baseRotationAngle * M_PI / 180.0;
    double angleRad = armAngle * M_PI / 180.0;

    int arm1x=centerX + armLength * cos(baseAngleRad - angleRad);
    int arm1y=centerY - armLength * sin(baseAngleRad - angleRad);
    int arm2x=centerX + armLength * cos(baseAngleRad + angleRad);
    int arm2y=centerY - armLength * sin(baseAngleRad + angleRad);
    // 计算旋转后手臂1和手臂2的端点坐标
    QPointF arm1End(arm1x,arm1y );
    QPointF arm2End(arm2x,arm2y );

   int clawx=arm1x+arm2x-centerX;
   int clawy=arm1y+arm2y-centerY;


    // 手爪的坐标计算
     QPointF clawCenter(clawx, clawy);

    // 绘制手臂
    painter.setPen(QPen(QColor(214,214,219), armWidth));
    painter.drawLine(QPointF(centerX, centerY), arm1End);
    painter.drawLine(QPointF(centerX, centerY), arm2End);

    // 绘制轴承，轴承在手臂1和手臂2的末端
    int bearingRadius = 2; // 轴承的半径
    painter.drawEllipse(arm1End, bearingRadius, bearingRadius);
    painter.drawEllipse(arm2End, bearingRadius, bearingRadius);

    // 绘制手臂3和手臂4
    painter.drawLine(arm1End, clawCenter);
    painter.drawLine(arm2End, clawCenter);

    // 绘制手爪
        //int clawWidth = 30; // 手爪的宽度
        //int clawHeight = 30; // 手爪的矩形部分的高度
        //int trapezoidHeight = 40; // 手爪梯形部分的高度
		int clawWidth = armLength / 3; // 手爪的宽度
		int clawHeight = armLength / 3; // 手爪的矩形部分的高度
		int trapezoidHeight = armLength / 2.8; // 手爪梯形部分的高度

        // 在绘制手爪之前保存画笔状态
        painter.save();
        // 将坐标系移动到手爪的中心
       painter.translate(clawCenter.x(), clawCenter.y());
        // 应用旋转变换，使手爪的方向能够随底座旋转而旋转
	   painter.rotate(90 - baseRotationAngle);//

        // 因为我们已经移动了坐标系，所以现在手爪的中心在(0, 0)
        QRectF rotatedRectangle(-clawWidth / 2.0, -clawHeight, clawWidth, clawHeight);

        // 重新计算梯形部分的坐标，现在的中心是(0, 0)
        QPainterPath rotatedTrapezoid;
        rotatedTrapezoid.moveTo(rotatedRectangle.bottomLeft());
        rotatedTrapezoid.lineTo(rotatedRectangle.bottomRight());
        rotatedTrapezoid.lineTo(QPointF(clawWidth / 2.0 * 0.75, -clawHeight - trapezoidHeight)); // 右上角
        rotatedTrapezoid.lineTo(QPointF(-clawWidth / 2.0 * 0.75, -clawHeight - trapezoidHeight)); // 左上角
        rotatedTrapezoid.closeSubpath();

        // 绘制旋转后的矩形和梯形
        painter.setPen(QPen(QColor(0, 0, 0), 1));
        painter.setBrush(QColor(214,214,219)); // 设置画刷颜色
        painter.drawRect(rotatedRectangle); // 绘制矩形
        painter.drawPath(rotatedTrapezoid); // 绘制梯形

		// 设置字体大小
		QFont font = painter.font();  
		font.setPointSize(15);        
		painter.setFont(font);        
		if (arm == 1){
			QPointF textPosA(-clawWidth / 2.0 + clawWidth / 3.5, -clawHeight + clawHeight / 1.3);
			painter.drawText(textPosA, "A");
		}
		else{
			QPointF textPosB(-clawWidth / 2.0 + clawWidth / 3.5, -clawHeight + clawHeight / 1.3);
			painter.drawText(textPosB, "B");
		}


        if(isWafer!=0){
            //int waferSize = 50;
            //QRectF waferRect(-waferSize / 2.0, -clawHeight - trapezoidHeight-waferSize/2-10, waferSize, waferSize); // 在梯形上方留出一定距离

            painter.setPen(Qt::NoPen); // 不绘制边框
            painter.setBrush(QColor(0, 255, 0)); // 设置为绿色
           // painter.drawRect(waferRect); // 绘制晶圆

			int waferSize = 60;
			painter.drawEllipse(-waferSize / 2.0, -clawHeight - trapezoidHeight - waferSize / 2, waferSize, waferSize);
        }
        // 恢复画笔状态
        painter.restore();
}

int vtmrobot::calculateShortestRotation(int currentAngle, int targetAngle) {
	int normalizedCurrent = normalizeAngle(currentAngle);
	int normalizedTarget = normalizeAngle(targetAngle);

	int delta = normalizedTarget - normalizedCurrent;
	if (delta > 180) {
		delta -= 360;
	}
	else if (delta < -180) {
		delta += 360;
	}

	int shortestRotation = currentAngle + delta;

	return shortestRotation;
}

int vtmrobot::normalizeAngle(int angle) {
	while (angle < 0) angle += 360;
	while (angle >= 360) angle -= 360;
	return angle;
}