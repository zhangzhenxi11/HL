#ifndef LTWIDGET_H
#define LTWIDGET_H

#include <QWidget>
#include <QPainter>
#ifdef MYCUSTOMLIB_EXPORTS
#define MYCUSTOMLIB_API __declspec(dllexport)
#else
#define MYCUSTOMLIB_API __declspec(dllimport)
#endif
namespace Ui {
class LTWidget;
}

class MYCUSTOMLIB_API LTWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LTWidget(QWidget *parent = 0);
    ~LTWidget();
    void paintEvent(QPaintEvent *event)override;

private:
    Ui::LTWidget *ui;
};

#endif // LTWIDGET_H
