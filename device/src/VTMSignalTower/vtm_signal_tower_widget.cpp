// Library: FortrendUI
// Package: Modules
//
// signal  tower  widget
//
// author xielonghua
//

#include "VTMSignalTower/vtm_signal_tower_widget.h"
#include "VTMSignalTower/fortrend_vtm_signal_tower.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QDebug>

#include <map>

#define QSS(on)  "QLabel:disabled{background-color:#000;} QLabel{background-color:#"#on";}"

KERNEL_NS_BEGIN


/**
* QFortrendVTMSignalTowerlWidgetPrivate
*/
class QFortrendVTMSignalTowerlWidgetPrivate: public KernelListener<FortrendVTMSignalTower>{
public:
	QFortrendVTMSignalTowerlWidgetPrivate(QFortrendVTMSignalTowerlWidget*p);
	virtual void onAttributeChange(const FortrendVTMSignalTower* module);
public:
	std::shared_ptr<FortrendVTMSignalTower> module;
	QFortrendVTMSignalTowerlWidget* pParent;
	std::map<FortrendVTMSignalTower::Output, QLabel*>  signal_labels;
};

QFortrendVTMSignalTowerlWidgetPrivate::QFortrendVTMSignalTowerlWidgetPrivate(QFortrendVTMSignalTowerlWidget*p)
:pParent(p){
	
	signal_labels[FortrendVTMSignalTower::RED_LIGHT] = new QLabel;
	//signal_labels[FortrendVTMSignalTower::RED_LIGHT]->setStyleSheet(QSS(f00));
	signal_labels[FortrendVTMSignalTower::RED_LIGHT]->setStyleSheet("QLabel{background-color:rgb(0,0,0);}");

	signal_labels[FortrendVTMSignalTower::YELLOW_LIGHT] = new QLabel;
	//signal_labels[FortrendVTMSignalTower::YELLOW_LIGHT]->setStyleSheet(QSS(ff0));
	signal_labels[FortrendVTMSignalTower::YELLOW_LIGHT]->setStyleSheet("QLabel{background-color:rgb(0,0,0);}");

	signal_labels[FortrendVTMSignalTower::GREEN_LIGHT] = new QLabel;
	//signal_labels[FortrendVTMSignalTower::GREEN_LIGHT]->setStyleSheet(QSS(0f0));
	signal_labels[FortrendVTMSignalTower::GREEN_LIGHT]->setStyleSheet("QLabel{background-color:rgb(0,0,0);}");

	signal_labels[FortrendVTMSignalTower::BLUE_LIGHT] = new QLabel;
	//signal_labels[FortrendVTMSignalTower::BLUE_LIGHT]->setStyleSheet(QSS(00f));
	signal_labels[FortrendVTMSignalTower::BLUE_LIGHT]->setStyleSheet("QLabel{background-color:rgb(0,0,0);}");

	signal_labels[FortrendVTMSignalTower::BUZZER] = new QLabel;
	//signal_labels[FortrendVTMSignalTower::BUZZER]->setStyleSheet(QSS(fff));
	signal_labels[FortrendVTMSignalTower::BUZZER]->setStyleSheet("QLabel{background-color:rgb(0,0,0);}");

	pParent->setObjectName("frame");
	pParent->setAttribute(Qt::WA_StyledBackground);
	pParent->setStyleSheet("QWidget#frame:disabled{border: 1px solid red;}");
}

void QFortrendVTMSignalTowerlWidgetPrivate::onAttributeChange(const FortrendVTMSignalTower* subststem){
	QMetaObject::invokeMethod(pParent, "onUpdateSignals", Qt::AutoConnection);
}

/**
* QFortrendVTMSignalTowerlWidget
*/
QFortrendVTMSignalTowerlWidget::QFortrendVTMSignalTowerlWidget(const std::shared_ptr<FortrendVTMSignalTower>& module, QWidget* parent)
: QWidget(parent)
, tower(module)
, d(new QFortrendVTMSignalTowerlWidgetPrivate(this)){
 
	QVBoxLayout* main_layout = new QVBoxLayout(this);
	main_layout->setMargin(1);
	for (auto& it : d->signal_labels){
		QSizePolicy policy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		it.second->setSizePolicy(policy);
		main_layout->addWidget(it.second);

	}

	//add Listener
	tower->addListener(d.get());
	setFixedSize(40,80);

	installEventFilter(this);
	onUpdateSignals();
}

QFortrendVTMSignalTowerlWidget::~QFortrendVTMSignalTowerlWidget(){
	tower->removeListener(d.get());
}

bool  QFortrendVTMSignalTowerlWidget::eventFilter(QObject *obj, QEvent *event){
	if (event->type() == QEvent::MouseButtonPress) {
		QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);	// 转换为鼠标事件对象
		if (mouseEvent->button() == Qt::LeftButton) {		// 鼠标左键按下
			emit clicked();
		}
		if (mouseEvent->button() == Qt::RightButton) {		// 鼠标右键按下
			tower->setEnabled(!tower->isEnabled());
		}
	}
	return QWidget::eventFilter(obj, event);
}


void QFortrendVTMSignalTowerlWidget::onUpdateSignals(){
	/*for (auto& it : d->signal_labels){
		it.second->setEnabled(tower->getOutput(it.first));
	}
	this->setEnabled(tower->isEnabled());*/
	if (tower->getOutput(FortrendVTMSignalTower::RED_LIGHT))
	{
		d->signal_labels[FortrendVTMSignalTower::RED_LIGHT]->setStyleSheet("QLabel{background-color:rgb(255,0,0);}");
	}
	else
	{
		d->signal_labels[FortrendVTMSignalTower::RED_LIGHT]->setStyleSheet("QLabel{background-color:rgb(0,0,0);}");
	}

	if (tower->getOutput(FortrendVTMSignalTower::YELLOW_LIGHT))
	{
		d->signal_labels[FortrendVTMSignalTower::YELLOW_LIGHT]->setStyleSheet("QLabel{background-color:rgb(255,255,0);}");
	}
	else
	{
		d->signal_labels[FortrendVTMSignalTower::YELLOW_LIGHT]->setStyleSheet("QLabel{background-color:rgb(0,0,0);}");
	}

	if (tower->getOutput(FortrendVTMSignalTower::GREEN_LIGHT))
	{
		d->signal_labels[FortrendVTMSignalTower::GREEN_LIGHT]->setStyleSheet("QLabel{background-color:rgb(0,255,0);}");
	}
	else
	{
		d->signal_labels[FortrendVTMSignalTower::GREEN_LIGHT]->setStyleSheet("QLabel{background-color:rgb(0,0,0);}");
	}
	if (tower->getOutput(FortrendVTMSignalTower::BLUE_LIGHT))
	{
		d->signal_labels[FortrendVTMSignalTower::BLUE_LIGHT]->setStyleSheet("QLabel{background-color:rgb(0,0,255);}");
	}
	else
	{
		d->signal_labels[FortrendVTMSignalTower::BLUE_LIGHT]->setStyleSheet("QLabel{background-color:rgb(0,0,0);}");
	}
	if (tower->getOutput(FortrendVTMSignalTower::BUZZER))
	{
		d->signal_labels[FortrendVTMSignalTower::BUZZER]->setStyleSheet("QLabel{background-color:rgb(255,255,255);}");
	}
	else
	{
		d->signal_labels[FortrendVTMSignalTower::BUZZER]->setStyleSheet("QLabel{background-color:rgb(0,0,0);}");
	}
	
}

KERNEL_NS_END
