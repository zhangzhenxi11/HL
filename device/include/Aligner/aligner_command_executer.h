/**
* @file            aligner_command_executer.h
* @brief           aligner command executer for fortrend
* @author			kai
*/

// Library: Fortrend
// Package: Subsystem/Aligner



#ifndef _XLH_FORTREND_ALIGNER_COMMAND_INCLUDE_
#define _XLH_FORTREND_ALIGNER_COMMAND_INCLUDE_
#include "kernel/Fortrend/fortrend_macros.h"
#include "Kernel/kernel_macros.h"
#include "Kernel/kernel_exception.h"
#include "Kernel/kernel_subsystem_command.h"

#include "Aligner/aligner_subsystem_helper.h"

KERNEL_NS_BEGIN

/**
*@brief  aligner command executer
*/
class AlignerCommandExecuter
{
public:
	DECLARE_PTR(AlignerCommandExecuter)
public:
	explicit AlignerCommandExecuter();
	explicit AlignerCommandExecuter(AlignerSubSystemHelper* hexHelper);

	/**
	*@brief   set helper for excuter
	*/
	void  setRndHelper(AlignerCommandExecuter* hexHelper);

public:
	std::shared_ptr<AlignerSubSystemHelper::DefinedError> getErrorCode(const int code_id);

protected:
	/**
	* @brief return true if success else false.
	*/
	virtual void onStop() throw(KernelException);

protected:
	/**
	*@brief  Send command to Aligner (auto add 0x0D)
	*/
	bool sendRequest(const std::string& command) throw(KernelException);

	/**
	*@brief  recv string command from Aligner (auto remove 0x0D)
	*/
	std::string recvResponse(unsigned int timeout_ms) throw(KernelException);

	/**
	*@brief  recv string command from Aligner (auto remove 0x0D)
	*/
	std::string recvResponseRDY(unsigned int timeout_ms) throw(KernelException);
	/**
	*@brief  get busy state from Aligner
	*/
	bool getBusyState();

private:
	DECLARE_PRIVATE(AlignerCommandExecuter)
};

KERNEL_NS_END

#endif
