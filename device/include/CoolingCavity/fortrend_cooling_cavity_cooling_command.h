/**
* @file            fortrend_cooling_cavity_cooling_command.h
* @brief           cooling command for CoolingCavity
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Subsystem/CoolingCavity

#ifndef FORTREND_COOLING_CAVITY_COOLING_COMMAND_INCLUDE_
#define FORTREND_COOLING_CAVITY_COOLING_COMMAND_INCLUDE_

#include "kernel/kernel_subsystem_command.h"

#include "KeyencePLC/keyence_plc_command_executer.h"
#include "KeyencePLC/keyence_plc_subsystem_helper.h"

namespace FC{

	/**
	* @brief  cooling command for cooling cavity
	*/
	class  CoolingCavityCoolingCommand : public  KernelSubsystemCommand, public KeyencePlcCommandExecuter{
	public:
		DECLARE_PTR(CoolingCavityCoolingCommand)
		CoolingCavityCoolingCommand(KeyencePlcSubSystemHelper* helper);
		virtual std::string getName()const override { return "Cooling"; }

	protected:
		virtual RunResult onRun() throw(KernelException);		
	private:
		DECLARE_PRIVATE(CoolingCavityCoolingCommand)
	};




}
#endif
