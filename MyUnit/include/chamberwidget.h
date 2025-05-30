#ifndef CHAMBERWIDGET_H
#define CHAMBERWIDGET_H

#include <QWidget>
#include <QTimer>
#ifdef MYCUSTOMLIB_EXPORTS
#define MYCUSTOMLIB_API __declspec(dllexport)
#else
#define MYCUSTOMLIB_API __declspec(dllimport)
#endif
namespace Ui {
class chamberWidget;
}

class MYCUSTOMLIB_API chamberWidget : public QWidget
{
    Q_OBJECT

public:
    explicit chamberWidget(QWidget *parent = 0);
    ~chamberWidget();
    void paintEvent(QPaintEvent *event);
	void mousePressEvent(QMouseEvent *event) override;
     void updateColor();
    void setStatus(bool busy);
    bool getStatus();
    void setRotationAngle(int angle);
    void setWafer(bool wafer);
	std::string GetName();
	void SetName(std::string name);

signals:
	void signalPMOpenTMCavityDoor(std::string name);
	void signalPMCloseTMCavityDoor(std::string name);
	void signalPMGetFinished(std::string name);
	void signalPMUplaodFinished(std::string name);
	void signalPMGetStatus(std::string name);
	void signalPMReset(std::string name);
	void signalRightClick();
private:
    Ui::chamberWidget *ui;
    QTimer *timer;
    bool isbusy;
    bool isAlarm;
    bool IsUpdate=false;
    int rotationAngle;
    bool isWafer;
	std::string name;
};

#endif // CHAMBERWIDGET_H
