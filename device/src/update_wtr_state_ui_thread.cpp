/**
* @file            update_wtr_state_ui_thread.h
* @brief           update wtr state auto vtm station widget
* @author			xielonghua
*/

// Library: FortrendUI
// Package: VTM

#include "update_wtr_state_ui_thread.h"
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
	UpdateWTRStateUIThread::UpdateWTRStateUIThread(QObject *parent)
	{

	}

	UpdateWTRStateUIThread::~UpdateWTRStateUIThread()
	{
	}

	void UpdateWTRStateUIThread::run()
	{
		double tm_last_pressure = 0.01;
		bool wtr_arm_a_last_has_object_state = false;
		bool wtr_arm_b_last_has_object_state = false;		
		std::shared_ptr<FortrendTMCavitySubsystem> tm = kernel->getKernelModule<FortrendTMCavitySubsystem>("TM");
		std::shared_ptr<FortrendSunwayRobotSubsystem> wtr = kernel->getKernelModule<FortrendSunwayRobotSubsystem>("WTR");
		
		bool change_state = false;
		while (enabled)
		{
			change_state = false;

			auto tm_current_pressure = tm->getTMCavityVacuumValue();
			if (tm_current_pressure != tm_last_pressure)
			{
				tm_last_pressure = tm_current_pressure;
				change_state = true;
			}

			//wtr arm has wafer
			bool wtr_arm_a_current_state = wtr->hasObject(0);
			if (wtr_arm_a_current_state != wtr_arm_a_last_has_object_state)
			{
				wtr_arm_a_last_has_object_state = wtr_arm_a_current_state;
				change_state = true;
			}
			bool wtr_arm_b_current_state = wtr->hasObject(1);
			if (wtr_arm_b_current_state != wtr_arm_b_last_has_object_state)
			{
				wtr_arm_b_last_has_object_state = wtr_arm_b_current_state;
				change_state = true;
			}
			if (change_state)
			{
				emit UpdateSignal();
			}
			Sleep(500);
		}
	}


}
