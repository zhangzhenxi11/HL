/**
* @file            update_loadlock_state_ui_thread.h
* @brief           update loadlock state auto vtm station widget
* @author			xielonghua
*/

// Library: FortrendUI
// Package: VTM

#pragma once

#include "kernel/kernel.h"
#include "LoadLock/fortrend_loadlock_subsystem.h"

#include <QObject>
#include <QThread>
namespace FC{
	class UpdateLoadLockStateUIThread : public QThread
	{
		Q_OBJECT

	public:
		bool enabled = true;
		std::shared_ptr<FortrendLoadLockSubsystem> loadlock;
	public:
		void run();
	public:
		UpdateLoadLockStateUIThread(QObject *parent = 0);
		~UpdateLoadLockStateUIThread();
	signals:
		void UpdateSignal();
	};
}
