/**
* @file            fortrend_pm_cavity_subsystem.h
* @brief           Fortrend PMCavity
* @author		   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/PMCavity


#ifndef _XLH_FORTREND_PM_CAVITY_SUBSYSTEM_INCLUDE_
#define _XLH_FORTREND_PM_CAVITY_SUBSYSTEM_INCLUDE_ 

#include "kernel/Fortrend/abstract_io_subsystem.h"
#include "kernel/Fortrend/fortrend_abstract_station.h"
#include "Kernel/Fortrend/hex_subsystem_helper.h"

#include "PMCavity/fortrend_pm_cavity_defined.h"
#include "PMCavity/fortrend_pm_cavity_reset_command.h"
#include "PMCavity/fortrend_pm_cavity_open_tm_cavity_door_command.h"
#include "PMCavity/fortrend_pm_cavity_close_tm_cavity_door_command.h"
#include "PMCavity/fortrend_pm_cavity_get_finished_command.h"
#include "PMCavity/fortrend_pm_cavity_upload_finished_command.h"
#include "PMCavity/fortrend_pm_cavity_read_process_parameters_command.h"
#include "PMCavity/fortrend_pm_cavity_write_process_parameters_command.h"
#include "PMCavity/fortrend_pm_cavity_clear_state_command.h"
#include "PMCavity/fortrend_pm_cavity_inserting_plate_opening_controller_command.h"

#include  "PMCavity/fortrend_pm_cavity_to_get_station_command.h"
#include  "PMCavity/fortrend_pm_cavity_to_put_station_command.h"

#include "KeyencePLC/keyence_plc_command_executer.h"
#include "KeyencePLC/keyence_plc_subsystem_helper.h"
#include "InovancePLC/inovance_plc_command_executer.h"
#include "InovancePLC/inovance_plc_subsystem_helper.h"


namespace FC{


	/**
	* @brief fortrend pm_cavity
	*/
	class  FortrendPMCavitySubsystem :public AbstractIOSubsystem, public FortrendAbstractStation, public KeyencePlcSubSystemHelper, public InovancePlcSubSystemHelper{
	public:
		DECLARE_PTR(FortrendPMCavitySubsystem)
		FortrendPMCavitySubsystem(IKernel*  kernel, const std::string& name);

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
		virtual std::shared_ptr<AbstractOutPutCommand>  createOutputCommand(int channel, bool stat)const override;

	public:

		std::shared_ptr<PMCavityOpenTMCavityDoorCommand> createOpenTMCavityDoorCommand()const;
		std::shared_ptr<PMCavityCloseTMCavityDoorCommand> createCloseTMCavityDoorCommand()const;

		std::shared_ptr<PMCavityGetFinishedCommand> createGetFinishedCommand()const;
		std::shared_ptr<PMCavityUploadFinishedCommand> createUploadFinishedCommand()const;

		std::shared_ptr<PMCavityToGetStationCommand> createToGetStationCommand()const;
		std::shared_ptr<PMCavityToPutStationCommand> createToPutStationCommand(int stationid=1)const;

		std::shared_ptr<PMCavityReadProcessParametersCommand> createReadProcessParametersCommand()const;
		std::shared_ptr<PMCavityWriteProcessParametersCommand> createWriteProcessParametersCommand(const PMCavityProcessParameters process_parame)const;
		std::shared_ptr<PMCavityClearStateCommand> createClearStateCommand()const;
		std::shared_ptr<PMCavityInsertingPlateOpeningControllerCommand> createInsertingPlateOpeningControllerCommand(const float percentage) const;

	public:
		void setDoorOpen(const bool value);
		//add 
		bool getPMCavityDoorrOpend()const;

		double getVacuumValue()const;
		double getTemperatureValue()const;
		void setVacuumEnable(const bool value);
		void setWithWaferModeEnable(const bool value);

		bool getVacuumValueReachesTheSetValue() const;
		bool getExhaustVacuumValueReachesTheSetValue() const;
		bool getTemperatureValueReachesTheSetValue() const;
		
		PMCavityMoniterValue getPMCavityMonityValue()const;

		bool getIsRunning();
		void setIsRunning(bool running);
		int getWaferSlot()const;
		void setWaferSlot(const int slot);

		bool getPMCavityGetRequest();
		bool getPMCavityUploadRequest();
		bool getPMCavitySafeSignal();
		void setPMCavitySafeSignal(bool value);
		bool getPMCavityMotorHomeSignal();
		bool getPMCavityMotorForwardSignal();
		bool getPMCavityMotorRunSignal();
		bool getPMCavityRemoteMode();
		//
		bool getPMCavityEnable()const;
		void setPMCavityEnable(const bool value);

		bool readPMCavityHasObjectState();
		bool writePMCavityHasObjectState(const bool value);
		bool getPMCavityHasAlarm();
		std::string getPMCavityAlarmMessage();

		bool getPMCavityUpdateProcessParameters();
		PMCavityProcessParameters getPMCavityProcessParameters();
		PMCavityMoniterFilmValue getPMCavityFilmValue();
		void setPMCavityProcessParameters(const PMCavityProcessParameters pm_param);

		/**
		* 获取镀膜时间
		*/
		float getPMCavityCoatingTime();
		/**
		* 获取工艺步骤
		*/
		short getPMCavityProcessingStep();

		double getPMCavityMagnitude()const;
		double getPMCavityAxleSpeed()const;
		double getPMCavityMotorSpeed()const;
		double getPMCavityAxleLocation();
		void setPMCavityAxleSpeed(double speed);
		void setPMCavityTurnSpeed(double speed);
		void setPMCavityForward(bool forward);
		void setPMCavityBackward(bool backward);
		int  getPMCavityCrftCountLLA()const;
		int  getPMCavityCrftCountLLB()const;
		void setPMCavityCrftCountLLA(int count);
		void setPMCavityCrftCountLLB(int count);
		void setVacuumSettingAndMagnitudeValue(const double setting_value, const double magnitude_value);

		void recardVacuum() const;


	protected:
		virtual void onInitialize()throw(KernelException)override;
		virtual void onUnInitialize()override;
		virtual void onProcess()override;
		virtual void onConfigure(const std::shared_ptr<KernelConfiguration> & config)override;
	private:
		DECLARE_PRIVATE(FortrendPMCavitySubsystem)

	};

}

#endif
