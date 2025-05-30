#ifndef _XLH_FORTREND_BREAKVACUUM_SUBSYSTEM_WIDGET_INCLUDE_
#define _XLH_FORTREND_BREAKVACUUM_SUBSYSTEM_WIDGET_INCLUDE_ 

#include "Kernel/kernel.h"
#include <QWidget>



namespace FC{

class QBreakVacuumSubsystemWidgetPrivate;
class QBreakVacuumSubsystemWidget : public QWidget{
    Q_OBJECT
public:
	explicit QBreakVacuumSubsystemWidget(const std::shared_ptr<IKernel>& kernel, QWidget *parent = 0);
    ~QBreakVacuumSubsystemWidget();

private slots:
	void onTMGMFKClicked(bool status);
	void onTMGMFMClicked(bool status);
	void onLLAGMFKClicked(bool status);
	void onLLAGMFMClicked(bool status);
	void onLLBGMFKClicked(bool status);
	void onLLBGMFMClicked(bool status);
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
