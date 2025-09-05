#ifndef _XLH_FORTREND_BREAKVACUUM_SUBSYSTEM_WIDGET_INCLUDE_
#define _XLH_FORTREND_BREAKVACUUM_SUBSYSTEM_WIDGET_INCLUDE_ 

#include "Kernel/kernel.h"
#include <QWidget>
#include <cmath> 

/*破真空*/
namespace FC{
	
class QBreakVacuumSubsystemWidgetPrivate;
class QBreakVacuumSubsystemWidget : public QWidget{
    Q_OBJECT
public:
	explicit QBreakVacuumSubsystemWidget(const std::shared_ptr<IKernel>& kernel, QWidget *parent = 0);
    ~QBreakVacuumSubsystemWidget();

private slots:
	void onTMGMFKClicked(bool status);//TM快
	void onTMGMFMClicked(bool status);//TM慢
	void onLLAGMFKClicked(bool status);//LLA快
	void onLLAGMFMClicked(bool status);//LLA慢
	void onLLBGMFKClicked(bool status);//LLB快
	void onLLBGMFMClicked(bool status);//LLB慢
	void showMessage(const QString& message);

	void onOpenLoadLock1AutoBreakVacuumCommand();//破真空

	void onOpenLoadLock2AutoBreakVacuumCommand();

	void onOpenTMCavityAutoBreakVacuumCommand();
	
private:
	void onAttributeUpdate()throw(KernelException);
	int convertRange(double vacuumValue);
private:
	Q_DECLARE_PRIVATE(QBreakVacuumSubsystemWidget)
	QBreakVacuumSubsystemWidgetPrivate *d_ptr;
};
}
#endif // TMWIDGETTOW_H
