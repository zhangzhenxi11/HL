/**
* @file            tcp_client_api.h
* @brief           tcp client
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Subsystem/TcpClient


#include "Kernel/kernel_log.h"
#include  "tcp_client_api.h"
#include <iostream>
#include <mutex>
#include <vector>
#include <list>
#include <sstream>
#include <thread>
//#include <WinSock2.h>
//#include <WS2tcpip.h>
#include "Kernel/Utils/utils_socket.h"
#include <cstring>
#include "Poco/format.h"

using namespace std;

KERNEL_NS_BEGIN



class RingBuffer {
private:
	std::vector<char> buffer;
	size_t head = 0;
	size_t tail = 0;
	size_t capacity;
	std::mutex mtx;

	size_t next_pos(size_t pos) const {
		return (pos + 1) % capacity;
	}

public:
	RingBuffer(size_t capacity) : capacity(capacity), buffer(capacity) {}

	bool write(const char* data, size_t bytes) {
		std::lock_guard<std::mutex> lock(mtx);
		for (size_t i = 0; i < bytes; i++) {
			if (next_pos(tail) == head) { // Buffer is full
				return false;
			}
			buffer[tail] = data[i];
			tail = next_pos(tail);
		}
		return true;
	}

	size_t read(char* data, size_t bytes) {
		std::lock_guard<std::mutex> lock(mtx);
		size_t count = 0;
		while (head != tail && count < bytes) {
			data[count++] = buffer[head];
			head = next_pos(head);
		}
		return count;
	}

	size_t peek(size_t skip, char* data, size_t bytes) {
		std::lock_guard<std::mutex> lock(mtx);
		size_t count = 0;
		size_t current_pos = head;
		while (count < skip && current_pos != tail) { // Skip initial bytes
			current_pos = next_pos(current_pos);
			count++;
		}
		count = 0;
		while (current_pos != tail && count < bytes) {
			data[count++] = buffer[current_pos];
			current_pos = next_pos(current_pos);
		}
		return count;
	}

	size_t available() {
		std::lock_guard<std::mutex> lock(mtx);
		if (tail >= head) {
			return tail - head;
		}
		return capacity - head + tail;
	}

	void clear() {
		std::lock_guard<std::mutex> lock(mtx);
		head = 0;
		tail = 0;
	}
};

static const size_t BUFFER_SIZE = 1024;
/**
* TcpClientApiPrivate
*/
class TcpClientApiPrivate{
public:
	TcpClientApiPrivate(TcpClientApi*p);
	~TcpClientApiPrivate();
	void initialize() throw(KernelException);
	void unInitialize();
	KernelApi::ControlState getControlState()const;
	void setCommunicationState(KernelApi::CommunicationState communicationState);
	KernelApi::CommunicationState getCommunicationState()const;
public:
	bool enable();
	bool disable();
	void setOffLine();
	void setLocalOnLine();
	void setRemoteOnLine();
	bool isCommunicationEnabled()const;
	void setAutoEnabled(bool enabled);
	void setInitMode(unsigned int model);

	void logMessage(const char* data, unsigned int len, const string& prefix);
	bool sendMessage(const char* data, unsigned int len);
	//bool receiveMessage(string &data);
	void setPort(unsigned int port);
	void setIp(string ip);
	bool reconnection();
	size_t readBuffer(char* data, size_t maxBytes);
	size_t peekBuffer(size_t skip, char* data, size_t maxBytes);
	size_t  getBufferSize() const;
	void clearBuffer();
	void receiveDataThread();

private:
	unsigned int port_=10;
	string ip_;
	std::shared_ptr<Socket> socket;
	std::shared_ptr<std::thread> receive_thread;
	RingBuffer buffer;  // 缓冲区大小为1024字节
	bool auto_enabled = false;
	unsigned int init_model = KernelApi::EQUIPMENT_OFF_LINE;
	KernelApi::ControlState controlState = KernelApi::EQUIPMENT_OFF_LINE;
	KernelApi::CommunicationState communicationState = KernelApi::NOT_COMMUNICATING;
	TcpClientApi* pParent = 0;
	bool enabled = false;
	bool reconnectionfail = false;
		
};


TcpClientApiPrivate::TcpClientApiPrivate(TcpClientApi* p)
	:pParent(p),
	buffer(BUFFER_SIZE),
	ip_("192.168.0.1"),
	port_(13001),
	socket(new Socket)
{


}

TcpClientApiPrivate::~TcpClientApiPrivate(){

}

