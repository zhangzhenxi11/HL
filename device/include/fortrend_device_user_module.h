/**
* @file            fortrend_device_user_module.h
* @brief           device user
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Modules

#ifndef _FORTREND_DEVICE_USER_MODULE_INCLUDE_
#define _FORTREND_DEVICE_USER_MODULE_INCLUDE_
#include "Kernel/kernel.h"

namespace FC{
	
class KernelConfiguration;

/**
* DeviceUserModule
*/
class  DeviceUserModule : virtual  public IKernelModule
{
public:
	DeviceUserModule(IKernel* kernel);
	virtual void configure(const std::shared_ptr<KernelConfiguration> & config) throw(KernelException){}
	virtual void initialize() throw(KernelException){}
	virtual void unInitialize(){}
	virtual void process();
	virtual std::string getName() const { return "tower_handler"; }
private:
	IKernel* kernel;
};


}
#endif
