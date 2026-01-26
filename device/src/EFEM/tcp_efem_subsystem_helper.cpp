/**
* @file            tcp_efem_subsystem_helper.h
* @brief           helper for sunway  protocol subsystem
* @author		   kai
*/

// Library: Fortrend
// Package: TcpEfem/TcpEfem


#include "Kernel/kernel_log.h"
#include "Kernel/kernel_configure.h"

#include "EFEM\tcp_efem_subsystem_helper.h" 

#include "Poco/String.h"
#include "Poco/Format.h"
#include "Poco/StringTokenizer.h"
#include "Poco/NumberParser.h"

#include <list>
#include <mutex>
#include <fstream>
#include <sstream>

#include <algorithm>
#include <thread>
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#pragma once

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

KERNEL_NS_BEGIN


/**
* TcpEfemSubSystemHelperPrivate
*/
class TcpEfemSubSystemHelperPrivate {
public:
	TcpEfemSubSystemHelperPrivate(TcpEfemSubSystemHelper* p, const std::string& name);
	bool open()throw(KernelException);
	void close()throw(KernelException);
	bool setTimeout(const uint32_t milsecTime);
	void configure(const std::shared_ptr<KernelConfiguration>& config);

	bool sendRequest(const std::string& command) throw(KernelException);
	std::string recvResponse(unsigned int timeout_ms) throw(KernelException);
	std::string recvResponseRDY(unsigned int timeout_ms) throw(KernelException);
	std::shared_ptr<TcpEfemSubSystemHelper::DefinedError> getErrorCode(const int code_id);
	bool getBusyState();

	void recvResponse2();
public:
	int port = 1102;
	std::string ip_address = "192.168.1.100";
	std::shared_ptr<SOCKET> client;
	std::string name;

	std::string axiLocation = "";
	std::string robotMessage = "";

	TcpEfemSubSystemHelper* p;
	std::mutex mtx;
	std::mutex mtx2;
	std::mutex comm_mutex; // 统一通信锁
	std::mutex recv_mutex;

	bool is_busy = false;

	std::list<std::shared_ptr<TcpEfemSubSystemHelper::DefinedError> > definedErrors;  //defined error from hardware
private:
	bool initialize();
	bool reconnect(); // 新增重连函数
public:
	bool isConnected = false;
	int max_retries = 5;         // 最大重连次数
	int retry_delay = 2000;      // 每次重连的间隔时间（毫秒）
};


TcpEfemSubSystemHelperPrivate::TcpEfemSubSystemHelperPrivate(TcpEfemSubSystemHelper* p, const std::string& name)
	:p(p)
	, name(name)
{

}


bool TcpEfemSubSystemHelperPrivate::open() {
	isConnected = initialize();
	if (!isConnected)
	{
		throw KernelSysException(__FILE__, KernelSysException::KR_MODULE_COMMUNICATION_ERROR, Poco::format("无法连接EFEM服务器: %d", port));
	}
	return isConnected;
}

void TcpEfemSubSystemHelperPrivate::close() {
	if (client)
	{
		if ((*client) != INVALID_SOCKET)
		{
			shutdown(*client, SD_BOTH);
			closesocket(*client);
			*client = INVALID_SOCKET;
		}
	}

	WSACleanup();
	isConnected = false;
	return;
}

bool TcpEfemSubSystemHelperPrivate::initialize()
{
	WORD verision = MAKEWORD(2, 2);
	WSADATA lpData;
	int intEr = WSAStartup(verision, &lpData);
	if (intEr != 0)
	{
		logError(name.c_str(), "初始化Windows的套接字失败！");
		return false;
	}
	client = std::make_shared<SOCKET>(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));


	if ((*client) == INVALID_SOCKET)
	{
		logError(name.c_str(), "初始化客户端套接字失败！");
		WSACleanup();
		return false;
	}

	logInform(name.c_str(),"port:%d,ip_address:%s",port, ip_address);

	DWORD timeout = 1000;
	setsockopt(*client, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
	// 设置服务器地址信息
	sockaddr_in serverAddr{};
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	if (inet_pton(AF_INET, ip_address.c_str(), &(serverAddr.sin_addr)) <= 0)
	{
		logError(name.c_str(), "无效的地址.");
		closesocket(*client);
		WSACleanup();
		return false;
	}

	// 连接服务器
	if (connect((*client), reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) < 0)
	{
		logError(name.c_str(), "连接EFEM服务器失败！");
		closesocket((*client));
		WSACleanup();
		return false;
	}
	isConnected = true;

	//2025-5-29 注释 执行函数报错
	//std::thread* thread = new std::thread(&TcpEfemSubSystemHelperPrivate::recvResponse2, this);
	//thread->detach();

	/*sendRequest("ACK:READY/COMM;");*/
	return true;
}

