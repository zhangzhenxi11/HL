#ifndef LOADLOCK_H
#define LOADLOCK_H

#include <QWidget>
#include <QPainter>
#ifdef MYCUSTOMLIB_EXPORTS
#define MYCUSTOMLIB_API __declspec(dllexport)
#else
#define MYCUSTOMLIB_API __declspec(dllimport)
#endif
namespace Ui {
class LoadLock;
}

class MYCUSTOMLIB_API LoadLock : public QWidget
{
    Q_OBJECT

public:
    explicit LoadLock(QWidget *parent = 0);
    ~LoadLock();
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
	void signalLoadLockReset(std::string name); // 定义一个信号
	void signalLoadLockGetStatus(std::string name); // 定义一个信号
	void signalLoadLockClearError(std::string name); // 定义一个信号
	void signalLoadLockOpenbox(std::string name);
	void signalLoadLockClosebox(std::string name);
	void signalLoadLockMapping(std::string name);
	void signalLoadLockMoveToSlot(std::string name, LoadLock *lk_widget);
	void signalLoadLockOpenTMCavityDoor(std::string name);
	void signalLoadLockCloseTMCavityDoor(std::string name);
	void signalLoadLockOpenDiaphragmValve(std::string name);
	void signalLoadLockCloseDiaphragmValve(std::string name);
	void signalLoadLockOpenExhaustValve(std::string name);
	void signalLoadLockCloseExhaustValve(std::string name);
	void signalLoadLockOpenAngleValve(std::string name);
	void signalLoadLockCloseAngleValve(std::string name);
	void signalRightClick();

private:
    Ui::LoadLock *ui;
    bool IsLeft;
	std::string name;
	int currentslot;
	int Selectslot;
};

#endif // LOADLOCK_H
