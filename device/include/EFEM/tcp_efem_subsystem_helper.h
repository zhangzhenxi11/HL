/**
* @file            tcp_efem_subsystem_helper.h
* @brief           helper for sunway  protocol subsystem
* @author		   kai
*/

// Library: Fortrend
// Package: Subsystem/TcpEfemRobot



#ifndef _XLH_FORTREND_TCP_EFEM_SUBSYSTEM_HELPER_INCLUDE_
#define _XLH_FORTREND_TCP_EFEM_SUBSYSTEM_HELPER_INCLUDE_

#include "kernel/Fortrend/fortrend_macros.h"
#include "Kernel/kernel_macros.h"
#include "Kernel/kernel_exception.h"

#include <vector>
#include <map>
#include <kernel\Fortrend\cassette.h>

KERNEL_NS_BEGIN
class KernelConfiguration;


/**
*@brief  helper for rnd protocol subsystem
*/
class TcpEfemSubSystemHelper
{
public:
	DECLARE_PTR(TcpEfemSubSystemHelper)
	typedef struct {
		int type;
		int code;
		std::string message;
		bool needReset;
	}DefinedError;
	explicit TcpEfemSubSystemHelper(const std::string& name);
public:

	/**
	*@brief  send command to TcpEfem (auto add 0x0D)
	*/
	bool sendRequest(const std::string& command) throw(KernelException);

	/**
	*@brief  recv string command from TcpEfem (auto remove 0x0D)
	*/
	std::string recvResponse(unsigned int timeout_ms) throw(KernelException);

	std::string recvResponseAxiLocation(unsigned int timeout_ms) throw(KernelException);

	std::string recvResponseRobotMessage(unsigned int timeout_ms) throw(KernelException);

	void setRobotMessage()throw(KernelException);

	bool getIsConnected();

	/**
	*@brief  recv string command from TcpEfem (auto remove 0x0D)
	*/
	std::string recvResponseRDY(unsigned int timeout_ms) throw(KernelException);

	/**
	*@brief  get error code
	*/
	std::shared_ptr<DefinedError> getErrorCode(const int code_id);

	bool getBusyState();
	void setBusyState();

protected:
	/**
	*@brief  config protocol
	*/
	void configTcpEfem(const std::shared_ptr<KernelConfiguration>& config);

	/**
	*@brief  enable protocol
	*/
	bool enableProtocol();


	/**
	*@brief  disable protocol
	*/
	void disableProtocol();



private:
	DECLARE_PRIVATE(TcpEfemSubSystemHelper);
};



KERNEL_NS_END

#endif
