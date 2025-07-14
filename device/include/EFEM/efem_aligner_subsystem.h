/**
 * @file            aligner_subsystem.h
 * @brief           aligner SubSystem
 * @author			xielonghua
 */

// Library: Fortrend
// Package: SubAligerImp



#ifndef _XLH_EFEM_ALIGNER_SUBSYSTEM_INCLUDE_
#define _XLH_EFEM_ALIGNER_SUBSYSTEM_INCLUDE_
#include "Kernel/Fortrend/aligner_abstract_subsystem.h" 
#include "Kernel/Fortrend/hex_subsystem_helper.h" 
#include "kernel/kernel_exception.h"
#include "EFEM\tcp_efem_command_executer.h"
#include "EFEM\tcp_efem_subsystem_helper.h"
#include "efem_ascii_api.h"
#include "fortrend_ascii_api.h"
#include <mutex>
#include <condition_variable>
KERNEL_NS_BEGIN

/**
 *@brief   Fortrend aligner 
*/
class  EFEMAlignerSubsystem : public AlignerAbstractSubsystem, public TcpEfemSubSystemHelper
{
public:
	DECLARE_PTR(EFEMAlignerSubsystem)
public:
	EFEMAlignerSubsystem(IKernel*  kernel,const std::string& name);
	
	virtual bool hasBoxPresent()const override{ return true; }
	virtual int inputCount()const override  { return 0; }
	virtual bool getInput(int index)const override { return false; }
	virtual int outputCount()const override  { return 0; }
public:
	virtual std::shared_ptr<KernelSubsystemResetCommand> createResetCommand()const override;
	virtual std::shared_ptr<AlignerAbstractRotateCommand> createRotateCommand(float angle)const override;
	virtual std::shared_ptr<KernelSubsystemUpdateCommand> createUpdateCommand() const override;
	virtual std::shared_ptr<AlignerAbstractAlignCommand> createAlignCommand() const override;
	virtual std::shared_ptr<AbstractOutPutCommand>  createOutputCommand(int channel, bool stat)const override;
	std::shared_ptr<AlignerAbstractVaccOnCommand> createVaccOnCommand() const override;
	std::shared_ptr<AlignerAbstractVaccOffCommand> createVaccOffCommand() const override;

	//EFEM ÕûÌå¸´Î»

protected:
	virtual void onInitialize()throw(KernelException)override;
	virtual void onUnInitialize()override;
	virtual void onProcess()override;
	virtual void onConfigure(const std::shared_ptr<KernelConfiguration> & config)override;

private:
	std::mutex mtx;
	std::condition_variable cv;

	DECLARE_PRIVATE(EFEMAlignerSubsystem)
};
KERNEL_NS_END

#endif
