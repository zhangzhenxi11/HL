#ifndef MROBOT_H
#define MROBOT_H

#include <QWidget>
#include <QPainter>
#include <QTimer>
#ifdef MYCUSTOMLIB_EXPORTS
#define MYCUSTOMLIB_API __declspec(dllexport)
#else
#define MYCUSTOMLIB_API __declspec(dllimport)
#endif
namespace Ui {
class mRobot;
}

class MYCUSTOMLIB_API mRobot : public QWidget
{
    Q_OBJECT

public:
    explicit mRobot(QWidget *parent = 0);
    ~mRobot();
    void paintEvent(QPaintEvent *event);
    void drawArm(QPainter &painter, int x, int y, int length);
    void updateAngle();
    void drawParallelLine(QPainter &painter, int x1, int y1, int x2, int y2, int X);
    int rotationAngle = 0; // 手臂旋转角度
private:
    Ui::mRobot *ui;
    QTimer *timer;

};

#endif // MROBOT_H
