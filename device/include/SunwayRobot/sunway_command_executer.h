/**
* @file            sunway_command_executer.h
* @brief           sunway command executer for fortrend
* @author			kai
*/

// Library: Fortrend
// Package: Subsystem/SunwayRobot



#ifndef _XLH_FORTREND_SUNWAY_COMMAND_INCLUDE_
#define _XLH_FORTREND_SUNWAY_COMMAND_INCLUDE_
#include "kernel/Fortrend/fortrend_macros.h"
#include "Kernel/kernel_macros.h"
#include "Kernel/kernel_exception.h"
#include "Kernel/kernel_subsystem_command.h"

#include "SunwayRobot/sunway_subsystem_helper.h"

KERNEL_NS_BEGIN

/**
*@brief  sunway command executer
*/
class SunwayCommandExecuter
{
public:
	DECLARE_PTR(SunwayCommandExecuter)
public:
	explicit SunwayCommandExecuter();
	explicit SunwayCommandExecuter(SunwaySubSystemHelper* hexHelper);

	/**
	*@brief   set helper for excuter
	*/
	void  setRndHelper(SunwayCommandExecuter* hexHelper);

public:
	std::shared_ptr<SunwaySubSystemHelper::DefinedError> getErrorCode(const int code_id);

	std::shared_ptr<SunwaySubSystemHelper::DefinedError> getErrorCode(const int type_id,const int code_id);

	bool handleErrorCode(const std::string &commandStr, const std::string errorStr,int& type,int& code);

	//解析参数
	bool parseResponse(const std::string& response,std::string& prefix,std::string& command,std::vector<std::string>& parameters);

protected:
	/**
	* @brief return true if success else false.
	*/
	virtual void onStop() throw(KernelException);

protected:
	/**
	*@brief  Send command to Sunway (auto add 0x0D)
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
	*@brief  get busy state from Sunway
	*/
	bool getBusyState();

	std::string recvResponseRobotMessage(unsigned int timeout_ms) throw(KernelException);

	std::string recvResponseRobotMessageMatching(unsigned int timeout_ms,
		const std::vector<std::string>& expectedPrefixes,
		const std::string& context = "") throw(KernelException);

	void clearRobotMessage()throw(KernelException);


	std::string sendCommand(const std::string& command, unsigned int timeout_ms)throw(KernelException);

private:
	DECLARE_PRIVATE(SunwayCommandExecuter)
};

KERNEL_NS_END

#endif
