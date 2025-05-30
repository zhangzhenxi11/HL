#ifndef VTMROBOT_H
#define VTMROBOT_H

#include <QWidget>
#include <QPainter>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include "robotdialog.h"
#ifdef MYCUSTOMLIB_EXPORTS
#define MYCUSTOMLIB_API __declspec(dllexport)
#else
#define MYCUSTOMLIB_API __declspec(dllimport)
#endif
namespace Ui {
class vtmrobot;
}

class MYCUSTOMLIB_API vtmrobot : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int baseRotationAngle READ getBaseRotationAngle WRITE setBaseRotationAngle)
    Q_PROPERTY(int armAngle READ getArmAngle WRITE setArmAngle)
    Q_PROPERTY(int arm2Angle READ getArm2Angle WRITE setArm2Angle)
    Q_PROPERTY(int isWaferarm1 READ getIsWaferArm1 WRITE setIsWaferArm1)
    Q_PROPERTY(int isWaferarm2 READ getIsWaferArm2 WRITE setIsWaferArm2)
public:
    explicit vtmrobot(QWidget *parent = 0);
    ~vtmrobot();
	void SetName(std::string Name);
	std::string GetName();
    void paintEvent(QPaintEvent *event)override;
	void mousePressEvent(QMouseEvent *event) override;
    void drawArm(QPainter &painter, int baseRotationAngle,int arm);
    int getBaseRotationAngle() const;
	int getCurrentRotationAngle() const;
    void setBaseRotationAngle(int angle);
    int getArmAngle() const;
    void setArmAngle(int angle);
    int getArm2Angle() const;
    void setArm2Angle(int angle);
    int getIsWaferArm1() const;
    void setIsWaferArm1(int wafer);
    int getIsWaferArm2() const;
    void setIsWaferArm2(int wafer);
	int getCurrentStation() const;
	void setCurrentStation(int station);
	int getCurrentArm() const;
	void setCurrentArm(int arm);
	void setArmSpeed(int speed);
	int getArmSpeed();
    int getBaseSpeed();
	void setBaseSpeed(int speed);
	void get(int arm=1, int rotationangle=40, int armangle=70);
	void put(int arm = 1, int rotationangle = 50, int armangle = 70);
	void reset();
	void animationPause();
	void animationResume();
	void animationAbort();
	void clearAnimation();
	void pauseAnimation();
	int calculateShortestRotation(int currentAngle, int targetAngle);
	int normalizeAngle(int angle);
	void startAnimationGroup();

signals:
	void signalRobotDialog(); // 定义一个信号
	void signalRobotReset(); // 定义一个信号
	void signalRobotGetStatus(); // 定义一个信号
	void signalRobotClearError(); // 定义一个信号
	//void signalRobotData(); // 定义一个信号
	//void signalRobotStatusDialog(); // 定义一个信号
	void extendAnimationGetFinished();
	void extendAnimationPutFinished();
	void signalRightClick();
private:
    Ui::vtmrobot *ui;
	QSequentialAnimationGroup *animationGroupReset;// 复位动画
	QSequentialAnimationGroup *animationGroup;
    int armWidth; // 手臂的宽度
    int armLength; // 手臂的长度
    int armAngle; // 手臂1和手臂2与垂直线的夹角
    int arm2Angle; // 手臂1和手臂2与垂直线的夹角
    int baseRotationAngle; //旋转角度
    int isWaferarm1; //手爪是否有晶圆
    int isWaferarm2; //手爪是否有晶圆
	int currentRotationAngle;//实时记录当前角度
	int currentStation;//记录当前取放的station工位
	int currentArm;//记录当前取放手爪
	int armSpeed;
	int baseSpeed;
	std::string name;

};

#endif // VTMROBOT_H
