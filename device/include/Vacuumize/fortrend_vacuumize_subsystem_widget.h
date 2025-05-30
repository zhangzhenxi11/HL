
/**
* @file            fortrend_tmCartoon_subsystem.h
* @brief           Fortrend TMCARTOON widget
* @author           xielonghua
*/

// Library: Fortrend
// Package: SubSystem/TMCARTOON

#ifndef _XLH_FORTREND_VACUUMMIZE_SUBSYSTEM_WIDGET_INCLUDE_
#define _XLH_FORTREND_VACUUMMIZE_SUBSYSTEM_WIDGET_INCLUDE_ 
#include  "kernel/FortrendUI/abstract_subsystem_widget.h"
#include  "Kernel/kernel.h"
#include  <QWidget>
#include  <vector>
#include  <memory>


namespace FC{
	
class QVacuumizeSubsystemWidgetPrivate;
class QVacuumizeSubsystemWidget : public QWidget{
    Q_OBJECT
public:
	QVacuumizeSubsystemWidget(const std::shared_ptr<IKernel>& kernel, QWidget* parent = NULL);
    ~QVacuumizeSubsystemWidget();
private slots:
    void onReset();
    void onGetStatus();
    void onOutput();
	void onTMPTMClicked(bool status);
	void onTMPLLAClicked(bool status);
	void onTMPLLBClicked(bool status);
	void onVPClicked(bool status);

	void onPAVTMClicked(bool status);
	void onPAVLLAClicked(bool status);
	void onPAVLLBClicked(bool status);
	void onPPVTMClicked(bool status);
	void onPPVLLAClicked(bool status);
	void onPPVLLBClicked(bool status);
	void onPBVTMClicked(bool status);
	void onPBVLLAClicked(bool status);
	void onPBVLLBClicked(bool status);
	void onOpenTMCavityAutoVacuumCommand();
	void onOpenLoadLock1AutoVacuumCommand();
	void onOpenLoadLock2AutoVacuumCommand();

	void showMessage(const QString& message);

private:
	void onAttributeUpdate()throw(KernelException);

private:
    Q_DECLARE_PRIVATE(QVacuumizeSubsystemWidget)
    QVacuumizeSubsystemWidgetPrivate *d_ptr;

};

}

#endif
