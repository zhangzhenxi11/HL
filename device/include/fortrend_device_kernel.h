/**
* @file            fortrend_device_kernel.h
* @brief           device kernel
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Main

#ifndef _XLH_FORTREND_DEVICE_KERNEL_INCLUDE_
#define _XLH_FORTREND_DEVICE_KERNEL_INCLUDE_
#include "Kernel/kernel.h"

namespace FC{

#define DECLARE_EVENT_ID(CLS_NAME)  \
	class CLS_NAME: public IEventId {\
	public:\
		CLS_NAME(){}\
		CLS_NAME(const std::string&name):IEventId(name){}\
	};

	DECLARE_EVENT_ID(EVENT_COMMAND_RUNNING);

/**
*  special device kernel for fortrend
*/
class  FortrendDeviceKernel : public  IKernel
{
public:
	DECLARE_PTR(FortrendDeviceKernel)
	FortrendDeviceKernel();
	virtual std::string getName() const { return "vtm"; }
	virtual void initialize() throw(KernelException);
	virtual std::shared_ptr<KernelAction> createAllSubResetAction()const override;
private:
	void updateSubsystem();

};

}
#endif
