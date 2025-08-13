
/**
* @file            fortrend_tm_cavity_subsystem.h
* @brief           Fortrend 
* @author		   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/TMCavity

#include <iostream>
#include <Windows.h>

#include "Poco/Format.h"
#include "Poco/StringTokenizer.h"
#include "Poco/NumberParser.h"

#include "kernel/kernel.h"
#include "Kernel/kernel_log.h"
#include "kernel/kernel_iocontrol.h"
#include "kernel/kernel_configure.h"
#include "TMCavity/fortrend_tm_cavity_subsystem.h" 
#include "TMCavity/fortrend_tm_cavity_reset_command.h"
#include "TMCavity/fortrend_tm_cavity_update_command.h"
#include "TMCavity/fortrend_tm_cavity_output_command.h"



#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

namespace FC{
	/**
	* FortrendSubsystemPrivate
	*/
	class FortrendTMCavitySubsystemPrivate{
	public:
		FortrendTMCavitySubsystemPrivate(FortrendTMCavitySubsystem*p);
		void setInput(uint8_t index, bool stat);
	public:
		FortrendTMCavitySubsystem* p;
		std::string io_input_address = "";

		std::string tm_cavity_vacuum_read_value_address = "";       //传输腔真空值地址
		double tm_cavity_vacuum_current_value = 100000.0;	        //传输腔当前真空值
		double tm_cavity_vacuum_extraction_set_value = 0.005;       //传输腔抽真空设定值
		double tm_cavity_vacuum_upper_limit_set_value = 0.005;      //传输腔上限值
		double tm_cavity_pid_set_value = 0.005;						//传输腔PID设定值
		double tm_cavity_vacuum_break_set_value = 100000.0;         //传输腔破气设定值

		std::string tm_cavity_vacuum_pressure_gage_address = "";    //传输腔真空压力表信号地址
		int tm_cavity_vacuum_pressure_gage_state = -1;				//传输腔真空压力表信号

		std::string tm_cavity_cover_safety_lock_address = "";		//传输腔腔盖安全锁地址
		std::string tm_cavity_pid_set_value_address = "";			//传输腔腔盖安全锁地址

		std::string molecule_pipeline_vacuum_read_value_address = "";  //分子泵管路真空值地址
		double molecule_pipeline_vacuum_current_value = 100000.0;	   //分子泵管路当前真空值

		std::string backing_pipeline_vacuum_read_value_address = "";   //前级泵管路真空值地址
		double backing_pipeline_vacuum_current_value = 100000.0;	   //前级泵管路当前真空值

		std::string Axial_pressure_gage_address = "";                  //轴向压力表-1 NL/真空
		bool axial_pressure_gage_state = false;                            //轴向压力表信号

		std::string CDA_pressure_address = "";                         //总气源空压三联件信号
		bool cda_signal_state = false;
		
		double rough_vacuum_set_value = 6;


		std::string diaphragm_valve_address1 = "";
		std::string diaphragm_valve_address2 = "";
		std::string high_vacuum_baffle_value_address = "";
		std::string open_angle_valve_address= "";
		std::string close_angle_valve_address = "";
		std::string open_inserting_plate_valve_address = "";
		std::string close_inserting_plate_valve_address = "";
		bool pm1_cavity_door_opend = false;
		std::string  pm1_cavity_door_opend_address = "";

		bool pm2_cavity_door_opend = false;
		std::string  pm2_cavity_door_opend_address = "";

		bool pm3_cavity_door_opend = false;
		std::string  pm3_cavity_door_opend_address = "";

		bool pm4_cavity_door_opend = false;
		std::string  pm4_cavity_door_opend_address = "";
		
		std::map<int, bool> pm_cavity_door_Opend; //PM腔门开关
		std::map<int, bool> awc_present_siganlMap; //AWC(在位)感应器

		unsigned short io_input_count = 0;
		std::vector<bool> io_input_last_value;
		//bool *ptr_io_input_state;
		std::unique_ptr<bool[]> ptr_io_input_state;
		
		bool slow_diaphragm_valve_opend = false;
		bool fast_diaphragm_valve_opend = false;
		bool ultrahigh_vacuum_baffle_valve_opend = false;
		bool inserting_plate_valve_opend = false;
		bool angle_valve_opend = false;
		bool pid_opend = false;
		bool tmCoverSafetyLock = false;

		bool vacuum_enable = false;
		bool with_wafer_mode = false;

		bool recard_enabled = false;
		std::thread thd_recard_vacuum;

		std::string plc_mode_address;
		int plc_mode_value;

	};

	/**
	* FortrendSubsystemPrivate
	*/
	FortrendTMCavitySubsystemPrivate::FortrendTMCavitySubsystemPrivate(FortrendTMCavitySubsystem*p)
		:p(p){

	}

	/**
	* FortrendSubsystem
	*/
	FortrendTMCavitySubsystem::FortrendTMCavitySubsystem(IKernel*  kernel, const std::string& name)
		:AbstractIOSubsystem(kernel, name)
		, FortrendAbstractStation(kernel)
		, KeyencePlcSubSystemHelper(name)
		, d(new FortrendTMCavitySubsystemPrivate(this)){
		//init 
		d->recard_enabled = true;
		d->thd_recard_vacuum = std::thread([this] { this->recardVacuum(); });
	}
	bool FortrendTMCavitySubsystem::hasBoxPlacement()const
	{
		return false;
	}

	bool FortrendTMCavitySubsystem::hasDoorOpend()const{
		return true;
	}

	int FortrendTMCavitySubsystem::inputCount()const
	{
		return helperInputCount();
	}

	int FortrendTMCavitySubsystem::outputCount()const
	{
		return helperOutputCount();
	}

	bool FortrendTMCavitySubsystem::getInput(int index)const
	{
		if (index < d->io_input_count)
		{
			return d->ptr_io_input_state[index];
		}
		return false;
	}

	std::string FortrendTMCavitySubsystem::getInputName(int index)const
	{
		return getHelperInputName(index);
	}

	std::string FortrendTMCavitySubsystem::getOutputName(int index)const
	{
		return getHelperOutputName(index);
	}

	bool FortrendTMCavitySubsystem::getHeightVacuumBaffleValveOpend()const{
		return d->ultrahigh_vacuum_baffle_valve_opend;
	}
	void FortrendTMCavitySubsystem::setHeightVacuumBaffleValveOpend(const bool value){
		if (d->ultrahigh_vacuum_baffle_valve_opend != value)
		{
			d->ultrahigh_vacuum_baffle_valve_opend = value;
			AbstractIOSubsystem::emitAttributeChanged(this);
		}
	}
	bool FortrendTMCavitySubsystem::getInsertingPlateValveOpend()const{
		return d->inserting_plate_valve_opend;
	}
	void FortrendTMCavitySubsystem::setInsertingPlateValveOpend(const bool value)
	{
		if (d->inserting_plate_valve_opend != value)
		{
			d->inserting_plate_valve_opend = value;
			AbstractIOSubsystem::emitAttributeChanged(this);
		}
	}

	bool FortrendTMCavitySubsystem::getSlowDiaphragmValveOpend()const{
		return d->slow_diaphragm_valve_opend ;
	}
	void FortrendTMCavitySubsystem::setSlowDiaphragmValveOpend(const bool value)
	{
		if (d->slow_diaphragm_valve_opend != value)
		{
			d->slow_diaphragm_valve_opend = value;
			AbstractIOSubsystem::emitAttributeChanged(this);
		}
	}
	bool FortrendTMCavitySubsystem::getFastDiaphragmValveOpend()const{
		return d->fast_diaphragm_valve_opend;
	}
	void FortrendTMCavitySubsystem::setFastDiaphragmValveOpend(const bool value)
	{
		if (d->fast_diaphragm_valve_opend != value)
		{
			d->fast_diaphragm_valve_opend = value;
			AbstractIOSubsystem::emitAttributeChanged(this);
		}
	}
	bool FortrendTMCavitySubsystem::getAngleValveOpend()const{
		return d->angle_valve_opend;
	}
	void FortrendTMCavitySubsystem::setAngleValveOpend(const bool value)
	{
		if (d->angle_valve_opend != value)
		{
			d->angle_valve_opend = value;
			AbstractIOSubsystem::emitAttributeChanged(this);
		}
	}



	bool FortrendTMCavitySubsystem::getPIDOpend() const{
		return d->pid_opend;
	}
	void FortrendTMCavitySubsystem::setPIDOpend(const bool value){
		if (d->pid_opend != value)
		{
			d->pid_opend = value;
			AbstractIOSubsystem::emitAttributeChanged(this);
		}

	}

	bool FortrendTMCavitySubsystem::getAxialPressureGageState() const
	{
		return d->axial_pressure_gage_state;
	}

	bool FortrendTMCavitySubsystem::getCDAPressureState() const
	{
		return d->cda_signal_state;
	}

	/*
	*传输腔腔盖门锁信号
	*/
	bool FortrendTMCavitySubsystem::TMCavityCoverSafetyLock(){
		return d->tmCoverSafetyLock;
	}

	double FortrendTMCavitySubsystem::getTMCavityVacuumValue()const{
		return d->tm_cavity_vacuum_current_value;
	}

	double FortrendTMCavitySubsystem::getMoleculePipelineVacuumValue() const{
		return d->molecule_pipeline_vacuum_current_value;
	}

	double FortrendTMCavitySubsystem::getBackingPipelineVacuumValue() const{
		return d->backing_pipeline_vacuum_current_value;
	}

	int FortrendTMCavitySubsystem::getTMCavityVacuumPressureGageState()const{
		return d->tm_cavity_vacuum_pressure_gage_state;
	}

	/*
	*达到粗抽压力
	*/
	bool FortrendTMCavitySubsystem::getTMCavityRoughVacuumReachesTheSetValue()const{
		return d->tm_cavity_vacuum_current_value < d->rough_vacuum_set_value;
	}

	/*
	*达到LoadLock粗抽压力,看分子泵工作条件判断，添加补偿值
	*/
	bool FortrendTMCavitySubsystem::getTMCavityRoughVacuumReachesTheSetValue(int value)const{
		return d->tm_cavity_vacuum_current_value < (d->rough_vacuum_set_value + value);
	}

	bool FortrendTMCavitySubsystem::getPMCavityDoorOpend(int number) const
	{
		bool value = false;
		auto select = d->pm_cavity_door_Opend.find(number);
		if (select != d->pm_cavity_door_Opend.end())
		{
			value = select->second;
		}
		return value;
	}

	void FortrendTMCavitySubsystem::setPMCavityDoorOpend(int number, bool state)
	{
		d->pm_cavity_door_Opend[number] = state;
		AbstractIOSubsystem::emitAttributeChanged(this);
	}

	/*
	*达到抽气设定值
	*/
	bool FortrendTMCavitySubsystem::getTMCavityVacuumValueReachesTheSetValue()const{
		if (d->tm_cavity_vacuum_current_value < d->tm_cavity_vacuum_extraction_set_value)
		{
			return true;
		}
		return false;
	}
	/*
	达到上限设定值
	*/
	bool FortrendTMCavitySubsystem::getTMCavityVacuumValueUpperLimitReachesTheSetValue()const{
		if (!d->vacuum_enable)
		{
			return true;
		}
		if (d->tm_cavity_vacuum_current_value < d->tm_cavity_vacuum_upper_limit_set_value)
		{
			return true;
		}
		//logInform1(getName().c_str(), Poco::format("当前压力值：%f,设定的上限压力值：%f", d->tm_cavity_vacuum_current_value, d->tm_cavity_vacuum_upper_limit_set_value).c_str());
		return false;
	}

	/*
	达到PID设定值
	*/
	bool FortrendTMCavitySubsystem::getTMCavityVacuumValueReachesThePIDSetValue() const{
		double upper_limit = d->tm_cavity_pid_set_value * 1.1;
		double lower_limit = d->tm_cavity_pid_set_value * 0.9;
		if ((d->tm_cavity_vacuum_current_value >= lower_limit) && (d->tm_cavity_vacuum_current_value <= upper_limit))
		{
			return true;
		}
		return false;
	}

	bool FortrendTMCavitySubsystem::getVacuumEnable()const{

		return d->vacuum_enable;
	}

	void FortrendTMCavitySubsystem::setVacuumEnable(const bool value){
		if (d->vacuum_enable != value)
		{
			d->vacuum_enable = value;
		}
	}
	void FortrendTMCavitySubsystem::setWithWaferModeEnable(const bool value){
		if (d->with_wafer_mode != value)
		{
			d->with_wafer_mode = value;
		}
	}



	void FortrendTMCavitySubsystem::onInitialize()throw(KernelException){
		
		if (SIM_MODE == 1)
		{
			setState(IKernelSubSystem::State::SUB_NORMAL);
		}
		else
		{
			try {
				if (enableProtocol())
					setState(IKernelSubSystem::State::SUB_IDEL);
				else
					setState(IKernelSubSystem::State::SUB_UNKNOWN);
			}
			catch (KernelException& e) {
				logError(getName().c_str(), e.what());
				//throw e;
			}
		}
	}

	void FortrendTMCavitySubsystem::onUnInitialize()throw(KernelException){
		d->recard_enabled = false;
		disableProtocol();
		if (d->thd_recard_vacuum.joinable())
		{
			d->thd_recard_vacuum.join();
		}
	}

	void FortrendTMCavitySubsystem::onProcess(){
		//pollProtocol();
		if (getState() != IKernelSubSystem::State::SUB_UNKNOWN )
		{

			bool plc_current_value = false;
			if (d->plc_mode_address != "" && readBit(d->plc_mode_address, plc_current_value))
			{
				if (d->plc_mode_value!= plc_current_value)
				{
					d->plc_mode_value = plc_current_value;
				}
			}


			bool io_changed = false;
			if ((d->io_input_count > 0) && readBits(d->io_input_address, d->io_input_count, d->ptr_io_input_state.get()))
			{
				
				for (size_t i = 0; i < d->io_input_count; i++)
				{
					if (d->ptr_io_input_state[i] != d->io_input_last_value[i])
					{
						d->io_input_last_value[i] = d->ptr_io_input_state[i];
						io_changed = true;
					}
				}
			}
			if (d->tm_cavity_vacuum_read_value_address != "")
			{
				double buff_vacuum = 0.0;
				if (readDouble(d->tm_cavity_vacuum_read_value_address, buff_vacuum) && d->tm_cavity_vacuum_current_value != buff_vacuum)
				{
					d->tm_cavity_vacuum_current_value = buff_vacuum;
					io_changed = true;
				}
			}

			//if (d->molecule_pipeline_vacuum_read_value_address != "")
			//{
			//	double buff_vacuum = 0.0;
			//	if (readDouble(d->molecule_pipeline_vacuum_read_value_address, buff_vacuum) && d->molecule_pipeline_vacuum_current_value != buff_vacuum)
			//	{
			//		d->molecule_pipeline_vacuum_current_value = buff_vacuum;
			//		io_changed = true;
			//	}
			//}
			//前级管道真空值
			if (d->backing_pipeline_vacuum_read_value_address != "")
			{
				double buff_vacuum = 0.0;
				if (readDouble(d->backing_pipeline_vacuum_read_value_address, buff_vacuum) && d->backing_pipeline_vacuum_current_value != buff_vacuum)
				{
					d->backing_pipeline_vacuum_current_value = buff_vacuum;
					io_changed = true;
				}
			}

#pragma region 更新PM门阀状态

			bool pm_flag = false;
			bool value = false;

			pm_flag = d->pm1_cavity_door_opend;
			value = 0;
			if (d->pm1_cavity_door_opend_address != "" && readBit(d->pm1_cavity_door_opend_address, value))//PM1门阀
			{
				if (pm_flag != value)
				{
					io_changed = true;
					d->pm1_cavity_door_opend = value;
					setPMCavityDoorOpend(1, value);
				}
			}

			pm_flag = d->pm2_cavity_door_opend;
			value = 0;
			if (d->pm2_cavity_door_opend_address != "" && readBit(d->pm2_cavity_door_opend_address, value))//PM2门阀
			{
				if (pm_flag != value)
				{
					io_changed = true;
					d->pm2_cavity_door_opend = value;
					setPMCavityDoorOpend(2, value);
				}
			}

			pm_flag = d->pm3_cavity_door_opend;
			value = 0;
			if (d->pm3_cavity_door_opend_address != "" && readBit(d->pm3_cavity_door_opend_address, value))//PM3门阀
			{
				if (pm_flag != value)
				{
					io_changed = true;
					d->pm3_cavity_door_opend = value;
					setPMCavityDoorOpend(3, value);
				}
			}
			pm_flag = d->pm4_cavity_door_opend;
			value = 0;
			if (d->pm4_cavity_door_opend_address != "" && readBit(d->pm4_cavity_door_opend_address, value))//PM4门阀
			{
				if (pm_flag != value)
				{
					io_changed = true;
					d->pm4_cavity_door_opend = value;
					setPMCavityDoorOpend(4, value);
				}
			}
#pragma endregion

#pragma region 自动更新门阀状态 
			bool flag = false;

			flag = d->tmCoverSafetyLock;
			if (readBit(d->tm_cavity_cover_safety_lock_address, d->tmCoverSafetyLock))
			{
				if (flag != d->tmCoverSafetyLock)
					io_changed = true;
			}
			
			flag = d->slow_diaphragm_valve_opend;
			if (d->diaphragm_valve_address1 != ""&&readBit(d->diaphragm_valve_address1, d->slow_diaphragm_valve_opend))
			{
				if (flag != d->slow_diaphragm_valve_opend)
					io_changed = true;
			}

			flag = d->fast_diaphragm_valve_opend;
			if (d->diaphragm_valve_address2 != ""&&readBit(d->diaphragm_valve_address2, d->fast_diaphragm_valve_opend))
			{
				if (flag != d->fast_diaphragm_valve_opend)
					io_changed = true;
			}

			flag = d->ultrahigh_vacuum_baffle_valve_opend;
			if (d->high_vacuum_baffle_value_address != ""&&readBit(d->high_vacuum_baffle_value_address, d->ultrahigh_vacuum_baffle_valve_opend))
			{
				if (flag != d->ultrahigh_vacuum_baffle_valve_opend)
					io_changed = true;
			}

			//bool open_angle_valve_value = false;
			//bool close_angle_valve_value = false;

			flag = d->angle_valve_opend;
			if (d->open_angle_valve_address != ""&&readBit(d->open_angle_valve_address, d->angle_valve_opend))
			{
				if (flag != d->angle_valve_opend)
					io_changed = true;
			}

			/*bool close_angle_valve_value = false; 冗余代码
			if (d->close_angle_valve_address != ""&&readBit(d->close_angle_valve_address, close_angle_valve_value))
			{
				if ((!d->angle_valve_opend && close_angle_valve_value))
				{
					setAngleValveOpend(false);
				}
			}*/

			//bool open_inserting_plate_valve_value = false;
			//bool close_inserting_plate_valve_value = false;

			flag = d->inserting_plate_valve_opend;
			if (d->open_inserting_plate_valve_address != "" && readBit(d->open_inserting_plate_valve_address, d->inserting_plate_valve_opend))
			{
				if (flag != d->inserting_plate_valve_opend)
					io_changed = true;
			}

			
#pragma endregion
			
			if (d->tm_cavity_vacuum_pressure_gage_address != "")
			{
				bool buff_vacuum_pressure_gage = false;
				int buff_vacuum_pressure_gage_state = -1;
				if (readBit(d->tm_cavity_vacuum_pressure_gage_address, buff_vacuum_pressure_gage))
				{
					buff_vacuum_pressure_gage_state = (buff_vacuum_pressure_gage)? 1:0;	//true:大气，false:真空
				}
				if (buff_vacuum_pressure_gage_state != d->tm_cavity_vacuum_pressure_gage_state)
				{
					d->tm_cavity_vacuum_pressure_gage_state = buff_vacuum_pressure_gage_state;
					io_changed = true;
				}
			}

			if (d->Axial_pressure_gage_address != "")
			{
				bool axial_signal = false;
				if (readBit(d->Axial_pressure_gage_address, axial_signal))
				{
					if (d->axial_pressure_gage_state != axial_signal)
					{
						d->axial_pressure_gage_state = axial_signal;
						io_changed = true;
					}
				}
			}
			
			if (d->CDA_pressure_address != "")
			{
				bool cda_signal = false;
				if (readBit(d->CDA_pressure_address, cda_signal))
				{
					if (d->cda_signal_state != cda_signal)
					{
						d->cda_signal_state = cda_signal;
						io_changed = true;
					}
				}
			}

			if (io_changed)
			{
				AbstractIOSubsystem::emitAttributeChanged(this);
			}
			Sleep(50);		
		}
	}

	//设置真空上限值和抽真空设定值
	void FortrendTMCavitySubsystem::setTMCavityVacuumUpperLimitAndExtractionValue(const double upper_limit_value, const double extraction_value){
		d->tm_cavity_vacuum_upper_limit_set_value = upper_limit_value;
		d->tm_cavity_vacuum_extraction_set_value = extraction_value;
		logInform(getName().c_str(), Poco::format("%s 设置真空上限值：%f, 抽真空设定值：%f", getName(), d->tm_cavity_vacuum_upper_limit_set_value, d->tm_cavity_vacuum_extraction_set_value).c_str());
	}
	/**
	*@brief  设置粗抽完成真空值
	*/
	void FortrendTMCavitySubsystem::setRoughVacuumValue(const double rough_vacuum_set_value){
		d->rough_vacuum_set_value = rough_vacuum_set_value;
		logInform(getName().c_str(), Poco::format("%s 设置粗抽完成真空设定值：%f", getName(),
			d->rough_vacuum_set_value).c_str());
	}
	void FortrendTMCavitySubsystem::setTMCavityPIDValue(const double pid_value){
		d->tm_cavity_pid_set_value = pid_value;
		if (writeTMCavityPIDValue(pid_value) == false)
		{
			logError(getName().c_str(), "PID设定值地址写入失败");
		}
		logInform(getName().c_str(), Poco::format("%s 设置PID设定值：%f", getName(), d->tm_cavity_pid_set_value).c_str());
	}
	bool FortrendTMCavitySubsystem::writeTMCavityPIDValue(const double pid_value){
		if (d->tm_cavity_pid_set_value_address != "")
		{
			return writeDouble(d->tm_cavity_pid_set_value_address, d->tm_cavity_pid_set_value);
		}
		logError(getName().c_str(), "PID设定值地址未设置");
		return false;
		
	}
	void FortrendTMCavitySubsystem::recardVacuum() const{
		int count = 0;
		while (d->recard_enabled)
		{
			Sleep(20);
			count++;
			if (count == 100)
			{
				count = 0;
				logInform1(Poco::format("%s_vacuum", getName()).c_str(), Poco::format("%f", getTMCavityVacuumValue()).c_str());
				logInform1("pipe_vacuum", Poco::format("%f", getMoleculePipelineVacuumValue()).c_str());
			}
			
			
		}
	}

	bool FortrendTMCavitySubsystem::getPlcMode() const
	{
		return d->plc_mode_value;
	}

	void FortrendTMCavitySubsystem::setPlcMode(bool mode)
	{
		d->plc_mode_value = mode;
	}

	bool FortrendTMCavitySubsystem::getAwcPresentSensor(int index) const
	{
		bool value = false;
		auto select = d->awc_present_siganlMap.find(index);
		if (select != d->awc_present_siganlMap.end())
		{
			value = select->second;
		}
		return value;
	}

	void FortrendTMCavitySubsystem::setAwcPresentSensor(int index, bool state)
	{
		d->awc_present_siganlMap[index] = state;

	}

	void FortrendTMCavitySubsystem::onConfigure(const std::shared_ptr<KernelConfiguration> & config){
		KernelAbstractSubSystem::onConfigure(config);
		FortrendAbstractStation::configure(config);
		configKeyencePlc(config);

		d->io_input_count = inputCount();
		for (size_t i = 0; i < d->io_input_count; i++)
		{
			d->io_input_last_value.push_back(false);
		}
		if (d->io_input_count>0)
		{
			//d->ptr_io_input_state = new bool[d->io_input_count];
			d->ptr_io_input_state = std::make_unique<bool[]>(d->io_input_count);
		}
		d->io_input_address = getHelperInputAddress();
		if (config->has("Vacuum"))
		{
			d->tm_cavity_vacuum_read_value_address = config->getString("Vacuum.TMCavityReadValueAddress", "");
			d->tm_cavity_vacuum_pressure_gage_address = config->getString("Vacuum.TMCavityPressureGageAddress", "");
			//d->vacuum_extraction_set_value = config->getDouble("Vacuum.TMCavityExtractionSetValue", 0.0);
			//d->vacuum_upper_limit_set_value = config->getDouble("Vacuum.TMCavityVacuumUpperLimitSetValue", 0.005);
			d->tm_cavity_vacuum_break_set_value = config->getDouble("Vacuum.TMCavityBreakSetValue", 0.0);

			d->molecule_pipeline_vacuum_read_value_address = config->getString("Vacuum.MoleculePipelineReadValueAddress", "");
			d->backing_pipeline_vacuum_read_value_address = config->getString("Vacuum.BackingPipelineReadValueAddress","");
			d->Axial_pressure_gage_address = config->getString("Vacuum.AxialPressureGageAddress","");

		}
		if (config->has("SignalAddress"))
		{
			d->tm_cavity_cover_safety_lock_address = config->getString("SignalAddress.CavityCoverSafetyLockAddress", "MR30505");
			d->tm_cavity_pid_set_value_address = config->getString("SignalAddress.PIDSetValueAddress", "DM400");
			d->CDA_pressure_address = config->getString("CdaPressureAddress","MR30506");
			
		}
		if (config->has("Update"))
		{
			d->plc_mode_address = config->getString("Update.PLC_Mode_address","");
			d->diaphragm_valve_address1 = config->getString("Update.diaphragm_valve_address1", "");//TM慢充隔膜阀
			d->diaphragm_valve_address2 = config->getString("Update.diaphragm_valve_address2", "");//TM快充隔膜阀
			d->high_vacuum_baffle_value_address = config->getString("Update.high_vacuum_baffle_value_address", "");
			d->open_angle_valve_address = config->getString("Update.open_angle_valve_address", "");
			d->close_angle_valve_address = config->getString("Update.close_angle_valve_address", "");
			d->open_inserting_plate_valve_address = config->getString("Update.open_inserting_plate_valve_address", "");
			d->close_inserting_plate_valve_address = config->getString("Update.close_inserting_plate_valve_address", "");
			//打开完成信号
			d->pm1_cavity_door_opend_address = config->getString("Update.pm1_cavity_door_address","");
			d->pm2_cavity_door_opend_address = config->getString("Update.pm2_cavity_door_address", "");
			d->pm3_cavity_door_opend_address = config->getString("Update.pm3_cavity_door_address", "");
			d->pm4_cavity_door_opend_address = config->getString("Update.pm4_cavity_door_address", "");
		}
		
	}


	std::shared_ptr<KernelSubsystemResetCommand> FortrendTMCavitySubsystem::createResetCommand()const{
		FortrendTMCavitySubsystem* self = const_cast<FortrendTMCavitySubsystem*>(this);
		TMCavityResetCommand::Ptr ret(new TMCavityResetCommand(self));
		return ret;
	}

	std::shared_ptr<KernelSubsystemUpdateCommand> FortrendTMCavitySubsystem::createUpdateCommand()const{
		FortrendTMCavitySubsystem* self = const_cast<FortrendTMCavitySubsystem*>(this);
		TMCavityUpdateCommand::Ptr ret(new TMCavityUpdateCommand(self));
		return ret;
	}

	std::shared_ptr<AbstractOutPutCommand>  FortrendTMCavitySubsystem::createOutputCommand(int channel, bool state)const{
		FortrendTMCavitySubsystem* self = const_cast<FortrendTMCavitySubsystem*>(this);
		AbstractOutPutCommand::Ptr ret(new TMCavityOutputCommand(channel, state, self));
		return ret;
	}

	//隔膜阀
	std::shared_ptr<TMCavityOpenDiaphragmValveCommand> FortrendTMCavitySubsystem::createOpenDiaphragmValveCommand(const TMCavityValveOpening opening)const{
		FortrendTMCavitySubsystem* self = const_cast<FortrendTMCavitySubsystem*>(this);
		TMCavityOpenDiaphragmValveCommand::Ptr ret(new TMCavityOpenDiaphragmValveCommand(opening, self));
		return ret;
	}

	std::shared_ptr<TMCavityCloseDiaphragmValveCommand>  FortrendTMCavitySubsystem::createCloseDiaphragmValveCommand(const TMCavityValveOpening opening)const{
		FortrendTMCavitySubsystem* self = const_cast<FortrendTMCavitySubsystem*>(this);
		TMCavityCloseDiaphragmValveCommand::Ptr ret(new TMCavityCloseDiaphragmValveCommand(opening, self));
		return ret;
	}

	//插板阀
	std::shared_ptr<TMCavityOpenInsertingPlateValveCommand> FortrendTMCavitySubsystem::createOpenInsertingPlateValveCommand()const{
		FortrendTMCavitySubsystem* self = const_cast<FortrendTMCavitySubsystem*>(this);
		TMCavityOpenInsertingPlateValveCommand::Ptr ret(new TMCavityOpenInsertingPlateValveCommand(self));
		return ret;
	}

	std::shared_ptr<TMCavityCloseInsertingPlateValveCommand> FortrendTMCavitySubsystem::createCloseInsertingPlateValveCommand()const{
		FortrendTMCavitySubsystem* self = const_cast<FortrendTMCavitySubsystem*>(this);
		TMCavityCloseInsertingPlateValveCommand::Ptr ret(new TMCavityCloseInsertingPlateValveCommand(self));
		return ret;
	}
	//挡板阀
	std::shared_ptr<TMCavityOpenHeightVacuumBaffleValveCommand> FortrendTMCavitySubsystem::createOpenHeightVacuumBaffleValveCommand()const{
		FortrendTMCavitySubsystem* self = const_cast<FortrendTMCavitySubsystem*>(this);
		TMCavityOpenHeightVacuumBaffleValveCommand::Ptr ret(new TMCavityOpenHeightVacuumBaffleValveCommand(self));
		return ret;
	}

	std::shared_ptr<TMCavityCloseHeightVacuumBaffleValveCommand> FortrendTMCavitySubsystem::createCloseHeightVacuumBaffleValveCommand()const{
		FortrendTMCavitySubsystem* self = const_cast<FortrendTMCavitySubsystem*>(this);
		TMCavityCloseHeightVacuumBaffleValveCommand::Ptr ret(new TMCavityCloseHeightVacuumBaffleValveCommand(self));
		return ret;
	}
	//角阀
	std::shared_ptr<TMCavityOpenAngleValveCommand> FortrendTMCavitySubsystem::createOpenAngleValveCommand()const{
		FortrendTMCavitySubsystem* self = const_cast<FortrendTMCavitySubsystem*>(this);
		TMCavityOpenAngleValveCommand::Ptr ret(new TMCavityOpenAngleValveCommand(self));
		return ret;
	}

	std::shared_ptr<TMCavityCloseAngleValveCommand>  FortrendTMCavitySubsystem::createCloseAngleValveCommand()const{
		FortrendTMCavitySubsystem* self = const_cast<FortrendTMCavitySubsystem*>(this);
		TMCavityCloseAngleValveCommand::Ptr ret(new TMCavityCloseAngleValveCommand(self));
		return ret;
	}
	

	

	//流量计隔膜阀
	std::shared_ptr<TMCavityOpenFlowmeterDiaphragmValveCommand> FortrendTMCavitySubsystem::createOpenFlowmeterDiaphragmValveCommand()const{
		FortrendTMCavitySubsystem* self = const_cast<FortrendTMCavitySubsystem*>(this);
		TMCavityOpenFlowmeterDiaphragmValveCommand::Ptr ret(new TMCavityOpenFlowmeterDiaphragmValveCommand(self));
		return ret;
	}

	std::shared_ptr<TMCavityCloseFlowmeterDiaphragmValveCommand>  FortrendTMCavitySubsystem::createCloseFlowmeterDiaphragmValveCommand()const{
		FortrendTMCavitySubsystem* self = const_cast<FortrendTMCavitySubsystem*>(this);
		TMCavityCloseFlowmeterDiaphragmValveCommand::Ptr ret(new TMCavityCloseFlowmeterDiaphragmValveCommand(self));
		return ret;
	}

	std::shared_ptr<TMCavityOpenPIDCommand> FortrendTMCavitySubsystem::createOpenPIDCommand()const{
		FortrendTMCavitySubsystem* self = const_cast<FortrendTMCavitySubsystem*>(this);
		TMCavityOpenPIDCommand::Ptr ret(new TMCavityOpenPIDCommand(self));
		return ret;
	}

	std::shared_ptr<TMCavityClosePIDCommand> FortrendTMCavitySubsystem::createClosePIDCommand()const{
		FortrendTMCavitySubsystem* self = const_cast<FortrendTMCavitySubsystem*>(this);
		TMCavityClosePIDCommand::Ptr ret(new TMCavityClosePIDCommand(self));
		return ret;
	}
}

