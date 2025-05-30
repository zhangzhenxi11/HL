/**
* @file            tcp_efem_command_executer.h
* @brief           sunway command executer for fortrend
* @author			kai
*/

// Library: Fortrend
// Package: Subsystem/TcpEfemRobot

#include "EFEM\tcp_efem_command_executer.h"
#include "EFEM\tcp_efem_subsystem_helper.h"
#include "Poco/Format.h"

#include <thread>
#include <sstream>
#include <chrono>

KERNEL_NS_BEGIN

/**
* SunwayCommandExecuterPrivate
*/
class TcpEfemCommandExecuterPrivate {
public:
	TcpEfemSubSystemHelper* helper;
};

TcpEfemCommandExecuter::TcpEfemCommandExecuter()
{
}

/**
* TcpEfemCommandExecuter
*/
TcpEfemCommandExecuter::TcpEfemCommandExecuter(TcpEfemSubSystemHelper* helper)
	:d(new TcpEfemCommandExecuterPrivate) {
	d->helper = helper;
}


/**
* return true if success else false.
*/
void TcpEfemCommandExecuter::onStop() throw(KernelException) {
	throw KernelSysException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, "can not stop.");
}


/**
* Send command to TcpEfem (auto add 0x0D)
*/
bool TcpEfemCommandExecuter::sendRequest(const std::string& command) throw(KernelException) {
	return d->helper->sendRequest(command);
}

/**
* recv string command from TcpEfem (auto add 0x0D)
*/
std::string TcpEfemCommandExecuter::recvResponse(unsigned int timeout_ms) throw(KernelException) {
	return d->helper->recvResponse(timeout_ms);
}

std::string TcpEfemCommandExecuter::recvResponseAxiLocation(unsigned int timeout_ms) throw(KernelException)
{
	return d->helper->recvResponseAxiLocation(timeout_ms);
}

std::string TcpEfemCommandExecuter::recvResponseRobotMessage(unsigned int timeout_ms) throw(KernelException)
{
	return d->helper->recvResponseRobotMessage(timeout_ms);
}

void TcpEfemCommandExecuter::setRobotMessage() throw(KernelException)
{
	d->helper->setRobotMessage();
}


/**
* recv string command from TcpEfem (auto add 0x0D)
*/
std::string TcpEfemCommandExecuter::recvResponseRDY(unsigned int timeout_ms) throw(KernelException) {
	return d->helper->recvResponseRDY(timeout_ms);

}

bool TcpEfemCommandExecuter::getBusyState() {
	return d->helper->getBusyState();
}

Cassette::Mapping TcpEfemCommandExecuter::getMappingChar(char str)
{
	Cassette::Mapping ret = Cassette::Mapping::Unknown;
	switch (str)
	{
	case 'P':
		ret = Cassette::Mapping::Present;
		break;
	case 'E':
		ret = Cassette::Mapping::Empty;
		break;
	case 'C':
		ret = Cassette::Mapping::Crossed;
		break;
	case 'D':
		ret = Cassette::Mapping::Double;
		break;
	default:
		break;
	}
	return ret;
}

void TcpEfemCommandExecuter::setRndHelper(TcpEfemCommandExecuter* hexHelper)
{
}

std::shared_ptr<TcpEfemSubSystemHelper::DefinedError> TcpEfemCommandExecuter::getErrorCode(const int code_id) {
	return d->helper->getErrorCode(code_id);
}

KERNEL_NS_END
