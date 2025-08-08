
/**
* @file            fortrend_StationMode_subsystem.h
* @brief           Fortrend STATIONMODE widget
* @author           xielonghua
*/

// Library: Fortrend
// Package: SubSystem/STATIONMODE

#ifndef _XLH_FORTREND_STATIONMODE_SUBSYSTEM_WIDGET_INCLUDE_
#define _XLH_FORTREND_STATIONMODE_SUBSYSTEM_WIDGET_INCLUDE_ 
#include  "kernel/FortrendUI/abstract_subsystem_widget.h"
#include  "fortrend_StationMode_subsystem.h"
#include  <QWidget>
#include  <vector>
#include  <memory>


namespace FC{

class QSTATIONMODESubsystemWidgetPrivate;
class QSTATIONMODESubsystemWidget : public QAbstractSubsystemWidget<FortrendSTATIONMODESubsystem>{
    Q_OBJECT
public:
    QSTATIONMODESubsystemWidget(const std::shared_ptr<FortrendSTATIONMODESubsystem>& fliper, QWidget* parent = NULL);
    ~QSTATIONMODESubsystemWidget();
private slots:
    void onReset();
    void onGetStatus();
    void onOutput();
    
    
    //special commands
    void onResetCommand();
    

private:
    Q_DECLARE_PRIVATE(QSTATIONMODESubsystemWidget)
    QSTATIONMODESubsystemWidgetPrivate *d_ptr;

};

}

#endif
