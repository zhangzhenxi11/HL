#ifndef FEEDANDDISCHARGE_H
#define FEEDANDDISCHARGE_H

#include <QWidget>
#include <QPainter>
#ifdef MYCUSTOMLIB_EXPORTS
#define MYCUSTOMLIB_API __declspec(dllexport)
#else
#define MYCUSTOMLIB_API __declspec(dllimport)
#endif
namespace Ui {
class FeedAndDischarge;
}

class MYCUSTOMLIB_API FeedAndDischarge : public QWidget
{
    Q_OBJECT

public:
    explicit FeedAndDischarge(QWidget *parent = 0);
    ~FeedAndDischarge();
    void paintEvent(QPaintEvent *event)override;
    double angle = 0;

private:
    Ui::FeedAndDischarge *ui;
};

#endif // FEEDANDDISCHARGE_H
