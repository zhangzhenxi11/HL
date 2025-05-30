#ifndef PIPEWIDGET_H
#define PIPEWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QTimer>
#include <QDateTime>

#ifdef MYCUSTOMLIB_EXPORTS
#define MYCUSTOMLIB_API __declspec(dllexport)
#else
#define MYCUSTOMLIB_API __declspec(dllimport)
#endif
namespace Ui {
class pipeWidget;
}

class MYCUSTOMLIB_API pipeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit pipeWidget(QWidget *parent = 0,int width=100,int height=100,int pipeDiameter=20,int angle=0);
    ~pipeWidget();
    void paintEvent(QPaintEvent *event)override;
    void drawRectEdges(QPainter &painter, const QRect &rect, const QRect &otherRect);
    void drawPartialEdge(QPainter &painter, const QPoint &start, const QPoint &end, const QRect &otherRect);
    void setRotationAngle(int angle);
    void setWaterDirection(int direction);

private slots:
    void updateWaterPosition();


private:
    Ui::pipeWidget *ui;
    int pipeWidth;
    int pipeHeight;
    int pipeDiameter;
    int rotationAngle; // 存储旋转角度
    QTimer *timer;
    qreal waterPosition; // 水流位置
    int waterDirection; // 水流方向，0: 水平, 1: 垂直
    qreal waterSpeed; // 水流速度
};

#endif // PIPEWIDGET_H
