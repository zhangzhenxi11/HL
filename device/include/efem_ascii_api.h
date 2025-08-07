// Library: LibDevicePlugin
// Package: Api/ASCII
//
// ascii api of kernel api
//
// author xielonghua
//


#ifndef _FORTREND_ASCII_API_INCLUDE_ 
#define _FORTREND_ASCII_API_INCLUDE_
#include "Kernel/Fortrend/tcp_server_api.h"
#include "tcp_client_api.h"
#include "Kernel/kernel_exception.h"
#include <memory>

namespace FC{

class IKernel;


/**
* pio tech api of kernel api
*/
class  EFEMAsciiApi : public TcpClientApi
{
public:
	DECLARE_PTR(EFEMAsciiApi)

	// 例如：GET:STATE/parameter1
	//            
	//Command 构成： Type : Message    Message构成: Base/parameter   
	//Type: Base/parameters

	typedef enum{ MOV, GET, SET, INF, ABS, EVT, ACK, NAK, TYPE_UNKNOWN } Type;

	typedef enum{ 
		READY, 
		INIT, 
		LOCK, 
		UNLOCK, 
		OPEN,
		CLOSE ,
		WAFSH,
		MAPDT,
		GOTO,
		LOAD,
		UNLOAD,
		ALIGN,
		HOLD,
		RESTR,
		ABORT,
		MODE,
		SIGOUT,
		STATE,
		SIGSTAT,
		TOWER,
		CSTID,
		SPEED,
		CONTROLMODE,
		HEARTBEAT,
		TOOLSLOT,
		CLAMP,
		TRIGGER,
		ALARM,
		POD, 
		DOORSTAT,
		TRANSF, //2025-7-22 新加
		BASE_UNKNOWN
	} Base;
	//空闲，等待回复，请求失败，响应超时，完成 
	typedef enum{ TRANS_IDEL, TRANS_WAIT_REPLY, TRANS_REQUEST_FAILD, TRANS_RESPONSE_TIMEOUT, TRANS_FINISHED }  State;
	
	typedef struct {
		Base base;
		std::vector<std::string> paramers;
	} Message;

	typedef struct {
		Type type;
		std::shared_ptr<Message> message;
	} Command;

	EFEMAsciiApi(IKernel*  kernel);
	virtual std::string getName() const { return "EFEMClient"; }
public:
	// response ACK for command 响应ACK命令 ,在刚开始连接的时，客户端用到。其他sendxx接口用不到。
	bool sendACK(const std::shared_ptr<Message>& message);
	// response NAK for command 
	bool sendNAK(const std::shared_ptr<Message>& message, int code);
	// response CAN for command
	// response INF for command
	bool sendINF(const std::shared_ptr<Message>& message);
	// response ABS for command   响应ABS命令
	bool sendABS(const std::shared_ptr<Message>& message, const std::string&source,  int type,  int code);
	// response EVT for command  响应EVT命令
	bool sendEVT(const std::shared_ptr<Message>& message);

	virtual bool sendMessage(const char* data, unsigned int len)override;

	std::string getData();

	std::string message_data;

protected:
	virtual void handle(const std::shared_ptr<Command>& command) = 0;
	virtual void update() = 0;
	virtual void process()override;
private:
	virtual void logMessage(const char* data, unsigned int len, const std::string& prefix)override;
	virtual void onConnect() override;
	virtual void onDisConnect() override;

	void onDataRecv(const char* data, unsigned int len);
	void processSingleMessage(const std::string& message);

	void processEFEMessage(std::string& message);

private:
	DECLARE_PRIVATE(EFEMAsciiApi)
};

}

#endif
