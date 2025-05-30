#ifndef MOLECULARPUMPWIDGET_H
#define MOLECULARPUMPWIDGET_H

#include <QWidget>
#include <QPainter>
#ifdef MYCUSTOMLIB_EXPORTS
#define MYCUSTOMLIB_API __declspec(dllexport)
#else
#define MYCUSTOMLIB_API __declspec(dllimport)
#endif
namespace Ui {
class MolecularPumpWidget;
}

class MYCUSTOMLIB_API MolecularPumpWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MolecularPumpWidget(QWidget *parent = 0);
    ~MolecularPumpWidget();

    void paintEvent(QPaintEvent *event)override;
    double angle = 0;

private:
    Ui::MolecularPumpWidget *ui;
};

#endif // MOLECULARPUMPWIDGET_H
