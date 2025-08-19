// Library: Fortrend
// Package: SubRobotImp
//
// Fortrend Wafer Robot 
//
// author xielonghua
//

//#include "fortrend_robot_tool_station.h" 
#include "fortrend_ascii_api.h"
#include "EFEM/efem_wafer_robot_subsystem.h" 
#include "Kernel/Fortrend/fortrend_cassette_manager.h"
#include "Kernel/kernel.h"
#include "Kernel/Fortrend/hex_robot_reset_command.h"
#include "Kernel/Fortrend/hex_robot_setspeed_command.h"
#include "Kernel/Fortrend/hex_reset_command.h"
#include "Kernel/Fortrend/hex_update_command.h"
#include "Kernel/Fortrend/hex_output_command.h"
#include "EFEM/efem_robot_speed_command.h"
#include "EFEM/efem_robot_reset_command.h"
#include "EFEM/efem_robot_getwafer_command.h"
#include "EFEM/efem_robot_putwafer_command.h"
#include "EFEM/efem_robot_status_command.h"
#include "Kernel/kernel_log.h"



#include "Poco/Format.h"
#include "Poco/StringTokenizer.h"
#include "Poco/NumberParser.h"
#include <map>

KERNEL_NS_BEGIN


class EFEMWaferRobotSubsystemPrivate{
public:
	EFEMWaferRobotSubsystemPrivate(EFEMWaferRobotSubsystem*p);
	void setObject(uint8_t arm_id,bool has);
public:
	//std::chrono::system_clock::time_point  status_timestamp;
	std::map<uint8_t, bool> armHasObjectMap;
	EFEMWaferRobotSubsystem* p;
	std::vector<std::shared_ptr<FortrendStation>>  stations;
	std::vector<uint8_t> present_inputs;
	std::vector<uint8_t> toolpresent_inputs;
	std::vector<uint8_t> tooldoor_inputs;
	std::vector<uint8_t> toolbusy_inputs;
};

/**
* EFEMWaferRobotSubsystemPrivate
*/
EFEMWaferRobotSubsystemPrivate::EFEMWaferRobotSubsystemPrivate(EFEMWaferRobotSubsystem*p)
:p(p){
}

void EFEMWaferRobotSubsystemPrivate::setObject(uint8_t arm_id, bool has){
	auto it = armHasObjectMap.find(arm_id);
	if (it == armHasObjectMap.end() || it->second != has){
		armHasObjectMap[arm_id] = has;
		p->emitSubsystemAttributeChanged();
	}
}


/**
* EFEMWaferRobotSubsystem
*/
EFEMWaferRobotSubsystem::EFEMWaferRobotSubsystem(IKernel*  kernel, const std::string& name, std::shared_ptr<EFEMAsciiApi> api)
:WaferRobotAbstractSubsystem(kernel, name)
, HexSubSystemHelper(name)
, api(api)
, d(new EFEMWaferRobotSubsystemPrivate(this)){
	
}


/**
* has object on Fortrend Robot Arm(arm_id)
*/
bool EFEMWaferRobotSubsystem::hasObject(unsigned int arm_id)const{
	auto it = d->armHasObjectMap.find(arm_id);
	if (it == d->armHasObjectMap.end()) return false;
	return  it->second;
}

/**
*@brief  virtual input count for this robot
*/
int EFEMWaferRobotSubsystem::inputCount()const{
	return hexInputCount();
}

/**
*@brief  virtual output count for this robot
*/
int EFEMWaferRobotSubsystem::outputCount()const{
	return hexOutputCount();
}
 


std::string EFEMWaferRobotSubsystem::getInputName(int index)const{
	return getHexInputName(index);
}

std::string EFEMWaferRobotSubsystem::getOutputName(int index)const{
	return getHexOutputName(index);
}



/**
* virtual input state
*/
bool EFEMWaferRobotSubsystem::getInput(int index)const{
	return getHexInput(index);
}
 
