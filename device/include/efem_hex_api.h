/**
* @file            efem_hex_api.h
* @brief           efem hex for fortrend for gui
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Api/HEX


#ifndef _FORTREND_HEX_API_INCLUDE_ 
#define _FORTREND_HEX_API_INCLUDE_
#include "kernel/Fortrend/tcp_server_api.h"
#include "Kernel/kernel_exception.h"

#include <memory>

namespace FC{

class IKernel;
class IKernelSubSystem;
class IKernelCommand;

#define CMD_OK_KEY  "OK"
#define CMD_FAILD_KEY  "FAILD"
#define CMD_MACRO_ID_KEY  "MACRO_ID"

/**
* hex api of kernel api
*/
class  EFEMHexApi : public TcpServerApi
{
public:
	DECLARE_PTR(EFEMHexApi)
	EFEMHexApi(IKernel*  kernel);
	virtual std::string getName() const override { return "HOST"; }
	IKernel* getKernel()const;
	//ack /nck
	void sendACK(uint32_t index);
	void sendNCK(uint32_t index, uint8_t staionid, uint8_t type, uint16_t errCode);
	void sendNCKSubsystemErrorCode(uint32_t index, uint8_t errCode, uint8_t subsystem_id);
	void sendNCKSystemError(uint32_t index,uint16_t errCode);
	void sendSubsystemErrorDetected(uint32_t index, uint8_t station_id, uint8_t command_id, uint8_t errCode);
	void sendSubsystemErrorCode(uint32_t index, uint8_t errCode, uint8_t subsystem_id);
	void sendSubsystemErrorCode_unint16(uint32_t index, uint16_t errCode);
	//response
	void sendSubsystemReponse(uint32_t index, uint8_t station_id,uint8_t response_id,uint8_t* param,uint8_t len);
	void sendSystemReponse(uint32_t index, uint16_t response_id, uint8_t* param, uint8_t len);
	void sendDefinedReponse(uint32_t index, uint8_t response_id, uint8_t* param, uint8_t len);
	void sendEventReponse(uint32_t index, uint16_t response_id, uint8_t* param, uint8_t len);
	void sendEvent(uint16_t event_id, uint8_t* param, uint8_t len);
	void sendEventResetCommandCompleted(uint8_t station_id);
	void sendEventResetCommandError(uint8_t station_id);
	void AddEventListListener();
	bool systemNotInRemoteMode(uint32_t index);
	bool subsystemStateNotNormalOrIsBusy(uint32_t index, uint8_t station_id, uint8_t command_id, const std::shared_ptr<IKernelSubSystem>& subsystem);
	bool convertErrorCodeIntToUint8_t(int code, uint8_t &error_code_hight, uint8_t &error_code_low);

	void submitTask(
		uint32_t index,
		uint8_t station_id,
		const std::shared_ptr<IKernelSubSystem>& sub,
		const::std::shared_ptr<IKernelCommand> & cmd,
		uint8_t cmdid=0x21,
		uint8_t isArmHasWaferOrRFIDChannel = 0);

protected:
	virtual void process()override;
	virtual void onConnect() override;
	virtual void onDisConnect() override;
	virtual bool sendMessage(const char* data, unsigned int len);
private:
	bool handle_subsystem(const char* data, size_t len);
	bool handle_defined(const char* data, size_t len);
	bool handle_system(const char* data, size_t len);
private:
	DECLARE_PRIVATE(EFEMHexApi)
};

}

#endif
