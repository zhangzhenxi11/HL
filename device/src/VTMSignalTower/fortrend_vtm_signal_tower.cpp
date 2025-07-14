// Library: Fortrend
// Package: Modules
//
// Fortrend VTM Signal tower
//
// author xielonghua
//
#include <iostream>
#include <memory>
#include "VTMSignalTower/fortrend_vtm_signal_tower.h"
#include "Kernel/kernel.h"
#include "Kernel/kernel_iocontrol.h"
#include "Kernel/kernel_log.h"
#include <cmath>

//#include "Pump/fortrend_pump_subsystem.h"


#include "Poco/Format.h"
#include "Poco/StringTokenizer.h"
#include "Poco/NumberParser.h"

KERNEL_NS_BEGIN


struct VTMSignalTowerIOBind {
	std::string address;
	bool lastVal;
	bool curVal;
};

static const std::string  TypeToName(FortrendVTMSignalTower::Output type){
	switch (type){
	case FortrendVTMSignalTower::RED_LIGHT: return "RED_LIGHT"; break;
	case FortrendVTMSignalTower::GREEN_LIGHT: return "GREEN_LIGHT"; break;
	case FortrendVTMSignalTower::BLUE_LIGHT: return "BLUE_LIGHT"; break;
	case FortrendVTMSignalTower::YELLOW_LIGHT: return "YELLOW_LIGHT"; break;
	case FortrendVTMSignalTower::BUZZER: return "BUZZER"; break;
	}
	return "";
}

/**
* FortrendVTMSignalTowerPrivate
*/
class FortrendVTMSignalTowerPrivate{
public:
	FortrendVTMSignalTowerPrivate(FortrendVTMSignalTower*p, std::shared_ptr<FortrendAsciiEFEMApi> a);
	void onProcess();
	void setBuzzerEnabled(bool enabled);
	bool isBuzzerEnabled();
	bool writeSignal(std::string address, bool value);
	void controlTowerSignal(FortrendVTMSignalTower::Output type, bool value);
	bool EfemResetAll();
	
public:
	IKernel*kernel = 0;
	std::map<FortrendVTMSignalTower::Output, std::shared_ptr<struct VTMSignalTowerIOBind >> output_bind;
	bool buzzer_state = false;
	bool green_state = false;
	bool blue_state = false;
	bool red_state = false;
	bool yellow_state = false;
private:
	FortrendVTMSignalTower* pParent;
	std::shared_ptr<FortrendAsciiEFEMApi> api;
	std::chrono::system_clock::time_point last_handle_time;
	bool buzzer_enable = true;
	bool error_state = false;
	bool buzzer_last_state = false;
	bool buzzer_first_time = true;
	bool error_recover = false;
	bool green_before_alarm = false;
	bool blue_before_alarm = false;
	bool yellow_before_alarm = false;
	bool init_time = true; //第一次打开软件


};

FortrendVTMSignalTowerPrivate::FortrendVTMSignalTowerPrivate(FortrendVTMSignalTower*p, std::shared_ptr<FortrendAsciiEFEMApi> a)
	:pParent(p), api(a){
	last_handle_time = std::chrono::system_clock::now();
	/*writeSignal(output_bind[FortrendVTMSignalTower::Output::GREEN_LIGHT]->address, false);
	writeSignal(output_bind[FortrendVTMSignalTower::Output::BLUE_LIGHT]->address, false);
	writeSignal(output_bind[FortrendVTMSignalTower::Output::BUZZER]->address, false);
	writeSignal(output_bind[FortrendVTMSignalTower::Output::RED_LIGHT]->address, false);
	writeSignal(output_bind[FortrendVTMSignalTower::Output::YELLOW_LIGHT]->address, true);*/

}

/**
* set buzzer enabled
*/
void FortrendVTMSignalTowerPrivate::setBuzzerEnabled(bool newStat){
	if (buzzer_enable != newStat)
	{
		buzzer_enable = newStat;
		if (buzzer_enable == false)
		{
			writeSignal(output_bind[FortrendVTMSignalTower::Output::BUZZER]->address, false);
		}
	}
	
}

