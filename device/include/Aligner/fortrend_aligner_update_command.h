/**
* @file            fortrend_aligner_update_command.h
* @brief           update command for Aligner
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Subsystem/Aligner

#ifndef FORTREND_ALIGNER_UPDATE_COMMAND_INCLUDE_
#define FORTREND_ALIGNER_UPDATE_COMMAND_INCLUDE_
#include "kernel/kernel_subsystem_command.h"
#include "Kernel/kernel_subsystem_update_command.h"
#include "kernel/kernel_exception.h" 

#include "Aligner/aligner_command_executer.h"
#include "Aligner/aligner_subsystem_helper.h"

namespace FC{

	/**
	* @brief  update command for Aligner
	*/
	class  AlignerUpdateCommand : public  KernelSubsystemUpdateCommand, public AlignerCommandExecuter{

	public:
		DECLARE_PTR(AlignerUpdateCommand)
		AlignerUpdateCommand(AlignerSubSystemHelper* helper);
		virtual std::string getName()const override { return "Update"; }

	protected:
		virtual RunResult onRun() throw(KernelException);		
	private:
		DECLARE_PRIVATE(AlignerUpdateCommand)
	};




}
#endif
