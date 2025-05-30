
/**
* @file            fortrend_aligner_update_command.h
* @brief           update command for aligner
* @author		   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/Aligner

#include "Poco/Format.h"

#include "Kernel/kernel.h"
#include "Kernel/kernel_abstract_subsystem.h"
#include "Kernel/kernel_command.h"
#include "kernel/kernel_command_reject_exception.h"
#include "Kernel/kernel_log.h"
#include "kernel/Fortrend/fortrend_cassette_manager.h"


#include "Aligner/fortrend_aligner_update_command.h"
#include "Aligner/fortrend_aligner_subsystem.h"


namespace FC{

	

	/**
	* AlignerUpdateCommand
	*/
	AlignerUpdateCommand::AlignerUpdateCommand(AlignerSubSystemHelper* hexHelper)
		:AlignerCommandExecuter(hexHelper){
		
	};
	
	/**
	* return true if success else false.
	*/
	AlignerUpdateCommand::RunResult AlignerUpdateCommand::onRun() throw(KernelException){
		FortrendAlignerSubsystem* ll = dynamic_cast<FortrendAlignerSubsystem*>(getSubsystem());
		//
		if (!ll){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "subsystem type error.", this);
		}
		
		
		
		//get command configure
		std::shared_ptr<KernelConfiguration> command_config = ll->getConfigure()->createView(getName());

		//fill params
		
		IKernelCommand::RunResult ret = IKernelCommand::RunResult::RUN_OK;
		if (ret == IKernelCommand::RunResult::RUN_OK)
		{

		}
		return ret;

	}



}