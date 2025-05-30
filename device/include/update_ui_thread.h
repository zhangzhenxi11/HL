/**
* @file            update_ui_thread.h
* @brief           update ui
* @author			xielonghua
*/

// Library: FortrendUI
// Package: VTM

#pragma once

#include <QObject>
#include <QThread>

class UpdateUIThread : public QThread
{
	Q_OBJECT
protected:
	void run();
public:
	UpdateUIThread(QObject *parent = 0);
	~UpdateUIThread();
signals:
	void UpdateSignal(int num);
};