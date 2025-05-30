
/**
* @file            fortrend_pump_subsystem.h
* @brief           Fortrend Pump
* @author		   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/Pump

#include <iostream>
#include <chrono>

#include "Poco/Format.h"
#include "Poco/StringTokenizer.h"
#include "Poco/NumberParser.h"

#include "kernel/kernel.h"
#include "kernel/kernel_listener.h"
#include "Kernel/kernel_log.h"
#include "kernel/kernel_iocontrol.h"
#include "kernel/kernel_configure.h"
#include "kernel/kernel_command_runner.h"
 
#include "Pump/fortrend_pump_reset_command.h"
#include "Pump/fortrend_pump_update_command.h"
#include "Pump/fortrend_pump_output_command.h"

#include "Pump/fortrend_pump_subsystem.h" 
#include "LoadLock/fortrend_loadlock_subsystem.h"
#include "TMCavity/fortrend_tm_cavity_subsystem.h"

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

namespace FC{
/**
* FortrendPumpSubsystemPrivate
*/
class FortrendPumpSubsystemPrivate{
public:
	FortrendPumpSubsystemPrivate(FortrendPumpSubsystem*p);
	void setInput(uint8_t index, bool stat);
public:
	FortrendPumpSubsystem* p;

	int wait_count = 0;

	bool vacuum_enable = false;					 //真空模式使能
	bool with_wafer_mode = false;				 //带晶圆模式
	bool mechanical_pump_opened = false;		 //机械泵打开状态

	bool molecular_pump_opened_lla = false;			 //分子泵打开状态
	bool molecular_pump_opened_llb = false;			 //分子泵打开状态
	bool molecular_pump_opened_tm = false;			 //分子泵打开状态

	int molecular_pump_state_lla = -1;			 //分子泵状态
	int molecular_pump_state_llb = -1;			 //分子泵状态
	int molecular_pump_state_tm = -1;			 //分子泵状态

	bool automatic_running_state_tm = false;		//自动运行状态
	bool automatic_running_state_lla = false;		//自动运行状态
	bool automatic_running_state_llb = false;		//自动运行状态

	bool plc_has_connected = false;				//plc已连接
	bool molecular_pump_has_connected = false; //分子泵已连接
	bool add_alarm_message = true;

	int molecular_pump_rev = 0;                  //分子泵转速
	std::shared_ptr<FortrendTMCavitySubsystem> tm;
	std::shared_ptr<FortrendLoadLockSubsystem> lk1;
	std::shared_ptr<FortrendLoadLockSubsystem> lk2;

	bool molecular_pump_reach_speed_lla = false;   //LLA分子泵达到转速
	bool molecular_pump_reach_speed_llb = false;  //LLB分子泵达到转速
	bool molecular_pump_reach_speed_tm = false;	  //TM分子泵达到转速

	std::chrono::system_clock::time_point angle_valve_close_time_clock_lla;  //LLA分子泵打开角阀关闭状态计时
	std::chrono::system_clock::time_point angle_valve_close_time_clock_llb;  //LLB分子泵打开角阀关闭状态计时
	std::chrono::system_clock::time_point angle_valve_close_time_clock_tm;  //TM分子泵打开角阀关闭状态计时
	bool angle_valve_close_time_update_lla = true;							 //LLA角阀关闭更新计时
	bool angle_valve_close_time_update_llb = true;							 //LLB角阀关闭更新计时
	bool angle_valve_close_time_update_tm = true;							 //TM角阀关闭更新计时

	std::string mechanical_pump_alarm_address = "";						 //机械报警地址
	std::string mechanical_pump_running_address = "";
	
	bool isVacuum = false;                                               //是否屏蔽真空模式,true真空模式，false屏蔽真空模式

	std::string molecular_pump_alarm_address_lla = "";			 //LLA分子泵报警地址
	std::string molecular_pump_alarm_address_llb = "";			 //LLB分子泵报警地址
	std::string molecular_pump_alarm_address_tm = "";			 //TM分子泵报警地址

