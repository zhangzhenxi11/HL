#ifndef TM_H
#define TM_H

#include <QWidget>
#include <QPropertyAnimation>
#include<QThread>
#include<QTimer>
#include <QTransform>
#ifdef MYCUSTOMLIB_EXPORTS
#define MYCUSTOMLIB_API __declspec(dllexport)
#else
#define MYCUSTOMLIB_API __declspec(dllimport)
#endif
namespace Ui {
class TM;
}

class MYCUSTOMLIB_API TM : public QWidget
{
    Q_OBJECT
	Q_PROPERTY(qreal rotationAngle READ rotationAngle WRITE setRotationAngle)
public:
    explicit TM(QWidget *parent = 0);
    ~TM();
    void paintEvent(QPaintEvent *event);
	void mousePressEvent(QMouseEvent *event) override;
	bool eventFilter(QObject *obj, QEvent *event) override;
    void updateColor();
	std::string GetName();
	void SetName(std::string name);
	int getIsWaferAligner() const;
	void setIsWaferAligner(int wafer);
    bool IsWaferAligner=false;
	bool IseventFilter;
	void startRotationAnimation(int numRotations);
	qreal rotationAngle() const;
	void setRotationAngle(qreal angle);
	void timeFS();//内部启动旋转
	

private:
	qreal m_rotationAngle = 0;
	QPropertyAnimation *animation;
	QTimer * timer1;


signals:
	void signalTMOpenDiaphragmValve(std::string name);
	void signalTMCloseDiaphragmValve(std::string name);
	void signalTMOpenHeightVacuumBaffleValve(std::string name);
	void signalTMCloseHeightVacuumBaffleValve(std::string name);
	void signalTMOpenAngleValve(std::string name);
	void signalTMCloseAngleValve(std::string name);
	void signalTMOpenInsertingPlateValve(std::string name);
	void signalTMCloseInsertingPlateValve(std::string name);
	void signalTMOpenFlowmeterDiaphragmValve(std::string name);
	void signalTMCloseFlowmeterDiaphragmValve(std::string name);
	void signalTMReset(std::string name);
	void signalTMGetStatus(std::string name);
	void signalAlignerReset(std::string name);
	void signalAlignerAlign(std::string name);
	void signalRightClick();
private:
    Ui::TM *ui;
	std::string name;

};

#endif // TM_H
