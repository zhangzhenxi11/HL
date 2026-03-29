/**
* @file        fortrend_SunwayRobot_subsystem.h
* @brief       Fortrend SunwayRobot
* @author      xielonghua
*/

// Library: Fortrend
// Package: SubSystem/SunwayRobot


#ifndef _XLH_FORTREND_SUNWAYROBOT_SUBSYSTEM_INCLUDE_
#define _XLH_FORTREND_SUNWAYROBOT_SUBSYSTEM_INCLUDE_

#include "kernel/Fortrend/robot_wafer_abstract_subsystem.h"
#include "Kernel/Fortrend/robot_abstract_command.h"
#include "Kernel/Fortrend/robot_getwafer_abstract_command.h"
#include "Kernel/Fortrend/robot_putwafer_abstract_command.h"

#include "SunwayRobot/sunway_subsystem_helper.h"
#include "SunwayRobot/sunway_command_executer.h"
#include "SunwayRobot/fortrend_sunwayrobot_set_axis_z_speed_command.h"
#include "SunwayRobot/fortrend_sunwayrobot_home_command.h"
#include "SunwayRobot/fortrend_sunwayrobot_set_load_command.h"
//special commands
#include "fortrend_sunwayrobot_get_wafer_command.h"
#include "fortrend_sunwayrobot_check_load_command.h"
#include "fortrend_sunwayrobot_rq_load_command.h"
#include "fortrend_sunwayrobot_clear_error_command.h"
#include "KeyencePLC/keyence_plc_command_executer.h"
#include "KeyencePLC/keyence_plc_subsystem_helper.h"
//调试宏
#define WTR_SIM_MODE 1

namespace FC{

	/**
	 * @brief fortrend SunwayRobot subsystem class
	 *
	 */
	class  FortrendSunwayRobotSubsystem : public WaferRobotAbstractSubsystem, public SunwaySubSystemHelper, public KeyencePlcSubSystemHelper {
	public:
		DECLARE_PTR(FortrendSunwayRobotSubsystem)
		FortrendSunwayRobotSubsystem(IKernel*  kernel, const std::string& name);

		struct AWCRecordData
		{
			double R = 0.0;
			double T = 0.0;
		};

		//override for FortrendAbstractStation
		virtual bool hasBoxPlacement()const override { return true; }
		virtual bool hasBoxPresent()const override { return true; }
		virtual int inputCount()const  override;
		virtual int outputCount()const  override;
		virtual bool getInput(int index)const override;

		virtual bool hasObject(unsigned int arm_id)const override; //有无片子

		virtual void  pause() override;
		virtual void  resume() override;
		virtual void  abort() override;
		std::string getRunningStatus()const;

		virtual std::shared_ptr<KernelSubsystemResetCommand> createResetCommand()const override;
		virtual std::shared_ptr<KernelSubsystemUpdateCommand> createUpdateCommand() const override;
		virtual std::shared_ptr<AbstractOutPutCommand>  createOutputCommand(int channel, bool stat)const override;

	public:
		//special commands
		virtual std::shared_ptr<RobotAbstractReadyGetWaferCommand> createReadyGetCommand(const std::shared_ptr<FortrendStation>&  station, int arm, int slot)const override;
		virtual std::shared_ptr<RobotAbstractReadyPutWaferCommand> createReadyPutCommand(const std::shared_ptr<FortrendStation>&  station, int arm, int slot)const override;
		virtual std::shared_ptr<RobotAbstractGetWaferCommand> createGetCommand(const std::shared_ptr<FortrendStation>&  station, int arm, int slot)const override;
		virtual std::shared_ptr<RobotAbstractPutWaferCommand> createPutCommand(const std::shared_ptr<FortrendStation>&  station, int arm, int slot) const override;
		virtual std::shared_ptr<RobotAbstractSetSpeedCommand> createSetSpeedCommand(uint8_t percentage) const override;
		
		std::shared_ptr<SunwayRobotCheckLoadCommand> createCheckLoadCommand(int arm, int station_id)const; //回原后检测手臂有无晶圆
		std::shared_ptr<SunwayRobotRQLoadCommand> createRQLoadCommand(int arm)const;//获取晶圆记忆
		std::shared_ptr<SunwayRobotSetLoadCommand> createSetLoadCommand(int arm, int state)const;
		std::shared_ptr<SunwayRobotClearErrorCommand> createClearErrorCommand()const;
		std::shared_ptr<SunwayRobotSetAxisZSpeedCommand> createSetAxisZSpeedCommand(uint8_t percentage)const;
		std::shared_ptr<SunwayRobotHomeCommand> createHomeCommand()const;

	public:
		virtual void setObject(unsigned int arm_id, bool has);

		AWCRecordData getAWCRecordData() const;
		void setAWCRecordData(const AWCRecordData value);

		void setVacuumEnable(const bool value);
		bool getVacuumEnable()const;


		bool getWithWaferModeEnable()const;
		void setWithWaferModeEnable(const bool value);

		int getWaferSlot(const int arm)const;
		void setWaferSlot(const int arm, const int slot);

		bool getHasResetFlag()const;
		void setHasResetFlag(const bool value);

		//2026-1-23 设置机械手上有无晶圆
		bool getHasArmWafer(int arm)const;
		void setHasArmWafer(const int arm);

		//获取安全信号到位
		bool getSafeSignalInPlace(const std::string & subsystem);
		void setSafeSignalInPlace(const std::string& subsystem,bool status);
	public:
		std::mutex external_mtx;

		std::mutex robot_mutex; //全局变量
	protected:
		virtual void onInitialize()throw(KernelException)override;
		virtual void onUnInitialize()override;
		virtual void onProcess()override;
		virtual void onConfigure(const std::shared_ptr<KernelConfiguration> & config)override;
		//virtual void onMessage(const HexMessage::Ptr& message,bool inputChanged)override;

		void emitStationAttributeChanged();
		void emitSubsystemAttributeChanged();
	private:
		DECLARE_PRIVATE(FortrendSunwayRobotSubsystem)

	};

}

#endif