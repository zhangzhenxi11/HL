/**
* @file            sunway_subsystem_helper.h
* @brief           helper for sunway  protocol subsystem
* @author		   kai
*/

// Library: Fortrend
// Package: Subsystem/SunwayRobot


#include "Kernel/kernel_log.h"
#include "Kernel/kernel_configure.h"

#include "SunwayRobot/sunway_subsystem_helper.h" 

#include "Poco/String.h"
#include "Poco/Format.h"
#include "Poco/StringTokenizer.h"
#include "Poco/NumberParser.h"

#include <deque>
#include <condition_variable>
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
* SunwaySubSystemHelperPrivate
*/
class SunwaySubSystemHelperPrivate{
public:
	SunwaySubSystemHelperPrivate(SunwaySubSystemHelper* p, const std::string& name);
	void open()throw(KernelException);
	void close()throw(KernelException);
	bool setTimeout(const uint32_t milsecTime);
	void configure(const std::shared_ptr<KernelConfiguration> & config);

	bool sendRequest(const std::string& command) throw(KernelException);
	std::string recvResponse(unsigned int timeout_ms) throw(KernelException);
	std::string recvResponseRDY(unsigned int timeout_ms) throw(KernelException);
	std::shared_ptr<SunwaySubSystemHelper::DefinedError> getErrorCode(const int code_id);
	//2025-5-21 增加
	std::shared_ptr<SunwaySubSystemHelper::DefinedError> getErrorCode(const int type_id,const int code_id);

	//2025-8-04 增加
	void recvResponse2(unsigned int timeout_ms)throw(KernelException);

	bool getBusyState();
	std::string recvResponseRobotMessageMatching(unsigned int timeout_ms,
		const std::vector<std::string>& expectedPrefixes,
		const std::string& context) throw(KernelException);
public:
	int port = 1102;
	std::string ip_address = "192.168.1.100";
	std::shared_ptr<SOCKET> client;
	std::string name;
	std::string robotMessage = "";
	SunwaySubSystemHelper* p;
	std::mutex mtx;
	bool is_busy = false;

	std::list<std::shared_ptr<SunwaySubSystemHelper::DefinedError> > definedErrors;  //defined error from hardware
private:
	bool initialize();
	bool reconnect(); // 新增重连函数

public:
	bool isConnected = false;

	// 添加消息队列和同步机制
	std::deque<std::string> messageQueue;
	std::mutex queueMutex;
	std::condition_variable queueCV;

private:
	bool isReconnect = false;     //此次连接是否为重连
	int max_retries = 5;         // 最大重连次数
	int attempts = 0;            //重连次数
	int retry_delay = 2000;      // 每次重连的间隔时间（毫秒）
};


SunwaySubSystemHelperPrivate::SunwaySubSystemHelperPrivate(SunwaySubSystemHelper*p, const std::string& name)
	:p(p)
	, name(name)
{

}

bool SunwaySubSystemHelperPrivate::reconnect() {

	//std::lock_guard<std::mutex> lock(mtx); // 加锁保护重连过程
	isReconnect = true;

	while (attempts < max_retries) {
		logInform1(name.c_str(), Poco::format("尝试重新连接服务器, 尝试次数: %d", attempts + 1).c_str());

		close(); // 先关闭现有连接
		Sleep(retry_delay); // 等待重连间隔

		if (initialize())
		{ // 重新连接
			isReconnect = false;
			logInform1(name.c_str(), "重新连接服务器成功.");
			return true;
		}

		attempts++;
	}
	logError(name.c_str(), "多次尝试连接服务器失败.");
	return false;
}



void SunwaySubSystemHelperPrivate::open(){
	isConnected = initialize();
	if (!isConnected)
	{
		throw KernelSysException(__FILE__, KernelSysException::KR_MODULE_COMMUNICATION_ERROR, Poco::format("无法连接机械手服务器: %d", port));
	}
	
}


void SunwaySubSystemHelperPrivate::close(){
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

bool SunwaySubSystemHelperPrivate::initialize()
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

	std::thread* thread = new std::thread(&SunwaySubSystemHelperPrivate::recvResponse2, this,500 );
	thread->detach();

	//if (!isReconnect)
	//{
	//	//未重连，第一次连接
	//	if (!sendRequest("QRY:VER;"))
	//	{
	//		logError(name.c_str(), "机械手查找失败！.");
	//		closesocket(*client);
	//		return false;
	//	}

	//	std::string data = robotMessage;

	//	if (data != "ACK;" && data.find("RPS:")== std::string::npos)
	//	{
	//		logError(name.c_str(), "接收机械手数据错误.");
	//		return false;
	//	}
	//	robotMessage.clear();
	//}

	return true;
}
bool SunwaySubSystemHelperPrivate::setTimeout(const uint32_t milsecTime)
{
	int ret = setsockopt(*client, SOL_SOCKET, SO_RCVTIMEO, (const char*)&milsecTime, sizeof(milsecTime));
	return ret != SOCKET_ERROR;
}

