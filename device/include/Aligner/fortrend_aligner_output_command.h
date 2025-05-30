/**
* @file            fortrend_aligner_output_command.h
* @brief           output command for Aligner
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Subsystem/Aligner

#ifndef FORTREND_ALIGNER_OUTPUT_COMMAND_INCLUDE_
#define FORTREND_ALIGNER_OUTPUT_COMMAND_INCLUDE_

#include "kernel/kernel_subsystem_command.h"
#include "Kernel/Fortrend/abstract_output_command.h"
#include "kernel/kernel_exception.h" 

#include "Aligner/aligner_command_executer.h"
#include "Aligner/aligner_subsystem_helper.h"

namespace FC{

	/**
	* @brief  output command for loadlock
	*/
	class  AlignerOutputCommand : public  AbstractOutPutCommand, public AlignerCommandExecuter{
	
	public:
		DECLARE_PTR(AlignerOutputCommand)
		AlignerOutputCommand(int chanel, bool state, AlignerSubSystemHelper* helper);
		virtual std::string getName()const override { return "Output"; }

	protected:
		virtual RunResult onRun() throw(KernelException);		
	private:
		DECLARE_PRIVATE(AlignerOutputCommand)
	};




}
#endif
