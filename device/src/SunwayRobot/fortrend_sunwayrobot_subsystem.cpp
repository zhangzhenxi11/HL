/**
* @file        fortrend_SunwayRobot_subsystem.cpp
* @brief       Fortrend SunwayRobot
* @author      xielonghua
*/

// Library: Fortrend
// Package: SubSystem/SunwayRobot
#include <map>
#include <string>

#include "kernel/kernel.h"
#include "Kernel/kernel_log.h"
#include "kernel/kernel_configure.h"

#include "SunwayRobot/fortrend_sunwayrobot_subsystem.h" 
#include "SunwayRobot/fortrend_sunwayrobot_output_command.h"
#include "SunwayRobot/fortrend_sunwayrobot_reset_command.h"
#include "SunwayRobot/fortrend_sunwayrobot_update_command.h"
#include "SunwayRobot/fortrend_sunwayrobot_get_wafer_command.h"
#include "SunwayRobot/fortrend_sunwayrobot_put_wafer_command.h"
#include "SunwayRobot/fortrend_sunwayrobot_ready_get_wafer_command.h"
#include "SunwayRobot/fortrend_sunwayrobot_ready_put_wafer_command.h"
#include "SunwayRobot/fortrend_sunwayrobot_set_speed_command.h"
#include "SunwayRobot/fortrend_sunwayrobot_set_axis_z_speed_command.h"

#include "kernel/kernel_command_reject_exception.h"
#include "Poco/String.h"
#include "Poco/Format.h"
#include "Poco/StringTokenizer.h"
#include "Poco/NumberParser.h"
#include <chrono> 

namespace FC{
	/**
	* FortrendSunwayRobotSubsystemPrivate
	*/
	class FortrendSunwayRobotSubsystemPrivate{
	public:
		FortrendSunwayRobotSubsystemPrivate(FortrendSunwayRobotSubsystem*p);
		void setInput(uint8_t index, bool stat);
		void setObject(uint8_t arm_id, bool has);
	public:
		FortrendSunwayRobotSubsystem* p;
		std::vector<std::shared_ptr<FortrendStation>>  stations;
		std::map<unsigned int, std::string> armNames;
		std::map<uint8_t, bool> armHasObjectMap;
		std::map<std::string, std::string> armSafeMap;

		std::vector<int> arm_wafer_slot;
		FortrendSunwayRobotSubsystem::AWCRecordData awc_record_data[3];

		std::string LLACavitySafetySignalAddress = "";
		std::string LLBCavitySafetySignalAddress = "";
		std::string PM1CavitySafetySignalAddress = "";
		std::string PM2CavitySafetySignalAddress = "";
		std::string PM3CavitySafetySignalAddress = "";
		std::string PM4CavitySafetySignalAddress = "";

		bool LLASafetySignal = false;
		bool LLBSafetySignal = false;
		bool PM1SafetySignal = false;
		bool PM2SafetySignal = false;
		bool PM3SafetySignal = false;
		bool PM4SafetySignal = false;

		bool vacuum_enable = false;  //真空模式
		bool with_wafer_mode = false;//晶圆模式
		bool has_reset_flag = false; //是否复位过的标记
		std::string status;
	};

	/**
	* FortrendSunwayRobotSubsystemPrivate
	*/
	FortrendSunwayRobotSubsystemPrivate::FortrendSunwayRobotSubsystemPrivate(FortrendSunwayRobotSubsystem*p)
		:p(p){
	}


	void FortrendSunwayRobotSubsystemPrivate::setObject(uint8_t arm_id, bool has){
		auto it = armHasObjectMap.find(arm_id);
		if (it == armHasObjectMap.end() || it->second != has){
			armHasObjectMap[arm_id] = has;
			p->emitSubsystemAttributeChanged();
		}
	}


	/**
	* FortrendSunwayRobotSubsystem
	*/
	FortrendSunwayRobotSubsystem::FortrendSunwayRobotSubsystem(IKernel*  kernel, const std::string& name)
		:WaferRobotAbstractSubsystem(kernel, name)
		, SunwaySubSystemHelper(name)
		, KeyencePlcSubSystemHelper(name)
		, d(new FortrendSunwayRobotSubsystemPrivate(this)){
		//init 
		setObject(0, false);
		setObject(1, false);


	}

	void  FortrendSunwayRobotSubsystem::pause(){
		d->status = "pause";//暂停
		AbstractIOSubsystem::emitAttributeChanged(this);
		KernelAbstractSubSystem::pause();
	}
	void  FortrendSunwayRobotSubsystem::resume(){
		d->status = "normal";//正常
		AbstractIOSubsystem::emitAttributeChanged(this);
		KernelAbstractSubSystem::resume();
	}
	void  FortrendSunwayRobotSubsystem::abort(){
		d->status = "abort";//停止
		AbstractIOSubsystem::emitAttributeChanged(this);
		KernelAbstractSubSystem::abort();
	}
	std::string FortrendSunwayRobotSubsystem::getRunningStatus()const{
		return d->status;
	}

