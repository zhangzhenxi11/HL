/**
* @file            fortrend_device_script_module.h
* @brief           device script
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Modules

#include "fortrend_device_script_module.h"
//kernel & utils
#include "Kernel/kernel_log.h"
#include "Kernel/kernel_default_block_manager.h"
#include "Kernel/kernel_action_subsystem.h"
#include "Kernel/kernel_script_subsystem.h"
#include "Kernel/kernel_sim_iocontrol.h"
#include "Kernel/kernel_configure.h"
#include "kernel/kernel_subsystem_reset_command.h"
#include "kernel/kernel_action.h"
#include "kernel/kernel_iocontrol.h"
#include "kernel/kaguya/type.hpp"

//fortrend subsystem
#include "Kernel/Fortrend/fortrend_wafer_robot_subsystem.h"
#include "Kernel/Fortrend/fortrend_foup_robot_subsystem.h"
#include "Kernel/Fortrend/fortrend_loadport_subsystem.h"
#include "Kernel/Fortrend/e84_subsystem.h"
#include "Kernel/Fortrend/rnd_aligner_subsystem.h"
#include "Kernel/Fortrend/fortrend_hex_rfid_subsystem.h"
#include "Kernel/Fortrend/fortrend_signal_tower.h"
#include "Kernel/Fortrend/fortrend_cassette_manager.h"
#include "Kernel/Fortrend/fortrend_storge_station.h"
#include "Kernel/Fortrend/fortrend_wafer_tool_station.h"

//fortrend command
#include "kernel/Fortrend/robot_abstract_command.h"
#include "kernel/Fortrend/robot_getfoup_abstract_command.h"
#include "kernel/Fortrend/robot_putfoup_abstract_command.h"
#include "kernel/Fortrend/robot_getwafer_abstract_command.h"
#include "kernel/Fortrend/robot_putwafer_abstract_command.h"
#include "kernel/Fortrend/loadport_abstract_command.h"
#include "kernel/Fortrend/rfid_abstract_command.h"


#include "Poco/Format.h"


namespace FC{

/**
* DeviceScriptModule
*/
DeviceScriptModule::DeviceScriptModule(IKernel* kernel)
	:kernel(kernel){

}

void DeviceScriptModule::initialize() throw(KernelException){
	//register script function
	for (auto sub : kernel->getKernelModules<KernelScriptSubsystem>()){
		regCoreFunction(sub);
		regWTRFunction(sub);
		regFTRFunction(sub);
		regLPFunction(sub);
		regIOFunction(sub);
		regRFIDFunction(sub);
	}
}

void DeviceScriptModule::unInitialize(){

}



