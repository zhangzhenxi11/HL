#ifndef AXIALPRESSUREGAUGE_H
#define AXIALPRESSUREGAUGE_H

#include <QWidget>
#include <QPainter>
#ifdef MYCUSTOMLIB_EXPORTS
#define MYCUSTOMLIB_API __declspec(dllexport)
#else
#define MYCUSTOMLIB_API __declspec(dllimport)
#endif
namespace Ui {
class AxialPressureGauge;
}

class MYCUSTOMLIB_API AxialPressureGauge : public QWidget
{
    Q_OBJECT

public:
    explicit AxialPressureGauge(QWidget *parent = 0);
    ~AxialPressureGauge();
    void paintEvent(QPaintEvent *event)override;
    double angle = 0;
private:
    Ui::AxialPressureGauge *ui;
};

#endif // AXIALPRESSUREGAUGE_H