	int FortrendSunwayRobotSubsystem::inputCount()const{
		return 0;
	};
	int FortrendSunwayRobotSubsystem::outputCount()const{
		return 0;
	};
	bool FortrendSunwayRobotSubsystem::getInput(int index)const{
		return false;
	};

	bool FortrendSunwayRobotSubsystem::hasObject(unsigned int arm_id) const{
		auto it = d->armHasObjectMap.find(arm_id);
		if (it == d->armHasObjectMap.end()) return false;
		return  it->second;
	}

	void FortrendSunwayRobotSubsystem::setObject(unsigned int arm_id, bool has){
		d->setObject(arm_id, has);
	}

	FortrendSunwayRobotSubsystem::AWCRecordData FortrendSunwayRobotSubsystem::getAWCRecordData(const int channel) const{
		
		if (channel > 3)
		{
			AWCRecordData data;
			return data;
		}
		return d->awc_record_data[channel];
	}
	void FortrendSunwayRobotSubsystem::setAWCRecordData(const int channel, const FortrendSunwayRobotSubsystem::AWCRecordData value){
		if (channel > 3)
		{
			return ;
		}
		d->awc_record_data[channel] = value;
		KernelAbstractSubSystem::emitAttributeChanged(this);
	}

	void FortrendSunwayRobotSubsystem::setVacuumEnable(const bool value){
		if (d->vacuum_enable != value)
		{
			d->vacuum_enable = value;
		}
	}

	bool FortrendSunwayRobotSubsystem::getWithWaferModeEnable()const{
		return d->with_wafer_mode;
	}
	void FortrendSunwayRobotSubsystem::setWithWaferModeEnable(const bool value){
		if (d->with_wafer_mode != value)
		{
			d->with_wafer_mode = value;
		}
	}

	int FortrendSunwayRobotSubsystem::getWaferSlot(const int arm)const{
		if (arm > d->arm_wafer_slot.size())
		{
			return 0;
		}
		return d->arm_wafer_slot[arm];
	}
	void FortrendSunwayRobotSubsystem::setWaferSlot(const int arm, const int slot){
		if (arm > d->arm_wafer_slot.size())
		{
			d->arm_wafer_slot[arm] = 0;
		}
		else
		{
			d->arm_wafer_slot[arm] = slot;
		}
	}

	bool FortrendSunwayRobotSubsystem::getHasResetFlag()const{
		return d->has_reset_flag;
	}
	void FortrendSunwayRobotSubsystem::setHasResetFlag(const bool value){
		d->has_reset_flag = value;
	}

	bool FortrendSunwayRobotSubsystem::getSafeSignalInPlace(const std::string& subsystem)
	{
		bool is_safe_inplace = false;
		int current_count = 0;
		int try_count = 10;

		auto it = d->armSafeMap.find(subsystem); //pair的迭代器
		if (it != d->armSafeMap.end())
		{
			while (current_count < try_count)
			{
				std::string safeSignal_address = it->second;

				logInform("WTR", "读:%s机械手安全地址", safeSignal_address.c_str());

				if (!KeyencePlcSubSystemHelper::readBit(safeSignal_address, is_safe_inplace)) {

					logError("WTR", "读:%s机械手安全地址失败", (it->second).c_str());
				}
				else
				{
					break;
				}
				current_count++;
				
				Sleep(500);
			}
			return is_safe_inplace;
		}
		logError("WTR", "找：%s机械手安全地址失败", it->second);
		return false;
	}

	void FortrendSunwayRobotSubsystem::setSafeSignalInPlace(const std::string& subsystem, bool status)
	{
		auto it = d->armSafeMap.find(subsystem);
		if (it != d->armSafeMap.end())
		{
			if (!writeBit(it->second, status))
			{
				logError("WTR", "写:%s机械手安全地址失败", it->second);
			}		
		}
	}

	void FortrendSunwayRobotSubsystem::emitStationAttributeChanged(){
		FortrendAbstractStation::emitAttributeChanged(this);
	}

	void FortrendSunwayRobotSubsystem::emitSubsystemAttributeChanged(){
		AbstractIOSubsystem::emitAttributeChanged(this);
	}

	void FortrendSunwayRobotSubsystem::onInitialize()throw(KernelException){
		try{
			SunwaySubSystemHelper::enableProtocol();
			setState(IKernelSubSystem::State::SUB_IDEL);
		}
		catch (KernelException& e){
			
			logError(getName().c_str(), e.what());;
			//throw e;
		}
	}

