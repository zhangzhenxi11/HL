// Library: Fortrend
// Package: SubLpImp
//
// Fortrend LoadPort
//
// author xielonghua
//

#include "EFEM/efem_loadport_subsystem.h" 


#include "Kernel/Fortrend/robot_abstract_subsystem.h"
#include "Kernel/Fortrend/fortrend_cassette_manager.h"

#include "EFEM/efem_loadport_status_command.h"


#include "Kernel/Fortrend/hex_update_command.h"
#include "Kernel/Fortrend/hex_output_command.h"
#include "EFEM/efem_loadport_reset_command.h"
#include "EFEM/efem_loadport_openbox_command.h"
#include "EFEM/efem_loadport_closebox_command.h"
#include "EFEM/efem_loadport_opendoor_command.h"
#include "EFEM/efem_loadport_closedoor_command.h"
#include "EFEM/efem_loadport_getmap_command.h"
#include "EFEM/efem_loadport_lockbox_command.h"
#include "EFEM/efem_loadport_unlockbox_command.h"



#include "Kernel/kernel.h"
#include "Kernel/kernel_log.h"
#include "Kernel/kernel_vector.h"
#include <chrono>  
#include <mutex>

#include "Poco/Format.h"
#include "Poco/StringTokenizer.h"
#include "Poco/NumberParser.h"
#include "Kernel/kernel_event_paramters.h"
#include "Kernel/Fortrend/fortrend_cassette_manager.h"

#if _MSC_VER >1600
#pragma execution_character_set("utf-8")
#endif
KERNEL_NS_BEGIN

/**
* get Mapping  by ascci
*/



/**
 * EFEMLPSubsystemPrivate
 */
class EFEMLPSubsystemPrivate{
public:
	void setPresent(bool);
	void setPlaceMent(bool);
	void setLocked(bool);
	void setDoorOpened(bool);
private:
	bool box_present = false;
	bool box_placement = false;
	bool box_locked = false;
	bool door_opend = false;
	bool robot_work = false;
	KernelApi::CommunicationState comm;
	std::vector<uint8_t> present_inputs;  //all present input indexs
	std::vector<uint8_t> pip_inputs;	 //all pip input indexs
	int manual_led_output = -1; //manual index
	bool init_manual_led = false;


	EFEMLPSubsystem* pLoadport;
	friend class EFEMLPSubsystem;
};
 
void EFEMLPSubsystemPrivate::setPresent(bool v){
	if (box_present != v){
		box_present = v;
		pLoadport->FortrendAbstractLoadPort::emitAttributeChanged(pLoadport);
		////send event
		//IEventId::Ptr evt = 0;
		//if (v) evt = NEW_EVENT_ID(EVENT_LP_BOX_PRESENT );
		//else  evt = NEW_EVENT_ID(EVENT_LP_BOX_REMOVED);

		//pLoadport->sendEvent(evt, 0);
	}
}

void EFEMLPSubsystemPrivate::setPlaceMent(bool v){
	if (box_placement != v){
		box_placement = v;
		pLoadport->FortrendAbstractLoadPort::emitAttributeChanged(pLoadport);
	}
}

void EFEMLPSubsystemPrivate::setLocked(bool v){
	if (box_locked != v){
		box_locked = v;
		pLoadport->FortrendAbstractLoadPort::emitAttributeChanged(pLoadport);
	}
}


void EFEMLPSubsystemPrivate::setDoorOpened(bool v){
	if (door_opend != v){
		door_opend = v;
		pLoadport->FortrendAbstractLoadPort::emitAttributeChanged(pLoadport);
	}
}




/**
* EFEMLPSubsystem 
*/
EFEMLPSubsystem::EFEMLPSubsystem(IKernel*  kernel, const std::string& name, std::shared_ptr<EFEMAsciiApi> api)
:LoadPortAbstractSubsystem(kernel,name)
, HexSubSystemHelper(name)
,api(api)
, d(new EFEMLPSubsystemPrivate){
	d->pLoadport = this;
	//api->pLoadport = this;
}


/**
* has box present
*/
bool EFEMLPSubsystem::hasBoxPresent()const{
	return d->box_present;
}


/**
* has box placement
*/
bool EFEMLPSubsystem::hasBoxPlacement()const{
	return d->box_placement;
}

/**
* has box locked
*/
bool EFEMLPSubsystem::hasBoxLocked()const{
	return d->box_locked;
}

/**
* has box locked
*/
bool EFEMLPSubsystem::hasDoorOpend()const{
	return d->door_opend;
}