void EFEMWaferRobotSubsystem::onMessage(const HexMessage::Ptr& message, bool inputChanged) {
	// status
	if (message->command() == 0x61){
		//get object status 
		if (message->size() >= 14){
			uint8_t byte = message->data()[14];
			d->setObject(0, byte & 0x08);
		}

		//if (d->toolpresent_inputs.size() > 1){
		//	bool present = getHexInput(d->toolpresent_inputs[1]);

		//	auto tool = getKernel()->getKernelModule<FortrendRobotToolStation>();
		//	if (tool)tool->setPresent(present);//tool present

		//	printf("61 tool present=%d  index=%d input=%d \r\n", present, d->toolpresent_inputs[1], getHexInput(d->toolpresent_inputs[1]));
		//}

		//if (d->tooldoor_inputs.size() > 1){
		//	bool dooropen = getHexInput(d->tooldoor_inputs[1]);
		//	auto tool = getKernel()->getKernelModule<FortrendRobotToolStation>();
		//	if (tool)tool->setDoorOpen(dooropen);//dooropen

		//	printf("61 tool dooropen=%d index=%d input=%d \r\n", dooropen, d->tooldoor_inputs[1], getHexInput(d->tooldoor_inputs[1]));
		//}

		//if (d->toolbusy_inputs.size() > 1){
		//	bool busy = getHexInput(d->toolbusy_inputs[1]);

		//	auto tool = getKernel()->getKernelModule<FortrendRobotToolStation>();
		//	if (tool)tool->setSoftBusy(busy);//tool busy

		//	printf("61 tool busy=%d  index=%d input=%d\r\n", busy, d->toolbusy_inputs[1], getHexInput(d->toolbusy_inputs[1]));
		//}

	}
	// macro response
	/*if (message->command() == 0x62 && message->size() >= 2){
		uint8_t byte = message->data()[1];
		d->setObject(0, byte & 0x01);
		d->setObject(1, byte & 0x02);
	}*/

	//tool 
	if (message->command() == 0x70){
		
		//if (d->toolpresent_inputs.size() > 0){
		//	bool present=getHexInput(d->toolpresent_inputs[0]);

		//	auto tool = getKernel()->getKernelModule<FortrendRobotToolStation>();
		//	if (tool)tool->setPresent(present);//tool present

		//	printf("tool present=%d  index=%d input=%d \r\n", present, d->toolpresent_inputs[0], getHexInput(d->toolpresent_inputs[0]));
		//}
		//if (d->tooldoor_inputs.size() > 0){
		//	bool dooropen = getHexInput(d->tooldoor_inputs[0]);
		//	auto tool = getKernel()->getKernelModule<FortrendRobotToolStation>();
		//	if (tool)tool->setDoorOpen(dooropen);//dooropen

		//	printf("tool dooropen=%d index=%d input=%d \r\n", dooropen, d->tooldoor_inputs[0], getHexInput(d->tooldoor_inputs[0]));
		//}

		//if (d->toolbusy_inputs.size() > 0){
		//	bool busy = getHexInput(d->toolbusy_inputs[0]);
		//	
		//	auto tool = getKernel()->getKernelModule<FortrendRobotToolStation>();
		//	if (tool)tool->setSoftBusy(busy);//tool busy

		//	printf("tool busy=%d  index=%d input=%d\r\n", busy, d->toolbusy_inputs[0], getHexInput(d->toolbusy_inputs[0]));
		//}

	}

	//present input detect
	/*if (d->present_inputs.size() > 0){
		bool armpresent = true;
		for (auto index : d->present_inputs){
			armpresent = armpresent && getHexInput(index);
			if (!armpresent) break;
		}
		d->setObject(0, armpresent);
	}*/

	//input changed
	if (inputChanged){
		emitSubsystemAttributeChanged();
	}
	auto cassManager = getKernel()->getKernelModule<FortrendCassetteManager>();
	auto cass = cassManager->getCassette(this);
}


