/**
* @file            kyky_command_executer.h
* @brief           kyky command executer for fortrend
* @author		   kai
*/

// Library: Fortrend
// Package: Subsystem/KYKY

#include "KYKY/kyky_command_executer.h"
#include "KYKY/kyky_subsystem_helper.h"
#include "Poco/Format.h"

#include <thread>
#include <sstream>
#include <chrono>

KERNEL_NS_BEGIN

/**
* KYKYCommandExecuterPrivate
*/
class KYKYCommandExecuterPrivate{
public:
	KYKYSubSystemHelper* helper;
};

/**
* KYKYCommandExecuter
*/
KYKYCommandExecuter::KYKYCommandExecuter(KYKYSubSystemHelper* helper)
	:d(new KYKYCommandExecuterPrivate){
	d->helper = helper;
}


/**
* return true if success else false.
*/
void KYKYCommandExecuter::onStop() throw(KernelException){
	throw KernelSysException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, "无法停止");
}

bool KYKYCommandExecuter::readData(int addr, uint16_t *data) throw(KernelException){

	return d->helper->readData(addr, data);
}

bool KYKYCommandExecuter::writeData(int addr, uint16_t data) throw(KernelException){

	return d->helper->writeData(addr, data);
}







KERNEL_NS_END