#pragma region 命令等待机制
void EFEMLPSubsystem::setCommandState(EFEMAsciiApi::State newState){
	state = newState;
	//wait command
	if (hasFinishedCommandState()){
		std::unique_lock<std::mutex> loc(mtx);
		cv.notify_one();  //finished command
	}
}

EFEMAsciiApi::State EFEMLPSubsystem::getCommandState(){
	return state;
}

void  EFEMLPSubsystem::wait(){
	std::unique_lock<std::mutex> loc(mtx);

	//has finished
	if (hasFinishedCommandState()){
		return;
	}
	//wait for it
	cv.wait(loc);  //wait
}

bool EFEMLPSubsystem::hasFinishedCommandState()const{
	return (state != EFEMAsciiApi::State::TRANS_IDEL && state != EFEMAsciiApi::State::TRANS_WAIT_REPLY);
}
#pragma endregion

/**
* ready to load box to station
*/
bool EFEMLPSubsystem::readyToLoadBox(IKernelSubSystem* workSub)const  {
	if (d->box_locked){
		return false;
	}
	if (dynamic_cast<RobotAbstractSubsystem*>(workSub)){
		d->robot_work = true;
	}
	return true;
}

/**
* ready to unload box from station
*/
bool EFEMLPSubsystem::readyToUnLoadBox(IKernelSubSystem* workSub)const  {
	if (d->box_locked){
		return false;
	}
	if (dynamic_cast<RobotAbstractSubsystem*>(workSub)){
		d->robot_work = true;
	}
	return true;
}

/**
* ready to unload box from station
*/
void EFEMLPSubsystem::finishedLoadUnLoadBox(IKernelSubSystem* workSub)const {
	d->robot_work = false;
}


void EFEMLPSubsystem::setBoxPresent(bool v){
	d->setPresent(v);
}

void EFEMLPSubsystem::setPlaceMent(bool v){
	d->setPlaceMent(v);
}

void EFEMLPSubsystem::setBoxLocked(bool v){
	d->setLocked(v);
}

void EFEMLPSubsystem::setDoorOpend(bool v){
	d->setDoorOpened(v);
}

bool EFEMLPSubsystem::onSetAccessMode(AccessMode mode){
	if (getState() != EFEMLPSubsystem::State::SUB_NORMAL) return false;  //must be normal state

	if (d->manual_led_output >= 0){
		try{
			auto cmd = createOutputCommand(d->manual_led_output, mode == AccessMode::ACCESS_MODE_AUTO ? false : true);
			cmd->setNeedResetwhenFaild(false);
			cmd->setCanBeStandby(true);
			startCommand(cmd);
		}
		catch (std::exception&e){
			logWarn(IKernelSubSystem::getName().c_str(), "can not change manual led out");
		}
	}
	return true;
}

bool EFEMLPSubsystem::onSetCommunicationState(KernelApi::CommunicationState comm){
	std::string param = comm == KernelApi::CommunicationState::COMMUNICATING ? "REMOTE" : "LOCAL";
	std::string str = Poco::format("SET:CONTROLMODE/%s", param);
	str.push_back(';');
	bool result = api->sendMessage(str.data(), str.size());
	logInform(getName().c_str(), "onSetCommunicationState %d", result);
	return result;
}

void EFEMLPSubsystem::onMessage(const HexMessage::Ptr& message, bool inputChanged) {
	auto cassManager = IKernelSubSystem::getKernel()->getKernelModule<FortrendCassetteManager>();
	auto cass = cassManager->getCassette(this);
 
	//present input detect
	bool present = d->present_inputs.size() > 0;
	for (auto index : d->present_inputs){
		present = present && getHexInput(index);
		if (!present) break;
	}

	//pip input detect
	bool placement = d->pip_inputs.size() > 0;
	for (auto index : d->pip_inputs){
		placement = placement && getHexInput(index);
		if (!placement) break;
	}

	//set box status & cassette status
	d->setPresent(present);
	d->setPlaceMent(placement);
	if (!d->robot_work){ //robot not work 
		if (cass && !d->box_present)cassManager->removeCassette(this);
		else if (!cass && d->box_present){
			cass = Cassette::Ptr(new Cassette(this));
			cassManager->loadCassette(this, cass);
		}
	}

	// map status
	if (message->command() == 0x73 && cass){
		if (message->size() != cass->slotCount()){
			logWarn(IKernelSubSystem::getName().c_str(), "slots count error，return %d, but expect %d", message->size(), cass->slotCount());
		}
		size_t min_size = std::min(message->size(), cass->slotCount());
		//d->map_status.clear();
		std::vector<int> slots;
		std::vector<Cassette::Mapping> mappings;
		for (size_t i = 0; i < min_size; i++){
			slots.push_back(i + 1);
			mappings.push_back(getMappingFromChar(message->data()[i]));
		}
		cass->setMapping(slots, mappings);

		std::string data{(char*)message->data(), min_size};
		KernelStringParameter parameter(data);
		sendEvent(NEW_EVENT_ID(EVENT_RETURN_MAP), &parameter);
	}

	//input changed
	if (inputChanged){
		//AbstractIOSubsystem::emitAttributeChanged(this);
	}
}


