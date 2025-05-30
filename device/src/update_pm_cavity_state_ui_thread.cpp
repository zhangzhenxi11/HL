/**
* @file            update_pm_state_ui_thread.h
* @brief           update pm state auto vtm station widget
* @author			xielonghua
*/

// Library: FortrendUI
// Package: VTM

#include "update_pm_cavity_state_ui_thread.h"
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
	UpdatePMCavityStateUIThread::UpdatePMCavityStateUIThread(QObject *parent)
	{

	}

	UpdatePMCavityStateUIThread::~UpdatePMCavityStateUIThread()
	{
	}

	void UpdatePMCavityStateUIThread::run()
	{
		
		IKernelSubSystem::State last_state = IKernelSubSystem::State::SUB_NORMAL;

		
		double last_pressure = 0.01;
		double last_tempterature = 0.01;

		Cassette::Mapping last_mapping = Cassette::Mapping::Unknown;

		int last_wafer_slot = -1;
		auto cassManager = pm->getKernel()->getKernelModule<FortrendCassetteManager>();
		auto cass = cassManager->getCassette(pm.get());

		bool change_state = false;
		while (enabled)
		{
			change_state = false;
			
			auto current_state = pm->getState();
			if (current_state != last_state)
			{
				last_state = current_state;
				change_state = true;
			}
			

			

			auto current_pressure = pm->getVacuumValue();
			if (current_pressure != last_pressure)
			{
				last_pressure = current_pressure;
				change_state = true;
			}
		

			//pm cavity temprature
			auto current_temperature = pm->getTemperatureValue();
			if (current_temperature != last_tempterature)
			{
				last_tempterature = current_temperature;
				change_state = true;
			}
			

			//pm cavity wafer
			auto current_mapping = cass->getMapping(1);
			if (current_mapping != last_mapping)
			{
				last_mapping = current_mapping;
				change_state = true;
			}
			

			int current_wafer_slot = pm->getWaferSlot();
			if (current_wafer_slot != last_wafer_slot)
			{
				last_wafer_slot = current_wafer_slot;
				change_state = true;
			}

			

			
			if (change_state)
			{
				emit UpdateSignal();
			}
			Sleep(300);
		}
	}


}
