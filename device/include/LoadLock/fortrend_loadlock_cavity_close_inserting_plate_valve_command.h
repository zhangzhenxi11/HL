/**
* @file            fortrend_tm_cavity_close_inserting_plate_valve_command.h
* @brief           close inserting_plate_valve command for LoadLock
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Subsystem/LoadLock

#ifndef FORTREND_LOADLOCK_CLOSE_INSERTING_PLATE_VALVE_COMMAND_INCLUDE_
#define FORTREND_LOADLOCK_CLOSE_INSERTING_PLATE_VALVE_COMMAND_INCLUDE_

#include "kernel/kernel_subsystem_command.h"

#include "KeyencePLC/keyence_plc_command_executer.h"
#include "KeyencePLC/keyence_plc_subsystem_helper.h"

namespace FC{

	/**
	* @brief  close cold pump high vacuum gate valve command for tm cavity
	*/
	class  LoadLockCloseInsertingPlateValveCommand : public  KernelSubsystemCommand, public KeyencePlcCommandExecuter{

	public:
		DECLARE_PTR(LoadLockCloseInsertingPlateValveCommand)
		LoadLockCloseInsertingPlateValveCommand(KeyencePlcSubSystemHelper* helper);
		virtual std::string getName()const override { return "CloseInsertingPlateValve"; }
		std::vector<IKernelResources* > resources() const override;
	protected:
		virtual RunResult onRun() throw(KernelException);		
	private:
		DECLARE_PRIVATE(LoadLockCloseInsertingPlateValveCommand)
	};




}
#endif
