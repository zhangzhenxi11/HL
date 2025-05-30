#ifndef RBWIDGET_H
#define RBWIDGET_H

#include <QWidget>
#include <QPainter>
#ifdef MYCUSTOMLIB_EXPORTS
#define MYCUSTOMLIB_API __declspec(dllexport)
#else
#define MYCUSTOMLIB_API __declspec(dllimport)
#endif
namespace Ui {
class RBWidget;
}

class MYCUSTOMLIB_API RBWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RBWidget(QWidget *parent = 0);
    ~RBWidget();
    void paintEvent(QPaintEvent *event)override;

private:
    Ui::RBWidget *ui;
};

#endif // RBWIDGET_H
