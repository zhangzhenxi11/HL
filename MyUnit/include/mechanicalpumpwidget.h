#ifndef MECHANICALPUMPWIDGET_H
#define MECHANICALPUMPWIDGET_H

#include <QWidget>
#include <QPainter>
#ifdef MYCUSTOMLIB_EXPORTS
#define MYCUSTOMLIB_API __declspec(dllexport)
#else
#define MYCUSTOMLIB_API __declspec(dllimport)
#endif
namespace Ui {
class MechanicalPumpWidget;
}

class MYCUSTOMLIB_API MechanicalPumpWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MechanicalPumpWidget(QWidget *parent = 0);
    ~MechanicalPumpWidget();

    void paintEvent(QPaintEvent *event)override;

private:
    Ui::MechanicalPumpWidget *ui;
};

#endif // MECHANICALPUMPWIDGET_H
