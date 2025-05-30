#ifndef COMPOUNDVACUUMGAUGE_H
#define COMPOUNDVACUUMGAUGE_H

#include <QWidget>
#include <QPainter>
#ifdef MYCUSTOMLIB_EXPORTS
#define MYCUSTOMLIB_API __declspec(dllexport)
#else
#define MYCUSTOMLIB_API __declspec(dllimport)
#endif
namespace Ui {
class CompoundVacuumGauge;
}

class MYCUSTOMLIB_API CompoundVacuumGauge : public QWidget
{
    Q_OBJECT

public:
    explicit CompoundVacuumGauge(QWidget *parent = 0);
    ~CompoundVacuumGauge();
    void paintEvent(QPaintEvent *event)override;
    double angle = 0;

private:
    Ui::CompoundVacuumGauge *ui;
};

#endif // COMPOUNDVACUUMGAUGE_H
