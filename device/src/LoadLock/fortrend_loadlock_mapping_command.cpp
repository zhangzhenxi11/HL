/**
* @file            fortrend_loadlock_mapping_command.h
* @brief           mapping command for loadlock
* @author		   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/LoadLock

#include "Poco/Format.h"

#include "Kernel/kernel.h"
#include "Kernel/kernel_command.h"
#include "kernel/kernel_command_reject_exception.h"
#include "Kernel/kernel_log.h"
#include "kernel/Fortrend/fortrend_cassette_manager.h"

#include "LoadLock/fortrend_loadlock_defined.h"
#include "LoadLock/fortrend_loadlock_mapping_command.h"
#include "LoadLock/fortrend_loadlock_subsystem.h"

#include <Windows.h>

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

namespace FC{


	/**
	* LoadLockMappingCommandPrivate
	*/
	class LoadLockMappingCommandPrivate{
	public:
		
	};

	/**
	* LoadLockMappingCommand
	*/
	LoadLockMappingCommand::LoadLockMappingCommand(KeyencePlcSubSystemHelper* hexHelper)
		:KeyencePlcCommandExecuter(hexHelper)
		, d(new LoadLockMappingCommandPrivate){
		//d->empty_cassette = empty_cassette;
		//setMessageName("Mapping");
		//setDescription("mapping the laodlock");
	};


	/**
	* return true if success else false.
	*/
	LoadLockMappingCommand::RunResult LoadLockMappingCommand::onRun() throw(KernelException){
		FortrendLoadLockSubsystem* sub = dynamic_cast<FortrendLoadLockSubsystem*>(getSubsystem());
		//
		if (!sub){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "子系统类型错误", this);
		}
		//if (!sub->hasBoxPresent())
		//{
		//	throw KernelCommandRejectException(__FILE__, KernelSysException::KR_STATION_WITHOUT_CASS_EXCEPTION, Poco::format("工位： %s当前没有晶圆盒", sub->getName()), this);
		//}
		//if (sub->getCassetteDoorOpend())
		//{
		//	throw KernelCommandRejectException(__FILE__, KernelSysException::KR_STATION_WITHOUT_CASS_EXCEPTION, Poco::format("工位： %s放晶圆盒的门已打开", sub->getName()), this);
		//}
		//if (sub->getTMCavityDoorOpend())
		//{
		//	throw KernelCommandRejectException(__FILE__, KernelSysException::KR_STATION_WITHOUT_CASS_EXCEPTION, Poco::format("工位： %s传输腔门阀已打开", sub->getName()), this);
		//}
		/*if (!sub->getProtrudingSensorState())
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_STATION_CONFLICT_EXCEPTION, Poco::format("工位： %s 检测到凸片", sub->getName()), this);
		}*/

		//check modules
		auto cassManager = sub->getKernel()->getKernelModule<FortrendCassetteManager>();
		auto subsystem_cass = cassManager->getCassette(sub);

		//get command configure
		std::shared_ptr<KernelConfiguration> command_config = sub->getConfigure()->createView(getName());
		//fill params
		std::string  first_layer_wafer_presence = command_config->getString("first_layer_detection_sensor","");
		std::string  second_layer_wafer_presence = command_config->getString("second_layer_detection_sensor","");
		//std::string  bulge_wafer_presence = command_config->getString("bulge_detection_sensor","");

		int timeout = command_config->getInt("timeout", -1);
		if (timeout < 1){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_DATA_OUTOF_RANGE, Poco::format("超时: %s 扫描指令设置超时时间错误", sub->getName()), this);
		}

		if ((first_layer_wafer_presence == "") || (second_layer_wafer_presence == ""))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, Poco::format("地址: 扫描地址未定义", getName()), this);
		}

		Cassette::Mapping mappingData1 = Cassette::Mapping::Unknown;

		
		//sub->setFirstLayerMapping(true);
		//sub->getFirstLayerMapping(mappingData1);
		//subsystem_cass->setMapping(1, mappingData1);
		//return IKernelCommand::RunResult::RUN_OK;

		sub->setBoxPlacement(false);
		logInform(sub->getName().c_str(), "检测槽是否有wafer命令开始.");

		bool nfirst_res = false;
		bool nSecond_res = false;
		short first_wafer_res = 0;
		short second_wafer_res = 0;

		if (!readBit(first_layer_wafer_presence, nfirst_res))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 读LoadLock1第一层检测感应器地址错误", sub->getName()), this);
		}
		if (!readBit(second_layer_wafer_presence, nSecond_res))
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 读LoadLock1第二层检测感应器地址错误", sub->getName()), this);
		}
		
		IKernelCommand::RunResult ret = IKernelCommand::RunResult::RUN_FAILD;
		int slot_count = subsystem_cass->slotCount();
		
		if (slot_count != 2)
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 获取casstte的槽数不是两层", sub->getName()), this);
		}
		std::unique_ptr<short[]> mapping_res(new short[slot_count]);
		
		first_wafer_res = (nfirst_res) ? 2: 1;
		second_wafer_res = (nSecond_res) ? 2: 1;
		
		for (int i = 0; i < slot_count; i++)
		{
			if(i ==0)
				mapping_res[i] = first_wafer_res;
			else
				mapping_res[i] = second_wafer_res;
		}

		bool all_slot_mapping_normal = true;
		bool all_slots_empty = true;
		std::vector<Cassette::Mapping> mappingData;
		std::string mappingDouble = "";
		std::string mappingCrossed = "";
		std::vector<int> asubSlots;
		std::string str_mapping_res = "";

		for (int i = 0; i < slot_count; i++){
			Cassette::Mapping slotState = Cassette::Mapping::Unknown;
			switch (mapping_res[i])
			{
			case 1:
				slotState = Cassette::Mapping::Empty;
				break;
			case 2:
				slotState = Cassette::Mapping::Present;
				all_slots_empty = false;
				break;
			case 3:
				slotState = Cassette::Mapping::Double;
				if (mappingDouble==""){
					mappingDouble += "叠片槽号：";
				}
				mappingDouble += std::to_string(i+1);
				mappingDouble += " ";
				break;
			case 4:
				slotState = Cassette::Mapping::Crossed;
				if (mappingCrossed==""){
					mappingCrossed += "斜片槽号：";
				}
				mappingCrossed += std::to_string(i + 1);
				mappingCrossed += " ";
				break;
			default:
				break;
			}
			mappingData.push_back(slotState);
			asubSlots.push_back(i + 1);
			str_mapping_res.append(std::to_string(mapping_res[i]));
		}
		subsystem_cass->setMapping(asubSlots, mappingData);

		logInform(sub->getName().c_str(), "扫描结束, 结果: %s", str_mapping_res.c_str());
		
		if (mappingDouble != "" || mappingCrossed != "")//!all_slot_mapping_normal
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_CASS_STATE_EXCEPTION, 
				Poco::format("工位: %s 扫描出晶圆异常%s %s", sub->getName(), mappingDouble, mappingCrossed), this);
		}
		ret = IKernelCommand::RunResult::RUN_OK;
		return ret;

		//AlarmMessage::Ptr alarm(new AlarmMessage(KernelSysException::TYPE, KernelSysException::KR_MODULE_COMMUNICATION_TIMEOUT, "扫描命令通讯超时"));
		//setAlarm(alarm);
		//logError(sub->getName().c_str(), "扫描命令通讯超时.");
	}

}