bool FortrendVTMSignalTowerPrivate::writeSignal(std::string address, bool value){
	/*std::shared_ptr<FortrendPumpSubsystem> pump = kernel->getKernelModule<FortrendPumpSubsystem>("PUMP");
	if (!pump)
	{
		return false;
	}
	return pump->writeBit(address, value);*/
	std::string mode = "RED";
	std::string onOff = value?"ON":"OFF";
	if (output_bind[FortrendVTMSignalTower::Output::RED_LIGHT]->address == address){
		mode = "RED";
	}
	else if (output_bind[FortrendVTMSignalTower::Output::GREEN_LIGHT]->address == address){
		mode = "GREEN";
	}
	else if (output_bind[FortrendVTMSignalTower::Output::BLUE_LIGHT]->address == address){
		mode = "BLUE";
	}
	else if (output_bind[FortrendVTMSignalTower::Output::YELLOW_LIGHT]->address == address){
		mode = "YELLOW";
	}
	else if (output_bind[FortrendVTMSignalTower::Output::BUZZER]->address == address){
		mode = "BUZZER";
	}

	std::string str = Poco::format("SET:TOWER/%s/%s", mode, onOff);
	str.push_back(';');
	bool result = api->sendMessage(str.data(), str.size());
	return result;
}

void FortrendVTMSignalTowerPrivate::controlTowerSignal(FortrendVTMSignalTower::Output type, bool value){
	switch (type)
	{
		case FortrendVTMSignalTower::RED_LIGHT:
		{
			if (red_state != value){
				writeSignal(output_bind[FortrendVTMSignalTower::Output::RED_LIGHT]->address, value);
				red_state = value;
			}
		}
		break;
		case FortrendVTMSignalTower::GREEN_LIGHT:
		{
			if (green_state != value){
			   writeSignal(output_bind[FortrendVTMSignalTower::Output::GREEN_LIGHT]->address, value);
			   green_state = value;
			}
		}
		break;
		case FortrendVTMSignalTower::BLUE_LIGHT:
		{
			if (blue_state != value){
				writeSignal(output_bind[FortrendVTMSignalTower::Output::BLUE_LIGHT]->address, value);
				blue_state = value;
			}
		}
		break;
		case FortrendVTMSignalTower::YELLOW_LIGHT:
		{
			if (yellow_state != value){
				writeSignal(output_bind[FortrendVTMSignalTower::Output::YELLOW_LIGHT]->address, value);
				yellow_state = value;
			}
		}
		break;
		case FortrendVTMSignalTower::BUZZER:
		{
			if (buzzer_enable&&buzzer_state != value){
				writeSignal(output_bind[FortrendVTMSignalTower::Output::BUZZER]->address, value);
				buzzer_state = value;
			}
			
		}
		break;
	}
}

bool FortrendVTMSignalTowerPrivate::EfemResetAll()
{
	std::string str = "MOV:INIT/ALL;";
	bool result = api->sendMessage(str.data(), str.size());
	int len = 10;

	//api->onDataRecv(str.c_str(), len);

	return result;
}

/**
* set enabled
*/
bool FortrendVTMSignalTowerPrivate::isBuzzerEnabled(){
	return buzzer_enable;
}

