/**
* @file            fortrend_pump_output_command.h
* @brief           output command for Pump
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Subsystem/Pump

#ifndef FORTREND_ALIGNER_OUTPUT_COMMAND_INCLUDE_
#define FORTREND_ALIGNER_OUTPUT_COMMAND_INCLUDE_

#include "kernel/kernel_subsystem_command.h"
#include "Kernel/Fortrend/abstract_output_command.h"
#include "kernel/kernel_exception.h" 

#include "KeyencePLC/keyence_plc_command_executer.h"
#include "KeyencePLC/keyence_plc_subsystem_helper.h"

namespace FC{

	/**
	* @brief  output command for loadlock
	*/
	class  PumpOutputCommand : public  AbstractOutPutCommand, public KeyencePlcCommandExecuter{
	
	public:
		DECLARE_PTR(PumpOutputCommand)
		PumpOutputCommand(int chanel, bool state, KeyencePlcSubSystemHelper* helper);
		virtual std::string getName()const override { return "Output"; }

	protected:
		virtual RunResult onRun() throw(KernelException);		
	private:
		DECLARE_PRIVATE(PumpOutputCommand)
	};




}
#endif
