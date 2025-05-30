#ifndef SLIDEVALVEWIDGET_H
#define SLIDEVALVEWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QMatrix>
#include <QAction>
#ifdef MYCUSTOMLIB_EXPORTS
#define MYCUSTOMLIB_API __declspec(dllexport)
#else
#define MYCUSTOMLIB_API __declspec(dllimport)
#endif
namespace Ui {
class SlideValveWidget;
}

class MYCUSTOMLIB_API SlideValveWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SlideValveWidget(QWidget *parent = 0);
    ~SlideValveWidget();
	void isDisableClick(bool disable);
	void mousePressEvent(QMouseEvent *event) override;
	void open();
	void close();
    void paintEvent(QPaintEvent *event) override;
    double angle = 0;//旋转角度

signals:
	void signalClicked(bool status); // 定义一个信号

private:
    Ui::SlideValveWidget *ui;
	bool isdisableclick = false;
	bool isImageOpen = true;//默认打开
	QAction openAction;
	QAction closeAction;
};

#endif // SLIDEVALVEWIDGET_H
