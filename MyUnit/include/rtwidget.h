#ifndef RTWIDGET_H
#define RTWIDGET_H

#include <QWidget>
#include <QPainter>
#ifdef MYCUSTOMLIB_EXPORTS
#define MYCUSTOMLIB_API __declspec(dllexport)
#else
#define MYCUSTOMLIB_API __declspec(dllimport)
#endif
namespace Ui {
class RTWidget;
}

class MYCUSTOMLIB_API RTWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RTWidget(QWidget *parent = 0);
    ~RTWidget();
    void paintEvent(QPaintEvent *event)override;

private:
    Ui::RTWidget *ui;
};

#endif // RTWIDGET_H
