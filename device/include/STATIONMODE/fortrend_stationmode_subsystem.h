/**
* @file        fortrend_StationMode_subsystem.h
* @brief       Fortrend StationMode 
* @author      xielonghua
*/

// Library: Fortrend
// Package: SubSystem/STATIONMODE


#ifndef _XLH_FORTREND_STATIONMODE_SUBSYSTEM_INCLUDE_
#define _XLH_FORTREND_STATIONMODE_SUBSYSTEM_INCLUDE_

#include "kernel/Fortrend/abstract_io_subsystem.h"
#include "Kernel/Fortrend/hex_subsystem_helper.h"
//special commands
#include "fortrend_stationmode_reset_command.h"
#include "kernel/Fortrend/fortrend_abstract_station.h"

namespace FC{

/**
 * @brief fortrend StationMode subsystem class
 *
 */
class  FortrendSTATIONMODESubsystem : public AbstractIOSubsystem,  public FortrendAbstractStation {
public:
     DECLARE_PTR(FortrendSTATIONMODESubsystem)
     FortrendSTATIONMODESubsystem(IKernel*  kernel, const std::string& name);

     //override for AbstractIOSubsystem
     virtual int inputCount()const;   
     virtual int outputCount()const;
     virtual bool getInput(int index)const;
     virtual std::string getInputName(int index)const;
     virtual std::string getOutputName(int index)const;

     virtual std::shared_ptr<KernelSubsystemResetCommand> createResetCommand()const override;
     virtual std::shared_ptr<KernelSubsystemUpdateCommand> createUpdateCommand() const override;
     virtual std::shared_ptr<AbstractOutPutCommand>  createOutputCommand(int channel, bool stat)const override;
     
public:
    //special commands
    //std::shared_ptr<StationmodeResetCommand> createResetCommand()const;


protected:
     virtual void onInitialize()throw(KernelException)override;
     virtual void onUnInitialize()override;
     virtual void onProcess()override;
     virtual void onConfigure(const std::shared_ptr<KernelConfiguration> & config)override;
private:
     DECLARE_PRIVATE(FortrendSTATIONMODESubsystem)

};

}

#endif