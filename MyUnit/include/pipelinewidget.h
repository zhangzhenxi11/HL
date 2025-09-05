#ifndef pipeLineWidget_H
#define pipeLineWidget_H

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
class pipeLineWidget;
}

class MYCUSTOMLIB_API pipeLineWidget : public QWidget
{
    Q_OBJECT

public:
    explicit pipeLineWidget(QWidget *parent = 0,int width=100,int pipeDiameter=20,int angle=0,int WaterDirection=0);
    ~pipeLineWidget();
    void paintEvent(QPaintEvent *event)override;
    void drawRectEdges(QPainter &painter, const QRect &rect, const QRect &otherRect);
    void drawPartialEdge(QPainter &painter, const QPoint &start, const QPoint &end, const QRect &otherRect);
    void setRotationAngle(int angle);
    void setWaterDirection(int direction);

    static pipeLineWidget& getInstance();
    // 删除拷贝构造函数和赋值操作符
    pipeLineWidget(const pipeLineWidget&) = delete;
    pipeLineWidget& operator=(const pipeLineWidget&) = delete;

    void start(int timeOut);
    void stop();

private slots:
    void updateWaterPosition();


private:
    Ui::pipeLineWidget *ui;
    int pipeWidth;
    int pipeDiameter;
    int rotationAngle; // 存储旋转角度
    QTimer *timer;
    qreal waterPosition; // 水流位置
    int waterDirection; // 水流方向，0: 水平, 1: 垂直
    qreal waterSpeed; // 水流速度
    int waterWidth;
    int currentWaterDirection;//流动方向,0从左往右，1从右往左，其他停止流动
    QMutex mutex;

public:
    int angle;
};


#endif // pipeLineWidget_H
