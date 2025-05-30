/**
* @file            aligner_subsystem_helper.h
* @brief           helper for aligner  protocol subsystem
* @author		   kai
*/

// Library: Fortrend
// Package: Subsystem/Aligner



#ifndef _XLH_FORTREND_ALIGNER_SUBSYSTEM_HELPER_INCLUDE_
#define _XLH_FORTREND_ALIGNER_SUBSYSTEM_HELPER_INCLUDE_

#include "kernel/Fortrend/fortrend_macros.h"
#include "Kernel/kernel_macros.h"
#include "Kernel/kernel_exception.h"

#include <vector>
#include <map>

KERNEL_NS_BEGIN
class KernelConfiguration;

/**
*@brief  helper for rnd protocol subsystem
*/
class AlignerSubSystemHelper
{
public:
	DECLARE_PTR(AlignerSubSystemHelper)
	typedef struct{
		int type;
		int code;
		std::string message;
		bool needReset;
	}DefinedError;
	explicit AlignerSubSystemHelper(const std::string& name);
public:

	/**
	*@brief  send command to Sunway (auto add 0x0D)
	*/
	bool sendRequest(const std::string& command) throw(KernelException);

	/**
	*@brief  recv string command from Sunway (auto remove 0x0D)
	*/
	std::string recvResponse(unsigned int timeout_ms) throw(KernelException);

	/**
	*@brief  recv string command from Sunway (auto remove 0x0D)
	*/
	std::string recvResponseRDY(unsigned int timeout_ms) throw(KernelException);

	/**
	*@brief  get error code
	*/
	std::shared_ptr<DefinedError> getErrorCode(const int code_id);

	bool getBusyState();

protected:
	/**
	*@brief  config protocol
	*/
	void configAligner(const std::shared_ptr<KernelConfiguration> & config);

	/**
	*@brief  enable protocol
	*/
	bool enableProtocol();


	/**
	*@brief  disable protocol
	*/
	void disableProtocol();


private:
	DECLARE_PRIVATE(AlignerSubSystemHelper);
};



KERNEL_NS_END

#endif
