#ifndef HIGHVACUUMBAFFLEVALVE_H
#define HIGHVACUUMBAFFLEVALVE_H

#include <QWidget>
#include <QPainter>
#ifdef MYCUSTOMLIB_EXPORTS
#define MYCUSTOMLIB_API __declspec(dllexport)
#else
#define MYCUSTOMLIB_API __declspec(dllimport)
#endif
namespace Ui {
class HighVacuumBaffleValve;
}

class MYCUSTOMLIB_API HighVacuumBaffleValve : public QWidget
{
    Q_OBJECT

public:
    explicit HighVacuumBaffleValve(QWidget *parent = 0);
    ~HighVacuumBaffleValve();
    void paintEvent(QPaintEvent *event)override;
    double angle = 0;

private:
    Ui::HighVacuumBaffleValve *ui;
};

#endif // HIGHVACUUMBAFFLEVALVE_H
