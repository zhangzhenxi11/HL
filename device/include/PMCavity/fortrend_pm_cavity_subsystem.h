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
#include "PMCavity/fortrend_pm_cavity_lifting_axis_home_command.h"
#include "PMCavity/fortrend_pm_cavity_rotating_axis_home_command.h"

#include "PMCavity/fortrend_pm_cavity_to_get_station_command.h"
#include "PMCavity/fortrend_pm_cavity_to_put_station_command.h"
#include "PMCavity/fortrend_pm_cavity_to_rotating_station_command.h"
#include "PMCavity/fortrend_pm_cavity_rotating_action_command.h"
#include "PMCavity/fortrend_pm_cavity_lifting_action_command.h"

#include "KeyencePLC/keyence_plc_command_executer.h"
#include "KeyencePLC/keyence_plc_subsystem_helper.h"
#include "InovancePLC/inovance_plc_command_executer.h"
#include "InovancePLC/inovance_plc_subsystem_helper.h"




namespace FC{


	/**
	* @brief fortrend pm_cavity
	*/
	class  FortrendPMCavitySubsystem :public AbstractIOSubsystem, public FortrendAbstractStation, public KeyencePlcSubSystemHelper{
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

		std::shared_ptr<PMCavityGetFinishedCommand> createGetFinishedCommand()const; //取片完成
		std::shared_ptr<PMCavityUploadFinishedCommand> createUploadFinishedCommand()const; //上片完成

		//升降轴动作
		std::shared_ptr<PMCavityLiftingActionCommand> createLiftingActionCommand(double targetPos)const;//自动命令到目标位
		std::shared_ptr<PMCavityToGetStationCommand> createToGetStationCommand()const;					//去取料位命令  位置1
		std::shared_ptr<PMCavityToRotatingStationCommand> createToRotatingStationCommand()const;	    //去旋转位命令  位置2
		std::shared_ptr<PMCavityToPutStationCommand> createToPutStationCommand()const;					//去工艺位命令  位置3
		

		//旋转轴动作  model 1:相对位置移动  model 2:绝对位置移动
		std::shared_ptr<PMCavityRotatingActionCommand> createRotatingActionCommand(double degree,int model = 1)const;

		std::shared_ptr<PMCavityReadProcessParametersCommand> createReadProcessParametersCommand()const;
		std::shared_ptr<PMCavityWriteProcessParametersCommand> createWriteProcessParametersCommand(const PMCavityAxisSettingParameters axis_parames)const;
		std::shared_ptr<PMCavityClearStateCommand> createClearStateCommand()const;
		std::shared_ptr<PMCavityInsertingPlateOpeningControllerCommand> createInsertingPlateOpeningControllerCommand(const float percentage) const;

		std::shared_ptr<PMCavityLiftingAxisHomeCommand> createPMCavityLiftingAxisHomeCommand()const;
		std::shared_ptr<PMCavityRotatingAxisHomeCommand> createPMCavityRotatingAxisHomeCommand()const;
	public:
		void setDoorOpen(const bool value);
		bool getPMCavityDoorrOpend()const;
		double getVacuumValue()const;
		double getTemperatureValue()const;
		void setVacuumEnable(const bool value);
		bool getVacuumEnable()const;

		bool getWithWaferModeEnable()const;
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

		//PM腔安全信号+_门阀开启to机械手 互锁
		bool getPMCavitySafeSignal();
		void setPMCavitySafeSignal(bool value);

		bool getPMCavityMotorHomeSignal(); 
		bool getPMCavityMotorForwardSignal();

		bool getPMCavityMotorRunSignal();
		bool getPMCavityRemoteMode();

		//Pm使能
		bool getPMCavityEnable()const;
		void setPMCavityEnable(const bool value);

		bool readPMCavityHasObjectState();
		bool writePMCavityHasObjectState(const bool value);
		bool getPMCavityHasAlarm();
		std::string getPMCavityAlarmMessage();

		bool getPMCavityUpdateProcessParameters();

		//PM腔工艺参数
		PMCavityProcessParameters getPMCavityProcessParameters();

		//膜厚仪监控参数值
		PMCavityMoniterFilmValue getPMCavityFilmValue();

		void setPMCavityProcessParameters(const PMCavityProcessParameters pm_param);

		//设置轴参数
		void setPMCavityAxisParameters(const PMCavityAxisSettingParameters pm_param);
		PMCavityAxisSettingParameters getPMCavityAxisParameters();

		bool getPMCavityUpdatAxisParameters();

		//最低面位检测
		bool getMinimumPlaneLevelSignal()const;
		
		//最高面位检测
		bool getMaximumPlaneLevelSignal()const;
		
		//中间旋转面位检测
		bool getRotatingimumPlaneLevelSignal()const;

		//顶升销面位检测
		bool getLiftingPinPlaneLevelSignal()const;