	std::string molecular_pump_running_address_lla = "";			 //LLA分子泵运行中地址
	std::string molecular_pump_running_address_llb = "";			 //LLB分子泵运行中地址
	std::string molecular_pump_running_address_tm = "";			 //TM分子泵运行中地址

	std::string molecular_pump_acc_address_lla = "";			 //LLA分子泵达到额定转速地址
	std::string molecular_pump_acc_address_llb = "";			 //LLB分子泵达到额定转速地址
	std::string molecular_pump_acc_address_tm = "";			 //TM分子泵达到额定转速地址

	bool molecular_pump_alarm_lla =false;			 //LLA分子泵报警
	bool molecular_pump_alarm_llb =false;			 //LLB分子泵报警
	bool molecular_pump_alarm_tm =false;			 //TM分子泵报警

	bool molecular_pump_running_lla =false;			 //LLA分子泵运行中
	bool molecular_pump_running_llb =false;			 //LLB分子泵运行中
	bool molecular_pump_running_tm =false;			 //TM分子泵运行中

	bool molecular_pump_acc_lla =false;			 //LLA分子泵达到额定转速
	bool molecular_pump_acc_llb =false;			 //LLB分子泵达到额定转速
	bool molecular_pump_acc_tm =false;			 //TM分子泵达到额定转速
};

/**
* FortrendPumpSubsystemPrivate
*/
FortrendPumpSubsystemPrivate::FortrendPumpSubsystemPrivate(FortrendPumpSubsystem*p)
	:p(p){

}

/**
* FortrendPumpSubsystem
*/
FortrendPumpSubsystem::FortrendPumpSubsystem(IKernel*  kernel, const std::string& name)
	:AbstractIOSubsystem(kernel, name)
	, FortrendAbstractStation(kernel)
	, KeyencePlcSubSystemHelper(name)
	, d(new FortrendPumpSubsystemPrivate(this)){
	//init 
	
} 

void FortrendPumpSubsystem::onInitialize()throw(KernelException){
	try{
		if (KeyencePlcSubSystemHelper::enableProtocol())
			setState(IKernelSubSystem::State::SUB_NORMAL);
		else
			setState(IKernelSubSystem::State::SUB_UNKNOWN);

	}
	catch (KernelException& e){
		logError(getName().c_str(), e.what());
		//throw e;
	}
}

void FortrendPumpSubsystem::onUnInitialize()throw(KernelException){
	KeyencePlcSubSystemHelper::disableProtocol();
}

