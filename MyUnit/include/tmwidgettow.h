#ifndef TMWIDGETTOW_H
#define TMWIDGETTOW_H

#include <QWidget>
#ifdef MYCUSTOMLIB_EXPORTS
#define MYCUSTOMLIB_API __declspec(dllexport)
#else
#define MYCUSTOMLIB_API __declspec(dllimport)
#endif
namespace Ui {
class tmwidgettow;
}

class MYCUSTOMLIB_API tmwidgettow : public QWidget
{
    Q_OBJECT

public:
    explicit tmwidgettow(QWidget *parent = 0);
    ~tmwidgettow();

private:
    Ui::tmwidgettow *ui;
};

#endif // TMWIDGETTOW_H
