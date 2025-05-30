#ifndef LOADLOCKA_H
#define LOADLOCKA_H

#include <QWidget>
#include <QPainter>
#ifdef MYCUSTOMLIB_EXPORTS
#define MYCUSTOMLIB_API __declspec(dllexport)
#else
#define MYCUSTOMLIB_API __declspec(dllimport)
#endif
namespace Ui {
class LoadLockA;
}

class MYCUSTOMLIB_API LoadLockA : public QWidget
{
    Q_OBJECT

public:
    explicit LoadLockA(QWidget *parent = 0);
    ~LoadLockA();
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
	void signalLoadLockAReset(std::string name); // 定义一个信号
	void signalLoadLockAGetStatus(std::string name); // 定义一个信号
	void signalLoadLockAClearError(std::string name); // 定义一个信号
	void signalLoadLockAOpenbox(std::string name);
	void signalLoadLockAClosebox(std::string name);
	void signalLoadLockAMapping(std::string name);
	void signalLoadLockAMoveToSlot(std::string name, LoadLockA *lk_widget);
	void signalLoadLockAOpenTMCavityDoor(std::string name);
	void signalLoadLockACloseTMCavityDoor(std::string name);
	void signalLoadLockAOpenDiaphragmValve(std::string name);
	void signalLoadLockACloseDiaphragmValve(std::string name);
	void signalLoadLockAOpenExhaustValve(std::string name);
	void signalLoadLockACloseExhaustValve(std::string name);
	void signalLoadLockAOpenAngleValve(std::string name);
	void signalLoadLockACloseAngleValve(std::string name);
	void signalLoaLockAOpenInsertingPlateValve(std::string name);
	void signalLoaLockACloseInsertingPlateValve(std::string name);
	void signalLoaLockAOpenHeightVacuumBaffleValve(std::string name);
	void signalLoaLockACloseHeightVacuumBaffleValve(std::string name);
	void signalLoadLockAMoveToOrigin(std::string name);
	

	void signalRightClick();

private:
    Ui::LoadLockA *ui;
    bool IsLeft;
	std::string name;
	int currentslot;
	int Selectslot;
};

#endif // LOADLOCK_H
