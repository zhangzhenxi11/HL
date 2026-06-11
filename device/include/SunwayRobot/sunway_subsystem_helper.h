/**
* @file            sunway_subsystem_helper.h
* @brief           helper for sunway  protocol subsystem
* @author		   kai
*/

// Library: Fortrend
// Package: Subsystem/SunwayRobot



#ifndef _XLH_FORTREND_SUNWAY_SUBSYSTEM_HELPER_INCLUDE_
#define _XLH_FORTREND_SUNWAY_SUBSYSTEM_HELPER_INCLUDE_

#include "kernel/Fortrend/fortrend_macros.h"
#include "Kernel/kernel_macros.h"
#include "Kernel/kernel_exception.h"

#include <vector>
#include <map>
#include <string>

KERNEL_NS_BEGIN
class KernelConfiguration;

/**
*@brief  helper for rnd protocol subsystem
*/
class SunwaySubSystemHelper
{
public:
	DECLARE_PTR(SunwaySubSystemHelper)
	typedef struct{
		int type;
		int code;
		std::string message;
		bool needReset;
	}DefinedError;
	explicit SunwaySubSystemHelper(const std::string& name);
public:

	/**
	*@brief  send command to Sunway (auto add 0x0D)
	*/
	bool sendRequest(const std::string& command) throw(KernelException);

	/**
	*@brief  recv string command from Sunway (auto remove 0x0D)
	*/
	std::string recvResponse(unsigned int timeout_ms) throw(KernelException);

	void recvResponse2(unsigned int timeout_ms) throw(KernelException);

	/**
	*@brief  recv string command from Sunway (auto remove 0x0D)
	*/
	std::string recvResponseRDY(unsigned int timeout_ms) throw(KernelException);

	/**
	*@brief  get error code
	*/

	std::shared_ptr<DefinedError> getErrorCode(const int code_id);

	/*
	*@brief  get error code,Meet two parameters at the same time.
	* 
	*/
	std::string recvResponseRobotMessage(unsigned int timeout_ms) throw(KernelException);

	std::string recvResponseRobotMessageMatching(unsigned int timeout_ms,
		const std::vector<std::string>& expectedPrefixes,
		const std::string& context = "") throw(KernelException);

	void clearRobotMessage()throw(KernelException);

	std::shared_ptr<DefinedError> getErrorCode(const int type_id,const int code_id);

	bool getBusyState();

	bool getIsConnected();

	//新增
	std::string sendCommand(const std::string& command,unsigned int timeout_ms);

protected:
	/**
	*@brief  config protocol
	*/
	void configSunway(const std::shared_ptr<KernelConfiguration> & config);

	/**
	*@brief  enable protocol
	*/
	void enableProtocol();


	/**
	*@brief  disable protocol
	*/
	void disableProtocol();


private:
	DECLARE_PRIVATE(SunwaySubSystemHelper);
};



KERNEL_NS_END

#endif
