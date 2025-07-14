/**
 * @file            fortrend_signal_tower.h
 * @brief           Fortrend Signal tower
 * @author			xielonghua
 */

// Library: Fortrend
// Package: Modules


#ifndef _XLH_FORTREND_VTM_SIGNAL_TOWER_INCLUDE_
#define _XLH_FORTREND_VTM_SIGNAL_TOWER_INCLUDE_
#include "Kernel/kernel_object.h"
#include "Kernel/kernel_listener.h"
#include "Kernel/kernel_exception.h"
#include "Kernel/Fortrend/fortrend_signal_tower.h"
#include "fortrend_ascii_api.h"
#include <vector>
#include <map>

KERNEL_NS_BEGIN
class IKernel;

/**
*@brief  Fortrend VTM Signal Tower
*/
class FortrendVTMSignalTower : public IKernelModule, public KernelListenerOwner<FortrendVTMSignalTower>
{
public:
	DECLARE_PTR(FortrendVTMSignalTower)
	FortrendVTMSignalTower(IKernel*kernel,  std::shared_ptr<FortrendAsciiEFEMApi> a);
public:
	typedef enum{ RED_LIGHT, YELLOW_LIGHT, GREEN_LIGHT, BLUE_LIGHT, BUZZER } Output;
	/**
	*@brief  get output status
	*/
	bool getOutput(Output output);

	/**
	*@brief  set output to io control
	*/
	void setOutput(Output output, const bool value);

	/**
	*@brief  set enabled
	*/
	void setEnabled(bool enabled);

	void setBuzzerState(bool enabled);
	void setGreenState(bool enabled);
	void setBlueState(bool enabled);
	void setRedState(bool enabled);
	void setYellowState(bool enabled);
	/**
	*@brief  set Efem ResetAll
	*/
	bool EfemResetAll();
	/**
	*@brief  set enabled
	*/
	bool isEnabled();

	/**
	*@brief  getname
	*/
	virtual std::string getName() const { return "Tower"; }
protected:
	/**
	*@brief  initialize
	*/
	virtual void initialize() throw(KernelException);

	/**
	*@brief  unInitialize
	*/
	virtual void unInitialize();

	/**
	*@brief  process the subsystem 
	*/
	virtual void process();

	/**
	*@brief  configure
	*/
	virtual void configure(const std::shared_ptr<KernelConfiguration> & config) throw(KernelException);
private:
	DECLARE_PRIVATE(FortrendVTMSignalTower);
};



KERNEL_NS_END

#endif
