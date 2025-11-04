/**
 * @file            aligner_abstract_subsystem_widget.h
 * @brief           abstract aligner subsystem widget
 * @author			xielonghua
 */

// Library: FortrendUI
// Package: Subsystem/Aligner



#ifndef _EFEM_ALIGNER_ABSTRACT_SUBSYSTEM_WIDGET_INCLUDE_
#define _EFEM_ALIGNER_ABSTRACT_SUBSYSTEM_WIDGET_INCLUDE_
#include  "Kernel/FortrendUI/fortrend_ui_macros.h"
#include  "Kernel/Fortrend/aligner_abstract_subsystem.h"
#include  "Kernel/FortrendUI/abstract_subsystem_widget.h"
#include  <QWidget>
#include  <vector>
#include  <memory>
namespace  KERNEL_NS {
class QEFEMAlignerAbstractSubsystemWidgetPrivate;

/**
 * @brief  abstract aligner subsystem widget
 */	
class  QEFEMAlignerAbstractSubsystemWidget : public QAbstractSubsystemWidget<AlignerAbstractSubsystem>{
	Q_OBJECT
public:
	QEFEMAlignerAbstractSubsystemWidget(const std::shared_ptr<AlignerAbstractSubsystem>& aligner, QWidget* parent = NULL);
	~QEFEMAlignerAbstractSubsystemWidget();
private slots:
	void generate();
	void reset();
	void align();
	void moveStartPos();
	void vaccum_on();
	void vaccum_off();
	void rotate();
	void readOcr(int index);
	void getMapdt();
	void updateOcrInfo();

private:
	Q_DECLARE_PRIVATE(QEFEMAlignerAbstractSubsystemWidget)
	QEFEMAlignerAbstractSubsystemWidgetPrivate *d_ptr;

};



}

#endif
