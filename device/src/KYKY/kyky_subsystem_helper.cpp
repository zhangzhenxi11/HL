
/**
* @file            kyky_subsystem_helper.h
* @brief           helper for kyky protocol subsystem
* @author		   kai
*/

// Library: Fortrend
// Package: Subsystem/KYKY


#include "Kernel/kernel_log.h"
#include "Kernel/kernel_configure.h"
#include "Poco/String.h"
#include "Poco/Format.h"
#include "Poco/StringTokenizer.h"
#include "Poco/NumberParser.h"

#include <list>
#include <mutex>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <thread>

#include "KYKY/kyky_subsystem_helper.h" 
#include "modbus_master.h"

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

KERNEL_NS_BEGIN


/**
* KYKYSubSystemHelperPrivate
*/
class KYKYSubSystemHelperPrivate{
public:
	KYKYSubSystemHelperPrivate(KYKYSubSystemHelper* p, const std::string& name);
	//IO config
	struct IOConfig{
		std::vector<std::string> input_names;
		std::string input_address;
		std::vector<std::string> output_names;
		std::vector<std::string> output_addresses;
	} ioConfig;
	bool open()throw(KernelException);
	void close()throw(KernelException);

	void configure(const std::shared_ptr<KernelConfiguration> & config);
	bool readData(int addr, uint16_t* data)throw(KernelException);
	bool writeData(int addr, uint16_t data)throw(KernelException);
	std::shared_ptr<KYKYSubSystemHelper::DefinedError> getErrorCode(const int code_id);


public:
	std::shared_ptr<ModbusMaster> modbus_master;
	int server_id;
	std::string com_port;
	int baud_rate;
	std::string name;

	KYKYSubSystemHelper* p;
	mutable std::mutex mutex;

	std::list<std::shared_ptr<KYKYSubSystemHelper::DefinedError> > definedErrors;  //defined error from hardware

private:
	bool isConnected = false;
};


KYKYSubSystemHelperPrivate::KYKYSubSystemHelperPrivate(KYKYSubSystemHelper*p, const std::string& name)
	:p(p)
	, name(name)
{
	modbus_master = std::make_shared<ModbusMaster>();
}


bool KYKYSubSystemHelperPrivate::open()throw(KernelException){
	bool res = true;
	if (modbus_master->init(server_id, com_port, baud_rate)){
		res = true;
		isConnected = true;
	}
	else
	{
		throw KernelSysException(__FILE__, KernelSysException::KR_MODULE_COMMUNICATION_ERROR, Poco::format("无法打开端口: %s", com_port));
	}
	return res;
}


void KYKYSubSystemHelperPrivate::close()throw(KernelException){
	if (isConnected)
	{
		modbus_master->close();
	}
	
}

void KYKYSubSystemHelperPrivate::configure(const std::shared_ptr<KernelConfiguration> & config){

	com_port = config->getString("com", "COM3");
	baud_rate = config->getInt("baud");
	server_id = config->getInt("serverId");
	//load defined alarms
	if (config->has("ErrorTxt"))
	{
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
			std::shared_ptr<KYKYSubSystemHelper::DefinedError> errorObj(new KYKYSubSystemHelper::DefinedError);
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

std::shared_ptr<KYKYSubSystemHelper::DefinedError> KYKYSubSystemHelperPrivate::getErrorCode(const int code_id)
{
	std::shared_ptr<KYKYSubSystemHelper::DefinedError> res = std::make_shared<KYKYSubSystemHelper::DefinedError>(KYKYSubSystemHelper::DefinedError{ 1, code_id, "", false });;
	for (const auto err : definedErrors)
	{
		if (code_id == err->code)
		{
			res = err;
			return res;
		}
	}
	res->type = 1;
	res->code = code_id;
	res->message = "Error code definition not found in hardware. ";
	res->needReset = true;
	return res;
}


bool KYKYSubSystemHelperPrivate::readData(int addr, uint16_t* data)throw(KernelException)
{
	//command
	size_t size = sizeof(data) / sizeof(data[0]);
	
	if (modbus_master->readRegister(addr, data)){
		std::string message = "";
		for (size_t i = 0; i < size; ++i) {
			message.append(std::to_string(data[i]));
		}
		logInform1(name.c_str(), "地址: %d, 数据: %s 读取成功", addr, message.c_str());
		return true;
	};
	logError(name.c_str(), "地址: %d 读取失败.", addr);
	return false;
	
	
}

bool KYKYSubSystemHelperPrivate::writeData(int addr, uint16_t data)throw(KernelException)
{
	if (modbus_master->writeRegister(addr, data))
	{
		logInform1(name.c_str(), "地址: %d, 数据: %d 写入成功", addr, data);
		return true;
	}
	logError(name.c_str(), "地址：%d ,数据：%d 写入失败.", addr, data);
	return false;
}


/**
* KYKYSubSystemHelper
*/
KYKYSubSystemHelper::KYKYSubSystemHelper(const std::string& name)
	: d(new KYKYSubSystemHelperPrivate(this, name)){

}


/**
* initialize
*/
bool KYKYSubSystemHelper::enableProtocol(){
	return d->open();
}


/**
* unInitialize
*/
void KYKYSubSystemHelper::disableProtocol(){
	d->close();
}

bool KYKYSubSystemHelper::readData(int addr, uint16_t* data) throw(KernelException){
	return d->readData(addr, data);
}

bool KYKYSubSystemHelper::writeData(int addr, uint16_t data) throw(KernelException){
	return d->writeData(addr, data);
}

void KYKYSubSystemHelper::configKYKY(const std::shared_ptr<KernelConfiguration> & config){
	d->configure(config);
}


KERNEL_NS_END