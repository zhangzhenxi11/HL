/**
* @file            fortrend_device_user_module.h
* @brief           device user
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Modules


#include "fortrend_device_user_module.h"

#include "Kernel/kernel_subsystem.h"
#include "Kernel/Fortrend/fortrend_signal_tower.h"
#include "kernel/kernel_api.h"



#include "Poco/Format.h"


namespace FC{

/**
* KernelUserModule
*/
DeviceUserModule::DeviceUserModule(IKernel* kernel)
	:kernel(kernel){

}

void DeviceUserModule::process(){
	//api
	auto api = kernel->getKernelModule<KernelApi>();
	//tower
	auto tower = kernel->getKernelModule<FortrendSignalTower>();
	if (tower){
		auto modules = kernel->getKernelModules <IKernelAlarmModule> ();
		bool busy_stat = false;
		bool error_stat = false;
		for (auto item : modules){
			//busy
			if (auto sub = std::dynamic_pointer_cast<IKernelSubSystem>(item)){
				if(sub->isBusy())busy_stat = true;
			}
			//error
			if (item->isAlarm()) error_stat = true;
		}
		//yellow
		tower->setOutPut(FortrendSignalTower::YELLOW_LIGHT, busy_stat ? FortrendSignalTower::OUT_ON : FortrendSignalTower::OUT_OFF);
		//error
		tower->setOutPut(FortrendSignalTower::RED_LIGHT, error_stat ? FortrendSignalTower::OUT_FLASH_FAST : FortrendSignalTower::OUT_OFF);
		tower->setOutPut(FortrendSignalTower::BUZZER, error_stat ? FortrendSignalTower::OUT_ON : FortrendSignalTower::OUT_OFF);
		//yellow
		tower->setOutPut(FortrendSignalTower::GREEN_LIGHT, !(busy_stat || error_stat) ? FortrendSignalTower::OUT_ON : FortrendSignalTower::OUT_OFF);
		
		if (api){
			//blue
			tower->setOutPut(FortrendSignalTower::BLUE_LIGHT, api->getCommunicationState() == KernelApi::COMMUNICATING ? FortrendSignalTower::OUT_ON : FortrendSignalTower::OUT_OFF);
		}
	}


}



}

