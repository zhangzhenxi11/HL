/**
* @file            fortrend_aligner_subsystem.h
* @brief           Fortrend Aligner 
* @author		   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/Aligner


#ifndef _XLH_FORTREND_ALIGNER_SUBSYSTEM_INCLUDE_
#define _XLH_FORTREND_ALIGNER_SUBSYSTEM_INCLUDE_ 

#include "kernel/Fortrend/abstract_io_subsystem.h"
#include "kernel/Fortrend/fortrend_abstract_station.h"
#include "kernel/Fortrend/fortrend_abstract_aligner.h"

#include "Aligner/fortrend_aligner_reset_command.h"
#include "Aligner/fortrend_aligner_align_command.h"
#include "Aligner/aligner_command_executer.h"
#include "Aligner/aligner_subsystem_helper.h"

namespace FC{


/**
* @brief fortrend loadlock  
*/
	class  FortrendAlignerSubsystem :public AbstractIOSubsystem, public FortrendAbstractAligner, public AlignerSubSystemHelper{
public:
	DECLARE_PTR(FortrendAlignerSubsystem)
	FortrendAlignerSubsystem(IKernel*  kernel, const std::string& name);

	//override for FortrendAbstractStation
	virtual bool hasBoxPlacement()const override { return true; };
	virtual bool hasBoxPresent()const override { return true; };
	virtual bool hasDoorOpend()const override { return true; };
	//override for AbstractIOSubsystem
	virtual int inputCount()const { return 0; };
	virtual int outputCount()const{ return 0; };
	virtual bool getInput(int index)const { return false; };
	virtual std::string getInputName(int index)const { return ""; };
	virtual std::string getOutputName(int index)const { return ""; };

	
	virtual std::shared_ptr<KernelSubsystemResetCommand> createResetCommand()const override;
	virtual std::shared_ptr<KernelSubsystemUpdateCommand> createUpdateCommand() const override;
	virtual std::shared_ptr<AbstractOutPutCommand>  createOutputCommand(int channel, bool stat)const override;


public:
	
	std::shared_ptr<AlignerAlignCommand> createAlignCommand()const;

public:
	void setAlignResult(const bool result, const int length, const int angle);
	void getAlignResult(bool &result,int & length, int &angle)const;

	void setVacuumEnable(const bool value);
	void setWithWaferModeEnable(const bool value);

	void getAlignClear();
protected:
	virtual void onInitialize()throw(KernelException)override;
	virtual void onUnInitialize()override;
	virtual void onProcess()override;
	virtual void onConfigure(const std::shared_ptr<KernelConfiguration> & config)override;
private:
	DECLARE_PRIVATE(FortrendAlignerSubsystem)

};

}

#endif
