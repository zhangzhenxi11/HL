// Library: LibDevicePlugin
// Package: Api/ASCII
//
// ascii api of kernel api
//
// author xielonghua
//


#include "efem_ascii_api.h"
#include "Kernel/kernel_log.h"
#include "Kernel/kernel_exception.h"
#include <WinSock2.h>
#include <map>
#include <sstream>

#include "Poco/Format.h"
#include "Poco/StringTokenizer.h"

#include <mutex>
#include <condition_variable>

#define MAX_BUFFER_SIZE 100

#undef ERROR

namespace FC{

static std::map<std::string, EFEMAsciiApi::Type> typeMap = {
	{ "MOV", EFEMAsciiApi::MOV },
	{ "GET", EFEMAsciiApi::GET },
	{ "SET", EFEMAsciiApi::SET },
	{ "INF", EFEMAsciiApi::INF },
	{ "ABS", EFEMAsciiApi::ABS },
	{ "EVT", EFEMAsciiApi::EVT },
	{ "ACK", EFEMAsciiApi::ACK },
	{ "NAK", EFEMAsciiApi::NAK },
};


static std::map<std::string, EFEMAsciiApi::Base> baseMap = {
	{ "READY", EFEMAsciiApi::READY },
	{ "INIT", EFEMAsciiApi::INIT },
	{ "LOCK", EFEMAsciiApi::LOCK },
	{ "UNLOCK", EFEMAsciiApi::UNLOCK },
	{ "OPEN", EFEMAsciiApi::OPEN },
	{ "CLOSE", EFEMAsciiApi::CLOSE },
	{ "WAFSH", EFEMAsciiApi::WAFSH },
	{ "MAPDT", EFEMAsciiApi::MAPDT },
	{ "GOTO", EFEMAsciiApi::GOTO },
	{ "LOAD", EFEMAsciiApi::LOAD },
	{ "UNLOAD", EFEMAsciiApi::UNLOAD },
	{ "ALIGN", EFEMAsciiApi::ALIGN },
	{ "HOLD", EFEMAsciiApi::HOLD },
	{ "RESTR", EFEMAsciiApi::RESTR },
	{ "ABORT", EFEMAsciiApi::ABORT },
	{ "MODE", EFEMAsciiApi::MODE },
	{ "SIGOUT", EFEMAsciiApi::SIGOUT },
	{ "STATE", EFEMAsciiApi::STATE },
	{ "SIGSTAT", EFEMAsciiApi::SIGSTAT },
	{ "TOWER", EFEMAsciiApi::TOWER },
	{ "CSTID", EFEMAsciiApi::CSTID },
	{ "SPEED", EFEMAsciiApi::SPEED },
	{ "CONTROLMODE", EFEMAsciiApi::CONTROLMODE },
	{ "HEARTBEAT", EFEMAsciiApi::HEARTBEAT },
	{ "TOOLSLOT", EFEMAsciiApi::TOOLSLOT },
	{ "CLAMP", EFEMAsciiApi::CLAMP },
	{ "TRIGGER", EFEMAsciiApi::TRIGGER },
	{ "ALRAM", EFEMAsciiApi::ALARM },
	{ "POD", EFEMAsciiApi::POD },
	{ "DOORSTAT", EFEMAsciiApi::DOORSTAT }
};

/**
* EFEMAsciiApiPrivate
*/
class EFEMAsciiApiPrivate{
public:
	EFEMAsciiApi::Type str2Type(const std::string& str);
	EFEMAsciiApi::Base str2Base(const std::string& str);
	std::string type2Str(EFEMAsciiApi::Type type);
	std::string  base2str(EFEMAsciiApi::Base);



public:
	std::chrono::system_clock::time_point last_send_ready_time = std::chrono::system_clock::now();
	char buffer[MAX_BUFFER_SIZE];

