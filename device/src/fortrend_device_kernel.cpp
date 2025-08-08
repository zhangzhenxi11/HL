/**
* @file            fortrend_device_kernel.h
* @brief           device kernel
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Main

#include <memory>
#include "Poco/Format.h"
#include <QMessageBox>

#include "fortrend_device_kernel.h"
#include "fortrend_device_script_module.h"
#include "fortrend_device_user_module.h"
#include "efem_hex_api.h"
#include "tcp_client_api.h"

#include "Kernel/kernel_log.h"
#include "Kernel/kernel_action_subsystem.h"
#include "kernel/kernel_script_subsystem.h"
#include "Kernel/kernel_sim_iocontrol.h"
#include "Kernel/kernel_configure.h"
#include "Kernel/kernel_subsystem_update_command.h"
#include "kernel/kernel_parallel_action.h"
#include "Kernel/kernel_subsystem_reset_command.h"


//fortrend common
#include "Kernel/Fortrend/fortrend_signal_tower.h"
#include "Kernel/Fortrend/fortrend_cassette_manager.h"

//VTM subsystem
#include "LoadLock/fortrend_loadlock_subsystem.h"
#include "SunwayRobot/fortrend_sunwayrobot_subsystem.h"
#include "Aligner/fortrend_aligner_subsystem.h"
#include "TMCavity/fortrend_tm_cavity_subsystem.h"
#include "PMCavity/fortrend_pm_cavity_subsystem.h"
#include "CoolingCavity/fortrend_cooling_cavity_subsystem.h"
#include "Pump/fortrend_pump_subsystem.h"
#include "VTMSignalTower/fortrend_vtm_signal_tower.h"
#include "fortrend_ascii_api.h"
//EFEM
#include "EFEM/efem_wafer_robot_subsystem.h"
#include "EFEM/efem_loadport_subsystem.h" 
#include "EFEM/efem_aligner_subsystem.h"

#include "STATIONMODE/fortrend_StationMode_subsystem.h" 

namespace FC{

	/**
	* FortrendDeviceKernel
	*/
	FortrendDeviceKernel::FortrendDeviceKernel(){

		KernelConfiguration::Ptr config(new KernelConfiguration("./kernel.xml"));
		//io board
		//bool sim_card = config->getBool("system.SimCard", false);


		auto api = std::make_shared<FortrendAsciiEFEMApi>(this);
		//this->addKernelModule(api);


		//VTM TOWER
		std::shared_ptr<FortrendVTMSignalTower> tower(new FortrendVTMSignalTower(this, api));
		//tower->setEnabled(false);
		
		//std::shared_ptr<FortrendSMIFSubsystem> smif(new FortrendSMIFSubsystem(this, "SMIF"));

		//VTM 
		std::shared_ptr<FortrendLoadLockSubsystem> loadlock1(new FortrendLoadLockSubsystem(this, "LLA"));
		std::shared_ptr<FortrendLoadLockSubsystem> loadlock2(new FortrendLoadLockSubsystem(this, "LLB"));
		std::shared_ptr<FortrendSunwayRobotSubsystem> wtr(new FortrendSunwayRobotSubsystem(this, "WTR"));
		std::shared_ptr<FortrendTMCavitySubsystem> tm(new FortrendTMCavitySubsystem(this, "TM"));
		//add
		std::shared_ptr<FortrendPMCavitySubsystem> pm1(new FortrendPMCavitySubsystem(this, "PM1"));
		std::shared_ptr<FortrendPMCavitySubsystem> pm2(new FortrendPMCavitySubsystem(this, "PM2"));
		std::shared_ptr<FortrendPMCavitySubsystem> pm3(new FortrendPMCavitySubsystem(this, "PM3"));
		std::shared_ptr<FortrendPMCavitySubsystem> pm4(new FortrendPMCavitySubsystem(this, "PM4"));
		std::shared_ptr<FortrendPumpSubsystem> pump(new FortrendPumpSubsystem(this, "PUMP"));

		//FortrendSTATIONMODESubsystem
		std::shared_ptr<FortrendSTATIONMODESubsystem> LLALower(new FortrendSTATIONMODESubsystem(this, "LLALower"));
		std::shared_ptr<FortrendSTATIONMODESubsystem> LLBUp(new FortrendSTATIONMODESubsystem(this, "LLBUp"));


		//std::shared_ptr<FortrendAlignerSubsystem> aligner(new FortrendAlignerSubsystem(this, "Aligner"));
		//std::shared_ptr<FortrendCoolingCavitySubsystem> cooling(new FortrendCoolingCavitySubsystem(this, "CoolingCavity"));

		//CASSETTE MANAGERLLALower
		std::shared_ptr<FortrendCassetteManager> cassManager(new FortrendCassetteManager(this));

		//USER SCRIPT SUBSYSTEM
		//std::shared_ptr<KernelScriptSubsystem> user_script(new KernelScriptSubsystem(this, "UserScript"));

		//this->addKernelModule(smif);

		this->addKernelModule(cassManager);
		this->addKernelModule(wtr);
		this->addKernelModule(loadlock1);
		this->addKernelModule(loadlock2);
		this->addKernelModule(tm);
		this->addKernelModule(pm1);
		this->addKernelModule(pm2);
		this->addKernelModule(pm3);
		this->addKernelModule(pm4);
		this->addKernelModule(pump);
		this->addKernelModule(LLALower);
		this->addKernelModule(LLBUp);

		//this->addKernelModule(aligner);
		//this->addKernelModule(cooling);

		this->addKernelModule(tower);
		//this->addKernelModule(user_script);

		this->addKernelModule(api);



		//use kernelmodule
		this->addKernelModule(IKernelModule::Ptr(new DeviceUserModule(this)));
		this->addKernelModule(IKernelModule::Ptr(new DeviceScriptModule(this)));


		//cassManager->addStation(smif.get());

		cassManager->addStation(loadlock1.get());
		cassManager->addStation(loadlock2.get());
		cassManager->addStation(wtr.get());
		cassManager->addStation(pm1.get());
		cassManager->addStation(pm2.get());
		cassManager->addStation(pm3.get());
		cassManager->addStation(pm4.get());
		cassManager->addStation(LLALower.get());
		cassManager->addStation(LLBUp.get());

		//cassManager->addStation(aligner.get());
		//cassManager->addStation(cooling.get());

		std::shared_ptr<EFEMWaferRobotSubsystem> ewtr(new EFEMWaferRobotSubsystem(this, "EWTR",api));
		std::shared_ptr<EFEMLPSubsystem> elp1(new EFEMLPSubsystem(this, "ELP1", api));
		std::shared_ptr<EFEMLPSubsystem> elp2(new EFEMLPSubsystem(this, "ELP2", api));
		std::shared_ptr<EFEMAlignerSubsystem> ealigner(new EFEMAlignerSubsystem(this, "EALIGNER",api));

		this->addKernelModule(ewtr);
		this->addKernelModule(elp1);
		this->addKernelModule(elp2);
		this->addKernelModule(ealigner);

		cassManager->addStation(ewtr.get());
		cassManager->addStation(elp1.get());
		cassManager->addStation(elp2.get());
		cassManager->addStation(ealigner.get());

		
	}


	void FortrendDeviceKernel::initialize()throw(KernelException){

		FortrendCassetteManager::Ptr cassManager = getKernelModule<FortrendCassetteManager>();

		//set smif virtual cassette
		//for (auto& smif : getKernelModules<FortrendSMIFSubsystem>()){
		//	//virtual cassette
		//	Cassette::Ptr cass(new Cassette(smif.get(), 15, true));
		//	cass->setBoxId(Poco::format("%s", smif->getName()));
		//	cass->setBoxOpened(true);
		//	for (size_t i = 0; i < 15; i++){
		//		cass->setMapping(i + 1, Cassette::Empty);
		//	}
		//	cassManager->loadCassette(smif.get(), cass);
		//}

		//set loadlock virtual cassette
		for (auto& lk : getKernelModules<FortrendLoadLockSubsystem>()){
			//virtual cassette
			Cassette::Ptr cass(new Cassette(lk.get(), lk->getCassetteSlotCount(), true));
			cass->setBoxOpened(true);
			for (size_t i = 0; i < lk->getCassetteSlotCount(); i++){
				cass->setMapping(i + 1, Cassette::Empty);
			}
			cassManager->loadCassette(lk.get(), cass);
		}

		//set robot virtual cassette
		for (auto& robot : getKernelModules<EFEMWaferRobotSubsystem>()){
			//virtual cassette
			Cassette::Ptr cass(new Cassette(robot.get(), 2, true));
			cass->setBoxId(Poco::format("%s", robot->getName()));
			cass->setBoxOpened(true);
			for (size_t i = 0; i < 2; i++){
				cass->setMapping(i + 1, Cassette::Empty);
			}
			cassManager->loadCassette(robot.get(), cass);
		}

		//set elp virtual cassette
		for (auto& ewtr : getKernelModules<EFEMLPSubsystem>()){
			//virtual cassette
			Cassette::Ptr cass(new Cassette(ewtr.get(), 25, true));
			cass->setBoxId(Poco::format("%s", ewtr->getName()));
			cass->setBoxOpened(true);
			for (size_t i = 0; i < 25; i++){
				cass->setMapping(i + 1, Cassette::Empty);
			}
			cassManager->loadCassette(ewtr.get(), cass);
		}

		//set efem aligner virtual cassette
		for (auto& ewtr : getKernelModules<EFEMAlignerSubsystem>()){
			//virtual cassette
			Cassette::Ptr cass(new Cassette(ewtr.get(), 1, true));
			cass->setBoxId(Poco::format("%s", ewtr->getName()));
			cass->setBoxOpened(true);
			for (size_t i = 0; i < 1; i++){
				cass->setMapping(i + 1, Cassette::Empty);
			}
			cassManager->loadCassette(ewtr.get(), cass);
		}

		//set wtr virtual cassette
		for (auto& wtr : getKernelModules<FortrendSunwayRobotSubsystem>()){
			//virtual cassette
			Cassette::Ptr cass(new Cassette(wtr.get(), 2, true));
			cass->setBoxId(Poco::format("%s", wtr->getName()));
			cass->setBoxOpened(true);
			for (size_t i = 0; i < 2; i++){
				cass->setMapping(i + 1, Cassette::Empty);
			}
			cassManager->loadCassette(wtr.get(), cass);
		}


		
		//set aligner virtual cassette
		//for (auto& aligner : getKernelModules<FortrendAlignerSubsystem>()){
		//	//virtual cassette
		//	Cassette::Ptr cass(new Cassette(aligner.get(), 1, true));
		//	cass->setBoxId(Poco::format("%s", aligner->getName()));
		//	cass->setBoxOpened(true);
		//	for (size_t i = 0; i < 1; i++){
		//		cass->setMapping(i + 1, Cassette::Empty);
		//	}
		//	cassManager->loadCassette(aligner.get(), cass);
		//}

		//set pm virtual cassette
		for (auto& pm : getKernelModules<FortrendPMCavitySubsystem>()){
			//virtual cassette
			Cassette::Ptr cass(new Cassette(pm.get(), 1, true));
			cass->setBoxId(Poco::format("%s", pm->getName()));
			cass->setBoxOpened(true);
			for (size_t i = 0; i < 1; i++){
				cass->setMapping(i + 1, Cassette::Empty);
			}
			cassManager->loadCassette(pm.get(), cass);
		}

		for (auto& stationMode : getKernelModules<FortrendSTATIONMODESubsystem>()) {
			//virtual cassette
			Cassette::Ptr cass(new Cassette(stationMode.get(), 1, true));
			cass->setBoxId(Poco::format("%s", stationMode->getName()));
			cass->setBoxOpened(true);
			for (size_t i = 0; i < 1; i++) {
				cass->setMapping(i + 1, Cassette::Empty);
			}
			cassManager->loadCassette(stationMode.get(), cass);
		}


		//set cooling virtual cassette
		//for (auto& col : getKernelModules<FortrendCoolingCavitySubsystem>()){
		//	//virtual cassette
		//	Cassette::Ptr cass(new Cassette(col.get(),1, true));
		//	cass->setBoxId(Poco::format("%s", col->getName()));
		//	cass->setBoxOpened(true);
		//	for (size_t i = 0; i < 1; i++){
		//		cass->setMapping(i + 1, Cassette::Empty);
		//	}
		//	cassManager->loadCassette(col.get(), cass);
		//}

		try{
			IKernel::initialize();
		}
		catch (std::exception&e){
			QMessageBox::critical(0, "Error", e.what());
		}
		//update subsystems
		updateSubsystem();

		/*auto api = getKernelModule<EFEMHexApi>();
		api->AddEventListListener();*/

	}

	void FortrendDeviceKernel::updateSubsystem(){
		try{
			for (auto& sub : getKernelModules<IKernelSubSystem>()){
				
				if (std::dynamic_pointer_cast<EFEMLPSubsystem>(sub) || std::dynamic_pointer_cast<EFEMWaferRobotSubsystem>(sub)){

				}
				else{
					auto cmd = sub->createUpdateCommand();
					if (cmd){
						sub->startCommand(cmd);
					}
				}
				
			}
		}
		catch (...){

		}
	}




	/**
	* create all subsystem reset action
	* default action is sequence reset all subsystem
	*/
	std::shared_ptr<KernelAction> FortrendDeviceKernel::createAllSubResetAction()const{
		//add sub commands for reset
		auto actSub = getKernelModule<KernelActionSubsytem>();
		KernelAction::Ptr robot_act(new KernelParallelAction("RobotReset", "robot sub reset"));
		KernelAction::Ptr other_act(new KernelParallelAction("OtherReset", "other sub reset"));
		KernelAction::Ptr main_act(new KernelAction("Reset", "All subsystem reset"));
		main_act->addCommand(actSub, robot_act);
		main_act->addCommand(actSub, other_act);

		for (auto& sub : getKernelModules<IKernelSubSystem>()){
			auto reset_cmd = sub->createResetCommand();
			if (!reset_cmd) continue;
			if (std::dynamic_pointer_cast<RobotAbstractSubsystem>(sub)){
				robot_act->addCommand(sub, reset_cmd);
			}
			else if (std::dynamic_pointer_cast<FortrendAlignerSubsystem>(sub)){
			
			}
			else{
				other_act->addCommand(sub, reset_cmd);
			}
		}
		return main_act;
	}
}

