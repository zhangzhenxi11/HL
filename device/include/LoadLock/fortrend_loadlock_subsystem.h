/**
* @file            fortrend_loadlock_subsystem.h
* @brief           Fortrend LoadLock
* @author		   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/LoadLock


#ifndef _XLH_FORTREND_LOADLOCK_SUBSYSTEM_INCLUDE_
#define _XLH_FORTREND_LOADLOCK_SUBSYSTEM_INCLUDE_ 

#include "kernel/Fortrend/abstract_io_subsystem.h"
#include "kernel/Fortrend/fortrend_abstract_station.h"
#include "Kernel/Fortrend/hex_subsystem_helper.h"

#include "LoadLock/fortrend_loadlock_defined.h"
#include "LoadLock/fortrend_loadlock_reset_command.h"
#include "LoadLock/fortrend_loadlock_open_cassette_door_command.h"
#include "LoadLock/fortrend_loadlock_close_cassette_door_command.h"
#include "LoadLock/fortrend_loadlock_mapping_command.h"
#include "LoadLock/fortrend_loadlock_move_to_slot_command.h"
#include "LoadLock/fortrend_loadlock_close_tm_cavity_door_command.h"
#include "LoadLock/fortrend_loadlock_open_tm_cavity_door_command.h"
#include "LoadLock/fortrend_loadlock_close_exhaust_valve_command.h"
#include "LoadLock/fortrend_loadlock_open_exhaust_valve_command.h"
#include "LoadLock/fortrend_loadlock_close_diaphragm_valve_command.h"
#include "LoadLock/fortrend_loadlock_open_diaphragm_valve_command.h"
#include "LoadLock/fortrend_loadlock_close_angle_valve_command.h"
#include "LoadLock/fortrend_loadlock_open_angle_valve_command.h"
#include "LoadLock/fortrend_loadlock_clear_error_command.h"
#include "LoadLock/fortrend_loadlock_auto_break_vacuum_command.h"

#include "LoadLock/fortrend_loadlock_cavity_open_inserting_plate_valve_command.h"
#include "LoadLock/fortrend_loadlock_cavity_close_inserting_plate_valve_command.h"
#include "LoadLock/fortrend_loadlock_cavity_open_height_vacuum_baffle_valve_command.h"
#include "LoadLock/fortrend_loadlock_cavity_close_height_vacuum_baffle_valve_command.h"

#include "KeyencePLC/keyence_plc_command_executer.h"
#include "KeyencePLC/keyence_plc_subsystem_helper.h"

namespace FC{


	/**
	* @brief fortrend loadlock
	*/
	class  FortrendLoadLockSubsystem :public AbstractIOSubsystem, public FortrendAbstractStation, public KeyencePlcSubSystemHelper{
	public:
		DECLARE_PTR(FortrendLoadLockSubsystem)
		FortrendLoadLockSubsystem(IKernel*  kernel, const std::string& name);

		//override for FortrendAbstractStation
		virtual bool hasBoxPlacement()const override;
		virtual bool hasBoxPresent()const override;
		virtual bool hasDoorOpend()const override;
		//override for AbstractIOSubsystem
		virtual int inputCount()const override;
		virtual int outputCount()const override;
		virtual bool getInput(int index)const override;
		virtual std::string getInputName(int index)const override;
		virtual std::string getOutputName(int index)const override;


		virtual std::shared_ptr<KernelSubsystemResetCommand> createResetCommand()const override;
		virtual std::shared_ptr<KernelSubsystemUpdateCommand> createUpdateCommand() const override;
		virtual std::shared_ptr<AbstractOutPutCommand>  createOutputCommand(int channel, bool stat)const override;

	public:
		std::shared_ptr<LoadLockOpenCassetteDoorCommand> createOpenCassetteDoorCommand()const;
		std::shared_ptr<LoadLockCloseCassetteDoorCommand> createCloseCassetteDoorCommand()const;
		std::shared_ptr<LoadLockMappingCommand> createMappingCommand()const;
		std::shared_ptr<LoadLockMoveToSlotCommand> createMoveToSlotCommand(const int slot)const;
		std::shared_ptr<LoadLockOpenTMCavityDoorCommand> createOpenTMCavityDoorCommand() const;
		std::shared_ptr<LoadLockCloseTMCavityDoorCommand> createCloseTMCavityDoorCommand() const;
		std::shared_ptr<LoadLockOpenDiaphragmValveCommand> createOpenDiaphragmValveCommand(const LoadLockValveOpening opening) const;
		std::shared_ptr<LoadLockCloseDiaphragmValveCommand> createCloseDiaphragmValveCommand(const LoadLockValveOpening opening) const;
		std::shared_ptr<LoadLockOpenExhaustValveCommand> createOpenExhaustValveCommand() const;
		std::shared_ptr<LoadLockCloseExhaustValveCommand> createCloseExhaustValveCommand() const;
		std::shared_ptr<LoadLockOpenAngleValveCommand> createOpenAngleValveCommand() const;
		std::shared_ptr<LoadLockCloseAngleValveCommand> createCloseAngleValveCommand() const;
		std::shared_ptr<LoadLockClearErrorCommand> createClearErrorCommand() const;
		std::shared_ptr<LoadLockOpenInsertingPlateValveCommand> FortrendLoadLockSubsystem::createOpenInsertingPlateValveCommand()const;
		std::shared_ptr<LoadLockCloseInsertingPlateValveCommand> FortrendLoadLockSubsystem::createCloseInsertingPlateValveCommand()const;
		std::shared_ptr<LoadLockOpenHeightVacuumBaffleValveCommand> FortrendLoadLockSubsystem::createOpenHeightVacuumBaffleValveCommand()const;
		std::shared_ptr<LoadLockCloseHeightVacuumBaffleValveCommand> FortrendLoadLockSubsystem::createCloseHeightVacuumBaffleValveCommand()const;

		std::shared_ptr<LoadLockAutoBreakVacuumCommand> FortrendLoadLockSubsystem::createAutoBreakVacuumCommand()const;//破真空指令

	public:
		bool getCassetteDoorOpend()const;
		void setCassetteDoorOpend(const bool value);
		bool getTMCavityDoorOpend()const;
		void setTMCavityDoorOpend(const bool value);

		bool getSlowDiaphragmValveOpend()const;
		void setSlowDiaphragmValveOpend(const bool value);

		bool getFastDiaphragmValveOpend()const;
		void setFastDiaphragmValveOpend(const bool value);

		bool getAngleValveOpend()const;
		void setAngleValveOpend(const bool value);

		bool getExhaustValveOpend()const;
		void setExhaustValveOpend(const bool value);

		bool getPresentSensorState(const int index);

		//LoadLock1凸出检测
		bool getProtrudingSensorState()const;

		bool getHeightVacuumBaffleValveOpend()const;
		void setHeightVacuumBaffleValveOpend(const bool value);

		bool getInsertingPlateValveOpend()const;
		void setInsertingPlateValveOpend(const bool value);

		void setBoxPlacement(const bool value);

		int getLastMoveSlot()const;
		void setLastMoveSlot(const int value);

		int getCassetteSlotCount() const;

		//当前腔室真空值
		double getVacuumValue()const;
		int getVacuumPressureGageState()const;

		bool getVacuumValueReachesTheSetValue()const;

		bool getVacuumValueUpperLimitReachesTheSetValue()const;


		//排气泄压
		bool getExhaustVacuumValueReachesTheSetValue()const;
		bool getQuickInflationValueReachesTheSetValue()const;

		
		bool getLoadLockRoughVacuumReachesTheSetValue()const;
		bool getLoadLockRoughVacuumReachesTheSetValue(int value)const;

		void setVacuumUpperLimitAndExtractionValue(const double upper_limit_value, const double extraction_value);
		void setVacuumFastDiapgragmValueAndAngleValue(const double diaphragm_value, const double angle_value);

		//设置粗抽真空值
		void setRoughVacuumValue(const double rough_vacuum_set_value);

		//达到角阀快抽条件
		bool getFastAngleValveReachesTheSetValue() const;

		bool getVacuumEnable()const;
		void setVacuumEnable(const bool value);

		bool getWithWaferModeEnable()const;
		void setWithWaferModeEnable(const bool value);

		void recardVacuum()const;

		bool getHasResetFlag()const;
		void setHasResetFlag(const bool value);

		//SMIF Lock
		bool hasReadyCloseDoor();
		void setReadyCloseDoorFlag(const bool value);

		void getFirstLayerMapping(Cassette::Mapping &map);
		bool setFirstLayerMapping(const bool value);

		void getSecondLayerMapping(Cassette::Mapping &map);
		bool setSecondLayerMapping(const bool value);

		//单槽mapping
		Cassette::Mapping handleSingleSlotMapping(short mapRes);


		bool getLoadLockCavitySafeSignal();
		void setLoadLockCavitySafeSignal(const bool value);

		bool getLoadLockCassetteCloseSafeSignal();
		void setLoadLockCassetteCloseSafeSignal(const bool value);

	protected:
		virtual void onInitialize()throw(KernelException)override;
		virtual void onUnInitialize()override;
		virtual void onProcess()override;
		virtual void onConfigure(const std::shared_ptr<KernelConfiguration> & config)override;
	private:
		DECLARE_PRIVATE(FortrendLoadLockSubsystem)

	};

}

#endif