/**
* set arm has object manualy
*/
void EFEMWaferRobotSubsystem::setObject(unsigned int arm_id, bool has){
	d->setObject(arm_id,has);
}

void EFEMWaferRobotSubsystem::handle(const std::shared_ptr<EFEMAsciiApi::Command>& command){

	if (command->type == EFEMAsciiApi::Type::INF){
#pragma region STATE
		if (command->message->base == EFEMAsciiApi::Base::STATE){
			if (command->message->paramers.size() == 3){
				std::string  status = command->message->paramers.at(1); //Data1
				std::string  busystatus = command->message->paramers.at(2);//Data2  BUSY or IDLE
			
				FortrendCassetteManager::Ptr cassManager = IKernelSubSystem::getKernel()->getKernelModule<FortrendCassetteManager>();
				auto cass1 = cassManager->getCassette(this);
				if (status == "UNKNOWN") setState(State::SUB_UNKNOWN);
				if (status == "IDLE") setState(State::SUB_IDEL);
				if (status == "NORMAL") setState(State::SUB_NORMAL);
				if (status == "ERROR") setState(State::SUB_ERROR);
			}
			else{
				logError(getName().c_str(), "STATE paramers length error %d", command->message->paramers.size());
			}
		}
#pragma endregion
#pragma region MAPDT
		else if (command->message->base == EFEMAsciiApi::Base::MAPDT){
			FortrendCassetteManager::Ptr cassManager = IKernelSubSystem::getKernel()->getKernelModule<FortrendCassetteManager>();
			std::string mapdt = command->message->paramers.at(1);
			auto cass = cassManager->getCassette(this);
			if (!cass) return;
			if (cass->slotCount() != mapdt.length()){
				logError(getName().c_str(), "slot count %d,mapdt %d", cass->slotCount(), mapdt.length());
				return;
			}
			std::vector<int> slots;
			std::vector<Cassette::Mapping> mappings;
			for (size_t i = 0; i < mapdt.size(); i++)
			{
				slots.push_back(i + 1);
				auto map = getMappingFromChar(mapdt[i]);
				mappings.push_back(map);

				setObject(i, map==Cassette::Present);
			}
			cass->setMapping(slots, mappings);
		}
#pragma endregion
		setCommandState(EFEMAsciiApi::State::TRANS_FINISHED);
	}
	else if (command->type == EFEMAsciiApi::Type::NAK){
		logInform(getName().c_str(), "ERROR NAK");
		std::string name = "";
		std::string errortype = "";
		std::string errorcode = "";
		if (command->message->paramers.size() == 3){
			 name= command->message->paramers.at(0);
			 errortype = command->message->paramers.at(0);
			 errorcode = command->message->paramers.at(0);
			 logInform(getName().c_str(), "ERROR Type=%s ERROR code=%s name=%s", errortype, errorcode, name);
			 if (name =="EFEM" ){
			 
			 }
			 else{
				 auto error_strucct= getDefinedError(std::stoi(errortype), std::stoi(errorcode));
				 AlarmMessage::Ptr alarm(new AlarmMessage(error_strucct->type, error_strucct->code, error_strucct->message));
				 addAlarm(alarm);
			 }
		}
		setCommandState(EFEMAsciiApi::State::TRANS_REQUEST_FAILD);
	}
#pragma region EVT
	else if (command->type == EFEMAsciiApi::Type::EVT){
#pragma region MAPDT
		if (command->message->base == EFEMAsciiApi::Base::MAPDT){
			FortrendCassetteManager::Ptr cassManager = IKernelSubSystem::getKernel()->getKernelModule<FortrendCassetteManager>();
			std::string mapdt = command->message->paramers.at(1);
			auto cass = cassManager->getCassette(this);
			if (!cass) return;
			std::vector<int> slots;
			if (cass->slotCount() != mapdt.size()){
				logError(getName().c_str(), "slot count %d,mapdt %d", cass->slotCount(), mapdt.size());
				return;
			}
			std::vector<Cassette::Mapping> mappings;
			for (size_t i = 0; i < mapdt.size(); i++){
				slots.push_back(i + 1);
				mappings.push_back(getMappingFromChar(mapdt[i]));
			}
			cass->setMapping(slots, mappings);
		}
#pragma endregion
	}
#pragma endregion
	
}
#pragma region ÃüÁîµÈ´ý»úÖÆ
void EFEMWaferRobotSubsystem::setCommandState(EFEMAsciiApi::State newState){
	state = newState;
	//wait command
	if (hasFinishedCommandState()){
		std::unique_lock<std::mutex> loc(mtx);
		cv.notify_one();  //finished command
	}
}
EFEMAsciiApi::State EFEMWaferRobotSubsystem::getCommandState(){
	return state;
}
void  EFEMWaferRobotSubsystem::wait(){
	std::unique_lock<std::mutex> loc(mtx);

	//has finished
	if (hasFinishedCommandState()){
		return;
	}
	//wait for it
	cv.wait(loc);  //wait
}