	void FortrendSunwayRobotSubsystem::onUnInitialize()throw(KernelException){
		SunwaySubSystemHelper::disableProtocol();
		KeyencePlcSubSystemHelper::disableProtocol();
	}

	void FortrendSunwayRobotSubsystem::onProcess(){
		// pollProtocol();
		AbstractIOSubsystem::emitAttributeChanged(this);
	}


	void FortrendSunwayRobotSubsystem::onConfigure(const std::shared_ptr<KernelConfiguration> & config){

		try{
			WaferRobotAbstractSubsystem::onConfigure(config);
			configSunway(config);
			configKeyencePlc(config);
			if (config->has("Stations"))
			{
				//load work stations
				do{
					Poco::StringTokenizer token(config->getString("Stations", ""), ",", Poco::StringTokenizer::TOK_IGNORE_EMPTY);
					for (int i = 0; i < token.count(); i++){
						std::string name = Poco::trim(token[i]);
						auto station = getKernel()->getKernelModule<FortrendStation>(name);
						if (station){
							d->stations.push_back(station);
						}
						else{
							throw KernelSysException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, Poco::format("station: %s not exist.", name));
						}
					}
				} while (false);
			}

			if (config->has("Arms"))
			{
				//load arm name
				do{
					Poco::StringTokenizer token(config->getString("Arms", ""), ",", Poco::StringTokenizer::TOK_IGNORE_EMPTY);
					for (int i = 0; i < token.count(); i++){
						std::string name = Poco::trim(token[i]);
						d->armNames[i] = name;
						d->arm_wafer_slot.push_back(0);
					}
				} while (false);
			}

			if (config->has("Update"))
			{
				d->LLACavitySafetySignalAddress = config->getString("Update.LLACavitySafetySignal","");
				d->LLBCavitySafetySignalAddress = config->getString("Update.LLBCavitySafetySignal","");
				d->PM1CavitySafetySignalAddress = config->getString("Update.PM1CavitySafetySignal","");
				d->PM2CavitySafetySignalAddress = config->getString("Update.PM2CavitySafetySignal", "");
				d->PM3CavitySafetySignalAddress = config->getString("Update.PM3CavitySafetySignal", "");
				d->PM4CavitySafetySignalAddress = config->getString("Update.PM4CavitySafetySignal", "");

				std::pair<std::string, std::string> LLAvalue("LLA", d->LLACavitySafetySignalAddress);
				std::pair<std::string, std::string> LLBvalue("LLB", d->LLBCavitySafetySignalAddress);
				std::pair<std::string, std::string> PM1value("PM1", d->PM1CavitySafetySignalAddress);
				std::pair<std::string, std::string> PM2value("PM2", d->PM2CavitySafetySignalAddress);
				std::pair<std::string, std::string> PM3value("PM3", d->PM3CavitySafetySignalAddress);
				std::pair<std::string, std::string> PM4value("PM4", d->PM4CavitySafetySignalAddress);

				d->armSafeMap.insert(LLAvalue);
				d->armSafeMap.insert(LLBvalue);
				d->armSafeMap.insert(PM1value);
				d->armSafeMap.insert(PM2value);
				d->armSafeMap.insert(PM3value);
				d->armSafeMap.insert(PM4value);
			}



		}
		catch (KernelException& e){
			logError(getName().c_str(), "%s onConfigure error %s", getName(), e.what());
		}
		catch (std::exception& e) {
			logError(getName().c_str(), "%s onConfigure error %s", getName(), e.what());
		}
		catch (...) {
			logError(getName().c_str(), "Unknown exception occurred.");
		}

	}


	std::shared_ptr<KernelSubsystemResetCommand> FortrendSunwayRobotSubsystem::createResetCommand()const{
		FortrendSunwayRobotSubsystem* self = const_cast<FortrendSunwayRobotSubsystem*>(this);
		KernelSubsystemResetCommand::Ptr ret(new SunwayRobotResetCommand(self));
		return ret;
	}

	std::shared_ptr<KernelSubsystemUpdateCommand> FortrendSunwayRobotSubsystem::createUpdateCommand()const{
		FortrendSunwayRobotSubsystem* self = const_cast<FortrendSunwayRobotSubsystem*>(this);
		KernelSubsystemUpdateCommand::Ptr ret(new SunwayRobotUpdateCommand(self));
		return ret;
	}

	std::shared_ptr<AbstractOutPutCommand>  FortrendSunwayRobotSubsystem::createOutputCommand(int channel, bool stat)const{
		FortrendSunwayRobotSubsystem* self = const_cast<FortrendSunwayRobotSubsystem*>(this);
		AbstractOutPutCommand::Ptr ret(new SunwayRobotOutputCommand(channel, stat, self));
		return ret;
	}

	//special commands

	std::shared_ptr<RobotAbstractGetWaferCommand>  FortrendSunwayRobotSubsystem::createGetCommand(const std::shared_ptr<FortrendStation>&  station, int arm, int slot)const{
		FortrendSunwayRobotSubsystem* self = const_cast<FortrendSunwayRobotSubsystem*>(this);
		RobotAbstractGetWaferCommand::Ptr ret(new SunwayRobotGetWaferCommand(station, arm, slot, self));
		return ret;
	}

	std::shared_ptr<RobotAbstractPutWaferCommand>  FortrendSunwayRobotSubsystem::createPutCommand(const std::shared_ptr<FortrendStation>&  station, int arm, int slot)const{
		FortrendSunwayRobotSubsystem* self = const_cast<FortrendSunwayRobotSubsystem*>(this);
		RobotAbstractPutWaferCommand::Ptr ret(new SunwayRobotPutWaferCommand(station, arm, slot, self));
		return ret;
	}

	std::shared_ptr<RobotAbstractReadyGetWaferCommand>  FortrendSunwayRobotSubsystem::createReadyGetCommand(const std::shared_ptr<FortrendStation>&  station, int arm, int slot)const{
		FortrendSunwayRobotSubsystem* self = const_cast<FortrendSunwayRobotSubsystem*>(this);
		RobotAbstractReadyGetWaferCommand::Ptr ret(new SunwayRobotReadyGetWaferCommand(station, arm, slot, self));
		return ret;
	}

	std::shared_ptr<RobotAbstractReadyPutWaferCommand>  FortrendSunwayRobotSubsystem::createReadyPutCommand(const std::shared_ptr<FortrendStation>&  station, int arm, int slot)const{
		FortrendSunwayRobotSubsystem* self = const_cast<FortrendSunwayRobotSubsystem*>(this);
		RobotAbstractReadyPutWaferCommand::Ptr ret(new SunwayRobotReadyPutWaferCommand(station, arm, slot, self));
		return ret;
	}

	std::shared_ptr<RobotAbstractSetSpeedCommand>  FortrendSunwayRobotSubsystem::createSetSpeedCommand(uint8_t percentage)const{
		FortrendSunwayRobotSubsystem* self = const_cast<FortrendSunwayRobotSubsystem*>(this);
		RobotAbstractSetSpeedCommand::Ptr ret(new SunwayRobotSetSpeedCommand(percentage, self));
		return ret;
	}

	std::shared_ptr<SunwayRobotCheckLoadCommand> FortrendSunwayRobotSubsystem::createCheckLoadCommand(int arm,int station_id)const{
		FortrendSunwayRobotSubsystem* self = const_cast<FortrendSunwayRobotSubsystem*>(this);
		SunwayRobotCheckLoadCommand::Ptr ret(new SunwayRobotCheckLoadCommand(arm, station_id, self));
		return ret;
	}

	std::shared_ptr<SunwayRobotRQLoadCommand> FortrendSunwayRobotSubsystem::createRQLoadCommand(int arm)const{
		FortrendSunwayRobotSubsystem* self = const_cast<FortrendSunwayRobotSubsystem*>(this);
		SunwayRobotRQLoadCommand::Ptr ret(new SunwayRobotRQLoadCommand(arm, self));
		return ret;
	}

	std::shared_ptr<SunwayRobotClearErrorCommand> FortrendSunwayRobotSubsystem::createClearErrorCommand()const{
		FortrendSunwayRobotSubsystem* self = const_cast<FortrendSunwayRobotSubsystem*>(this);
		SunwayRobotClearErrorCommand::Ptr ret(new SunwayRobotClearErrorCommand(self));
		return ret;
	}
	std::shared_ptr<SunwayRobotSetAxisZSpeedCommand>  FortrendSunwayRobotSubsystem::createSetAxisZSpeedCommand(uint8_t percentage)const{
		FortrendSunwayRobotSubsystem* self = const_cast<FortrendSunwayRobotSubsystem*>(this);
		SunwayRobotSetAxisZSpeedCommand::Ptr ret(new SunwayRobotSetAxisZSpeedCommand(percentage, self));
		return ret;
	}
	std::shared_ptr<SunwayRobotHomeCommand> FortrendSunwayRobotSubsystem::createHomeCommand() const
	{
		FortrendSunwayRobotSubsystem* self = const_cast<FortrendSunwayRobotSubsystem*>(this);
		SunwayRobotHomeCommand::Ptr ret(new SunwayRobotHomeCommand(self));
		return ret;
	}
}