void TcpClientApiPrivate::initialize() throw(KernelException){
	enable();
	switch (init_model){
	case 0:  //offline
		setOffLine();
		break;
	case 1:  //local online
		setLocalOnLine();
		break;
	case 2:  //remote online
		setRemoteOnLine();
		break;
	default:
		break;
	}
}

void TcpClientApiPrivate::unInitialize(){
	disable();
}

void TcpClientApiPrivate::setAutoEnabled(bool enabled){
	auto_enabled = enabled;
}

void TcpClientApiPrivate::setInitMode(unsigned int model){
	init_model = model;
}
void TcpClientApiPrivate::setPort(unsigned int port){
	port_ = port;
}
void TcpClientApiPrivate::setIp(string ip){
	ip_ = ip;
}

void TcpClientApiPrivate::clearBuffer(){
	buffer.clear();
}

size_t TcpClientApiPrivate::readBuffer(char* data, size_t maxBytes) {
	return buffer.read(data, maxBytes);
}

size_t TcpClientApiPrivate::peekBuffer(size_t skip, char* data, size_t maxBytes) {
	return buffer.peek(skip, data, maxBytes);
}
size_t  TcpClientApiPrivate::getBufferSize() const{
	return BUFFER_SIZE;
}

void TcpClientApiPrivate::setCommunicationState(KernelApi::CommunicationState communicationState){
	if (this->communicationState != communicationState){
		this->communicationState = communicationState;
		pParent->emitAttributeChanged(pParent);
	}
}

KernelApi::CommunicationState TcpClientApiPrivate::getCommunicationState()const{
	return communicationState;
}




KernelApi::ControlState TcpClientApiPrivate::getControlState()const{
	return controlState;
}


bool TcpClientApiPrivate::sendMessage(const char* data, unsigned int len){
	if (!socket->isConnected()){
		logInform("Socket", "send port not open");

		// 尝试重新连接
		if (!reconnection()) {
			logError(pParent->getName().c_str(), "Reconnection failed, cannot send message.");
			return false;
		}
		else {
			logInform(pParent->getName().c_str(), "Reconnected successfully, attempting to resend message...");
		}
		
	}
	size_t ret = socket->send((const char*)data, len);

	return ret == len;
}


void  TcpClientApiPrivate::logMessage(const char* data, unsigned int len, const std::string& prefix){
	std::stringstream bytes_ss;
	bytes_ss.fill('0');
	bytes_ss << std::hex;
	for (int i = 0; i < len; i++) {
		bytes_ss.width(2);
		bytes_ss << (data[i] & 0xFF) << " ";
	}
	logInform(pParent->getName().c_str(), "%s %s", prefix.c_str(), bytes_ss.str().c_str());
}

bool TcpClientApiPrivate::enable(){
	Socket::Param param;
	param.port = port_;
	param.ip = ip_;

	std::cout << "port:" << param.port << std::endl;
	std::cout << "ip:" << param.ip << std::endl;

	if (!socket->open(param)){
		setCommunicationState(KernelApi::CommunicationState::NOT_COMMUNICATING);
		//throw KernelSysException(__FILE__, KernelSysException::KR_MODULE_COMMUNICATION_ERROR, Poco::format("Can not open socket %s:%d", ip_,port_));
		logError(pParent->getName().c_str(), "Can not open socket %s:%d", ip_, port_);
		return false;
	}
	//if (!(socket->isConnected()))
	//{
	//	logError(pParent->getName().c_str(), "Can not connected socket %s:%d", ip_, port_);
	//}
	//
	enabled = true;
	socket->setMode(1);
	setCommunicationState(KernelApi::CommunicationState::COMMUNICATING);
	logInform(pParent->getName().c_str(), "Open socket");
	//开启线程
	receive_thread.reset(new std::thread(&TcpClientApiPrivate::receiveDataThread, this));
	return true;
}
void TcpClientApiPrivate::receiveDataThread(){
	while (enabled){
		//接收数据存到缓存
		std::vector<unsigned char> data(BUFFER_SIZE);
		int receivedBytes = socket->recv(data.data(), BUFFER_SIZE);
		if (receivedBytes > 0) {

			std::stringstream bytes_ss;
			bytes_ss.fill('0');
			bytes_ss << std::hex;
			for (int i = 0; i < receivedBytes; i++) {
				bytes_ss.width(2);
				bytes_ss << (data[i] & 0xFF) << " ";
			}
			logInform(pParent->getName().c_str(), "%s %s( %s)", "RS", 
				std::string(data.data(), data.data() + receivedBytes).c_str(), bytes_ss.str().c_str());
			buffer.write(reinterpret_cast<const char*>(data.data()), receivedBytes);
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));//delay 10 ms
	}
}
bool TcpClientApiPrivate::disable(){
	enabled = false;//提前关闭循环
	if (receive_thread) receive_thread->join();
	receive_thread = 0;
	if (socket)	socket->close();
	return true;
}