void FortrendPumpSubsystem::onProcess(){
	//if (!d->isVacuum)return;//屏蔽真空
	//分子泵保护
	if (!d->tm)
	{
		d->tm = getKernel()->getKernelModule<FortrendTMCavitySubsystem>("TM");
	}
	if (!d->lk1)
	{
		d->lk1 = getKernel()->getKernelModule<FortrendLoadLockSubsystem>("LLA");
	}
	if (!d->lk2)
	{
		d->lk2 = getKernel()->getKernelModule<FortrendLoadLockSubsystem>("LLB");
	}

	//软件刚打开后会立即关闭分子泵
	if (d->wait_count < 20)
	{
		Sleep(100);
		++d->wait_count;
		return;
	}

#pragma region TM分子泵逻辑
	if (d->molecular_pump_opened_tm)//TM分子泵打开
	{
		
		if (d->tm->getInsertingPlateValveOpend())//TM插板阀打开
		{
			double tm_vacuum = d->tm->getTMCavityVacuumValue();
			if (tm_vacuum > 200.0)
			{
				logError(getName().c_str(), Poco::format("腔室压力过高，关闭分子泵,传输腔当前压力：%f", tm_vacuum).c_str());

				auto cmd = createMolecularCloseCommand("TM");
				cmd->setClient(this);
				KernelCommandRunner runner(this->getKernel(), cmd, this);
				runner.runCmd();

				logError(getName().c_str(), Poco::format("腔室压力过高，关闭分子泵,传输腔当前压力：%f", tm_vacuum).c_str());
				//setMolecularPumpOpenedTM(false);
				setMolecularPumpReachSpeedTM(false);
			}
		}
		else//TM插板阀关闭
		{
			//if (d->tm->getAngleValveOpend())//TM角阀打开
			//{
			//	
			//	double tm_vacuum = d->tm->getTMCavityVacuumValue();
			//	double lk1_vacuum = d->lk1->getVacuumValue();
			//	double lk2_vacuum = d->lk2->getVacuumValue();
			//	bool close = false;
			//	if (tm_vacuum > 200.0  && d->tm->getHeightVacuumBaffleValveOpend())
			//	{
			//		close = true;
			//		logError(getName().c_str(), Poco::format("腔室压力过高，关闭分子泵,传输腔当前压力：%f", tm_vacuum).c_str());
			//	} 
			//	if (d->lk1->getAngleValveOpend() && lk1_vacuum > 200.0)
			//	{
			//		close = true;
			//		logError(getName().c_str(), Poco::format("腔室压力过高，关闭分子泵,LoadLock1腔当前压力：%f", lk1_vacuum).c_str());
			//	} 
			//	if (d->lk2->getAngleValveOpend() && lk2_vacuum > 200.0)
			//	{
			//		close = true;
			//		logError(getName().c_str(), Poco::format("腔室压力过高，关闭分子泵,LoadLock2腔当前压力：%f", lk2_vacuum).c_str());
			//	}
			//	if (close)
			//	{
			//		//关闭TM分子泵
			//		auto cmd = createMolecularCloseCommand("TM");
			//		cmd->setClient(this);
			//		KernelCommandRunner runner(this->getKernel(), cmd, this);
			//		runner.runCmd();

			//		//setMolecularPumpOpenedTM(false);
			//		setMolecularPumpReachSpeedTM(false);
			//	}
			//}
		}

		if (d->tm->getAngleValveOpend())
		{
			d->angle_valve_close_time_update_tm = true;
		}
		else//TM角阀关闭
		{
			if (d->angle_valve_close_time_update_tm)
			{
				d->angle_valve_close_time_update_tm = false;
				d->angle_valve_close_time_clock_tm = std::chrono::system_clock::now();
			}
			int pass = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - d->angle_valve_close_time_clock_tm).count();
			if (pass > (40 * 60 * 1000))
			{
				logError(getName().c_str(), "检测到TM分子泵在角阀关闭时的运行时长已超过20分钟，为防止分子泵异常，关闭分子泵");
				//关闭TM分子泵
				auto cmd = createMolecularCloseCommand("TM");
				cmd->setClient(this);
				KernelCommandRunner runner(this->getKernel(), cmd, this);
				runner.runCmd();
				//setMolecularPumpOpenedTM(false);
				setMolecularPumpReachSpeedTM(false);
				d->angle_valve_close_time_update_tm = true;
			}
		}
	}
#pragma endregion

