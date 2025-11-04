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
#include "efem_ascii_api.h"
#include "fortrend_ascii_api.h"
#include <mutex>
#include <condition_variable>
#include "EFEM/efem_aligner_ocr_command.h"
#include "EFEM/efem_aligner_getmap_command.h"
#include "EFEM/efem_aligner_status_command.h"
KERNEL_NS_BEGIN

/**
 *@brief   Fortrend aligner 
*/
class  EFEMAlignerSubsystem : public AlignerAbstractSubsystem, public HexSubSystemHelper
{
public:
	DECLARE_PTR(EFEMAlignerSubsystem)
public:
	EFEMAlignerSubsystem(IKernel*  kernel,const std::string& name, std::shared_ptr<EFEMAsciiApi> api);
	std::shared_ptr<EFEMAsciiApi> api;

	virtual bool hasBoxPresent()const override{ return true; }
	virtual bool hasBoxPlacement()const override { return true; }
	virtual bool hasBoxLocked()const override { return true; }

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
	std::shared_ptr<EFEMAlignerOcrCommand> createOcrCommand(int dirct) const;//OCR
	std::shared_ptr<EFEMAlignerGetMapCommand> createGetMapCommand()const;//getmapdt

public:
	void GetOCRCommand(int dirct);

	bool getPresentWafer()const; //有无片在寻边器上
	void setPresentWafer(bool present);

	std::string getWaferID();
	void setCommandState(EFEMAsciiApi::State newState);
	bool hasFinishedCommandState()const;
	void wait();
public:
	void handle(const std::shared_ptr<EFEMAsciiApi::Command>& command);
	EFEMAsciiApi::State getCommandState();
	bool onSetCommunicationState(KernelApi::CommunicationState comm);
	bool isPresentWafer = false;

	uint32_t timeout = 10000; //10s 
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


private:
	EFEMAsciiApi::State state;
	std::mutex mtx;
	std::condition_variable cv;

	DECLARE_PRIVATE(EFEMAlignerSubsystem)
};
KERNEL_NS_END

#endif
