/**
* @file            update_station_state_ui_thread.h
* @brief           update station state auto vtm station widget
* @author			xielonghua
*/

// Library: FortrendUI
// Package: VTM

#include "update_station_state_ui_thread.h"
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
	UpdateStationStateUIThread::UpdateStationStateUIThread(QObject *parent)
	{
		
	}

	UpdateStationStateUIThread::~UpdateStationStateUIThread()
	{
	}

	void UpdateStationStateUIThread::run()
	{
		IKernelSubSystem::State ll1_last_state = IKernelSubSystem::State::SUB_NORMAL;
		IKernelSubSystem::State ll2_last_state = IKernelSubSystem::State::SUB_NORMAL;
		IKernelSubSystem::State pm1_last_state = IKernelSubSystem::State::SUB_NORMAL;
		IKernelSubSystem::State pm2_last_state = IKernelSubSystem::State::SUB_NORMAL;
		IKernelSubSystem::State pm3_last_state = IKernelSubSystem::State::SUB_NORMAL;

		double ll1_last_pressure = 0.01;
		double ll2_last_pressure = 0.01;
		double tm_last_pressure = 0.01;
		double pm1_last_pressure = 0.01;
		double pm2_last_pressure = 0.01;
		double pm3_last_pressure = 0.01;

		double pm1_last_tempterature = 0.01;
		double pm2_last_tempterature = 0.01;
		double pm3_last_tempterature = 0.01;

		Cassette::Mapping pm1_last_mapping = Cassette::Mapping::Unknown;
		Cassette::Mapping pm2_last_mapping = Cassette::Mapping::Unknown;
		Cassette::Mapping pm3_last_mapping = Cassette::Mapping::Unknown;

		int pm1_last_wafer_slot = -1;
		int pm2_last_wafer_slot = -1;
		int pm3_last_wafer_slot = -1;

		std::vector<Cassette::Mapping> lk1_current_all_mapping;
		std::vector<Cassette::Mapping> lk2_current_all_mapping;
		std::vector<Cassette::Mapping> lk1_last_all_mapping;
		std::vector<Cassette::Mapping> lk2_last_all_mapping;

		bool wtr_arm_a_last_has_object_state = false;
		bool wtr_arm_b_last_has_object_state = false;

		bool ll1_last_has_box_state = false;
		bool ll2_last_has_box_state = false;

		std::shared_ptr<FortrendLoadLockSubsystem> lk1 = kernel->getKernelModule<FortrendLoadLockSubsystem>("LLA");
		std::shared_ptr<FortrendLoadLockSubsystem> lk2 = kernel->getKernelModule<FortrendLoadLockSubsystem>("LLB");
		//std::shared_ptr<FortrendPMCavitySubsystem> pm1 = kernel->getKernelModule<FortrendPMCavitySubsystem>("PM1");
		std::shared_ptr<FortrendPMCavitySubsystem> pm2 = kernel->getKernelModule<FortrendPMCavitySubsystem>("PM");
		//std::shared_ptr<FortrendPMCavitySubsystem> pm3 = kernel->getKernelModule<FortrendPMCavitySubsystem>("PM3");
		std::shared_ptr<FortrendTMCavitySubsystem> tm = kernel->getKernelModule<FortrendTMCavitySubsystem>("TM");
		std::shared_ptr<FortrendSunwayRobotSubsystem> wtr = kernel->getKernelModule<FortrendSunwayRobotSubsystem>("WTR");

		auto cassManager = lk2->getKernel()->getKernelModule<FortrendCassetteManager>();
		auto ll1_cass = cassManager->getCassette(lk1.get());
		auto ll2_cass = cassManager->getCassette(lk2.get());
		//auto pm1_cass = cassManager->getCassette(pm1.get());
		auto pm2_cass = cassManager->getCassette(pm2.get());
		//auto pm3_cass = cassManager->getCassette(pm3.get());

		for (size_t i = 0; i < ll1_cass->slotCount(); i++)
		{
			lk1_last_all_mapping.push_back(Cassette::Mapping::Unknown);
		}
		for (size_t i = 0; i < ll2_cass->slotCount(); i++)
		{
			lk2_last_all_mapping.push_back(Cassette::Mapping::Unknown);
		}


		bool change_state = false;
		while (enabled)
		{
			change_state = false;
			//modules state
			auto ll1_current_state = lk1->getState();
			if (ll1_current_state != ll1_last_state)
			{
				ll1_last_state = ll1_current_state;
				change_state = true;
			}
			auto ll2_current_state = lk2->getState();
			if (ll2_current_state != ll2_last_state)
			{
				ll2_last_state = ll2_current_state;
				change_state = true;
			}
			/*auto pm1_current_state = pm1->getState();
			if (pm1_current_state != pm1_last_state)
			{
				pm1_last_state = pm1_current_state;
				change_state = true;
			}*/
			auto pm2_current_state = pm2->getState();
			if (pm2_current_state != pm2_last_state)
			{
				pm2_last_state = pm2_current_state;
				change_state = true;
			}
			/*auto pm3_current_state = pm3->getState();
			if (pm3_current_state != pm3_last_state)
			{
				pm3_last_state = pm3_current_state;
				change_state = true;
			}*/

			// cavity vacuum
			auto ll1_current_pressure = lk1->getVacuumValue();
			if (ll1_current_pressure != ll1_last_pressure)
			{
				ll1_last_pressure = ll1_current_pressure;
				change_state = true;
			}
			auto ll2_current_pressure = lk2->getVacuumValue();
			if (ll2_current_pressure != ll2_last_pressure)
			{
				ll2_last_pressure = ll2_current_pressure;
				change_state = true;
			}
			auto tm_current_pressure = tm->getTMCavityVacuumValue();
			if (tm_current_pressure != tm_last_pressure)
			{
				tm_last_pressure = tm_current_pressure;
				change_state = true;
			}

		/*	auto pm1_current_pressure = pm1->getVacuumValue();
			if (pm1_current_pressure != pm1_last_pressure)
			{
				pm1_last_pressure = pm1_current_pressure;
				change_state = true;
			}*/
			auto pm2_current_pressure = pm2->getVacuumValue();
			if (pm2_current_pressure != pm2_last_pressure)
			{
				pm2_last_pressure = pm2_current_pressure;
				change_state = true;
			}
			/*auto pm3_current_pressure = pm3->getVacuumValue();
			if (pm3_current_pressure != pm3_last_pressure)
			{
				pm3_last_pressure = pm3_current_pressure;
				change_state = true;
			}*/

			//pm cavity temprature
			/*auto pm1_current_temperature = pm1->getTemperatureValue();
			if (pm1_current_temperature != pm1_last_tempterature)
			{
				pm1_last_tempterature = pm1_current_temperature;
				change_state = true;
			}*/
			auto pm2_current_temperature = pm2->getTemperatureValue();
			if (pm2_current_temperature != pm2_last_tempterature)
			{
				pm2_last_tempterature = pm2_current_temperature;
				change_state = true;
			}
			/*auto pm3_current_temperature = pm3->getTemperatureValue();
			if (pm3_current_temperature != pm3_last_tempterature)
			{
				pm3_last_tempterature = pm3_current_temperature;
				change_state = true;
			}*/

			//pm cavity wafer
			/*auto pm1_current_mapping = pm1_cass->getMapping(1);
			if (pm1_current_mapping != pm1_last_mapping)
			{
				pm1_last_mapping = pm1_current_mapping;
				change_state = true;
			}*/
			auto pm2_current_mapping = pm2_cass->getMapping(1);
			if (pm2_current_mapping != pm2_last_mapping)
			{
				pm2_last_mapping = pm2_current_mapping;
				change_state = true;
			}
			/*auto pm3_current_mapping = pm3_cass->getMapping(1);
			if (pm3_current_mapping != pm3_last_mapping)
			{
				pm3_last_mapping = pm3_current_mapping;
				change_state = true;
			}*/

			/*int pm1_current_wafer_slot = pm1->getWaferSlot();
			if (pm1_current_wafer_slot != pm1_last_wafer_slot)
			{
				pm1_last_wafer_slot = pm1_current_wafer_slot;
				change_state = true;
			}*/

			int pm2_current_wafer_slot = pm2->getWaferSlot();
			if (pm2_current_wafer_slot != pm2_last_wafer_slot)
			{
				pm2_last_wafer_slot = pm2_current_wafer_slot;
				change_state = true;
			}

			/*int pm3_current_wafer_slot = pm3->getWaferSlot();
			if (pm3_current_wafer_slot != pm3_last_wafer_slot)
			{
				pm3_last_wafer_slot = pm3_current_wafer_slot;
				change_state = true;
			}*/

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




			//loadlock cassette state
			auto ll1_current_has_box = lk1->hasBoxPresent();
			if (ll1_current_has_box != ll1_last_has_box_state)
			{
				ll1_last_has_box_state = ll1_current_has_box;
				change_state = true;
			}
			auto ll2_current_has_box = lk2->hasBoxPresent();
			if (ll2_current_has_box != ll2_last_has_box_state)
			{
				ll2_last_has_box_state = ll2_current_has_box;
				change_state = true;
			}
			lk1_current_all_mapping = ll1_cass->getAllMapping();
			for (size_t i = 0; i <lk1_current_all_mapping.size(); i++)
			{
				if (lk1_current_all_mapping[i] != lk1_last_all_mapping[i])
				{
					lk1_last_all_mapping[i] = lk1_current_all_mapping[i];
					change_state = true;
				}
			}
			lk2_current_all_mapping = ll2_cass->getAllMapping();
			for (size_t i = 0; i <lk2_current_all_mapping.size(); i++)
			{
				if (lk2_current_all_mapping[i] != lk2_last_all_mapping[i])
				{
					lk2_last_all_mapping[i] = lk2_current_all_mapping[i];
					change_state = true;
				}
			}
			if (change_state)
			{
				emit UpdateSignal();
			}
			Sleep(500);
		}
	}


}
