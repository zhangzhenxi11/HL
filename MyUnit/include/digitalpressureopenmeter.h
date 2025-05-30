#ifndef DIGITALPRESSUREOPENMETER_H
#define DIGITALPRESSUREOPENMETER_H

#include <QWidget>
#include <QPainter>
#ifdef MYCUSTOMLIB_EXPORTS
#define MYCUSTOMLIB_API __declspec(dllexport)
#else
#define MYCUSTOMLIB_API __declspec(dllimport)
#endif
namespace Ui {
class DigitalPressureOpenMeter;
}

class MYCUSTOMLIB_API DigitalPressureOpenMeter : public QWidget
{
    Q_OBJECT

public:
    explicit DigitalPressureOpenMeter(QWidget *parent = 0);
    ~DigitalPressureOpenMeter();
    void paintEvent(QPaintEvent *event)override;
    double angle = 0;
private:
    Ui::DigitalPressureOpenMeter *ui;
};

#endif // DIGITALPRESSUREOPENMETER_H