bool SunwaySubSystemHelperPrivate::sendRequest(const std::string& command) throw(KernelException){

	std::lock_guard<std::mutex> lock(mtx); 

	if (!isConnected)
	{
		logInform1(name.c_str(), "检测到连接已断开，开始重连...");
		if (!reconnect()) {
			logError(name.c_str(), "重连失败，无法发送命令.");
			return false; // 重连失败
		}
	}
	//command
	is_busy = true;
	std::string data = command;
	data.push_back(0x0D);

	int err = send(*client, data.c_str(), data.size(), 0);
	if (err == SOCKET_ERROR || err == 0)
	{
		logError(name.c_str(), "发送命令: %s 失败.", command.c_str());
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

	is_busy = false;
	logInform1(name.c_str(), "Snd: %s ", command.c_str());
	return true;
}

/**
* recv string command from sunway (auto remove  0x0D)
*/
std::string SunwaySubSystemHelperPrivate::recvResponse(unsigned int timeout_ms) throw(KernelException){

	//std::lock_guard<std::mutex> lock(mtx); //加锁
	if (!isConnected) {
		logInform1(name.c_str(), "检测到连接已断开，开始重连...");
		if (!reconnect()) {
			is_busy = false;
			throw KernelSysException(__FILE__, KernelSysException::KR_MODULE_COMMUNICATION_TIMEOUT, "重连失败，无法接收数据.");
		}
	}
	setTimeout(timeout_ms);
	std::string data;
	char receiveBuf[1024];
	int recvLen = recv(*client, receiveBuf, 1024, 0);
	if (recvLen == SOCKET_ERROR || recvLen == 0)
	{
		logError(name.c_str(), "接收数据错误，可能连接已断开.");
		isConnected = false; // 连接断开
		is_busy = false;
		if (!reconnect()) {
			data = "error";
			throw KernelSysException(__FILE__, KernelSysException::KR_MODULE_COMMUNICATION_TIMEOUT, "重连失败，无法接收数据.");
		}
		// 重新尝试接收数据
		recvLen = recv(*client, receiveBuf, 1024, 0);
		if (recvLen == SOCKET_ERROR || recvLen == 0) {
			data = "error";
			is_busy = false;
			throw KernelSysException(__FILE__, KernelSysException::KR_MODULE_COMMUNICATION_TIMEOUT, "重连后接收数据失败.");
		}
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
			throw KernelSysException(__FILE__, KernelSysException::KR_MODULE_COMMUNICATION_TIMEOUT, "socket重新接收数据错误.");
		}
		data.append(receiveBuf, recvLen);
		found = data.find("\r"); //0x0D
		if (found != std::string::npos)
		{
			data.erase(found, -1);

		}
		else{
			is_busy = false;
			logInform1(name.c_str(), "Rcv_Format_Error: %s", data.c_str());
			throw KernelSysException(__FILE__, KernelSysException::KR_MODULE_COMMUNICATION_TIMEOUT, "返回数据格式错误.");
		}
		
	}
	
	is_busy = false;

	logInform1(name.c_str(), "Rcv: %s", data.c_str());
	return data;
}

/**
* recv string command from sunway (auto remove  0x0D)
*/
std::string SunwaySubSystemHelperPrivate::recvResponseRDY(unsigned int timeout_ms) throw(KernelException){
	//std::lock_guard<std::mutex> lock(mtx); //加锁
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
			logInform1(name.c_str(), "Rcv_Format_Error: %s", data.c_str());
			throw KernelSysException(__FILE__, KernelSysException::KR_MODULE_COMMUNICATION_TIMEOUT, "返回数据格式错误.");
		}

	}
	is_busy = false;
	//logInform1(name.c_str(), "Rcv: %s", data.c_str());
	return data;
}


