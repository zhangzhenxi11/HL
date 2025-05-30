/**
* @file            fortrend_loadlock_move_to_slot_command.h
* @brief           move to slot command for LoadLock
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Subsystem/LoadLock

#ifndef FORTREND_LOADLOCK_MOVE_TO_SLOT_COMMAND_INCLUDE_
#define FORTREND_LOADLOCK_MOVE_TO_SLOT_COMMAND_INCLUDE_

#include "kernel/kernel_subsystem_command.h"

#include "KeyencePLC/keyence_plc_command_executer.h"
#include "KeyencePLC/keyence_plc_subsystem_helper.h"

namespace FC{

	/**
	* @brief  move to slot command for loadlock
	*/
	class  LoadLockMoveToSlotCommand : public  KernelSubsystemCommand, public KeyencePlcCommandExecuter{
	public:
		DECLARE_PTR(LoadLockMoveToSlotCommand)
		LoadLockMoveToSlotCommand(const int slot, KeyencePlcSubSystemHelper* helper);
		virtual std::string getName()const override { return "MoveToSlot"; }
		int getSlot();

	protected:
		virtual RunResult onRun() throw(KernelException);
	private:
		DECLARE_PRIVATE(LoadLockMoveToSlotCommand)
	};




}
#endif
