
/**
* @file            fortrend_loadlock_subsystem.h
* @brief           Fortrend LoadLock
* @author		   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/LoadLock

#include <iostream>
#include <Windows.h>

#include "Poco/Format.h"
#include "Poco/StringTokenizer.h"
#include "Poco/NumberParser.h"

#include "kernel/kernel.h"
#include "Kernel/kernel_log.h"
#include "kernel/kernel_iocontrol.h"
#include "kernel/kernel_configure.h"

#include "LoadLock/fortrend_loadlock_subsystem.h" 
#include "LoadLock/fortrend_loadlock_reset_command.h"
#include "LoadLock/fortrend_loadlock_update_command.h"
#include "LoadLock/fortrend_loadlock_output_command.h"

#include "kernel/Fortrend/fortrend_cassette_manager.h"

#if _MSC_VER >1600
#pragma execution_character_set("utf-8")
#endif

namespace FC{
	/**
	* FortrendLoadLockSubsystemPrivate
	*/
	class FortrendLoadLockSubsystemPrivate{
	public:
		FortrendLoadLockSubsystemPrivate(FortrendLoadLockSubsystem*p);
		void setInput(uint8_t index, bool stat);
	public:
		FortrendLoadLockSubsystem* p;

		// present sensor
		std::vector<std::string> io_present_sensor_names;
		std::string io_present_sensor_address = "";

		bool *ptr_io_present_sensor_state;
		std::vector<bool> io_present_sensor_last_value; //左右晶圆检测结果

		//protruding sensor
		std::string io_protruding_sensor_names = "";
		std::string io_protruding_sensor_address = "";
		bool io_protruding_sensor_last_value = false; //突出检测信号

		//Layer detection sensor  
		std::string io_first_layer_detection_sensor_address = "";
		bool io_first_layer_wafer_presence_value = false;

		std::string io_second_layer_detection_sensor_address = "";
		bool io_second_layer_wafer_presence_value = false;

		std::string io_safeSignal_address = "";
		bool  io_safeSignal_value = false;

		
		bool box_placement = false;
		bool cassette_door_opend = false;
		bool tm_cavity_door_opend = false;
		bool slow_diaphragm_valve_opend = false;//慢充隔膜阀状态
		bool fast_diaphragm_valve_opend = false;//快充隔膜阀状态
		bool angle_valve_opend = false;
		bool exhaust_valve_opend = false;
		bool vacuum_enable = false;
		bool with_wafer_mode = false;

		bool ultrahigh_vacuum_baffle_valve_opend = false;
		bool inserting_plate_valve_opend = false;

		int cassette_slot_count = 13;

		int last_move_slot = 0;

		std::string vacuum_read_value_address = "";
		double vacuum_extraction_set_value = 1.3;       //真空抽气设定值
		double vacuum_upper_limit_set_value = 8.0;      //真空上限设定值
		double vacuum_break_set_value = 99600.0;        //真空破气设定值

		double vacuum_current_value = 100000.0;		    //真空当前值
		std::string vacuum_pressure_gage_address = "";  //真空压力表信号地址
		int vacuum_pressure_gage_state = -1;			//真空压力表信号

		double rough_vacuum_set_value = 6.0;        //粗抽设定值
		double vacuum_diaphragm_valve_fast_charge_set_value = 5000.0;	   //隔膜阀快充真空设定值
		double vacuum_angle_valve_fast_vacuumization_set_value = 10000.0;	//角阀快充真空设定值

		std::string diaphragm_valve_address1 = "";
		std::string diaphragm_valve_address2 = "";
		std::string high_vacuum_baffle_value_address = "";
		std::string open_angle_valve_address = "";
		std::string close_angle_valve_address = "";
		std::string open_inserting_plate_valve_address = "";
		std::string close_inserting_plate_valve_address = "";

		std::string open_cassette_door_address = "";
		std::string open_tm_cavity_door_address = "";

		bool has_reset_flag = false;	//已复位标志

		

		bool recard_enabled = false;
		std::thread thd_recard_vacuum;
	};

	/**
	* FortrendLoadLockSubsystemPrivate
	*/
	FortrendLoadLockSubsystemPrivate::FortrendLoadLockSubsystemPrivate(FortrendLoadLockSubsystem*p)
		:p(p){

	}

	/**
	* FortrendLoadLockSubsystem
	*/
	FortrendLoadLockSubsystem::FortrendLoadLockSubsystem(IKernel*  kernel, const std::string& name)
		:AbstractIOSubsystem(kernel, name)
		, FortrendAbstractStation(kernel)
		, KeyencePlcSubSystemHelper(name)
		, d(new FortrendLoadLockSubsystemPrivate(this)){
		//init 
		d->recard_enabled = true;
		d->thd_recard_vacuum = std::thread([this] { this->recardVacuum(); });
		
	}
	
	bool FortrendLoadLockSubsystem::hasBoxPlacement()const
	{
		return true;
		//return d->box_placement;
	}
	bool FortrendLoadLockSubsystem::hasBoxPresent()const
	{
		return true;
		/*bool res = true;
		for (const bool state : d->io_present_sensor_last_value)
		{
			if (!state)
			{
				res = false;
				break;
			}
		}
		return  res;*/
	}

	bool FortrendLoadLockSubsystem::hasDoorOpend()const{
		return d->tm_cavity_door_opend;
	}

	int FortrendLoadLockSubsystem::inputCount()const 
	{
		return helperInputCount();
	}

	int FortrendLoadLockSubsystem::outputCount()const
	{
		return helperOutputCount();
	}

	bool FortrendLoadLockSubsystem::getInput(int index)const
	{
		return false;
	}

	std::string FortrendLoadLockSubsystem::getInputName(int index)const
	{
		return getHelperInputName(index);
	}

	std::string FortrendLoadLockSubsystem::getOutputName(int index)const
	{
		return getHelperOutputName(index);
	}

	bool FortrendLoadLockSubsystem::getCassetteDoorOpend()const{
		return d->cassette_door_opend;
	}
	void FortrendLoadLockSubsystem::setCassetteDoorOpend(const bool value){
		if (d->cassette_door_opend != value)
		{
			d->cassette_door_opend = value;
			AbstractIOSubsystem::emitAttributeChanged(this);
		}
	}
	bool FortrendLoadLockSubsystem::getTMCavityDoorOpend()const{
		return d->tm_cavity_door_opend;
	}
	void FortrendLoadLockSubsystem::setTMCavityDoorOpend(const bool value){
		if (d->tm_cavity_door_opend != value)
		{
			d->tm_cavity_door_opend = value;
			AbstractIOSubsystem::emitAttributeChanged(this);
		}
	}
	bool FortrendLoadLockSubsystem::getSlowDiaphragmValveOpend()const{
		return d->slow_diaphragm_valve_opend;
	}
	void FortrendLoadLockSubsystem::setSlowDiaphragmValveOpend(const bool value){
		if (d->slow_diaphragm_valve_opend != value)
		{
			d->slow_diaphragm_valve_opend = value;
			AbstractIOSubsystem::emitAttributeChanged(this);
		}
	}
	bool FortrendLoadLockSubsystem::getFastDiaphragmValveOpend()const{
		return d->fast_diaphragm_valve_opend;
	}
	void FortrendLoadLockSubsystem::setFastDiaphragmValveOpend(const bool value){
		if (d->fast_diaphragm_valve_opend != value)
		{
			d->fast_diaphragm_valve_opend = value;
			AbstractIOSubsystem::emitAttributeChanged(this);
		}
	}
	bool FortrendLoadLockSubsystem::getAngleValveOpend()const{
		return d->angle_valve_opend;
	}
	void FortrendLoadLockSubsystem::setAngleValveOpend(const bool value){
		if (d->angle_valve_opend != value)
		{
			d->angle_valve_opend = value;
			AbstractIOSubsystem::emitAttributeChanged(this);
		}
	}
	bool FortrendLoadLockSubsystem::getExhaustValveOpend()const{
		return d->exhaust_valve_opend;
	}
	void FortrendLoadLockSubsystem::setExhaustValveOpend(const bool value){
		if (d->exhaust_valve_opend != value)
		{
			d->exhaust_valve_opend = value;
			AbstractIOSubsystem::emitAttributeChanged(this);
		}
	}

	bool FortrendLoadLockSubsystem::getPresentSensorState(const int index){
		return true;
		//return d->io_present_sensor_last_value[index];
	}

	bool FortrendLoadLockSubsystem::getProtrudingSensorState()const{
		//return true;
		return d->io_protruding_sensor_last_value;
	}

	void FortrendLoadLockSubsystem::setBoxPlacement(const bool value)
	{
		d->box_placement = value;
	}

	int FortrendLoadLockSubsystem::getLastMoveSlot() const
	{
		return d->last_move_slot;
	}

	void FortrendLoadLockSubsystem::setLastMoveSlot(const int value)
	{
		d->last_move_slot = value;
	}

	int FortrendLoadLockSubsystem::getCassetteSlotCount() const
	{
		return d->cassette_slot_count;
	}

	double FortrendLoadLockSubsystem::getVacuumValue()const{
		//return 100000;
		return d->vacuum_current_value;
	}
	
	int FortrendLoadLockSubsystem::getVacuumPressureGageState()const{
		return d->vacuum_pressure_gage_state;
	}

	bool FortrendLoadLockSubsystem::getVacuumValueReachesTheSetValue()const{
		if (d->vacuum_current_value < d->vacuum_extraction_set_value)
		{
			return true;
		}
		return false;
	}

	bool FortrendLoadLockSubsystem::getVacuumValueUpperLimitReachesTheSetValue()const{
		if (d->vacuum_enable == false)
		{
			return true;
		}
		if (d->vacuum_current_value < d->vacuum_upper_limit_set_value)
		{
			return true;
		}
		//logInform(getName().c_str(), Poco::format("当前压力值：%f,设定的上限压力值：%f", d->vacuum_current_value, d->vacuum_upper_limit_set_value).c_str());
		return false;
	}

	/**
	真空值达到大气设定值
	*/
	bool FortrendLoadLockSubsystem::getExhaustVacuumValueReachesTheSetValue()const{
		if (d->vacuum_current_value > d->vacuum_break_set_value)
		{
			return true;
		}
		return false;
	}



	/*
	*达到LoadLock粗抽压力,看分子泵工作条件判断
	*/
	bool FortrendLoadLockSubsystem::getLoadLockRoughVacuumReachesTheSetValue()const{
		return d->vacuum_current_value < d->rough_vacuum_set_value;
	}
	/*
	*达到LoadLock粗抽压力,看分子泵工作条件判断，添加补偿值
	*/
	bool FortrendLoadLockSubsystem::getLoadLockRoughVacuumReachesTheSetValue(int value)const{
		return d->vacuum_current_value < (d->rough_vacuum_set_value + value);
	}

	/**
	隔膜阀快充条件
	*/
	bool FortrendLoadLockSubsystem::getQuickInflationValueReachesTheSetValue()const{
		return d->vacuum_current_value > d->vacuum_diaphragm_valve_fast_charge_set_value;
	}

	/**
	*@brief  角阀快抽条件
	*/
	bool FortrendLoadLockSubsystem::getFastAngleValveReachesTheSetValue() const{
		return d->vacuum_current_value >= d->vacuum_angle_valve_fast_vacuumization_set_value;
	}

	/**
	*@brief  设置快充隔膜阀和快抽角阀真空值
	*/
	void FortrendLoadLockSubsystem::setVacuumFastDiapgragmValueAndAngleValue(const double diaphragm_value, const double angle_value){
		d->vacuum_diaphragm_valve_fast_charge_set_value = diaphragm_value;
		d->vacuum_angle_valve_fast_vacuumization_set_value = angle_value;
		logInform(getName().c_str(), Poco::format("%s 设置快充隔膜阀真空设定值：%f, 快抽角阀真空设定值：%f", getName(),
			d->vacuum_diaphragm_valve_fast_charge_set_value, d->vacuum_angle_valve_fast_vacuumization_set_value).c_str());
	}
	/**
	*@brief  设置粗抽完成真空值
	*/
	void FortrendLoadLockSubsystem::setRoughVacuumValue(const double rough_vacuum_set_value){
		d->rough_vacuum_set_value = rough_vacuum_set_value;
		logInform(getName().c_str(), Poco::format("%s 设置粗抽完成真空设定值：%f", getName(),
			d->rough_vacuum_set_value).c_str());
	}

	//设置真空上限值和抽真空设定值
	void FortrendLoadLockSubsystem::setVacuumUpperLimitAndExtractionValue(const double upper_limit_value, const double extraction_value){
		d->vacuum_upper_limit_set_value = upper_limit_value;
		d->vacuum_extraction_set_value = extraction_value;
		logInform(getName().c_str(), Poco::format("%s 设置真空上限值：%f, 抽真空设定值：%f",getName(), d->vacuum_upper_limit_set_value, d->vacuum_extraction_set_value).c_str());
	}

	bool FortrendLoadLockSubsystem::getHeightVacuumBaffleValveOpend()const{
		return d->ultrahigh_vacuum_baffle_valve_opend;
	}
	void FortrendLoadLockSubsystem::setHeightVacuumBaffleValveOpend(const bool value){
		if (d->ultrahigh_vacuum_baffle_valve_opend != value)
		{
			d->ultrahigh_vacuum_baffle_valve_opend = value;
			AbstractIOSubsystem::emitAttributeChanged(this);
		}
	}
	bool FortrendLoadLockSubsystem::getInsertingPlateValveOpend()const{
		return d->inserting_plate_valve_opend;
	}
	void FortrendLoadLockSubsystem::setInsertingPlateValveOpend(const bool value)
	{
		if (d->inserting_plate_valve_opend != value)
		{
			d->inserting_plate_valve_opend = value;
			AbstractIOSubsystem::emitAttributeChanged(this);
		}
	}

	bool  FortrendLoadLockSubsystem::getVacuumEnable()const{
		return d->vacuum_enable;
	}

	void FortrendLoadLockSubsystem::setVacuumEnable(const bool value){
		if (d->vacuum_enable != value)
		{
			d->vacuum_enable = value;
		}
	}
	bool FortrendLoadLockSubsystem::getWithWaferModeEnable()const{
		return d->with_wafer_mode;
	}

	void FortrendLoadLockSubsystem::setWithWaferModeEnable(const bool value){
		if (d->with_wafer_mode != value)
		{
			d->with_wafer_mode = value;
		}
	}

	void FortrendLoadLockSubsystem::onInitialize()throw(KernelException){

		if (SIMULATION_TEST == 1)
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

	void FortrendLoadLockSubsystem::onUnInitialize()throw(KernelException){
		d->recard_enabled = false;
		disableProtocol();
		if (d->thd_recard_vacuum.joinable())
		{
			d->thd_recard_vacuum.join();
		}
	}

	void FortrendLoadLockSubsystem::onProcess(){

#if 0
		if (getState() != IKernelSubSystem::State::SUB_UNKNOWN)
		{
			bool io_changed = false;


			bool signal_value = false;
			if (d->io_safeSignal_address != "" && readBit(d->io_safeSignal_address, signal_value))
			{
				if (signal_value != d->io_safeSignal_value)
				{
					d->io_safeSignal_value = signal_value;
					io_changed = true;
				}

			}

			bool wafer_prsence = false;		
			if (d->io_first_layer_detection_sensor_address != "" && readBit(d->io_first_layer_detection_sensor_address, wafer_prsence))
			{
				if (wafer_prsence != d->io_first_layer_wafer_presence_value)
				{
					io_changed = true;
					d->io_first_layer_wafer_presence_value = wafer_prsence;
				}
			}

			if (d->io_second_layer_detection_sensor_address != "" && readBit(d->io_second_layer_detection_sensor_address, wafer_prsence))
			{
				if (wafer_prsence != d->io_second_layer_wafer_presence_value)
				{
					io_changed = true;
					d->io_second_layer_wafer_presence_value = wafer_prsence;
				}
			}
			
#pragma region 自动更新门阀状态 
			bool flag = false;
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
			flag = d->angle_valve_opend;
			if (d->open_angle_valve_address != ""&&readBit(d->open_angle_valve_address, d->angle_valve_opend))
			{
				if (flag != d->angle_valve_opend)
					io_changed = true;
			}
			flag = d->cassette_door_opend;
			if (d->open_cassette_door_address != ""&&readBit(d->open_cassette_door_address, d->cassette_door_opend))
			{
				if (flag != d->cassette_door_opend)
					io_changed = true;
			}

			flag = d->tm_cavity_door_opend;
			if (d->open_tm_cavity_door_address != ""&&readBit(d->open_tm_cavity_door_address, d->tm_cavity_door_opend))
			{
				if (flag != d->tm_cavity_door_opend)
					io_changed = true;
			}

#pragma endregion

			if (d->vacuum_read_value_address != "")
			{
				double buff_vacuum = 0.0;
				if (readDouble(d->vacuum_read_value_address, buff_vacuum) && d->vacuum_current_value != buff_vacuum)
				{
					d->vacuum_current_value = buff_vacuum;
					io_changed = true;
				}
			}
			if (d->vacuum_pressure_gage_address != "")
			{
				bool buff_vacuum_pressure_gage = false;
				int buff_vacuum_pressure_gage_state = -1;
				if (readBit(d->vacuum_pressure_gage_address, buff_vacuum_pressure_gage))
				{
					if (buff_vacuum_pressure_gage)
					{
						buff_vacuum_pressure_gage_state = 1;
					}
					else
					{
						buff_vacuum_pressure_gage_state = 0;
					}
				}
				if (buff_vacuum_pressure_gage_state != d->vacuum_pressure_gage_state)
				{
					d->vacuum_pressure_gage_state = buff_vacuum_pressure_gage_state;
					io_changed = true;
				}
			}
			if (io_changed)
			{
				AbstractIOSubsystem::emitAttributeChanged(this);
			}
			Sleep(50);
			
		}
	
#endif

}

	void FortrendLoadLockSubsystem::recardVacuum()const{

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

	bool FortrendLoadLockSubsystem::getHasResetFlag()const{
		return d->has_reset_flag;
	}
	void FortrendLoadLockSubsystem::setHasResetFlag(const bool value){
		d->has_reset_flag = value;
	}

	void FortrendLoadLockSubsystem::getFirstLayerMapping(Cassette::Mapping &map)
	{
		short first_wafer_res = 0;
		first_wafer_res = (d->io_first_layer_wafer_presence_value) ? 2:1;
		Cassette::Mapping slotState = handleSingleSlotMapping(first_wafer_res);
		map = slotState;
	}

	bool FortrendLoadLockSubsystem::setFirstLayerMapping(const bool value)
	{
		d->io_first_layer_wafer_presence_value = value;
		return true;
	}

	void FortrendLoadLockSubsystem::getSecondLayerMapping(Cassette::Mapping &map)
	{
		short second_wafer_res = 0;
		second_wafer_res = (d->io_second_layer_wafer_presence_value) ? 2 : 1;
		Cassette::Mapping slotState = handleSingleSlotMapping(second_wafer_res);
		map = slotState;
	}

	bool FortrendLoadLockSubsystem::setSecondLayerMapping(const bool value)
	{
		d->io_second_layer_wafer_presence_value = value;
		return true;
	}
	//返回mapping 结果
	Cassette::Mapping FortrendLoadLockSubsystem::handleSingleSlotMapping(short mapRes)
	{
		Cassette::Mapping slotState = Cassette::Mapping::Unknown;
		switch (mapRes)
		{
			case 1:
				slotState = Cassette::Mapping::Empty;
				break;
			case 2:
				slotState = Cassette::Mapping::Present;
				break;
			default:
				slotState = Cassette::Mapping::Unknown;
			break;
		}
		return slotState;
	}

	bool FortrendLoadLockSubsystem::getLoadLockCavitySafeSignal()
	{

		return d->io_safeSignal_value;
	}

	void FortrendLoadLockSubsystem::setLoadLockCavitySafeSignal(const bool value)
	{
		d->io_safeSignal_value = value;
	}

	void FortrendLoadLockSubsystem::onConfigure(const std::shared_ptr<KernelConfiguration> & config){
		KernelAbstractSubSystem::onConfigure(config);
		FortrendAbstractStation::configure(config);
		configKeyencePlc(config);
		
		if (config->has("IO"))
		{
			//mapping Present Sensor Names
			//do{
			//	Poco::StringTokenizer token(config->getString("IO.PresentSensorNames", ""), ",", Poco::StringTokenizer::TOK_IGNORE_EMPTY);
			//	for (int i = 0; i < token.count(); i++){
			//		std::cout <<"token:"<< token[i] << std::endl;

			//		d->io_present_sensor_names.push_back(token[i]);

			//	}
			//} while (false);
			//d->io_present_sensor_address = config->getString("IO.PresentSensorAddress", "");//LoadLock1/在位感应器1

			//int count = d->io_present_sensor_names.size();

			//d->ptr_io_present_sensor_state = new bool[count];

			//for (size_t i = 0; i < d->io_present_sensor_names.size(); i++)
			//{
			//	d->io_present_sensor_last_value.push_back(false);
			//	d->ptr_io_present_sensor_state[i] = false;
			//}
			//
			//d->io_protruding_sensor_names = config->getString("IO.ProtrudingSensorName", "Protruding Detection Sensor"); //突出检测传感器
			//d->io_protruding_sensor_address = config->getString("IO.ProtrudingSensorAddress", "");
		}
		if (config->has("Vacuum"))
		{
			d->vacuum_read_value_address = config->getString("Vacuum.ReadValueAddress", "");
			d->vacuum_pressure_gage_address = config->getString("Vacuum.PressureGageAddress", "");
			//d->vacuum_extraction_set_value = config->getDouble("Vacuum.ExtractionSetValue", 1.3);
			//d->vacuum_upper_limit_set_value = config->getDouble("Vacuum.VacuumUpperLimitSetValue", 8);
			d->vacuum_break_set_value = config->getDouble("Vacuum.BreakSetValue", 103000.0);
		}
		if (config->has("slotCount"))
		{
			d->cassette_slot_count = config->getInt("slotCount", 13);
		}

		if (config->has("Update"))
		{
			d->diaphragm_valve_address1 = config->getString("Update.diaphragm_valve_address1", "");
			d->diaphragm_valve_address2 = config->getString("Update.diaphragm_valve_address2", "");
			d->high_vacuum_baffle_value_address = config->getString("Update.high_vacuum_baffle_value_address", "");
			d->open_angle_valve_address = config->getString("Update.open_angle_valve_address", "");
			d->close_angle_valve_address = config->getString("Update.close_angle_valve_address", "");
			d->open_inserting_plate_valve_address = config->getString("Update.open_inserting_plate_valve_address", "");
			d->close_inserting_plate_valve_address = config->getString("Update.close_inserting_plate_valve_address", "");
			d->open_cassette_door_address = config->getString("Update.open_cassette_door_address", "");
			d->open_tm_cavity_door_address = config->getString("Update.open_tm_cavity_door_address", "");

			d->io_first_layer_detection_sensor_address = config->getString("Update.first_layer_detection_sensor","");
			d->io_second_layer_detection_sensor_address = config->getString("Update.second_layer_detection_sensor", "");

			d->io_safeSignal_address = config->getString("Update.LLCavitySafetySignal","");
		}
	}


	std::shared_ptr<KernelSubsystemResetCommand> FortrendLoadLockSubsystem::createResetCommand()const{
		FortrendLoadLockSubsystem* self = const_cast<FortrendLoadLockSubsystem*>(this);
		LoadLockResetCommand::Ptr ret(new LoadLockResetCommand(self));
		return ret;
	}

	std::shared_ptr<KernelSubsystemUpdateCommand> FortrendLoadLockSubsystem::createUpdateCommand()const{
		FortrendLoadLockSubsystem* self = const_cast<FortrendLoadLockSubsystem*>(this);
		LoadLockUpdateCommand::Ptr ret(new LoadLockUpdateCommand(self));
		return ret;
	}

	std::shared_ptr<LoadLockClearErrorCommand> FortrendLoadLockSubsystem::createClearErrorCommand()const{
		FortrendLoadLockSubsystem* self = const_cast<FortrendLoadLockSubsystem*>(this);
		LoadLockClearErrorCommand::Ptr ret(new LoadLockClearErrorCommand(self));
		return ret;
	}

	std::shared_ptr<AbstractOutPutCommand>  FortrendLoadLockSubsystem::createOutputCommand(int channel, bool stat)const{
		FortrendLoadLockSubsystem* self = const_cast<FortrendLoadLockSubsystem*>(this);
		AbstractOutPutCommand::Ptr ret(new LoadLockOutputCommand(channel, stat, self));
		return ret;
	}

	std::shared_ptr<LoadLockOpenCassetteDoorCommand>  FortrendLoadLockSubsystem::createOpenCassetteDoorCommand()const{
		FortrendLoadLockSubsystem* self = const_cast<FortrendLoadLockSubsystem*>(this);
		LoadLockOpenCassetteDoorCommand::Ptr ret(new LoadLockOpenCassetteDoorCommand(self));
		return ret;
	}

	std::shared_ptr<LoadLockCloseCassetteDoorCommand>  FortrendLoadLockSubsystem::createCloseCassetteDoorCommand()const{
		FortrendLoadLockSubsystem* self = const_cast<FortrendLoadLockSubsystem*>(this);
		LoadLockCloseCassetteDoorCommand::Ptr ret(new LoadLockCloseCassetteDoorCommand(self));
		return ret;
	}

	std::shared_ptr<LoadLockMappingCommand>  FortrendLoadLockSubsystem::createMappingCommand()const{
		FortrendLoadLockSubsystem* self = const_cast<FortrendLoadLockSubsystem*>(this);
		LoadLockMappingCommand::Ptr ret(new LoadLockMappingCommand(self));
		return ret;
	}

	std::shared_ptr<LoadLockMoveToSlotCommand>  FortrendLoadLockSubsystem::createMoveToSlotCommand(const int slot)const{
		FortrendLoadLockSubsystem* self = const_cast<FortrendLoadLockSubsystem*>(this);
		LoadLockMoveToSlotCommand::Ptr ret(new LoadLockMoveToSlotCommand(slot, self));
		return ret;
	}

	std::shared_ptr<LoadLockOpenTMCavityDoorCommand> FortrendLoadLockSubsystem::createOpenTMCavityDoorCommand()const{
		FortrendLoadLockSubsystem* self = const_cast<FortrendLoadLockSubsystem*>(this);
		LoadLockOpenTMCavityDoorCommand::Ptr ret(new LoadLockOpenTMCavityDoorCommand(self));
		return ret;
	}

	std::shared_ptr<LoadLockCloseTMCavityDoorCommand> FortrendLoadLockSubsystem::createCloseTMCavityDoorCommand()const{
		FortrendLoadLockSubsystem* self = const_cast<FortrendLoadLockSubsystem*>(this);
		LoadLockCloseTMCavityDoorCommand::Ptr ret(new LoadLockCloseTMCavityDoorCommand(self));
		return ret;
	}

	//隔膜阀
	std::shared_ptr<LoadLockOpenDiaphragmValveCommand> FortrendLoadLockSubsystem::createOpenDiaphragmValveCommand(const LoadLockValveOpening opening)const{
		FortrendLoadLockSubsystem* self = const_cast<FortrendLoadLockSubsystem*>(this);
		LoadLockOpenDiaphragmValveCommand::Ptr ret(new LoadLockOpenDiaphragmValveCommand(opening, self));
		return ret;
	}

	std::shared_ptr<LoadLockCloseDiaphragmValveCommand> FortrendLoadLockSubsystem::createCloseDiaphragmValveCommand(const LoadLockValveOpening opening)const{
		FortrendLoadLockSubsystem* self = const_cast<FortrendLoadLockSubsystem*>(this);
		LoadLockCloseDiaphragmValveCommand::Ptr ret(new LoadLockCloseDiaphragmValveCommand(opening,self));
		return ret;
	}

	//排气阀
	std::shared_ptr<LoadLockOpenExhaustValveCommand> FortrendLoadLockSubsystem::createOpenExhaustValveCommand()const{
		FortrendLoadLockSubsystem* self = const_cast<FortrendLoadLockSubsystem*>(this);
		LoadLockOpenExhaustValveCommand::Ptr ret(new LoadLockOpenExhaustValveCommand(self));
		return ret;
	}

	std::shared_ptr<LoadLockCloseExhaustValveCommand> FortrendLoadLockSubsystem::createCloseExhaustValveCommand()const{
		FortrendLoadLockSubsystem* self = const_cast<FortrendLoadLockSubsystem*>(this);
		LoadLockCloseExhaustValveCommand::Ptr ret(new LoadLockCloseExhaustValveCommand(self));
		return ret;
	}
	//角阀
	std::shared_ptr<LoadLockOpenAngleValveCommand> FortrendLoadLockSubsystem::createOpenAngleValveCommand()const{
		FortrendLoadLockSubsystem* self = const_cast<FortrendLoadLockSubsystem*>(this);
		LoadLockOpenAngleValveCommand::Ptr ret(new LoadLockOpenAngleValveCommand(self));
		return ret;
	}

	std::shared_ptr<LoadLockCloseAngleValveCommand> FortrendLoadLockSubsystem::createCloseAngleValveCommand()const{
		FortrendLoadLockSubsystem* self = const_cast<FortrendLoadLockSubsystem*>(this);
		LoadLockCloseAngleValveCommand::Ptr ret(new LoadLockCloseAngleValveCommand(self));
		return ret;
	}

	//插板阀
	std::shared_ptr<LoadLockOpenInsertingPlateValveCommand> FortrendLoadLockSubsystem::createOpenInsertingPlateValveCommand()const{
		FortrendLoadLockSubsystem* self = const_cast<FortrendLoadLockSubsystem*>(this);
		LoadLockOpenInsertingPlateValveCommand::Ptr ret(new LoadLockOpenInsertingPlateValveCommand(self));
		return ret;
	}
	std::shared_ptr<LoadLockCloseInsertingPlateValveCommand> FortrendLoadLockSubsystem::createCloseInsertingPlateValveCommand()const{
		FortrendLoadLockSubsystem* self = const_cast<FortrendLoadLockSubsystem*>(this);
		LoadLockCloseInsertingPlateValveCommand::Ptr ret(new LoadLockCloseInsertingPlateValveCommand(self));
		return ret;
	}
	//挡板阀
	std::shared_ptr<LoadLockOpenHeightVacuumBaffleValveCommand> FortrendLoadLockSubsystem::createOpenHeightVacuumBaffleValveCommand()const{
		FortrendLoadLockSubsystem* self = const_cast<FortrendLoadLockSubsystem*>(this);
		LoadLockOpenHeightVacuumBaffleValveCommand::Ptr ret(new LoadLockOpenHeightVacuumBaffleValveCommand(self));
		return ret;
	}
	std::shared_ptr<LoadLockCloseHeightVacuumBaffleValveCommand> FortrendLoadLockSubsystem::createCloseHeightVacuumBaffleValveCommand()const{
		FortrendLoadLockSubsystem* self = const_cast<FortrendLoadLockSubsystem*>(this);
		LoadLockCloseHeightVacuumBaffleValveCommand::Ptr ret(new LoadLockCloseHeightVacuumBaffleValveCommand(self));
		return ret;
	}
	std::shared_ptr<LoadLockAutoBreakVacuumCommand> FortrendLoadLockSubsystem::createAutoBreakVacuumCommand()const{
		FortrendLoadLockSubsystem* self = const_cast<FortrendLoadLockSubsystem*>(this);
		LoadLockAutoBreakVacuumCommand::Ptr ret(new LoadLockAutoBreakVacuumCommand(self));
		return ret;
	}
}