#pragma region LLA分子泵逻辑
	if (d->molecular_pump_opened_lla)//LLA分子泵打开
	{

		if (d->lk1->getInsertingPlateValveOpend())//LLA插板阀打开
		{
			double lk1_vacuum = d->lk1->getVacuumValue();
			if (lk1_vacuum > 200.0)
			{
				logError(getName().c_str(), Poco::format("LoadLockA腔室压力过高，关闭分子泵,当前压力：%f", lk1_vacuum).c_str());

				auto cmd = createMolecularCloseCommand("LLA");
				cmd->setClient(this);
				KernelCommandRunner runner(this->getKernel(), cmd, this);
				runner.runCmd();

				logError(getName().c_str(), Poco::format("LoadLockA腔室压力过高，关闭分子泵,当前压力：%f", lk1_vacuum).c_str());
				//setMolecularPumpOpenedLLA(false);
				setMolecularPumpReachSpeedLLA(false);
			}
		}
		else//LLA插板阀关闭
		{
			//if (d->lk1->getAngleValveOpend())//LLA角阀打开
			//{
			//	double lk1_vacuum = d->lk1->getVacuumValue();

			//	if (d->lk1->getAngleValveOpend() && lk1_vacuum > 200.0)
			//	{
			//		//关闭TM分子泵
			//		auto cmd = createMolecularCloseCommand("LLA");
			//		cmd->setClient(this);
			//		KernelCommandRunner runner(this->getKernel(), cmd, this);
			//		runner.runCmd();
			//		logError(getName().c_str(), Poco::format("腔室压力过高，关闭分子泵,LoadLock1腔当前压力：%f", lk1_vacuum).c_str());
			//		//setMolecularPumpOpenedLLA(false);
			//		setMolecularPumpReachSpeedLLA(false);
			//	}
			//}
		}
		if (d->lk1->getAngleValveOpend())
		{
			d->angle_valve_close_time_update_lla = true;
		}
		else//角阀关闭时监控分子泵运行时间是否超过20分钟
		{
			if (d->angle_valve_close_time_update_lla)
			{
				d->angle_valve_close_time_update_lla = false;
				d->angle_valve_close_time_clock_lla = std::chrono::system_clock::now();
			}
			int pass = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - d->angle_valve_close_time_clock_lla).count();
			if (pass > (40 * 60 * 1000))
			{
				logError(getName().c_str(), "检测到LoadLock1分子泵在角阀关闭时的运行时长已超过20分钟，为防止分子泵异常，关闭分子泵");

				//关闭TM分子泵
				auto cmd = createMolecularCloseCommand("LLA");
				cmd->setClient(this);
				KernelCommandRunner runner(this->getKernel(), cmd, this);
				runner.runCmd();
				//setMolecularPumpOpenedLLA(false);
				setMolecularPumpReachSpeedLLA(false);
				d->angle_valve_close_time_update_lla = true;
			}
		}
	}
#pragma endregion

#pragma region LLB分子泵逻辑
	if (d->molecular_pump_opened_llb)//LLB分子泵打开
	{

		if (d->lk2->getInsertingPlateValveOpend())//LLB插板阀打开
		{
			double lk2_vacuum = d->lk2->getVacuumValue();
			if (lk2_vacuum > 200.0)
			{
				logError(getName().c_str(), Poco::format("LoadLockB腔室压力过高，关闭分子泵,当前压力：%f", lk2_vacuum).c_str());

				auto cmd = createMolecularCloseCommand("LLB");
				cmd->setClient(this);
				KernelCommandRunner runner(this->getKernel(), cmd, this);
				runner.runCmd();

				logError(getName().c_str(), Poco::format("LoadLockB腔室压力过高，关闭分子泵,当前压力：%f", lk2_vacuum).c_str());
				//setMolecularPumpOpenedLLB(false);
				setMolecularPumpReachSpeedLLB(false);
			}
		}
		else//LLB插板阀关闭
		{
			//if (d->lk2->getAngleValveOpend())//LLB角阀打开
			//{
			//	double lk2_vacuum = d->lk2->getVacuumValue();

			//	if (d->lk2->getAngleValveOpend() && lk2_vacuum > 200.0)
			//	{
			//		//关闭TM分子泵
			//		auto cmd = createMolecularCloseCommand("LLB");
			//		cmd->setClient(this);
			//		KernelCommandRunner runner(this->getKernel(), cmd, this);
			//		runner.runCmd();
			//		logError(getName().c_str(), Poco::format("腔室压力过高，关闭分子泵,LoadLock2腔当前压力：%f", lk2_vacuum).c_str());
			//		//setMolecularPumpOpenedLLB(false);
			//		setMolecularPumpReachSpeedLLB(false);
			//	}
			//}
		}
		if (d->lk2->getAngleValveOpend())
		{
			d->angle_valve_close_time_update_llb = true;
		}
		else//角阀关闭时监控分子泵运行时间是否超过20分钟
		{
			if (d->angle_valve_close_time_update_llb)
			{
				d->angle_valve_close_time_update_llb = false;
				d->angle_valve_close_time_clock_llb = std::chrono::system_clock::now();
			}
			int pass = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - d->angle_valve_close_time_clock_llb).count();
			if (pass > (40 * 60 * 1000))
			{
				logError(getName().c_str(), "检测到LoadLock2分子泵在角阀关闭时的运行时长已超过20分钟，为防止分子泵异常，关闭分子泵");

				//关闭TM分子泵
				auto cmd = createMolecularCloseCommand("LLB");
				cmd->setClient(this);
				KernelCommandRunner runner(this->getKernel(), cmd, this);
				runner.runCmd();
				//setMolecularPumpOpenedLLB(false);
				setMolecularPumpReachSpeedLLB(false);
				d->angle_valve_close_time_update_llb = true;
			}
		}
	}
