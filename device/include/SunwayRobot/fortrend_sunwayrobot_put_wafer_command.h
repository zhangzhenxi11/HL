/**
* @file     fortrend_sunwayrobot_put_wafer_command.h
* @brief    put wafer command for sunway robot
* @author   kai
*/

// Library: Fortrend
// Package: SubSystem/SunwayRobot

#ifndef _XLH_FORTREND_SUNWAYROBOT_PUT_WAFER_INCLUDE_
#define _XLH_FORTREND_SUNWAYROBOT_PUT_WAFER_INCLUDE_ 

#include "kernel/kernel_subsystem_command.h"
#include "Kernel/Fortrend/robot_putwafer_abstract_command.h"

#include "SunwayRobot/sunway_subsystem_helper.h"
#include "SunwayRobot/sunway_command_executer.h"

namespace FC{

/**
* @brief put_wafer command for SunwayRobot
*/
	class  SunwayRobotPutWaferCommand :public std::enable_shared_from_this<SunwayRobotPutWaferCommand>, public  RobotAbstractPutWaferCommand, public SunwayCommandExecuter
{
public:
	DECLARE_PTR(SunwayRobotPutWaferCommand)
	SunwayRobotPutWaferCommand(const std::shared_ptr<FortrendStation>&  station, int arm, int slot,SunwaySubSystemHelper* helper);
	virtual std::string getName()const override {return "PutWafer";}
	std::vector<IKernelResources* > resources() const override;

	// 通用操作函数
	RunResult performRobotOperation(const std::function<std::string()>& commandBuilder,const std::function<bool()>& onSuccess);

	// 更新晶圆状态
	bool updateWaferMapping();

	//通用操作函数
	RunResult robotRobotOperation(const std::function<std::string()>& commandBuilder);

	bool updateAwcData();


protected:
	virtual RunResult onRun() throw(KernelException);

private:
	DECLARE_PRIVATE(SunwayRobotPutWaferCommand)
};

}
#endif

