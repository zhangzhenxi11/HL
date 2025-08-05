// Library: LibDevicePlugin
// Package: Api/ASCII
//
// piotech api 
//
// author xielonghua
//

#include "fortrend_ascii_api.h"
#include "fortrend_device_kernel.h"

#include "Kernel/kernel_command_runner.h"
//kernel
#include "Kernel/kernel_action_subsystem.h"
#include "Kernel/kernel_action.h"
#include "Kernel/kernel_iocontrol.h"
#include "Kernel/kernel.h" 
#include "Kernel/kernel_subsystem_reset_command.h"
#include "Kernel/kernel_exception.h"

#include "EFEM/efem_wafer_robot_subsystem.h"
#include "EFEM/efem_loadport_subsystem.h" 
#include "EFEM/efem_aligner_subsystem.h"
//utils
#include "Kernel/kernel_log.h"
#include "Poco/StringTokenizer.h"
//fortrend
#include "Kernel/Fortrend/robot_wafer_abstract_subsystem.h"
#include "Kernel/Fortrend/robot_foup_abstract_subsystem.h"
#include "Kernel/Fortrend/loadport_abstract_subsystem.h"
#include "Kernel/Fortrend/rfid_abstract_subsystem.h"
#include "Kernel/Fortrend/aligner_abstract_subsystem.h"
#include "Kernel/Fortrend/e84_subsystem.h"


#include "Kernel/kernel_subsystem_update_command.h"


#include "Kernel/Fortrend/fortrend_cassette_manager.h"
#include "Kernel/Fortrend/fortrend_signal_tower.h"

#include "Kernel/Fortrend/rfid_abstract_command.h"
#include "Kernel/Fortrend/loadport_abstract_command.h"
#include "Kernel/Fortrend/robot_getwafer_abstract_command.h"
#include "Kernel/Fortrend/robot_putwafer_abstract_command.h"
#include "Kernel/Fortrend/aligner_abstract_command.h"
#include "Kernel/Fortrend/abstract_output_command.h"
#include "Kernel/Fortrend/fortrend_wafer_tool_station.h"

#include "Poco/Format.h"
#include "source_version.h"
#include "Kernel/kernel_event_paramters.h"

#include <regex>
#include <list>
#include <mutex>
#include <atomic>
#include <condition_variable>


#undef ERROR

