#ifndef ROBOTDIALOG_H
#define ROBOTDIALOG_H

#include <QDialog>
#include <QRadioButton>

#ifdef MYCUSTOMLIB_EXPORTS
#define MYCUSTOMLIB_API __declspec(dllexport)
#else
#define MYCUSTOMLIB_API __declspec(dllimport)
#endif

namespace Ui {
class RobotDialog;
}

class MYCUSTOMLIB_API RobotDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RobotDialog(QWidget *parent = 0);
    ~RobotDialog();
	void AddStation(QRadioButton* selectBtn);
	void AddArm(QRadioButton* selectBtn);
	int  getSelectStation()const;
	int  getSelectArm()const;
signals:
	void signalget(int arm, int stationid); // 定义一个信号
	void signalput(int arm, int stationid); // 定义一个信号
	void signalspeed(int speed); // 定义一个信号
	void signalzspeed(int speed); // 定义一个信号

private slots:
	void onGet();
	void onPut();
	void onSetSpeed();
	void onSetZSpeed();


private:
    Ui::RobotDialog *ui;
};

#endif // ROBOTDIALOG_H
