/**
* @file            fortrend_pm_cavity_defined.h
* @brief           Fortrend PM Cavity defined
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Subsystem/PMCavity


#ifndef _XLH_FORTREND_PM_CAVITY_SUBSYSTEM_DEFINED_INCLUDE_
#define _XLH_FORTREND_PM_CAVITY_SUBSYSTEM_DEFINED_INCLUDE_ 
#include <vector>
#include <map>

namespace FC {

	/**
	* @brief    PM Cavity process parameters
	* @struct    process parameters
	*/
	struct PMCavityProcessParameters
	{
		float heating_temperature = 0.0F;
		float initial_extraction_pressure = 0.0F;
		float purified_extraction_pressure = 0.0F;
		float sputtering_pressure = 0.0F;
		float sputtering_flow_rate1 = 0.0F;
		float sputtering_flow_rate2 = 0.0F;
		float sputtering_flow_rate3 = 0.0F;
		float sputtering_power1 = 0.0F;
		float sputtering_power_gear_up1 = 0.0F;
		float sputtering_power2 = 0.0F;
		float sputtering_power_gear_up2 = 0.0F;
		float sputtering_power3 = 0.0F;
		float sputtering_power_gear_up3 = 0.0F;
		float pre_sputtering_time = 0.0F;
		float substrate_speed = 0.0F;
		float process_sputtering_time = 0.0F;
		int cathode_power_selection_1 = 0;
		int cathode_power_selection_2 = 0;
		int cathode_power_selection_3 = 0;
	};

	struct PMCavityStateAddress
	{
		std::string temperature = "";
		std::string vacuum_value = "";
		std::string pre_stage_pippeline_vacuum = "";
		std::string sample_rotation_speed = "";
		std::string flow_meter_1 = "";
		std::string flow_meter_2 = "";
		std::string flow_meter_3 = "";
		std::string dc_vlotage_1 = "";
		std::string dc_vlotage_2 = "";
		std::string dc_vlotage_3 = "";
		std::string dc_current_1 = "";
		std::string dc_current_2 = "";
		std::string dc_current_3 = "";
		std::string dc_power_1 = "";
		std::string dc_power_2 = "";
		std::string dc_power_3 = "";

	};

	struct PMCavityMoniterStateFloat {
		std::string address;
		std::string name;
		float last_value;
		float current_value;
	};
	struct PMCavityMoniterStateShort {
		std::string address;
		std::string name;
		short last_value;
		short current_value;
	};
	struct PMCavityMoniterStateInt {
		std::string address;
		std::string name;
		int last_value;
		int current_value;
	};
	//PM腔状态监控
	struct PMCavityMoniterValue
	{

		float vacuum_value = 0.0;
		float pre_stage_pippeline_vacuum = 0.0;
		float sample_rotation_speed = 0.0;
		float flow_meter_1 = 0.0;
		float flow_meter_2 = 0.0;
		float flow_meter_3 = 0.0;


		short dc_vlotage_1 = 0;
		short dc_vlotage_2 = 0;
		short dc_vlotage_3 = 0;
		short dc_current_1 = 0;
		short dc_current_2 = 0;
		short dc_current_3 = 0;
		short dc_power_1 = 0;
		short dc_power_2 = 0;
		short dc_power_3 = 0;
		short temperature = 0;
	};

	/**
	* @brief    膜厚仪监控参数值
	* @struct    PMCavityMoniterFilmValue
	*/
	struct PMCavityMoniterFilmValue {
		int frequency = 0;			//频率
		int rate = 0;				//速率
		int accumulative_total = 0;	//累计膜厚
		int lifetime = 0;			//寿命

	};
	/*
	* @brief     运动过程参数值
	* @struct    PMMotionProcessParameters
	
	*/
	struct PMMotionProcessParameters
	{
		int cycle = 0;
		float lifting_axis_acce1 = 0.0F;
		float lifting_axis_acce2 = 0.0F;
		float lifting_axis_acce3 = 0.0F;
		float lifting_axis_acce4 = 0.0F;
		float rotating_axis_acce1 = 0.0F;
		float rotating_axis_acce2 = 0.0F;
		float rotating_axis_acce3 = 0.0F;
		float rotating_axis_acce4 = 0.0F;
	};




	/**
	* @brief    PM Cavity Axis Setting parameters
	* @struct    Axis Setting parameters
	*/
	struct PMCavityAxisSettingParameters
	{
		float lifting_axis_acce = 0.0F;
		float lifting_axis_dece = 0.0F;
		float lifting_axis_startup_speed = 0.0F;
		float lifting_axis_target_position = 0.0F;
		float lifting_axis_target_pressure = 0.0F;
		float lifting_axis_target1_position = 0.0F;
		float lifting_axis_target2_position = 0.0F;
		float lifting_axis_target3_position = 0.0F;
		float lifting_axis_jog_speed = 0.0F;
		float lifting_axis_inch_movement = 0.0F;
		float lifting_axis_jerk_value = 0.0F;
		float rotating_axis_acce = 0.0F;
		float rotating_axis_dece = 0.0F;
		float rotating_axis_startup_speed = 0.0F;
		float rotating_axis_jerk_value = 0.0F;
		float rotating_axis_target_position = 0.0F;
		float rotating_axis_jog_speed = 0.0F;
		float rotating_axis_inch_movement = 0.0F;
	};
	struct ParameterMapping {
		std::string config_key;
		float PMCavityAxisSettingParameters::* member_ptr;
		std::string description;
	};

	static const std::vector<ParameterMapping>& getMappingTable()
	{

		static const std::vector<ParameterMapping> mapping_table = {
			// 升降轴参数映射
		{ "lifting_axis_acce_address", &PMCavityAxisSettingParameters::lifting_axis_acce, "升降轴加速度" },
		{ "lifting_axis_dece_address", &PMCavityAxisSettingParameters::lifting_axis_dece, "升降轴减速度" },
		{ "lifting_axis_startup_speed_address", &PMCavityAxisSettingParameters::lifting_axis_startup_speed, "升降轴启动速度" },
		{ "lifting_axis_target_position_address", &PMCavityAxisSettingParameters::lifting_axis_target_position, "升降轴目标坐标" },
		{ "lifting_axis_target_pressure_address", &PMCavityAxisSettingParameters::lifting_axis_target_pressure, "升降轴目标压力" },
		{ "lifting_axis_target1_position_address", &PMCavityAxisSettingParameters::lifting_axis_target1_position, "升降轴位置1" },
		{ "lifting_axis_target2_position_address", &PMCavityAxisSettingParameters::lifting_axis_target2_position, "升降轴位置2" },
		{ "lifting_axis_target3_position_address", &PMCavityAxisSettingParameters::lifting_axis_target3_position, "升降轴位置3" },
		{ "lifting_axis_jog_speed_address", &PMCavityAxisSettingParameters::lifting_axis_jog_speed, "升降轴Jog速度" },
		{ "lifting_axis_inch_movement_address", &PMCavityAxisSettingParameters::lifting_axis_inch_movement, "升降轴寸动移动量" },
		{ "lifting_axis_jerk_address",&PMCavityAxisSettingParameters::lifting_axis_jerk_value,"升降轴jerk"},
		{ "rotating_axis_acce_address", &PMCavityAxisSettingParameters::rotating_axis_acce, "旋转轴加速度" },
		{ "rotating_axis_dece_address", &PMCavityAxisSettingParameters::rotating_axis_dece, "旋转轴减速度" },
		{ "rotating_axis_startup_speed_address", &PMCavityAxisSettingParameters::rotating_axis_startup_speed, "旋转轴启动速度" },
		{ "rotating_axis_jerk_address",&PMCavityAxisSettingParameters::rotating_axis_jerk_value,"旋转轴jerk"},
		{ "rotating_axis_target_position_address", &PMCavityAxisSettingParameters::rotating_axis_target_position, "旋转轴目标坐标" },
		{ "rotating_axis_jog_speed_address", &PMCavityAxisSettingParameters::rotating_axis_jog_speed, "旋转轴Jog速度" },
		{ "rotating_axis_inch_movement_address", &PMCavityAxisSettingParameters::rotating_axis_inch_movement, "旋转轴寸动移动量" }
		};
		return mapping_table;
	}


	

}

#endif
