#ifndef TMCAVITY_H
#define TMCAVITY_H

#include <QWidget>
#include <QPainter>
#ifdef MYCUSTOMLIB_EXPORTS
#define MYCUSTOMLIB_API __declspec(dllexport)
#else
#define MYCUSTOMLIB_API __declspec(dllimport)
#endif
namespace Ui {
class TMCavity;
}

class MYCUSTOMLIB_API TMCavity : public QWidget
{
    Q_OBJECT

public:
    explicit TMCavity(QWidget *parent = 0);
    ~TMCavity();
    void paintEvent(QPaintEvent *event)override;
    double angle = 0;
private:
    Ui::TMCavity *ui;
};

#endif // TMCAVITY_H
