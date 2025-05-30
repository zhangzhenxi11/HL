
/**
* @file            fortrend_aligner_subsystem.h
* @brief           Fortrend Aligner
* @author		   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/Aligner

#include <iostream>

#include "Poco/Format.h"
#include "Poco/StringTokenizer.h"
#include "Poco/NumberParser.h"

#include "kernel/kernel.h"
#include "kernel/kernel_listener.h"
#include "Kernel/kernel_log.h"
#include "kernel/kernel_iocontrol.h"
#include "kernel/kernel_configure.h"
#include "Kernel/kernel_configure.h"

#include "Aligner/fortrend_aligner_subsystem.h" 
#include "Aligner/fortrend_aligner_reset_command.h"
#include "Aligner/fortrend_aligner_update_command.h"
#include "Aligner/fortrend_aligner_output_command.h"
#include "Aligner/fortrend_aligner_align_command.h"

namespace FC{
/**
* FortrendAlignerSubsystemPrivate
*/
class FortrendAlignerSubsystemPrivate{
public:
	FortrendAlignerSubsystemPrivate(FortrendAlignerSubsystem*p);
	void setInput(uint8_t index, bool stat);
public:
	FortrendAlignerSubsystem* p;
	bool align_result = false;
	int align_length = 0.0;
	int align_angle = 0.0;

	bool vacuum_enable = false;
	bool with_wafer_mode = false;
};

/**
* FortrendAlignerSubsystemPrivate
*/
FortrendAlignerSubsystemPrivate::FortrendAlignerSubsystemPrivate(FortrendAlignerSubsystem*p)
	:p(p){

}

/**
* FortrendAlignerSubsystem
*/
FortrendAlignerSubsystem::FortrendAlignerSubsystem(IKernel*  kernel, const std::string& name)
	:AbstractIOSubsystem(kernel, name)
	, FortrendAbstractAligner(kernel)
	, AlignerSubSystemHelper(name)
	, d(new FortrendAlignerSubsystemPrivate(this)){
	//init 
	
} 

void FortrendAlignerSubsystem::onInitialize()throw(KernelException){
	try{
		setState(IKernelSubSystem::State::SUB_NORMAL);
		/*if(enableProtocol())
			setState(IKernelSubSystem::State::SUB_IDEL);
		else
			setState(IKernelSubSystem::State::SUB_UNKNOWN);*/
	}
	catch (KernelException& e){
		logError(getName().c_str(), e.what());
		//throw e;
	}
}

void FortrendAlignerSubsystem::onUnInitialize()throw(KernelException){
	disableProtocol();
}

void FortrendAlignerSubsystem::onProcess(){
	//pollProtocol();
}


void FortrendAlignerSubsystem::onConfigure(const std::shared_ptr<KernelConfiguration> & config){
	KernelAbstractSubSystem::onConfigure(config);
	FortrendAbstractAligner::configure(config);
	configAligner(config);

}


void FortrendAlignerSubsystem::getAlignResult(bool &result, int &length, int & angle)const
{
	length = d->align_length;
	angle = d->align_angle;
	result = d->align_result;
}
void FortrendAlignerSubsystem::getAlignClear()
{
	d->align_length = 0.0;
	d->align_angle = 0.0;
	d->align_result = false;
}
void FortrendAlignerSubsystem::setAlignResult(const bool result,const int length,const int angle)
{
	d->align_result = result;
	d->align_length = length;
	d->align_angle = angle;
	KernelAbstractSubSystem::emitAttributeChanged(this);
}

void FortrendAlignerSubsystem::setVacuumEnable(const bool value){
	if (d->vacuum_enable != value)
	{
		d->vacuum_enable = value;
	}
}
void FortrendAlignerSubsystem::setWithWaferModeEnable(const bool value){
	if (d->with_wafer_mode != value)
	{
		d->with_wafer_mode = value;
	}
}


std::shared_ptr<KernelSubsystemResetCommand> FortrendAlignerSubsystem::createResetCommand()const{
	FortrendAlignerSubsystem* self = const_cast<FortrendAlignerSubsystem*>(this);
	AlignerResetCommand::Ptr ret(new AlignerResetCommand(self));
	return ret;
}

std::shared_ptr<KernelSubsystemUpdateCommand> FortrendAlignerSubsystem::createUpdateCommand()const{
	FortrendAlignerSubsystem* self = const_cast<FortrendAlignerSubsystem*>(this);
	AlignerUpdateCommand::Ptr ret(new AlignerUpdateCommand(self));
	return ret;
}

std::shared_ptr<AbstractOutPutCommand>  FortrendAlignerSubsystem::createOutputCommand(int channel, bool stat)const{
	FortrendAlignerSubsystem* self = const_cast<FortrendAlignerSubsystem*>(this);
	AbstractOutPutCommand::Ptr ret(new AlignerOutputCommand(channel, stat,self));
	return ret;
}

std::shared_ptr<AlignerAlignCommand>  FortrendAlignerSubsystem::createAlignCommand()const{
	FortrendAlignerSubsystem* self = const_cast<FortrendAlignerSubsystem*>(this);
	AlignerAlignCommand::Ptr ret(new AlignerAlignCommand(self));
	return ret;
}

//std::shared_ptr<AlignerLockCommand>  FortrendAlignerSubsystem::createLockCommand(Aligner_Station station, Aligner_Arm arm)const{
//	FortrendAlignerSubsystem* self = const_cast<FortrendAlignerSubsystem*>(this);
//	AlignerLockCommand::Ptr ret(new AlignerLockCommand(station, arm, self));
//	return ret;
//}
//
//std::shared_ptr<AlignerUnLockCommand>  FortrendAlignerSubsystem::createUnLockCommand(Aligner_Station station, Aligner_Arm arm)const{
//	FortrendAlignerSubsystem* self = const_cast<FortrendAlignerSubsystem*>(this);
//	AlignerUnLockCommand::Ptr ret(new AlignerUnLockCommand(station, arm, self));
//	return ret;
//}

}

