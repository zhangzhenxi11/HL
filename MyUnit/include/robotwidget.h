#ifndef ROBOTWIDGET_H
#define ROBOTWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QPropertyAnimation>
#ifdef MYCUSTOMLIB_EXPORTS
#define MYCUSTOMLIB_API __declspec(dllexport)
#else
#define MYCUSTOMLIB_API __declspec(dllimport)
#endif
namespace Ui {
class robotWidget;
}

class MYCUSTOMLIB_API robotWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal rotationAngle READ rotationAngle WRITE setRotationAngle NOTIFY rotationAngleChanged)

public:
    explicit robotWidget(QWidget *parent = 0);
    ~robotWidget();
    void paintEvent(QPaintEvent *event)override;
    qreal rotationAngle() const { return m_rotationAngle; }

public slots:
    void setRotationAngle(qreal angle) {
        if (m_rotationAngle == angle)
            return;
        m_rotationAngle = angle;
        emit rotationAngleChanged(angle);
        update(); // 重绘Widget
    }

signals:
    void rotationAngleChanged(qreal angle);

private:
    qreal m_rotationAngle; // 存储旋转角度

private:
    Ui::robotWidget *ui;
};

#endif // ROBOTWIDGET_H
