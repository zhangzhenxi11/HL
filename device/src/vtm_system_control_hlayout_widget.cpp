// Library: Core
// Package: Core

#include "vtm_system_control_hlayout_widget.h"
#include <QVBoxLayout>
#include <QMessageBox>


#include "Kernel/kernel.h"
#include "Kernel/kernel_log.h"
#include "Kernel/kernel_action_subsystem.h"
#include "Kernel/kernel_action.h" 



#if _MSC_VER >1600
#pragma execution_character_set("utf-8")
#endif
KERNEL_NS_BEGIN

class QVTMSystemControlHLayoutWidgetPrivate : public KernelEventListener{
	Q_DECLARE_PUBLIC(QVTMSystemControlHLayoutWidget)
public:
	QVTMSystemControlHLayoutWidgetPrivate(QVTMSystemControlHLayoutWidget* p, const std::shared_ptr<IKernel>& kernel);
	virtual void onKernelEvent(KernelEventModule* kernelModule, const IEventId::Ptr& eventId, KernelEventParameter* context){
		QMetaObject::invokeMethod(q_ptr, "q_update_status", Qt::AutoConnection);
	}
private:
	QPushButton* pause_button, *reset_button, *start_button, *abort_button,*auto_button;
	std::shared_ptr<IKernel> kernel;
	QVTMSystemControlHLayoutWidget* q_ptr;
	QSlotTransferCycleVTMWidget* cycle;
	FortrendDeviceModel* model;
	bool currentpause = false;
	bool currentreset = false;
	bool currentstart = false;
	bool currentabort = false;
	bool currentplcauto = false;

};

QVTMSystemControlHLayoutWidgetPrivate::QVTMSystemControlHLayoutWidgetPrivate(QVTMSystemControlHLayoutWidget* p, const std::shared_ptr<IKernel>& kernel)
:q_ptr(p)
, kernel(kernel){
	
}



	
//QVTMSystemControlHLayoutWidget
QVTMSystemControlHLayoutWidget::QVTMSystemControlHLayoutWidget(const std::shared_ptr<IKernel>& kernel,QSlotTransferCycleVTMWidget* cycle, FortrendDeviceModel* model, QWidget* parent)
	:QKernelModuleWidget(parent)
	, d_ptr(new QVTMSystemControlHLayoutWidgetPrivate(this, kernel)){
	Q_D(QVTMSystemControlHLayoutWidget);
	d->cycle = cycle;
	d->model = model;
	d->pause_button = new QPushButton();
	d->reset_button = new QPushButton();
	d->start_button = new QPushButton();
	d->abort_button = new QPushButton();
	d->auto_button = new QPushButton();
	QHBoxLayout* main_layout = new QHBoxLayout(this);
	main_layout->setSpacing(1);
	main_layout->setMargin(1);
	main_layout->addWidget(d->pause_button);
	main_layout->addWidget(d->reset_button);
	main_layout->addWidget(d->start_button);
	main_layout->addWidget(d->abort_button);
	main_layout->addWidget(d->auto_button);

	d->start_button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	d->abort_button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	d->pause_button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	d->reset_button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	d->auto_button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	d->start_button->setMinimumSize(QSize(100, 30)); 
	d->abort_button->setMinimumSize(QSize(100, 30)); 
	d->pause_button->setMinimumSize(QSize(100, 30)); 
	d->reset_button->setMinimumSize(QSize(100, 30)); 
	d->auto_button->setMinimumSize(QSize(100, 30)); 

	q_update_status();
	btnIco(d->kernel);

	d->kernel->addEventListener(d);
}

