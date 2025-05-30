/**
* @file            fortrend_cooling_cavity_output_command.h
* @brief           output command for CoolingCavity
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Subsystem/CoolingCavity

#ifndef FORTREND_COOLING_CAVITY_OUTPUT_COMMAND_INCLUDE_
#define FORTREND_COOLING_CAVITY_OUTPUT_COMMAND_INCLUDE_

#include "kernel/kernel_subsystem_command.h"
#include "Kernel/Fortrend/abstract_output_command.h"
#include "kernel/kernel_exception.h" 

#include "KeyencePLC/keyence_plc_command_executer.h"
#include "KeyencePLC/keyence_plc_subsystem_helper.h"

namespace FC{

	/**
	* @brief  output command for cooling_cavity
	*/
	class  CoolingCavityOutputCommand : public  AbstractOutPutCommand, public KeyencePlcCommandExecuter{
	
	public:
		DECLARE_PTR(CoolingCavityOutputCommand)
		CoolingCavityOutputCommand(int chanel, bool state, KeyencePlcSubSystemHelper* hexHelper);
		virtual std::string getName()const override { return "Output"; }

	protected:
		virtual RunResult onRun() throw(KernelException);		
	private:
		DECLARE_PRIVATE(CoolingCavityOutputCommand)
	};




}
#endif
