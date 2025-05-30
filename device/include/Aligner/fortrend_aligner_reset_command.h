/**
* @file            fortrend_aligner_reset_command.h
* @brief           reset command for Aligner
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Subsystem/Aligner

#ifndef FORTREND_ALIGNER_RESET_COMMAND_INCLUDE_
#define FORTREND_ALIGNER_RESET_COMMAND_INCLUDE_

#include "kernel/kernel_subsystem_command.h"
#include "Kernel/kernel_subsystem_reset_command.h"

#include "Aligner/aligner_command_executer.h"
#include "Aligner/aligner_subsystem_helper.h"

namespace FC{



	/**
	* @brief  output command for aligner
	*/
	class  AlignerResetCommand : public  KernelSubsystemResetCommand, public AlignerCommandExecuter{

	public:
		DECLARE_PTR(AlignerResetCommand)
		AlignerResetCommand(AlignerSubSystemHelper* hexHelper);
		virtual std::string getName()const override { return "Reset"; }

	protected:
		virtual RunResult onRun() throw(KernelException);
	private:
		DECLARE_PRIVATE(AlignerResetCommand)
	};


}
#endif
