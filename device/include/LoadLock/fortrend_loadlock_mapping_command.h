/**
* @file            fortrend_loadlock_mapping_command.h
* @brief           mapping command for LoadLock
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Subsystem/LoadLock

#ifndef FORTREND_LOADLOCK_MAPPING_COMMAND_INCLUDE_
#define FORTREND_LOADLOCK_MAPPING_COMMAND_INCLUDE_

#include "kernel/kernel_subsystem_command.h"

#include "KeyencePLC/keyence_plc_command_executer.h"
#include "KeyencePLC/keyence_plc_subsystem_helper.h"

namespace FC{

	/**
	* @brief  mapping command for loadlock
	*/
	class  LoadLockMappingCommand : public  KernelSubsystemCommand, public KeyencePlcCommandExecuter{
	public:
		DECLARE_PTR(LoadLockMappingCommand)
		LoadLockMappingCommand(KeyencePlcSubSystemHelper* helper);
		virtual std::string getName()const override { return "Mapping"; }

	protected:
		virtual RunResult onRun() throw(KernelException);
	private:
		DECLARE_PRIVATE(LoadLockMappingCommand)
	};




}
#endif
