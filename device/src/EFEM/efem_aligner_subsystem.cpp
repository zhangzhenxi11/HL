// Library: Fortrend
// Package: SubAligerImp
//
// RND aligner SubSystem
//
// author xielonghua
//
#include "EFEM/efem_aligner_subsystem.h"
#include "EFEM/efem_aligner_reset_command.h"
#include "EFEM/efem_aligner_status_command.h"
#include "EFEM/efem_aligner_align_command.h"
#include "EFEM/efem_aligner_rotate_command.h"

#include "Kernel/Fortrend/rnd_output_command.h"
#include "Kernel/kernel_log.h"
#include <chrono>  
#include <mutex>

#include "Poco/Format.h"
#include "Poco/StringTokenizer.h"
#include "Poco/NumberParser.h"
KERNEL_NS_BEGIN

/**
* EFEMAlignerSubsystem 
*/

class EFEMAlignerSubsystemPrivate {
public:

};


EFEMAlignerSubsystem::EFEMAlignerSubsystem(IKernel*  kernel,const std::string& name)
:AlignerAbstractSubsystem(kernel, name)
, TcpEfemSubSystemHelper(name){

}


void EFEMAlignerSubsystem::onInitialize()throw(KernelException){
	
	
	try{
		enableProtocol();
		//setState(IKernelSubSystem::State::SUB_IDEL);
		setState(IKernelSubSystem::State::SUB_NORMAL);
	}
	catch (KernelException& e){
		setState(IKernelSubSystem::State::SUB_NORMAL);
		logError(getName().c_str(), e.what());;
		//throw e;
	}
}

void EFEMAlignerSubsystem::onUnInitialize()throw(KernelException){
	disableProtocol();
	setState(IKernelSubSystem::State::SUB_UNKNOWN);
}

void EFEMAlignerSubsystem::onProcess(){


}


void EFEMAlignerSubsystem::onConfigure(const std::shared_ptr<KernelConfiguration> & conf){
	AlignerAbstractSubsystem::onConfigure(conf);
	//EFEM config
	configTcpEfem(conf);

}

std::shared_ptr<KernelSubsystemResetCommand> EFEMAlignerSubsystem::createResetCommand()const{
	EFEMAlignerSubsystem* self = const_cast<EFEMAlignerSubsystem*>(this);
	EFEMAlignerResetCommand::Ptr ret(new EFEMAlignerResetCommand(self));
	return ret;
}

std::shared_ptr<AlignerAbstractRotateCommand> EFEMAlignerSubsystem::createRotateCommand(float angle)const{
	EFEMAlignerSubsystem* self = const_cast<EFEMAlignerSubsystem*>(this);
	AlignerAbstractRotateCommand::Ptr ret(new EFEMAlignerRotateCommand(self, angle));
	return ret;
}

std::shared_ptr<KernelSubsystemUpdateCommand> EFEMAlignerSubsystem::createUpdateCommand()const{
	EFEMAlignerSubsystem* self = const_cast<EFEMAlignerSubsystem*>(this);
	KernelSubsystemUpdateCommand::Ptr ret(new EFEMAlignerStatusCommand(self));
	return ret;
}

std::shared_ptr<AlignerAbstractAlignCommand> EFEMAlignerSubsystem::createAlignCommand()const{
	EFEMAlignerSubsystem* self = const_cast<EFEMAlignerSubsystem*>(this);
	AlignerAbstractAlignCommand::Ptr ret(new EFEMAlignerAlignCommand(self));
	return ret;
}

std::shared_ptr<AbstractOutPutCommand> EFEMAlignerSubsystem::createOutputCommand(int channel, bool stat)const{
	EFEMAlignerSubsystem* self = const_cast<EFEMAlignerSubsystem*>(this);
	//auto cmd = std::make_shared<HexOutputCommand>(channel, stat, self);
	//cmd->setHasReply(false);
	return nullptr;
}
std::shared_ptr<AlignerAbstractVaccOnCommand> EFEMAlignerSubsystem::createVaccOnCommand() const {
	throw std::exception("not impl");
}
std::shared_ptr<AlignerAbstractVaccOffCommand> EFEMAlignerSubsystem::createVaccOffCommand() const {
	throw std::exception("not impl");
}

KERNEL_NS_END