int EFEMLPSubsystem::inputCount()const{
	return hexInputCount();
}

int EFEMLPSubsystem::outputCount()const{
	return hexOutputCount();
}

bool EFEMLPSubsystem::getInput(int index)const{
	return getHexInput(index);
}	

std::string EFEMLPSubsystem::getInputName(int index)const{
	return getHexInputName(index);
}

std::string EFEMLPSubsystem::getOutputName(int index)const{
	return getHexOutputName(index);
}

void EFEMLPSubsystem::handle(const std::shared_ptr<EFEMAsciiApi::Command>& command){
#pragma region INF
	if (command->type == EFEMAsciiApi::Type::INF){
		#pragma region STATE
			if (command->message->base == EFEMAsciiApi::Base::STATE){
				if (command->message->paramers.size() == 5){
					std::string  status = command->message->paramers.at(1);
					std::string  busystatus = command->message->paramers.at(2);//BUSY or IDLE
					std::string  boxstatus = command->message->paramers.at(3);//EMPTY or PRESENT
					std::string  foupOpenStatus = command->message->paramers.at(4);//CLOSED or OPENED
					//logError(getName().c_str(), "status=%s busystatus=%s boxstatus=%s foupOpenStatus=%s", status, busystatus, boxstatus, foupOpenStatus);
					FortrendCassetteManager::Ptr cassManager =IKernelSubSystem::getKernel()->getKernelModule<FortrendCassetteManager>();
					auto cass1 = cassManager->getCassette(this);
					if (status == "UNKNOWN") setState(State::SUB_UNKNOWN);
					if (status == "IDLE") setState(State::SUB_IDEL);
					if (status == "NORMAL") setState(State::SUB_NORMAL);
					if (status == "ERROR") setState(State::SUB_ERROR);
					if (foupOpenStatus == "CLOSE"){
						setDoorOpend(false);
					}
					else{
						setDoorOpend(true);
					}
				
					if (boxstatus == "EMPTY"){
						setBoxPresent(false); //NO FOUP
						setPlaceMent(false);
						if (cass1)
						 cassManager->removeCassette(this);
					}
					else{
						setBoxPresent(true);//Present Foup
						setPlaceMent(true);
						if (!cass1){
							Cassette::Ptr cass(new Cassette(this, 25, true));
							cass->setBoxId(Poco::format("%s", getName()));
							if (foupOpenStatus == "CLOSE"){
								cass->setBoxOpened(false);
							}
							else{
								cass->setBoxOpened(true);
							}
							
							for (size_t i = 0; i < 25; i++){
								cass->setMapping(i + 1, Cassette::Unknown);
							}
							cassManager->loadCassette(this, cass);
						}
						 
					}
				}
				else{
					logError(getName().c_str(), "STATE paramers length error %d", command->message->paramers.size());
				}
			}
		#pragma endregion
		#pragma region MAPDT
		else if (command->message->base == EFEMAsciiApi::Base::MAPDT){//去拿之前的lp的mapping结果
				FortrendCassetteManager::Ptr cassManager = IKernelSubSystem::getKernel()->getKernelModule<FortrendCassetteManager>();
				std::string mapdt = command->message->paramers.at(1);
				auto cass = cassManager->getCassette(this);
				if (!cass) return;
				if (cass->slotCount() != mapdt.size()){
					logError(getName().c_str(), "slot count %d,mapdt %d", cass->slotCount(), mapdt.size());
					return;
				}
				std::vector<int> slots;
				std::vector<Cassette::Mapping> mappings;
				map_crossed = "";//交叉
				map_unknown = "";
				map_double = "";//叠片
				for (size_t i = 0; i < mapdt.size(); i++){
					slots.push_back(i + 1);
					Cassette::Mapping map = getMappingFromChar(mapdt[i]);
					mappings.push_back(map);

					if (map == Cassette::Unknown){
						if (map_unknown == ""){
							map_unknown += "以下槽号未扫描到：";
						}
						map_unknown += std::to_string(i + 1);
						map_unknown += " ";
						//unknowncount++;
					}
					if (map == Cassette::Crossed){
						if (map_crossed == ""){
							map_crossed += "以下槽号斜片：";
						}
						map_crossed += std::to_string(i + 1);
						map_crossed += " ";
					}
					if (map == Cassette::Double){
						if (map_double == ""){
							map_double += "以下槽号叠片：";
						}
						map_double += std::to_string(i + 1);
						map_double += " ";
					}
				}
				cass->setMapping(slots, mappings);
				/*if (unknowncount == mapdt.size()){
				map_unknown = "";
				}*/
				if (map_crossed != "" || map_unknown != "" || map_double != ""){
					/*AlarmMessage::Ptr alarm(new AlarmMessage(0, 0, Poco::format("工位: %s 扫描出晶圆异常%s %s %s", getName(), map_double, map_crossed, map_unknown)));
					addAlarm(alarm);*/
					setCommandState(EFEMAsciiApi::State::TRANS_REQUEST_FAILD);
					return;
				}
			}
		  #pragma endregion
		setCommandState(EFEMAsciiApi::State::TRANS_FINISHED);
	}
#pragma endregion
#pragma region NAK
	else if (command->type == EFEMAsciiApi::Type::NAK){
		std::string name = "";
		std::string errortype = "";
		std::string errorcode = "";
		if (command->message->paramers.size() == 3){
			name = command->message->paramers.at(0);
			errortype = command->message->paramers.at(0);
			errorcode = command->message->paramers.at(0);
			logInform(getName().c_str(), "ERROR Type=%s ERROR code=%s name=%s", errortype, errorcode, name);
			if (name == "EFEM"){

			}
			else{
				auto error_strucct = getDefinedError(std::stoi(errortype), std::stoi(errorcode));
				AlarmMessage::Ptr alarm(new AlarmMessage(error_strucct->type, error_strucct->code, error_strucct->message));
				addAlarm(alarm);
			}
		}
		setCommandState(EFEMAsciiApi::State::TRANS_REQUEST_FAILD);
	}
#pragma endregion
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
				map_crossed = "";
				map_unknown = "";
				map_double = "";
				//int unknowncount = 0;
				for (size_t i = 0; i < mapdt.size(); i++){
					slots.push_back(i + 1);
					Cassette::Mapping map = getMappingFromChar(mapdt[i]);
					mappings.push_back(map);

					if (map == Cassette::Unknown){
						if (map_unknown == ""){
							map_unknown += "以下槽号未扫描到：";
						}
						map_unknown += std::to_string(i + 1);
						map_unknown += " ";
						//unknowncount++;
					}
					if (map == Cassette::Crossed){
						if (map_crossed == ""){
							map_crossed += "以下槽号斜片：";
						}
						map_crossed += std::to_string(i + 1);
						map_crossed += " ";
					}
					if (map == Cassette::Double){
						if (map_double == ""){
							map_double += "以下槽号叠片：";
						}
						map_double += std::to_string(i + 1);
						map_double += " ";
					}
					
				}
				cass->setMapping(slots, mappings);
				/*if (unknowncount == mapdt.size()){
					map_unknown = "";
				}*/
				if (map_crossed != "" || map_unknown != "" || map_double != ""){
					AlarmMessage::Ptr alarm(new AlarmMessage(0, 0, Poco::format("工位: %s 扫描出晶圆异常%s %s %s", getName(), map_double, map_crossed, map_unknown)));
					addAlarm(alarm);
					setCommandState(EFEMAsciiApi::State::TRANS_REQUEST_FAILD);
				}
			}
		#pragma endregion
		#pragma region DOORSTAT
					else if (command->message->base == EFEMAsciiApi::Base::DOORSTAT){
						std::string foupOpenStatus = command->message->paramers.at(1);
						if (foupOpenStatus == "OPENED"){
							setDoorOpend(true);
						}
						else{
							setDoorOpend(false);
						}
					}
		#pragma endregion
	}
