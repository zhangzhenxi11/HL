#ifndef MANUALDIAPHRAGMVALVE_H
#define MANUALDIAPHRAGMVALVE_H

#include <QWidget>
#include <QPainter>
#ifdef MYCUSTOMLIB_EXPORTS
#define MYCUSTOMLIB_API __declspec(dllexport)
#else
#define MYCUSTOMLIB_API __declspec(dllimport)
#endif
namespace Ui {
class ManualDiaphragmValve;
}

class MYCUSTOMLIB_API ManualDiaphragmValve : public QWidget
{
    Q_OBJECT

public:
    explicit ManualDiaphragmValve(QWidget *parent = 0);
    ~ManualDiaphragmValve();
    void paintEvent(QPaintEvent *event)override;
    double angle = 0;
private:
    Ui::ManualDiaphragmValve *ui;
};

#endif // MANUALDIAPHRAGMVALVE_H