		//z轴定位坐标
		float getPmLiftingTargetPos()const;
		//r轴定位坐标
		float getPmRotatingTargetPos()const;

		//2026-4-23
		float getPmLiftPinSafeAnglePos()const;

		/**
		* 获取镀膜时间
		*/
		float getPMCavityCoatingTime();
		/**
		* 获取工艺步骤
		*/
		short getPMCavityProcessingStep();

		//工艺次数
		void setPMCavityCraftCount(int count);

		int getPMCavityCraftCount();
		//Z轴控使能
		bool getLiftingAxisPowerDone()const;
		void setLiftingAxisPower(bool enable);

		//Z轴回原
		bool getLiftingHomeDone()const;
		void setLiftingHome(bool enable);

		//Z轴控报警
		bool getZAxisAlarm() const;

		//R轴控报警
		bool getRAxisAlarm() const;

		//清除Z轴控报警
		bool getZAxisClearErrorDone()const;
		void setZAxisClearError(bool enable);

		//清除R轴控报警
		bool getRAxisClearErrorDone()const;
		void setRAxisClearError(bool enable);

		//R轴控使能
		bool getRotationAxisPowerDone()const;
		void setRotationAxisPower(bool enable);

		//R轴回原
		bool getRotationHomeDone()const;
		void setRotationHome(bool enable);

		/*=================Z轴========================*/
		//Z轴速度
		float getPMCavityZAxleSpeed()const;
		//Z轴位置
		float getPMCavityZAxleLocation()const;

		//设置Z轴加速度
		void setPMCavityZAxleAcc(float accValue);

		//设置Z轴减速度
		void setPMCavityZAxleDcc(float dccValue);

		//Z轴加速度
		float getPMCavityZAxleAcc()const;
		//Z轴减速度
		float getPMCavityZAxleDcc()const;

		//Z轴jerk 加加速度
		uint32_t getPMCavityZAxleJerk() const;

		void setPMCavityZAxleJerk(uint32_t value);

		/*=================R轴========================*/
		//R轴速度
		double getPMCavityRAxleSpeed()const;
		//R轴位置
		double getPMCavityRAxleLocation()const;

		//R轴加速度
		float getPMCavityRAxleAcc()const;
		//R轴减速度
		float getPMCavityRAxleDcc()const;

		//设置R轴加速度
		void setPMCavityRAxleAcc(float accValue);
		//设置R轴减速度
		void setPMCavityRAxleDcc(float dccValue);

		//R轴jerk
		uint32_t getPMCavityZRxleJerk() const;
		
		void setPMCavityRAxleJerk(uint32_t value);

		//JOG运行中
		bool getZAxleJogRunning()const; 

		//Z轴移动结束  
		bool getZAxleAutoRunDone()const;
		//Z轴移动中
		bool getZAxleAutoRunning()const;

		//R轴停止完成
		bool getRAxleStopDone() const;

		//JOG运行中
		bool getRAxleJogRunning()const;

		//R轴移动结束  
		bool getRAxleAutoDone()const;
		//R轴移动中
		bool getRAxleAutoRunning()const;

		double getPMCavityMagnitude()const;

		float getPMCavityMotorSpeed()const; //Z轴jog速度

		float getPMCavityRAxisMotorSpeed()const; //R轴jog速度

		double getPMCavityAxleSpeed()const; //z轴速度  -->废弃
		
		double getPMCavityAxleLocation();//z轴位置-->废弃

		//Z轴运行速度
		void setPMCavityAxleSpeed(float speed);
		//Z轴jog速度
		void setPMCavityTurnSpeed(float speed);

		//R轴运行速度
		void setPMCavityRAxleSpeed(float speed);
		//R轴jog速度
		void setsetPMCavityRAxleTurnSpeed(float speed);

		//Z JOG+
		void setPMCavityForward(bool forward);
		//Z JOG-
		void setPMCavityBackward(bool backward);

		//R JOG+
		void setPMCavityRAxlePositive(bool Positive);
		//R JOG-
		void setPMCavityRAxleNegative(bool Negative);

		int  getPMCavityCrftCountLLA()const;
		int  getPMCavityCrftCountLLB()const;

		void setPMCavityCrftCountLLA(int count);
		void setPMCavityCrftCountLLB(int count);

		void setVacuumSettingAndMagnitudeValue(const double setting_value, const double magnitude_value);

		void recardVacuum() const;

		bool safe_read_bit(const std::string& tag, bool& output);
		bool safe_read_float(const std::string& tag, float& output);
		bool safe_read_double(const std::string& tag, double& output);
		bool safe_read_int(const std::string&tag,int&output);
		bool safe_read_short(const std::string &tag, short&output);
		bool safe_read_unsignedInt(const std::string& tag, uint32_t& output);
		
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
