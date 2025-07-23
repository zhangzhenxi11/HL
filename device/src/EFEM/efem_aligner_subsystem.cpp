// Library: Fortrend
// Package: SubAligerImp
//
// RND aligner SubSystem
//
// author xielonghua
//

#include "fortrend_ascii_api.h"
#include "EFEM/efem_aligner_subsystem.h"
#include "EFEM/efem_aligner_reset_command.h"
#include "EFEM/efem_aligner_status_command.h"
#include "EFEM/efem_aligner_align_command.h"
#include "EFEM/efem_aligner_rotate_command.h"

#include "Kernel/Fortrend/fortrend_cassette_manager.h"
#include "Kernel/kernel.h"
#include "Kernel/Fortrend/hex_robot_reset_command.h"
#include "Kernel/Fortrend/hex_robot_setspeed_command.h"
#include "Kernel/Fortrend/hex_reset_command.h"
#include "Kernel/Fortrend/hex_update_command.h"
#include "Kernel/Fortrend/hex_output_command.h"

#include "Kernel/Fortrend/rnd_output_command.h"
#include "Kernel/kernel_log.h"
#include <chrono>  
#include <mutex>

#include "Poco/Format.h"
#include "Poco/StringTokenizer.h"
#include "Poco/NumberParser.h"
#include "Kernel/kernel_event_paramters.h"

#if _MSC_VER >1600
#pragma execution_character_set("utf-8")
#endif
KERNEL_NS_BEGIN

/**
* EFEMAlignerSubsystem 
*/

class EFEMAlignerSubsystemPrivate {
public:

	KernelApi::CommunicationState comm;
	EFEMAlignerSubsystem* ealigner;
	std::vector<uint8_t> present_inputs;  //all present input indexs
	std::vector<uint8_t> pip_inputs;	 //all pip input indexs

	std::string OcrCode; //wafer ID
	std::vector<std::string> OcrCodeList;

	int manual_led_output = -1; //manual index
	bool init_manual_led = false;

};


EFEMAlignerSubsystem::EFEMAlignerSubsystem(IKernel*  kernel,const std::string& name, std::shared_ptr<EFEMAsciiApi> api)
:AlignerAbstractSubsystem(kernel, name)
, HexSubSystemHelper(name)
, api(api)
, d(new EFEMAlignerSubsystemPrivate){
	d->ealigner = this;



}


