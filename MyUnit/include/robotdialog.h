#ifndef ROBOTDIALOG_H
#define ROBOTDIALOG_H

#include <QDialog>
#include <QRadioButton>
#include  "Kernel/kernel.h"
#include  "Kernel/FortrendUI/cassette_widget.h"
#include  "Kernel/FortrendUI/slot_widget.h"
#include  "Kernel/CoreUI/kernel_subsystem_status_widget.h"  
#include  "Kernel/Fortrend/fortrend_station.h"
#include  "Kernel/Fortrend/fortrend_cassette_manager.h"
#include  "Kernel/kernel_subsystem_reset_command.h"
#include  "Kernel/kernel_subsystem_update_command.h"
#include  "Kernel/Fortrend/abstract_output_command.h"



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
	void AddSlot(QRadioButton* selectSlot);

	int  getSelectStation()const;
	int  getSelectArm()const;
	int  getSelectSlotId()const;
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
	std::shared_ptr<FC::IKernel> kernel_ = 0;
};

#endif // ROBOTDIALOG_H
