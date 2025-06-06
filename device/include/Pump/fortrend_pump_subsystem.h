/**
* @file            fortrend_pump_subsystem.h
* @brief           Fortrend Pump 
* @author		   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/Pump


#ifndef _XLH_FORTREND_PUMP_SUBSYSTEM_INCLUDE_
#define _XLH_FORTREND_PUMP_SUBSYSTEM_INCLUDE_ 

#include "kernel/Fortrend/abstract_io_subsystem.h"
#include "kernel/Fortrend/fortrend_abstract_station.h"

#include "KeyencePLC/keyence_plc_command_executer.h"
#include "KeyencePLC/keyence_plc_subsystem_helper.h"

#include "Pump/fortrend_pump_mechanical_close_command.h"
#include "Pump/fortrend_pump_mechanical_open_command.h"
#include "Pump/fortrend_pump_molecular_close_command.h"
#include "Pump/fortrend_pump_molecular_open_command.h"
#include "Pump/fortrend_pump_open_loadlock1_auto_vacuum_command.h"
#include "Pump/fortrend_pump_open_loadlock2_auto_vacuum_command.h"
#include "Pump/fortrend_pump_open_tm_cavity_auto_vacuum_command.h"
#include "Pump/fortrend_pump_clear_error_command.h"

namespace FC{


/**
* @brief fortrend pump
*/
	class  FortrendPumpSubsystem :public AbstractIOSubsystem, public FortrendAbstractStation, public KeyencePlcSubSystemHelper{
public:
	DECLARE_PTR(FortrendPumpSubsystem)
	FortrendPumpSubsystem(IKernel*  kernel, const std::string& name);

	//override for FortrendAbstractStation
	virtual bool hasBoxPlacement()const override { return true; };
	virtual bool hasBoxPresent()const override { return true; };
	virtual bool hasDoorOpend()const override { return true; };
	virtual bool clearAlarm() override;
	//override for AbstractIOSubsystem
	virtual int inputCount()const { return 0; };
	virtual int outputCount()const{ return 0; };
	virtual bool getInput(int index)const { return false; };
	virtual std::string getInputName(int index)const { return ""; };
	virtual std::string getOutputName(int index)const { return ""; };
	
	
	virtual std::shared_ptr<KernelSubsystemResetCommand> createResetCommand()const override;
	virtual std::shared_ptr<KernelSubsystemUpdateCommand> createUpdateCommand() const override;
	virtual std::shared_ptr<AbstractOutPutCommand>  createOutputCommand(int channel, bool stat)const override;


public:
	
	std::shared_ptr<PumpMechanicalCloseCommand> createMechanicalCloseCommand()const;
	std::shared_ptr<PumpMechanicalOpenCommand> createMechanicalOpenCommand()const;
	std::shared_ptr<PumpMolecularCloseCommand> createMolecularCloseCommand(std::string name)const;
	std::shared_ptr<PumpMolecularOpenCommand> createMolecularOpenCommand(std::string name)const;
	std::shared_ptr<PumpOpenLoadLock1AutoVacuumCommand> createOpenLoadLock1AutoVacuumCommand() const;//LL1打开真空（抽真空）
	std::shared_ptr<PumpOpenLoadLock2AutoVacuumCommand> createOpenLoadLock2AutoVacuumCommand() const;//LL2打开真空（抽真空）
	std::shared_ptr<PumpOpenTMCavityAutoVacuumCommand> createOpenTMCavityAutoVacuumCommand() const;//TM打开真空（抽真空）
	std::shared_ptr<PumpClearErrorCommand> createClearErrorCommand()const;

public:
	
	/*
	*获取分子泵运行状态(0:待机,1:运行,其他:错误)
	*/
	int getMolecularPumpRunningStateTM()const;
	void setMolecularPumpRunningStateTM(const int value);
	int getMolecularPumpRunningStateLLA()const;
	void setMolecularPumpRunningStateLLA(const int value);
	int getMolecularPumpRunningStateLLB()const;
	void setMolecularPumpRunningStateLLB(const int value);

	void setAutomaticRunningStateTM(const bool value);
	bool getAutomaticRunningStateTM() const;
	void setAutomaticRunningStateLLA(const bool value);
	bool getAutomaticRunningStateLLA() const;
	void setAutomaticRunningStateLLB(const bool value);
	bool getAutomaticRunningStateLLB() const;

	bool getMechanicalPumpOpened()const;
	void setMechanicalPumpOpened(const bool value);
	bool getMechanicalPumpHasAlarm();

	bool getMolecularPumpOpenedLLA()const;
	void setMolecularPumpOpenedLLA(const bool value);
	bool getMolecularPumpOpenedLLB()const;
	void setMolecularPumpOpenedLLB(const bool value);
	bool getMolecularPumpOpenedTM()const;
	void setMolecularPumpOpenedTM(const bool value);

	bool getMolecularPumpReachSpeedLLA() const;
	void setMolecularPumpReachSpeedLLA(const bool value);
	bool getMolecularPumpReachSpeedLLB() const;
	void setMolecularPumpReachSpeedLLB(const bool value);
	bool getMolecularPumpReachSpeedTM() const;
	void setMolecularPumpReachSpeedTM(const bool value);

	int getMolecularPumpRev()const;
	void setMolecularPumpRev(const int value);

	void setVacuumEnable(const bool value);
	void setWithWaferModeEnable(const bool value);
	/*
	*更新分子泵状态
	*/
	void updateMolecularPumpState();

protected:
	virtual void onInitialize()throw(KernelException)override;
	virtual void onUnInitialize()override;
	virtual void onProcess()override;
	virtual void onConfigure(const std::shared_ptr<KernelConfiguration> & config)override;
private:
	DECLARE_PRIVATE(FortrendPumpSubsystem)

};

}

#endif