void FortrendVTMSignalTowerPrivate::onProcess(){
	if (init_time)
	{
		init_time = false;
		controlTowerSignal(FortrendVTMSignalTower::Output::RED_LIGHT, false);
		controlTowerSignal(FortrendVTMSignalTower::Output::GREEN_LIGHT, false);
		controlTowerSignal(FortrendVTMSignalTower::Output::YELLOW_LIGHT, false);
		controlTowerSignal(FortrendVTMSignalTower::Output::BLUE_LIGHT, false);
		controlTowerSignal(FortrendVTMSignalTower::Output::BUZZER, false);

		/*std::string str = "GET:TOWER/RED";
		str.push_back(';');
		api->sendMessage(str.data(), str.size());
		Sleep(200);

	    str = "GET:TOWER/YELLOW";
		str.push_back(';');
		api->sendMessage(str.data(), str.size());
		Sleep(200);

		str = "GET:TOWER/GREEN";
		str.push_back(';');
		api->sendMessage(str.data(), str.size());
		Sleep(200);

		str = "GET:TOWER/BLUE";
		str.push_back(';');
		api->sendMessage(str.data(), str.size());
		Sleep(200);

		str = "GET:TOWER/BUZZER";
		str.push_back(';');
		api->sendMessage(str.data(), str.size());
		Sleep(200);*/
		return;
	}
	Sleep(50);
	
	
	bool changed = false;
	//read light
	auto modules = kernel->getKernelModules <IKernelAlarmModule>();
	error_state = false;
	for (auto item : modules){
		//error
		if (item->isAlarm()) error_state = true;
	}
	if (error_state)
	{
		error_recover = true;
		green_before_alarm = output_bind[FortrendVTMSignalTower::Output::GREEN_LIGHT]->curVal;
		blue_before_alarm = output_bind[FortrendVTMSignalTower::Output::BLUE_LIGHT]->curVal;
		yellow_before_alarm = output_bind[FortrendVTMSignalTower::Output::YELLOW_LIGHT]->curVal;

		output_bind[FortrendVTMSignalTower::Output::GREEN_LIGHT]->curVal = false;
		output_bind[FortrendVTMSignalTower::Output::YELLOW_LIGHT]->curVal = false;
		output_bind[FortrendVTMSignalTower::Output::BLUE_LIGHT]->curVal = false;
		output_bind[FortrendVTMSignalTower::Output::BUZZER]->curVal = true;
		//output_bind[FortrendVTMSignalTower::Output::RED_LIGHT]->curVal = true;

		
	}
	else
	{
		if (error_recover)
		{
			output_bind[FortrendVTMSignalTower::Output::GREEN_LIGHT]->curVal = green_before_alarm;
			output_bind[FortrendVTMSignalTower::Output::YELLOW_LIGHT]->curVal = yellow_before_alarm;
			output_bind[FortrendVTMSignalTower::Output::BLUE_LIGHT]->curVal = blue_before_alarm;
		}
		error_recover = false;
		output_bind[FortrendVTMSignalTower::Output::BUZZER]->curVal = false;
		//output_bind[FortrendVTMSignalTower::Output::RED_LIGHT]->curVal = false;
	}
	if (output_bind[FortrendVTMSignalTower::Output::GREEN_LIGHT]->curVal != output_bind[FortrendVTMSignalTower::Output::GREEN_LIGHT]->lastVal)
	{
		output_bind[FortrendVTMSignalTower::Output::GREEN_LIGHT]->lastVal = output_bind[FortrendVTMSignalTower::Output::GREEN_LIGHT]->curVal;
		controlTowerSignal(FortrendVTMSignalTower::Output::GREEN_LIGHT, output_bind[FortrendVTMSignalTower::Output::GREEN_LIGHT]->lastVal);
		changed = true;
	}
	if (output_bind[FortrendVTMSignalTower::Output::YELLOW_LIGHT]->curVal != output_bind[FortrendVTMSignalTower::Output::YELLOW_LIGHT]->lastVal)
	{
		output_bind[FortrendVTMSignalTower::Output::YELLOW_LIGHT]->lastVal = output_bind[FortrendVTMSignalTower::Output::YELLOW_LIGHT]->curVal;
		controlTowerSignal(FortrendVTMSignalTower::Output::YELLOW_LIGHT, output_bind[FortrendVTMSignalTower::Output::YELLOW_LIGHT]->lastVal);
		changed = true;
	}
	if (output_bind[FortrendVTMSignalTower::Output::BLUE_LIGHT]->curVal != output_bind[FortrendVTMSignalTower::Output::BLUE_LIGHT]->lastVal)
	{
		output_bind[FortrendVTMSignalTower::Output::BLUE_LIGHT]->lastVal = output_bind[FortrendVTMSignalTower::Output::BLUE_LIGHT]->curVal;
		controlTowerSignal(FortrendVTMSignalTower::Output::BLUE_LIGHT, output_bind[FortrendVTMSignalTower::Output::BLUE_LIGHT]->lastVal);
		changed = true;
	}
	output_bind[FortrendVTMSignalTower::Output::RED_LIGHT]->curVal = error_state;
	if (output_bind[FortrendVTMSignalTower::Output::RED_LIGHT]->curVal != output_bind[FortrendVTMSignalTower::Output::RED_LIGHT]->lastVal)
	{
		output_bind[FortrendVTMSignalTower::Output::RED_LIGHT]->lastVal = output_bind[FortrendVTMSignalTower::Output::RED_LIGHT]->curVal;
		controlTowerSignal(FortrendVTMSignalTower::Output::RED_LIGHT, error_state);
		changed = true;
	}
	output_bind[FortrendVTMSignalTower::Output::BUZZER]->curVal = error_state;
	if (output_bind[FortrendVTMSignalTower::Output::BUZZER]->curVal != output_bind[FortrendVTMSignalTower::Output::BUZZER]->lastVal)
	{
		output_bind[FortrendVTMSignalTower::Output::BUZZER]->lastVal = output_bind[FortrendVTMSignalTower::Output::BUZZER]->curVal;
		controlTowerSignal(FortrendVTMSignalTower::Output::BUZZER, output_bind[FortrendVTMSignalTower::Output::BUZZER]->lastVal);
		changed = true;
	}
	//if (output_bind[FortrendVTMSignalTower::Output::BUZZER]->curVal)
	//{
	//	if (buzzer_first_time)
	//	{
	//		output_bind[FortrendVTMSignalTower::Output::BUZZER]->lastVal = output_bind[FortrendVTMSignalTower::Output::BUZZER]->curVal;
	//		controlTowerSignal(FortrendVTMSignalTower::Output::BUZZER, true);
	//		last_handle_time = std::chrono::system_clock::now();
	//		buzzer_last_state = true;
	//		buzzer_first_time = false;
	//		changed = true;
	//	}
	//	else
	//	{
	//		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - last_handle_time).count();
	//		if (duration >= 1000)
	//		{
	//			last_handle_time = std::chrono::system_clock::now();
	//			buzzer_last_state = !buzzer_last_state;
	//			output_bind[FortrendVTMSignalTower::Output::BUZZER]->lastVal = buzzer_last_state;
	//			controlTowerSignal(FortrendVTMSignalTower::Output::BUZZER, buzzer_last_state);
	//			changed = true;
	//		}
	//	}
	//	
	//}
	//else
	//{
	//	buzzer_first_time = true;
	//	output_bind[FortrendVTMSignalTower::Output::BUZZER]->lastVal = false;
	//	controlTowerSignal(FortrendVTMSignalTower::Output::BUZZER, false);
	//	last_handle_time = std::chrono::system_clock::now();
	//}
	if (changed){
		pParent->emitAttributeChanged(pParent);
	}
}

