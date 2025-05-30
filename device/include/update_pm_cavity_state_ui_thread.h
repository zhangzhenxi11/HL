/**
* @file            update_pm_cavity_state_ui_thread.h
* @brief           update pm cavity state auto vtm station widget
* @author			xielonghua
*/

// Library: FortrendUI
// Package: VTM

#pragma once

#include "kernel/kernel.h"
#include "PMCavity/fortrend_pm_cavity_subsystem.h"

#include <QObject>
#include <QThread>
namespace FC{
	class UpdatePMCavityStateUIThread : public QThread
	{
		Q_OBJECT

	public:
		bool enabled = true;
		std::shared_ptr<FortrendPMCavitySubsystem> pm;
	public:
		void run();
	public:
		UpdatePMCavityStateUIThread(QObject *parent = 0);
		~UpdatePMCavityStateUIThread();
	signals:
		void UpdateSignal();
	};
}
