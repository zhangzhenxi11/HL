#ifndef LPWIDGET_H
#define LPWIDGET_H

#include <QWidget>
#ifdef MYCUSTOMLIB_EXPORTS
#define MYCUSTOMLIB_API __declspec(dllexport)
#else
#define MYCUSTOMLIB_API __declspec(dllimport)
#endif
namespace Ui {
class LPWidget;
}

class MYCUSTOMLIB_API LPWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LPWidget(QWidget *parent = 0);
    ~LPWidget();
    void paintEvent(QPaintEvent *event);
private:
    Ui::LPWidget *ui;
};

#endif // LPWIDGET_H