#pragma endregion

}

void EFEMLPSubsystem::onInitialize()throw(KernelException){
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

void EFEMLPSubsystem::onUnInitialize()throw(KernelException){
	//disableProtocol();
	setState(IKernelSubSystem::State::SUB_UNKNOWN);
}

void EFEMLPSubsystem::onProcess(){
	//pollProtocol();

	//TODO turn on or off led
	//if (!d->init_manual_led && onSetAccessMode(getAccessMode())){
	//	d->init_manual_led = true;
	//}
	if (d->comm != api->getCommunicationState()){
		d->comm = api->getCommunicationState();
		FortrendAbstractLoadPort::emitAttributeChanged(this);
	}


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


void EFEMLPSubsystem::onConfigure(const std::shared_ptr<KernelConfiguration> & config){
	LoadPortAbstractSubsystem::onConfigure(config);
	configHex(config);


	//get present inputs config 
	do{
		Poco::StringTokenizer token(config->getString("IO.Present", ""), ",", Poco::StringTokenizer::TOK_IGNORE_EMPTY  | Poco::StringTokenizer::TOK_TRIM);
		for (int i = 0; i < token.count(); i++){
			d->present_inputs.push_back(Poco::NumberParser::parse(token[i]));
		}
	} while (false);

	///get pip inputs config 
	do{
		Poco::StringTokenizer token(config->getString("IO.Pip", ""), ",", Poco::StringTokenizer::TOK_IGNORE_EMPTY  | Poco::StringTokenizer::TOK_TRIM);
		for (int i = 0; i < token.count(); i++){
			d->pip_inputs.push_back(Poco::NumberParser::parse(token[i]));
		}
	} while (false);

	///get manual led output config 
	do{
		int led_manual = config->getInt("IO.Manual", -1);
		d->manual_led_output = led_manual;
	} while (false);
}


/**
* create Reset command for subsystem
*/
std::shared_ptr<KernelSubsystemResetCommand> EFEMLPSubsystem::createResetCommand() const{
	EFEMLoadPortResetCommand::Ptr cmd(new EFEMLoadPortResetCommand(const_cast<EFEMLPSubsystem*>(this)));
	return cmd;
}

std::shared_ptr<KernelSubsystemUpdateCommand> EFEMLPSubsystem::createUpdateCommand()const{
	EFEMLoadPortStatusCommand::Ptr ret(new EFEMLoadPortStatusCommand());
	return ret;
}


std::shared_ptr<LoadPortAbstractOpenBoxCommand> EFEMLPSubsystem::createOpenBoxCommand(){
	LoadPortAbstractOpenBoxCommand::Ptr ret(new EFEMLoadPortOpenBoxCommand(this));
	return ret;
}


std::shared_ptr<LoadPortAbstractCloseBoxCommand> EFEMLPSubsystem::createCloseBoxCommand(){
	LoadPortAbstractCloseBoxCommand::Ptr ret(new EFEMLoadPortCloseBoxCommand(this));
	return ret;
}

std::shared_ptr<LoadPortAbstractOpenDoorCommand> EFEMLPSubsystem::createOpenDoorCommand(){
	LoadPortAbstractOpenDoorCommand::Ptr ret(new EFEMLoadPortOpenDoorCommand(this));
	return ret;
}


std::shared_ptr<LoadPortAbstractCloseDoorCommand> EFEMLPSubsystem::createCloseDoorCommand(){
	LoadPortAbstractCloseDoorCommand::Ptr ret(new EFEMLoadPortCloseDoorCommand(this));
	return ret;
}



std::shared_ptr<LoadPortAbstractLockBoxCommand> EFEMLPSubsystem::createLockBoxCommand(){
	LoadPortAbstractLockBoxCommand::Ptr ret(new EFEMLoadPortLockBoxCommand(this));
	return ret;
}

std::shared_ptr<LoadPortAbstractUnlockBoxCommand> EFEMLPSubsystem::createUnlockBoxCommand(){
	LoadPortAbstractUnlockBoxCommand::Ptr ret(new EFEMLoadPortUnlockBoxCommand(this));
	return ret;
}


std::shared_ptr<LoadPortAbstractGetMapCommand> EFEMLPSubsystem::createGetMapCommand() {
	LoadPortAbstractGetMapCommand::Ptr ret(new HexLoadPorGetMapCommand(this));
	return ret;
}


std::shared_ptr<AbstractOutPutCommand>  EFEMLPSubsystem::createOutputCommand(int channel, bool stat)const{
	EFEMLPSubsystem* self = const_cast<EFEMLPSubsystem*>(this);
	auto cmd = std::make_shared<HexOutputCommand>(channel, stat, self);
	cmd->setHasReply(false);
	return cmd;
}

KERNEL_NS_END
