/**
* @file        fortrend_StationMode_subsystem.cpp
* @brief       Fortrend StationMode 
* @author      xielonghua
*/

// Library: Fortrend
// Package: SubSystem/STATIONMODE



#include "STATIONMODE/fortrend_StationMode_subsystem.h" 

#include "kernel/kernel.h"
#include "Kernel/kernel_log.h"
#include "kernel/kernel_configure.h"
#include "Kernel/Fortrend/hex_reset_command.h"
#include "Kernel/Fortrend/hex_update_command.h"
#include "Kernel/Fortrend/hex_output_command.h"

#include "Poco/Format.h"
#include "Poco/StringTokenizer.h"
#include "Poco/NumberParser.h"

namespace FC{
/**
* FortrendSTATIONMODESubsystemPrivate
*/
class FortrendSTATIONMODESubsystemPrivate{
public:
    FortrendSTATIONMODESubsystemPrivate(FortrendSTATIONMODESubsystem*p);
    void setInput(uint8_t index, bool stat);
public:
    FortrendSTATIONMODESubsystem* p;
};

/**
* FortrendSTATIONMODESubsystemPrivate
*/
FortrendSTATIONMODESubsystemPrivate::FortrendSTATIONMODESubsystemPrivate(FortrendSTATIONMODESubsystem*p)
    :p(p){
}



/**
* FortrendSTATIONMODESubsystem
*/
FortrendSTATIONMODESubsystem::FortrendSTATIONMODESubsystem(IKernel*  kernel, const std::string& name)
    :AbstractIOSubsystem(kernel, name)
    , FortrendAbstractStation(kernel)
    , d(new FortrendSTATIONMODESubsystemPrivate(this)){
    //init 
    
}



int FortrendSTATIONMODESubsystem::inputCount()const{
    return 0;
}

int FortrendSTATIONMODESubsystem::outputCount()const{
    return 0;
}

bool FortrendSTATIONMODESubsystem::getInput(int index)const{
    return false;
}

std::string FortrendSTATIONMODESubsystem::getInputName(int index)const{
    return "";
}

std::string FortrendSTATIONMODESubsystem::getOutputName(int index)const{
    return "";
}



//void FortrendSTATIONMODESubsystem::onMessage(const HexMessage::Ptr& message,bool inputChanged) {
// /*   if (inputChanged){
//        AbstractIOSubsystem::emitAttributeChanged(this);
//    }*/
//}

 

void FortrendSTATIONMODESubsystem::onInitialize()throw(KernelException){
    try{
       // enableProtocol();
        setState(IKernelSubSystem::State::SUB_NORMAL);
    }
    catch (KernelException& e){
        logError(getName().c_str(), e.what());;
        //throw e;
    }
}

void FortrendSTATIONMODESubsystem::onUnInitialize()throw(KernelException){
    //disableProtocol();
}

void FortrendSTATIONMODESubsystem::onProcess(){
    //pollProtocol();
}


void FortrendSTATIONMODESubsystem::onConfigure(const std::shared_ptr<KernelConfiguration> & config){
    KernelAbstractSubSystem::onConfigure(config);
    FortrendAbstractStation::configure(config);
   // configHex(config);

}


std::shared_ptr<KernelSubsystemResetCommand> FortrendSTATIONMODESubsystem::createResetCommand()const{
    FortrendSTATIONMODESubsystem* self = const_cast<FortrendSTATIONMODESubsystem*>(this);
   /* KernelSubsystemResetCommand::Ptr ret(new HexResetCommand(self));*/
    return nullptr;
}

std::shared_ptr<KernelSubsystemUpdateCommand> FortrendSTATIONMODESubsystem::createUpdateCommand()const{
    FortrendSTATIONMODESubsystem* self = const_cast<FortrendSTATIONMODESubsystem*>(this);
    //HexUpdateCommand::Ptr ret(new HexUpdateCommand(self));
    return nullptr;
}

std::shared_ptr<AbstractOutPutCommand>  FortrendSTATIONMODESubsystem::createOutputCommand(int channel, bool stat)const{
    FortrendSTATIONMODESubsystem* self = const_cast<FortrendSTATIONMODESubsystem*>(this);
    //AbstractOutPutCommand::Ptr ret(new HexOutputCommand(channel, stat, self));
    return nullptr;
}

//special commands

//std::shared_ptr<StationmodeResetCommand>  FortrendSTATIONMODESubsystem::createResetCommand()const{
//    FortrendSTATIONMODESubsystem* self = const_cast<FortrendSTATIONMODESubsystem*>(this);
//    StationmodeResetCommand::Ptr ret(new StationmodeResetCommand(self));
//    return ret;
//}



}
