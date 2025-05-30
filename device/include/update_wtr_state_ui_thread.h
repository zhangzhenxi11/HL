/**
* @file            update_wtr_state_ui_thread.h
* @brief           update wtr state auto vtm station widget
* @author			xielonghua
*/

// Library: FortrendUI
// Package: VTM

#pragma once

#include "kernel/kernel.h"

#include <QObject>
#include <QThread>
namespace FC{
	class UpdateWTRStateUIThread : public QThread
	{
		Q_OBJECT

	public:
		bool enabled = true;
		std::shared_ptr<IKernel> kernel;
	public:
		void run();
	public:
		UpdateWTRStateUIThread(QObject *parent = 0);
		~UpdateWTRStateUIThread();
	signals:
		void UpdateSignal();
	};
}
