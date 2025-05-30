/**
* @file            kyky_command_executer.h
* @brief           kyky command executer for fortrend
* @author		   kai
*/

// Library: Fortrend
// Package: Subsystem/KYKY



#ifndef _XLH_FORTREND_KYKY_COMMAND_EXECUTER_INCLUDE_
#define _XLH_FORTREND_KYKY_COMMAND_EXECUTER_INCLUDE_
#include "kernel/Fortrend/fortrend_macros.h"
#include "Kernel/kernel_macros.h"
#include "Kernel/kernel_exception.h"
#include "Kernel/kernel_subsystem_command.h"

#include "KYKY/kyky_subsystem_helper.h"

KERNEL_NS_BEGIN


/**
*@brief  kyky command executer
*/
class KYKYCommandExecuter
{
public:
	DECLARE_PTR(KYKYCommandExecuter)
public:
	explicit KYKYCommandExecuter();
	explicit KYKYCommandExecuter(KYKYSubSystemHelper* helper);

	/**
	*@brief   set helper for excuter
	*/
	void  setKYKYHelper(KYKYCommandExecuter* helper);

protected:
	/**
	* @brief return true if success else false.
	*/
	virtual void onStop() throw(KernelException);
	virtual bool readData(int addr, uint16_t* data)throw(KernelException);
	virtual bool writeData(int addr, uint16_t data) throw(KernelException);


private:
	DECLARE_PRIVATE(KYKYCommandExecuter)
};

KERNEL_NS_END

#endif
