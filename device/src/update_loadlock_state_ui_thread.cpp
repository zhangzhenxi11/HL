/**
* @file            update_loadlock_state_ui_thread.h
* @brief           update loadlock state auto vtm station widget
* @author			xielonghua
*/

// Library: FortrendUI
// Package: VTM

#include "update_loadlock_state_ui_thread.h"
#include "kernel/kernel_subsystem.h"
#include "Kernel/Fortrend/fortrend_cassette_manager.h"
#include "Kernel/Fortrend/cassette.h"

#include "LoadLock/fortrend_loadlock_subsystem.h"
#include "PMCavity/fortrend_pm_cavity_subsystem.h"
#include "SunwayRobot/fortrend_sunwayrobot_subsystem.h"
#include "TMCavity/fortrend_tm_cavity_subsystem.h"

#include <thread>
#include <Windows.h>
#include <iostream>
#include <vector>

namespace FC{
	UpdateLoadLockStateUIThread::UpdateLoadLockStateUIThread(QObject *parent)
	{

	}

	UpdateLoadLockStateUIThread::~UpdateLoadLockStateUIThread()
	{
	}

	void UpdateLoadLockStateUIThread::run()
	{
		IKernelSubSystem::State last_state = IKernelSubSystem::State::SUB_NORMAL;
		double last_pressure = 0.01;
	

		std::vector<Cassette::Mapping> current_all_mapping;
		std::vector<Cassette::Mapping> last_all_mapping;

		bool last_has_box_state = false;



		auto cassManager = loadlock->getKernel()->getKernelModule<FortrendCassetteManager>();
		auto cass = cassManager->getCassette(loadlock.get());
		for (size_t i = 0; i < cass->slotCount(); i++)
		{
			last_all_mapping.push_back(Cassette::Mapping::Unknown);
		}
		bool change_state = false;
		while (enabled)
		{
			change_state = false;
			//modules state
			auto current_state = loadlock->getState();
			if (current_state != last_state)
			{
				last_state = current_state;
				change_state = true;
			}

			// cavity vacuum
			auto current_pressure = loadlock->getVacuumValue();
			if (current_pressure != last_pressure)
			{
				last_pressure = current_pressure;
				change_state = true;
			}
			//loadlock cassette state
			auto current_has_box = loadlock->hasBoxPresent();
			if (current_has_box != last_has_box_state)
			{
				last_has_box_state = current_has_box;
				change_state = true;
			}
			current_all_mapping = cass->getAllMapping();
			for (size_t i = 0; i <current_all_mapping.size(); i++)
			{
				if (current_all_mapping[i] != last_all_mapping[i])
				{
					last_all_mapping[i] = current_all_mapping[i];
					change_state = true;
				}
			}
			if (change_state)
			{
				emit UpdateSignal();
			}
			Sleep(300);
		}
	}


}