#pragma endregion
	
	updateMolecularPumpState();
}

void FortrendPumpSubsystem::onConfigure(const std::shared_ptr<KernelConfiguration> & config){
	KernelAbstractSubSystem::onConfigure(config);
	
	configKeyencePlc(config);
	if (config->has("Update"))
	{
		d->mechanical_pump_alarm_address = config->getString("Update.MechanicalPumpAlarmAddress", "MR8000");
		d->mechanical_pump_running_address = config->getString("Update.MechanicalPumpRunningAddress", "MR8000");
	}
	if (config->has("IsVacuum")){
		d->isVacuum = config->getBool("IsVacuum", false);
	}

	if (config->has("acc_tm")){
		d->molecular_pump_acc_address_tm = config->getString("acc_tm", "MR30324");
	}
	if (config->has("acc_lla")){
		d->molecular_pump_acc_address_lla = config->getString("acc_lla", "MR30306");
	}
	if (config->has("acc_llb")){
		d->molecular_pump_acc_address_llb = config->getString("acc_llb", "MR30315");
	}

	if (config->has("running_tm")){
		d->molecular_pump_running_address_tm = config->getString("running_tm", "MR30326");
	}
	if (config->has("running_lla")){
		d->molecular_pump_running_address_lla = config->getString("running_lla", "MR30308");
	}
	if (config->has("running_llb")){
		d->molecular_pump_running_address_llb = config->getString("running_llb", "MR30317");
	}

	if (config->has("alarm_tm")){
		d->molecular_pump_alarm_address_tm = config->getString("alarm_tm", "MR30325");
	}
	if (config->has("alarm_lla")){
		d->molecular_pump_alarm_address_lla = config->getString("alarm_lla", "MR30307");
	}
	if (config->has("alarm_llb")){
		d->molecular_pump_alarm_address_llb = config->getString("alarm_llb", "MR30316");
	}

}

bool FortrendPumpSubsystem::clearAlarm(){
	setState(IKernelSubSystem::State::SUB_NORMAL);
	return true;
}