/**
* FortrendVTMSignalTower
*/
FortrendVTMSignalTower::FortrendVTMSignalTower(IKernel*kernel,  std::shared_ptr<FortrendAsciiEFEMApi> a)
	:d(new FortrendVTMSignalTowerPrivate(this,a)){
	d->kernel = kernel;
}

void FortrendVTMSignalTower::initialize()throw(KernelException){
	
}

void FortrendVTMSignalTower::unInitialize(){
	d->writeSignal(d->output_bind[FortrendVTMSignalTower::Output::GREEN_LIGHT]->address, false);
	d->writeSignal(d->output_bind[FortrendVTMSignalTower::Output::BLUE_LIGHT]->address, false);
	d->writeSignal(d->output_bind[FortrendVTMSignalTower::Output::BUZZER]->address, false);
	d->writeSignal(d->output_bind[FortrendVTMSignalTower::Output::RED_LIGHT]->address, false);
	d->writeSignal(d->output_bind[FortrendVTMSignalTower::Output::YELLOW_LIGHT]->address, false);
}

void FortrendVTMSignalTower::process(){
	d->onProcess();
}



void FortrendVTMSignalTower::configure(const std::shared_ptr<KernelConfiguration> & config) throw(KernelException){
	if (config->has(TypeToName(FortrendVTMSignalTower::Output::RED_LIGHT)))
	{
		std::string address = config->getString(TypeToName(FortrendVTMSignalTower::Output::RED_LIGHT), "");
		d->output_bind[FortrendVTMSignalTower::Output::RED_LIGHT] = std::shared_ptr<VTMSignalTowerIOBind>(new VTMSignalTowerIOBind{ address, false, false });
	}
	if (config->has(TypeToName(FortrendVTMSignalTower::Output::GREEN_LIGHT)))
	{
		std::string address = config->getString(TypeToName(FortrendVTMSignalTower::Output::GREEN_LIGHT), "");
		d->output_bind[FortrendVTMSignalTower::Output::GREEN_LIGHT] = std::shared_ptr<VTMSignalTowerIOBind>(new VTMSignalTowerIOBind{ address,false, false });
	}
	if (config->has(TypeToName(FortrendVTMSignalTower::Output::YELLOW_LIGHT)))
	{
		std::string address = config->getString(TypeToName(FortrendVTMSignalTower::Output::YELLOW_LIGHT), "");
		d->output_bind[FortrendVTMSignalTower::Output::YELLOW_LIGHT] = std::shared_ptr<VTMSignalTowerIOBind>(new VTMSignalTowerIOBind{ address,false, false });
	}
	if (config->has(TypeToName(FortrendVTMSignalTower::Output::BLUE_LIGHT)))
	{
		std::string address = config->getString(TypeToName(FortrendVTMSignalTower::Output::BLUE_LIGHT), "");
		d->output_bind[FortrendVTMSignalTower::Output::BLUE_LIGHT] = std::shared_ptr<VTMSignalTowerIOBind>(new VTMSignalTowerIOBind{ address, false, false });
	}
	if (config->has(TypeToName(FortrendVTMSignalTower::Output::BUZZER)))
	{
		std::string address = config->getString(TypeToName(FortrendVTMSignalTower::Output::BUZZER), "");
		d->output_bind[FortrendVTMSignalTower::Output::BUZZER] = std::shared_ptr<VTMSignalTowerIOBind>(new VTMSignalTowerIOBind{ address, false, false });
	}
}

