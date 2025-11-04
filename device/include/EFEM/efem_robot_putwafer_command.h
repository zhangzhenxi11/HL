/**
 * @file            hex_robot_putwafer_command.h
 * @brief           put wafer command for fortrend wafer robot
 * @author			xielonghua
 */

// Library: Fortrend
// Package: CommandImp/Hex/Robot


#ifndef _XLH_EFEM_PUTWAFER_COMMAND_INCLUDE_
#define _XLH_EFEM_PUTWAFER_COMMAND_INCLUDE_
#include "Kernel/Fortrend/robot_putwafer_abstract_command.h"
#include "Kernel/Fortrend/hex_command_executer.h"

KERNEL_NS_BEGIN
class HexSubSystemHelper;
class FortrendStation;
void handleRobotPutStationName(std::string& stationName_, int& slotnum_);
/**
 *@brief  ready to getObject command for fortrend robot
*/
class EFEMRobotReadyPutWaferCommand : public  RobotAbstractReadyPutWaferCommand, public HexCommandExecuter
{
public:
	DECLARE_PTR(EFEMRobotReadyPutWaferCommand)
	EFEMRobotReadyPutWaferCommand(const std::shared_ptr<FortrendStation>& station, int arm, int slot, HexSubSystemHelper* hexHelper);

	virtual std::string getName() const override;
	virtual std::string getDescription() const override;
protected:
	virtual RunResult onRun() throw(KernelException);

private:
	DECLARE_PRIVATE(EFEMRobotReadyPutWaferCommand)
};




/**
*@brief  PutObject command for fortrend robot
*/
class EFEMRobotPutWaferCommand :public std::enable_shared_from_this<EFEMRobotPutWaferCommand>, public  RobotAbstractPutWaferCommand, public HexCommandExecuter
{
public:
	DECLARE_PTR(EFEMRobotPutWaferCommand)
	EFEMRobotPutWaferCommand(const std::shared_ptr<FortrendStation>& station, int arm,int slot, HexSubSystemHelper* hexHelper);

	virtual std::string getName() const override;
	virtual std::string getDescription() const override;
	std::vector<IKernelResources* > resources() const override;
protected:
	virtual RunResult onRun() throw(KernelException);

private:
	DECLARE_PRIVATE(EFEMRobotReadyPutWaferCommand)
};




KERNEL_NS_END

#endif