void DeviceScriptModule::regCoreFunction(const std::shared_ptr<KernelScriptSubsystem>&script){
	//register kernel
	script->getScriptState()["Kernel"].setClass(kaguya::UserdataMetatable<IKernel>());
	script->getScriptState()["Dev"] = kernel;

	//register command class
	script->getScriptState()["IKernelCommand"].setClass(kaguya::UserdataMetatable<IKernelCommand>()
		.addFunction("Wait", &IKernelCommand::wait)
		.addFunction("HasError", &IKernelCommand::hasError));
	script->getScriptState()["KernelSubsystemCommand"].setClass(kaguya::UserdataMetatable<KernelSubsystemCommand, IKernelCommand>());
	script->getScriptState()["KernelSubsystemResetCommand"].setClass(kaguya::UserdataMetatable<KernelSubsystemResetCommand, KernelSubsystemCommand>());

	//register actions
	script->getScriptState()["KernelAction"].setClass(kaguya::UserdataMetatable<KernelAction, KernelSubsystemCommand>());


	//register station class
	script->getScriptState()["FortrendStation"].setClass(kaguya::UserdataMetatable<FortrendStation>()
		.addFunction("HasBoxPresent", &FortrendStation::hasBoxPresent)
		.addFunction("HasDoorOpend", &FortrendStation::hasDoorOpend)
	);
	script->getScriptState()["FortrendAbstractStation"].setClass(kaguya::UserdataMetatable<FortrendAbstractStation, FortrendStation>());
	script->getScriptState()["FortrendStorgeStation"].setClass(kaguya::UserdataMetatable<FortrendStorgeStation, FortrendAbstractStation>());
	script->getScriptState()["FortrendWaferToolStation"].setClass(kaguya::UserdataMetatable<FortrendWaferToolStation, FortrendAbstractStation>());

	script->getScriptState()["FortrendAbstractLoadPort"].setClass(kaguya::UserdataMetatable<FortrendAbstractLoadPort, FortrendAbstractStation>());
	script->getScriptState()["FortrendAbstractAligner"].setClass(kaguya::UserdataMetatable<FortrendAbstractAligner, FortrendAbstractStation>());


	//register subsystem class
	script->getScriptState()["IKernelSubSystem"].setClass(kaguya::UserdataMetatable<IKernelSubSystem, KernelEventModule>());
	script->getScriptState()["KernelAbstractSubSystem"].setClass(kaguya::UserdataMetatable<KernelAbstractSubSystem,IKernelSubSystem>()
		.addFunction("StartCommand", &KernelAbstractSubSystem::startCommand)
		.addFunction("GetState", &KernelAbstractSubSystem::getState)
		.addFunction("IsBusy", &KernelAbstractSubSystem::isBusy)
		.addFunction("CreateResetCommand", &KernelAbstractSubSystem::createResetCommand)
		.addFunction("CreateUpdateCommand", &KernelAbstractSubSystem::createUpdateCommand)
	);
	script->getScriptState()["E84SubSystem"].setClass(kaguya::UserdataMetatable<E84SubSystem, KernelAbstractSubSystem>());
	script->getScriptState()["KernelActionSubsytem"].setClass(kaguya::UserdataMetatable<KernelActionSubsytem, KernelAbstractSubSystem>());
	script->getScriptState()["RobotAbstractSubsystem"].setClass(kaguya::UserdataMetatable<RobotAbstractSubsystem, kaguya::MultipleBase<KernelAbstractSubSystem, FortrendAbstractStation> >());
	script->getScriptState()["LoadPortAbstractSubsystem"].setClass(kaguya::UserdataMetatable<LoadPortAbstractSubsystem, kaguya::MultipleBase<KernelAbstractSubSystem, FortrendAbstractLoadPort> >()
		.addFunction("CreateOpenDoorCommand", &LoadPortAbstractSubsystem::createOpenDoorCommand)
		.addFunction("CreateCloseDoorCommand", &LoadPortAbstractSubsystem::createCloseDoorCommand)
		.addFunction("CreateLockBoxCommand", &LoadPortAbstractSubsystem::createLockBoxCommand)
		.addFunction("CreateUnlockBoxCommand", &LoadPortAbstractSubsystem::createUnlockBoxCommand)
		.addFunction("CreateOpenBoxCommand", &LoadPortAbstractSubsystem::createOpenBoxCommand)
		.addFunction("CreateCloseBoxCommand", &LoadPortAbstractSubsystem::createCloseBoxCommand)
		.addFunction("CreateGetMapCommand", &LoadPortAbstractSubsystem::createGetMapCommand)
	);


	script->getScriptState()["AlignerAbstractSubsystem"].setClass(kaguya::UserdataMetatable<AlignerAbstractSubsystem, kaguya::MultipleBase<KernelAbstractSubSystem, FortrendAbstractAligner> >());
	script->getScriptState()["RfidAbstractSubsystem"].setClass(kaguya::UserdataMetatable<RfidAbstractSubsystem,KernelAbstractSubSystem>()
		.addFunction("CreateReadCommand", &RfidAbstractSubsystem::createReadCommand)
	);

	//station module register
	for (auto sub : kernel->getKernelModules<FortrendStation>()){
		script->getScriptState()[sub->getName()] = sub;
	}

	//all action subsystem module register
	for (auto sub : kernel->getKernelModules<KernelActionSubsytem>()){
		script->getScriptState()[sub->getName()] = sub;
	}


}

void DeviceScriptModule::regWTRFunction(const std::shared_ptr<KernelScriptSubsystem>&script){
	script->getScriptState()["RobotGetPutObjectCommand"].setClass(kaguya::UserdataMetatable<RobotGetPutObjectCommand, KernelSubsystemCommand>());
	script->getScriptState()["RobotAbstractGetPutWaferCommand"].setClass(kaguya::UserdataMetatable<RobotAbstractGetPutWaferCommand, RobotGetPutObjectCommand>());

	script->getScriptState()["RobotAbstractReadyPutWaferCommand"].setClass(kaguya::UserdataMetatable<RobotAbstractReadyPutWaferCommand, RobotGetPutObjectCommand>());
	script->getScriptState()["RobotAbstractReadyGetWaferCommand"].setClass(kaguya::UserdataMetatable<RobotAbstractReadyGetWaferCommand, RobotGetPutObjectCommand>());
	script->getScriptState()["RobotAbstractGetWaferCommand"].setClass(kaguya::UserdataMetatable<RobotAbstractGetWaferCommand, RobotGetPutObjectCommand>());
	script->getScriptState()["RobotAbstractPutWaferCommand"].setClass(kaguya::UserdataMetatable<RobotAbstractPutWaferCommand, RobotGetPutObjectCommand>());

	

	//WaferRobotAbstractSubsystem
	script->getScriptState()["WaferRobotAbstractSubsystem"].setClass(kaguya::UserdataMetatable<WaferRobotAbstractSubsystem, RobotAbstractSubsystem>()
		.addFunction("CreateReadyGetCommand", &WaferRobotAbstractSubsystem::createReadyGetCommand)
		.addFunction("CreateReadyPutCommand", &WaferRobotAbstractSubsystem::createReadyPutCommand)
		.addFunction("CreateGetCommand", &WaferRobotAbstractSubsystem::createGetCommand)
		.addFunction("CreatePutCommand", &WaferRobotAbstractSubsystem::createPutCommand)
	);

	//wtr register
	for (auto sub : kernel->getKernelModules<WaferRobotAbstractSubsystem>()){
		script->getScriptState()[sub->getName()] = sub;
	}

}