bool FortrendVTMSignalTower::getOutput(Output type){
	auto& it = d->output_bind.find(type);
	if (it == d->output_bind.end()){
		return false;
	}
	return it->second->lastVal;
}



void FortrendVTMSignalTower::setOutput(Output type, const bool value){
	auto& it = d->output_bind.find(type);
	if (it == d->output_bind.end()){
		return;
	}
	it->second->curVal = value;;
}


/**
* set enabled
*/
void FortrendVTMSignalTower::setEnabled(bool enabled){
	d->setBuzzerEnabled(enabled);
}

void FortrendVTMSignalTower::setBuzzerState(bool enabled){
	d->buzzer_state = enabled;
}
void FortrendVTMSignalTower::setGreenState(bool enabled){
	d->green_state = enabled;
}
void FortrendVTMSignalTower::setBlueState(bool enabled){
	d->blue_state = enabled;
}
void FortrendVTMSignalTower::setRedState(bool enabled){
	d->red_state = enabled;
}
void FortrendVTMSignalTower::setYellowState(bool enabled){
	d->yellow_state = enabled;
}

bool FortrendVTMSignalTower::EfemResetAll()
{
	return d->EfemResetAll();
}

/**
* set enabled
*/
bool FortrendVTMSignalTower::isEnabled(){
	return	d->isBuzzerEnabled();
}

KERNEL_NS_END
