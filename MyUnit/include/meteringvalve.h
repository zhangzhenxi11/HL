#ifndef METERINGVALVE_H
#define METERINGVALVE_H

#include <QWidget>
#include <QPainter>
#ifdef MYCUSTOMLIB_EXPORTS
#define MYCUSTOMLIB_API __declspec(dllexport)
#else
#define MYCUSTOMLIB_API __declspec(dllimport)
#endif
namespace Ui {
class MeteringValve;
}

class MYCUSTOMLIB_API MeteringValve : public QWidget
{
    Q_OBJECT

public:
    explicit MeteringValve(QWidget *parent = 0);
    ~MeteringValve();
    void paintEvent(QPaintEvent *event)override;
    double angle = 0;
private:
    Ui::MeteringValve *ui;
};

#endif // METERINGVALVE_H
