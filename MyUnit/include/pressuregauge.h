#ifndef PRESSUREGAUGE_H
#define PRESSUREGAUGE_H

#include <QWidget>
#include <QPainter>
#ifdef MYCUSTOMLIB_EXPORTS
#define MYCUSTOMLIB_API __declspec(dllexport)
#else
#define MYCUSTOMLIB_API __declspec(dllimport)
#endif
namespace Ui {
class PressureGauge;
}

class MYCUSTOMLIB_API PressureGauge : public QWidget
{
    Q_OBJECT

public:
    explicit PressureGauge(QWidget *parent = 0);
    ~PressureGauge();
    void paintEvent(QPaintEvent *event)override;
    double angle = 0;
private:
    Ui::PressureGauge *ui;
};

#endif // PRESSUREGAUGE_H
