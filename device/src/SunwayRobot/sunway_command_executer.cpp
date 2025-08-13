/**
* @file            sunway_command_executer.h
* @brief           sunway command executer for fortrend
* @author			kai
*/

// Library: Fortrend
// Package: Subsystem/SunwayRobot

#include "SunwayRobot/sunway_command_executer.h"
#include "SunwayRobot/sunway_subsystem_helper.h"
#include "Poco/Format.h"

#include <thread>
#include <sstream>
#include <chrono>
#include <iostream>
#include <string>
KERNEL_NS_BEGIN

/**
* SunwayCommandExecuterPrivate
*/
class SunwayCommandExecuterPrivate{
public:
	SunwaySubSystemHelper* helper;
};

/**
* SunwayCommandExecuter
*/
SunwayCommandExecuter::SunwayCommandExecuter(SunwaySubSystemHelper* helper)
	:d(new SunwayCommandExecuterPrivate){
	d->helper = helper;
}


/**
* return true if success else false.
*/
void SunwayCommandExecuter::onStop() throw(KernelException){
	throw KernelSysException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, "can not stop.");
}


/**
* Send command to Sunway (auto add 0x0D)
*/
bool SunwayCommandExecuter::sendRequest(const std::string& command) throw(KernelException){

	return d->helper->sendRequest(command);
}

/**
* recv string command from Sunway (auto add 0x0D)
*/
std::string SunwayCommandExecuter::recvResponse(unsigned int timeout_ms) throw(KernelException){
	return d->helper->recvResponse(timeout_ms);

}

void SunwayCommandExecuter::recvResponse2(unsigned int timeout_ms) throw(KernelException)
{
	d->helper->recvResponse2(timeout_ms);
}

/**
* recv string command from Sunway (auto add 0x0D)
*/
std::string SunwayCommandExecuter::recvResponseRDY(unsigned int timeout_ms) throw(KernelException){
	return d->helper->recvResponseRDY(timeout_ms);

}

bool SunwayCommandExecuter::getBusyState(){
	return d->helper->getBusyState();
}

std::string SunwayCommandExecuter::recvResponseRobotMessage(unsigned int timeout_ms) throw(KernelException)
{
	return d->helper->recvResponseRobotMessage(timeout_ms);
}

void SunwayCommandExecuter::clearRobotMessage() throw(KernelException)
{
	d->helper->clearRobotMessage();
}

std::string SunwayCommandExecuter::sendCommand(const std::string& command, unsigned int timeout_ms) throw(KernelException)
{
	return  d->helper->sendCommand(command, timeout_ms);
}

std::shared_ptr<SunwaySubSystemHelper::DefinedError> SunwayCommandExecuter::getErrorCode(const int code_id){
	return d->helper->getErrorCode(code_id);
}

std::shared_ptr<SunwaySubSystemHelper::DefinedError> SunwayCommandExecuter::getErrorCode(const int type_id, const int code_id)
{
	return d->helper->getErrorCode(type_id, code_id);
}

bool SunwayCommandExecuter::handleErrorCode(const std::string &commandStr, const std::string errorStr, int& type, int& code)
{
	size_t found = commandStr.find(errorStr.c_str());
	if (found == std::string::npos) {
		return false; // 没有 ERR 标记，直接返回
	}

	// 查找第一个 '/' 和第二个 '/'
	size_t firstSlash = commandStr.find('/', found); // 从 "ERR" 后开始找第一个 '/'
	size_t secondSlash = commandStr.find('/', firstSlash + 1);

	// 查找分号 ';' 的位置（code 的结束位置）
	size_t semicolonPos = commandStr.find(';', secondSlash + 1);

	// 检查关键位置是否有效
	if (firstSlash == std::string::npos ||
		secondSlash == std::string::npos ||
		semicolonPos == std::string::npos) {
		std::cerr << "无效的错误格式" << std::endl;
		return false;
	}

	// 提取 type 和 code 的字符串
	std::string str_type = commandStr.substr(
		firstSlash + 1,
		secondSlash - (firstSlash + 1)
	);
	std::string str_code = commandStr.substr(
		secondSlash + 1,
		semicolonPos - (secondSlash + 1)
	);

	// 转换为整数
	try {
		type = std::stoi(str_type);
		code = std::stoi(str_code);
		std::cout << "type: " << type << ", code: " << code << std::endl;
		return true;
	}
	catch (const std::invalid_argument& e) {
		std::cerr << "转换失败: 非数字字符" << std::endl;
		return false;
	}
	catch (const std::out_of_range& e) {
		std::cerr << "转换失败: 数字超出范围" << std::endl;
		return false;
	}
	return false;
}

bool SunwayCommandExecuter::parseResponse(const std::string& response, std::string& prefix, std::string& command, std::vector<std::string>& parameters)
{
	size_t colonPos = response.find(':');
	if (colonPos == std::string::npos) {
		std::cerr << "解析失败: 未找到冒号分隔符" << std::endl;
		return false;
	}
	prefix = response.substr(0, colonPos);

	size_t semicolonPos = response.find(';', colonPos);
	if (semicolonPos == std::string::npos) {
		std::cerr << "解析失败: 未找到结束分号" << std::endl;
		return false;
	}

	std::string content = response.substr(
		colonPos + 1,
		semicolonPos - (colonPos + 1)
	);

	size_t firstSlash = content.find('/');
	if (firstSlash == std::string::npos) {
		// 没有参数的情况
		command = content;
		return true;
	}

	command = content.substr(0, firstSlash);
	std::string paramsStr = content.substr(firstSlash + 1);

	// 解析参数列表
	size_t start = 0;
	size_t end = 0;
	while ((end = paramsStr.find('/', start)) != std::string::npos) {
		parameters.push_back(paramsStr.substr(start, end - start));
		start = end + 1;
	}
	// 添加最后一个参数
	if (start < paramsStr.length()) {
		parameters.push_back(paramsStr.substr(start));
	}

	return true;
}

KERNEL_NS_END