void FortrendPumpSubsystem::updateMolecularPumpState(){
	readBit(d->molecular_pump_alarm_address_lla,d->molecular_pump_alarm_lla);
	readBit(d->molecular_pump_alarm_address_llb, d->molecular_pump_alarm_llb);
	readBit(d->molecular_pump_alarm_address_tm, d->molecular_pump_alarm_tm);

	readBit(d->molecular_pump_running_address_lla, d->molecular_pump_running_lla);
	readBit(d->molecular_pump_running_address_llb, d->molecular_pump_running_llb);
	readBit(d->molecular_pump_running_address_tm, d->molecular_pump_running_tm);

	readBit(d->molecular_pump_acc_address_lla, d->molecular_pump_acc_lla);
	readBit(d->molecular_pump_acc_address_llb, d->molecular_pump_acc_llb);
	readBit(d->molecular_pump_acc_address_tm, d->molecular_pump_acc_tm);

	readBit(d->mechanical_pump_running_address, d->mechanical_pump_opened);

	if (d->mechanical_pump_opened != getMechanicalPumpOpened()){
		setMechanicalPumpOpened(d->mechanical_pump_opened);
	}

	if (d->molecular_pump_alarm_lla != (getMolecularPumpRunningStateLLA() == 2)){
		setMolecularPumpRunningStateLLA(2);
	}
	if (d->molecular_pump_alarm_llb != (getMolecularPumpRunningStateLLB() == 2)){
		setMolecularPumpRunningStateLLB(2);
	}
	if (d->molecular_pump_alarm_tm != (getMolecularPumpRunningStateTM() == 2)){
		setMolecularPumpRunningStateTM(2);
	}

	if (d->molecular_pump_acc_lla != getMolecularPumpReachSpeedLLA()){
		setMolecularPumpReachSpeedLLA(d->molecular_pump_acc_lla);
	}
	if (d->molecular_pump_acc_llb != getMolecularPumpReachSpeedLLB()){
		setMolecularPumpReachSpeedLLB(d->molecular_pump_acc_llb);
	}
	if (d->molecular_pump_acc_tm != getMolecularPumpReachSpeedTM()){
		setMolecularPumpReachSpeedTM(d->molecular_pump_acc_tm);
	}

	if (d->molecular_pump_running_lla != (getMolecularPumpRunningStateLLA()==1)){
		setMolecularPumpRunningStateLLA(1);
	}
	if (d->molecular_pump_running_llb != (getMolecularPumpRunningStateLLB() == 1)){
		setMolecularPumpRunningStateLLB(1);
	}
	if (d->molecular_pump_running_tm != (getMolecularPumpRunningStateTM() == 1)){
		setMolecularPumpRunningStateTM(1);
	}
	setMolecularPumpOpenedTM(d->molecular_pump_running_tm);
	setMolecularPumpOpenedLLA(d->molecular_pump_running_lla);
	setMolecularPumpOpenedLLB(d->molecular_pump_running_llb);
}


/*
*获取分子泵运行状态(0:待机,1:运行,其他:错误)
*/
int FortrendPumpSubsystem::getMolecularPumpRunningStateTM()const{
	return d->molecular_pump_state_tm;
}

void FortrendPumpSubsystem::setMolecularPumpRunningStateTM(const int value){

	if (value != d->molecular_pump_state_tm)
	{
		d->molecular_pump_state_tm = value;
		AbstractIOSubsystem::emitAttributeChanged(this);
	}
}
int FortrendPumpSubsystem::getMolecularPumpRunningStateLLA()const{
	return d->molecular_pump_state_lla;
}

void FortrendPumpSubsystem::setMolecularPumpRunningStateLLA(const int value){

	if (value != d->molecular_pump_state_lla)
	{
		d->molecular_pump_state_lla = value;
		AbstractIOSubsystem::emitAttributeChanged(this);
	}
}
int FortrendPumpSubsystem::getMolecularPumpRunningStateLLB()const{
	return d->molecular_pump_state_llb;
}

void FortrendPumpSubsystem::setMolecularPumpRunningStateLLB(const int value){

	if (value != d->molecular_pump_state_llb)
	{
		d->molecular_pump_state_llb = value;
		AbstractIOSubsystem::emitAttributeChanged(this);
	}
}

void FortrendPumpSubsystem::setAutomaticRunningStateTM(const bool value){
	if (d->automatic_running_state_tm != value)
	{
		d->automatic_running_state_tm = value;
	}

}
bool FortrendPumpSubsystem::getAutomaticRunningStateTM() const{
	return d->automatic_running_state_tm;
}
void FortrendPumpSubsystem::setAutomaticRunningStateLLA(const bool value){
	if (d->automatic_running_state_lla != value)
	{
		d->automatic_running_state_lla = value;
	}

}
bool FortrendPumpSubsystem::getAutomaticRunningStateLLA() const{
	return d->automatic_running_state_lla;
}
void FortrendPumpSubsystem::setAutomaticRunningStateLLB(const bool value){
	if (d->automatic_running_state_llb != value)
	{
		d->automatic_running_state_llb = value;
	}

}
bool FortrendPumpSubsystem::getAutomaticRunningStateLLB() const{
	return d->automatic_running_state_llb;
}


bool FortrendPumpSubsystem::getMechanicalPumpOpened()const{
	return d->mechanical_pump_opened;
}

