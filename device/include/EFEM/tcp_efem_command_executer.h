/**
* @file            tcp_efem_command_executer.h
* @brief           tcp_efem command executer for fortrend
* @author			kai
*/

// Library: Fortrend
// Package: Subsystem/TcpEfemRobot



#ifndef _XLH_FORTREND_TCP_EFEM_COMMAND_INCLUDE_
#define _XLH_FORTREND_TCP_EFEM_COMMAND_INCLUDE_
#include "kernel/Fortrend/fortrend_macros.h"
#include "Kernel/kernel_macros.h"
#include "Kernel/kernel_exception.h"
#include "Kernel/kernel_subsystem_command.h"

#include "tcp_efem_subsystem_helper.h"

KERNEL_NS_BEGIN

/**
*@brief  tcp_efem command executer
*/
class TcpEfemCommandExecuter
{
public:
	DECLARE_PTR(TcpEfemCommandExecuter)
public:
	explicit TcpEfemCommandExecuter();
	explicit TcpEfemCommandExecuter(TcpEfemSubSystemHelper* hexHelper);

	/**
	*@brief   set helper for excuter
	*/
	void  setRndHelper(TcpEfemCommandExecuter* hexHelper);

public:
	std::shared_ptr<TcpEfemSubSystemHelper::DefinedError> getErrorCode(const int code_id);

protected:
	/**
	* @brief return true if success else false.
	*/
	virtual void onStop() throw(KernelException);

protected:
	/**
	*@brief  Send command to TcpEfem (auto add 0x0D)
	*/
	bool sendRequest(const std::string& command) throw(KernelException);

	/**
	*@brief  recv string command from TcpEfem (auto remove 0x0D)
	*/
	std::string recvResponse(unsigned int timeout_ms) throw(KernelException);

	std::string recvResponseAxiLocation(unsigned int timeout_ms) throw(KernelException);

	std::string recvResponseRobotMessage(unsigned int timeout_ms) throw(KernelException);

	void setRobotMessage() throw(KernelException);

	/**
	*@brief  recv string command from TcpEfem (auto remove 0x0D)
	*/
	std::string recvResponseRDY(unsigned int timeout_ms) throw(KernelException);
	/**
	*@brief  get busy state from TcpEfem
	*/
	bool getBusyState();

	/*
	*@brief  getMappingFromChar
	*/
	Cassette::Mapping getMappingChar(char str);

private:
	DECLARE_PRIVATE(TcpEfemCommandExecuter)
};

KERNEL_NS_END

#endif
