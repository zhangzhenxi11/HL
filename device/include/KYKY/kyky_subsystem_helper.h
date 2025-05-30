/**
* @file            kyky_subsystem_helper.h
* @brief           Fortrend kyky molecular pump
* @author		   kai
*/

// Library: Fortrend
// Package: Subsystem/KeyencePLC



#ifndef _XLH_FORTREND_KYKY_SUBSYSTEM_HELPER_INCLUDE_
#define _XLH_FORTREND_KYKY_SUBSYSTEM_HELPER_INCLUDE_

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
class KYKYSubSystemHelper
{
public:
	DECLARE_PTR(KYKYSubSystemHelper)
	typedef struct{
		int type;
		int code;
		std::string message;
		bool needReset;
	}DefinedError;
	
	explicit KYKYSubSystemHelper(const std::string& name);
	

public:
	/**
	*@brief  send command to kyky
	*/
	bool readData(int addr, uint16_t* data) throw(KernelException);

	/**
	*@brief  recv string command from kyky 
	*/
	bool writeData(int addr, uint16_t data) throw(KernelException);

protected:
	/**
	*@brief  config protocol
	*/
	void configKYKY(const std::shared_ptr<KernelConfiguration> & config);

	/**
	*@brief  enable protocol
	*/
	bool enableProtocol();


	/**
	*@brief  disable protocol
	*/
	void disableProtocol();
	

private:
	DECLARE_PRIVATE(KYKYSubSystemHelper);
};



KERNEL_NS_END

#endif