void QVTMSystemControlHLayoutWidget::btnIco(std::shared_ptr<IKernel> kernel){
	Q_D(QVTMSystemControlHLayoutWidget);
	static bool b_manual_automatic = false;


	btnPath(d->start_button, "main_start_Enabled.png", "开始");
	connect(d->start_button, &QPushButton::clicked, this, [=]{
		if (d->auto_button->toolTip() == "手动"){
			logWarn("Cycle", "请切换为自动模式后启动!");
			return;
		}
		d->model->StatusClose();//禁用

		d->cycle->clickStart();
		btnUpdatePath();
	});
	d->start_button->setEnabled(false);

	d->pause_button->setEnabled(false);
	btnPath(d->pause_button, "main_Pause.png", "暂停");
	connect(d->pause_button, &QPushButton::clicked, this, [=]{
		d->model->StatusOpen();//启用
		btnUpdatePath();
		d->cycle->clickPause();
	});

	d->abort_button->setEnabled(false);
	btnPath(d->abort_button, "main_stop.png", "停止");
	connect(d->abort_button, &QPushButton::clicked, this, [=]{
		d->model->StatusOpen();//启用
		btnUpdatePath();
		d->cycle->clickAbort();
	});


	static bool b_reset = false;
	btnPath(d->reset_button, "main_reset.png", "复位");
	connect(d->reset_button, &QPushButton::clicked, this, [=]{
		if (d->auto_button->toolTip() == "手动"){
			logWarn("Cycle", "请切换为自动模式后启动!");
			return;
		}
		d->model->StatusOpen();//启用
		b_reset = true;
		btnUpdatePath();
		d->cycle->clickReset();

	});


	QTimer * btn_reset_time = new QTimer(this);
	connect(btn_reset_time, &QTimer::timeout, this, [=]{
		if (b_reset){
			/*d->start_button->setEnabled(true);
			d->reset_button->setEnabled(true);
			d->auto_button->setEnabled(true);*/
			b_reset = false;
			/*b_manual_automatic = true;*/
		}
		if (d->cycle->isAuto()){
			btnPath(d->auto_button, "main_automaticMode.png", "自动");
		}
		else{
			btnPath(d->auto_button, "main_manualmode.png", "手动");
		}
		btnUpdatePath();
	});
	//btn_reset_time->start(400);



	connect(d->auto_button, &QPushButton::clicked, this, [=]{
		if (b_manual_automatic == false){
			b_manual_automatic = true;
			d->cycle->clickplcAuto(false);
			btnPath(d->auto_button, "main_manualmode.png", "手动");
		}
		else
		{
			b_manual_automatic = false;
			d->cycle->clickplcAuto(true);
			btnPath(d->auto_button, "main_automaticMode.png", "自动");
		}

	});


	btnUpdatePath();
}
void QVTMSystemControlHLayoutWidget::btnUpdatePath(){
	Q_D(QVTMSystemControlHLayoutWidget);

	if (d->cycle->isEnabledStart() != d->currentstart){
		d->currentstart = d->cycle->isEnabledStart();
		d->start_button->setEnabled(d->currentstart);
		logInform("Test", "d->currentstart=%d", d->currentstart);
		if (!d->cycle->isEnabledStart()){//d->start_button->isEnabled() == false
			btnPath(d->start_button, "main_start_Enabled.png", "");
			btnPath(d->auto_button, "main_automaticMode_Enabled.png", "");
		}
		else{
			btnPath(d->start_button, "main_start.png", "");
		}
	}

	if (d->cycle->isEnabledPause() != d->currentpause){
		d->currentpause = d->cycle->isEnabledPause();
		d->pause_button->setEnabled(d->currentpause);
		logInform("Test", "d->currentpause=%d", d->currentpause);
		if (!d->cycle->isEnabledPause()){//d->pause_button->isEnabled() == false
			btnPath(d->pause_button, "main_Pause_Enabled.png", "");
			btnPath(d->auto_button, "main_automaticMode.png", "");
		}
		else{
			btnPath(d->pause_button, "main_Pause.png", "");
		}
	}
	if (d->cycle->isEnabledAbort() != d->currentabort){
		d->currentabort = d->cycle->isEnabledAbort();
		d->abort_button->setEnabled(d->currentabort);
		logInform("Test", "d->currentabort=%d", d->currentabort);
		if (!d->cycle->isEnabledAbort()){//d->abort_button->isEnabled() == false
			btnPath(d->abort_button, "main_stop_Enabled.png", "");
			btnPath(d->auto_button, "main_automaticMode.png", "");
		}
		else{
			btnPath(d->abort_button, "main_stop.png", "");
		}
	}
	if (d->cycle->isEnabledReset() != d->currentreset){
		d->currentreset = d->cycle->isEnabledReset();
		d->reset_button->setEnabled(d->currentreset);
		logInform("Test", "d->currentreset=%d", d->currentreset);
		if (!d->cycle->isEnabledReset()){//d->reset_button->isEnabled() == false
			btnPath(d->reset_button, "main_reset_Enabled.png", "");
		}
		else{
			btnPath(d->reset_button, "main_reset.png", "");
		}
	}

	if (d->cycle->isEnabledplcAuto() != d->currentplcauto){
		d->currentplcauto = d->cycle->isEnabledplcAuto();
		logInform("Test", "d->currentplcauto=%d", d->currentplcauto);
		if (!d->cycle->isEnabledplcAuto()){//d->reset_button->isEnabled() == false
			btnPath(d->auto_button, "main_manualmode.png", "");
		}
		else{
			btnPath(d->auto_button, "main_automaticMode.png", "");
		}
	}
	

}

void QVTMSystemControlHLayoutWidget::btnPath(QPushButton *btn, QString t_png, QString btn_tool_txt){

	QString path = QCoreApplication::applicationDirPath() + "/images/" + t_png;
	btn->setIcon(QIcon(path));

	btn->setIconSize(QSize(35, 35));
	if (btn_tool_txt != ""){
		btn->setToolTip(QString::fromLocal8Bit(btn_tool_txt.toUtf8()));
		btn->setStyleSheet("QToolTip { border:1px solid rgb(118, 118, 118); background-color: rgba(18, 26, 40,0.7); color:rgb(255, 255, 255); font:14px; }");
	}
}

QVTMSystemControlHLayoutWidget::~QVTMSystemControlHLayoutWidget(){
	Q_D(QVTMSystemControlHLayoutWidget);
	d->kernel->removeEventListener(d);
	delete d_ptr;
}

void QVTMSystemControlHLayoutWidget::q_update_status(){
	Q_D(QVTMSystemControlHLayoutWidget);

	/*if (d->kernel->isPaused()){
		d->pause_button->setText("Resume/Abort");
	}
	else{
		d->pause_button->setText("Pause");
	}*/

}

KERNEL_NS_END
