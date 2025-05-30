/**
* @file            update_station_state_ui_thread.h
* @brief           update station state auto vtm station widget
* @author			xielonghua
*/

// Library: FortrendUI
// Package: VTM

#pragma once

#include "kernel/kernel.h"

#include <QObject>
#include <QThread>
namespace FC{
	class UpdateStationStateUIThread : public QThread
	{
		Q_OBJECT
		
	public:
		bool enabled = true;
		std::shared_ptr<IKernel> kernel;
	public:
		void run();
	public:
		UpdateStationStateUIThread(QObject *parent = 0);
		~UpdateStationStateUIThread();
	signals:
		void UpdateSignal();
	};
}