	EFEMAsciiApi::State state;
	std::mutex mtx;
	std::condition_variable cv;
};

EFEMAsciiApi::Type EFEMAsciiApiPrivate::str2Type(const std::string& str){
	auto it = typeMap.find(str);
	if (it == typeMap.end()) return EFEMAsciiApi::TYPE_UNKNOWN;
	return it->second;
}


EFEMAsciiApi::Base EFEMAsciiApiPrivate::str2Base(const std::string& str){
	auto it = baseMap.find(str);
	if (it == baseMap.end()) return EFEMAsciiApi::BASE_UNKNOWN;
	return it->second;
}

std::string EFEMAsciiApiPrivate::type2Str(EFEMAsciiApi::Type type){
	for (auto& it : typeMap){
		if (it.second == type){
			return it.first;
		}
	}
	return "";
}

std::string  EFEMAsciiApiPrivate::base2str(EFEMAsciiApi::Base base){
	for (auto& it : baseMap){
		if (it.second == base){
			return it.first;
		}
	}
	return "";
}



/**
 * EFEMAsciiApi
 */
EFEMAsciiApi::EFEMAsciiApi(IKernel*  kernel)
	//:TcpClientApi(kernel)
	:TcpClientApi(kernel)
, d(new EFEMAsciiApiPrivate){

}

void   EFEMAsciiApi::logMessage(const char* data, unsigned int len, const std::string& prefix){
	std::stringstream bytes_ss;
	bytes_ss.fill('0');
	bytes_ss << std::hex;
	for (int i = 0; i < len; i++) {
		bytes_ss.width(2);
		bytes_ss << (data[i] & 0xFF) << " ";
	}
	logInform(getName().c_str(), "%s %s( %s)", prefix.c_str(), std::string(data, data + len).c_str(), bytes_ss.str().c_str());
}


bool EFEMAsciiApi::sendMessage(const char* data, unsigned int len){
	char *buf = new char[len + 1];
	memcpy(buf, data, len);
	buf[len] = 0x0D;
	bool ret = TcpClientApi::sendMessage(buf, len + 1);
	delete[] buf;
	if (ret){
		logMessage(data, len, "S");
	}
	return ret;
}


// response ACK for command
bool EFEMAsciiApi::sendACK(const std::shared_ptr<Message>& message){
	std::string str = Poco::format("ACK:%s",  d->base2str(message->base));
	for (auto& item : message->paramers){
		str = str + "/";
		str = str + item;
	}
	str.push_back(';');
	return sendMessage(str.data(), str.size());
}

// response NAK for command
bool EFEMAsciiApi::sendNAK(const std::shared_ptr<Message>& message, int code){
	std::string msg;
	for (auto& item : message->paramers){
		msg = msg + "/";
		msg = msg + item;
	}

	std::string str = Poco::format("NAK:%s|%d;", msg, code);
	return sendMessage(str.data(), str.size());
}


// response INF for command
bool EFEMAsciiApi::sendINF(const std::shared_ptr<Message>& message){
	std::string str = Poco::format("INF:%s", d->base2str(message->base));
	for (auto& item : message->paramers){
		str = str + "/";
		str = str + item;
	}
	str.push_back(';');
	return sendMessage(str.data(), str.size());
}

// response ABS for command
bool EFEMAsciiApi::sendABS(const std::shared_ptr<Message>& message, const std::string&source, int type, int code){
	std::string str = Poco::format("ABS:%s", d->base2str(message->base));
	for (auto& item : message->paramers){
		str = str + "/";
		str = str + item;
	}

	str = Poco::format("%s|ERROR/%s/%d/%d;", str, source, type,code);
	return sendMessage(str.data(), str.size());
}

// response EVT for command
bool EFEMAsciiApi::sendEVT(const std::shared_ptr<Message>& message){
	std::string str = Poco::format("EVT:%s", d->base2str(message->base));
	for (auto& item : message->paramers){
		str = str + "/";
		str = str + item;
	}

	str = Poco::format("%s;", str);
	return sendMessage(str.data(), str.size());
}

void EFEMAsciiApi::process(){
	//if (!isCommunicationEnabled()) return;
	//auto time = std::chrono::system_clock::now();
	//int pass = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - d->last_send_ready_time).count();
	////send communication request (2s timeout)
	//if ((pass > 2) && (CommunicationState::NOT_COMMUNICATING== getCommunicationState()) && isOnline()){
	//	std::shared_ptr<Message> message(new Message);
	//	message->base = READY;
	//	message->paramers.push_back("COMM");
	//	if (sendINF(message)){
	//		d->last_send_ready_time = time;
	//	}
	//}
	memset(d->buffer, 0, sizeof(d->buffer)); // 清零操作
	//handle data
	for (int i = 0; i < getBufferSize();i++){
		peekBuffer(i, d->buffer, 1);//从缓存读取数据
		if (d->buffer[0] == 0x0d){
			size_t size = min(MAX_BUFFER_SIZE, i + 1);
			readBuffer(d->buffer, size);  //read from buffer
			logMessage(d->buffer, size, "R");
			onDataRecv(d->buffer, size);  //handle command
			break;
		}
	}
}


void EFEMAsciiApi::onConnect(){
	//clearBuffer();
	//setRemoteOnLine();
}

void EFEMAsciiApi::onDisConnect(){
	//setLocalOnLine();
	logInform(getName().c_str(),"onDisConnect");
}



void EFEMAsciiApi::onDataRecv(const char* data, unsigned int len){
	//find => :
	std::string message(data, data + len);
	
	Poco::StringTokenizer messages(message, ";\r", Poco::StringTokenizer::TOK_IGNORE_EMPTY);
	// Process each message separately
	for (int i = 0; i < messages.count(); ++i) {
		std::string singleMessage = messages[i];
		message_data = singleMessage;
		logInform(getName().c_str(), "onDataRecv singleMessage=%s", singleMessage.c_str());
		if (singleMessage.size()>3){
			processSingleMessage(singleMessage);
		}
	}	
	
}


std::string EFEMAsciiApi::getData()
{
	logInform(getName().c_str(), "onDataRecv message_data=%s", message_data.c_str());

	return message_data;
}


void EFEMAsciiApi::processSingleMessage(const std::string& message) {
	//logInform(getName().c_str(), "1 processSingleMessage=%s", message.c_str());
	int pos1 = message.find(':');
	int pos2 = message.find(';');
	if (pos1 < 0 || pos1 >= message.size()){
		return;
	}
	//logInform(getName().c_str(), "2 processSingleMessage=%s %s", message.c_str(), message.substr(0, pos1));

	std::string tp = message.substr(0, pos1);
	char byteToRemove = '\x0A';
	tp.erase(tp.begin(), std::find_if(tp.begin(), tp.end(), [byteToRemove](char ch) {
		return ch != byteToRemove;
	}));

	//type
	Type type = d->str2Type(tp);
	
	//only INF, ABS, EVT ACK NAK type to handle it.
	if (type != INF && type != ABS && type != EVT && type != ACK&& type != NAK){
		return;
	}
	//logInform(getName().c_str(), "3 processSingleMessage=%s", message.c_str());
	//body
	//std::string body = message.substr(pos1 + 1, pos2 - pos1 - 1);
	std::string body = message.substr(pos1 + 1);
	logInform(getName().c_str(), "onDataRecv body=%s", body.c_str());
	//get base & paramers
	Poco::StringTokenizer tokenizer(body, "/", 2);
	if (tokenizer.count() == 0){
		return;
	}
	Base base = d->str2Base(tokenizer[0]);
	//paramers
	std::vector<std::string> paramers;
	for (int i = 1; i < tokenizer.count();i++){
		paramers.push_back(tokenizer[i]);
	}
	//create command
	std::shared_ptr<EFEMAsciiApi::Command> command(new EFEMAsciiApi::Command);
	std::shared_ptr<EFEMAsciiApi::Message> msg(new EFEMAsciiApi::Message);
	msg->base = base;
	msg->paramers = paramers;

	command->type = type;
	command->message = msg;



	if (base == BASE_UNKNOWN || type == TYPE_UNKNOWN){
		logError(getName().c_str(), "BASE_UNKNOWN or TYPE_UNKNOWN");
		sendNAK(command->message, KernelSysException::ErrCode::KR_COMMON_COMMAND_NO_SUPPORT);
		return;
	}

	if (base == READY && type == INF && msg->paramers.size() == 1 && msg->paramers.at(0) == std::string("COMM")){//收到Ready消息回复
		sendACK(command->message);
		logInform(getName().c_str(), "sendACK processSingleMessage=%s", message.c_str());
		setCommunicationState(COMMUNICATING);
		//setRemoteOnLine();
		update();
		return;
	}
	logInform(getName().c_str(), "10 processSingleMessage=%s", message.c_str());
	//only remote can handle the message
	/*if (COMMUNICATING == getCommunicationState()){*/
		handle(command);
	//}
}
void EFEMAsciiApi::processEFEMessage(std::string& message)
{
	int pos1 = message.find(':');
	int pos2 = message.find(';');
	if (pos1 < 0 || pos1 >= message.size()) {
		return;
	}
	std::string body = message.substr(pos1 + 1);
	logInform(getName().c_str(), "onDataRecv body=%s", body.c_str());

	if (body == "INIT/ALL")
	{
		return;
	}
}
}