bool EFEMWaferRobotSubsystem::hasFinishedCommandState()const{
	return (state != EFEMAsciiApi::State::TRANS_IDEL && state != EFEMAsciiApi::State::TRANS_WAIT_REPLY);
}
#pragma endregion


void EFEMWaferRobotSubsystem::onInitialize()throw(KernelException){
	try {
		if (api->getCommunicationState() == KernelApi::CommunicationState::COMMUNICATING) {
			setState(IKernelSubSystem::State::SUB_IDEL);
		}
		else {
			setState(IKernelSubSystem::State::SUB_UNKNOWN);
		}
	}
	catch (KernelException& e) {
		logError(IKernelSubSystem::getName().c_str(), e.what());;
		//throw e;
	}
}

void EFEMWaferRobotSubsystem::onUnInitialize()throw(KernelException){
	//disableProtocol();
	setState(IKernelSubSystem::State::SUB_UNKNOWN);
}

void EFEMWaferRobotSubsystem::onProcess(){
	//pollProtocol();

	if (getCommandState() == EFEMAsciiApi::State::TRANS_WAIT_REPLY){
		auto now = std::chrono::system_clock::now();
		if (std::chrono::duration_cast<std::chrono::milliseconds>(now - timestamp).count() > timeout){
			//timeout
			setCommandState(EFEMAsciiApi::State::TRANS_RESPONSE_TIMEOUT);
			logError(getName().c_str(), "Timeout command: %s for module", primaryMessageName.c_str());
			return;
		}
	}
}


void EFEMWaferRobotSubsystem::onConfigure(const std::shared_ptr<KernelConfiguration> & config){
	WaferRobotAbstractSubsystem::onConfigure(config);
	configHex(config);

	try{
		//get present inputs config 
		do{
			Poco::StringTokenizer token(config->getString("IO.Present", ""), ",", Poco::StringTokenizer::TOK_IGNORE_EMPTY);
			for (int i = 0; i < token.count(); i++){
				d->present_inputs.push_back(Poco::NumberParser::parse(token[i]));
			}
		} while (false);

		do{
			Poco::StringTokenizer token(config->getString("IO.ToolPresent", ""), ",", Poco::StringTokenizer::TOK_IGNORE_EMPTY);
			for (int i = 0; i < token.count(); i++){
				d->toolpresent_inputs.push_back(Poco::NumberParser::parse(token[i]));
			}
		} while (false);
		do{
			Poco::StringTokenizer token(config->getString("IO.ToolDoor", ""), ",", Poco::StringTokenizer::TOK_IGNORE_EMPTY);
			for (int i = 0; i < token.count(); i++){
				d->tooldoor_inputs.push_back(Poco::NumberParser::parse(token[i]));
			}
		} while (false);
		do{
			Poco::StringTokenizer token(config->getString("IO.ToolBusy", ""), ",", Poco::StringTokenizer::TOK_IGNORE_EMPTY);
			for (int i = 0; i < token.count(); i++){
				d->toolbusy_inputs.push_back(Poco::NumberParser::parse(token[i]));
			}
		} while (false);

	}
	catch (KernelException& e){
		logError(getName().c_str(), "%s onConfigure error %s", getName(), e.what());
	}
	catch (std::exception& e) {
		logError(getName().c_str(), "%s onConfigure error %s", getName(), e.what());
	}
	catch (...) {
		logError(getName().c_str(), "Unknown exception occurred.");
	}
}

