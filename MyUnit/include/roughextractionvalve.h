#ifndef ROUGHEXTRACTIONVALVE_H
#define ROUGHEXTRACTIONVALVE_H

#include <QWidget>
#include <QPainter>
#include<QTimer>
#include <QAction>
#ifdef MYCUSTOMLIB_EXPORTS
#define MYCUSTOMLIB_API __declspec(dllexport)
#else
#define MYCUSTOMLIB_API __declspec(dllimport)
#endif
namespace Ui {
class RoughExtractionValve;
}

class MYCUSTOMLIB_API RoughExtractionValve : public QWidget
{
    Q_OBJECT

public:
    explicit RoughExtractionValve(QWidget *parent = 0);
    ~RoughExtractionValve();

	void isDisableClick(bool disable);
	void mousePressEvent(QMouseEvent *event) override;
	void open();
	void close();
    void paintEvent(QPaintEvent *event)override;
    double angle = 0;

signals:
	void signalClicked(bool status); // 定义一个信号

private:
    Ui::RoughExtractionValve *ui;
	bool isdisableclick = false;
	bool isImageOpen=true;//默认打开
	QAction openAction;
	QAction closeAction;
};

#endif // ROUGHEXTRACTIONVALVE_H
