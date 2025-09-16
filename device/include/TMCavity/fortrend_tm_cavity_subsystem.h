/**
* @file            fortrend_tm_cavity_subsystem.h
* @brief           Fortrend 
* @author		   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/TMCavity


#ifndef _XLH_FORTREND_TM_CAVITY_SUBSYSTEM_INCLUDE_
#define _XLH_FORTREND_TM_CAVITY_SUBSYSTEM_INCLUDE_ 

#include "kernel/Fortrend/abstract_io_subsystem.h"
#include "kernel/Fortrend/fortrend_abstract_station.h"
#include "Kernel/Fortrend/hex_subsystem_helper.h"

#include "TMCavity/fortrend_tm_cavity_defined.h"
#include "TMCavity/fortrend_tm_cavity_reset_command.h"
#include "TMCavity/fortrend_tm_cavity_open_diaphragm_valve_command.h"
#include "TMCavity/fortrend_tm_cavity_close_diaphragm_valve_command.h"

#include "TMCavity/fortrend_tm_cavity_open_angle_valve_command.h"
#include "TMCavity/fortrend_tm_cavity_close_angle_valve_command.h"
#include "TMCavity/fortrend_tm_cavity_open_inserting_plate_valve_command.h"
#include "TMCavity/fortrend_tm_cavity_close_inserting_plate_valve_command.h"
#include "TMCavity/fortrend_tm_cavity_open_height_vacuum_baffle_valve_command.h"
#include "TMCavity/fortrend_tm_cavity_close_height_vacuum_baffle_valve_command.h"
#include "TMCavity/fortrend_tm_cavity_open_flowmeter_diaphragm_valve_command.h"
#include "TMCavity/fortrend_tm_cavity_close_flowmeter_diaphragm_valve_command.h"
#include "TMCavity/fortrend_tm_cavity_close_pid_command.h"
#include "TMCavity/fortrend_tm_cavity_open_pid_command.h"

#include "KeyencePLC/keyence_plc_command_executer.h"
#include "KeyencePLC/keyence_plc_subsystem_helper.h"


namespace FC{


	/**
	* @brief fortrend tm_cavity
	*/
	class  FortrendTMCavitySubsystem :public AbstractIOSubsystem, public FortrendAbstractStation, public KeyencePlcSubSystemHelper{
	public:
		DECLARE_PTR(FortrendTMCavitySubsystem)
		FortrendTMCavitySubsystem(IKernel*  kernel, const std::string& name);

		//override for FortrendAbstractStation
		virtual bool hasBoxPlacement()const override;
		virtual bool hasBoxPresent()const override { return true; }
		virtual bool hasDoorOpend()const;
		//override for AbstractIOSubsystem
		virtual int inputCount()const override;
		virtual int outputCount()const override;
		virtual bool getInput(int index)const override;
		virtual std::string getInputName(int index)const override;
		virtual std::string getOutputName(int index)const override;

		virtual std::shared_ptr<KernelSubsystemResetCommand> createResetCommand()const override;
		virtual std::shared_ptr<KernelSubsystemUpdateCommand> createUpdateCommand() const override;
		virtual std::shared_ptr<AbstractOutPutCommand>  createOutputCommand(int channel, bool state)const override;
		
	public:
		std::shared_ptr<TMCavityOpenDiaphragmValveCommand> createOpenDiaphragmValveCommand(const TMCavityValveOpening opening)const;
		std::shared_ptr<TMCavityCloseDiaphragmValveCommand> createCloseDiaphragmValveCommand(const TMCavityValveOpening opening)const;

		std::shared_ptr<TMCavityOpenHeightVacuumBaffleValveCommand> createOpenHeightVacuumBaffleValveCommand()const;
		std::shared_ptr<TMCavityCloseHeightVacuumBaffleValveCommand> createCloseHeightVacuumBaffleValveCommand()const;

		std::shared_ptr<TMCavityOpenAngleValveCommand> createOpenAngleValveCommand()const;
		std::shared_ptr<TMCavityCloseAngleValveCommand> createCloseAngleValveCommand()const;

		std::shared_ptr<TMCavityOpenInsertingPlateValveCommand> createOpenInsertingPlateValveCommand()const;
		std::shared_ptr<TMCavityCloseInsertingPlateValveCommand> createCloseInsertingPlateValveCommand()const;

		std::shared_ptr<TMCavityOpenFlowmeterDiaphragmValveCommand> createOpenFlowmeterDiaphragmValveCommand()const;
		std::shared_ptr<TMCavityCloseFlowmeterDiaphragmValveCommand> createCloseFlowmeterDiaphragmValveCommand()const;

		std::shared_ptr<TMCavityOpenPIDCommand> createOpenPIDCommand()const;
		std::shared_ptr<TMCavityClosePIDCommand> createClosePIDCommand()const;

	public:
		bool getSlowDiaphragmValveOpend()const;
		void setSlowDiaphragmValveOpend(const bool value);

		bool getFastDiaphragmValveOpend()const;
		void setFastDiaphragmValveOpend(const bool value);

		bool getHeightVacuumBaffleValveOpend()const;
		void setHeightVacuumBaffleValveOpend(const bool value);

		bool getInsertingPlateValveOpend()const;
		void setInsertingPlateValveOpend(const bool value);

		bool getAngleValveOpend()const;
		void setAngleValveOpend(const bool value);

		bool getPIDOpend() const;
		void setPIDOpend(const bool value);

		//前级泵管路压力表信号
		bool getAxialPressureGageState()const;

		//常务CDA信号
		bool getCDAPressureState() const;

		/*
		*传输腔腔盖门锁信号
		*/
		bool TMCavityCoverSafetyLock();

		//获取TM腔体真空值
		double getTMCavityVacuumValue()const;

		//获取分子管道真空值
		double getMoleculePipelineVacuumValue()const;

		//获取前级泵管道真空值
		double getBackingPipelineVacuumValue()const;

		//获取tm腔真空压力表状态
		int getTMCavityVacuumPressureGageState()const;

		//获取真空上限值
		double getTMCavityVacuumUpperlimitValue();

		//获取抽真空设定值
		double getTMCavityVacuumExtractionSetValue();

		bool getTMCavityVacuumValueReachesTheSetValue() const;

		//TM真空上限达到设定值
		bool getTMCavityVacuumValueUpperLimitReachesTheSetValue()const;
		bool getTMCavityVacuumValueReachesThePIDSetValue() const;
		bool getTMCavityRoughVacuumReachesTheSetValue()const;
		bool getTMCavityRoughVacuumReachesTheSetValue(int value)const;

		//pm腔门
		bool getPMCavityDoorOpend(int number)const;
		void setPMCavityDoorOpend(int number, bool state);

		bool getVacuumEnable()const;
		void setVacuumEnable(const bool value);

		void setWithWaferModeEnable(const bool value);
		void setTMCavityVacuumUpperLimitAndExtractionValue(const double upper_limit_value, const double extraction_value);
		void setRoughVacuumValue(const double rough_vacuum_set_value);
		void setTMCavityPIDValue(const double pid_value);
		bool writeTMCavityPIDValue(const double pid_value);
		void recardVacuum()const;

		//plc模式切换
		bool getPlcMode()const;
		void setPlcMode(bool mode);

		//AWC(在位)感应器
		bool getAwcPresentSensor(int index) const;
		void setAwcPresentSensor(int index,bool state);

		//抽真空延时
		void setVacuumPumpingDelayTime(double tiemValue);

		double getVacuumPumpingDelayTime();

	protected:
		virtual void onInitialize()throw(KernelException)override;
		virtual void onUnInitialize()override;
		virtual void onProcess()override;
		virtual void onConfigure(const std::shared_ptr<KernelConfiguration> & config)override;
	private:
		DECLARE_PRIVATE(FortrendTMCavitySubsystem)

	};

}

#endif
