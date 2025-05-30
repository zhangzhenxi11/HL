/**
* @file            aligner_command_executer.h
* @brief           aligner command executer for fortrend
* @author			kai
*/

// Library: Fortrend
// Package: Subsystem/aligner

#include "Aligner/aligner_command_executer.h"
#include "Aligner/aligner_subsystem_helper.h"
#include "Poco/Format.h"

#include <thread>
#include <sstream>
#include <chrono>

KERNEL_NS_BEGIN

/**
* AlignerCommandExecuterPrivate
*/
class AlignerCommandExecuterPrivate{
public:
	AlignerSubSystemHelper* helper;
};

/**
* AlignerCommandExecuter
*/
AlignerCommandExecuter::AlignerCommandExecuter(AlignerSubSystemHelper* helper)
	:d(new AlignerCommandExecuterPrivate){
	d->helper = helper;
}


/**
* return true if success else false.
*/
void AlignerCommandExecuter::onStop() throw(KernelException){
	throw KernelSysException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, "can not stop.");
}


/**
* Send command to Aligner (auto add 0x0D)
*/
bool AlignerCommandExecuter::sendRequest(const std::string& command) throw(KernelException){

	return d->helper->sendRequest(command);
}

/**
* recv string command from Aligner (auto add 0x0D)
*/
std::string AlignerCommandExecuter::recvResponse(unsigned int timeout_ms) throw(KernelException){
	return d->helper->recvResponse(timeout_ms);

}

/**
* recv string command from Aligner (auto add 0x0D)
*/
std::string AlignerCommandExecuter::recvResponseRDY(unsigned int timeout_ms) throw(KernelException){
	return d->helper->recvResponseRDY(timeout_ms);

}

bool AlignerCommandExecuter::getBusyState(){
	return d->helper->getBusyState();
}

std::shared_ptr<AlignerSubSystemHelper::DefinedError> AlignerCommandExecuter::getErrorCode(const int code_id){
	return d->helper->getErrorCode(code_id);
}

KERNEL_NS_END