std::shared_ptr<KernelSubsystemResetCommand> EFEMWaferRobotSubsystem::createResetCommand()const{
	EFEMWaferRobotSubsystem* self = const_cast<EFEMWaferRobotSubsystem*>(this);
	KernelSubsystemResetCommand::Ptr ret(new EFEMRobotResetCommand(self));
	return ret;
}


std::shared_ptr<KernelSubsystemUpdateCommand> EFEMWaferRobotSubsystem::createUpdateCommand()const{
	EFEMRobotStatusCommand::Ptr ret(new EFEMRobotStatusCommand());
	return ret;
}


std::shared_ptr<RobotAbstractGetWaferCommand> EFEMWaferRobotSubsystem::createGetCommand(const std::shared_ptr<FortrendStation>&  station, int arm, int slot)const{
	EFEMWaferRobotSubsystem* self = const_cast<EFEMWaferRobotSubsystem*>(this);
	RobotAbstractGetWaferCommand::Ptr ret(new EFEMRobotGetWaferCommand(station, arm, slot, self));
	return ret;
}

std::shared_ptr<RobotAbstractPutWaferCommand> EFEMWaferRobotSubsystem::createPutCommand(const std::shared_ptr<FortrendStation>&  station, int arm, int slot)const{
	EFEMWaferRobotSubsystem* self = const_cast<EFEMWaferRobotSubsystem*>(this);
	RobotAbstractPutWaferCommand::Ptr ret(new EFEMRobotPutWaferCommand(station, arm, slot, self));
	return ret;
}

std::shared_ptr<RobotAbstractReadyGetWaferCommand> EFEMWaferRobotSubsystem::createReadyGetCommand(const std::shared_ptr<FortrendStation>&  station, int arm, int slot)const{
	EFEMWaferRobotSubsystem* self = const_cast<EFEMWaferRobotSubsystem*>(this);
	RobotAbstractReadyGetWaferCommand::Ptr ret(new EFEMRobotReadyGetWaferCommand(station, arm, slot, self));
	return ret;

}

std::shared_ptr<RobotAbstractReadyPutWaferCommand> EFEMWaferRobotSubsystem::createReadyPutCommand(const std::shared_ptr<FortrendStation>&  station, int arm, int slot)const{
	EFEMWaferRobotSubsystem* self = const_cast<EFEMWaferRobotSubsystem*>(this);
	RobotAbstractReadyPutWaferCommand::Ptr ret(new EFEMRobotReadyPutWaferCommand(station, arm, slot, self));
	return ret;
}

std::shared_ptr<RobotAbstractSetSpeedCommand> EFEMWaferRobotSubsystem::createSetSpeedCommand(uint8_t percentage) const{
	EFEMWaferRobotSubsystem* self = const_cast<EFEMWaferRobotSubsystem*>(this);
	EFEMRobotSpeedCommand::Ptr ret(new EFEMRobotSpeedCommand(self,percentage));
	return ret;
}

std::shared_ptr<AbstractOutPutCommand>  EFEMWaferRobotSubsystem::createOutputCommand(int channel, bool stat)const{
	EFEMWaferRobotSubsystem* self = const_cast<EFEMWaferRobotSubsystem*>(this);
	AbstractOutPutCommand::Ptr ret(new HexOutputCommand(channel, stat, self));
	return ret;
}

KERNEL_NS_END
