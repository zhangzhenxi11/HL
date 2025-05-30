#ifndef SAFETYVALVE_H
#define SAFETYVALVE_H

#include <QWidget>
#include <QPainter>
#ifdef MYCUSTOMLIB_EXPORTS
#define MYCUSTOMLIB_API __declspec(dllexport)
#else
#define MYCUSTOMLIB_API __declspec(dllimport)
#endif
namespace Ui {
class SafetyValve;
}

class MYCUSTOMLIB_API  SafetyValve : public QWidget
{
    Q_OBJECT

public:
    explicit SafetyValve(QWidget *parent = 0);
    ~SafetyValve();
    void paintEvent(QPaintEvent *event)override;
    double angle = 0;
private:
    Ui::SafetyValve *ui;
};

#endif // SAFETYVALVE_H
