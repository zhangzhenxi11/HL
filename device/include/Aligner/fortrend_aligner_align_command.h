/**
* @file            fortrend_aligner_align_command.h
* @brief           align command for aligner
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Subsystem/Aligner

#ifndef FORTREND_ALIGNER_ALIGN_COMMAND_INCLUDE_
#define FORTREND_ALIGNER_ALIGN_COMMAND_INCLUDE_

#include "kernel/kernel_subsystem_command.h"

#include "Aligner/aligner_command_executer.h"
#include "Aligner/aligner_subsystem_helper.h"

namespace FC{

	/**
	* @brief  align command for Aligner
	*/
	class  AlignerAlignCommand : public std::enable_shared_from_this<AlignerAlignCommand>, public  KernelSubsystemCommand, public AlignerCommandExecuter{
	public:
		DECLARE_PTR(AlignerAlignCommand)
		AlignerAlignCommand(AlignerSubSystemHelper* helper);
		virtual std::string getName()const override { return "Align"; }

	protected:
		virtual RunResult onRun() throw(KernelException);		
	private:
		DECLARE_PRIVATE(AlignerAlignCommand)
	};




}
#endif