void EFEMAlignerSubsystem::onInitialize()throw(KernelException){
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

void EFEMAlignerSubsystem::onUnInitialize()throw(KernelException){
	//disableProtocol();
	setState(IKernelSubSystem::State::SUB_UNKNOWN);
}

void EFEMAlignerSubsystem::onProcess(){

	//if (d->comm != api->getCommunicationState())
	//{
	//	d->comm = api->getCommunicationState();
	//	/*EFEMAlignerSubsystem::emitAttributeChanged(this);*/
	//}

	std::string ocr = getWaferID();
	if (ocr != "")
	{
		FortrendAbstractAligner::emitAttributeChanged(this);
	}
	if (getCommandState() == EFEMAsciiApi::State::TRANS_WAIT_REPLY) {
		auto now = std::chrono::system_clock::now();
		if (std::chrono::duration_cast<std::chrono::milliseconds>(now - timestamp).count() > timeout) {
			//timeout
			setCommandState(EFEMAsciiApi::State::TRANS_RESPONSE_TIMEOUT);
			logError(getName().c_str(), "Timeout command: %s for module", primaryMessageName.c_str());
			return;
		}
	}


}


void EFEMAlignerSubsystem::onConfigure(const std::shared_ptr<KernelConfiguration> & conf){
	AlignerAbstractSubsystem::onConfigure(conf);
	KernelAbstractSubSystem::onConfigure(conf);//add
	FortrendAbstractStation::configure(conf);//add
	configHex(conf);

}

void EFEMAlignerSubsystem::onMessage(const HexMessage::Ptr& message, bool inputChanged)
{


}

std::shared_ptr<KernelSubsystemResetCommand> EFEMAlignerSubsystem::createResetCommand()const{
	EFEMAlignerSubsystem* self = const_cast<EFEMAlignerSubsystem*>(this);
	EFEMAlignerResetCommand::Ptr ret(new EFEMAlignerResetCommand(self));
	return ret;
}

std::shared_ptr<AlignerAbstractRotateCommand> EFEMAlignerSubsystem::createRotateCommand(float angle)const{
	EFEMAlignerSubsystem* self = const_cast<EFEMAlignerSubsystem*>(this);
	AlignerAbstractRotateCommand::Ptr ret(new EFEMAlignerRotateCommand(self, angle));
	return ret;
}

std::shared_ptr<KernelSubsystemUpdateCommand> EFEMAlignerSubsystem::createUpdateCommand()const{
	EFEMAlignerSubsystem* self = const_cast<EFEMAlignerSubsystem*>(this);
	KernelSubsystemUpdateCommand::Ptr ret(new EFEMAlignerStatusCommand());
	return ret;
}

std::shared_ptr<AlignerAbstractAlignCommand> EFEMAlignerSubsystem::createAlignCommand()const{
	EFEMAlignerSubsystem* self = const_cast<EFEMAlignerSubsystem*>(this);
	AlignerAbstractAlignCommand::Ptr ret(new EFEMAlignerAlignCommand(self));
	return ret;
}

std::shared_ptr<EFEMAlignerOcrCommand> EFEMAlignerSubsystem::createOcrCommand(int dirct) const
{
	EFEMAlignerSubsystem* self = const_cast<EFEMAlignerSubsystem*>(this);
	EFEMAlignerOcrCommand::Ptr ret(new EFEMAlignerOcrCommand(self, dirct));//默认正面
	return ret;
}

std::shared_ptr<AbstractOutPutCommand> EFEMAlignerSubsystem::createOutputCommand(int channel, bool stat)const{
	EFEMAlignerSubsystem* self = const_cast<EFEMAlignerSubsystem*>(this);
	//auto cmd = std::make_shared<HexOutputCommand>(channel, stat, self);
	//cmd->setHasReply(false);
	return nullptr;
}
std::shared_ptr<AlignerAbstractVaccOnCommand> EFEMAlignerSubsystem::createVaccOnCommand() const {
	throw std::exception("not impl");
}
std::shared_ptr<AlignerAbstractVaccOffCommand> EFEMAlignerSubsystem::createVaccOffCommand() const {
	throw std::exception("not impl");
}

void EFEMAlignerSubsystem::GetOCRCommand(int dirct) {

	std::string command = "MOV:TRIGGER/OCR";
	std::string ocr_str = std::to_string(dirct);
	command.append(ocr_str);
	command.append(";");
	bool result = api->sendMessage(command.data(), command.size());
	Sleep(500);
	std::string waferId = api->getData();
	d->OcrCode = waferId;

}

std::string EFEMAlignerSubsystem::getWaferID()
{
	return d->OcrCode;
}

void EFEMAlignerSubsystem::setCommandState(EFEMAsciiApi::State newState)
{
	state = newState;
	//wait command
	if (hasFinishedCommandState()) {
		std::unique_lock<std::mutex> loc(mtx);
		cv.notify_one();  //finished command
	}
}

bool EFEMAlignerSubsystem::hasFinishedCommandState() const
{
	return (state != EFEMAsciiApi::State::TRANS_IDEL && state != EFEMAsciiApi::State::TRANS_WAIT_REPLY);
}

void EFEMAlignerSubsystem::wait()
{
	std::unique_lock<std::mutex> loc(mtx);

	//has finished
	if (hasFinishedCommandState()) {
		return;
	}
	//wait for it
	cv.wait(loc);  //wait
}

// MOV, GET, SET, INF, ABS, EVT, ACK, NAK  
void EFEMAlignerSubsystem::handle(const std::shared_ptr<EFEMAsciiApi::Command>& command)
{
#pragma region INF

	if (command->type == EFEMAsciiApi::Type::INF)
	{
		#pragma region STATE
		if (command->message->base == EFEMAsciiApi::Base::STATE)
		{
			if (command->message->paramers.size() == 3)
			{
				std::string  status = command->message->paramers.at(1); //Data1    NORMAL
				std::string  busystatus = command->message->paramers.at(2);//Data2  BUSY or IDLE

				FortrendCassetteManager::Ptr cassManager = IKernelSubSystem::getKernel()->getKernelModule<FortrendCassetteManager>();
				auto cass1 = cassManager->getCassette(this);
				if (status == "UNKNOWN") setState(State::SUB_UNKNOWN);
				if (status == "IDLE") setState(State::SUB_IDEL);
				if (status == "NORMAL") setState(State::SUB_NORMAL);
				if (status == "ERROR") setState(State::SUB_ERROR);
			}
		}
		//ALIGN
		if (command->message->base == EFEMAsciiApi::Base::ALIGN)
		{
			setCommandState(EFEMAsciiApi::State::TRANS_FINISHED);
		}

		#pragma endregion

		#pragma region MAPDT
		if (command->message->base == EFEMAsciiApi::Base::MAPDT)
		{//获得寻边chuck 是否有晶圆
			if (command->message->paramers.size() == 2)
			{
				FortrendCassetteManager::Ptr cassManager = IKernelSubSystem::getKernel()->getKernelModule<FortrendCassetteManager>();
				std::string mapdt = command->message->paramers.at(1);
				char* modifiable_ptr = &mapdt[0];
				char newStr = modifiable_ptr[0]; //  'P'  'E'
				auto cass = cassManager->getCassette(this);
				if (!cass) return;
				if (cass->slotCount() != mapdt.size()) {
					logError(getName().c_str(), "slot count %d,mapdt %d", cass->slotCount(), mapdt.size());
					return;
				}
				Cassette::Mapping map = getMappingFromChar(newStr);
				cass->setMapping(1, map);	
				setCommandState(EFEMAsciiApi::State::TRANS_FINISHED);

			}
		}
		#pragma endregion

		#pragma region TRIGGER
		if (command->message->base == EFEMAsciiApi::Base::TRIGGER)
		{
			if (command->message->paramers.size() == 2)
			{
				FortrendCassetteManager::Ptr cassManager = IKernelSubSystem::getKernel()->getKernelModule<FortrendCassetteManager>();
				auto cass = cassManager->getCassette(this);
				std::string waferInfo = command->message->paramers.at(1);
				d->OcrCode = waferInfo;
				//cass->setWaferId();
				d->OcrCodeList.push_back(d->OcrCode);
			}
		}

		if (command->message->base == EFEMAsciiApi::Base::INIT)
		{
			setCommandState(EFEMAsciiApi::State::TRANS_FINISHED);
		}
		#pragma endregion
	}
#pragma endregion

#pragma region NAK
	else if (command->type == EFEMAsciiApi::Type::NAK)
	{
		logInform(getName().c_str(), "ERROR NAK");
		std::string name = "";
		std::string errortype = "";
		std::string errorcode = "";
		if (command->message->paramers.size() == 3) {
			name = command->message->paramers.at(0);
			errortype = command->message->paramers.at(0);
			errorcode = command->message->paramers.at(0);
			logInform(getName().c_str(), "ERROR Type=%s ERROR code=%s name=%s", errortype, errorcode, name);
			if (name == "EFEM") {

			}
			else {
				auto error_strucct = getDefinedError(std::stoi(errortype), std::stoi(errorcode));
				AlarmMessage::Ptr alarm(new AlarmMessage(error_strucct->type, error_strucct->code, error_strucct->message));
				addAlarm(alarm);
			}
		}
		setCommandState(EFEMAsciiApi::State::TRANS_REQUEST_FAILD);
	}
#pragma endregion

#pragma region EVT
	else if (command->type == EFEMAsciiApi::Type::EVT)
	{

		auto message_list = command->message->paramers;

		for (auto& mess : message_list)
		{
			logInform(getName().c_str(), "mess=%s", mess.c_str());
		}

		if (command->message->base == EFEMAsciiApi::Base::ALARM)
		{
			if (command->message->paramers.size() == 3)
			{
				std::string  ERROR_SUB = command->message->paramers.at(0); //Data1  
				std::string  ERROR_TYPE = command->message->paramers.at(1); //Data1  
				std::string  ERROR_CODE = command->message->paramers.at(2);//Data2

				logError(getName().c_str(), "ERROR_SUB=%s, ERROR_TYPE=%s,ERROR_CODE=%s ",
					ERROR_SUB.c_str(), ERROR_TYPE.c_str(), ERROR_CODE.c_str());
			}
		}

		setCommandState(EFEMAsciiApi::State::TRANS_REQUEST_FAILD);
	}
	
#pragma endregion
	
}

EFEMAsciiApi::State EFEMAlignerSubsystem::getCommandState()
{
	return state;
}

bool EFEMAlignerSubsystem::onSetCommunicationState(KernelApi::CommunicationState comm)
{
	std::string param = comm == KernelApi::CommunicationState::COMMUNICATING ? "REMOTE" : "LOCAL";
	std::string str = Poco::format("SET:CONTROLMODE/%s", param);
	str.push_back(';');
	bool result = api->sendMessage(str.data(), str.size());
	logInform(getName().c_str(), "onSetCommunicationState %d", result);
	return result;
}

KERNEL_NS_END
