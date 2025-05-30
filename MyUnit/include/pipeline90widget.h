#ifndef pipeLine90Widget_H
#define pipeLine90Widget_H

#include <QWidget>
#include <QPainter>
#include <QTimer>
#include <QDateTime>

#include <QMutex>

#ifdef MYCUSTOMLIB_EXPORTS
#define MYCUSTOMLIB_API __declspec(dllexport)
#else
#define MYCUSTOMLIB_API __declspec(dllimport)
#endif

namespace Ui {
class pipeLine90Widget;
}

class MYCUSTOMLIB_API pipeLine90Widget : public QWidget
{
    Q_OBJECT

public:
    explicit pipeLine90Widget(QWidget *parent = 0);
    ~pipeLine90Widget();
    void paintEvent(QPaintEvent *event)override;
    void drawRectEdges(QPainter &painter, const QRect &rect, const QRect &otherRect);
    void drawPartialEdge(QPainter &painter, const QPoint &start, const QPoint &end, const QRect &otherRect);
   // void setRotationAngle(int angle);
    void setWaterDirection(int direction);

private slots:
    void updateWaterPosition();


public:
    Ui::pipeLine90Widget *ui;
    int pipeHeight;
    int pipeDiameter;
//    int rotationAngle; // 存储旋转角度
    QTimer *timer;
    qreal waterPosition; // 水流位置
    int waterDirection; // 水流方向，0: 水平, 1: 垂直
    qreal waterSpeed; // 水流速度
    int waterHeight; //水流高度
    int papeLineWidth90;
    int currentWaterDirection;//水流方向，0从上往下，1从下往上，其他停止流动
    QMutex mutex;
};

#endif // pipeLine90Widget_H
