#ifndef VALVEWHEEL_H
#define VALVEWHEEL_H

#include <QWidget>

#include <QPainter>
#include <QTransform>
#include"thread"
#include<QThread>
#include<QTimer>
#include <QAction>
#ifdef MYCUSTOMLIB_EXPORTS
#define MYCUSTOMLIB_API __declspec(dllexport)
#else
#define MYCUSTOMLIB_API __declspec(dllimport)
#endif
namespace Ui {
class ValveWheel;
}

class MYCUSTOMLIB_API ValveWheel : public QWidget
{
    Q_OBJECT

public:
    explicit ValveWheel(QWidget *parent = 0);
    ~ValveWheel();

    void paintEvent(QPaintEvent *event)override;
	void mousePressEvent(QMouseEvent *event) override;

	void open();
	void close();
	void isDisableClick(bool disable);
    void timeFS();//内部启动旋转
    int rotationAngle = 0;//每次旋转的角度
    //外部调用控制
    bool statr = true;//启动旋转
    QColor colorLine = QColor(151, 155, 155);//线条颜色
    int thicknessLine = 5;//线条粗细
    bool AngleDirection = true;//旋转方向：顺时针 true,逆时针 false
	QTimer * timer1;

signals:
	void signalClicked(bool status); // 定义一个无参数的信号

private:
    Ui::ValveWheel *ui;
	bool isdisableclick = false;
	QAction openAction;
	QAction closeAction;
};

#endif // VALVEWHEEL_H
