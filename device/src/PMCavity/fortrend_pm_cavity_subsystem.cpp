
/**
* @file            fortrend_pm_cavity_subsystem.h
* @brief           Fortrend PMCavity
* @author		   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/PMCavity

#include <iostream>
#include <Windows.h>

#include "Poco/Format.h"
#include "Poco/StringTokenizer.h"
#include "Poco/NumberParser.h"

#include "kernel/kernel.h"
#include "Kernel/kernel_log.h"
#include "kernel/kernel_iocontrol.h"
#include "kernel/kernel_configure.h"
#include "Kernel/kernel_configure.h"

#include "PMCavity/fortrend_pm_cavity_subsystem.h" 
#include "PMCavity/fortrend_pm_cavity_reset_command.h"
#include "PMCavity/fortrend_pm_cavity_update_command.h"
#include "PMCavity/fortrend_pm_cavity_output_command.h"

#include "kernel/kernel.h"
#include "Kernel/Fortrend/fortrend_cassette_manager.h"
#include "Kernel/Fortrend/cassette.h"
#include "LoadLock/fortrend_loadlock_subsystem.h" 

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif


namespace FC{
	/**
	* FortrendPMCavitySubsystemPrivate
	*/
	class FortrendPMCavitySubsystemPrivate{
	public:
		FortrendPMCavitySubsystemPrivate(FortrendPMCavitySubsystem*p);
		void setInput(uint8_t index, bool stat);
	public:
		FortrendPMCavitySubsystem* p;
		std::string io_input_address = "";
		unsigned short io_input_count = 0;
		std::vector<bool> io_input_last_value;
		//bool *ptr_io_input_state;
		std::unique_ptr<bool[]> ptr_io_input_state;
		bool tm_cavity_door_opend = false;   //传输腔门阀
		bool vacuum_enable = false;			 //真空开启
		bool with_wafer_mode = false;		 //带晶圆模式
		int wafer_slot = 0;					 //晶圆槽号
		bool pm_cavity_enable = true;		 //腔室是否启用

		double vacuum_set_value = 0.0;					//真空设定值
		double vacuum_break_set_value = 99600.0;        //真空破气设定值
		double temperature_set_value = 300.0;			//温度设定值
		double vacuum_magnitude_value = 0.1;			//传输腔比工艺腔数量级

		//PM Cavity Moniter State
		std::string dc_power_initial_address = "";
		short dc_power_address_length = 9;
		std::vector<PMCavityMoniterStateShort> pm_cavity_motiner_short_state;
		std::vector<PMCavityMoniterStateFloat> pm_cavity_motiner_float_state;
		std::vector<PMCavityMoniterStateShort> pm_cavity_motiner_dc_power_state;	//直流电源
		std::vector<PMCavityMoniterStateInt> pm_cavity_film_state;					//膜厚仪状态

		std::string get_request_address = "";			           //PM腔取片请求地址
		std::string upload_request_address = "";		           //PM腔上片请求地址
		std::string pm_cavity_general_alarm_address = "";          //PM腔总报警地址
		std::string pm_cavity_alarm_start_address = "";            //PM腔报警起始地址
		int pm_cavity_read_alarm_length = 65;			           //PM腔报警读取长度
		std::string pm_cavity_has_object_address = "";             //PM腔是否有晶圆地址
		std::string pm_cavity_safe_address = "";                   //PM腔安全信号地址
		std::string pm_cavity_motor_home_address = "MR35105";      //PM腔电机后退完成信号地址
		std::string pm_cavity_motor_forward_address = "MR35104";   //PM腔电机前进完成信号地址

		bool pm_cavity_safe = false;				//PM腔安全信号
		bool pm_cavity_motor_home = false;         //PM腔步进电机后退到位信号
		bool pm_cavity_motor_forward = false;
		bool pm_cavity_motor_run = false;
		bool isRunning = false;
		double speed = 0;
		double motorspeed = 0;
		int crft_count_lla = 1;
		int crft_count_llb = 1;
		double axlelocation = 0;
		std::string pm_remote_mode_address = "";		 //PM腔远程模式地址
		bool recard_enabled = false;
		std::thread thd_recard_vacuum;

		std::string pm_cavity_coating_time_address = ""; //PM腔镀膜时间地址
		std::string pm_cavity_processing_step_address = "";  //PM腔工艺步骤地址

		bool pm_update_process_parameters = false;
		PMCavityProcessParameters pm_process_parameters;   //PM腔工艺参数

	};

	/**
	* FortrendPMCavitySubsystemPrivate
	*/
	FortrendPMCavitySubsystemPrivate::FortrendPMCavitySubsystemPrivate(FortrendPMCavitySubsystem*p)
		:p(p){


	}

	/**
	* FortrendPMCavitySubsystem
	*/
	FortrendPMCavitySubsystem::FortrendPMCavitySubsystem(IKernel*  kernel, const std::string& name)
		:AbstractIOSubsystem(kernel, name)
		, FortrendAbstractStation(kernel)
		, KeyencePlcSubSystemHelper(name)
		, InovancePlcSubSystemHelper(name)
		, d(new FortrendPMCavitySubsystemPrivate(this)){
		//init 
		d->recard_enabled = true;
		d->thd_recard_vacuum = std::thread([this] { this->recardVacuum(); });
	}
	bool FortrendPMCavitySubsystem::hasBoxPlacement()const
	{
		return true;
	}

	bool FortrendPMCavitySubsystem::hasDoorOpend()const{
		return d->tm_cavity_door_opend;
	}

	int FortrendPMCavitySubsystem::inputCount()const
	{
		return KeyencePlcSubSystemHelper::helperInputCount();
	}

	int FortrendPMCavitySubsystem::outputCount()const
	{
		return KeyencePlcSubSystemHelper::helperOutputCount();
	}

	bool FortrendPMCavitySubsystem::getInput(int index)const
	{
		if (index < d->io_input_count)
		{
			return d->ptr_io_input_state[index];
		}
		return false;
	}

	std::string FortrendPMCavitySubsystem::getInputName(int index)const
	{
		return KeyencePlcSubSystemHelper::getHelperInputName(index);
	}

	std::string FortrendPMCavitySubsystem::getOutputName(int index)const
	{
		return KeyencePlcSubSystemHelper::getHelperOutputName(index);
	}

	void FortrendPMCavitySubsystem::setDoorOpen(const bool value)
	{
		if (d->tm_cavity_door_opend != value)
		{
			d->tm_cavity_door_opend = value;
			AbstractIOSubsystem::emitAttributeChanged(this);
		}

	}

	bool FortrendPMCavitySubsystem::getPMCavityDoorrOpend() const
	{
		return false;
	}

	bool FortrendPMCavitySubsystem::getIsRunning(){
		return d->isRunning;
	}
	void FortrendPMCavitySubsystem::setIsRunning(const bool running){
		d->isRunning = running;
	}
	double FortrendPMCavitySubsystem::getVacuumValue()const{
		if (d->pm_cavity_motiner_float_state.size() > 0)
		{
			return d->pm_cavity_motiner_float_state[0].last_value;
		}
		//return 100000;
	}

	double FortrendPMCavitySubsystem::getTemperatureValue()const{
		if (d->pm_cavity_motiner_short_state.size() >= 1)
		{
			return (d->pm_cavity_motiner_short_state[0].last_value / 10.0);
		}
		return 10000;
	}

	void FortrendPMCavitySubsystem::setVacuumEnable(const bool value){
		if (d->vacuum_enable != value)
		{
			d->vacuum_enable = value;
		}
	}
	void FortrendPMCavitySubsystem::setWithWaferModeEnable(const bool value){
		if (d->with_wafer_mode != value)
		{
			d->with_wafer_mode = value;
		}
	}

	bool FortrendPMCavitySubsystem::getVacuumValueReachesTheSetValue() const{
		if (!d->vacuum_enable)
		{
			return true;
		}
		if (getVacuumValue() < d->vacuum_set_value)
		{
			return true;
		}
		return false;
	}

	bool FortrendPMCavitySubsystem::getExhaustVacuumValueReachesTheSetValue()const{
		if (getVacuumValue() >= d->vacuum_break_set_value)
		{
			return true;
		}
		return false;
	}

	bool FortrendPMCavitySubsystem::getTemperatureValueReachesTheSetValue() const{

		if (getTemperatureValue() < d->temperature_set_value)
		{
			return true;
		}
		return false;
	}


	PMCavityMoniterValue FortrendPMCavitySubsystem::getPMCavityMonityValue()const{
		PMCavityMoniterValue buff = {};
		buff.temperature = d->pm_cavity_motiner_short_state[0].last_value;

		buff.vacuum_value = d->pm_cavity_motiner_float_state[0].last_value;
		buff.pre_stage_pippeline_vacuum = d->pm_cavity_motiner_float_state[1].last_value;
		buff.sample_rotation_speed = d->pm_cavity_motiner_float_state[2].last_value;
		buff.flow_meter_1 = d->pm_cavity_motiner_float_state[3].last_value;
		buff.flow_meter_2 = d->pm_cavity_motiner_float_state[4].last_value;
		buff.flow_meter_3 = d->pm_cavity_motiner_float_state[5].last_value;

		buff.dc_vlotage_1 = d->pm_cavity_motiner_dc_power_state[0].last_value;
		buff.dc_current_1 = d->pm_cavity_motiner_dc_power_state[1].last_value;
		buff.dc_power_1 = d->pm_cavity_motiner_dc_power_state[2].last_value;

		buff.dc_vlotage_2 = d->pm_cavity_motiner_dc_power_state[3].last_value;
		buff.dc_current_2 = d->pm_cavity_motiner_dc_power_state[4].last_value;
		buff.dc_power_2 = d->pm_cavity_motiner_dc_power_state[5].last_value;

		buff.dc_vlotage_3 = d->pm_cavity_motiner_dc_power_state[6].last_value;
		buff.dc_current_3 = d->pm_cavity_motiner_dc_power_state[7].last_value;
		buff.dc_power_3 = d->pm_cavity_motiner_dc_power_state[8].last_value;

		return buff;
	}

	bool FortrendPMCavitySubsystem::getPMCavityUpdateProcessParameters(){
		return d->pm_update_process_parameters;
	}

	PMCavityProcessParameters FortrendPMCavitySubsystem::getPMCavityProcessParameters(){
		d->pm_update_process_parameters = false;
		return d->pm_process_parameters;

	}

	PMCavityMoniterFilmValue FortrendPMCavitySubsystem::getPMCavityFilmValue(){
		PMCavityMoniterFilmValue buff = {};
		buff.frequency = d->pm_cavity_film_state[0].last_value;
		buff.rate = d->pm_cavity_film_state[1].last_value;
		buff.accumulative_total = d->pm_cavity_film_state[2].last_value;
		buff.lifetime = d->pm_cavity_film_state[3].last_value;
		return buff;

	}


	//PM腔工艺参数
	void FortrendPMCavitySubsystem::setPMCavityProcessParameters(const PMCavityProcessParameters pm_param){
		bool change = false;
		if (d->pm_process_parameters.heating_temperature != pm_param.heating_temperature)
		{
			d->pm_process_parameters.heating_temperature = pm_param.heating_temperature;
			change = true;
		}
		if (d->pm_process_parameters.initial_extraction_pressure != pm_param.initial_extraction_pressure)
		{
			d->pm_process_parameters.initial_extraction_pressure = pm_param.initial_extraction_pressure;
			change = true;
		}
		if (d->pm_process_parameters.purified_extraction_pressure != pm_param.purified_extraction_pressure)
		{
			d->pm_process_parameters.purified_extraction_pressure = pm_param.purified_extraction_pressure;
			change = true;
		}
		if (d->pm_process_parameters.sputtering_pressure != pm_param.sputtering_pressure)
		{
			d->pm_process_parameters.sputtering_pressure = pm_param.sputtering_pressure;
			change = true;
		}
		if (d->pm_process_parameters.sputtering_flow_rate1 != pm_param.sputtering_flow_rate1)
		{
			d->pm_process_parameters.sputtering_flow_rate1 = pm_param.sputtering_flow_rate1;
			change = true;
		}
		if (d->pm_process_parameters.sputtering_flow_rate2 != pm_param.sputtering_flow_rate2)
		{
			d->pm_process_parameters.sputtering_flow_rate2 = pm_param.sputtering_flow_rate2;
			change = true;
		}
		if (d->pm_process_parameters.sputtering_flow_rate3 != pm_param.sputtering_flow_rate3)
		{
			d->pm_process_parameters.sputtering_flow_rate3 = pm_param.sputtering_flow_rate3;
			change = true;
		}
		if (d->pm_process_parameters.sputtering_power1 != pm_param.sputtering_power1)
		{
			d->pm_process_parameters.sputtering_power1 = pm_param.sputtering_power1;
			change = true;
		}
		if (d->pm_process_parameters.sputtering_power_gear_up1 != pm_param.sputtering_power_gear_up1)
		{
			d->pm_process_parameters.sputtering_power_gear_up1 = pm_param.sputtering_power_gear_up1;
			change = true;
		}
		if (d->pm_process_parameters.sputtering_power2 != pm_param.sputtering_power2)
		{
			d->pm_process_parameters.sputtering_power2 = pm_param.sputtering_power2;
			change = true;
		}
		if (d->pm_process_parameters.sputtering_power_gear_up2 != pm_param.sputtering_power_gear_up2)
		{
			d->pm_process_parameters.sputtering_power_gear_up2 = pm_param.sputtering_power_gear_up2;
			change = true;
		}
		if (d->pm_process_parameters.sputtering_power3 != pm_param.sputtering_power3)
		{
			d->pm_process_parameters.sputtering_power3 = pm_param.sputtering_power3;
			change = true;
		}
		if (d->pm_process_parameters.sputtering_power_gear_up3 != pm_param.sputtering_power_gear_up3)
		{
			d->pm_process_parameters.sputtering_power_gear_up3 = pm_param.sputtering_power_gear_up3;
			change = true;
		}
		if (d->pm_process_parameters.pre_sputtering_time != pm_param.pre_sputtering_time)
		{
			d->pm_process_parameters.pre_sputtering_time = pm_param.pre_sputtering_time;
			change = true;
		}
		if (d->pm_process_parameters.substrate_speed != pm_param.substrate_speed)
		{
			d->pm_process_parameters.substrate_speed = pm_param.substrate_speed;
			change = true;
		}
		if (d->pm_process_parameters.process_sputtering_time != pm_param.process_sputtering_time)
		{
			d->pm_process_parameters.process_sputtering_time = pm_param.process_sputtering_time;
			change = true;
		}
		if (d->pm_process_parameters.cathode_power_selection_1 != pm_param.cathode_power_selection_1)
		{
			d->pm_process_parameters.cathode_power_selection_1 = pm_param.cathode_power_selection_1;
			change = true;
		}
		if (d->pm_process_parameters.cathode_power_selection_2 != pm_param.cathode_power_selection_2)
		{
			d->pm_process_parameters.cathode_power_selection_2 = pm_param.cathode_power_selection_2;
			change = true;
		}
		if (d->pm_process_parameters.cathode_power_selection_3 != pm_param.cathode_power_selection_3)
		{
			d->pm_process_parameters.cathode_power_selection_3 = pm_param.cathode_power_selection_3;
			change = true;
		}
		if (change)
		{
			d->pm_update_process_parameters = true;
			AbstractIOSubsystem::emitAttributeChanged(this);
		}
	}
	/**

	* 获取镀膜时间
	*/
	float FortrendPMCavitySubsystem::getPMCavityCoatingTime(){
		float result = 0;
		if (d->pm_cavity_coating_time_address != "")
		{
			InovancePlcSubSystemHelper::readFloat(d->pm_cavity_coating_time_address, result);
		}
		return result;

	}

	/**
	* 获取工艺步骤
	*/
	short FortrendPMCavitySubsystem::getPMCavityProcessingStep(){
		short result = 0;
		if (d->pm_cavity_processing_step_address != "")
		{
			InovancePlcSubSystemHelper::readShort(d->pm_cavity_processing_step_address, result);
		}

		return result;
	}

	/**
	* 获取晶圆槽号
	*/
	int FortrendPMCavitySubsystem::getWaferSlot()const{
		return d->wafer_slot;
	}

	/**
	* 设置晶圆槽号
	*/
	void FortrendPMCavitySubsystem::setWaferSlot(const int slot){
		d->wafer_slot = slot;
	}

	bool FortrendPMCavitySubsystem::getPMCavityGetRequest(){
		bool result = false;

		InovancePlcSubSystemHelper::readBit(d->get_request_address, result);
		logInform1(getName().c_str(), Poco::format("获取取片请求address = %s, result = %d", d->get_request_address, (int)result).c_str());
		return result;
	}
	bool FortrendPMCavitySubsystem::getPMCavityUploadRequest(){
		bool result = false;
		InovancePlcSubSystemHelper::readBit(d->upload_request_address, result);
		logInform1(getName().c_str(), Poco::format("获取上片请求address = %s, result = %d", d->upload_request_address, (int)result).c_str());
		return result;
	}

	void FortrendPMCavitySubsystem::setPMCavitySafeSignal(bool value){
		d->pm_cavity_safe = value;
	}

	//获取PM腔安全信号
	bool FortrendPMCavitySubsystem::getPMCavitySafeSignal(){
		//bool result = false;
		//KeyencePlcSubSystemHelper::readBit(d->pm_cavity_safe_address, result);
		//logInform1(getName().c_str(), Poco::format("获取%s腔安全信号address = %s, result = %d", getName(), d->pm_cavity_safe_address, (int)result).c_str());
		//return result;
		return d->pm_cavity_safe;
	}

	//获取PM腔步进电机后退到位信号
	bool FortrendPMCavitySubsystem::getPMCavityMotorHomeSignal(){
		//bool result = false;
		//KeyencePlcSubSystemHelper::readBit(d->pm_cavity_safe_address, result);
		//logInform1(getName().c_str(), Poco::format("获取%s腔安全信号address = %s, result = %d", getName(), d->pm_cavity_safe_address, (int)result).c_str());
		//return result;
		return d->pm_cavity_motor_home;
	}

	//获取PM腔步进电机前进到位信号
	bool FortrendPMCavitySubsystem::getPMCavityMotorForwardSignal(){
		//bool result = false;
		//KeyencePlcSubSystemHelper::readBit(d->pm_cavity_safe_address, result);
		//logInform1(getName().c_str(), Poco::format("获取%s腔安全信号address = %s, result = %d", getName(), d->pm_cavity_safe_address, (int)result).c_str());
		//return result;
		return d->pm_cavity_motor_forward;
	}

	//获取PM腔步进电机运动信号
	bool FortrendPMCavitySubsystem::getPMCavityMotorRunSignal(){
		//bool result = false;
		//KeyencePlcSubSystemHelper::readBit(d->pm_cavity_safe_address, result);
		//logInform1(getName().c_str(), Poco::format("获取%s腔安全信号address = %s, result = %d", getName(), d->pm_cavity_safe_address, (int)result).c_str());
		//return result;
		return d->pm_cavity_motor_run;
	}
	//获取PM腔是否处于远程模式
	bool FortrendPMCavitySubsystem::getPMCavityRemoteMode(){
		bool result = false;
		InovancePlcSubSystemHelper::readBit(d->pm_remote_mode_address, result);
		logInform1(getName().c_str(), Poco::format("获取%s腔远程模式address = %s, result = %d", getName(), d->pm_remote_mode_address, (int)result).c_str());
		return result;
	}

	bool FortrendPMCavitySubsystem::readPMCavityHasObjectState(){
		bool result = false;
		InovancePlcSubSystemHelper::readBit(d->pm_cavity_has_object_address, result);
		return result;
	}

	bool FortrendPMCavitySubsystem::getPMCavityHasAlarm(){//
		if (getState() != IKernelSubSystem::State::SUB_UNKNOWN)
		{
			//读取PM腔的PLC报警
			bool pm_alarm = false;

			if (InovancePlcSubSystemHelper::readBit(d->pm_cavity_general_alarm_address, pm_alarm))
			{
				return pm_alarm;
			}
		}
		return false;
	}

	std::string FortrendPMCavitySubsystem::getPMCavityAlarmMessage()
	{
		std::string message = "";
		//bool * alarm_address = new bool[d->pm_cavity_read_alarm_length];
		std::unique_ptr<bool[]> alarm_address(new bool[d->pm_cavity_read_alarm_length]);
		InovancePlcSubSystemHelper::readBits(d->pm_cavity_alarm_start_address, d->pm_cavity_read_alarm_length, alarm_address.get());
		for (size_t i = 0; i < d->pm_cavity_read_alarm_length; i++)
		{
			if (alarm_address[i])
			{
				auto alarm_message = InovancePlcSubSystemHelper::getErrorCode(1, 900 + i);
				message = Poco::format(" %s腔报警：地址：M%d，报警信息：%s", getName(), int(900 + i), alarm_message->message);
				break;
			}
		}
		
		logError(getName().c_str(), message.c_str());
		return message;
	}

	//给PM腔PLC写放置晶圆状态
	bool FortrendPMCavitySubsystem::writePMCavityHasObjectState(const bool value){
		bool result = false;
		result = InovancePlcSubSystemHelper::writeBit(d->pm_cavity_has_object_address, value);;
		return result;
	}

	//获取是否启用PM腔
	bool FortrendPMCavitySubsystem::getPMCavityEnable()const{

		return d->pm_cavity_enable;
	}

	//设置是否启用PM腔
	void FortrendPMCavitySubsystem::setPMCavityEnable(const bool value){
		if (value != d->pm_cavity_enable)
		{
			d->pm_cavity_enable = value;
		}

	}

	//获取PM腔真空数量级
	double FortrendPMCavitySubsystem::getPMCavityMagnitude()const{
		return d->vacuum_magnitude_value;
	}

	double FortrendPMCavitySubsystem::getPMCavityAxleSpeed()const{
		return d->speed;
	}

	double FortrendPMCavitySubsystem::getPMCavityMotorSpeed()const{
		return d->motorspeed;
	}
	int FortrendPMCavitySubsystem::getPMCavityCrftCountLLA()const{
		return d->crft_count_lla;
	}

	int FortrendPMCavitySubsystem::getPMCavityCrftCountLLB()const{
		return d->crft_count_llb;
	}

	void FortrendPMCavitySubsystem::setPMCavityCrftCountLLA(int count){
		d->crft_count_lla = count;
		std::shared_ptr<FortrendCassetteManager> cassManager = getKernel()->getKernelModule<FortrendCassetteManager>();
		auto lk1 = getKernel()->getKernelModule<FortrendLoadLockSubsystem>("LLA");
		auto  cass = cassManager->getCassette(lk1.get());
		cass->setPodSize(count);
	}
	void FortrendPMCavitySubsystem::setPMCavityCrftCountLLB(int count){
		d->crft_count_llb = count;
		std::shared_ptr<FortrendCassetteManager> cassManager = getKernel()->getKernelModule<FortrendCassetteManager>();
		auto lk2 = getKernel()->getKernelModule<FortrendLoadLockSubsystem>("LLB");
		auto  cass = cassManager->getCassette(lk2.get());
		cass->setPodSize(count);
	}

	double FortrendPMCavitySubsystem::getPMCavityAxleLocation(){
		if (getState() == IKernelSubSystem::State::SUB_IDEL){
			KeyencePlcSubSystemHelper::readDouble("DM15040", d->axlelocation);
		}
		return d->axlelocation;
	}

	void FortrendPMCavitySubsystem::setPMCavityAxleSpeed(double speed){
		//KeyencePlcSubSystemHelper::writeDouble("DM15070", speed);
		float sp = speed;
		//logInform("Test", "PM腔速度设置，地址：DM15070  值%.2f", sp);
		KeyencePlcSubSystemHelper::writeFloat("DM15070", sp);
	}
	void FortrendPMCavitySubsystem::setPMCavityTurnSpeed(double speed){
		float sp = speed;
		//logInform("Test", "PM腔步进电机速度设置，地址：DM15080  值%.2f", sp);
		KeyencePlcSubSystemHelper::writeFloat("DM15080", sp);
	}
	void FortrendPMCavitySubsystem::setPMCavityForward(bool forward){
		//logInform("Test", "PM腔步进电机前进，地址：MR35213  值%d", forward);
		if (getIsRunning()){
			logWarn(getName().c_str(), "自动模式下无法控制电机前进！");
		}
		else{
			KeyencePlcSubSystemHelper::writeBit("MR35213", forward);
		}
		
	}
	void FortrendPMCavitySubsystem::setPMCavityBackward(bool backward){
		//logInform("Test", "PM腔步进电机后退，地址：MR35214  值%d", backward);
		KeyencePlcSubSystemHelper::writeBit("MR35214", backward);
	}
	//设定真空值以及真空数量级
	void FortrendPMCavitySubsystem::setVacuumSettingAndMagnitudeValue(const double setting_value, const double magnitude_value){
		d->vacuum_set_value = setting_value;
		d->vacuum_magnitude_value = magnitude_value;
		logInform(getName().c_str(), Poco::format("%s 设置真空上限值：%f, 数量级设定值：%f", getName(), d->vacuum_set_value, d->vacuum_magnitude_value).c_str());

	}

	void FortrendPMCavitySubsystem::onInitialize()throw(KernelException){

		if (SIM_MODE == 1)
		{
			setState(IKernelSubSystem::State::SUB_NORMAL);
		}
		else
		{
			try {
				if (KeyencePlcSubSystemHelper::enableProtocol())
				{
					setState(IKernelSubSystem::State::SUB_IDEL);
				}
				else {
					setState(IKernelSubSystem::State::SUB_ERROR);
				}

			}
			catch (KernelException& e) {
				logError(getName().c_str(), e.what());
				//throw e;
			}
		}
	}

	void FortrendPMCavitySubsystem::onUnInitialize()throw(KernelException){
		d->recard_enabled = false;
		KeyencePlcSubSystemHelper::disableProtocol();
		InovancePlcSubSystemHelper::disableProtocol();
		if (d->thd_recard_vacuum.joinable())
		{
			d->thd_recard_vacuum.join();
		}
	}

	void FortrendPMCavitySubsystem::onProcess(){
		//pollProtocol();
		//short * dc_power_current_value = new short[d->dc_power_address_length];
		if (getState() != IKernelSubSystem::State::SUB_UNKNOWN)
		{

			bool io_changed = false;
			bool result = d->pm_cavity_safe;
			if (d->pm_cavity_safe_address != "" &&KeyencePlcSubSystemHelper::readBit(d->pm_cavity_safe_address, d->pm_cavity_safe))
			{
				if (result != d->pm_cavity_safe){
					//logInform("Test", "PM腔安全信号读取，地址：%s  值%d", d->pm_cavity_safe_address, d->pm_cavity_safe);
					io_changed = true;
				}
			}
			bool result2 = d->pm_cavity_motor_home;
			if (d->pm_cavity_motor_home_address != "" &&KeyencePlcSubSystemHelper::readBit(d->pm_cavity_motor_home_address, d->pm_cavity_motor_home))
			{
				if (result2 != d->pm_cavity_motor_home){
					io_changed = true;
				}
			}

			bool result3 = d->pm_cavity_motor_forward;
			if (d->pm_cavity_motor_forward_address != "" &&KeyencePlcSubSystemHelper::readBit(d->pm_cavity_motor_forward_address, d->pm_cavity_motor_forward))
			{
				if (result3 != d->pm_cavity_motor_forward){
					io_changed = true;
				}
			}
			bool result4 = d->pm_cavity_motor_run;
			if (KeyencePlcSubSystemHelper::readBit("MR35106", d->pm_cavity_motor_run))
			{
				if (result4 != d->pm_cavity_motor_run){
					io_changed = true;
				}
			}
			float resultspeed = d->speed;
			if (KeyencePlcSubSystemHelper::readFloat("DM15070", resultspeed)){
				if (resultspeed != d->speed){
					d->speed = resultspeed;
					logInform("Test", "PM腔速度读取，地址：DM15074  值%.2f", d->speed);
					io_changed = true;
				}
			}

			float resultspeed2 = d->motorspeed;
			if (KeyencePlcSubSystemHelper::readFloat("DM15080", resultspeed2)){
				if (resultspeed2 != d->motorspeed){
					d->motorspeed = resultspeed2;
					logInform("Test", "PM腔电机速度读取，地址：DM15080  值%.2f", d->motorspeed);
					io_changed = true;
				}
			}

			double resultlocation = d->axlelocation;
			if (KeyencePlcSubSystemHelper::readDouble("DM15040", d->axlelocation)){
				if (resultlocation != d->axlelocation){
					io_changed = true;
				}
			}		
			/*if (d->io_input_count > 0)
			{
				if (KeyencePlcSubSystemHelper::readBits(d->io_input_address, d->io_input_count, d->ptr_io_input_state))
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
			}
			for (size_t i = 0; i < d->pm_cavity_motiner_float_state.size(); i++)
			{
				if (InovancePlcSubSystemHelper::readFloat(d->pm_cavity_motiner_float_state[i].address, d->pm_cavity_motiner_float_state[i].current_value)
					&& d->pm_cavity_motiner_float_state[i].last_value != d->pm_cavity_motiner_float_state[i].current_value){
					d->pm_cavity_motiner_float_state[i].last_value = d->pm_cavity_motiner_float_state[i].current_value;
					io_changed = true;
				}
			}
			for (size_t i = 0; i < d->pm_cavity_motiner_short_state.size(); i++)
			{
				if (InovancePlcSubSystemHelper::readShort(d->pm_cavity_motiner_short_state[i].address, d->pm_cavity_motiner_short_state[i].current_value)
					&& d->pm_cavity_motiner_short_state[i].last_value != d->pm_cavity_motiner_short_state[i].current_value){
					d->pm_cavity_motiner_short_state[i].last_value = d->pm_cavity_motiner_short_state[i].current_value;
					io_changed = true;
				}
			}
			if (d->dc_power_initial_address != "" &&
				InovancePlcSubSystemHelper::readShorts(d->dc_power_initial_address, d->dc_power_address_length, dc_power_current_value))
			{
				for (size_t i = 0; i < d->dc_power_address_length; i++)
				{
					d->pm_cavity_motiner_dc_power_state[i].current_value = dc_power_current_value[i];
					if (dc_power_current_value[i] != d->pm_cavity_motiner_dc_power_state[i].last_value)
					{
						d->pm_cavity_motiner_dc_power_state[i].last_value = dc_power_current_value[i];
						io_changed = true;
					}
				}
			}

			for (size_t i = 0; i < d->pm_cavity_film_state.size(); i++)
			{
				if (InovancePlcSubSystemHelper::readInt(d->pm_cavity_film_state[i].address, d->pm_cavity_film_state[i].current_value)
					&& d->pm_cavity_film_state[i].last_value != d->pm_cavity_film_state[i].current_value){
					d->pm_cavity_film_state[i].last_value = d->pm_cavity_film_state[i].current_value;
					io_changed = true;
				}
			}*/
			//if (io_changed)
			//{
			//	AbstractIOSubsystem::emitAttributeChanged(this);
			//}
			//Sleep(100);

		}
	}

	void FortrendPMCavitySubsystem::recardVacuum()const{
		int count = 0;
		while (d->recard_enabled)
		{
			Sleep(20);
			count++;
			if (count == 100)
			{
				count = 0;
				logInform1(Poco::format("%s_vacuum", getName()).c_str(), Poco::format("%f", getVacuumValue()).c_str());
			}


		}
	}



	void FortrendPMCavitySubsystem::onConfigure(const std::shared_ptr<KernelConfiguration> & config){
		KernelAbstractSubSystem::onConfigure(config);
		FortrendAbstractStation::configure(config);
		configKeyencePlc(config);
		d->io_input_count = inputCount();
		for (size_t i = 0; i < d->io_input_count; i++)
		{
			d->io_input_last_value.push_back(false);
		}
		if (d->io_input_count > 0)
		{
			//d->ptr_io_input_state = new bool[d->io_input_count];
			d->ptr_io_input_state = std::make_unique<bool[]>(d->io_input_count);
		}
		d->io_input_address = KeyencePlcSubSystemHelper::getHelperInputAddress();
		if (config->has("PM_Parameter"))
		{
			d->vacuum_break_set_value = config->getDouble("PM_Parameter.BreakSetValue", 99600.0);
			d->temperature_set_value = config->getDouble("PM_Parameter.TemperatureSetValue", 300);
		}
		if (config->has("PMCavityStateAddress"))
		{
			if (d->pm_cavity_motiner_short_state.size() > 0)
			{
				d->pm_cavity_motiner_short_state.clear();
			}
			if (d->pm_cavity_motiner_float_state.size() > 0)
			{
				d->pm_cavity_motiner_float_state.clear();
			}
			if (d->pm_cavity_motiner_dc_power_state.size() > 0)
			{
				d->pm_cavity_motiner_dc_power_state.clear();
			}
			if (d->pm_cavity_film_state.size() > 0)
			{
				d->pm_cavity_film_state.clear();
			}
			PMCavityMoniterStateShort temperature = { config->getString("PMCavityStateAddress.TemperatureAddress", ""), "Temperature", 0.0F, 0.1F };
			d->pm_cavity_motiner_short_state.push_back(temperature);

			PMCavityMoniterStateFloat vacuum_value = { config->getString("PMCavityStateAddress.VacuumValueAddress", ""), "VacuumValue", 0.0F, 0.1F };
			PMCavityMoniterStateFloat pre_stage_pippeline_vacuum = { config->getString("PMCavityStateAddress.PreStagePippelineVacuumAddress", "Pre_stagePippelineVacuum"), "", 0.0, 0.1F };
			PMCavityMoniterStateFloat sample_rotation_speed = { config->getString("PMCavityStateAddress.SampleRotationAddress", ""), "SampleRotation", 0.0F, 0.1F };
			PMCavityMoniterStateFloat flow_meter_1 = { config->getString("PMCavityStateAddress.FlowMeter1Address", ""), "FlowMeter1", 0.0F, 0.0F };
			PMCavityMoniterStateFloat flow_meter_2 = { config->getString("PMCavityStateAddress.FlowMeter2Address", ""), "FlowMeter2", 0.0F, 0.0F };
			PMCavityMoniterStateFloat flow_meter_3 = { config->getString("PMCavityStateAddress.FlowMeter3Address", ""), "FlowMeter3", 0.0F, 0.0F };
			d->pm_cavity_motiner_float_state.push_back(vacuum_value);
			d->pm_cavity_motiner_float_state.push_back(pre_stage_pippeline_vacuum);
			d->pm_cavity_motiner_float_state.push_back(sample_rotation_speed);
			d->pm_cavity_motiner_float_state.push_back(flow_meter_1);
			d->pm_cavity_motiner_float_state.push_back(flow_meter_2);
			d->pm_cavity_motiner_float_state.push_back(flow_meter_3);

			d->dc_power_initial_address = config->getString("PMCavityStateAddress.DCPowerInitialAddress", "");
			d->dc_power_address_length = (short)(config->getInt("PMCavityStateAddress.DCPowerAddressLength", 9));

			for (size_t i = 0; i < d->dc_power_address_length; i++)
			{
				PMCavityMoniterStateShort buff = { std::to_string(i), std::to_string(i), 0, 0 };
				d->pm_cavity_motiner_dc_power_state.push_back(buff);
			}

			PMCavityMoniterStateInt frequency = { config->getString("PMCavityStateAddress.FilmFrequencyAddress", ""), "FilmFrequency", 0, 0 };
			PMCavityMoniterStateInt rate = { config->getString("PMCavityStateAddress.FilmRateAddress", ""), "FilmRate", 0, 0 };
			PMCavityMoniterStateInt accumulative_total = { config->getString("PMCavityStateAddress.FilmAccumulativeTotalAddress", ""), "FilmAccumulativeTotal", 0, 0 };
			PMCavityMoniterStateInt lifetime = { config->getString("PMCavityStateAddress.FilmLifetimeAddress", ""), "FilmLifetime", 0, 0 };
			d->pm_cavity_film_state.push_back(frequency);
			d->pm_cavity_film_state.push_back(rate);
			d->pm_cavity_film_state.push_back(accumulative_total);
			d->pm_cavity_film_state.push_back(lifetime);
		}
		if (config->has("PMCavityRequestAddress"))
		{
			d->get_request_address = config->getString("PMCavityRequestAddress.GetAddress", "M21");
			d->upload_request_address = config->getString("PMCavityRequestAddress.UploadAddress", "M20");
		}
		if (config->has("PMCavityRecardAddress"))
		{
			d->pm_cavity_has_object_address = config->getString("PMCavityRecardAddress.HasObjectAddress", "M1004");
			d->pm_cavity_safe_address = config->getString("PMCavityRecardAddress.SafeAddress", "MR30602");
			d->pm_remote_mode_address = config->getString("PMCavityRecardAddress.RemoteModeAddress", "M33");
			d->pm_cavity_general_alarm_address = config->getString("PMCavityRecardAddress.GeneralAlarmAddress", "");
			d->pm_cavity_alarm_start_address = config->getString("PMCavityRecardAddress.AlarmStartAddress", "");
			d->pm_cavity_read_alarm_length = config->getInt("PMCavityRecardAddress.ReadAlarmLength", 65);
			d->pm_cavity_coating_time_address = config->getString("PMCavityRecardAddress.CoatingTimeAddress", "");
			d->pm_cavity_processing_step_address = config->getString("PMCavityRecardAddress.ProcessingStepAddress", "");
			d->pm_cavity_motor_home_address = config->getString("PMCavityRecardAddress.MotorHomeAddress", "MR35105");
			d->pm_cavity_motor_forward_address=config->getString("PMCavityRecardAddress.MotorForwardAddress", "MR35104");
		}

	}


	std::shared_ptr<KernelSubsystemResetCommand> FortrendPMCavitySubsystem::createResetCommand()const{
		FortrendPMCavitySubsystem* self = const_cast<FortrendPMCavitySubsystem*>(this);
		PMCavityResetCommand::Ptr ret(new PMCavityResetCommand(self));
		return ret;
	}

	std::shared_ptr<KernelSubsystemUpdateCommand> FortrendPMCavitySubsystem::createUpdateCommand()const{
		FortrendPMCavitySubsystem* self = const_cast<FortrendPMCavitySubsystem*>(this);
		PMCavityUpdateCommand::Ptr ret(new PMCavityUpdateCommand(self));
		return ret;
	}

	std::shared_ptr<AbstractOutPutCommand>  FortrendPMCavitySubsystem::createOutputCommand(int channel, bool stat)const{
		FortrendPMCavitySubsystem* self = const_cast<FortrendPMCavitySubsystem*>(this);
		AbstractOutPutCommand::Ptr ret(new PMCavityOutputCommand(channel, stat, self));
		return ret;
	}

	std::shared_ptr<PMCavityOpenTMCavityDoorCommand>  FortrendPMCavitySubsystem::createOpenTMCavityDoorCommand()const{
		FortrendPMCavitySubsystem* self = const_cast<FortrendPMCavitySubsystem*>(this);
		PMCavityOpenTMCavityDoorCommand::Ptr ret(new PMCavityOpenTMCavityDoorCommand(self));
		return ret;
	}

	std::shared_ptr<PMCavityCloseTMCavityDoorCommand>  FortrendPMCavitySubsystem::createCloseTMCavityDoorCommand()const{
		FortrendPMCavitySubsystem* self = const_cast<FortrendPMCavitySubsystem*>(this);
		PMCavityCloseTMCavityDoorCommand::Ptr ret(new PMCavityCloseTMCavityDoorCommand(self));
		return ret;
	}

	std::shared_ptr<PMCavityGetFinishedCommand> FortrendPMCavitySubsystem::createGetFinishedCommand()const{
		FortrendPMCavitySubsystem* self = const_cast<FortrendPMCavitySubsystem*>(this);
		PMCavityGetFinishedCommand::Ptr ret(new PMCavityGetFinishedCommand(self));
		return ret;
	}
	std::shared_ptr<PMCavityUploadFinishedCommand> FortrendPMCavitySubsystem::createUploadFinishedCommand()const{
		FortrendPMCavitySubsystem* self = const_cast<FortrendPMCavitySubsystem*>(this);
		PMCavityUploadFinishedCommand::Ptr ret(new PMCavityUploadFinishedCommand(self));
		return ret;
	}


	std::shared_ptr<PMCavityToGetStationCommand> FortrendPMCavitySubsystem::createToGetStationCommand()const{
		FortrendPMCavitySubsystem* self = const_cast<FortrendPMCavitySubsystem*>(this);
		PMCavityToGetStationCommand::Ptr ret(new PMCavityToGetStationCommand(self));
		return ret;
	}
	std::shared_ptr<PMCavityToPutStationCommand> FortrendPMCavitySubsystem::createToPutStationCommand(int stationid)const{
		FortrendPMCavitySubsystem* self = const_cast<FortrendPMCavitySubsystem*>(this);
		PMCavityToPutStationCommand::Ptr ret(new PMCavityToPutStationCommand(self, stationid));
		return ret;
	}

	std::shared_ptr<PMCavityReadProcessParametersCommand>  FortrendPMCavitySubsystem::createReadProcessParametersCommand()const{
		FortrendPMCavitySubsystem* self = const_cast<FortrendPMCavitySubsystem*>(this);
		PMCavityReadProcessParametersCommand::Ptr ret(new PMCavityReadProcessParametersCommand(self));
		return ret;
	}

	std::shared_ptr<PMCavityWriteProcessParametersCommand>  FortrendPMCavitySubsystem::createWriteProcessParametersCommand(const PMCavityProcessParameters process_parameters)const{
		FortrendPMCavitySubsystem* self = const_cast<FortrendPMCavitySubsystem*>(this);
		PMCavityWriteProcessParametersCommand::Ptr ret(new PMCavityWriteProcessParametersCommand(process_parameters, self));
		return ret;
	}

	std::shared_ptr<PMCavityClearStateCommand>  FortrendPMCavitySubsystem::createClearStateCommand()const{
		FortrendPMCavitySubsystem* self = const_cast<FortrendPMCavitySubsystem*>(this);
		PMCavityClearStateCommand::Ptr ret(new PMCavityClearStateCommand(self));
		return ret;
	}

	std::shared_ptr<PMCavityInsertingPlateOpeningControllerCommand>  FortrendPMCavitySubsystem::createInsertingPlateOpeningControllerCommand(const float percentage) const{
		FortrendPMCavitySubsystem* self = const_cast<FortrendPMCavitySubsystem*>(this);
		PMCavityInsertingPlateOpeningControllerCommand::Ptr ret(new PMCavityInsertingPlateOpeningControllerCommand(percentage, self));
		return ret;
	}
}


