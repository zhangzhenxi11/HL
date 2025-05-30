/**
* @file            fortrend_aligner_subsystem.h
* @brief           Fortrend Aligner widget
* @author		   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/Aligner


#ifndef _XLH_FORTREND_ALIGNER_SUBSYSTEM_WIDGET_INCLUDE_
#define _XLH_FORTREND_ALIGNER_SUBSYSTEM_WIDGET_INCLUDE_ 
#include  "kernel/FortrendUI/abstract_subsystem_widget.h"
#include  "fortrend_aligner_subsystem.h"
#include  <QWidget>
#include  <vector>
#include  <memory>


namespace FC{

class QAlignerSubsystemWidgetPrivate;
class  QAlignerSubsystemWidget : public QAbstractSubsystemWidget<FortrendAlignerSubsystem>{
	Q_OBJECT
public:
	QAlignerSubsystemWidget(const std::shared_ptr<FortrendAlignerSubsystem>& aligner, QWidget* parent = NULL);
	~QAlignerSubsystemWidget();
private slots:
	void onReset();
	void onGetStatus();
	void onAlign();

private:
	void init();
	void onAttributeUpdate();
private:
	Q_DECLARE_PRIVATE(QAlignerSubsystemWidget)
	QAlignerSubsystemWidgetPrivate *d_ptr;

};

}

#endif