void FortrendPumpSubsystem::setMechanicalPumpOpened(const bool value){
	if (value != d->mechanical_pump_opened)
	{
		d->mechanical_pump_opened = value;
		AbstractIOSubsystem::emitAttributeChanged(this);
	}
	
}

/*
*获取机械泵报警
*/
bool FortrendPumpSubsystem::getMechanicalPumpHasAlarm(){
	bool result = false;
	if (d->mechanical_pump_alarm_address != "")
	{
		KeyencePlcSubSystemHelper::readBit(d->mechanical_pump_alarm_address, result);
	}
	return result;
}

bool FortrendPumpSubsystem::getMolecularPumpOpenedLLA()const{
	
	return d->molecular_pump_opened_lla;
}

void FortrendPumpSubsystem::setMolecularPumpOpenedLLA(const bool value){
	if (value != d->molecular_pump_opened_lla)
	{
		d->molecular_pump_opened_lla = value;
		AbstractIOSubsystem::emitAttributeChanged(this);
	}
}
bool FortrendPumpSubsystem::getMolecularPumpOpenedLLB()const{

	return d->molecular_pump_opened_llb;
}

void FortrendPumpSubsystem::setMolecularPumpOpenedLLB(const bool value){
	if (value != d->molecular_pump_opened_llb)
	{
		d->molecular_pump_opened_llb = value;
		AbstractIOSubsystem::emitAttributeChanged(this);
	}
}
bool FortrendPumpSubsystem::getMolecularPumpOpenedTM()const{

	return d->molecular_pump_opened_tm;
}

void FortrendPumpSubsystem::setMolecularPumpOpenedTM(const bool value){
	if (value != d->molecular_pump_opened_tm)
	{
		d->molecular_pump_opened_tm = value;
		AbstractIOSubsystem::emitAttributeChanged(this);
	}
}

bool FortrendPumpSubsystem::getMolecularPumpReachSpeedLLA()const{

	return d->molecular_pump_reach_speed_lla;
}

void FortrendPumpSubsystem::setMolecularPumpReachSpeedLLA(const bool value){
	if (value != d->molecular_pump_reach_speed_lla)
	{
		d->molecular_pump_reach_speed_lla = value;
		AbstractIOSubsystem::emitAttributeChanged(this);
	}
}

bool FortrendPumpSubsystem::getMolecularPumpReachSpeedLLB()const{

	return d->molecular_pump_reach_speed_llb;
}

void FortrendPumpSubsystem::setMolecularPumpReachSpeedLLB(const bool value){
	if (value != d->molecular_pump_reach_speed_llb)
	{
		d->molecular_pump_reach_speed_llb = value;
		AbstractIOSubsystem::emitAttributeChanged(this);
	}
}

bool FortrendPumpSubsystem::getMolecularPumpReachSpeedTM()const{

	return d->molecular_pump_reach_speed_tm;
}

void FortrendPumpSubsystem::setMolecularPumpReachSpeedTM(const bool value){
	if (value != d->molecular_pump_reach_speed_tm)
	{
		d->molecular_pump_reach_speed_tm = value;
		AbstractIOSubsystem::emitAttributeChanged(this);
	}
}


int FortrendPumpSubsystem::getMolecularPumpRev()const{
	return d->molecular_pump_rev;
}

void FortrendPumpSubsystem::setMolecularPumpRev(const int value){
	if (value != d->molecular_pump_rev)
	{
		d->molecular_pump_rev = value;
		AbstractIOSubsystem::emitAttributeChanged(this);
	}
}

void FortrendPumpSubsystem::setVacuumEnable(const bool value){
	if (d->vacuum_enable != value)
	{
		d->vacuum_enable = value;
	}
}

void FortrendPumpSubsystem::setWithWaferModeEnable(const bool value){
	if (d->with_wafer_mode != value)
	{
		d->with_wafer_mode = value;
	}
}

