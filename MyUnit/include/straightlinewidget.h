#ifndef STRAIGHTLINEWIDGET_H
#define STRAIGHTLINEWIDGET_H

#include <QWidget>
#include <QPainter>
#ifdef MYCUSTOMLIB_EXPORTS
#define MYCUSTOMLIB_API __declspec(dllexport)
#else
#define MYCUSTOMLIB_API __declspec(dllimport)
#endif
namespace Ui {
class StraightLineWidget;
}

class MYCUSTOMLIB_API StraightLineWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StraightLineWidget(QWidget *parent = 0);
    ~StraightLineWidget();
    void paintEvent(QPaintEvent *event)override;
private:
    Ui::StraightLineWidget *ui;
};

#endif // STRAIGHTLINEWIDGET_H
