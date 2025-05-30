#ifndef LBWIDGET_H
#define LBWIDGET_H

#include <QWidget>
#include <QPainter>
#ifdef MYCUSTOMLIB_EXPORTS
#define MYCUSTOMLIB_API __declspec(dllexport)
#else
#define MYCUSTOMLIB_API __declspec(dllimport)
#endif
namespace Ui {
class LBWidget;
}

class MYCUSTOMLIB_API LBWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LBWidget(QWidget *parent = 0);
    ~LBWidget();
    void paintEvent(QPaintEvent *event)override;

private:
    Ui::LBWidget *ui;
};

#endif // LBWIDGET_H
