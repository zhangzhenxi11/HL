/**
 * @file            efem_wafer_robot_subsystem.h
 * @brief           fortrend wafer Robot
 * @author			xielonghua
 */

// Library: Fortrend
// Package: SubRobotImp



#ifndef _XLH_EFEM_WAFER_ROBOT_SUBSYSTEM_INCLUDE_
#define _XLH_EFEM_WAFER_ROBOT_SUBSYSTEM_INCLUDE_ 
#include "Kernel/Fortrend/robot_wafer_abstract_subsystem.h"
#include "Kernel/Fortrend/hex_subsystem_helper.h" 

#include "efem_ascii_api.h"
#include <mutex>
#include <condition_variable>
KERNEL_NS_BEGIN

/**
* @brief fortrend wafer robot 
*/
class EFEMWaferRobotSubsystem : public WaferRobotAbstractSubsystem, public HexSubSystemHelper
{
public:
	DECLARE_PTR(EFEMWaferRobotSubsystem)
	EFEMWaferRobotSubsystem(IKernel*  kernel, const std::string& name, std::shared_ptr<EFEMAsciiApi> api);
	std::shared_ptr<EFEMAsciiApi> api;
	/**
	*@brief  has object on Fortrend Robot Arm(arm_id)
	*/
	virtual bool hasObject(unsigned int arm_id)const override;

	/**
	*@brief  virtual input count for this robot
	*/
	virtual int inputCount()const  override;

	/**
	*@brief  virtual output count for this robot
	*/
	virtual int outputCount()const  override;

	/**
	* virtual input state
	*/
	virtual bool getInput(int index)const override;

	/**
	*@brief  virtual input alibs name
	*/
	virtual std::string getInputName(int index)const;

	/**
	*@brief  virtual input alibs name
	*/
	virtual std::string getOutputName(int index)const;


	/**
	*@brief  has box placement
	*/
	virtual bool hasBoxPlacement()const override { return true; }

	/**
	*@brief  has box present
	*/
	virtual bool hasBoxPresent()const override { return true; }

public:
	/**
	*@brief  set arm has object manualy
	*/
	void setObject(unsigned int arm_id, bool has);

public:
	virtual std::shared_ptr<KernelSubsystemResetCommand> createResetCommand()const override;
	virtual std::shared_ptr<KernelSubsystemUpdateCommand> createUpdateCommand() const override;
	virtual std::shared_ptr<RobotAbstractReadyGetWaferCommand> createReadyGetCommand(const std::shared_ptr<FortrendStation>&  station, int arm, int slot)const override;
	virtual std::shared_ptr<RobotAbstractReadyPutWaferCommand> createReadyPutCommand(const std::shared_ptr<FortrendStation>&  station, int arm, int slot)const override;
	virtual std::shared_ptr<RobotAbstractSetSpeedCommand> createSetSpeedCommand(uint8_t percentage) const override;

	virtual std::shared_ptr<RobotAbstractGetWaferCommand> createGetCommand(const std::shared_ptr<FortrendStation>&  station, int arm, int slot)const override;
	virtual std::shared_ptr<RobotAbstractPutWaferCommand> createPutCommand(const std::shared_ptr<FortrendStation>&  station, int arm, int slot) const override;
	virtual std::shared_ptr<AbstractOutPutCommand>  createOutputCommand(int channel, bool stat)const override;
protected:
	virtual void onInitialize()throw(KernelException)override;
	virtual void onUnInitialize()override;
	virtual void onProcess()override;
	virtual void onConfigure(const std::shared_ptr<KernelConfiguration> & config)override;
	virtual void onMessage(const HexMessage::Ptr& message, bool inputChanged)override;

public:
	void handle(const std::shared_ptr<EFEMAsciiApi::Command>& command);
	void setCommandState(EFEMAsciiApi::State newState);
	EFEMAsciiApi::State getCommandState();
	bool hasFinishedCommandState()const;
	void wait();
	uint32_t timeout = 3000;   //default 3s
	std::chrono::system_clock::time_point timestamp;
	std::string primaryMessageName;

private:
	EFEMAsciiApi::State state;
	std::mutex mtx;
	std::condition_variable cv;
	DECLARE_PRIVATE(EFEMWaferRobotSubsystem)

};



KERNEL_NS_END

#endif