void DeviceScriptModule::regFTRFunction(const std::shared_ptr<KernelScriptSubsystem>&script){
	script->getScriptState()["RobotAbstractReadyPutFoupCommand"].setClass(kaguya::UserdataMetatable<RobotAbstractReadyPutFoupCommand, RobotGetPutObjectCommand>());
	script->getScriptState()["RobotAbstractReadyGetFoupCommand"].setClass(kaguya::UserdataMetatable<RobotAbstractReadyGetFoupCommand, RobotGetPutObjectCommand>());
	script->getScriptState()["RobotAbstractGetFoupCommand"].setClass(kaguya::UserdataMetatable<RobotAbstractGetFoupCommand, RobotGetPutObjectCommand>());
	script->getScriptState()["RobotAbstractPutFoupCommand"].setClass(kaguya::UserdataMetatable<RobotAbstractPutFoupCommand, RobotGetPutObjectCommand>());



	//FoupRobotAbstractSubsystem
	script->getScriptState()["FoupRobotAbstractSubsystem"].setClass(kaguya::UserdataMetatable<FoupRobotAbstractSubsystem, RobotAbstractSubsystem>()
		.addFunction("CreateReadyGetCommand", &FoupRobotAbstractSubsystem::createReadyGetCommand)
		.addFunction("CreateReadyPutCommand", &FoupRobotAbstractSubsystem::createReadyPutCommand)
		.addFunction("CreateGetCommand", &FoupRobotAbstractSubsystem::createGetCommand)
		.addFunction("CreatePutCommand", &FoupRobotAbstractSubsystem::createPutCommand)
	);

	//ftr register
	for (auto sub : kernel->getKernelModules<FoupRobotAbstractSubsystem>()){
		script->getScriptState()[sub->getName()] = sub;
	}
}

void DeviceScriptModule::regLPFunction(const std::shared_ptr<KernelScriptSubsystem>&script){
	script->getScriptState()["LoadPortAbstractOpenBoxCommand"].setClass(kaguya::UserdataMetatable<LoadPortAbstractOpenBoxCommand, KernelSubsystemCommand>());
	script->getScriptState()["LoadPortAbstractCloseBoxCommand"].setClass(kaguya::UserdataMetatable<LoadPortAbstractCloseBoxCommand, KernelSubsystemCommand>());
	script->getScriptState()["LoadPortAbstractGetMapCommand"].setClass(kaguya::UserdataMetatable<LoadPortAbstractGetMapCommand, KernelSubsystemCommand>());

	script->getScriptState()["LoadPortAbstractLockBoxCommand"].setClass(kaguya::UserdataMetatable<LoadPortAbstractLockBoxCommand, KernelSubsystemCommand>());
	script->getScriptState()["LoadPortAbstractUnlockBoxCommand"].setClass(kaguya::UserdataMetatable<LoadPortAbstractUnlockBoxCommand, KernelSubsystemCommand>());
	script->getScriptState()["LoadPortAbstractOpenDoorCommand"].setClass(kaguya::UserdataMetatable<LoadPortAbstractOpenDoorCommand, KernelSubsystemCommand>());
	script->getScriptState()["LoadPortAbstractCloseDoorCommand"].setClass(kaguya::UserdataMetatable<LoadPortAbstractCloseDoorCommand, KernelSubsystemCommand>());



	//loadport register
	for (auto sub : kernel->getKernelModules<LoadPortAbstractSubsystem>()){
		script->getScriptState()[sub->getName()] = sub;
	}
}


void DeviceScriptModule::regRFIDFunction(const std::shared_ptr<KernelScriptSubsystem>&script){
	script->getScriptState()["RfidAbstractReadCommand"].setClass(kaguya::UserdataMetatable<RfidAbstractReadCommand, KernelSubsystemCommand>());

	//rfid register
	for (auto sub : kernel->getKernelModules<RfidAbstractSubsystem>()){
		script->getScriptState()[sub->getName()] = sub;
	}
}



void DeviceScriptModule::regIOFunction(const std::shared_ptr<KernelScriptSubsystem>&script){
	script->getScriptState()["IKernelIOControl"].setClass(kaguya::UserdataMetatable<IKernelIOControl, IKernelModule>()
		.addFunction("GetInput", &IKernelIOControl::getInput)
		.addFunction("GetOutput", &IKernelIOControl::getOutput)
		.addFunction("SetOutput", &IKernelIOControl::setOutput)
	);

	//all io board
	for (auto sub : kernel->getKernelModules<IKernelIOControl>()){
		script->getScriptState()[sub->getName()] = sub;
	}
}


}