std::shared_ptr<KernelSubsystemResetCommand> FortrendPumpSubsystem::createResetCommand()const{
	FortrendPumpSubsystem* self = const_cast<FortrendPumpSubsystem*>(this);
	PumpResetCommand::Ptr ret(new PumpResetCommand(self));
	return ret;
}

std::shared_ptr<KernelSubsystemUpdateCommand> FortrendPumpSubsystem::createUpdateCommand()const{
	FortrendPumpSubsystem* self = const_cast<FortrendPumpSubsystem*>(this);
	PumpUpdateCommand::Ptr ret(new PumpUpdateCommand(self));
	return ret;
}

std::shared_ptr<AbstractOutPutCommand>  FortrendPumpSubsystem::createOutputCommand(int channel, bool stat)const{
	FortrendPumpSubsystem* self = const_cast<FortrendPumpSubsystem*>(this);
	AbstractOutPutCommand::Ptr ret(new PumpOutputCommand(channel, stat,self));
	return ret;
}

std::shared_ptr<PumpMechanicalOpenCommand>  FortrendPumpSubsystem::createMechanicalOpenCommand()const{
	FortrendPumpSubsystem* self = const_cast<FortrendPumpSubsystem*>(this);
	PumpMechanicalOpenCommand::Ptr ret(new PumpMechanicalOpenCommand(self));
	return ret;
}

std::shared_ptr<PumpMechanicalCloseCommand>  FortrendPumpSubsystem::createMechanicalCloseCommand()const{
	FortrendPumpSubsystem* self = const_cast<FortrendPumpSubsystem*>(this);
	PumpMechanicalCloseCommand::Ptr ret(new PumpMechanicalCloseCommand(self));
	return ret;
}

std::shared_ptr<PumpMolecularOpenCommand>  FortrendPumpSubsystem::createMolecularOpenCommand(std::string name)const{
	FortrendPumpSubsystem* self = const_cast<FortrendPumpSubsystem*>(this);
	PumpMolecularOpenCommand::Ptr ret(new PumpMolecularOpenCommand(self, name));
	return ret;
}

std::shared_ptr<PumpMolecularCloseCommand>  FortrendPumpSubsystem::createMolecularCloseCommand(std::string name)const{
	FortrendPumpSubsystem* self = const_cast<FortrendPumpSubsystem*>(this);
	PumpMolecularCloseCommand::Ptr ret(new PumpMolecularCloseCommand(self,name));
	return ret;
}

std::shared_ptr<PumpOpenLoadLock1AutoVacuumCommand>  FortrendPumpSubsystem::createOpenLoadLock1AutoVacuumCommand()const{
	FortrendPumpSubsystem* self = const_cast<FortrendPumpSubsystem*>(this);
	PumpOpenLoadLock1AutoVacuumCommand::Ptr ret(new PumpOpenLoadLock1AutoVacuumCommand(self));
	return ret;
}

std::shared_ptr<PumpOpenLoadLock2AutoVacuumCommand>  FortrendPumpSubsystem::createOpenLoadLock2AutoVacuumCommand()const{
	FortrendPumpSubsystem* self = const_cast<FortrendPumpSubsystem*>(this);
	PumpOpenLoadLock2AutoVacuumCommand::Ptr ret(new PumpOpenLoadLock2AutoVacuumCommand(self));
	return ret;
}

std::shared_ptr<PumpOpenTMCavityAutoVacuumCommand>  FortrendPumpSubsystem::createOpenTMCavityAutoVacuumCommand()const{
	FortrendPumpSubsystem* self = const_cast<FortrendPumpSubsystem*>(this);
	PumpOpenTMCavityAutoVacuumCommand::Ptr ret(new PumpOpenTMCavityAutoVacuumCommand(self));
	return ret;
}

std::shared_ptr<PumpClearErrorCommand>  FortrendPumpSubsystem::createClearErrorCommand()const{
	FortrendPumpSubsystem* self = const_cast<FortrendPumpSubsystem*>(this);
	PumpClearErrorCommand::Ptr ret(new PumpClearErrorCommand(self));
	return ret;
}





}

