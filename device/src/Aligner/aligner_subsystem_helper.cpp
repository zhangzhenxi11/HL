/**
* @file            sunway_subsystem_helper.h
* @brief           helper for sunway  protocol subsystem
* @author		   kai
*/

// Library: Fortrend
// Package: Subsystem/AlignerRobot


#include "Kernel/kernel_log.h"
#include "Kernel/kernel_configure.h"

#include "Aligner/aligner_subsystem_helper.h" 

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
* AlignerSubSystemHelperPrivate
*/
class AlignerSubSystemHelperPrivate{
public:
	AlignerSubSystemHelperPrivate(AlignerSubSystemHelper* p, const std::string& name);
	bool open()throw(KernelException);
	void close()throw(KernelException);
	bool setTimeout(const uint32_t milsecTime);
	void configure(const std::shared_ptr<KernelConfiguration> & config);

	bool sendRequest(const std::string& command) throw(KernelException);
	std::string recvResponse(unsigned int timeout_ms) throw(KernelException);
	std::string recvResponseRDY(unsigned int timeout_ms) throw(KernelException);
	std::shared_ptr<AlignerSubSystemHelper::DefinedError> getErrorCode(const int code_id);
	bool getBusyState();
public:
	int port = 1102;
	std::string ip_address = "192.168.1.100";
	std::shared_ptr<SOCKET> client;
	std::string name;

	AlignerSubSystemHelper* p;
	std::mutex mtx;
	bool is_busy = false;

	std::list<std::shared_ptr<AlignerSubSystemHelper::DefinedError> > definedErrors;  //defined error from hardware
private:
	bool initialize();
private:
	bool isConnected = false;
};


AlignerSubSystemHelperPrivate::AlignerSubSystemHelperPrivate(AlignerSubSystemHelper*p, const std::string& name)
	:p(p)
	, name(name)
{

}


bool AlignerSubSystemHelperPrivate::open(){
	isConnected = initialize();
	if (!isConnected)
	{
		throw KernelSysException(__FILE__, KernelSysException::KR_MODULE_COMMUNICATION_ERROR, Poco::format("无法连接寻边器服务器: %d", port));
	}
	return isConnected;
}


void AlignerSubSystemHelperPrivate::close(){
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

bool AlignerSubSystemHelperPrivate::initialize()
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
		logError(name.c_str(), "连接机械手服务器失败！");
		closesocket((*client));
		WSACleanup();
		return false;
	}
	isConnected = true;
	return isConnected;
}
bool AlignerSubSystemHelperPrivate::setTimeout(const uint32_t milsecTime)
{
	int ret = setsockopt(*client, SOL_SOCKET, SO_RCVTIMEO, (const char*)&milsecTime, sizeof(milsecTime));
	return ret != SOCKET_ERROR;
}

/**
* Send command to sunway (auto add 0x0D)
*/
bool AlignerSubSystemHelperPrivate::sendRequest(const std::string& command) throw(KernelException){
	if (!isConnected)
	{
		return false;
	}
	mtx.lock();
	//int optval = 1;
	//setsockopt(*client, SOL_SOCKET, SO_RCVBUF, (const char*)&optval, sizeof(optval));
	//command
	is_busy = true;
	std::string data = command;
	data.push_back(0x0D);
	data.push_back(0x0A);
	int err = send(*client, data.c_str(), data.size(), 0);
	if (err == SOCKET_ERROR || err == 0){
		logError(name.c_str(), "发送命令: %s 失败", command.c_str());
		is_busy = false;
		mtx.unlock();
		return false;
	};
	Sleep(50);
	//log
	logInform1(name.c_str(), "Snd: %s ", command.c_str());
	return true;
}

