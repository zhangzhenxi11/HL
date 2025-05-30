#ifndef LOADLOCKB_H
#define LOADLOCKB_H

#include <QWidget>
#include <QPainter>
#ifdef MYCUSTOMLIB_EXPORTS
#define MYCUSTOMLIB_API __declspec(dllexport)
#else
#define MYCUSTOMLIB_API __declspec(dllimport)
#endif
namespace Ui {
class LoadLockB;
}

class MYCUSTOMLIB_API LoadLockB : public QWidget
{
    Q_OBJECT

public:
    explicit LoadLockB(QWidget *parent = 0);
    ~LoadLockB();
    void paintEvent(QPaintEvent *event)override;
	void mousePressEvent(QMouseEvent *event) override;
    void SetIsLeft(bool left);
	void SetName(std::string name);
	void  SetCurrentSlot(int slot);
	int GetCurrentSlot();
	void  SetSelectSlot(int slot);
	int GetSelectSlot();
	std::string GetName();

signals:
	void signalLoadLockBReset(std::string name); // 定义一个信号
	void signalLoadLockBGetStatus(std::string name); // 定义一个信号
	void signalLoadLockBClearError(std::string name); // 定义一个信号
	void signalLoadLockBOpenbox(std::string name);
	void signalLoadLockBClosebox(std::string name);
	void signalLoadLockBMapping(std::string name);
	void signalLoadLockBMoveToSlot(std::string name, LoadLockB *lk_widget);
	void signalLoadLockBOpenTMCavityDoor(std::string name);
	void signalLoadLockBCloseTMCavityDoor(std::string name);
	void signalLoadLockBOpenDiaphragmValve(std::string name);
	void signalLoadLockBCloseDiaphragmValve(std::string name);
	void signalLoadLockBOpenExhaustValve(std::string name);
	void signalLoadLockBCloseExhaustValve(std::string name);
	void signalLoadLockBOpenAngleValve(std::string name);
	void signalLoadLockBCloseAngleValve(std::string name);
	void signalLoaLockBOpenInsertingPlateValve(std::string name);
	void signalLoaLockBCloseInsertingPlateValve(std::string name);
	void signalLoaLockBOpenHeightVacuumBaffleValve(std::string name);
	void signalLoaLockBCloseHeightVacuumBaffleValve(std::string name);
	void signalLoadLockAMoveToOrigin(std::string name);

	void signalRightClick();

private:
    Ui::LoadLockB *ui;
    bool IsLeft;
	std::string name;
	int currentslot;
	int Selectslot;
};

#endif // LOADLOCK_H