namespace FC{

class FortrendAsciiEFEMApiPrivate{
public:
	FortrendAsciiEFEMApiPrivate(FortrendAsciiEFEMApi*p);
	void setCmdState(FortrendAsciiEFEMApi::CmdState newState);
	bool hasFinishedState()const;
	void wait();

public:
	FortrendAsciiEFEMApi*pParent;
	std::mutex mtx;
	std::condition_variable cv;

};

FortrendAsciiEFEMApiPrivate::FortrendAsciiEFEMApiPrivate(FortrendAsciiEFEMApi*p)
:pParent(p){

}


FortrendAsciiEFEMApi::FortrendAsciiEFEMApi(IKernel*  kernel)
:EFEMAsciiApi(kernel),
d(new FortrendAsciiEFEMApiPrivate(this)){
	handleMap = std::map<Base, std::function<void(const std::shared_ptr<Command>&)>>({
		{ Base::INIT, std::bind(&FortrendAsciiEFEMApi::handle_INIT, this, std::placeholders::_1) },
		{ Base::LOCK, std::bind(&FortrendAsciiEFEMApi::handle_LOCK, this, std::placeholders::_1) },
		{ Base::UNLOCK, std::bind(&FortrendAsciiEFEMApi::handle_UNLOCK, this, std::placeholders::_1) },
		{ Base::OPEN, std::bind(&FortrendAsciiEFEMApi::handle_OPEN, this, std::placeholders::_1) },
		{ Base::CLOSE, std::bind(&FortrendAsciiEFEMApi::handle_CLOSE, this, std::placeholders::_1) },
		{ Base::WAFSH, std::bind(&FortrendAsciiEFEMApi::handle_WAFSH, this, std::placeholders::_1) },
		{ Base::MAPDT, std::bind(&FortrendAsciiEFEMApi::handle_MAPDT, this, std::placeholders::_1) },//EVT
		{ Base::GOTO, std::bind(&FortrendAsciiEFEMApi::handle_GOTO, this, std::placeholders::_1) },
		{ Base::LOAD, std::bind(&FortrendAsciiEFEMApi::handle_LOAD, this, std::placeholders::_1) },
		{ Base::UNLOAD, std::bind(&FortrendAsciiEFEMApi::handle_UNLOAD, this, std::placeholders::_1) },
		{ Base::ALIGN, std::bind(&FortrendAsciiEFEMApi::handle_ALIGN, this, std::placeholders::_1) },
		{ Base::HOLD, std::bind(&FortrendAsciiEFEMApi::handle_HOLD, this, std::placeholders::_1) },
		{ Base::RESTR, std::bind(&FortrendAsciiEFEMApi::handle_RESTR, this, std::placeholders::_1) },
		{ Base::ABORT, std::bind(&FortrendAsciiEFEMApi::handle_ABORT, this, std::placeholders::_1) },
		{ Base::MODE, std::bind(&FortrendAsciiEFEMApi::handle_MODE, this, std::placeholders::_1) },//EVT
		{ Base::SIGOUT, std::bind(&FortrendAsciiEFEMApi::handle_SIGOUT, this, std::placeholders::_1) },
		{ Base::STATE, std::bind(&FortrendAsciiEFEMApi::handle_STATE, this, std::placeholders::_1) },
		{ Base::SIGSTAT, std::bind(&FortrendAsciiEFEMApi::handle_SIGSTAT, this, std::placeholders::_1) },//EVT
		{ Base::TOWER, std::bind(&FortrendAsciiEFEMApi::handle_TOWER, this, std::placeholders::_1) },
		
		{ Base::CSTID, std::bind(&FortrendAsciiEFEMApi::handle_TOWER, this, std::placeholders::_1) },//EVT
		{ Base::SPEED, std::bind(&FortrendAsciiEFEMApi::handle_SPEED, this, std::placeholders::_1) },
		{ Base::CONTROLMODE, std::bind(&FortrendAsciiEFEMApi::handle_CONTROLMODE, this, std::placeholders::_1) },//EVT
		{ Base::HEARTBEAT, std::bind(&FortrendAsciiEFEMApi::handle_TOWER, this, std::placeholders::_1) },
		{ Base::TOOLSLOT, std::bind(&FortrendAsciiEFEMApi::handle_TOWER, this, std::placeholders::_1) },
		{ Base::CLAMP, std::bind(&FortrendAsciiEFEMApi::handle_TOWER, this, std::placeholders::_1) },
		{ Base::TRIGGER, std::bind(&FortrendAsciiEFEMApi::handle_TOWER, this, std::placeholders::_1) },
		{ Base::ALARM, std::bind(&FortrendAsciiEFEMApi::handle_TOWER, this, std::placeholders::_1) },//EVT
		{ Base::POD, std::bind(&FortrendAsciiEFEMApi::handle_TOWER, this, std::placeholders::_1) },//EVT
		{ Base::DOORSTAT, std::bind(&FortrendAsciiEFEMApi::handle_TOWER, this, std::placeholders::_1) },//EVT
		{Base::TRANSF,std::bind(&FortrendAsciiEFEMApi::handle_TRANSF,this,std::placeholders::_1)}

	});
	stateMap = std::map<Base, std::map<std::string, CmdState>>({
		{ Base::INIT,std::map<std::string, CmdState>() },
		{ Base::LOCK,std::map<std::string, CmdState>() },
		{ Base::UNLOCK,std::map<std::string, CmdState>() },
		{ Base::OPEN,std::map<std::string, CmdState>() },
		{ Base::CLOSE,std::map<std::string, CmdState>() },
		{ Base::WAFSH,std::map<std::string, CmdState>() },
		{ Base::MAPDT,std::map<std::string, CmdState>() },
		{ Base::GOTO,std::map<std::string, CmdState>() },
		{ Base::LOAD,std::map<std::string, CmdState>() },
		{ Base::UNLOAD,std::map<std::string, CmdState>() },
		{ Base::ALIGN,std::map<std::string, CmdState>() },
		{ Base::MODE,std::map<std::string, CmdState>() },
		{ Base::SIGOUT,std::map<std::string, CmdState>() },
		{ Base::STATE,std::map<std::string, CmdState>() },
		{ Base::SIGSTAT,std::map<std::string, CmdState>() },
		{ Base::TOWER,std::map<std::string, CmdState>() },

		{ Base::HOLD,std::map<std::string, CmdState>() },
		{ Base::RESTR,std::map<std::string, CmdState>() },
		{ Base::ABORT,std::map<std::string, CmdState>() },
		{Base::TRANSF,std::map<std::string,CmdState>()}
	});
}

void FortrendAsciiEFEMApi::initialize() throw(KernelException){
	logInform(getName().c_str(), "FortrendAsciiEFEMApi initialize");
	EFEMAsciiApi::initialize();
	//registe all subsystem  event
	auto subs = kernel->getKernelModules<KernelAbstractSubSystem>();
	for (auto item : subs){
		item->addEventListener(this);
	}

}

void FortrendAsciiEFEMApi::unInitialize(){
	EFEMAsciiApi::unInitialize();
	//registe all subsystem  event
	auto subs = kernel->getKernelModules<KernelAbstractSubSystem>();
	for (auto item : subs){
		item->removeEventListener(this);
	}

}


//hooks
bool FortrendAsciiEFEMApi::hookEnable(){ 
	auto cassManager = kernel->getKernelModule<FortrendCassetteManager>();
	if (cassManager && cassManager->conflictStations().size() > 0){
		logError(getName().c_str(), "storage conflict,can not enabled communication.");
		return false;
	}
	return true; 
}

void FortrendAsciiEFEMApi::handle_INIT(const std::shared_ptr<Command>& command){
	auto& paramers = command->message->paramers;
	if (paramers.size() == 0){
		logError(getName().c_str(), "handle_INIT not paramers");
		sendNAK(command->message, KernelSysException::ErrCode::KR_COMMON_COMMAND_PARAMER_ERROR);
		return;
	}

	std::string paramer1 = paramers.at(0);

	std::regex lp_reg("^LP(\\d+)");  //LP
	std::smatch result;
	if (std::regex_match(paramer1, result, lp_reg)){
		auto lp = kernel->getKernelModule<EFEMLPSubsystem>("ELP" + result.str(1));
		lp->handle(command);
	}
	else if (paramer1=="WTR"){
		auto wtr = kernel->getKernelModule<EFEMWaferRobotSubsystem>("EWTR");
		wtr->handle(command);
	}
	else if (paramer1 == "ALIGNER") //2025-7-21 add
	{
		auto aligner = kernel->getKernelModule<EFEMAlignerSubsystem>("EALIGNER");
		aligner->handle(command);
	}
}


void FortrendAsciiEFEMApi::handle_LOCK(const std::shared_ptr<Command>& command){
	auto& paramers = command->message->paramers;
	if (paramers.size() == 0){
		sendNAK(command->message, KernelSysException::ErrCode::KR_COMMON_COMMAND_PARAMER_ERROR);
		return;
	}
	std::string paramer1 = paramers.at(0);
	Poco::StringTokenizer tokenizer(paramer1, "|", 2);//排除|ERROR
	if (tokenizer.count() != 0){
		paramer1 = tokenizer[0];
	}
	auto lp = kernel->getKernelModule<EFEMLPSubsystem>("E" + paramer1);
	if (!lp)return;
	lp->handle(command);
}

void FortrendAsciiEFEMApi::handle_UNLOCK(const std::shared_ptr<Command>& command){
	auto& paramers = command->message->paramers;
	if (paramers.size() == 0){
		sendNAK(command->message, KernelSysException::ErrCode::KR_COMMON_COMMAND_PARAMER_ERROR);
		return;
	}
	std::string paramer1 = paramers.at(0);
	Poco::StringTokenizer tokenizer(paramer1, "|", 2);//排除|ERROR
	if (tokenizer.count() != 0){
		paramer1 = tokenizer[0];
	}
	auto lp = kernel->getKernelModule<EFEMLPSubsystem>("E" + paramer1);
	if (!lp)return;
	lp->handle(command);
}

void FortrendAsciiEFEMApi::handle_OPEN(const std::shared_ptr<Command>& command){
	auto& paramers = command->message->paramers;
	if (paramers.size() == 0){
		sendNAK(command->message, KernelSysException::ErrCode::KR_COMMON_COMMAND_PARAMER_ERROR);
		return;
	}
	std::string paramer1 = paramers.at(0);
	Poco::StringTokenizer tokenizer(paramer1, "|", 2);//排除|ERROR
	if (tokenizer.count() != 0){
		paramer1 = tokenizer[0];
	}
	auto lp = kernel->getKernelModule<EFEMLPSubsystem>("E" + paramer1);
	if (!lp)return;
	lp->handle(command);
}

void FortrendAsciiEFEMApi::handle_CLOSE(const std::shared_ptr<Command>& command){
	auto& paramers = command->message->paramers;
	if (paramers.size() == 0){
		sendNAK(command->message, KernelSysException::ErrCode::KR_COMMON_COMMAND_PARAMER_ERROR);
		return;
	}
	std::string paramer1 = paramers.at(0);
	Poco::StringTokenizer tokenizer(paramer1, "|", 2);//排除|ERROR
	if (tokenizer.count() != 0){
		paramer1 = tokenizer[0];
	}
	auto lp = kernel->getKernelModule<EFEMLPSubsystem>("E" + paramer1);
	if (!lp)return;
	lp->handle(command);
}


void FortrendAsciiEFEMApi::handle_WAFSH(const std::shared_ptr<Command>& command){//WAFSH
	
}	


void FortrendAsciiEFEMApi::handle_MAPDT(const std::shared_ptr<Command>& command){
	//logError(getName().c_str(), "handle_MAPDT");
	auto& paramers = command->message->paramers;
	if (paramers.size() == 0){
		logError(getName().c_str(), "handle_MAPDT not paramers");
		sendNAK(command->message, KernelSysException::ErrCode::KR_COMMON_COMMAND_PARAMER_ERROR);
		return;
	}
	std::string paramer1 = paramers.at(0);
	std::regex lp_reg("^LP(\\d+)");  //LP
	std::smatch result;
	if (std::regex_match(paramer1, result, lp_reg)){
		auto lp = kernel->getKernelModule<EFEMLPSubsystem>("ELP" + result.str(1));
		lp->handle(command);
	}
	else if (paramer1 == "WTR"){
		auto wtr = kernel->getKernelModule<EFEMWaferRobotSubsystem>("EWTR");
		wtr->handle(command);
	}
	else if (paramer1 == "ALIGNER") //2025-7-21 add
	{
		auto aligner = kernel->getKernelModule<EFEMAlignerSubsystem>("EALIGNER");
		aligner->handle(command);
	}
}

void FortrendAsciiEFEMApi::handle_GOTO(const std::shared_ptr<Command>& command){
	//logError(getName().c_str(), "handle_GOTO");
	auto& paramers = command->message->paramers;
	if (paramers.size() == 0){
		logError(getName().c_str(), "handle_GOTO not paramers");
		sendNAK(command->message, KernelSysException::ErrCode::KR_COMMON_COMMAND_PARAMER_ERROR);
		return;
	}
	std::string paramer1 = paramers.at(0);
	if (paramer1 == "WTR"){
		auto wtr = kernel->getKernelModule<EFEMWaferRobotSubsystem>("EWTR");
		wtr->handle(command);
	}
}

void FortrendAsciiEFEMApi::handle_load_unload(const std::shared_ptr<Command>& command, bool load){
	
}

void FortrendAsciiEFEMApi::handle_LOAD(const std::shared_ptr<Command>& command){//LOAD
	//logError(getName().c_str(), "handle_LOAD");
	auto& paramers = command->message->paramers;
	if (paramers.size() == 0){
		logError(getName().c_str(), "handle_GOTO not paramers");
		sendNAK(command->message, KernelSysException::ErrCode::KR_COMMON_COMMAND_PARAMER_ERROR);
		return;
	}
		auto wtr = kernel->getKernelModule<EFEMWaferRobotSubsystem>("EWTR");
		wtr->handle(command);
}

void FortrendAsciiEFEMApi::handle_UNLOAD(const std::shared_ptr<Command>& command){//UNLOAD
	//logError(getName().c_str(), "handle_UNLOAD");
	auto& paramers = command->message->paramers;
	if (paramers.size() == 0){
		logError(getName().c_str(), "handle_GOTO not paramers");
		sendNAK(command->message, KernelSysException::ErrCode::KR_COMMON_COMMAND_PARAMER_ERROR);
		return;
	}
		auto wtr = kernel->getKernelModule<EFEMWaferRobotSubsystem>("EWTR");
		wtr->handle(command);
}


void FortrendAsciiEFEMApi::handle_ALIGN(const std::shared_ptr<Command>& command){//ALIGN
	auto& paramers = command->message->paramers;
	
	//if (paramers.size() == 0) {
	//	logError(getName().c_str(), "handle_ALIGN not paramers");
	//	sendNAK(command->message, KernelSysException::ErrCode::KR_COMMON_COMMAND_PARAMER_ERROR);
	//	return;
	//}
	auto aligner = kernel->getKernelModule<EFEMAlignerSubsystem>("EALIGNER");
	aligner->handle(command);
}

void  FortrendAsciiEFEMApi::handle_HOLD(const std::shared_ptr<Command>& command){
	//logError(getName().c_str(), "handle_HOLD");
	auto& paramers = command->message->paramers;
	if (paramers.size() == 0){
		logError(getName().c_str(), "handle_MAPDT not paramers");
		sendNAK(command->message, KernelSysException::ErrCode::KR_COMMON_COMMAND_PARAMER_ERROR);
		return;
	}
	std::string paramer1 = paramers.at(0);
	std::regex lp_reg("^LP(\\d+)");  //LP
	std::smatch result;
	if (std::regex_match(paramer1, result, lp_reg)){
		auto lp = kernel->getKernelModule<EFEMLPSubsystem>("ELP" + result.str(1));
		lp->handle(command);
	}
}

void  FortrendAsciiEFEMApi::handle_RESTR(const std::shared_ptr<Command>& command){
	//logError(getName().c_str(), "handle_RESTR");
}


void FortrendAsciiEFEMApi::handle_ABORT(const std::shared_ptr<Command>& command){
	//logError(getName().c_str(), "handle_ABORT");
}

void FortrendAsciiEFEMApi::handle_TRANSF(const std::shared_ptr<Command>& command)
{
	//2025-7-22 新加TRANSF指令
	auto& paramers = command->message->paramers;
	if (paramers.size() == 0) {
		logError(getName().c_str(), "handle_TRANSF not paramers");
		sendNAK(command->message, KernelSysException::ErrCode::KR_COMMON_COMMAND_PARAMER_ERROR);
		return;
	}
	auto wtr = kernel->getKernelModule<EFEMWaferRobotSubsystem>("EWTR");
	wtr->handle(command);
}

void FortrendAsciiEFEMApi::handle_MODE(const std::shared_ptr<Command>& command){
	//logError(getName().c_str(), "handle_MODE");
}



void FortrendAsciiEFEMApi::handle_SIGOUT(const std::shared_ptr<Command>& command){//SIGOUT
	//logError(getName().c_str(), "handle_SIGOUT");
	
}



void FortrendAsciiEFEMApi::handle_STATE(const std::shared_ptr<Command> &command) {
	auto& paramers = command->message->paramers;
	if (paramers.size() == 0){
		logError(getName().c_str(), "handle_INIT not paramers");
		sendNAK(command->message, KernelSysException::ErrCode::KR_COMMON_COMMAND_PARAMER_ERROR);
		return;
	}

	std::string paramer1 = paramers.at(0);

	Poco::StringTokenizer tokenizer(paramer1, "|", 2);//排除|ERROR
	if (tokenizer.count() != 0){
		paramer1 = tokenizer[0];
	}
	
	std::regex lp_reg("^LP(\\d+)");  //LP
	std::smatch result;
	if (std::regex_match(paramer1, result, lp_reg)){
		auto lp = kernel->getKernelModule<EFEMLPSubsystem>("ELP" + result.str(1));
		lp->handle(command);
	}
	else if (paramer1 == "WTR"){
		auto wtr = kernel->getKernelModule<EFEMWaferRobotSubsystem>("EWTR");
		wtr->handle(command);
	}
	else if (paramer1 == "ALIGNER") //2025-7-21 add
	{
		auto aligner = kernel->getKernelModule<EFEMAlignerSubsystem>("EALIGNER");
		aligner->handle(command);
	}
}

void FortrendAsciiEFEMApi::handle_SIGSTAT(const std::shared_ptr<Command>& command){//SIGSTAT
	//logError(getName().c_str(), "handle_SIGSTAT");
	auto& paramers = command->message->paramers;
	if (paramers.size() == 0){
		logError(getName().c_str(), "handle_SIGSTAT not paramers");
		sendNAK(command->message, KernelSysException::ErrCode::KR_COMMON_COMMAND_PARAMER_ERROR);
		return;
	}

	std::string paramer1 = paramers.at(0);

	Poco::StringTokenizer tokenizer(paramer1, "|", 2);//排除|ERROR
	if (tokenizer.count() != 0){
		paramer1 = tokenizer[0];
	}

	std::regex lp_reg("^LP(\\d+)");  //LP
	std::smatch result;
	if (std::regex_match(paramer1, result, lp_reg)){
		auto lp = kernel->getKernelModule<EFEMLPSubsystem>("ELP" + result.str(1));
		lp->handle(command);
	}
	else if (paramer1 == "WTR"){
		auto wtr = kernel->getKernelModule<EFEMWaferRobotSubsystem>("EWTR");
		wtr->handle(command);
	}
	else if (paramer1 == "ALIGNER") //2025-7-21 add
	{
		auto aligner = kernel->getKernelModule<EFEMAlignerSubsystem>("EALIGNER");
		aligner->handle(command);
	}
}


void FortrendAsciiEFEMApi::handle_TOWER(const std::shared_ptr<Command> &command) {
	//logError(getName().c_str(), "handle_TOWER");
	//auto& paramers = command->message->paramers;
	//if (paramers.size() == 0){
	//	logError(getName().c_str(), "handle_TOWER not paramers");
	//	sendNAK(command->message, KernelSysException::ErrCode::KR_COMMON_COMMAND_PARAMER_ERROR);
	//	return;
	//}

	//std::string paramer1 = paramers.at(0);

	//Poco::StringTokenizer tokenizer(paramer1, "|", 2);//排除|ERROR
	//if (tokenizer.count() != 0){
	//	paramer1 = tokenizer[0];
	//}

	//std::regex lp_reg("^LP(\\d+)");  //LP
	//std::smatch result;
	//if (std::regex_match(paramer1, result, lp_reg)){
	//	auto lp = kernel->getKernelModule<EFEMLPSubsystem>("ELP" + result.str(1));
	//	lp->handle(command);
	//}
	//else if (paramer1 == "WTR"){
	//	auto wtr = kernel->getKernelModule<EFEMWaferRobotSubsystem>("EWTR");
	//	wtr->handle(command);
	//}
}

void FortrendAsciiEFEMApi::handle_CSTID(const std::shared_ptr<Command>& command){//CSTID
	//logError(getName().c_str(), "handle_CSTID");
}

void FortrendAsciiEFEMApi::handle_SPEED(const std::shared_ptr<Command>& command){
	auto& paramers = command->message->paramers;
	if (paramers.size() == 0){
		logError(getName().c_str(), "handle_SPEED not paramers");
		sendNAK(command->message, KernelSysException::ErrCode::KR_COMMON_COMMAND_PARAMER_ERROR);
		return;
	}

	std::string paramer1 = paramers.at(0);
	Poco::StringTokenizer tokenizer(paramer1, "|", 2);//排除|ERROR
	if (tokenizer.count() != 0){
		paramer1 = tokenizer[0];
	}

    if (paramer1 == "WTR"){
		auto wtr = kernel->getKernelModule<EFEMWaferRobotSubsystem>("EWTR");
		wtr->handle(command);
	}
}

void FortrendAsciiEFEMApi::handle_CONTROLMODE(const std::shared_ptr<Command>& command){
	
	auto& paramers = command->message->paramers;
	if (paramers.size() == 0){
		logError(getName().c_str(), "handle_TOWER not paramers");
		sendNAK(command->message, KernelSysException::ErrCode::KR_COMMON_COMMAND_PARAMER_ERROR);
		return;
	}

	std::string paramer1 = paramers.at(0);
	//logError(getName().c_str(), "handle_CONTROLMODE %s", paramer1);

	Poco::StringTokenizer tokenizer(paramer1, "|", 2);//排除|ERROR
	if (tokenizer.count() != 0){
		paramer1 = tokenizer[0];
	}
	if (paramer1 == "LOCAL"){
		setCommunicationState(NOT_COMMUNICATING);
		//setLocalOnLine();
	}
	else{
		setCommunicationState(COMMUNICATING);
		//setRemoteOnLine();
	}
	
}//CONTROLMODE
void FortrendAsciiEFEMApi::handle_HEARTBEAT(const std::shared_ptr<Command>& command){
	//logError(getName().c_str(), "handle_HEARTBEAT");
	auto& paramers = command->message->paramers;
	if (paramers.size() == 0){
		logError(getName().c_str(), "handle_TOWER not paramers");
		sendNAK(command->message, KernelSysException::ErrCode::KR_COMMON_COMMAND_PARAMER_ERROR);
		return;
	}

	std::string paramer1 = paramers.at(0);

	Poco::StringTokenizer tokenizer(paramer1, "|", 2);//排除|ERROR
	if (tokenizer.count() != 0){
		paramer1 = tokenizer[0];
	}

	std::regex lp_reg("^LP(\\d+)");  //LP
	std::smatch result;
	if (std::regex_match(paramer1, result, lp_reg)){
		auto lp = kernel->getKernelModule<EFEMLPSubsystem>("ELP" + result.str(1));
		lp->handle(command);
	}
	else if (paramer1 == "WTR"){
		auto wtr = kernel->getKernelModule<EFEMWaferRobotSubsystem>("EWTR");
		wtr->handle(command);
	}
}//HEARTBEAT
void FortrendAsciiEFEMApi::handle_TOOLSLOT(const std::shared_ptr<Command>& command){
	//logError(getName().c_str(), "handle_TOOLSLOT");
	auto& paramers = command->message->paramers;
	if (paramers.size() == 0){
		logError(getName().c_str(), "handle_TOWER not paramers");
		sendNAK(command->message, KernelSysException::ErrCode::KR_COMMON_COMMAND_PARAMER_ERROR);
		return;
	}

	std::string paramer1 = paramers.at(0);

	Poco::StringTokenizer tokenizer(paramer1, "|", 2);//排除|ERROR
	if (tokenizer.count() != 0){
		paramer1 = tokenizer[0];
	}

	std::regex lp_reg("^LP(\\d+)");  //LP
	std::smatch result;
	if (std::regex_match(paramer1, result, lp_reg)){
		auto lp = kernel->getKernelModule<EFEMLPSubsystem>("ELP" + result.str(1));
		lp->handle(command);
	}
	else if (paramer1 == "WTR"){
		auto wtr = kernel->getKernelModule<EFEMWaferRobotSubsystem>("EWTR");
		wtr->handle(command);
	}
}//TOOLSLOT
void FortrendAsciiEFEMApi::handle_CLAMP(const std::shared_ptr<Command>& command){
	//logError(getName().c_str(), "handle_CLAMP");
	auto& paramers = command->message->paramers;
	if (paramers.size() == 0){
		logError(getName().c_str(), "handle_TOWER not paramers");
		sendNAK(command->message, KernelSysException::ErrCode::KR_COMMON_COMMAND_PARAMER_ERROR);
		return;
	}

	std::string paramer1 = paramers.at(0);

	Poco::StringTokenizer tokenizer(paramer1, "|", 2);//排除|ERROR
	if (tokenizer.count() != 0){
		paramer1 = tokenizer[0];
	}

	std::regex lp_reg("^LP(\\d+)");  //LP
	std::smatch result;
	if (std::regex_match(paramer1, result, lp_reg)){
		auto lp = kernel->getKernelModule<EFEMLPSubsystem>("ELP" + result.str(1));
		lp->handle(command);
	}
	else if (paramer1 == "WTR"){
		auto wtr = kernel->getKernelModule<EFEMWaferRobotSubsystem>("EWTR");
		wtr->handle(command);
	}
}//CLAMP
void FortrendAsciiEFEMApi::handle_TRIGGER(const std::shared_ptr<Command>& command){
	//logError(getName().c_str(), "handle_TRIGGER");
	auto& paramers = command->message->paramers;
	if (paramers.size() == 0){
		logError(getName().c_str(), "handle_TOWER not paramers");
		sendNAK(command->message, KernelSysException::ErrCode::KR_COMMON_COMMAND_PARAMER_ERROR);
		return;
	}

	std::string paramer1 = paramers.at(0);

	Poco::StringTokenizer tokenizer(paramer1, "|", 2);//排除|ERROR
	if (tokenizer.count() != 0){
		paramer1 = tokenizer[0];
	}

	std::regex lp_reg("^LP(\\d+)");  //LP
	std::smatch result;
	if (std::regex_match(paramer1, result, lp_reg)){
		auto lp = kernel->getKernelModule<EFEMLPSubsystem>("ELP" + result.str(1));
		lp->handle(command);
	}
	else if (paramer1 == "WTR"){
		auto wtr = kernel->getKernelModule<EFEMWaferRobotSubsystem>("EWTR");
		wtr->handle(command);
	}
	else if (paramer1 == "ALIGNER") //2025-7-21 add
	{
		auto aligner = kernel->getKernelModule<EFEMAlignerSubsystem>("EALIGNER");
		aligner->handle(command);
	}

}//TRIGGER
void FortrendAsciiEFEMApi::handle_ALARM(const std::shared_ptr<Command>& command){
	//logError(getName().c_str(), "handle_ALARM");
	auto& paramers = command->message->paramers;
	if (paramers.size() == 0){
		logError(getName().c_str(), "handle_TOWER not paramers");
		sendNAK(command->message, KernelSysException::ErrCode::KR_COMMON_COMMAND_PARAMER_ERROR);
		return;
	}

	std::string paramer1 = paramers.at(0);

	Poco::StringTokenizer tokenizer(paramer1, "|", 2);//排除|ERROR
	if (tokenizer.count() != 0){
		paramer1 = tokenizer[0];
	}

	std::regex lp_reg("^LP(\\d+)");  //LP
	std::smatch result;
	if (std::regex_match(paramer1, result, lp_reg)){
		auto lp = kernel->getKernelModule<EFEMLPSubsystem>("ELP" + result.str(1));
		lp->handle(command);
	}
	else if (paramer1 == "WTR"){
		auto wtr = kernel->getKernelModule<EFEMWaferRobotSubsystem>("EWTR");
		wtr->handle(command);
	}
}//ALARM
void FortrendAsciiEFEMApi::handle_POD(const std::shared_ptr<Command>& command){
	//logError(getName().c_str(), "handle_POD");
	auto& paramers = command->message->paramers;
	if (paramers.size() == 0){
		logError(getName().c_str(), "handle_TOWER not paramers");
		sendNAK(command->message, KernelSysException::ErrCode::KR_COMMON_COMMAND_PARAMER_ERROR);
		return;
	}

	std::string paramer1 = paramers.at(0);

	Poco::StringTokenizer tokenizer(paramer1, "|", 2);//排除|ERROR
	if (tokenizer.count() != 0){
		paramer1 = tokenizer[0];
	}

	std::regex lp_reg("^LP(\\d+)");  //LP
	std::smatch result;
	if (std::regex_match(paramer1, result, lp_reg)){
		auto lp = kernel->getKernelModule<EFEMLPSubsystem>("ELP" + result.str(1));
		lp->handle(command);
	}
	else if (paramer1 == "WTR"){
		auto wtr = kernel->getKernelModule<EFEMWaferRobotSubsystem>("EWTR");
		wtr->handle(command);
	}
}//POD
void FortrendAsciiEFEMApi::handle_DOORSTAT(const std::shared_ptr<Command>& command){
	//logError(getName().c_str(), "handle_DOORSTAT");
	auto& paramers = command->message->paramers;
	if (paramers.size() == 0){
		logError(getName().c_str(), "handle_TOWER not paramers");
		sendNAK(command->message, KernelSysException::ErrCode::KR_COMMON_COMMAND_PARAMER_ERROR);
		return;
	}

	std::string paramer1 = paramers.at(0);

	Poco::StringTokenizer tokenizer(paramer1, "|", 2);//排除|ERROR
	if (tokenizer.count() != 0){
		paramer1 = tokenizer[0];
	}

	std::regex lp_reg("^LP(\\d+)");  //LP
	std::smatch result;
	if (std::regex_match(paramer1, result, lp_reg)){
		auto lp = kernel->getKernelModule<EFEMLPSubsystem>("ELP" + result.str(1));
		lp->handle(command);
	}
	else if (paramer1 == "WTR"){
		auto wtr = kernel->getKernelModule<EFEMWaferRobotSubsystem>("EWTR");
		wtr->handle(command);
	}
}//DOORSTAT

static std::string stateToString(IKernelSubSystem::State state){
	switch (state) {
	case IKernelSubSystem::State::SUB_UNKNOWN:{
		return "UNKNOWN";
	}
	case IKernelSubSystem::State::SUB_IDEL:{
		return "IDLE";
	}
	case IKernelSubSystem::State::SUB_NORMAL:{
		return "NORMAL";
	}
	case IKernelSubSystem::State::SUB_ERROR:{
		return "ERROR";
	}
	default:{
		throw KernelSysException(__FILE__, KernelSysException::KR_COMMON_INTER_DRIVER_ERROR, "no subsystem state:" + std::to_string(state));
	}
	}
}

static FortrendSignalTower::Output stringToTowerSignalIndex(const std::string& name){
	if ("RED" == name){
		return FortrendSignalTower::RED_LIGHT;
	}
	else if ("GREEN" == name){
		return FortrendSignalTower::GREEN_LIGHT;
	}
	else if ("BLUE" == name){
		return FortrendSignalTower::BLUE_LIGHT;
	}
	else if ("YELLOW" == name){
		return FortrendSignalTower::YELLOW_LIGHT;
	}
	else if ("BLUE" == name){
		return FortrendSignalTower::BLUE_LIGHT;
	}
	else if ("BUZZER" == name){
		return FortrendSignalTower::BUZZER;
	}
	else{
		throw KernelSysException(__FILE__, KernelSysException::KR_COMMON_INTER_DRIVER_ERROR, "no signal index for " + name);
	}
}

static FortrendSignalTower::Action stringToTowerSignalAction(const std::string& name){
	if ("ON" == name){
		return FortrendSignalTower::OUT_ON;
	}
	else if ("OFF" == name){
		return FortrendSignalTower::OUT_OFF;
	}
	else if ("FLASH_SLOW" == name){
		return FortrendSignalTower::OUT_FLASH_SLOW;
	}
	else if ("FLASH_FAST" == name){
		return FortrendSignalTower::OUT_FLASH_FAST;
	}
	else{
		throw KernelSysException(__FILE__, KernelSysException::KR_COMMON_INTER_DRIVER_ERROR, "no signal value for " + name);
	}
}

static std::string towerSignalActionToString(FortrendSignalTower::Action signal){
	switch (signal) {
	case FortrendSignalTower::OUT_ON:{
		return "ON";
	}
	case FortrendSignalTower::OUT_OFF:{
		return "OFF";
	}
	case FortrendSignalTower::OUT_FLASH_SLOW:{
		return "FLASH_SLOW";
	}
	case FortrendSignalTower::OUT_FLASH_FAST:{
		return "FLASH_FAST";
	}
	default:{
		throw KernelSysException(__FILE__, KernelSysException::KR_COMMON_INTER_DRIVER_ERROR, "no signal value " + std::to_string(signal));

	}
	}
}

void  FortrendAsciiEFEMApi::submitTask(
	Base base, const::std::string Name,
	const::std::string& command){

	//std::string str = Poco::format("ACK:%s", d->base2str(state));
	//for (auto& item : message->paramers){
	//	str = str + "/";
	//	str = str + item;
	//}
	//str.push_back(';');

	if (stateMap.find(base) != stateMap.end()) {
		auto& innerMap = stateMap[base];  // 获取对应于 Base 的内部 map
		innerMap[Name] = CmdState::IDEL;//不存在时自动创建健
	}
	else{
		printf("not base \r\n");
		return;
	}

	sendMessage(command.data(), command.size());
	
	
}


void FortrendAsciiEFEMApi::onHandleEvent(KernelEventModule* kernelModule, const std::shared_ptr<IEventId>& evtId, KernelEventParameter* context){
	std::shared_ptr<Message> message(new Message);
	//RFID module
	if (RfidAbstractSubsystem* rfid_module = dynamic_cast<RfidAbstractSubsystem*>(kernelModule)){
		//rfid read event
		if (std::dynamic_pointer_cast<EVENT_RFID_READ>(evtId)){
			std::smatch result;
			if (std::regex_match(rfid_module->getName(), result, std::regex("^RFID(\\d+)")) && result.size() == 2){
				std::string data = rfid_module->getLastReadId(0);
				message->base = CSTID;
				message->paramers.push_back("P" + result.str(1));
				message->paramers.push_back(data.empty() ? "NOREAD" : data);
				sendEVT(message);
			}
		}
		return;
	}

	//Loadport subsystem
	if (LoadPortAbstractSubsystem* module = dynamic_cast<LoadPortAbstractSubsystem*>(kernelModule)){
		//pod present
		if (std::dynamic_pointer_cast<EVENT_LP_BOX_PRESENT>(evtId)){
			message->base = POD;
			message->paramers.push_back(module->getName());
			message->paramers.push_back("PRESENT");
			sendEVT(message);
			return;
		}
		
		if (std::dynamic_pointer_cast<EVENT_LP_BOX_REMOVED>(evtId)){
			message->base = POD;
			message->paramers.push_back(module->getName());
			message->paramers.push_back("EMPTY");
			sendEVT(message);
			return;
		}

		if(std::dynamic_pointer_cast<EVENT_RETURN_MAP>(evtId)){
			auto wafer_map = dynamic_cast<KernelStringParameter*>(context);
			message->base = MAPDT;
			message->paramers.push_back(module->getName());
			message->paramers.push_back(wafer_map->value());
			sendEVT(message);
			return;
		}

	}

}

void FortrendAsciiEFEMApi::onHandleAlarm(KernelEventModule* kernelModule, const std::shared_ptr<IEventId>& evtId, KernelEventParameter* context){
	//alarm create
	if (!std::dynamic_pointer_cast<EVENT_ALARM_CREATE>(evtId)){
		return;
	}

	AlarmMessage* alarm = dynamic_cast<AlarmMessage*>(context);
	if (!alarm)return;
	std::shared_ptr<Message> message(new Message);
	message->base = EFEMAsciiApi::ALARM;
	message->paramers.push_back(kernelModule->getName());
	message->paramers.push_back(Poco::format("%d",(int)alarm->type()));
	message->paramers.push_back(Poco::format("%d", (int)alarm->code()));
	sendEVT(message);
}

/**
* auto call when event reached,
*/
void  FortrendAsciiEFEMApi::onKernelEvent(KernelEventModule* kernelModule, const std::shared_ptr<IEventId>& evtId, KernelEventParameter* context){
	//not communication or offline
	if ((NOT_COMMUNICATING == getCommunicationState()) || !isOnline()){
		return;
	}

	//alarm create
	onHandleAlarm(kernelModule, evtId, context);
	//handle event 
	onHandleEvent(kernelModule, evtId, context);
}

void FortrendAsciiEFEMApi::update(){
	kernel->submitTask([=](){
		auto lp1 = kernel->getKernelModule<EFEMLPSubsystem>("ELP1");
		auto lp2 = kernel->getKernelModule<EFEMLPSubsystem>("ELP2");
		auto wtr = kernel->getKernelModule<EFEMWaferRobotSubsystem>("EWTR");
		auto aligner = kernel->getKernelModule<EFEMAlignerSubsystem>("EALIGNER");

		if (lp1){
			auto cmd = lp1->createUpdateCommand();
			lp1->startCommand(cmd);
			cmd->wait();
			if (!cmd->hasError()){
			}
		}
		if (lp2){
			auto cmd = lp2->createUpdateCommand();
			lp2->startCommand(cmd);
			cmd->wait();
			if (!cmd->hasError()){
			}
		}
		if (wtr){
			auto cmd = wtr->createUpdateCommand();
			wtr->startCommand(cmd);
			cmd->wait();
			if (!cmd->hasError()){
			}
		}
		//2025-7-21 add
		if (aligner)
		{
			auto cmd = aligner->createUpdateCommand();
			aligner->startCommand(cmd);
			cmd->wait();
			if (!cmd->hasError()) {
			}
		}
	
	});

}

void FortrendAsciiEFEMApi::handle(const std::shared_ptr<Command>& command){

	auto& it = handleMap.find(command->message->base);
	if (it == handleMap.end()){
		logInform(getName().c_str(), "handle no base %s", command->message->base);
		sendNAK(command->message, KernelSysException::ErrCode::KR_COMMON_COMMAND_PARAMER_ERROR);
		return;
	}

	try{
		it->second(command);
	}
	catch (...){
		//logInform(getName().c_str(), "handle Unknow error %s", command->message->base);
		sendNAK(command->message, KernelSysException::ErrCode::KR_SYSTEM_UNKNOWN_ERROR);
		return;
	}
}

void FortrendAsciiEFEMApi::onAttributeChange(const KernelAbstractIOControl *io_board) {
	if ((NOT_COMMUNICATING == getCommunicationState()) || !isOnline()){
		return;
	}

	auto message = createMessageFrom(*io_board);

//	sendINF(message);
}
std::shared_ptr<EFEMAsciiApi::Message> FortrendAsciiEFEMApi::createMessageFrom(const IKernelIOControl &io_control) {

	std::vector<char> input;
	for(int i = (int)io_control.inputCount() - 1; i >= 0; i--){
		auto ret = io_control.getInput(i);
		input.push_back(ret?'1':'0');
	}

	std::vector<char> output;
	for(int i = (int)io_control.outputCount() - 1; i >= 0; i--){
		auto ret = io_control.getOutput(i);
		output.push_back(ret?'1':'0');
	}
	std::shared_ptr<Message> message(new Message);
	message->base = SIGSTAT;
	message->paramers.push_back(io_control.getName());
	message->paramers.emplace_back(input.begin(), input.end());
	message->paramers.emplace_back(output.begin(), output.end());

	return message;
}

}