bool TcpEfemSubSystemHelperPrivate::reconnect()
{
	int attempts = 0;
	while (attempts < max_retries) {
		logInform1(name.c_str(), Poco::format("尝试重新连接服务器, 尝试次数: %d", attempts + 1).c_str());

		close(); // 先关闭现有连接
		Sleep(retry_delay); // 等待重连间隔
		if (initialize()) { // 重新连接
			logInform1(name.c_str(), "重新连接服务器成功.");
			return true;
		}

		attempts++;
	}
	logError(name.c_str(), "多次尝试连接服务器失败.");
	return false;
}



bool TcpEfemSubSystemHelperPrivate::setTimeout(const uint32_t milsecTime)
{
	int ret = setsockopt(*client, SOL_SOCKET, SO_RCVTIMEO, (const char*)&milsecTime, sizeof(milsecTime));
	return ret != SOCKET_ERROR;
}

/**
* Send command to sunway (auto add 0x0D)
*/
bool TcpEfemSubSystemHelperPrivate::sendRequest(const std::string& command) throw(KernelException) {
	std::lock_guard<std::mutex> lock(comm_mutex);

	if (!isConnected)
	{
		logInform1(name.c_str(), "检测到连接已断开，开始重连...");
		if (!reconnect()) {
			logError(name.c_str(), "重连失败，无法发送命令.");
			return false; // 重连失败
		}
	}
	//logInform1(name.c_str(), "Snd:Sart %s ", command.c_str());

	is_busy = true;
	std::string data = command;
	uint8_t ack[] = { 0x06 };
	int err = 0;

	if (data == std::string("ACK"))  //ACK十六进制：0x06  
	{
		err = send(*client, (char*)ack, sizeof(ack), 0);
		logInform1(name.c_str(), "Snd: %d ", ack[0]);

	}
	else
	{
		data.push_back('\r');
		err = send(*client, data.c_str(), data.size(), 0);
		logInform1(name.c_str(), "Snd: %s ", data.c_str());
	}

	if (err == SOCKET_ERROR || err == 0) {
		logError(name.c_str(), "发送命令: %s 失败", data.c_str());
		is_busy = false;
		isConnected = false; // 连接已断开

		if (!reconnect()) {
			logError(name.c_str(), "重连失败，发送命令终止.");
			return false;
		}
		// 重新发送命令
		err = send(*client, data.c_str(), data.size(), 0);
		if (err == SOCKET_ERROR || err == 0) {
			logError(name.c_str(), "重连后发送命令仍然失败.");
			return false;
		}
	};
	Sleep(50);
	//log

	is_busy = false;
	//mtx.unlock();
	return true;
}

/**
* recv string command from sunway (auto remove  0x0D)
*/
std::string TcpEfemSubSystemHelperPrivate::recvResponse(unsigned int timeout_ms) throw(KernelException)
{
	std::lock_guard<std::mutex> lock(recv_mutex);

	if (!isConnected) {
		logInform1(name.c_str(), "检测到连接已断开，开始重连...");
		if (!reconnect()) {
			is_busy = false;
			throw KernelSysException(__FILE__, KernelSysException::KR_MODULE_COMMUNICATION_TIMEOUT, "重连失败，无法接收数据.");
		}
	}
	int reconnet_cout = 5;//5
	int count = 0;
	is_busy = true;
	int recvLen;
	setTimeout(timeout_ms);
	std::string data;
	char receiveBuf[1024];
	try {
		while (count < reconnet_cout)
		{
			Sleep(1000);
			recvLen = recv(*client, receiveBuf, 1024, 0);
			if (recvLen == SOCKET_ERROR || recvLen == 0)
			{
				logInform1(name.c_str(), Poco::format("尝试重新接收数据, 尝试次数: %d", count + 1).c_str());
				data = "error";
				is_busy = false;
				if (!reconnect())
				{
					data = "error";
					throw KernelSysException(__FILE__, KernelSysException::KR_MODULE_COMMUNICATION_TIMEOUT, "重连失败，无法接收数据.");
				}
			}
			else
			{
				break;
			}
			count++;
		}
		
		data.append(receiveBuf, recvLen);
		size_t found = data.find("\r"); //0x0D
		if (found != std::string::npos)
		{
			data.erase(found, -1);
		}
		else
		{
			setTimeout(3000);
			recvLen = recv(*client, receiveBuf, 1024, 0);
			if (recvLen == SOCKET_ERROR || recvLen == 0)
			{
				data = "error";
				is_busy = false;
				//mtx2.unlock();
				throw KernelSysException(__FILE__, KernelSysException::KR_MODULE_COMMUNICATION_TIMEOUT, "socket重新接收数据错误.");
			}
			data.append(receiveBuf, recvLen);
			found = data.find("\r"); //0x0D
			if (found != std::string::npos)
			{
				data.erase(found, -1);
			}
			else {
				is_busy = false;
				//mtx2.unlock();
				logInform1(name.c_str(), "Rcv_Format_Error: %s", data.c_str());
				throw KernelSysException(__FILE__, KernelSysException::KR_MODULE_COMMUNICATION_TIMEOUT, "返回数据格式错误.");
			}

		}
	}
	catch (...)
	{
		// +++ 异常时关闭连接 +++
		//if (isConnected) {
		//	closesocket(*client);
		//	*client = INVALID_SOCKET;
		//	isConnected = false;
		//}
		close(); 
		is_busy = false;
		throw; // 重新抛出异常
	}
	is_busy = false;
	logInform1(name.c_str(), "Rcv: %s", data.c_str());
	//mtx2.unlock();
	return data;
}


