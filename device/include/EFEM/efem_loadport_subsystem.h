/**
 * @file            fortrend_loadport_subsystem.h
 * @brief           Fortrend LoadPort
 * @author			xielonghua
 */

// Library: Fortrend
// Package: SubLpImp



#ifndef _XLH_EFEM_LOADPORT_SUBSYSTEM_INCLUDE_
#define _XLH_EFEM_LOADPORT_SUBSYSTEM_INCLUDE_
#include "Kernel/Fortrend/hex_subsystem_helper.h" 
#include "Kernel/Fortrend/loadport_abstract_subsystem.h" 

#include "Kernel/kernel_api.h"
#include "efem_ascii_api.h"
#include "fortrend_ascii_api.h"
#include <mutex>
#include <condition_variable>
KERNEL_NS_BEGIN

#define SIMULATION_DEBUGGING 1

/**
* @brief Fortrend LoadPort 
*/
class  EFEMLPSubsystem : public LoadPortAbstractSubsystem, public HexSubSystemHelper
{
public:
	DECLARE_PTR(EFEMLPSubsystem)
	EFEMLPSubsystem(IKernel* kernel, const std::string& name, std::shared_ptr<EFEMAsciiApi> api);
	std::shared_ptr<EFEMAsciiApi> api;
	//override for station
	virtual bool hasBoxPresent()const override;
	virtual bool hasBoxPlacement()const override;
	virtual bool hasBoxLocked()const override;
	virtual bool hasDoorOpend()const override;
	virtual bool readyToLoadBox(IKernelSubSystem* workSub)const  override;
	virtual bool readyToUnLoadBox(IKernelSubSystem* workSub)const  override;
	virtual void finishedLoadUnLoadBox(IKernelSubSystem* workSub)const override;

	//override for AbstractIOSubsystem	
	virtual int inputCount()const override;
	virtual int outputCount()const override;
	virtual bool getInput(int index)const override;
	virtual std::string getInputName(int index)const override;
	virtual std::string getOutputName(int index)const override;

public:
	void setBoxPresent(bool v);
	void setBoxLocked(bool v) ;
	void setDoorOpend(bool v);
	void setPlaceMent(bool v);
	void setCommandState(EFEMAsciiApi::State newState);
	bool hasFinishedCommandState()const;
	void wait();

public:
	virtual std::shared_ptr<KernelSubsystemResetCommand> createResetCommand() const override;
	virtual std::shared_ptr<KernelSubsystemUpdateCommand> createUpdateCommand() const override;
	virtual std::shared_ptr<LoadPortAbstractOpenBoxCommand> createOpenBoxCommand() override;
	virtual std::shared_ptr<LoadPortAbstractCloseBoxCommand> createCloseBoxCommand() override;
	virtual std::shared_ptr<LoadPortAbstractOpenDoorCommand> createOpenDoorCommand() override;
	virtual std::shared_ptr<LoadPortAbstractCloseDoorCommand> createCloseDoorCommand() override;
	virtual std::shared_ptr<LoadPortAbstractLockBoxCommand> createLockBoxCommand() override;
	virtual std::shared_ptr<LoadPortAbstractUnlockBoxCommand> createUnlockBoxCommand() override;
	virtual std::shared_ptr<LoadPortAbstractGetMapCommand> createGetMapCommand() override;
	virtual std::shared_ptr<AbstractOutPutCommand>  createOutputCommand(int channel, bool stat)const override;

public:
	void handle(const std::shared_ptr<EFEMAsciiApi::Command>& command);
	EFEMAsciiApi::State getCommandState();
	bool onSetCommunicationState(KernelApi::CommunicationState comm);
	uint32_t timeout = 3000;   //default 3s
	std::chrono::system_clock::time_point timestamp;
	std::string primaryMessageName;
	std::string  map_crossed;
	std::string  map_double;
	std::string map_unknown;

protected:
	virtual void onInitialize()throw(KernelException)override;

	virtual void onUnInitialize()override;

	virtual void onProcess()override;

	virtual void onConfigure(const std::shared_ptr<KernelConfiguration> & config)override;

	virtual void onMessage(const HexMessage::Ptr& message, bool inputChanged);

	virtual bool onSetAccessMode(AccessMode mode)override;

private:
	EFEMAsciiApi::State state;
	std::mutex mtx;
	std::condition_variable cv;
	DECLARE_PRIVATE(EFEMLPSubsystem)
};



KERNEL_NS_END

#endif
