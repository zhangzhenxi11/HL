#ifndef GDTPMWIDGET_H
#define GDTPMWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QPainter>
#ifdef MYCUSTOMLIB_EXPORTS
#define MYCUSTOMLIB_API __declspec(dllexport)
#else
#define MYCUSTOMLIB_API __declspec(dllimport)
#endif
namespace Ui {
class PMGDTWidget;
}

class MYCUSTOMLIB_API PMGDTWidget : public QWidget
{
    Q_OBJECT
	Q_PROPERTY(int yOffset READ yOffset WRITE setYOffset)
public:
    explicit PMGDTWidget(QWidget *parent = 0);
    ~PMGDTWidget();
    void paintEvent(QPaintEvent *event);
	void mousePressEvent(QMouseEvent *event) override;
     void updateColor();
    void setStatus(bool busy);
    bool getStatus();
    void setRotationAngle(int angle);
    void setWafer(bool wafer);
	std::string GetName();
	void SetName(std::string name);
	int yOffset() const;
	void setYOffset(int offset);
	void animateToYOffset(int newYOffset,int speed);

signals:
	void signalPMOpenTMCavityDoor(std::string name);
	void signalPMCloseTMCavityDoor(std::string name);
	void signalPMGetFinished(std::string name);
	void signalPMUplaodFinished(std::string name);
	void signalPMGetStatus(std::string name);
	void signalPMReset(std::string name);
	void signalRightClick();
private:
    Ui::PMGDTWidget *ui;
    QTimer *timer;
    bool isbusy;
    bool isAlarm;
    bool IsUpdate=false;
    int rotationAngle;
    bool isWafer;
	std::string name;
	int isUpdateColor = 0;

	int lineLength;  // 线段长度
	int lineSpacing; // 线段间距
	int yOffsetValue; // Y轴偏移量
	QSequentialAnimationGroup *animationGroup;
};

#endif // CHAMBERWIDGET_H
