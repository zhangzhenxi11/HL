#ifndef ANGLEVALVEWIDGET_H
#define ANGLEVALVEWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QAction>
#ifdef MYCUSTOMLIB_EXPORTS
#define MYCUSTOMLIB_API __declspec(dllexport)
#else
#define MYCUSTOMLIB_API __declspec(dllimport)
#endif
namespace Ui {
class AngleValveWidget;
}

class MYCUSTOMLIB_API AngleValveWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AngleValveWidget(QWidget *parent = 0);
    ~AngleValveWidget();

	void isDisableClick(bool disable);
	void mousePressEvent(QMouseEvent *event) override;
	void open();
	void close();
    void paintEvent(QPaintEvent *event) override;
	double angle = 0;

signals:
	void signalClicked(bool status); // 定义一个信号
private:
    Ui::AngleValveWidget *ui;
	bool isdisableclick = false;
	bool isImageOpen = true;//默认打开
	QAction openAction;
	QAction closeAction;
};

#endif // ANGLEVALVEWIDGET_H
