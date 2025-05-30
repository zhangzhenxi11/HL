/**
* @file            fortrend_cooling_cavity_subsystem.h
* @brief           Fortrend CoolingCavity
* @author		   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/CoolingCavity


#ifndef _XLH_FORTREND_COOLING_CAVITY_SUBSYSTEM_INCLUDE_
#define _XLH_FORTREND_COOLING_CAVITY_SUBSYSTEM_INCLUDE_ 

#include "kernel/Fortrend/abstract_io_subsystem.h"
#include "kernel/Fortrend/fortrend_abstract_station.h"
#include "Kernel/Fortrend/hex_subsystem_helper.h"

#include "CoolingCavity/fortrend_cooling_cavity_reset_command.h"
#include "CoolingCavity/fortrend_cooling_cavity_cooling_command.h"

#include "KeyencePLC/keyence_plc_command_executer.h"
#include "KeyencePLC/keyence_plc_subsystem_helper.h"

namespace FC{


	/**
	* @brief fortrend cooling_cavity
	*/
	class  FortrendCoolingCavitySubsystem :public AbstractIOSubsystem, public FortrendAbstractStation, public KeyencePlcSubSystemHelper{
	public:
		DECLARE_PTR(FortrendCoolingCavitySubsystem)
		FortrendCoolingCavitySubsystem(IKernel*  kernel, const std::string& name);

		//override for FortrendAbstractStation
		virtual bool hasBoxPlacement()const override;
		virtual bool hasBoxPresent()const override { return true; }
		virtual bool hasDoorOpend()const;
		//override for AbstractIOSubsystem
		virtual int inputCount()const override;
		virtual int outputCount()const override;
		virtual bool getInput(int index)const override;
		virtual std::string getInputName(int index)const override;
		virtual std::string getOutputName(int index)const override;


		virtual std::shared_ptr<KernelSubsystemResetCommand> createResetCommand()const override;
		virtual std::shared_ptr<KernelSubsystemUpdateCommand> createUpdateCommand() const override;
		virtual std::shared_ptr<AbstractOutPutCommand>  createOutputCommand(int channel, bool stat)const override;


	public:

		std::shared_ptr<CoolingCavityCoolingCommand> createCoolingCommand()const;

	public:
		void setVacuumEnable(const bool value);
		void setWithWaferModeEnable(const bool value);
	protected:
		virtual void onInitialize()throw(KernelException)override;
		virtual void onUnInitialize()override;
		virtual void onProcess()override;
		virtual void onConfigure(const std::shared_ptr<KernelConfiguration> & config)override;


	private:
		DECLARE_PRIVATE(FortrendCoolingCavitySubsystem)

	};

}

#endif