void TcpEfemSubSystemHelperPrivate::recvResponse2() throw(KernelException)
{
	
	while (true)
	{
		setTimeout(300000);
		std::string RecvData;
		
		char receiveBuf[1024] = {0};
		
		int recvLen = recv(*client, receiveBuf, 1024, 0);

		if (recvLen == SOCKET_ERROR || recvLen == 0)
		{
			continue;
		}
		RecvData.append(receiveBuf, recvLen);
		
		if (RecvData != std::string(""))
		{
			robotMessage = RecvData;
			logInform1(name.c_str(), "其他--Rcv: %s", robotMessage.c_str());
		}
	}
	Sleep(1);
}
/**
* recv string command from sunway (auto remove  0x0D)
*/
std::string TcpEfemSubSystemHelperPrivate::recvResponseRDY(unsigned int timeout_ms) throw(KernelException) {
	Sleep(150);
	setTimeout(timeout_ms);

	std::string data;
	char receiveBuf[1024];
	int recvLen = recv(*client, receiveBuf, 1024, 0);
	if (recvLen == SOCKET_ERROR || recvLen == 0)
	{
		logInform1(name.c_str(), "Rcv_Error, Retry Once");
		recvLen = recv(*client, receiveBuf, 1024, 0);
		if (recvLen == SOCKET_ERROR || recvLen == 0)
		{
			is_busy = false;
			mtx.unlock();
			data = "error";
			throw KernelSysException(__FILE__, KernelSysException::KR_MODULE_COMMUNICATION_TIMEOUT, "socket接收数据错误.");
		}
	}

	data.append(receiveBuf, recvLen);
	std::replace(data.begin(), data.end(), '\r', ' ');
	logInform1(name.c_str(), "Rcv: %s", data.c_str());
	std::string target = " _RDY ";
	size_t found = data.find(target);
	if (found != std::string::npos)
	{
		data.erase(found, target.length());
	}
	else
	{
		setTimeout(1000);
		recvLen = recv(*client, receiveBuf, 1024, 0);
		if (recvLen == SOCKET_ERROR || recvLen == 0)
		{
			is_busy = false;
			mtx.unlock();
			data = "error";
			throw KernelSysException(__FILE__, KernelSysException::KR_MODULE_COMMUNICATION_TIMEOUT, "socket重新接收数据错误.");
		}
		data.append(receiveBuf, recvLen);
		std::replace(data.begin(), data.end(), '\r', ' ');
		found = data.find(target);
		if (found != std::string::npos)
		{
			data.erase(found, target.length());
		}
		else {
			is_busy = false;
			mtx.unlock();
			logInform1(name.c_str(), "Rcv_Format_Error: %s", data.c_str());
			throw KernelSysException(__FILE__, KernelSysException::KR_MODULE_COMMUNICATION_TIMEOUT, "返回数据格式错误.");
		}

	}
	is_busy = false;
	mtx.unlock();
	//logInform1(name.c_str(), "Rcv: %s", data.c_str());
	return data;
}


