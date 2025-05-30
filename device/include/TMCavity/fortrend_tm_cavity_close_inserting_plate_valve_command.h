/**
* @file            fortrend_tm_cavity_close_inserting_plate_valve_command.h
* @brief           close inserting_plate_valve command for TMCavity
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Subsystem/TMCavity

#ifndef FORTREND_TM_CAVITY_CLOSE_INSERTING_PLATE_VALVE_COMMAND_INCLUDE_
#define FORTREND_TM_CAVITY_CLOSE_INSERTING_PLATE_VALVE_COMMAND_INCLUDE_

#include "kernel/kernel_subsystem_command.h"

#include "KeyencePLC/keyence_plc_command_executer.h"
#include "KeyencePLC/keyence_plc_subsystem_helper.h"

namespace FC{

	/**
	* @brief  close cold pump high vacuum gate valve command for tm cavity
	*/
	class  TMCavityCloseInsertingPlateValveCommand : public  KernelSubsystemCommand, public KeyencePlcCommandExecuter{

	public:
		DECLARE_PTR(TMCavityCloseInsertingPlateValveCommand)
		TMCavityCloseInsertingPlateValveCommand(KeyencePlcSubSystemHelper* helper);
		virtual std::string getName()const override { return "CloseInsertingPlateValve"; }

	protected:
		virtual RunResult onRun() throw(KernelException);		
	private:
		DECLARE_PRIVATE(TMCavityCloseInsertingPlateValveCommand)
	};




}
#endif
