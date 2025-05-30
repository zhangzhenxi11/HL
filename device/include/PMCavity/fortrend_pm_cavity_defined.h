/**
* @file            fortrend_pm_cavity_defined.h
* @brief           Fortrend PM Cavity defined
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Subsystem/PMCavity


#ifndef _XLH_FORTREND_PM_CAVITY_SUBSYSTEM_DEFINED_INCLUDE_
#define _XLH_FORTREND_PM_CAVITY_SUBSYSTEM_DEFINED_INCLUDE_ 

namespace FC{

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

	struct PMCavityMoniterStateFloat{
		std::string address;
		std::string name;
		float last_value;
		float current_value;
	};
	struct PMCavityMoniterStateShort{
		std::string address;
		std::string name;
		short last_value;
		short current_value;
	};
	struct PMCavityMoniterStateInt{
		std::string address;
		std::string name;
		int last_value;
		int current_value;
	};
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
	struct PMCavityMoniterFilmValue{
		int frequency = 0;			//频率
		int rate = 0;				//速率
		int accumulative_total = 0;	//累计膜厚
		int lifetime = 0;			//寿命

	};
}

#endif
