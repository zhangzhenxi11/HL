/**
 * @file            efem_robot_getwafer_command.h
 * @brief           get wafer command for fortrend wafer robot
 * @author			xielonghua
 */

// Library: Fortrend
// Package: CommandImp/Hex/Robot

#ifndef _XLH_EFEM_GETWAFER_COMMAND_INCLUDE_
#define _XLH_EFEM_GETWAFER_COMMAND_INCLUDE_
#include "Kernel/Fortrend/robot_getwafer_abstract_command.h"
#include "Kernel/Fortrend/hex_command_executer.h"

KERNEL_NS_BEGIN
class HexSubSystemHelper;
class FortrendStation;

void handleStationName(std::string& stationName_, int& slotnum_);

/**
 *@brief  ready to getObject command for fortrend robot
*/
class EFEMRobotReadyGetWaferCommand : public  RobotAbstractReadyGetWaferCommand, public HexCommandExecuter
{
public:
	DECLARE_PTR(EFEMRobotReadyGetWaferCommand)
	EFEMRobotReadyGetWaferCommand(const std::shared_ptr<FortrendStation>& station, int arm, int slot, HexSubSystemHelper* hexHelper);

	virtual std::string getName() const override;
	virtual std::string getDescription() const override;
protected:
	virtual RunResult onRun() throw(KernelException);

private:
	DECLARE_PRIVATE(EFEMRobotReadyGetWaferCommand)
};




/**
*@brief  GetObject command for fortrend robot
*/
class EFEMRobotGetWaferCommand : public  RobotAbstractGetWaferCommand, public HexCommandExecuter
{
public:
	DECLARE_PTR(EFEMRobotGetWaferCommand)
	EFEMRobotGetWaferCommand(const std::shared_ptr<FortrendStation>& station, int arm,int slot, HexSubSystemHelper* hexHelper);

	virtual std::string getName() const override;
	virtual std::string getDescription() const override;
	std::vector<IKernelResources* > resources() const override;

	

protected:
	virtual RunResult onRun() throw(KernelException);

private:
	DECLARE_PRIVATE(EFEMRobotReadyGetWaferCommand)
};




KERNEL_NS_END

#endif