std::shared_ptr<SunwaySubSystemHelper::DefinedError> SunwaySubSystemHelperPrivate::getErrorCode(const int code_id)
{
	std::shared_ptr < SunwaySubSystemHelper::DefinedError > res = std::make_shared<SunwaySubSystemHelper::DefinedError>();
	auto it = std::find_if(definedErrors.begin(), definedErrors.end(), [code_id](const std::shared_ptr<SunwaySubSystemHelper::DefinedError>& p) {
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

std::shared_ptr<SunwaySubSystemHelper::DefinedError> SunwaySubSystemHelperPrivate::getErrorCode(const int type_id, const int code_id)
{
	std::shared_ptr < SunwaySubSystemHelper::DefinedError > res = std::make_shared<SunwaySubSystemHelper::DefinedError>();
	auto it = std::find_if(definedErrors.begin(), definedErrors.end(), 
		[type_id,code_id](const std::shared_ptr<SunwaySubSystemHelper::DefinedError>& p) 
	{
		return (p->type == type_id) && (p->code == code_id);
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

/*
通过线程安全队列解决消息覆盖问题，使用条件变量实现精准同步，避免轮询间隔导致的丢失问题。改造后能正确处理快速连续消息（20ms间隔）。
*/
void SunwaySubSystemHelperPrivate::recvResponse2(unsigned int timeout_ms)throw(KernelException)
{
	std::string data; // 累积缓冲区
	while (true)
	{
		setTimeout(timeout_ms);
		//std::string data;
		//char receiveBuf[1024];
		char receiveBuf[4096];
		int recvLen = recv(*client, receiveBuf, sizeof(receiveBuf) - 1, 0);

		if (recvLen <= 0)
		{
			if (WSAGetLastError() == WSAETIMEDOUT) continue;
			Sleep(100);
			continue;
		}

		data.append(receiveBuf, recvLen);

		// 处理完整消息（以\r分隔）
		size_t pos = 0;
		while ((pos = data.find('\r')) != std::string::npos)
		{
			// 1. 提取从开始到\r之前的内容（不包含\r）
			std::string message = data.substr(0, pos);

			// 2. 从缓冲区删除已处理部分（包含\r）
			data.erase(0, pos + 1);  // +1 确保删除\r字符

			{
				std::lock_guard<std::mutex> lock(queueMutex);
				logInform1(name.c_str(), "Rcv_Format_: %s", message.c_str());
				messageQueue.push_back(message);// 这里message不包含\r
			}
			queueCV.notify_one();
		}
		Sleep(5); // 适当降低CPU占用

#if 0
		size_t found = data.find("\r"); //0x0D
		if (found != std::string::npos)
		{
			data.erase(found, -1);
		}
		else
		{
			setTimeout(3000);
			recvLen = recv(*client, receiveBuf, sizeof(receiveBuf) - 1, 0);
			if (recvLen == SOCKET_ERROR || recvLen == 0)
			{
				Sleep(100);
				continue;
			}
			data.append(receiveBuf, recvLen);
			found = data.find("\r"); //0x0D
			if (found != std::string::npos)
			{
				data.erase(found, -1);
			}
			else {
				//is_busy = false;
				logInform1(name.c_str(), "Rcv_Format_Error: %s", data.c_str());
				throw KernelSysException(__FILE__, KernelSysException::KR_MODULE_COMMUNICATION_TIMEOUT, "返回数据格式错误.");
			}
		}



		logInform1(name.c_str(), "Rcv_Format_: %s", data.c_str());

		is_busy = false;

		// ... 接收数据 ...
		{
			std::lock_guard<std::mutex> lock(queueMutex);
			messageQueue.push(data); // 入队不覆盖
		}
		queueCV.notify_one(); // 通知等待线程

		Sleep(5); // 适当降低CPU占用
#endif
	}
}

bool SunwaySubSystemHelperPrivate::getBusyState(){
	return is_busy;
}

std::string SunwaySubSystemHelperPrivate::recvResponseRobotMessageMatching(
	unsigned int timeout_ms,
	const std::vector<std::string>& expectedPrefixes,
	const std::string& context) throw(KernelException)
{
	auto matchesExpectedMessage = [&expectedPrefixes](const std::string& message) {
		if (expectedPrefixes.empty())
		{
			return true;
		}
		return std::any_of(expectedPrefixes.begin(), expectedPrefixes.end(),
			[&message](const std::string& prefix) {
				return !prefix.empty() && message.find(prefix) == 0;
			});
	};

	auto buildExpectedSummary = [&expectedPrefixes]() {
		std::ostringstream oss;
		for (size_t i = 0; i < expectedPrefixes.size(); ++i)
		{
			if (i > 0)
			{
				oss << ",";
			}
			oss << expectedPrefixes[i];
		}
		return oss.str();
	};

	const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout_ms);
	const std::string expectedSummary = buildExpectedSummary();
	std::unique_lock<std::mutex> lock(queueMutex);
	while (true)
	{
		for (auto it = messageQueue.begin(); it != messageQueue.end(); ++it)
		{
			if (!matchesExpectedMessage(*it))
			{
				continue;
			}

			auto msg = *it;
			messageQueue.erase(it);
			logInform1("WTR", "messageQueue pop[%s]: %s ", context.c_str(), msg.c_str());
			return msg;
		}

		if (std::chrono::steady_clock::now() >= deadline)
		{
			if (!context.empty() && !expectedSummary.empty())
			{
				logInform1(name.c_str(), "messageQueue timeout[%s], expected: %s", context.c_str(), expectedSummary.c_str());
			}
			return "";
		}

		if (!context.empty() && !expectedSummary.empty() && !messageQueue.empty())
		{
			logInform1(name.c_str(), "messageQueue keep waiting[%s], expected: %s, head: %s",
				context.c_str(), expectedSummary.c_str(), messageQueue.front().c_str());
		}
		queueCV.wait_until(lock, deadline);
	}
}

void SunwaySubSystemHelperPrivate::configure(const std::shared_ptr<KernelConfiguration> & config){

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
			std::shared_ptr<SunwaySubSystemHelper::DefinedError> errorObj(new SunwaySubSystemHelper::DefinedError);
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
* SunwaySubSystemHelper
*/
SunwaySubSystemHelper::SunwaySubSystemHelper(const std::string& name)
	: d(new SunwaySubSystemHelperPrivate(this, name)){

}


/**
* initialize
*/
void SunwaySubSystemHelper::enableProtocol(){
	d->open();
}


/**
* unInitialize
*/
void SunwaySubSystemHelper::disableProtocol(){
	d->close();
}


void SunwaySubSystemHelper::configSunway(const std::shared_ptr<KernelConfiguration> & config){
	d->configure(config);
}

/* Send command to sunway (auto add 0x0D) */
bool SunwaySubSystemHelper::sendRequest(const std::string& command) throw(KernelException){

	return d->sendRequest(command);
}

/**
* recv string command from sunway (auto remove 0x0A 0x0D)
*/
std::string SunwaySubSystemHelper::recvResponse(unsigned int timeout_ms) throw(KernelException){
	return d->recvResponse(timeout_ms);


}

void SunwaySubSystemHelper::recvResponse2(unsigned int timeout_ms) throw(KernelException)
{
	d->recvResponse2(timeout_ms);
}

/**
* recv string command from sunway (auto remove 0x0A 0x0D)
*/
std::string SunwaySubSystemHelper::recvResponseRDY(unsigned int timeout_ms) throw(KernelException){
	return d->recvResponseRDY(timeout_ms);


}

/**
* recv string command from sunway (auto remove 0x0A 0x0D)
*/
std::shared_ptr<SunwaySubSystemHelper::DefinedError> SunwaySubSystemHelper::getErrorCode(const int code_id) {
	return d->getErrorCode(code_id);


}

std::string SunwaySubSystemHelper::recvResponseRobotMessage(unsigned int timeout_ms) throw(KernelException)
{
	return d->recvResponseRobotMessageMatching(timeout_ms, {}, "legacy");
}

std::string SunwaySubSystemHelper::recvResponseRobotMessageMatching(
	unsigned int timeout_ms,
	const std::vector<std::string>& expectedPrefixes,
	const std::string& context) throw(KernelException)
{
	return d->recvResponseRobotMessageMatching(timeout_ms, expectedPrefixes, context);
}

void SunwaySubSystemHelper::clearRobotMessage() throw(KernelException)
{
	std::lock_guard<std::mutex> lock(d->queueMutex);
	while (!d->messageQueue.empty())
	{
		d->messageQueue.pop_front();
	}

}

std::shared_ptr<SunwaySubSystemHelper::DefinedError> SunwaySubSystemHelper::getErrorCode(const int type_id, const int code_id)
{
	return d->getErrorCode(type_id, code_id);
}

bool SunwaySubSystemHelper::getBusyState(){
	return d->getBusyState();
}

bool SunwaySubSystemHelper::getIsConnected()
{
	return d->isConnected;
}

// SunwaySubSystemHelper 类中新增 public 方法
std::string SunwaySubSystemHelper::sendCommand(const std::string& command,unsigned int timeout_ms) throw(KernelException)
{
	std::lock_guard<std::mutex> lock(d->mtx); // 锁定整个请求-响应过程
	if (!d->sendRequest(command))
	{
		throw KernelSysException(__FILE__, KernelSysException::KR_MODULE_COMMUNICATION_ERROR, "Send failed");
	}
	return d->recvResponse(timeout_ms);
}

KERNEL_NS_END
