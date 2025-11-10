#ifndef EROBOTDIALOG_H
#define EROBOTDIALOG_H

#include  <QDialog>
#include  <QRadioButton>
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
	class ERobotDialog;
}
class  MYCUSTOMLIB_API ERobotDialog : public QDialog
{
	Q_OBJECT

public:
	ERobotDialog(QWidget *parent = nullptr);
	~ERobotDialog();

	void addStation(QRadioButton* selectBtn);
	void addArm(QRadioButton* armBtn);
	void addSlot(FC::QFortrendSlotWidget* selectSlot);

	int  getSelectEfemStation()const;
	int  getSelectEfemArm()const;
	int  getSelectEfemSlotId()const;

signals:
	void signalGet(int stationid, int arm, int slot); // 定义一个信号
	void signalPut(int stationid, int arm, int slot); // 定义一个信号
	void signalSpeed(int speed); // 定义一个信号


private slots:
	void onGetWafer();
	void onPutWafer();
	void onSetRobotSpeed();
private:
	Ui::ERobotDialog *ui;
	std::shared_ptr<FC::IKernel> kernel_ = 0;
};

#endif //EROBOTDIALOG_H