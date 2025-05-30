#ifndef CLEANFILTER_H
#define CLEANFILTER_H

#include <QWidget>
#include <QPainter>
#ifdef MYCUSTOMLIB_EXPORTS
#define MYCUSTOMLIB_API __declspec(dllexport)
#else
#define MYCUSTOMLIB_API __declspec(dllimport)
#endif
namespace Ui {
class CleanFilter;
}

class MYCUSTOMLIB_API CleanFilter : public QWidget
{
    Q_OBJECT

public:
    explicit CleanFilter(QWidget *parent = 0);
    ~CleanFilter();
    void paintEvent(QPaintEvent *event)override;
    double angle = 0;
private:
    Ui::CleanFilter *ui;
};

#endif // CLEANFILTER_H
