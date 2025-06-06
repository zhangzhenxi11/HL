#ifndef _XLH_FORTREND_BREAKVACUUM_SUBSYSTEM_WIDGET_INCLUDE_
#define _XLH_FORTREND_BREAKVACUUM_SUBSYSTEM_WIDGET_INCLUDE_ 

#include "Kernel/kernel.h"
#include <QWidget>


/*破真空*/
namespace FC{
	
class QBreakVacuumSubsystemWidgetPrivate;
class QBreakVacuumSubsystemWidget : public QWidget{
    Q_OBJECT
public:
	explicit QBreakVacuumSubsystemWidget(const std::shared_ptr<IKernel>& kernel, QWidget *parent = 0);
    ~QBreakVacuumSubsystemWidget();

private slots:
	void onTMGMFKClicked(bool status);//快
	void onTMGMFMClicked(bool status);//慢
	void onLLAGMFKClicked(bool status);//快
	void onLLAGMFMClicked(bool status);//慢
	void onLLBGMFKClicked(bool status);//快
	void onLLBGMFMClicked(bool status);//慢
	void showMessage(const QString& message);
	void onOpenLoadLock1AutoBreakVacuumCommand();
	void onOpenLoadLock2AutoBreakVacuumCommand();
	void onOpenTMCavityAutoBreakVacuumCommand();
	
private:
	void onAttributeUpdate()throw(KernelException);
private:
	Q_DECLARE_PRIVATE(QBreakVacuumSubsystemWidget)
	QBreakVacuumSubsystemWidgetPrivate *d_ptr;
};
}
#endif // TMWIDGETTOW_H