std::shared_ptr<TcpEfemSubSystemHelper::DefinedError> TcpEfemSubSystemHelperPrivate::getErrorCode(const int code_id)
{
	std::shared_ptr < TcpEfemSubSystemHelper::DefinedError > res = std::make_shared<TcpEfemSubSystemHelper::DefinedError>();
	auto it = std::find_if(definedErrors.begin(), definedErrors.end(), [code_id](const std::shared_ptr<TcpEfemSubSystemHelper::DefinedError>& p) {
		return p->code == code_id;
		});
	if (it != definedErrors.end()) {
		res = *it;
	}
	else
	{
		res->type = 1;
		res->code = code_id;
		res->message = "Error code definition not found in hardware. ";
		res->needReset = true;
	}
	return res;
}


bool TcpEfemSubSystemHelperPrivate::getBusyState() {
	return is_busy;
}

void TcpEfemSubSystemHelperPrivate::configure(const std::shared_ptr<KernelConfiguration>& config) {

	port = std::stoi(config->getString("port", "1102"));
	ip_address = config->getString("ip", "192.168.1.100");
	logInform(name.c_str(), "WTR read configure: port = %d, ip_address = %s", port, ip_address.c_str());

	if (config->has("ErrorTxt")) {
		std::string errFileName = config->getString("ErrorTxt", "Error_WTR.txt");
		// read file
		std::ifstream infile;
		std::string buffstr;
		infile.open(errFileName, std::ios::in);
		if (!infile.is_open()) {
			throw KernelSysException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, Poco::format("defined error file: %s open faild.", errFileName));
		}
		// read line and line
		while (getline(infile, buffstr))
		{
			std::shared_ptr<TcpEfemSubSystemHelper::DefinedError> errorObj(new TcpEfemSubSystemHelper::DefinedError);
			Poco::StringTokenizer token(buffstr, ",", Poco::StringTokenizer::TOK_IGNORE_EMPTY);
			if (token.count() != 3) {
				throw KernelSysException(__FILE__, KernelSysException::KR_SYSTEM_UNKNOWN_ERROR, Poco::format("defined error file: %s format error.", errFileName));
			}

			errorObj->type = Poco::NumberParser::parse(token[0]);//type
			errorObj->code = Poco::NumberParser::parse(token[1]);//code
			errorObj->message = Poco::trim(token[2]);  //message

			errorObj->needReset = errorObj->message.back() == '*';  //need to reset
			if (errorObj->needReset) errorObj->message.pop_back();
			//add to defined errors
			definedErrors.push_back(errorObj);
		}
	}
}





/**
* TcpEfemSubSystemHelper
*/
TcpEfemSubSystemHelper::TcpEfemSubSystemHelper(const std::string& name)
	: d(new TcpEfemSubSystemHelperPrivate(this, name)) {

}


/**
* initialize
*/
bool TcpEfemSubSystemHelper::enableProtocol() {
	return d->open();
}


/**
* unInitialize
*/
void TcpEfemSubSystemHelper::disableProtocol() {
	d->close();
}



void TcpEfemSubSystemHelper::configTcpEfem(const std::shared_ptr<KernelConfiguration>& config) {
	d->configure(config);
}

/**
* Send command to sunway (auto add 0x0D)
*/
bool TcpEfemSubSystemHelper::sendRequest(const std::string& command) throw(KernelException) {

	return d->sendRequest(command);
}

/**
* recv string command from sunway (auto remove 0x0A 0x0D)
*/
std::string TcpEfemSubSystemHelper::recvResponse(unsigned int timeout_ms) throw(KernelException) {
	return d->recvResponse(timeout_ms);
}

//轴位置
std::string TcpEfemSubSystemHelper::recvResponseAxiLocation(unsigned int timeout_ms) throw(KernelException)
{
	return d->axiLocation;
}

//其他消息
std::string TcpEfemSubSystemHelper::recvResponseRobotMessage(unsigned int timeout_ms) throw(KernelException)
{
	return d->robotMessage;
}


void TcpEfemSubSystemHelper::setRobotMessage() throw(KernelException)
{
	d->robotMessage = "";
}

bool TcpEfemSubSystemHelper::getIsConnected()
{
	return d->isConnected;
}

/**
* recv string command from sunway (auto remove 0x0A 0x0D)
*/
std::string TcpEfemSubSystemHelper::recvResponseRDY(unsigned int timeout_ms) throw(KernelException) {
	return d->recvResponseRDY(timeout_ms);
}

/**
* recv string command from sunway (auto remove 0x0A 0x0D)
*/
std::shared_ptr<TcpEfemSubSystemHelper::DefinedError> TcpEfemSubSystemHelper::getErrorCode(const int code_id) {
	return d->getErrorCode(code_id);


}

bool TcpEfemSubSystemHelper::getBusyState() {
	return d->getBusyState();
}

void TcpEfemSubSystemHelper::setBusyState()
{
	d->is_busy = false;
}

KERNEL_NS_END
