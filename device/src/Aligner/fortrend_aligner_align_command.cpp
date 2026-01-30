/**
* @file            fortrend_loadlock_align_command.h
* @brief           align command for Aligner
* @author		   xielonghua
*/

// Library: Fortrend
// Package: SubSystem/Aligner

#include "Poco/Format.h"

#include "Kernel/kernel.h"
#include "Kernel/kernel_command.h"
#include "kernel/kernel_command_reject_exception.h"
#include "Kernel/kernel_log.h"
#include "kernel/Fortrend/fortrend_cassette_manager.h"

#include "Aligner/fortrend_aligner_align_command.h"
#include "Aligner/fortrend_aligner_subsystem.h"

#include "Kernel/kernel_event_paramters.h"
#include "fortrend_device_kernel.h"

#include <windows.h>
#include <sstream>
#include <vector>
#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif


namespace FC{



	/**
	* AlignerAlignCommand
	*/
	AlignerAlignCommand::AlignerAlignCommand(AlignerSubSystemHelper* hexHelper)
		:AlignerCommandExecuter(hexHelper){
		//setMessageName("Align");
		//setDescription("align the aligner");
	};


	/**
	* return true if success else false.
	*/
	AlignerAlignCommand::RunResult AlignerAlignCommand::onRun() throw(KernelException){
		FortrendAlignerSubsystem* sub = dynamic_cast<FortrendAlignerSubsystem*>(getSubsystem());

		//
		if (!sub){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, "子系统类型错误", this);
		}
		//get command configure
		std::shared_ptr<KernelConfiguration> command_config = sub->getConfigure()->createView(getName());

		//fill params
		RunResult ret = RunResult::RUN_FAILD;
		int timeout = command_config->getInt("timeout", 120000);
		if (timeout < 10){
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, Poco::format("超时: 寻边命令超时时间设置错误.", getName()), this);
		}

		if (sendRequest("ALGN RSLT 0 "))
		{
			KernelCommandParameter parameter(shared_from_this());
			sub->sendEvent(NEW_EVENT_ID_WITHNAME(EVENT_COMMAND_RUNNING), &parameter);
			std::string result = recvResponse(timeout);
			if (result != "error")
			{
				std::vector<std::string> arr_data;
				std::stringstream stm_awc(result);
				std::string token;

				while (std::getline(stm_awc, token, ' ')) {
					arr_data.push_back(token);
				}
				if (arr_data.size() == 4)
				{
					int result_length = std::stoi(arr_data[2]);
					int result_angle = std::stoi(arr_data[3]);
					sub->setAlignResult(true, result_length, result_angle);
					ret = RunResult::RUN_OK;
				}
				else
				{
					throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 数据解析错误，返回结果：%s", sub->getName(), result), this);
				}
			}
			else{
				throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 接收结果错误.", sub->getName()), this);
			}

		}
		else{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_MODULE_RESPONSE_ERROR, Poco::format(" %s 发送寻边命令失败！", sub->getName()), this);

		}

		return ret;

	}



}