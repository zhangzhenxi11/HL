
/**
* @file            fortrend_cooling_cavity_subsystem.h
* @brief           Fortrend CoolingCavity
* @author		   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/CoolingCavity

#include <iostream>
#include <Windows.h>

#include "Poco/Format.h"
#include "Poco/StringTokenizer.h"
#include "Poco/NumberParser.h"

#include "kernel/kernel.h"
#include "Kernel/kernel_log.h"
#include "kernel/kernel_iocontrol.h"
#include "kernel/kernel_configure.h"
#include "Kernel/kernel_configure.h"

#include "CoolingCavity/fortrend_cooling_cavity_subsystem.h" 
#include "CoolingCavity/fortrend_cooling_cavity_reset_command.h"
#include "CoolingCavity/fortrend_cooling_cavity_update_command.h"
#include "CoolingCavity/fortrend_cooling_cavity_output_command.h"


namespace FC{
	/**
	* FortrendCoolingCavitySubsystemPrivate
	*/
	class FortrendCoolingCavitySubsystemPrivate{
	public:
		FortrendCoolingCavitySubsystemPrivate(FortrendCoolingCavitySubsystem*p);
		void setInput(uint8_t index, bool stat);
	public:
		FortrendCoolingCavitySubsystem* p;
		std::string io_input_address = "";
		unsigned short io_input_count = 0;
		std::vector<bool> io_input_last_value;
		bool *ptr_io_input_state;

		bool vacuum_enable = false;
		bool with_wafer_mode = false;
	};

	/**
	* FortrendCoolingCavitySubsystemPrivate
	*/
	FortrendCoolingCavitySubsystemPrivate::FortrendCoolingCavitySubsystemPrivate(FortrendCoolingCavitySubsystem*p)
		:p(p){

	}

	/**
	* FortrendCoolingCavitySubsystem
	*/
	FortrendCoolingCavitySubsystem::FortrendCoolingCavitySubsystem(IKernel*  kernel, const std::string& name)
		:AbstractIOSubsystem(kernel, name)
		, FortrendAbstractStation(kernel)
		, KeyencePlcSubSystemHelper(name)
		, d(new FortrendCoolingCavitySubsystemPrivate(this)){
		//init 

	}
	bool FortrendCoolingCavitySubsystem::hasBoxPlacement()const
	{
		return getInput(0) && getInput(1);
	}

	bool FortrendCoolingCavitySubsystem::hasDoorOpend()const{
		return false;
	}

	int FortrendCoolingCavitySubsystem::inputCount()const 
	{
		return helperInputCount();
	}

	int FortrendCoolingCavitySubsystem::outputCount()const
	{
		return helperOutputCount();
	}

	bool FortrendCoolingCavitySubsystem::getInput(int index)const
	{
		if (index < d->io_input_count)
		{
			return d->ptr_io_input_state[index];
		}
		return false;
	}

	std::string FortrendCoolingCavitySubsystem::getInputName(int index)const
	{
		return getHelperInputName(index);
	}

	std::string FortrendCoolingCavitySubsystem::getOutputName(int index)const
	{
		return getHelperOutputName(index);
	}

	void FortrendCoolingCavitySubsystem::onInitialize()throw(KernelException){
		try{
			if (enableProtocol())
				setState(IKernelSubSystem::State::SUB_IDEL);
			else
				setState(IKernelSubSystem::State::SUB_UNKNOWN);


		}
		catch (KernelException& e){
			logError(getName().c_str(), e.what());
			//throw e;
		}
	}

	void FortrendCoolingCavitySubsystem::onUnInitialize()throw(KernelException){
		disableProtocol();
	}

	void FortrendCoolingCavitySubsystem::onProcess(){
		//pollProtocol();
		if (getState() != IKernelSubSystem::State::SUB_UNKNOWN && (d->io_input_count > 0))
		{
			
			if (readBits(d->io_input_address, d->io_input_count, d->ptr_io_input_state))
			{
				bool io_changed = false;
				for (size_t i = 0; i < d->io_input_count; i++)
				{
					if (d->ptr_io_input_state[i] != d->io_input_last_value[i])
					{
						d->io_input_last_value[i] = d->ptr_io_input_state[i];
						io_changed = true;
					}
				}
				if (io_changed)
				{
					AbstractIOSubsystem::emitAttributeChanged(this);
				}
			}
			Sleep(50);
		}
	}


	void FortrendCoolingCavitySubsystem::onConfigure(const std::shared_ptr<KernelConfiguration> & config){
		KernelAbstractSubSystem::onConfigure(config);
		FortrendAbstractStation::configure(config);
		configKeyencePlc(config);
		d->io_input_count = inputCount();
		for (size_t i = 0; i < d->io_input_count; i++)
		{
			d->io_input_last_value.push_back(false);
		}
		if (d->io_input_count>0)
		{
			d->ptr_io_input_state = new bool[d->io_input_count];
		}
		d->io_input_address = getHelperInputAddress();
	}


	void FortrendCoolingCavitySubsystem::setVacuumEnable(const bool value){
		if (d->vacuum_enable != value)
		{
			d->vacuum_enable = value;
		}
	}
	void FortrendCoolingCavitySubsystem::setWithWaferModeEnable(const bool value){
		if (d->with_wafer_mode != value)
		{
			d->with_wafer_mode = value;
		}
	}

	std::shared_ptr<KernelSubsystemResetCommand> FortrendCoolingCavitySubsystem::createResetCommand()const{
		FortrendCoolingCavitySubsystem* self = const_cast<FortrendCoolingCavitySubsystem*>(this);
		CoolingCavityResetCommand::Ptr ret(new CoolingCavityResetCommand(self));
		return ret;
	}

	std::shared_ptr<KernelSubsystemUpdateCommand> FortrendCoolingCavitySubsystem::createUpdateCommand()const{
		FortrendCoolingCavitySubsystem* self = const_cast<FortrendCoolingCavitySubsystem*>(this);
		CoolingCavityUpdateCommand::Ptr ret(new CoolingCavityUpdateCommand(self));
		return ret;
	}

	std::shared_ptr<AbstractOutPutCommand>  FortrendCoolingCavitySubsystem::createOutputCommand(int channel, bool stat)const{
		FortrendCoolingCavitySubsystem* self = const_cast<FortrendCoolingCavitySubsystem*>(this);
		AbstractOutPutCommand::Ptr ret(new CoolingCavityOutputCommand(channel, stat, self));
		return ret;
	}

	std::shared_ptr<CoolingCavityCoolingCommand>  FortrendCoolingCavitySubsystem::createCoolingCommand()const{
		FortrendCoolingCavitySubsystem* self = const_cast<FortrendCoolingCavitySubsystem*>(this);
		CoolingCavityCoolingCommand::Ptr ret(new CoolingCavityCoolingCommand(self));
		return ret;
	}

}