/**
* recv string command from sunway (auto remove  0x0D)
*/
std::string AlignerSubSystemHelperPrivate::recvResponse(unsigned int timeout_ms) throw(KernelException){
	setTimeout(timeout_ms);

	std::string data;
	char receiveBuf[1024];
	int recvLen = recv(*client, receiveBuf, 1024, 0);
	if (recvLen == SOCKET_ERROR || recvLen == 0)
	{
		data = "error";
		is_busy = false;
		mtx.unlock();
		throw KernelSysException(__FILE__, KernelSysException::KR_MODULE_COMMUNICATION_TIMEOUT, "socket接收数据错误.");
	}
	data.append(receiveBuf, recvLen);
	size_t found = data.find("\n"); //0x0D
	if (found != std::string::npos)
	{
		//data.erase(found, -1);
		int len = data.length();
		data = data.substr(0, len - 2);
	}
	else
	{
		setTimeout(30000);
		recvLen = recv(*client, receiveBuf, 1024, 0);
		if (recvLen == SOCKET_ERROR || recvLen == 0)
		{
			data = "error";
			is_busy = false;
			mtx.unlock();
			throw KernelSysException(__FILE__, KernelSysException::KR_MODULE_COMMUNICATION_TIMEOUT, "socket重新接收数据错误.");
		}
		data.append(receiveBuf, recvLen);
		found = data.find("\n"); //0x0D
		if (found != std::string::npos)
		{
			//data.erase(found, -1);
			int len = data.length();
			data = data.substr(0, len - 2);
		}
		else{
			is_busy = false;
			mtx.unlock();
			logInform1(name.c_str(), "Rcv_Format_Error: %s", data.c_str());
			throw KernelSysException(__FILE__, KernelSysException::KR_MODULE_COMMUNICATION_TIMEOUT, "返回数据格式错误.");
		}
		
	}
	is_busy = false;
	mtx.unlock();
	logInform1(name.c_str(), "Rcv: %s", data.c_str());
	return data;
}

/**
* recv string command from sunway (auto remove  0x0D)
*/
std::string AlignerSubSystemHelperPrivate::recvResponseRDY(unsigned int timeout_ms) throw(KernelException){
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
		else{
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


std::shared_ptr<AlignerSubSystemHelper::DefinedError> AlignerSubSystemHelperPrivate::getErrorCode(const int code_id)
{
	std::shared_ptr < AlignerSubSystemHelper::DefinedError > res = std::make_shared<AlignerSubSystemHelper::DefinedError>();
	auto it = std::find_if(definedErrors.begin(), definedErrors.end(), [code_id](const std::shared_ptr<AlignerSubSystemHelper::DefinedError>& p) {
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


bool AlignerSubSystemHelperPrivate::getBusyState(){
	return is_busy;
}

void AlignerSubSystemHelperPrivate::configure(const std::shared_ptr<KernelConfiguration> & config){

	port = std::stoi(config->getString("port", "1102"));
	ip_address = config->getString("ip", "192.168.1.100");
	//logInform1(name.c_str(), "read configure: port = %d, ip_address = %s", port, ip_address.c_str());
	if (config->has("ErrorTxt")){
		std::string errFileName = config->getString("ErrorTxt", name + ".txt");
		// read file
		std::ifstream infile;
		std::string buffstr;
		infile.open(errFileName, std::ios::in);
		if (!infile.is_open()){
			throw KernelSysException(__FILE__, KernelSysException::KR_SYSTEM_WITHOUT_RESOURCE, Poco::format("defined error file: %s open faild.", errFileName));
		}
		// read line and line
		while (getline(infile, buffstr))
		{
			std::shared_ptr<AlignerSubSystemHelper::DefinedError> errorObj(new AlignerSubSystemHelper::DefinedError);
			Poco::StringTokenizer token(buffstr, ",", Poco::StringTokenizer::TOK_IGNORE_EMPTY);
			if (token.count() != 3){
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
* AlignerSubSystemHelper
*/
AlignerSubSystemHelper::AlignerSubSystemHelper(const std::string& name)
	: d(new AlignerSubSystemHelperPrivate(this, name)){

}


/**
* initialize
*/
bool AlignerSubSystemHelper::enableProtocol(){
	return d->open();
}


/**
* unInitialize
*/
void AlignerSubSystemHelper::disableProtocol(){
	d->close();
}


void AlignerSubSystemHelper::configAligner(const std::shared_ptr<KernelConfiguration> & config){
	d->configure(config);
}

/**
* Send command to sunway (auto add 0x0D)
*/
bool AlignerSubSystemHelper::sendRequest(const std::string& command) throw(KernelException){

	return d->sendRequest(command);
}

/**
* recv string command from sunway (auto remove 0x0A 0x0D)
*/
std::string AlignerSubSystemHelper::recvResponse(unsigned int timeout_ms) throw(KernelException){
	return d->recvResponse(timeout_ms);


}

/**
* recv string command from sunway (auto remove 0x0A 0x0D)
*/
std::string AlignerSubSystemHelper::recvResponseRDY(unsigned int timeout_ms) throw(KernelException){
	return d->recvResponseRDY(timeout_ms);


}

/**
* recv string command from sunway (auto remove 0x0A 0x0D)
*/
std::shared_ptr<AlignerSubSystemHelper::DefinedError> AlignerSubSystemHelper::getErrorCode(const int code_id) {
	return d->getErrorCode(code_id);


}

bool AlignerSubSystemHelper::getBusyState(){
	return d->getBusyState();
}

KERNEL_NS_END