bool TcpClientApiPrivate::isCommunicationEnabled()const{
	return true;
}

void TcpClientApiPrivate::setOffLine(){
	if (controlState != KernelApi::EQUIPMENT_OFF_LINE &&  pParent->hookSetOffLine()){
		controlState = KernelApi::EQUIPMENT_OFF_LINE;
		pParent->emitAttributeChanged(pParent);
		logInform(pParent->getName().c_str(), "%s", "change control mode to offline.");
	}
}

void TcpClientApiPrivate::setLocalOnLine(){
	if (isCommunicationEnabled() && controlState != KernelApi::LOCAL_ON_LINE && pParent->hookSetLocalOnLine()){
		controlState = KernelApi::LOCAL_ON_LINE;
		pParent->emitAttributeChanged(pParent);
		logInform(pParent->getName().c_str(), "%s", "change control mode to local online.");
	}
}

void TcpClientApiPrivate::setRemoteOnLine(){
	if (isCommunicationEnabled() && controlState != KernelApi::REMOTE_ON_LINE && pParent->hookSetRemoteOnLine()){
		controlState = KernelApi::REMOTE_ON_LINE;
		pParent->emitAttributeChanged(pParent);
		logInform(pParent->getName().c_str(), "%s", "change control mode to remote online.");
	}
}

bool TcpClientApiPrivate::reconnection(){
	int reconnectionCount = 0;
	if (reconnectionfail){
		logError(pParent->getName().c_str(), "The reconnection has failed %d times", reconnectionCount);
		return false;
	}
	while (reconnectionCount < 3)
	{
		++reconnectionCount;
		unInitialize();
		initialize();
		if (communicationState == KernelApi::COMMUNICATING)
		{
			_sleep(100);
			return true;
		}
		logError(pParent->getName().c_str(), "The reconnection has failed %d times", reconnectionCount);
		_sleep(1000);
		
	}
	reconnectionfail = true;
	return false;
}

TcpClientApi::TcpClientApi(IKernel*  kernel) : KernelAbstractApi(kernel), d(new TcpClientApiPrivate(this)){

}

KernelApi::ControlState TcpClientApi::getControlState()const{
	return d->getControlState();
}

void TcpClientApi::setCommunicationState(KernelApi::CommunicationState communicationState){
	d->setCommunicationState(communicationState);
}
bool TcpClientApi::sendMessage(const char* data, unsigned int len){
	return d->sendMessage(data, len);
}


void TcpClientApi::clearBuffer(){
	d->clearBuffer();
}
size_t  TcpClientApi::readBuffer(char* data, size_t btr){
	return d->readBuffer(data, btr);
}
size_t  TcpClientApi::peekBuffer(size_t skip_count, char* data, size_t btp){
	return d->peekBuffer(skip_count, data, btp);
}
size_t  TcpClientApi::getBufferSize() const{
	return d->getBufferSize();
}

void TcpClientApi::initialize() throw(KernelException){
	d->initialize();
}

void TcpClientApi::unInitialize(){
	d->unInitialize();
}

/**
* process the subsystem
*/
void TcpClientApi::process(){


}

/**
* configure the subsystem
*/
void TcpClientApi::configure(const std::shared_ptr<KernelConfiguration> & config) throw(KernelException){
	d->setPort(config->getUInt("port", 8000));
	d->setIp(config->getString("ip", "192.168.127.253"));

	//auto enabled
	d->setAutoEnabled(config->getBool("auto_enable", true));
	//auto model
	d->setInitMode(config->getUInt("auto_model", 0));
}

void TcpClientApi::logMessage(const char* data, unsigned int len, const std::string& prefix){
	d->logMessage(data, len, prefix);
}

KernelApi::CommunicationState TcpClientApi::getCommunicationState()const{
	return d->getCommunicationState();
}

bool TcpClientApi::enable(){
	return d->enable();
}

bool TcpClientApi::disable(){
	return d->disable();
}
void TcpClientApi::setOffLine(){
	d->setOffLine();
}


void TcpClientApi::setLocalOnLine(){
	d->setLocalOnLine();
}

void TcpClientApi::setRemoteOnLine(){
	d->setRemoteOnLine();
}
bool TcpClientApi::isCommunicationEnabled()const{
	return d->isCommunicationEnabled();
}

KERNEL_NS_END
