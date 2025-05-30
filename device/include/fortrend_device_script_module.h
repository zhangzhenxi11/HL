/**
* @file            fortrend_device_script_module.h
* @brief           device script
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Modules

#ifndef _FORTREND_DEVICE_SCRIPT_MODULE_INCLUDE_
#define _FORTREND_DEVICE_SCRIPT_MODULE_INCLUDE_
#include "Kernel/kernel.h"

namespace FC{
	
class KernelConfiguration;
class KernelScriptSubsystem;


/**
* DeviceScriptModule
*/
class  DeviceScriptModule : virtual  public IKernelModule
{
public:
	DeviceScriptModule(IKernel* kernel);
	virtual void configure(const std::shared_ptr<KernelConfiguration> & config) throw(KernelException){}
	virtual void initialize() throw(KernelException);
	virtual void unInitialize();
	virtual void process(){}
	virtual std::string getName() const { return "script_module"; }
private:
	void regCoreFunction(const std::shared_ptr<KernelScriptSubsystem>&sub);
	void regWTRFunction(const std::shared_ptr<KernelScriptSubsystem>&sub);
	void regFTRFunction(const std::shared_ptr<KernelScriptSubsystem>&sub);
	void regLPFunction(const std::shared_ptr<KernelScriptSubsystem>&sub);
	void regRFIDFunction(const std::shared_ptr<KernelScriptSubsystem>&script);
	void regIOFunction(const std::shared_ptr<KernelScriptSubsystem>&sub);
private:
	IKernel* kernel;
};


}
#endif
