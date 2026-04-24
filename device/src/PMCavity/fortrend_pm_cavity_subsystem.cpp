
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

		bool pm_update_axis_setting_parameters = false;
		PMCavityAxisSettingParameters  axis_setting_parameters; //PM腔电机设置参数

		std::string minimum_plane_level_detection_address = "MR34000"; //最低面位检测
		std::string rotating_plane_level_detection_address= "MR34001"; //中间旋转面位检测
		std::string maximum_plane_level_detection_address = "MR34002"; //最高面位检测
		std::string liftpin_plane_level_detection_address = "MR34003"; //顶升销面位检测

		bool pm_cavity_motor_minimum_plane_signal = false;
		bool pm_cavity_motor_rotating_plane_signal = false;
		bool pm_cavity_motor_maximum_plane_signal = false;
		bool pm_cavity_motor_liftpin_plane_signal = false;

		std::string open_tm_cavity_door_address = "";
		std::string close_tm_cavity_door_address = "";

		std::string lifting_axis_clear_error_address = ""; //清除Z轴错误

		std::string rotating_axis_clear_error_address = ""; //清除R轴错误

		std::string lifting_axis_rest_address = ""; //Z轴回原

		std::string rotating_axis_rest_address = "";//R轴回原

		std::string lifting_axis_return_original_completion_address = "";//Z轴回原完成
		bool axis_origin_done;

		std::string lifting_axis_clear_error_completion_address = ""; //升降轴清除轴错误完成
		bool lifting_axis_clear_done;

		std::string lifting_axis_jog_running_address; //升降轴JOG运行中
		bool lifting_axis_jog_running;

		std::string lifting_axis_return_original_running_address; //升降轴回原中
		bool lifting_axis_return_original_running;

		std::string lifting_axis_enable_address;//升降轴使能ON
		bool lifting_axis_enable_done;

		std::string lifting_axis_moving_address;//升降轴移动中
		bool lifting_axis_moving;

		std::string lifting_axis_move_end_address;//升降轴移动结束
		bool lifting_axis_move_done;

		std::string lifting_axis_feedback_position_addresss;//升降轴反馈位置
		float lifting_axis_feedback_position = 0.0F;

		std::string lifting_axis_current_coordinate_addresss;//升降轴当前坐标
		float lifting_axis_current_coordinate = 0.0F;

		std::string lifting_axis_current_speed_addresss;//升降轴当前速度
		float lifting_axis_current_speed;

		std::string lifting_axis_current_control_mode_addresss;//升降轴当前控制模式
		uint16_t current_control_mode;

		std::string lifting_axis_feedback_torque_monitor_address;//升降轴反馈转矩监视器
		float lifting_axis_feedback_torque_monitor;

		std::string rotating_axis_return_original_completion_address; //R轴回原完成
		bool rotating_axis_return_original_done;

		std::string rotating_axis_clear_error_completion_address;//旋转轴清除轴错误完成
		bool rotating_axis_clear_error_done;

		std::string rotating_axis_stop_completion_address;//旋转轴轴停止完成
		bool rotating_axis_stop_done;

		std::string rotating_axis_jog_running_address;//旋转轴JOG运行中
		bool rotating_axis_jog_running;

		std::string rotating_axis_return_original_running_address;//旋转轴回原中
		bool rotating_axis_return_original_running;

		std::string rotating_axis_enable_address;//旋转轴使能ON
		bool rotating_axis_enable_done;

		std::string rotating_axis_moving_address;//旋转轴移动中
		bool  rotating_axis_moving;

		std::string rotating_axis_move_end_address;//旋转轴移动结束
		bool  rotating_axis_move_end;

		std::string rotating_axis_feedback_position_addresss;//旋转轴反馈位置
		int  rotating_axis_feedback_position;

		std::string rotating_axis_current_coordinate_addresss;//旋转轴当前坐标
		float rotating_axis_current_coordinate;

		std::string rotating_axis_current_speed_addresss;//旋转轴当前速度
		float rotating_axis_current_speed;

		int pm_craft_count = 0; //工艺次数

		std::string lifting_axis_motor_speed_address; //Z轴 jog 速度
		float lifting_motor_speed;
		
		std::string rotating_axis_motor_speed_address;//R轴 jog 速度
		float rotating_motor_speed;

		std::string lifting_axis_motor_alarm_address; //z轴报警地址
		short lifting_axis_alarm_code;


		std::string rotating_axis_motor_alarm_address; //R轴报警地址
		short rotating_axis_alarm_code;

		std::string lifting_axis_acce_address; //Z轴acc
		std::string lifting_axis_dece_address; //Z轴dce
		float lifting_axis_acce_value = 0.0f;
		float lifting_axis_dece_value = 0.0f;

		std::string rotating_axis_acce_address;//R轴acc
		std::string rotating_axis_dece_address;//R轴dce
		float rotating_axis_acce_value = 0.0f;
		float rotating_axis_dece_value = 0.0f;

		// z轴jerk
		std::string lifting_axis_jerk_address;
		float lifting_axis_jerk_value;

		// r轴jerk
		std::string rotating_axis_jerk_address;
		float rotating_axis_jerk_value;

		//lift pin位置角度 
		std::string rotating_axis_safe_angle_address;
		float rotating_axis_safe_angle_value;

	};

	/**
	* FortrendPMCavitySubsystemPrivate
	*/
	FortrendPMCavitySubsystemPrivate::FortrendPMCavitySubsystemPrivate(FortrendPMCavitySubsystem*p)
		:p(p){


	}

	bool FortrendPMCavitySubsystem::safe_read_bit(const std::string& tag, bool& output)
	{
		if (tag.empty()) return false;
		bool value;
		if (KeyencePlcSubSystemHelper::readBit(tag, value))
		{
			if (output != value)
			{
				output = value;
				return true;
			}
			return false;
		}
		else
		{
			return false;
		}
	}

	bool FortrendPMCavitySubsystem::safe_read_float(const std::string& tag, float& output)
	{
		if (tag.empty()) return false;
		float value;
		if (KeyencePlcSubSystemHelper::readFloat(tag, value))
		{
			if (output != value)
			{
				output = value;
				return true;
			}
			return false;
		}
		else
		{
			return false;
		}
	}

	bool FortrendPMCavitySubsystem::safe_read_double(const std::string& tag, double& output)
	{
		if (tag.empty()) return false;
		double value;
		if (KeyencePlcSubSystemHelper::readDouble(tag, value))
		{
			if (output != value)
			{
				output = value;
				return true;
			}
			return false;
		}
		else
		{
			return false;
		}
	}

	bool FortrendPMCavitySubsystem::safe_read_int(const std::string& tag, int& output)
	{
		if (tag.empty()) return false;
		int value;
		if (KeyencePlcSubSystemHelper::readInt(tag, value))
		{
			if (output != value)
			{
				output = value;
				return true;
			}
			return false;
		}
		else
		{
			return false;
		}
	}

	bool FortrendPMCavitySubsystem::safe_read_short(const std::string& tag, short& output)
	{
		if (tag.empty()) return false;
		short value;
		if (KeyencePlcSubSystemHelper::readShort(tag, value))
		{
			if (output != value)
			{
				output = value;
				return true;
			}
			return false;
		}
		else
		{
			return false;
		}
	}

	bool FortrendPMCavitySubsystem::safe_read_unsignedInt(const std::string& tag, uint32_t& output)
	{
		if (tag.empty()) return false;
		uint32_t value;
		if (KeyencePlcSubSystemHelper::readUnsignedInt(tag, value))
		{
			if (output != value)
			{
				output = value;
				return true;
			}
			return false;
		}
		else
		{
			return false;
		}
	}

	/**
	* FortrendPMCavitySubsystem
	*/
	FortrendPMCavitySubsystem::FortrendPMCavitySubsystem(IKernel*  kernel, const std::string& name)
		:AbstractIOSubsystem(kernel, name)
		, FortrendAbstractStation(kernel)
		, KeyencePlcSubSystemHelper(name)
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

	bool  FortrendPMCavitySubsystem::getVacuumEnable()const {
		return d->vacuum_enable;
	}

	bool FortrendPMCavitySubsystem::getWithWaferModeEnable()const{
		return d->with_wafer_mode;
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

	//PM腔伺服电机参数
	void FortrendPMCavitySubsystem::setPMCavityAxisParameters(const PMCavityAxisSettingParameters pm_param)
	{
		bool change = false;
		if (d->axis_setting_parameters.lifting_axis_acce != pm_param.lifting_axis_acce)
		{
			d->axis_setting_parameters.lifting_axis_acce = pm_param.lifting_axis_acce;
			change = true;
		}

		if (d->axis_setting_parameters.lifting_axis_dece != pm_param.lifting_axis_dece)
		{
			d->axis_setting_parameters.lifting_axis_dece = pm_param.lifting_axis_dece;

		}

		if (d->axis_setting_parameters.lifting_axis_startup_speed != pm_param.lifting_axis_startup_speed)
		{
			d->axis_setting_parameters.lifting_axis_startup_speed = pm_param.lifting_axis_startup_speed;
			change = true;
		}
		if (d->axis_setting_parameters.lifting_axis_target_position != pm_param.lifting_axis_target_position)
		{
			d->axis_setting_parameters.lifting_axis_target_position = pm_param.lifting_axis_target_position;
			change = true;
		}
		if (d->axis_setting_parameters.lifting_axis_target_pressure != pm_param.lifting_axis_target_pressure)
		{
			d->axis_setting_parameters.lifting_axis_target_pressure = pm_param.lifting_axis_target_pressure;
			change = true;
		}
		
		if (d->axis_setting_parameters.lifting_axis_target1_position != pm_param.lifting_axis_target1_position)
		{
			d->axis_setting_parameters.lifting_axis_target1_position = pm_param.lifting_axis_target1_position;
			change = true;
		}

		if (d->axis_setting_parameters.lifting_axis_target2_position != pm_param.lifting_axis_target2_position)
		{
			d->axis_setting_parameters.lifting_axis_target2_position = pm_param.lifting_axis_target2_position;
			change = true;
		}
		if (d->axis_setting_parameters.lifting_axis_target3_position != pm_param.lifting_axis_target3_position)
		{
			d->axis_setting_parameters.lifting_axis_target3_position = pm_param.lifting_axis_target3_position;
			change = true;
		}
		if (d->axis_setting_parameters.lifting_axis_target4_position != pm_param.lifting_axis_target4_position)
		{
			d->axis_setting_parameters.lifting_axis_target4_position = pm_param.lifting_axis_target4_position;
			change = true;
		}

		if (d->axis_setting_parameters.lifting_axis_jog_speed != pm_param.lifting_axis_jog_speed)
		{
			d->axis_setting_parameters.lifting_axis_jog_speed = pm_param.lifting_axis_jog_speed;
			change = true;
		}

		if (d->axis_setting_parameters.lifting_axis_inch_movement != pm_param.lifting_axis_inch_movement)
		{
			d->axis_setting_parameters.lifting_axis_inch_movement = pm_param.lifting_axis_inch_movement;
			change = true;
		}

		if (d->axis_setting_parameters.rotating_axis_acce != pm_param.rotating_axis_acce)
		{
			d->axis_setting_parameters.rotating_axis_acce = pm_param.rotating_axis_acce;
			change = true;
		}


		if (d->axis_setting_parameters.rotating_axis_dece != pm_param.rotating_axis_dece)
		{
			d->axis_setting_parameters.rotating_axis_dece = pm_param.rotating_axis_dece;
			change = true;
		}
		if (d->axis_setting_parameters.rotating_axis_startup_speed != pm_param.rotating_axis_startup_speed)
		{
			d->axis_setting_parameters.rotating_axis_startup_speed = pm_param.rotating_axis_startup_speed;
			change = true;
		}

		if (d->axis_setting_parameters.rotating_axis_target_position != pm_param.rotating_axis_target_position)
		{
			d->axis_setting_parameters.rotating_axis_target_position = pm_param.rotating_axis_target_position;
			change = true;
		}

		if (d->axis_setting_parameters.rotating_axis_jog_speed != pm_param.rotating_axis_jog_speed)
		{
			d->axis_setting_parameters.rotating_axis_jog_speed = pm_param.rotating_axis_jog_speed;
			change = true;
		}


		if (d->axis_setting_parameters.rotating_axis_inch_movement != pm_param.rotating_axis_inch_movement)
		{
			d->axis_setting_parameters.rotating_axis_inch_movement = pm_param.rotating_axis_inch_movement;
			change = true;
		}

		//z-jerk
		if (d->axis_setting_parameters.lifting_axis_jerk_value != pm_param.lifting_axis_jerk_value)
		{
			d->axis_setting_parameters.lifting_axis_jerk_value = pm_param.lifting_axis_jerk_value;
			change = true;
		}

		//r-jerk
		if (d->axis_setting_parameters.rotating_axis_jerk_value != pm_param.rotating_axis_jerk_value)
		{
			d->axis_setting_parameters.rotating_axis_jerk_value = pm_param.rotating_axis_jerk_value;
			change = true;
		}
		
		if (change)
		{
			d->pm_update_axis_setting_parameters = true;
			AbstractIOSubsystem::emitAttributeChanged(this);
		}
	}
	PMCavityAxisSettingParameters FortrendPMCavitySubsystem::getPMCavityAxisParameters()
	{
		d->pm_update_axis_setting_parameters = false;
		return d->axis_setting_parameters;
	}
	bool FortrendPMCavitySubsystem::getPMCavityUpdatAxisParameters()
	{
		return d->pm_update_axis_setting_parameters;
	}
	bool FortrendPMCavitySubsystem::getMinimumPlaneLevelSignal() const
	{
		return d->pm_cavity_motor_minimum_plane_signal;
	}
	bool FortrendPMCavitySubsystem::getMaximumPlaneLevelSignal() const
	{
		return d->pm_cavity_motor_maximum_plane_signal;
	}
	bool FortrendPMCavitySubsystem::getRotatingimumPlaneLevelSignal() const
	{
		return d->pm_cavity_motor_rotating_plane_signal;
	}
	bool FortrendPMCavitySubsystem::getLiftingPinPlaneLevelSignal() const
	{
		return d->pm_cavity_motor_liftpin_plane_signal;
	}
	float FortrendPMCavitySubsystem::getPmLiftingTargetPos() const
	{
		return d->axis_setting_parameters.lifting_axis_target_position;
	}
	float FortrendPMCavitySubsystem::getPmRotatingTargetPos() const
	{
		return d->axis_setting_parameters.rotating_axis_target_position;
	}

	float FortrendPMCavitySubsystem::getPmLiftPinSafeAnglePos() const
	{
		return d->rotating_axis_safe_angle_value;
	}
	/**

	* 获取镀膜时间
	*/
	float FortrendPMCavitySubsystem::getPMCavityCoatingTime(){
		float result = 0;
		if (d->pm_cavity_coating_time_address != "")
		{
			KeyencePlcSubSystemHelper::readFloat(d->pm_cavity_coating_time_address, result);
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
			KeyencePlcSubSystemHelper::readShort(d->pm_cavity_processing_step_address, result);
		}

		return result;
	}

	void FortrendPMCavitySubsystem::setPMCavityCraftCount(int count)
	{
		d->pm_craft_count = count;
	}

	int FortrendPMCavitySubsystem::getPMCavityCraftCount()
	{
		return d->pm_craft_count;
	}

	bool FortrendPMCavitySubsystem::getLiftingAxisPowerDone() const
	{
		return d->lifting_axis_enable_done;
	}

	void FortrendPMCavitySubsystem::setLiftingAxisPower(bool enable)
	{
		logInform(getName().c_str(), "写Z轴使能开始.");
		if(!KeyencePlcSubSystemHelper::writeBit(d->lifting_axis_enable_address, enable)) 
		{
			logError(getName().c_str(), Poco::format("写Z轴使能address = %s 失败!", d->lifting_axis_enable_address).c_str());
		}
		logInform(getName().c_str(), "触发Z轴使能结束.");
	}

	bool FortrendPMCavitySubsystem::getLiftingHomeDone() const
	{
		return d->axis_origin_done;
	}

	void FortrendPMCavitySubsystem::setLiftingHome(bool enable)
	{
		logInform(getName().c_str(),"Z轴回原开始.");
		if (!KeyencePlcSubSystemHelper::writeBit(d->lifting_axis_rest_address, enable))
		{
			logError(getName().c_str(), Poco::format("写Z轴回原address = %s 失败!", d->lifting_axis_rest_address).c_str());
		}
		logInform(getName().c_str(), "触发Z轴回原结束.");

	}

	bool FortrendPMCavitySubsystem::getZAxisAlarm() const
	{
		//!=0 :alarm
		return d->lifting_axis_alarm_code;
	}

	bool FortrendPMCavitySubsystem::getRAxisAlarm() const
	{
		//!=0 : alarm
		return d->rotating_axis_alarm_code;
	}

	bool FortrendPMCavitySubsystem::getZAxisClearErrorDone() const
	{
		return d->lifting_axis_clear_done;
	}

	void FortrendPMCavitySubsystem::setZAxisClearError(bool enable)
	{
		logInform(getName().c_str(), "写清除Z轴控报警开始.");
		if (!KeyencePlcSubSystemHelper::writeBit(d->lifting_axis_clear_error_address, enable)) {
		
			logError(getName().c_str(), Poco::format("写清除Z轴控报警address = %s 失败!", d->lifting_axis_clear_error_address).c_str());
		}
		logInform(getName().c_str(), "触发清除Z轴控报警完成.");
	}

	bool FortrendPMCavitySubsystem::getRAxisClearErrorDone() const
	{
		return d->rotating_axis_clear_error_done;
	}

	void FortrendPMCavitySubsystem::setRAxisClearError(bool enable)
	{
		logInform(getName().c_str(), "写清除R轴控报警开始.");
		if (!KeyencePlcSubSystemHelper::writeBit(d->rotating_axis_clear_error_address, enable))
		{
			logError(getName().c_str(), Poco::format("写清除R轴控报警address = %s 失败!", d->rotating_axis_clear_error_address).c_str());
		}
		logInform(getName().c_str(), "触发清除R轴控报警完成.");
	}

	bool FortrendPMCavitySubsystem::getRotationAxisPowerDone() const
	{
		return d->rotating_axis_enable_done;
	}
	void FortrendPMCavitySubsystem::setRotationAxisPower(bool enable)
	{
		logInform(getName().c_str(), "触发R轴使能开始.");
		if (KeyencePlcSubSystemHelper::writeBit(d->rotating_axis_enable_address, enable))
		{
			logError(getName().c_str(), Poco::format("写R轴控使能address = %s 失败!", d->rotating_axis_enable_address).c_str());
		}
		logInform(getName().c_str(), "触发R轴使能结束.");
	}

	bool FortrendPMCavitySubsystem::getRotationHomeDone() const
	{
		return d->rotating_axis_return_original_done;
	}

	void FortrendPMCavitySubsystem::setRotationHome(bool enable)
	{
		logInform(getName().c_str(), "写R轴回原开始.");
		if (!KeyencePlcSubSystemHelper::writeBit(d->rotating_axis_rest_address, enable))
		{
			logError(getName().c_str(), Poco::format("写R轴回原address = %s 失败!", d->rotating_axis_rest_address).c_str());
		}
	}

	float FortrendPMCavitySubsystem::getPMCavityZAxleSpeed() const
	{
		return d->lifting_axis_current_speed;
	}

	float FortrendPMCavitySubsystem::getPMCavityZAxleLocation() const
	{
		return d->lifting_axis_current_coordinate;
	}

	void FortrendPMCavitySubsystem::setPMCavityZAxleAcc(float accValue)
	{

		logInform(getName().c_str(), "写Z轴加速度.");
		if (!KeyencePlcSubSystemHelper::writeFloat(d->lifting_axis_acce_address, accValue))
		{
			logError(getName().c_str(), Poco::format("写Z轴加速度address = %s 失败!", d->lifting_axis_acce_address).c_str());
		}
	}

	void FortrendPMCavitySubsystem::setPMCavityZAxleDcc(float dccValue)
	{
		logInform(getName().c_str(), "写Z轴减速度.");
		if (!KeyencePlcSubSystemHelper::writeFloat(d->lifting_axis_dece_address, dccValue))
		{
			logError(getName().c_str(), Poco::format("写Z轴减速度address = %s 失败!", d->lifting_axis_dece_address).c_str());
		}
	}

	float FortrendPMCavitySubsystem::getPMCavityZAxleAcc() const
	{
		return d->lifting_axis_acce_value;
	}

	float FortrendPMCavitySubsystem::getPMCavityZAxleDcc() const
	{
		return d->lifting_axis_dece_value;
	}

	uint32_t FortrendPMCavitySubsystem::getPMCavityZAxleJerk() const
	{
		logInform(getName().c_str(), "读取R轴jerk.");
		return  d->rotating_axis_jerk_value;
	}

	void FortrendPMCavitySubsystem::setPMCavityZAxleJerk(uint32_t value)
	{
		logInform(getName().c_str(), "设置R轴jerk.");
		if (!KeyencePlcSubSystemHelper::writeUnsignedInt(d->rotating_axis_jerk_address, value))
		{
			logError(getName().c_str(), Poco::format("写R轴jerk address = %s 失败!", d->rotating_axis_jerk_address).c_str());
		}
	}

	double FortrendPMCavitySubsystem::getPMCavityRAxleSpeed() const
	{
		return d->rotating_axis_current_speed;
	}

	double FortrendPMCavitySubsystem::getPMCavityRAxleLocation() const
	{
		//logInform(getName().c_str(), Poco::format("获取r轴位置：address = %s, result = %f", d->rotating_axis_current_coordinate_addresss, 
		// d->rotating_axis_current_coordinate).c_str());
		return d->rotating_axis_current_coordinate;
	}

	float FortrendPMCavitySubsystem::getPMCavityRAxleAcc() const
	{
		return d->rotating_axis_acce_value;
	}

	float FortrendPMCavitySubsystem::getPMCavityRAxleDcc() const
	{
		return d->rotating_axis_dece_value;
	}

	void FortrendPMCavitySubsystem::setPMCavityRAxleAcc(float accValue)
	{
		logInform(getName().c_str(), "写R轴加速度.");
		if (!KeyencePlcSubSystemHelper::writeFloat(d->rotating_axis_acce_address, accValue))
		{
			logError(getName().c_str(), Poco::format("写R轴加速度address = %s 失败!", d->rotating_axis_acce_address).c_str());
		}
	}

	void FortrendPMCavitySubsystem::setPMCavityRAxleDcc(float dccValue)
	{
		logInform(getName().c_str(), "写R轴减速度.");
		if (!KeyencePlcSubSystemHelper::writeFloat(d->rotating_axis_dece_address, dccValue))
		{
			logError(getName().c_str(), Poco::format("写R轴减速度address = %s 失败!", d->rotating_axis_dece_address).c_str());
		}
	}

	uint32_t FortrendPMCavitySubsystem::getPMCavityZRxleJerk() const
	{
		logInform(getName().c_str(),"读取Z轴jerk.");
		return  d->lifting_axis_jerk_value;
	}

	void FortrendPMCavitySubsystem::setPMCavityRAxleJerk(uint32_t value)
	{
		logInform(getName().c_str(), "设置Z轴jerk.");
		if (!KeyencePlcSubSystemHelper::writeUnsignedInt(d->lifting_axis_jerk_address, value))
		{
			logError(getName().c_str(), Poco::format("写Z轴jerk address = %s 失败!", d->lifting_axis_jerk_address).c_str());
		}
	}

	bool FortrendPMCavitySubsystem::getZAxleJogRunning() const
	{
		return d->lifting_axis_jog_running;
	}

	bool FortrendPMCavitySubsystem::getZAxleAutoRunDone() const
	{
		return d->lifting_axis_move_done;
	}

	bool FortrendPMCavitySubsystem::getZAxleAutoRunning() const
	{
		return d->lifting_axis_moving;
	}

	bool FortrendPMCavitySubsystem::getRAxleStopDone() const
	{
		return d->rotating_axis_stop_done;
	}

	bool FortrendPMCavitySubsystem::getRAxleJogRunning() const
	{
		return d->rotating_axis_jog_running;
	}

	bool FortrendPMCavitySubsystem::getRAxleAutoDone() const
	{
		return d->rotating_axis_move_end;
	}

	bool FortrendPMCavitySubsystem::getRAxleAutoRunning() const
	{
		return  d->rotating_axis_moving;
	}


	//获取晶圆槽号
	int FortrendPMCavitySubsystem::getWaferSlot()const{
		return d->wafer_slot;
	}


	//设置晶圆槽号
	void FortrendPMCavitySubsystem::setWaferSlot(const int slot){
		d->wafer_slot = slot;
	}

	bool FortrendPMCavitySubsystem::getPMCavityGetRequest(){
		bool result = false;

		KeyencePlcSubSystemHelper::readBit(d->get_request_address, result);
		logInform1(getName().c_str(), Poco::format("获取取片请求address = %s, result = %d", d->get_request_address, (int)result).c_str());
		return result;
	}
	bool FortrendPMCavitySubsystem::getPMCavityUploadRequest(){
		bool result = false;
		KeyencePlcSubSystemHelper::readBit(d->upload_request_address, result);
		logInform1(getName().c_str(), Poco::format("获取上片请求address = %s, result = %d", d->upload_request_address, (int)result).c_str());
		return result;
	}

	void FortrendPMCavitySubsystem::setPMCavitySafeSignal(bool value){
		d->pm_cavity_safe = value;
	}

	//获取PM腔安全信号
	bool FortrendPMCavitySubsystem::getPMCavitySafeSignal(){
		return d->pm_cavity_safe;
	}

	//获取PM腔步进电机后退到位信号
	bool FortrendPMCavitySubsystem::getPMCavityMotorHomeSignal(){
		return d->pm_cavity_motor_home;
	}

	//获取PM腔步进电机前进到位信号
	bool FortrendPMCavitySubsystem::getPMCavityMotorForwardSignal(){

		return d->pm_cavity_motor_forward;
	}

	//获取PM腔步进电机运动信号
	bool FortrendPMCavitySubsystem::getPMCavityMotorRunSignal(){
		return d->pm_cavity_motor_run;
	}

	//获取PM腔是否处于远程模式
	bool FortrendPMCavitySubsystem::getPMCavityRemoteMode(){
		bool result = false;
		KeyencePlcSubSystemHelper::readBit(d->pm_remote_mode_address, result);
		logInform1(getName().c_str(), Poco::format("获取%s腔远程模式address = %s, result = %d", getName(), d->pm_remote_mode_address, (int)result).c_str());
		return result;
	}

	bool FortrendPMCavitySubsystem::readPMCavityHasObjectState(){
		bool result = false;
		KeyencePlcSubSystemHelper::readBit(d->pm_cavity_has_object_address, result);
		return result;
	}

	bool FortrendPMCavitySubsystem::getPMCavityHasAlarm(){//
		if (getState() != IKernelSubSystem::State::SUB_UNKNOWN)
		{
			//读取PM腔的PLC报警
			bool pm_alarm = false;

			if (KeyencePlcSubSystemHelper::readBit(d->pm_cavity_general_alarm_address, pm_alarm))
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
		KeyencePlcSubSystemHelper::readBits(d->pm_cavity_alarm_start_address, d->pm_cavity_read_alarm_length, alarm_address.get());
		for (size_t i = 0; i < d->pm_cavity_read_alarm_length; i++)
		{
			if (alarm_address[i])
			{
				auto alarm_message = KeyencePlcSubSystemHelper::getErrorCode(1, 900 + i);
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
		result = KeyencePlcSubSystemHelper::writeBit(d->pm_cavity_has_object_address, value);;
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

	//轴速度 
	double FortrendPMCavitySubsystem::getPMCavityAxleSpeed()const{
		return d->speed;
	}

	float FortrendPMCavitySubsystem::getPMCavityMotorSpeed()const{
		return d->lifting_motor_speed;
	}

	float FortrendPMCavitySubsystem::getPMCavityRAxisMotorSpeed() const
	{
		return d->rotating_motor_speed;
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

	//电机的自动速度
	void FortrendPMCavitySubsystem::setPMCavityAxleSpeed(float speed){
		KeyencePlcSubSystemHelper::writeFloat("DM6620", speed);
	}

	//电机的手动速度(jog+ ,jog-)
	void FortrendPMCavitySubsystem::setPMCavityTurnSpeed(float speed){
		//logInform("Test", "PM腔步进电机速度设置，地址：DM15080  值%.2f", speed);
		KeyencePlcSubSystemHelper::writeFloat("DM6632", speed);
	}

	void FortrendPMCavitySubsystem::setPMCavityRAxleSpeed(float speed)
	{
		KeyencePlcSubSystemHelper::writeFloat("DM6808", speed);
	}
	void FortrendPMCavitySubsystem::setsetPMCavityRAxleTurnSpeed(float speed)
	{
		KeyencePlcSubSystemHelper::writeFloat("DM6816", speed);
	}

	//jog+
	void FortrendPMCavitySubsystem::setPMCavityForward(bool forward){
		//logInform("Test", "PM腔步进电机前进，地址：MR8101  值%d", forward);
		if (getIsRunning()){
			logWarn(getName().c_str(), "自动模式下无法控制电机前进！");
		}
		else{
			KeyencePlcSubSystemHelper::writeBit("MR8101", forward);
		}		
	}
	//jog-
	void FortrendPMCavitySubsystem::setPMCavityBackward(bool backward){
		//logInform("Test", "PM腔步进电机后退，地址：MR8102  值%d", backward);
		KeyencePlcSubSystemHelper::writeBit("MR8102", backward);
	}
	void FortrendPMCavitySubsystem::setPMCavityRAxlePositive(bool Positive)
	{
		if (getIsRunning()) {
			logWarn(getName().c_str(), "自动模式下无法控制电机前进！");
		}
		else {
			KeyencePlcSubSystemHelper::writeBit("MR8501", Positive);
		}
	}
	void FortrendPMCavitySubsystem::setPMCavityRAxleNegative(bool Negative)
	{
		KeyencePlcSubSystemHelper::writeBit("MR8502", Negative);
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

			//PM腔安全信号
			io_changed |= safe_read_bit(d->pm_cavity_safe_address, d->pm_cavity_safe);

			//TM腔门
			io_changed |= safe_read_bit(d->open_tm_cavity_door_address, d->tm_cavity_door_opend);

			/*******************************************************升降轴******************************************************/
			//升降轴当前速度
			io_changed |= safe_read_float(d->lifting_axis_current_speed_addresss, d->lifting_axis_current_speed);
			//升降轴当前坐标
			io_changed |= safe_read_float(d->lifting_axis_current_coordinate_addresss, d->lifting_axis_current_coordinate);

			//清除轴错误完成
			io_changed |= safe_read_bit(d->lifting_axis_clear_error_completion_address, d->lifting_axis_clear_done);

			//升降轴回原完成
			io_changed |= safe_read_bit(d->lifting_axis_return_original_completion_address, d->axis_origin_done);

			//升降轴回原中
			io_changed |= safe_read_bit(d->lifting_axis_return_original_running_address, d->lifting_axis_return_original_running);

			//升降轴JOG运行中
			io_changed |= safe_read_bit(d->lifting_axis_jog_running_address, d->lifting_axis_jog_running);

			//升降轴使能ON
			io_changed |= safe_read_bit(d->lifting_axis_enable_address, d->lifting_axis_enable_done);

			//升降轴移动中
			io_changed |= safe_read_bit(d->lifting_axis_moving_address, d->lifting_axis_moving);

			//升降轴移动结束
			io_changed |= safe_read_bit(d->lifting_axis_move_end_address, d->lifting_axis_move_done);

			//升降轴反馈位置
			io_changed |= safe_read_float(d->lifting_axis_feedback_position_addresss, d->lifting_axis_feedback_position);

			//升降轴JOG速度
			io_changed |= safe_read_float(d->lifting_axis_motor_speed_address, d->lifting_motor_speed);

			//最低面位检测
			io_changed |= safe_read_bit(d->minimum_plane_level_detection_address,d->pm_cavity_motor_minimum_plane_signal);
			//中间旋转面位检测
			io_changed |= safe_read_bit(d->rotating_plane_level_detection_address, d->pm_cavity_motor_rotating_plane_signal);
			//最高面位检测
			io_changed |= safe_read_bit(d->maximum_plane_level_detection_address, d->pm_cavity_motor_maximum_plane_signal);
			//顶升销面位检测
			io_changed |= safe_read_bit(d->liftpin_plane_level_detection_address, d->pm_cavity_motor_liftpin_plane_signal);

			/*******************************************************旋转轴******************************************************/
			//清除轴错误完成
			io_changed |= safe_read_bit(d->rotating_axis_clear_error_completion_address, d->rotating_axis_clear_error_done);
			//轴停止完成
			io_changed |= safe_read_bit(d->rotating_axis_stop_completion_address, d->rotating_axis_stop_done);
			//JOG运行中
			io_changed |= safe_read_bit(d->rotating_axis_jog_running_address, d->rotating_axis_jog_running);
			//使能ON
			io_changed |= safe_read_bit(d->rotating_axis_enable_address, d->rotating_axis_enable_done);
			//移动中
			io_changed |= safe_read_bit(d->rotating_axis_moving_address, d->rotating_axis_moving);
			//移动结束
			io_changed |= safe_read_bit(d->rotating_axis_move_end_address, d->rotating_axis_move_end);
			
			//回原完成
			io_changed |= safe_read_bit(d->rotating_axis_return_original_completion_address, d->rotating_axis_return_original_done);

			//旋转轴运行速度
			io_changed |= safe_read_float(d->rotating_axis_current_speed_addresss, d->rotating_axis_current_speed);

			//旋转轴当前坐标
			io_changed |= safe_read_float(d->rotating_axis_current_coordinate_addresss, d->rotating_axis_current_coordinate);

			//旋转轴反馈位置
			io_changed |= safe_read_int(d->rotating_axis_feedback_position_addresss, d->rotating_axis_feedback_position);

			//旋转轴JOG速度
			io_changed |= safe_read_float(d->rotating_axis_motor_speed_address, d->rotating_motor_speed);

			//Z轴报警
			io_changed |= safe_read_short(d->lifting_axis_motor_alarm_address, d->lifting_axis_alarm_code);

			//R轴报警
			io_changed |= safe_read_short(d->rotating_axis_motor_alarm_address, d->rotating_axis_alarm_code);

			//Z轴加速度
			io_changed |= safe_read_float(d->lifting_axis_acce_address, d->lifting_axis_acce_value);

			//Z轴减速度
			io_changed |= safe_read_float(d->lifting_axis_dece_address, d->lifting_axis_dece_value);

			//R轴加速度
			io_changed |= safe_read_float(d->rotating_axis_acce_address, d->rotating_axis_acce_value);

			//R轴减速度
			io_changed |= safe_read_float(d->rotating_axis_dece_address, d->rotating_axis_dece_value);

			//jerk
			io_changed |= safe_read_float(d->lifting_axis_jerk_address, d->lifting_axis_jerk_value);
			//jerk
			io_changed |= safe_read_float(d->rotating_axis_jerk_address, d->rotating_axis_jerk_value);
			//lift pin angle
			io_changed |= safe_read_float(d->rotating_axis_safe_angle_address,d->rotating_axis_safe_angle_value);


			if (io_changed)
			{
				AbstractIOSubsystem::emitAttributeChanged(this);
			}
			Sleep(100);

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
				//logInform1(Poco::format("%s_vacuum", getName()).c_str(), Poco::format("%f", getVacuumValue()).c_str());
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
		if (config->has("LiftingAxisHome"))
		{
			d->lifting_axis_rest_address = config->getString("LiftingAxisHome.start_address","");
		}
		if (config->has("RotatingAxisHome"))
		{
			d->rotating_axis_rest_address = config->getString("RotatingAxisHome.start_address","");
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

			d->pm_remote_mode_address = config->getString("PMCavityRecardAddress.RemoteModeAddress", "M33");

			d->pm_cavity_general_alarm_address = config->getString("PMCavityRecardAddress.GeneralAlarmAddress", "");

			d->pm_cavity_alarm_start_address = config->getString("PMCavityRecardAddress.AlarmStartAddress", "");

			d->pm_cavity_read_alarm_length = config->getInt("PMCavityRecardAddress.ReadAlarmLength", 65);

			d->pm_cavity_coating_time_address = config->getString("PMCavityRecardAddress.CoatingTimeAddress", "");

			d->pm_cavity_processing_step_address = config->getString("PMCavityRecardAddress.ProcessingStepAddress", "");

			d->pm_cavity_motor_home_address = config->getString("PMCavityRecardAddress.MotorHomeAddress", "MR35105");

			d->pm_cavity_motor_forward_address=config->getString("PMCavityRecardAddress.MotorForwardAddress", "MR35104");
		}
		if (config->has("Update"))
		{
			d->pm_cavity_safe_address = config->getString("Update.PMCavitySafetySignal", "");//PM腔安全信号+_门阀开启to机械手

			d->lifting_axis_return_original_completion_address = config->getString("Update.lifting_axis_return_original_completion_address", "");
			d->lifting_axis_clear_error_completion_address = config->getString("Update.lifting_axis_clear_error_completion_address","");
			d->lifting_axis_jog_running_address = config->getString("Update.lifting_axis_jog_running_address", "");
			d->lifting_axis_return_original_running_address = config->getString("Update.lifting_axis_return_original_running_address", "");
			d->lifting_axis_enable_address = config->getString("Update.lifting_axis_enable_address", "");

			d->lifting_axis_moving_address = config->getString("Update.lifting_axis_moving_address", "");
			d->lifting_axis_move_end_address = config->getString("Update.lifting_axis_move_end_address", "");

			d->lifting_axis_feedback_position_addresss = config->getString("Update.lifting_axis_feedback_position_addresss", "");
			d->lifting_axis_current_coordinate_addresss = config->getString("Update.lifting_axis_current_coordinate_addresss", "");

			d->lifting_axis_current_speed_addresss = config->getString("Update.lifting_axis_current_speed_addresss", "");
			d->lifting_axis_current_control_mode_addresss = config->getString("Update.lifting_axis_current_control_mode_addresss", "");

			d->lifting_axis_feedback_torque_monitor_address = config->getString("Update.lifting_axis_feedback_torque_monitor_address", "");
			d->rotating_axis_return_original_completion_address = config->getString("Update.rotating_axis_return_original_completion_address", "");

			d->rotating_axis_clear_error_completion_address = config->getString("Update.rotating_axis_clear_error_completion_address", "");
			d->rotating_axis_stop_completion_address = config->getString("Update.rotating_axis_stop_completion_address", "");

			d->rotating_axis_jog_running_address = config->getString("Update.rotating_axis_jog_running_address", "");
			d->rotating_axis_return_original_running_address = config->getString("Update.rotating_axis_return_original_running_address", "");

			d->rotating_axis_enable_address = config->getString("Update.rotating_axis_enable_address", "");
			d->rotating_axis_moving_address = config->getString("Update.rotating_axis_moving_address", "");

			d->rotating_axis_move_end_address = config->getString("Update.rotating_axis_move_end_address", "");
			d->rotating_axis_feedback_position_addresss = config->getString("Update.rotating_axis_feedback_position_addresss", "");

			d->rotating_axis_current_coordinate_addresss = config->getString("Update.rotating_axis_current_coordinate_addresss", "");
			d->rotating_axis_current_speed_addresss = config->getString("Update.rotating_axis_current_speed_addresss", "");

			d->lifting_axis_motor_alarm_address = config->getString("Update.lifting_axis_alarm_address","");

			d->rotating_axis_motor_alarm_address = config->getString("Update.rotating_axis_alarm_address","");

			d->rotating_axis_safe_angle_address = config->getString("Update.rotating_axis_safe_Angle_address","");

		}
		if (config->has("AxisReadParameters"))
		{
			d->lifting_axis_motor_speed_address = config->getString("AxisReadParameters.lifting_axis_jog_speed_address","");
			d->rotating_axis_motor_speed_address = config->getString("AxisReadParameters.rotating_axis_jog_speed_address","");
			d->lifting_axis_acce_address = config->getString("AxisReadParameters.lifting_axis_acce_address","");
			d->lifting_axis_dece_address = config->getString("AxisReadParameters.lifting_axis_dece_address", "");
			d->rotating_axis_acce_address = config->getString("AxisReadParameters.rotating_axis_acce_address","");
			d->rotating_axis_dece_address = config->getString("AxisReadParameters.rotating_axis_dece_address", "");
			d->lifting_axis_jerk_address = config->getString("AxisReadParameters.lifting_axis_jerk_address","");
			d->rotating_axis_jerk_address = config->getString("AxisReadParameters.rotating_axis_jerk_address","");
		}

		if (config->has("Reset"))
		{
			d->rotating_axis_clear_error_address = config->getString("Reset.rotating_axis_start_address","");
			d->lifting_axis_clear_error_address = config->getString("Reset.lifting_axis_start_address","");
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


	std::shared_ptr<PMCavityLiftingActionCommand> FortrendPMCavitySubsystem::createLiftingActionCommand(double targetPos) const
	{
		FortrendPMCavitySubsystem* self = const_cast<FortrendPMCavitySubsystem*>(this);
		PMCavityLiftingActionCommand::Ptr ret(new PMCavityLiftingActionCommand(self,targetPos));
		return ret;
	}

	std::shared_ptr<PMCavityToGetStationCommand> FortrendPMCavitySubsystem::createToGetStationCommand()const{
		FortrendPMCavitySubsystem* self = const_cast<FortrendPMCavitySubsystem*>(this);
		PMCavityToGetStationCommand::Ptr ret(new PMCavityToGetStationCommand(self));
		return ret;
	}
	std::shared_ptr<PMCavityToPutStationCommand> FortrendPMCavitySubsystem::createToPutStationCommand()const{
		FortrendPMCavitySubsystem* self = const_cast<FortrendPMCavitySubsystem*>(this);
		PMCavityToPutStationCommand::Ptr ret(new PMCavityToPutStationCommand(self));
		return ret;
	}

	std::shared_ptr<PMCavityToRotatingStationCommand> FortrendPMCavitySubsystem::createToRotatingStationCommand() const
	{
		FortrendPMCavitySubsystem* self = const_cast<FortrendPMCavitySubsystem*>(this);
		PMCavityToRotatingStationCommand::Ptr ret(new PMCavityToRotatingStationCommand(self));
		return ret;
	}

	std::shared_ptr<PMCavityRotatingActionCommand> FortrendPMCavitySubsystem::createRotatingActionCommand(double degree, int model) const
	{
		FortrendPMCavitySubsystem* self = const_cast<FortrendPMCavitySubsystem*>(this);
		PMCavityRotatingActionCommand::Ptr ret(new PMCavityRotatingActionCommand(self, degree, model));
		return ret;
	}

	std::shared_ptr<PMCavityReadProcessParametersCommand>  FortrendPMCavitySubsystem::createReadProcessParametersCommand()const{
		FortrendPMCavitySubsystem* self = const_cast<FortrendPMCavitySubsystem*>(this);
		PMCavityReadProcessParametersCommand::Ptr ret(new PMCavityReadProcessParametersCommand(self));
		return ret;
	}

	std::shared_ptr<PMCavityWriteProcessParametersCommand>  FortrendPMCavitySubsystem::createWriteProcessParametersCommand(const PMCavityAxisSettingParameters axis_parames)const{
		FortrendPMCavitySubsystem* self = const_cast<FortrendPMCavitySubsystem*>(this);
		PMCavityWriteProcessParametersCommand::Ptr ret(new PMCavityWriteProcessParametersCommand(axis_parames, self));
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
	std::shared_ptr<PMCavityLiftingAxisHomeCommand> FortrendPMCavitySubsystem::createPMCavityLiftingAxisHomeCommand() const
	{
		FortrendPMCavitySubsystem* self = const_cast<FortrendPMCavitySubsystem*>(this);
		PMCavityLiftingAxisHomeCommand::Ptr ret(new PMCavityLiftingAxisHomeCommand(self));
		return ret;
	}
	std::shared_ptr<PMCavityRotatingAxisHomeCommand> FortrendPMCavitySubsystem::createPMCavityRotatingAxisHomeCommand() const
	{
		FortrendPMCavitySubsystem* self = const_cast<FortrendPMCavitySubsystem*>(this);
		PMCavityRotatingAxisHomeCommand::Ptr ret(new PMCavityRotatingAxisHomeCommand(self));
		return ret;
	}
}


