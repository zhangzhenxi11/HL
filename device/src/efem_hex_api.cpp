/**
* @file            efem_hex_api.h
* @brief           efem hex for fortrend for gui
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Api/HEX

#include <iostream>

#include "efem_hex_api.h"
#include "base_hex_api.h"
#include "kernel/kernel.h"
#include "kernel/kernel_subsystem.h"
#include "kernel/kernel_log.h"
#include "kernel/kernel_command.h"
#include "kernel/kernel_action.h"
#include "Kernel/Fortrend/hex_update_command.h"
#include "kernel/kernel_exception.h"
#include "Kernel/Fortrend/fortrend_foup_robot_subsystem.h"
#include "Kernel/Fortrend/fortrend_loadport_subsystem.h"
#include "Kernel/Fortrend/fortrend_fims_subsystem.h"



#include "Poco/Format.h"
#include "Poco/NumberParser.h"
#include <cmath>
#include <map>

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif
#define MAX_BUFFER_SIZE 100

#define STATION_ID_KEY   "StationId"

namespace FC{

	class EFEMHexApiPrivate{
	public:
		EFEMHexApiPrivate(EFEMHexApi*parent);
		char buffer[MAX_BUFFER_SIZE];
		BaseHexApi::HandlerMap handle_map;
		EFEMHexApi* pParent;

		//handlers
		std::vector<BaseHexApi::Ptr> sub_handler;
		std::vector<BaseHexApi::Ptr> defined_handler;
		std::vector<BaseHexApi::Ptr> system_handler;

	};

	EFEMHexApiPrivate::EFEMHexApiPrivate(EFEMHexApi*parent)
		:pParent(parent){
		//init members
		handle_map = BaseHexApi::HandlerMap{
			{ 0x00, std::bind(&EFEMHexApi::handle_subsystem, pParent, std::placeholders::_1, std::placeholders::_2) },
			{ 0x01, std::bind(&EFEMHexApi::handle_defined, pParent, std::placeholders::_1, std::placeholders::_2) },
			{ 0x02, std::bind(&EFEMHexApi::handle_system, pParent, std::placeholders::_1, std::placeholders::_2) }
		};
	}

	EFEMHexApi::EFEMHexApi(IKernel*  kernel)
		:TcpServerApi(kernel)
		, d(new EFEMHexApiPrivate(this)){

	}


	IKernel* EFEMHexApi::getKernel()const{
		return kernel;
	}


	bool  EFEMHexApi::sendMessage(const char* data, unsigned int len){
		char *buf = new char[len + 1];
		memcpy(buf + 1, data, len);
		buf[0] = len;
		bool ret = TcpServerApi::sendMessage(buf, len + 1);
		delete[] buf;
		return ret;
	}

	void EFEMHexApi::sendACK(uint32_t index){
		uint8_t data[6];
		memcpy(data, (uint8_t*)&index, sizeof(index));
		data[4] = 0x00;
		data[5] = 0x00;

		sendMessage((char*)data, sizeof(data));
	}


	void EFEMHexApi::sendNCK(uint32_t index, uint8_t staionid, uint8_t type, uint16_t errCode){
		uint8_t data[10];
		memcpy(data, (uint8_t*)&index, sizeof(index));
		data[4] = 0x00;
		data[5] = 0x01;

		data[6] = staionid;
		data[7] = type;
		data[8] = errCode >> 8 & 0Xff;
		data[9] = errCode & 0Xff;

		sendMessage((char*)data, sizeof(data));
	}
	
	

	void EFEMHexApi::sendNCKSubsystemErrorCode(uint32_t index, uint8_t errCode, uint8_t subsystem_id)
	{
		uint8_t data[10];
		memcpy(data, (uint8_t*)&index, sizeof(index));
		data[4] = 0x00;
		data[5] = 0x01;

		data[6] = 0x80;
		data[7] = errCode;
		data[8] = subsystem_id;
		data[9] = 0x00;

		sendMessage((char*)data, sizeof(data));
	}

	void EFEMHexApi::sendNCKSystemError(uint32_t index, uint16_t errCode){
		uint8_t data[10];
		memcpy(data, (uint8_t*)&index, sizeof(index));
		data[4] = 0x00;
		data[5] = 0x01;

		data[6] = 0x80;
		data[7] = errCode >> 8 & 0Xff;
		data[8] = errCode & 0Xff;
		data[9] = 0x00;

		sendMessage((char*)data, sizeof(data));
	}
	void EFEMHexApi::sendSubsystemErrorDetected(uint32_t index, uint8_t station_id, uint8_t command_id, uint8_t errCode)
	{
		uint8_t data[9];
		memcpy(data, (uint8_t*)&index, sizeof(index));
		data[4] = 0x01;


		data[5] = station_id;
		data[6] = 0xFF;
		data[7] = command_id;
		data[8] = errCode;

		sendMessage((char*)data, sizeof(data));
	}
	void EFEMHexApi::sendSubsystemErrorCode(uint32_t index, uint8_t errCode, uint8_t subsystem_id){
		uint8_t data[9];
		memcpy(data, (uint8_t*)&index, sizeof(index));
		data[4] = 0x01;
		data[5] = 0x80;
		data[6] = errCode;
		data[7] = subsystem_id;
		data[8] = 0x00;

		sendMessage((char*)data, sizeof(data));
	}
	void EFEMHexApi::sendSubsystemErrorCode_unint16(uint32_t index, uint16_t errCode){
		uint8_t data[9];
		memcpy(data, (uint8_t*)&index, sizeof(index));
		data[4] = 0x01;
		data[5] = 0x80;
		data[6] = errCode >> 8 & 0Xff;
		data[7] = errCode & 0Xff;
		data[8] = 0x00;

		sendMessage((char*)data, sizeof(data));
	}
	void EFEMHexApi::sendSubsystemReponse(uint32_t index, uint8_t staionid, uint8_t response_id, uint8_t* param, uint8_t len){
		uint8_t data[100];
		memcpy(data, (uint8_t*)&index, sizeof(index));
		data[4] = 0x01;
		data[5] = staionid;

		data[6] = response_id;

		if (!param) len = 0;
		for (int i = 0; i < len && param; i++){
			data[7 + i] = param[i];
		}

		sendMessage((char*)data, 7 + len);
	}

	void EFEMHexApi::sendSystemReponse(uint32_t index, uint16_t response_id, uint8_t* param, uint8_t len){
		uint8_t data[100];
		memcpy(data, (uint8_t*)&index, sizeof(index));
		data[4] = 0x03;
		data[5] = response_id >> 8 & 0Xff;
		data[6] = response_id & 0Xff;
		if (!param) len = 0;
		for (int i = 0; i < len && param; i++){
			data[7 + i] = param[i];
		}

		sendMessage((char*)data, 7 + len);
	}

	void EFEMHexApi::sendDefinedReponse(uint32_t index, uint8_t response_id, uint8_t* param, uint8_t len){
		uint8_t data[100];
		memcpy(data, (uint8_t*)&index, sizeof(index));
		data[4] = 0x02;
		data[5] = response_id;
		if (!param) len = 0;
		for (int i = 0; i < len && param; i++){
			data[6 + i] = param[i];
		}
		sendMessage((char*)data, 6 + len);
	}

	void EFEMHexApi::sendEventReponse(uint32_t index, uint16_t response_id, uint8_t* param, uint8_t len){
		uint8_t data[100];
		memcpy(data, (uint8_t*)&index, sizeof(index));
		data[4] = 0x04;
		data[5] = response_id >> 8 & 0Xff;
		data[6] = response_id & 0Xff;
		if (!param) len = 0;
		for (int i = 0; i < len && param; i++){
			data[7 + i] = param[i];
		}
		sendMessage((char*)data, 7 + len);
	}

	void EFEMHexApi::sendEvent(uint16_t event_id, uint8_t* param, uint8_t len){
		uint8_t data[120];
		for (size_t i = 0; i < 4; i++)
		{
			data[i] = 0x00;
		}
		data[4] = 0x04;
		data[5] = event_id >> 8 & 0Xff;
		data[6] = event_id & 0Xff;
		if (!param) len = 0;
		for (int i = 0; i < len && param; i++){
			data[7 + i] = param[i];
		}
		sendMessage((char*)data, 7 + len);
	}

	void EFEMHexApi::sendEventResetCommandCompleted(uint8_t station_id){
		uint8_t param[3] = { 0x00, station_id, 0x22 };
		sendEvent(0x0000, param, 3);
	}

	void EFEMHexApi::sendEventResetCommandError(uint8_t station_id){
		uint8_t param[7] = { 0x00, station_id, 0x22, station_id, 0xA0, 0x00, 0x01};
		sendEvent(0x0001, param, 7);
	}

	void EFEMHexApi::process(){
		//handle data
		if (getBufferSize() <= 0) return;

		peekBuffer(0, d->buffer, 1);
		int size = d->buffer[0];
		//too low
		if (getBufferSize() <= size) return;

		size = min(MAX_BUFFER_SIZE, size + 1);
		readBuffer(d->buffer, size);  //read from buffer
		logMessage(d->buffer, size, "R");

		//data too low
		if (size < 0x06) return;

		int request_type = d->buffer[5];
		int station_id = d->buffer[6];

		auto it = d->handle_map.find(request_type);
		try{
			if (it != d->handle_map.end()){
				it->second(d->buffer + 1, size - 1);  //handle command
			}
		}
		catch (KernelException& e){
			uint32_t index = *(uint32_t*)(d->buffer + 1);
			sendNCK(index, station_id, e.getType(), e.getErr());
		}
		catch (...){
			logError(getName().c_str(), "unknown error when handle api");
		}
	}




	void EFEMHexApi::onConnect(){
		setCommunicationState(KernelApi::COMMUNICATING);
		setRemoteOnLine();
	}

	void EFEMHexApi::onDisConnect(){
		setCommunicationState(KernelApi::NOT_COMMUNICATING);
		setLocalOnLine();
	}

	bool EFEMHexApi::handle_subsystem(const char* data, size_t len){
		for (auto sub : d->sub_handler){
			if (sub->handle(data, len)){
				return true;
			}
		}
		return false;
	}

	bool EFEMHexApi::handle_defined(const char* data, size_t len){
		for (auto sub : d->defined_handler){
			if (sub->handle(data, len)){
				return true;
			}
		}
		return false;
	}

	bool EFEMHexApi::handle_system(const char* data, size_t len){
		for (auto sub : d->system_handler){
			if (sub->handle(data, len)){
				return true;
			}
		}
		return false;
	}



	void  EFEMHexApi::submitTask(
		uint32_t index,
		uint8_t station_id,
		const std::shared_ptr<IKernelSubSystem>& t_sub,
		const::std::shared_ptr<IKernelCommand> & t_sub_command,
		uint8_t cmdid,
		uint8_t isArmHasWaferOrRFIDChannel){
		//check command
		t_sub->checkCommand(t_sub_command);

		int okResponse = Poco::NumberParser::parse(t_sub_command->getProperty(CMD_OK_KEY, "98"));
		int failResponse = Poco::NumberParser::parse(t_sub_command->getProperty(CMD_FAILD_KEY, "99"));

		//submit task
		kernel->submitTask([=](){
			sendACK(index);
			try{
				std::string subName = t_sub->getName();

				t_sub->startCommand(t_sub_command);
				t_sub_command->wait();
				if (!t_sub_command->hasError()) {

					if (okResponse == 97){//判断是否是获取状态命令
						auto hexUpdateCommand = std::dynamic_pointer_cast<HexUpdateCommand>(t_sub_command);
						auto ReplyMessage = hexUpdateCommand->getReplyMessage();
						if (ReplyMessage)
						{
							auto rps = ReplyMessage->data();//模组返回的61后面的数据
							int size = ReplyMessage->size();
							std::unique_ptr<uint8_t[]> ptrResponse(new uint8_t[size]);
							for (size_t i = 0; i < size; i++)
							{
								ptrResponse[i] = static_cast<uint8_t>(rps[i]);
							}
							sendSubsystemReponse(index, station_id, okResponse, ptrResponse.get(), size);
							
							return;
						}
					}
					else if (okResponse == 113)
					{

						//uint8_t param[2] = { isArmHasWaferOrRFIDChannel, 0 };
						uint8_t param[2] = { 0x00, 0x00 };
						if (t_sub->getName() == "RFID1")
						{
							param[0] = 0x01;
						}
						else if (t_sub->getName() == "RFID2")
						{
							param[0] = 0x02;
						}
						else if (t_sub->getName() == "RFID3")
						{
							param[0] = 0x03;
						}
						else
						{
							param[0] = 0x04;
						}
						sendSubsystemReponse(index, station_id, okResponse, param, 2);
						return;
					}
					else
					{
						uint8_t param[3] = { cmdid, isArmHasWaferOrRFIDChannel, 0 };
						sendSubsystemReponse(index, station_id, okResponse, param, 3);
						return;
					}
					/*uint8_t param[3] = { cmdid, isArmHasWafer, 0 };
					sendSubsystemReponse(index, station_id, okResponse, param, 3);
					return;*/

				}
				std::string str_subsystem_name = t_sub->getName();
				auto alarm = t_sub_command->alarmMessage();
				int error_type = alarm->type();
				int error_code = alarm->code();
				//logInform(getName().c_str(), "error_type = %d, error_code= %d", error_type, error_code);
				// Software Detection Error
				if (error_type == 0)
				{
					if (error_code == int(KernelSysException::KR_MODULE_STATE_EXCEPTION))
					{
						if (str_subsystem_name == "Finder")
						{
							sendSubsystemErrorCode(index, 0x80, station_id);
						}
						else
						{
							sendSubsystemErrorCode(index, 0x81, station_id);
						}
						
					}					
					else if (error_code == int(KernelSysException::KR_STATION_WITHOUT_CASS_EXCEPTION))
					{
						if (str_subsystem_name == "WHR")
						{
							sendSubsystemErrorCode(index, 0x90, station_id);
						}
						else
						{
							sendSubsystemErrorCode(index, 0x83, station_id);
						}
					}
					else if (error_code == int(KernelSysException::KR_MODULE_DOOR_EXCEPTION))
					{
						sendSubsystemErrorCode(index, 0x84, station_id);

					}
					
					else if (error_code == int(KernelSysException::KR_SYSTEM_LOGIC_ERROR))
					{
						//FOUP Robot 朝 LP 进行中
						sendSubsystemErrorCode(index, 0x85, station_id);

					}
					else if (error_code == int(KernelSysException::KR_STATION_BOX_LOCKED_EXCEPTION))
					{
						sendSubsystemErrorCode(index, 0x8C, station_id);

					}
					else if (error_code == int(KernelSysException::KR_STATION_WITH_CASS_EXCEPTION))
					{
						if (str_subsystem_name == "WHR")
						{
							sendSubsystemErrorCode(index, 0x91, station_id);
						}
						else
						{
							sendSubsystemErrorCode(index, 0x99, station_id);
						}
					}
					
					else if (error_code == int(KernelSysException::KR_STATION_CONFLICT_EXCEPTION))
					{
						//天车抵达/LP非Auto
						if (str_subsystem_name.find("LP") != std::string::npos)
						{
							sendSubsystemErrorCode_unint16(index, 0x1011);
						}
						else
						{
							//使用错误匹配的Arm
							sendSubsystemErrorCode_unint16(index, 0x1064);
						}
						
					}
					
					else if (error_code == int(KernelSysException::KR_STATION_CAN_NOT_LOAD_UNLOAD))
					{
						sendSubsystemErrorCode_unint16(index, 0x1103);
					}
					else if (error_code == int(KernelSysException::KR_SYSTEM_SAFE_ALARM))
					{
						//LP 光栅触发
						if (str_subsystem_name.find("LP") != std::string::npos)
						{
							sendSubsystemErrorCode(index, 0x87, station_id);
						}
						else if (str_subsystem_name == "HV")
						{
							sendSubsystemErrorCode(index, 0x85, station_id);
						}
						else
						{
							sendSubsystemErrorCode_unint16(index, 0x1103);
						}
						
					}
					else
					{
						sendSubsystemErrorDetected(index, station_id, cmdid, 0xA1);
					}
					

				}
				//subsystem command error 0x63
				else if (error_type == 1 || error_type == 2)
				{
					if (str_subsystem_name == "WHR" || str_subsystem_name == "FTR")
					{
						//Parameter 1:	Command in Progress
						//Parameter 2 : Error Type.
						//Parameter 3 : Error Code(High Byte).
						//Parameter 4 : Error Code(Low Byte).
						uint8_t robot_param[4] = { 0 };
						robot_param[0] = cmdid;
						robot_param[1] = error_type;
						convertErrorCodeIntToUint8_t(error_code, robot_param[2], robot_param[3]);
						sendSubsystemReponse(index, station_id, failResponse, robot_param, 4);
					}
					else if (str_subsystem_name.find("OPENER") != std::string::npos)
					{
						//Parameter 1 : Error Code
						//Parameter 2 : Error Type(1 = Common, 2 = Opener Specific).
						//Parameter 3 : Command in progress(0xff if none).
						//Parameter 4 : Opener Status(0 = Standby, 1 = Operational).
						//Parameter 5 : Error Parameter(only used by Development Tool).
						//Parameter 6 : The last run motor
						uint8_t opener_param[6] = { 0 };
						opener_param[0] = static_cast<uint8_t>(error_code);
						opener_param[1] = error_type;
						opener_param[2] = cmdid;
						opener_param[3] = 0x01;
						opener_param[4] = 0x00;
						opener_param[5] = 0x00;
						sendSubsystemReponse(index, station_id, failResponse, opener_param, 6);
					}
					else if (str_subsystem_name.find("LP") != std::string::npos)
					{
						//Parameter 1:	Command in Progress
						//Parameter 2 : Error Type.
						//Parameter 3 : Error Code.
						uint8_t lp_param[3] = { 0 };
						lp_param[0] = cmdid;
						lp_param[1] = error_type;
						lp_param[2] = static_cast<uint8_t>(error_code);
						sendSubsystemReponse(index, station_id, failResponse, lp_param, 3);
					}
					else if (str_subsystem_name.find("RFID") != std::string::npos)
					{

						//Parameter 1 : Error Code.
						//Parameter 2:	Command in Progress
						uint8_t rfid_param[2] = { 0 };
						rfid_param[0] = error_type;
						rfid_param[1] = cmdid;

						sendSubsystemReponse(index, station_id, failResponse, rfid_param, 2);
					}
					else if (str_subsystem_name == "HV" || str_subsystem_name == "Pusher" || str_subsystem_name == "Finder")
					{
						//Parameter 1:	Command in Progress
						//Parameter 2 : Error Type.
						//Parameter 3 : Error Code.
						uint8_t sub_param[3] = { 0 };
						sub_param[0] = cmdid;
						sub_param[1] = error_type;
						sub_param[2] = static_cast<uint8_t>(error_code);
						sendSubsystemReponse(index, station_id, failResponse, sub_param, 3);
					}
					else
					{
						//Parameter 1:	Command in Progress
						//Parameter 2 : Error Type.
						//Parameter 3 : Error Code.
						uint8_t default_param[3] = { 0 };
						default_param[0] = cmdid;
						default_param[1] = error_type;
						default_param[2] = static_cast<uint8_t>(error_code);
						sendSubsystemReponse(index, station_id, failResponse, default_param, 3);
					}
				}
				else
				{
					uint8_t default_param[3] = { 0 };
					default_param[0] = static_cast<uint8_t>(error_code);
					default_param[1] = cmdid;
					default_param[2] = 0x01;
					sendSubsystemReponse(index, station_id, 0x66, default_param, 3);
				}
			}
			catch (KernelException& e){
				uint8_t param[2] = { e.getType(), e.getErr() };
				sendSubsystemReponse(index, station_id, failResponse, param, 2);
			}
			catch (std::exception& e){
				uint8_t param[2] = { 0, KernelSysException::KR_SYSTEM_UNKNOWN_ERROR };
				sendSubsystemReponse(index, station_id, failResponse, param, 2);
			}
		});
	}

	void EFEMHexApi::AddEventListListener(){
		for (auto sub : d->sub_handler){
			sub->AddEventListener();
		}
		for (auto sys_handler : d->system_handler)
		{
			sys_handler->AddEventListener();
		}
	}

	bool EFEMHexApi::systemNotInRemoteMode(uint32_t index)
	{
		KernelApi::ControlState state = getKernel()->getKernelModule<KernelApi>()->getControlState();
		if (state == KernelApi::ControlState::REMOTE_ON_LINE)
		{
			return false;
		}
		sendNCKSystemError(index, 0x1009);
		return true;
	}
	bool EFEMHexApi::subsystemStateNotNormalOrIsBusy(uint32_t index, uint8_t station_id, uint8_t command_id, const std::shared_ptr<IKernelSubSystem>&  subsystem)
	{
		auto subsystem_state = subsystem->getState();
		if (subsystem_state == IKernelSubSystem::State::SUB_UNKNOWN)
		{
			//Sub System com port not open.
			sendNCKSubsystemErrorCode(index, 0x82, station_id);
			return true;
		}
		if (command_id == 0x21)
		{
			if (subsystem_state == IKernelSubSystem::State::SUB_IDEL || subsystem_state == IKernelSubSystem::State::SUB_ERROR)
			{
				//Sub System not reset
				sendNCKSubsystemErrorCode(index, 0x80, station_id);
				return true;
			}
		}
		if (subsystem->isBusy())
		{
			//0x81%%. Sub-System%% is busy
			sendNCKSubsystemErrorCode(index, 0x81, station_id);
			return true;
		}
		return false;
	}

	bool EFEMHexApi::convertErrorCodeIntToUint8_t(int code, uint8_t &error_code_hight, uint8_t &error_code_low)
	{
		uint16_t num_uint16 = static_cast<uint16_t>(code);
		// 将uint16_t的值存储在uint8_t数组中
		error_code_low = static_cast<uint8_t>(num_uint16 & 0xFF); // 取低8位
		error_code_hight = static_cast<uint8_t>((num_uint16 >> 8) & 0xFF); // 取高8位
		return true;
	}
}