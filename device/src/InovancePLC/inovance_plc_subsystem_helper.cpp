/**
* @file            inovance_plc_subsystem_helper.h
* @brief           helper for inovance plc protocol subsystem
* @author		   kai
*/

// Library: Fortrend
// Package: Subsystem/InovancePlc


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
#include <cctype>



#include "InovancePLCExternal/Include.h"
#include "InovancePLCExternal/Hc_Modbus_Api.h"
//#pragma comment(lib, "F:\\Projects\\VTM\\IBDTEC\\src\\VTM-dev\\device\\InovancePLCExternal\\StandardModbusApi.lib")

#include "InovancePLC/inovance_plc_subsystem_helper.h" 

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif


KERNEL_NS_BEGIN


/**
* InovancePlcSubSystemHelperPrivate
*/
class InovancePlcSubSystemHelperPrivate{
public:
	InovancePlcSubSystemHelperPrivate(InovancePlcSubSystemHelper* p, const std::string& name);
	//IO config
	struct IOConfig{
		std::vector<std::string> input_names;
		std::string input_address;
		std::vector<std::string> output_names;
		std::vector<std::string> output_addresses;
	} ioConfig;
	struct PLCAddress{
		SoftElemType type;
		int address = 0;

	};
	bool open()throw(KernelException);
	void close()throw(KernelException);

	bool setTimeout(const uint32_t milsecTime);
	PLCAddress getSoftElemTypeAndAddress(const std::string address);
	int convertStringToInt(const std::string str_int);
	void configure(const std::shared_ptr<KernelConfiguration> & config);
	std::shared_ptr<InovancePlcSubSystemHelper::DefinedError> getErrorCode(const int code_type, const int code_id);

public:
	bool readBit(const std::string address, bool &value);
	bool readBits(const std::string address, const uint16_t length, bool *value);
	bool readShort(const std::string address, int16_t &value);
	bool readShorts(const std::string address, const uint16_t length, int16_t *value);
	bool readUnsignedShort(const std::string address, uint16_t &value);
	bool readUnsignedShorts(const std::string address, const uint16_t length, uint16_t *value);
	bool readInt(const std::string address, int32_t &value);
	bool readInts(const std::string address, const uint16_t length, int32_t *value);
	bool readUnsignedInt(const std::string address, uint32_t &value);
	bool readUnsignedInts(const std::string address, const uint16_t length, uint32_t *value);
	bool readFloat(const std::string address, float &value);
	bool readFloats(const std::string address, const uint16_t length, float *value);
	bool readDouble(const std::string address, double &value);
	bool readDoubles(const std::string address, const uint16_t length, double *value);
	bool readString(const std::string address, const uint16_t length, std::string &value);

	bool writeBit(const std::string address, const bool value);
	bool writeBits(const std::string address, const uint16_t length, const bool *values);
	bool writeShort(const std::string address, const int16_t value);
	bool writeShorts(const std::string address, const uint16_t length, const int16_t *values);
	bool writeUnsignedShort(const std::string address, const uint16_t value);
	bool writeUnsignedShorts(const std::string address, const uint16_t length, const uint16_t *values);
	bool writeInt(const std::string address, const int32_t value);
	bool writeInts(const std::string address, const uint16_t length, const int32_t *values);
	bool writeUnsignedInt(const std::string address, const uint32_t value);
	bool writeUnsignedInts(const std::string address, const uint16_t length, const uint32_t *values);
	bool writeFloat(const std::string address, const float value);
	bool writeFloats(const std::string address, const uint16_t length, const float *values);
	bool writeDouble(const std::string address, const double value);
	bool writeDoubles(const std::string address, const uint16_t length, const double *values);
	bool writeString(const std::string address, const std::string value);
public:
	uint16_t port = 502;
	std::string ip_address = "192.168.1.88";
	uint32_t plc_timeout_milsec = 2000;
	InovancePlcSubSystemHelper::PLCType plc_type = InovancePlcSubSystemHelper::PLCType::AM600;
	int plc_net_id = 0;
	std::string name;
	const int plc_operation_sucess = 1;
	InovancePlcSubSystemHelper* p;
	mutable std::mutex mutex;
	int retry_count = 0;

	std::list<std::shared_ptr<InovancePlcSubSystemHelper::DefinedError> > definedErrors;  //defined error from hardware

private:
	bool initialize();
	std::vector<std::string> splitData(const std::string& data, char delimiter);
private:
	bool isConnected = false;
};


InovancePlcSubSystemHelperPrivate::InovancePlcSubSystemHelperPrivate(InovancePlcSubSystemHelper*p, const std::string& name)
	:p(p)
	, name(name)
{

}


bool InovancePlcSubSystemHelperPrivate::open(){
	isConnected = initialize();
	if (!isConnected)
	{
		throw KernelSysException(__FILE__, KernelSysException::KR_MODULE_COMMUNICATION_ERROR, Poco::format("无法连接PLC服务器: %d", port));
	}
	return isConnected;
}


void InovancePlcSubSystemHelperPrivate::close(){
	isConnected = false;
	Exit_ETH(plc_net_id);
	return;
}

bool InovancePlcSubSystemHelperPrivate::initialize()
{
	logInform1(name.c_str(), "read configure: port = %d, ip_address = %s, plc_net_id = %d", port, ip_address.c_str(), plc_net_id);
	char * ptr_ip = const_cast<char*>(ip_address.c_str());
	return Init_ETH_String(ptr_ip, plc_net_id, port);
}


bool InovancePlcSubSystemHelperPrivate::setTimeout(const uint32_t milsecTime)
{
	return Set_ETH_TimeOut(milsecTime, plc_net_id);
}


InovancePlcSubSystemHelperPrivate::PLCAddress InovancePlcSubSystemHelperPrivate::getSoftElemTypeAndAddress(const std::string address){
	InovancePlcSubSystemHelperPrivate::PLCAddress addr;
	string type;
	// 提取所有字母
	std::copy_if(address.begin(), address.end(), std::back_inserter(type), [](char c) { return std::isalpha(c); });
	//得到除类型外的地址
	size_t pos = address.find(type);
	string number_address = address;
	number_address.erase(pos, type.length());
	//字母转换为大写
	std::transform(type.begin(), type.end(), type.begin(), [](unsigned char c) { return std::toupper(c); });
	if (plc_type == InovancePlcSubSystemHelper::PLCType::AM600)
	{
		if (number_address.find(".") != std::string::npos) {
			/*std::stringstream ss(number_address);
			std::string first_part, second_part;

			std::getline(ss, first_part, '.');
			std::getline(ss, second_part);

			int nAddressBuff = stringToInt(first_part);
			int nOffsetBuff = stringToInt(second_part);
			if (nAddressBuff >=0 && nOffsetBuff >=0 )
			{
			addr.address = nAddressBuff + nOffsetBuff;
			}
			else
			{
			addr.address = -1;
			}
			*/
			number_address.erase(std::remove(number_address.begin(), number_address.end(), '.'), number_address.end());

		}
		addr.address = convertStringToInt(number_address);
		if (type == "QX")
		{
			addr.address *= 10;
			addr.type = SoftElemType::ELEM_QX;
		}
		else if (type == "MW")
		{
			addr.type = SoftElemType::ELEM_MW;

		}
		else if (type == "X" || type == "IX")
		{
			addr.type = SoftElemType::ELEM_X;
		}
		else if (type == "Y" || type == "Q")
		{
			addr.type = SoftElemType::ELEM_Y;
		}
		else
		{
			addr.address = -1;//获取类型失败

		}


	}
	else if (plc_type == InovancePlcSubSystemHelper::PLCType::H3U)
	{
		if (type == "Y")
		{
			addr.type = SoftElemType::REGI_H3U_Y;
		}
		else if (type == "X")
		{
			addr.type = SoftElemType::REGI_H3U_X;
		}
		else if (type == "S")
		{
			addr.type = SoftElemType::REGI_H3U_S;
		}
		else if (type == "M")
		{
			addr.type = SoftElemType::REGI_H3U_M;
		}
		else if (type == "TB")
		{
			addr.type = SoftElemType::REGI_H3U_TB;
		}
		else if (type == "TW")
		{
			addr.type = SoftElemType::REGI_H3U_TW;
		}
		else if (type == "CB")
		{
			addr.type = SoftElemType::REGI_H3U_CB;
		}
		else if (type == "CW")
		{
			addr.type = SoftElemType::REGI_H3U_CW;
		}
		else if (type == "DW")
		{
			addr.type = SoftElemType::REGI_H3U_DW;
		}
		else if (type == "CW2")//异常，待解决
		{
			addr.type = SoftElemType::REGI_H3U_CW2;
		}
		else if (type == "SM")
		{
			addr.type = SoftElemType::REGI_H3U_SM;
		}
		else if (type == "SD")
		{
			addr.type = SoftElemType::REGI_H3U_SD;
		}
		else if (type == "R")
		{
			addr.type = SoftElemType::REGI_H3U_R;
		}
		else
		{
			addr.address = -1;//获取类型失败

		}
		addr.address = convertStringToInt(number_address);
	}
	else
	{
		if (type == "Y")
		{
			addr.type = SoftElemType::REGI_H5U_Y;
		}
		else if (type == "X")
		{
			addr.type = SoftElemType::REGI_H5U_X;
		}
		else if (type == "S")
		{
			addr.type = SoftElemType::REGI_H5U_S;
		}
		else if (type == "M")
		{
			addr.type = SoftElemType::REGI_H5U_M;
		}
		else if (type == "B")
		{
			addr.type = SoftElemType::REGI_H5U_B;
		}
		else if (type == "D")
		{
			addr.type = SoftElemType::REGI_H5U_D;
		}
		else if (type == "R")
		{
			addr.type = SoftElemType::REGI_H5U_R;
		}
		else
		{
			addr.address = -1;//获取类型失败

		}
		addr.address = convertStringToInt(number_address);
	}
	return addr;
}

int InovancePlcSubSystemHelperPrivate::convertStringToInt(const std::string str_int)
{
	int num = -1;
	try {
		num = std::stoi(str_int);
	}
	catch (const std::invalid_argument& e) {
		num = -1;
	}
	return num;
}

bool InovancePlcSubSystemHelperPrivate::readBit(const std::string address, bool &value)
{
	PLCAddress addr = getSoftElemTypeAndAddress(address);
	if (addr.address < 0)
	{
		return false;
	}
	BYTE resRead[1] = { 0 };
	int operation = 0;
	if (plc_type == InovancePlcSubSystemHelper::PLCType::AM600)
	{
		operation = Am600_Read_Soft_Elem(addr.type, addr.address, 1, resRead, plc_net_id);

	}
	else if (plc_type == InovancePlcSubSystemHelper::PLCType::H3U)
	{

		operation = H3u_Read_Soft_Elem(addr.type, addr.address, 1, resRead, plc_net_id);
	}
	else
	{
		operation = H5u_Read_Device_Block(addr.type, addr.address, 1, resRead, plc_net_id);
	}
	value = resRead[0] == 1;
	bool res = (operation == plc_operation_sucess);
	if (res == false)
	{
		int retry_count = 0;
		while (!res&&isConnected && (retry_count < 5))
		{
			Sleep(200);
			logInform1("inovance_plc", Poco::format("%s地址读取失败,重新读取bool第%d次", address, retry_count).c_str());
			operation = H5u_Read_Device_Block(addr.type, addr.address, 1, resRead, plc_net_id);
			res = (operation == plc_operation_sucess);
			retry_count++;
		}
	}
	return res;
}

bool InovancePlcSubSystemHelperPrivate::readBits(const std::string address, const uint16_t length, bool *value)
{
	PLCAddress addr = getSoftElemTypeAndAddress(address);
	if (addr.address < 0)
	{
		return false;
	}
	//BYTE *resRead = new BYTE[length];
	std::unique_ptr<BYTE[]>resRead(new BYTE[length]);
	int operation = 0;
	if (plc_type == InovancePlcSubSystemHelper::PLCType::AM600)
	{
		operation = Am600_Read_Soft_Elem(addr.type, addr.address, length, resRead.get(), plc_net_id);
	}
	else if (plc_type == InovancePlcSubSystemHelper::PLCType::H3U)
	{
		operation = H3u_Read_Soft_Elem(addr.type, addr.address, length, resRead.get(), plc_net_id);
	}
	else
	{
		operation = H5u_Read_Device_Block(addr.type, addr.address, length, resRead.get(), plc_net_id);
	}
	if (operation == plc_operation_sucess)
	{
		for (size_t i = 0; i < length; i++)
		{
			value[i] = resRead[i] == 1;
		}
	}
	return operation == plc_operation_sucess;
}

bool InovancePlcSubSystemHelperPrivate::readShort(const std::string address, int16_t &value)
{

	PLCAddress addr = getSoftElemTypeAndAddress(address);
	if (addr.address < 0)
	{
		return false;
	}
	int operation = 0;
	if (plc_type == InovancePlcSubSystemHelper::PLCType::AM600)
	{
		operation = Am600_Read_Soft_Elem_Int16(addr.type, addr.address, 1, &value, plc_net_id);

	}
	else if (plc_type == InovancePlcSubSystemHelper::PLCType::H3U)
	{

		operation = H3u_Read_Soft_Elem_Int16(addr.type, addr.address, 1, &value, plc_net_id);
	}
	else
	{
		operation = H5u_Read_Soft_Elem_Int16(addr.type, addr.address, 1, &value, plc_net_id);
	}
	bool operation_res = (operation == plc_operation_sucess);
	if (operation_res == false)
	{
		int retry_count = 0;
		while (!operation_res && (retry_count < 5))
		{
			Sleep(200);
			logInform1("inovance_plc", Poco::format("%s地址读取失败,重新读取Short第%d次", address, retry_count).c_str());
			operation = H5u_Read_Soft_Elem_Int16(addr.type, addr.address, 1, &value, plc_net_id);
			operation_res = (operation == plc_operation_sucess);
			retry_count++;
		}
	}
	
	return operation == plc_operation_sucess;
}

bool InovancePlcSubSystemHelperPrivate::readShorts(const std::string address, const uint16_t length, int16_t *value)
{

	PLCAddress addr = getSoftElemTypeAndAddress(address);
	if (addr.address < 0)
	{
		return false;
	}
	int operation = 0;
	if (plc_type == InovancePlcSubSystemHelper::PLCType::AM600)
	{
		operation = Am600_Read_Soft_Elem_Int16(addr.type, addr.address, length, value, plc_net_id);

	}
	else if (plc_type == InovancePlcSubSystemHelper::PLCType::H3U)
	{

		operation = H3u_Read_Soft_Elem_Int16(addr.type, addr.address, length, value, plc_net_id);
	}
	else
	{
		operation = H5u_Read_Soft_Elem_Int16(addr.type, addr.address, length, value, plc_net_id);
	}
	return operation == plc_operation_sucess;
}

bool InovancePlcSubSystemHelperPrivate::readUnsignedShort(const std::string address, uint16_t &value)
{
	PLCAddress addr = getSoftElemTypeAndAddress(address);
	if (addr.address < 0)
	{
		return false;
	}
	int operation = 0;
	if (plc_type == InovancePlcSubSystemHelper::PLCType::AM600)
	{
		operation = Am600_Read_Soft_Elem_UInt16(addr.type, addr.address, 1, &value, plc_net_id);

	}
	else if (plc_type == InovancePlcSubSystemHelper::PLCType::H3U)
	{

		operation = H3u_Read_Soft_Elem_UInt16(addr.type, addr.address, 1, &value, plc_net_id);
	}
	else
	{
		operation = H5u_Read_Soft_Elem_UInt16(addr.type, addr.address, 1, &value, plc_net_id);
	}
	return operation == plc_operation_sucess;
}

bool InovancePlcSubSystemHelperPrivate::readUnsignedShorts(const std::string address, const uint16_t length, uint16_t *value)
{
	PLCAddress addr = getSoftElemTypeAndAddress(address);
	if (addr.address < 0)
	{
		return false;
	}
	int operation = 0;
	if (plc_type == InovancePlcSubSystemHelper::PLCType::AM600)
	{
		operation = Am600_Read_Soft_Elem_UInt16(addr.type, addr.address, length, value, plc_net_id);

	}
	else if (plc_type == InovancePlcSubSystemHelper::PLCType::H3U)
	{

		operation = H3u_Read_Soft_Elem_UInt16(addr.type, addr.address, length, value, plc_net_id);
	}
	else
	{
		operation = H5u_Read_Soft_Elem_UInt16(addr.type, addr.address, length, value, plc_net_id);
	}
	return operation == plc_operation_sucess;
}

bool InovancePlcSubSystemHelperPrivate::readInt(const std::string address, int32_t &value)
{

	PLCAddress addr = getSoftElemTypeAndAddress(address);
	if (addr.address < 0)
	{
		return false;
	}
	/*
	int operation = 0;
	long resRead = 0;
	if (plc_type == InovancePlcSubSystemHelper::PLCType::AM600)
	{
	operation = Am600_Read_Soft_Elem_Int32(addr.type, addr.address, 1, &resRead, plc_net_id);

	}
	else if (plc_type == InovancePlcSubSystemHelper::PLCType::H3U)
	{

	operation = H3u_Read_Soft_Elem_Int32(addr.type, addr.address, 1, &resRead, plc_net_id);
	}
	else
	{
	operation = H5u_Read_Soft_Elem_Int32(addr.type, addr.address, 1, &resRead, plc_net_id);
	}
	value = resRead;
	return operation == plc_operation_sucess;*/

	union {
		int32_t value;
		BYTE bytes[4];
	} u;
	int operation = 0;
	if (plc_type == InovancePlcSubSystemHelper::PLCType::AM600)
	{
		operation = Am600_Read_Soft_Elem(addr.type, addr.address, 2, u.bytes, plc_net_id);
	}
	else if (plc_type == InovancePlcSubSystemHelper::PLCType::H3U)
	{

		operation = H3u_Read_Soft_Elem(addr.type, addr.address, 2, u.bytes, plc_net_id);
	}
	else
	{
		operation = H5u_Read_Soft_Elem(addr.type, addr.address, 2, u.bytes, plc_net_id);
	}
	bool operation_res = operation == plc_operation_sucess;
	if (operation_res)
	{
		value = u.value;
	}

	return operation_res;
}

bool InovancePlcSubSystemHelperPrivate::readInts(const std::string address, const uint16_t length, int32_t *value)
{
	PLCAddress addr = getSoftElemTypeAndAddress(address);
	if (addr.address < 0)
	{
		return false;
	}
	
	int read_length = length * 2;
	//BYTE *read_result = new BYTE[length * 4];
	std::unique_ptr<BYTE[]>read_result(new BYTE[length * 4]);
	int operation = 0;
	if (plc_type == InovancePlcSubSystemHelper::PLCType::AM600)
	{
		operation = Am600_Read_Soft_Elem(addr.type, addr.address, read_length, read_result.get(), plc_net_id);
	}
	else if (plc_type == InovancePlcSubSystemHelper::PLCType::H3U)
	{
		operation = H3u_Read_Soft_Elem(addr.type, addr.address, read_length, read_result.get(), plc_net_id);
	}
	else
	{
		operation = H5u_Read_Soft_Elem(addr.type, addr.address, read_length, read_result.get(), plc_net_id);
	}
	bool operation_res = operation == plc_operation_sucess;
	if (operation_res)
	{

		for (size_t i = 0; i < length; i++)
		{
			union {
				int32_t value;
				BYTE bytes[4];
			} u;
			for (size_t j = 0; j < 4; j++)
			{
				u.bytes[j] = read_result[i * 4 + j];
			}
			value[i] = u.value;
		}

	}
	return operation_res;

}

bool InovancePlcSubSystemHelperPrivate::readUnsignedInt(const std::string address, uint32_t &value)
{
	PLCAddress addr = getSoftElemTypeAndAddress(address);
	if (addr.address < 0)
	{
		return false;
	}
	/*
	int operation = 0;
	unsigned long resRead = 0;
	if (plc_type == InovancePlcSubSystemHelper::PLCType::AM600)
	{
	operation = Am600_Read_Soft_Elem_UInt32(addr.type, addr.address, 1, &resRead, plc_net_id);
	}
	else if (plc_type == InovancePlcSubSystemHelper::PLCType::H3U)
	{
	operation = H3u_Read_Soft_Elem_UInt32(addr.type, addr.address, 1, &resRead, plc_net_id);
	}
	else
	{
	operation = H5u_Read_Soft_Elem_UInt32(addr.type, addr.address, 1, &resRead, plc_net_id);
	}
	value = resRead;
	return operation == plc_operation_sucess;*/

	union {
		uint32_t value;
		BYTE bytes[4];
	} u;
	int operation = 0;
	if (plc_type == InovancePlcSubSystemHelper::PLCType::AM600)
	{
		operation = Am600_Read_Soft_Elem(addr.type, addr.address, 2, u.bytes, plc_net_id);
	}
	else if (plc_type == InovancePlcSubSystemHelper::PLCType::H3U)
	{

		operation = H3u_Read_Soft_Elem(addr.type, addr.address, 2, u.bytes, plc_net_id);
	}
	else
	{
		operation = H5u_Read_Soft_Elem(addr.type, addr.address, 2, u.bytes, plc_net_id);
	}
	bool operation_res = operation == plc_operation_sucess;
	if (operation_res)
	{
		value = u.value;
	}

	return operation_res;
}

bool InovancePlcSubSystemHelperPrivate::readUnsignedInts(const std::string address, const uint16_t length, uint32_t *value)
{

	PLCAddress addr = getSoftElemTypeAndAddress(address);
	if (addr.address < 0)
	{
		return false;
	}
	
	int read_length = length * 2;
	//BYTE *read_result = new BYTE[length * 4];
	std::unique_ptr<BYTE[]>read_result(new BYTE[length * 4]);
	int operation = 0;
	if (plc_type == InovancePlcSubSystemHelper::PLCType::AM600)
	{
		operation = Am600_Read_Soft_Elem(addr.type, addr.address, read_length, read_result.get(), plc_net_id);
	}
	else if (plc_type == InovancePlcSubSystemHelper::PLCType::H3U)
	{
		operation = H3u_Read_Soft_Elem(addr.type, addr.address, read_length, read_result.get(), plc_net_id);
	}
	else
	{
		operation = H5u_Read_Soft_Elem(addr.type, addr.address, read_length, read_result.get(), plc_net_id);
	}
	bool operation_res = operation == plc_operation_sucess;
	if (operation_res)
	{

		for (size_t i = 0; i < length; i++)
		{
			union {
				uint32_t value;
				BYTE bytes[4];
			} u;
			for (size_t j = 0; j < 4; j++)
			{
				u.bytes[j] = read_result[i * 4 + j];
			}
			value[i] = u.value;
		}

	}
	return operation_res;
}

bool InovancePlcSubSystemHelperPrivate::readFloat(const std::string address, float &value)
{
	PLCAddress addr = getSoftElemTypeAndAddress(address);
	if (addr.address < 0)
	{
		return false;
	}
	/*
	int operation = 0;
	if (plc_type == InovancePlcSubSystemHelper::PLCType::AM600)
	{
	operation = Am600_Read_Soft_Elem_Float(addr.type, addr.address, 1, &value, plc_net_id);

	}
	else if (plc_type == InovancePlcSubSystemHelper::PLCType::H3U)
	{

	operation = H3u_Read_Soft_Elem_Float(addr.type, addr.address, 1, &value, plc_net_id);
	}
	else
	{
	operation = H5u_Read_Soft_Elem_Float(addr.type, addr.address, 1, &value, plc_net_id);
	}
	return operation == plc_operation_sucess;
	*/
	union {
		float f;
		BYTE bytes[4];
	} u;
	int operation = 0;
	if (plc_type == InovancePlcSubSystemHelper::PLCType::AM600)
	{
		operation = Am600_Read_Soft_Elem(addr.type, addr.address, 2, u.bytes, plc_net_id);
	}
	else if (plc_type == InovancePlcSubSystemHelper::PLCType::H3U)
	{

		operation = H3u_Read_Soft_Elem(addr.type, addr.address, 2, u.bytes, plc_net_id);
	}
	else
	{
		operation = H5u_Read_Soft_Elem(addr.type, addr.address, 2, u.bytes, plc_net_id);
	}
	bool operation_res = operation == plc_operation_sucess;
	if (operation_res == false)
	{
		int retry_count = 0;
		while (!operation_res && (retry_count < 8))
		{
			Sleep(200);
			logInform1("inovance_plc", Poco::format("%s地址读取失败,重新读取float第%d次", address, retry_count).c_str());
			operation = H5u_Read_Soft_Elem(addr.type, addr.address, 2, u.bytes, plc_net_id);
			operation_res = (operation == plc_operation_sucess);
			retry_count++;
		}
	}
	if (operation_res)
	{
		value = u.f;
	}

	return operation_res;
}

bool InovancePlcSubSystemHelperPrivate::readFloats(const std::string address, const uint16_t length, float *value)
{
	PLCAddress addr = getSoftElemTypeAndAddress(address);
	if (addr.address < 0)
	{
		return false;
	}
	/*
	int operation = 0;
	if (plc_type == InovancePlcSubSystemHelper::PLCType::AM600)
	{
	operation = Am600_Read_Soft_Elem_Float(addr.type, addr.address, length, value, plc_net_id);
	}
	else if (plc_type == InovancePlcSubSystemHelper::PLCType::H3U)
	{
	operation = H3u_Read_Soft_Elem_Float(addr.type, addr.address, length, value, plc_net_id);
	}
	else
	{
	operation = H5u_Read_Soft_Elem_Float(addr.type, addr.address, length, value, plc_net_id);
	}
	return operation == plc_operation_sucess;*/
	int read_length = length * 2;
	//BYTE *read_result = new BYTE[length * 4];
	std::unique_ptr<BYTE[]>read_result(new BYTE[length * 4]);
	int operation = 0;
	if (plc_type == InovancePlcSubSystemHelper::PLCType::AM600)
	{
		operation = Am600_Read_Soft_Elem(addr.type, addr.address, read_length, read_result.get(), plc_net_id);
	}
	else if (plc_type == InovancePlcSubSystemHelper::PLCType::H3U)
	{
		operation = H3u_Read_Soft_Elem(addr.type, addr.address, read_length, read_result.get(), plc_net_id);
	}
	else
	{
		operation = H5u_Read_Soft_Elem(addr.type, addr.address, read_length, read_result.get(), plc_net_id);
	}
	bool operation_res = operation == plc_operation_sucess;
	if (operation_res)
	{

		for (size_t i = 0; i < length; i++)
		{
			union {
				float f;
				BYTE bytes[4];
			} u;
			for (size_t j = 0; j < 4; j++)
			{
				u.bytes[j] = read_result[i * 4 + j];
			}
			value[i] = u.f;
		}

	}
	return operation_res;
}

bool InovancePlcSubSystemHelperPrivate::readDouble(const std::string address, double &value)
{
	PLCAddress addr = getSoftElemTypeAndAddress(address);
	if (addr.address < 0)
	{
		return false;
	}
	union {
		double d;
		BYTE bytes[8];
	} u;
	int operation = 0;
	if (plc_type == InovancePlcSubSystemHelper::PLCType::AM600)
	{
		operation = Am600_Read_Soft_Elem(addr.type, addr.address, 4, u.bytes, plc_net_id);
	}
	else if (plc_type == InovancePlcSubSystemHelper::PLCType::H3U)
	{

		operation = H3u_Read_Soft_Elem(addr.type, addr.address, 4, u.bytes, plc_net_id);
	}
	else
	{
		operation = H5u_Read_Soft_Elem(addr.type, addr.address, 4, u.bytes, plc_net_id);
	}
	bool operation_res = operation == plc_operation_sucess;
	if (operation_res)
	{
		value = u.d;
	}

	return operation_res;
}

bool InovancePlcSubSystemHelperPrivate::readDoubles(const std::string address, const uint16_t length, double *value)
{
	PLCAddress addr = getSoftElemTypeAndAddress(address);
	if (addr.address < 0)
	{
		return false;
	}
	int read_length = length * 4;
	//BYTE *read_result = new BYTE[length * 8];
	std::unique_ptr<BYTE[]>read_result(new BYTE[length * 8]);
	int operation = 0;
	if (plc_type == InovancePlcSubSystemHelper::PLCType::AM600)
	{
		operation = Am600_Read_Soft_Elem(addr.type, addr.address, read_length, read_result.get(), plc_net_id);
	}
	else if (plc_type == InovancePlcSubSystemHelper::PLCType::H3U)
	{
		operation = H3u_Read_Soft_Elem(addr.type, addr.address, read_length, read_result.get(), plc_net_id);
	}
	else
	{
		operation = H5u_Read_Soft_Elem(addr.type, addr.address, read_length, read_result.get(), plc_net_id);
	}
	bool operation_res = operation == plc_operation_sucess;
	if (operation_res)
	{

		for (size_t i = 0; i < length; i++)
		{
			union {
				double d;
				BYTE bytes[8];
			} u;
			for (size_t j = 0; j < 8; j++)
			{
				u.bytes[j] = read_result[i * 8 + j];
			}
			value[i] = u.d;
		}

	}
	return operation_res;
}

bool InovancePlcSubSystemHelperPrivate::readString(const std::string address, const uint16_t length, std::string &value)
{
	PLCAddress addr = getSoftElemTypeAndAddress(address);
	if (addr.address < 0)
	{
		return false;
	}
	//BYTE *read_result = new BYTE[length * 2];
	std::unique_ptr<BYTE[]>read_result(new BYTE[length * 2]);
	int operation = 0;
	if (plc_type == InovancePlcSubSystemHelper::PLCType::AM600)
	{
		operation = Am600_Read_Soft_Elem(addr.type, addr.address, length, read_result.get(), plc_net_id);
	}
	else if (plc_type == InovancePlcSubSystemHelper::PLCType::H3U)
	{
		operation = H3u_Read_Soft_Elem(addr.type, addr.address, length, read_result.get(), plc_net_id);
	}
	else
	{
		operation = H5u_Read_Soft_Elem(addr.type, addr.address, length, read_result.get(), plc_net_id);
	}
	bool operation_res = operation == plc_operation_sucess;
	if (operation_res)
	{

		std::string str(reinterpret_cast<char *>(read_result.get()), length);
		value = str;
	}
	read_result = nullptr;
	return operation_res;
}

bool InovancePlcSubSystemHelperPrivate::writeBit(const std::string address, const bool value)
{
	PLCAddress addr = getSoftElemTypeAndAddress(address);
	if (addr.address < 0)
	{
		return false;
	}
	bool res = false;
	int operation = 0;
	BYTE write_value[1] = { value };
	if (plc_type == InovancePlcSubSystemHelper::PLCType::AM600)
	{
		operation = Am600_Write_Soft_Elem(addr.type, addr.address, 1, write_value, plc_net_id);
	}
	else if (plc_type == InovancePlcSubSystemHelper::PLCType::H3U)
	{
		operation = H3u_Write_Soft_Elem(addr.type, addr.address, 1, write_value, plc_net_id);
	}
	else
	{
		operation = H5u_Write_Soft_Elem(addr.type, addr.address, 1, write_value, plc_net_id);
		res = (operation == plc_operation_sucess);
		if (res == false)
		{
			int retry_count = 0;
			while (!res&&isConnected && (retry_count < 10))
			{
				Sleep(500);
				logInform1("inovance_plc", Poco::format("%s地址写入失败,重写bool第%d次", address, retry_count).c_str());
				operation = H5u_Write_Soft_Elem(addr.type, addr.address, 1, write_value, plc_net_id);
				res = (operation == plc_operation_sucess);
				retry_count++;
			}
		}
	}
	return res;
}
bool InovancePlcSubSystemHelperPrivate::writeBits(const std::string address, const uint16_t length, const bool *values)
{
	PLCAddress addr = getSoftElemTypeAndAddress(address);
	if (addr.address < 0)
	{
		return false;
	}
	bool res = false;
	int operation = 0;
	//BYTE *write_values = new BYTE[length];
	std::unique_ptr<BYTE[]>write_values(new BYTE[length]);
	for (size_t i = 0; i < length; i++)
	{
		write_values[i] = values[i];
	}

	if (plc_type == InovancePlcSubSystemHelper::PLCType::AM600)
	{
		operation = Am600_Write_Soft_Elem(addr.type, addr.address, length, write_values.get(), plc_net_id);
	}
	else if (plc_type == InovancePlcSubSystemHelper::PLCType::H3U)
	{
		operation = H3u_Write_Soft_Elem(addr.type, addr.address, length, write_values.get(), plc_net_id);
	}
	else
	{
		operation = H5u_Write_Soft_Elem(addr.type, addr.address, length, write_values.get(), plc_net_id);
	}
	return operation == plc_operation_sucess;
}
bool InovancePlcSubSystemHelperPrivate::writeShort(const std::string address, const int16_t value)
{
	PLCAddress addr = getSoftElemTypeAndAddress(address);
	if (addr.address < 0)
	{
		return false;
	}
	bool res = false;
	int operation = 0;
	short write_value = value;
	if (plc_type == InovancePlcSubSystemHelper::PLCType::AM600)
	{
		operation = Am600_Write_Soft_Elem_Int16(addr.type, addr.address, 1, &write_value, plc_net_id);
	}
	else if (plc_type == InovancePlcSubSystemHelper::PLCType::H3U)
	{
		operation = H3u_Write_Soft_Elem_Int16(addr.type, addr.address, 1, &write_value, plc_net_id);
	}
	else
	{
		operation = H5u_Write_Soft_Elem_Int16(addr.type, addr.address, 1, &write_value, plc_net_id);
		res = (operation == plc_operation_sucess);
		if (res == false)
		{
			int retry_count = 0;
			while (!res&&isConnected && (retry_count < 10))
			{
				Sleep(500);
				logInform1("inovance_plc", Poco::format("%s地址写入失败,重写Short第%d次", address, retry_count).c_str());
				operation = H5u_Write_Soft_Elem_Int16(addr.type, addr.address, 1, &write_value, plc_net_id);
				res = (operation == plc_operation_sucess);
				retry_count++;
			}
		}
	}
	return operation == plc_operation_sucess;
}
bool InovancePlcSubSystemHelperPrivate::writeShorts(const std::string address, const uint16_t length, const int16_t *values)
{
	PLCAddress addr = getSoftElemTypeAndAddress(address);
	if (addr.address < 0)
	{
		return false;
	}
	bool res = false;
	int operation = 0;
	//short *write_values = new short[length];
	std::unique_ptr<short[]>write_values(new short[length]);
	for (size_t i = 0; i < length; i++)
	{
		write_values[i] = values[i];
	}

	if (plc_type == InovancePlcSubSystemHelper::PLCType::AM600)
	{
		operation = Am600_Write_Soft_Elem_Int16(addr.type, addr.address, length, write_values.get(), plc_net_id);
	}
	else if (plc_type == InovancePlcSubSystemHelper::PLCType::H3U)
	{
		operation = H3u_Write_Soft_Elem_Int16(addr.type, addr.address, length, write_values.get(), plc_net_id);
	}
	else
	{
		operation = H5u_Write_Soft_Elem_Int16(addr.type, addr.address, length, write_values.get(), plc_net_id);
	}

	return operation == plc_operation_sucess;
}
bool InovancePlcSubSystemHelperPrivate::writeUnsignedShort(const std::string address, const uint16_t value)
{
	PLCAddress addr = getSoftElemTypeAndAddress(address);
	if (addr.address < 0)
	{
		return false;
	}
	bool res = false;
	int operation = 0;
	unsigned short write_value = value;
	if (plc_type == InovancePlcSubSystemHelper::PLCType::AM600)
	{
		operation = Am600_Write_Soft_Elem_UInt16(addr.type, addr.address, 1, &write_value, plc_net_id);
	}
	else if (plc_type == InovancePlcSubSystemHelper::PLCType::H3U)
	{
		operation = H3u_Write_Soft_Elem_UInt16(addr.type, addr.address, 1, &write_value, plc_net_id);
	}
	else
	{
		operation = H5u_Write_Soft_Elem_UInt16(addr.type, addr.address, 1, &write_value, plc_net_id);
		res = (operation == plc_operation_sucess);
		if (res == false)
		{
			int retry_count = 0;
			while (!res&&isConnected && (retry_count < 10))
			{
				Sleep(500);
				logInform1("inovance_plc", Poco::format("%s地址写入失败,重写UnsignedShort第%d次", address, retry_count).c_str());
				operation = H5u_Write_Soft_Elem_UInt16(addr.type, addr.address, 1, &write_value, plc_net_id);
				res = (operation == plc_operation_sucess);
				retry_count++;
			}
		}
	}
	return operation == plc_operation_sucess;
}
bool InovancePlcSubSystemHelperPrivate::writeUnsignedShorts(const std::string address, const uint16_t length, const uint16_t *values)
{

	PLCAddress addr = getSoftElemTypeAndAddress(address);
	if (addr.address < 0)
	{
		return false;
	}
	bool res = false;
	int operation = 0;
	//unsigned short *write_values = new unsigned short[length];
	std::unique_ptr<unsigned short[]>write_values(new unsigned short[length]);
	for (size_t i = 0; i < length; i++)
	{
		write_values[i] = values[i];
	}

	if (plc_type == InovancePlcSubSystemHelper::PLCType::AM600)
	{
		operation = Am600_Write_Soft_Elem_UInt16(addr.type, addr.address, length, write_values.get(), plc_net_id);
	}
	else if (plc_type == InovancePlcSubSystemHelper::PLCType::H3U)
	{
		operation = H3u_Write_Soft_Elem_UInt16(addr.type, addr.address, length, write_values.get(), plc_net_id);
	}
	else
	{
		operation = H5u_Write_Soft_Elem_UInt16(addr.type, addr.address, length, write_values.get(), plc_net_id);
	}

	return operation == plc_operation_sucess;
}
bool InovancePlcSubSystemHelperPrivate::writeInt(const std::string address, const int32_t value)
{
	PLCAddress addr = getSoftElemTypeAndAddress(address);
	if (addr.address < 0)
	{
		return false;
	}
	/*
	bool res = false;
	int operation = 0;
	long write_value = value;
	if (plc_type == InovancePlcSubSystemHelper::PLCType::AM600)
	{
	operation = Am600_Write_Soft_Elem_Int32(addr.type, addr.address, 1, &write_value, plc_net_id);
	}
	else if (plc_type == InovancePlcSubSystemHelper::PLCType::H3U)
	{
	operation = H3u_Write_Soft_Elem_Int32(addr.type, addr.address, 1, &write_value, plc_net_id);
	}
	else
	{
	operation = H5u_Write_Soft_Elem_Int32(addr.type, addr.address, 1, &write_value, plc_net_id);
	}
	return operation == plc_operation_sucess;*/
	bool res = false;
	union {
		int32_t value;
		BYTE bytes[4];
	} u;
	u.value = value;
	int operation = 0;
	if (plc_type == InovancePlcSubSystemHelper::PLCType::AM600)
	{
		operation = Am600_Write_Soft_Elem(addr.type, addr.address, 2, u.bytes, plc_net_id);
		res = operation == plc_operation_sucess;
	}
	else if (plc_type == InovancePlcSubSystemHelper::PLCType::H3U)
	{

		operation = H3u_Write_Soft_Elem(addr.type, addr.address, 2, u.bytes, plc_net_id);
		res = operation == plc_operation_sucess;
	}
	else
	{
		operation = H5u_Write_Soft_Elem(addr.type, addr.address, 2, u.bytes, plc_net_id);
		res = (operation == plc_operation_sucess);
		if (res == false)
		{
			int retry_count = 0;
			while (!res &&isConnected && (retry_count < 10))
			{
				Sleep(500);
				logInform1("inovance_plc", Poco::format("%s地址写入失败,重写UnsignedInt第%d次", address, retry_count).c_str());
				operation = H5u_Write_Soft_Elem(addr.type, addr.address, 2, u.bytes, plc_net_id);
				res = (operation == plc_operation_sucess);
				retry_count++;
			}
		}
	}
	return res;
}
bool InovancePlcSubSystemHelperPrivate::writeInts(const std::string address, const uint16_t length, const int32_t *values)
{
	PLCAddress addr = getSoftElemTypeAndAddress(address);
	if (addr.address < 0)
	{
		return false;
	}
	
	union {
		uint32_t value;
		BYTE bytes[4];
	} u;
	int write_length = length * 2;
	//BYTE *write_value = new BYTE[write_length * 2];
	std::unique_ptr<BYTE[]>write_value(new BYTE[length * 2]);
	for (size_t i = 0; i < length; i++)
	{
		u.value = values[i];
		for (size_t j = 0; j < 4; j++)
		{
			write_value[i * 4 + j] = u.bytes[j];
		}
	}

	int operation = 0;
	if (plc_type == InovancePlcSubSystemHelper::PLCType::AM600)
	{
		operation = Am600_Write_Soft_Elem(addr.type, addr.address, write_length, write_value.get(), plc_net_id);

	}
	else if (plc_type == InovancePlcSubSystemHelper::PLCType::H3U)
	{

		operation = H3u_Write_Soft_Elem(addr.type, addr.address, write_length, write_value.get(), plc_net_id);
	}
	else
	{
		operation = H5u_Write_Soft_Elem(addr.type, addr.address, write_length, write_value.get(), plc_net_id);
	}
	return operation == plc_operation_sucess;
}
bool InovancePlcSubSystemHelperPrivate::writeUnsignedInt(const std::string address, const uint32_t value)
{
	PLCAddress addr = getSoftElemTypeAndAddress(address);
	if (addr.address < 0)
	{
		return false;
	}
	/*
	bool res = false;
	int operation = 0;
	unsigned long write_value = value;
	if (plc_type == InovancePlcSubSystemHelper::PLCType::AM600)
	{
	operation = Am600_Write_Soft_Elem_UInt32(addr.type, addr.address, 1, &write_value, plc_net_id);
	}
	else if (plc_type == InovancePlcSubSystemHelper::PLCType::H3U)
	{
	operation = H3u_Write_Soft_Elem_UInt32(addr.type, addr.address, 1, &write_value, plc_net_id);
	}
	else
	{
	operation = H5u_Write_Soft_Elem_UInt32(addr.type, addr.address, 1, &write_value, plc_net_id);
	}
	return operation == plc_operation_sucess;*/
	bool res = false;
	union {
		uint32_t value;
		BYTE bytes[4];
	} u;
	u.value = value;
	int operation = 0;
	if (plc_type == InovancePlcSubSystemHelper::PLCType::AM600)
	{
		operation = Am600_Write_Soft_Elem(addr.type, addr.address, 2, u.bytes, plc_net_id);
		res = (operation == plc_operation_sucess);
	}
	else if (plc_type == InovancePlcSubSystemHelper::PLCType::H3U)
	{

		operation = H3u_Write_Soft_Elem(addr.type, addr.address, 2, u.bytes, plc_net_id);
		res = (operation == plc_operation_sucess);
	}
	else
	{
		operation = H5u_Write_Soft_Elem(addr.type, addr.address, 2, u.bytes, plc_net_id);
		res = (operation == plc_operation_sucess);
		if (res == false)
		{
			int retry_count = 0;
			while (!res&&isConnected && (retry_count < 10))
			{
				Sleep(500);
				logInform1("inovance_plc", Poco::format("%s地址写入失败,重写UnsignedInt第%d次", address, retry_count).c_str());
				operation = H5u_Write_Soft_Elem(addr.type, addr.address, 2, u.bytes, plc_net_id);
				res = (operation == plc_operation_sucess);
				retry_count++;
			}
		}
	}
	return res;
}
bool InovancePlcSubSystemHelperPrivate::writeUnsignedInts(const std::string address, const uint16_t length, const uint32_t *values)
{
	PLCAddress addr = getSoftElemTypeAndAddress(address);
	if (addr.address < 0)
	{
		return false;
	}
	union {
		uint32_t value;
		BYTE bytes[4];
	} u;
	int write_length = length * 2;
	//BYTE *write_value = new BYTE[write_length * 2];
	std::unique_ptr<BYTE[]>write_value(new BYTE[write_length * 2]);
	for (size_t i = 0; i < length; i++)
	{
		u.value = values[i];
		for (size_t j = 0; j < 4; j++)
		{
			write_value[i * 4 + j] = u.bytes[j];
		}
	}

	int operation = 0;
	if (plc_type == InovancePlcSubSystemHelper::PLCType::AM600)
	{
		operation = Am600_Write_Soft_Elem(addr.type, addr.address, write_length, write_value.get(), plc_net_id);

	}
	else if (plc_type == InovancePlcSubSystemHelper::PLCType::H3U)
	{

		operation = H3u_Write_Soft_Elem(addr.type, addr.address, write_length, write_value.get(), plc_net_id);
	}
	else
	{
		operation = H5u_Write_Soft_Elem(addr.type, addr.address, write_length, write_value.get(), plc_net_id);
	}
	return operation == plc_operation_sucess;
}
bool InovancePlcSubSystemHelperPrivate::writeFloat(const std::string address, const float value)
{

	PLCAddress addr = getSoftElemTypeAndAddress(address);
	if (addr.address < 0)
	{
		return false;
	}
	/*
	bool res = false;
	int operation = 0;
	float write_value = value;
	if (plc_type == InovancePlcSubSystemHelper::PLCType::AM600)
	{
	operation = Am600_Write_Soft_Elem_Float(addr.type, addr.address, 1, &write_value, plc_net_id);
	}
	else if (plc_type == InovancePlcSubSystemHelper::PLCType::H3U)
	{
	operation = H3u_Write_Soft_Elem_Float(addr.type, addr.address, 1, &write_value, plc_net_id);
	}
	else
	{
	operation = H5u_Write_Soft_Elem_Float(addr.type, addr.address, 1, &write_value, plc_net_id);
	}
	return operation == plc_operation_sucess;*/
	bool res = false;
	union {
		float f;
		BYTE bytes[4];
	} u;
	u.f = value;
	int operation = 0;
	if (plc_type == InovancePlcSubSystemHelper::PLCType::AM600)
	{
		operation = Am600_Write_Soft_Elem(addr.type, addr.address, 2, u.bytes, plc_net_id);
		res = (operation == plc_operation_sucess);
	}
	else if (plc_type == InovancePlcSubSystemHelper::PLCType::H3U)
	{
		operation = H3u_Write_Soft_Elem(addr.type, addr.address, 2, u.bytes, plc_net_id);
		res = (operation == plc_operation_sucess);
	}
	else
	{
		operation = H5u_Write_Soft_Elem(addr.type, addr.address, 2, u.bytes, plc_net_id);
		res = (operation == plc_operation_sucess);
		if (res == false)
		{
			int retry_count = 0;
			while (!res&&isConnected && (retry_count < 10))
			{
				Sleep(500);
				logInform1("inovance_plc", Poco::format("%s地址写入失败,重写Float第%d次", address, retry_count).c_str());
				operation = H5u_Write_Soft_Elem(addr.type, addr.address, 2, u.bytes, plc_net_id);
				res = (operation == plc_operation_sucess);
				retry_count++;
			}
		}
	}

	return res;
}

bool InovancePlcSubSystemHelperPrivate::writeFloats(const std::string address, const uint16_t length, const float *values)
{
	PLCAddress addr = getSoftElemTypeAndAddress(address);
	if (addr.address < 0)
	{
		return false;
	}
	
	union {
		float f;
		BYTE bytes[4];
	} u;
	int write_length = length * 2;
	//BYTE *write_value = new BYTE[write_length * 2];
	std::unique_ptr<BYTE[]>write_value(new BYTE[write_length * 2]);
	for (size_t i = 0; i < length; i++)
	{
		u.f = values[i];
		for (size_t j = 0; j < 4; j++)
		{
			write_value[i * 4 + j] = u.bytes[j];
		}
	}

	int operation = 0;
	if (plc_type == InovancePlcSubSystemHelper::PLCType::AM600)
	{
		operation = Am600_Write_Soft_Elem(addr.type, addr.address, write_length, write_value.get(), plc_net_id);

	}
	else if (plc_type == InovancePlcSubSystemHelper::PLCType::H3U)
	{

		operation = H3u_Write_Soft_Elem(addr.type, addr.address, write_length, write_value.get(), plc_net_id);
	}
	else
	{
		operation = H5u_Write_Soft_Elem(addr.type, addr.address, write_length, write_value.get(), plc_net_id);
	}
	return operation == plc_operation_sucess;
}

bool InovancePlcSubSystemHelperPrivate::writeDouble(const std::string address, const double value){

	PLCAddress addr = getSoftElemTypeAndAddress(address);
	if (addr.address < 0)
	{
		return false;
	}
	union {
		double d;
		BYTE bytes[8];
	} u;
	u.d = value;
	bool res = false;
	int operation = 0;
	if (plc_type == InovancePlcSubSystemHelper::PLCType::AM600)
	{
		operation = Am600_Write_Soft_Elem(addr.type, addr.address, 4, u.bytes, plc_net_id);
		res = (operation == plc_operation_sucess);
	}
	else if (plc_type == InovancePlcSubSystemHelper::PLCType::H3U)
	{
		operation = H3u_Write_Soft_Elem(addr.type, addr.address, 4, u.bytes, plc_net_id);
		res = (operation == plc_operation_sucess);
	}
	else
	{
		operation = H5u_Write_Soft_Elem(addr.type, addr.address, 4, u.bytes, plc_net_id);
		res = (operation == plc_operation_sucess);
		if (res == false)
		{
			int retry_count = 0;
			while (!res&&isConnected && (retry_count < 10))
			{
				Sleep(500);
				logInform1("inovance_plc", Poco::format("%s地址写入失败,重写Double第%d次", address, retry_count).c_str());
				operation = H5u_Write_Soft_Elem(addr.type, addr.address, 4, u.bytes, plc_net_id);
				res = (operation == plc_operation_sucess);
				retry_count++;
			}
		}
	}

	return operation == plc_operation_sucess;
}
bool InovancePlcSubSystemHelperPrivate::writeDoubles(const std::string address, const uint16_t length, const double *values){

	PLCAddress addr = getSoftElemTypeAndAddress(address);
	if (addr.address < 0)
	{
		return false;
	}

	union {
		double d;
		BYTE bytes[8];
	} u;
	int write_length = length * 4;
	//BYTE *write_value = new BYTE[write_length * 2];
	std::unique_ptr<BYTE[]>write_value(new BYTE[write_length * 2]);
	for (size_t i = 0; i < length; i++)
	{
		u.d = values[i];
		for (size_t j = 0; j < 8; j++)
		{
			write_value[i * 8 + j] = u.bytes[j];
		}
	}

	int operation = 0;
	if (plc_type == InovancePlcSubSystemHelper::PLCType::AM600)
	{
		operation = Am600_Write_Soft_Elem(addr.type, addr.address, write_length, write_value.get(), plc_net_id);

	}
	else if (plc_type == InovancePlcSubSystemHelper::PLCType::H3U)
	{

		operation = H3u_Write_Soft_Elem(addr.type, addr.address, write_length, write_value.get(), plc_net_id);
	}
	else
	{
		operation = H5u_Write_Soft_Elem(addr.type, addr.address, write_length, write_value.get(), plc_net_id);
	}
	return operation == plc_operation_sucess;
}
bool InovancePlcSubSystemHelperPrivate::writeString(const std::string address, const std::string value){
	PLCAddress addr = getSoftElemTypeAndAddress(address);
	if (addr.address < 0)
	{
		return false;
	}
	int length = value.length();
	int write_length = 0;
	if (length % 2 == 0)
	{
		write_length = length;
	}
	else
	{
		write_length = length + 1;
	}
	//BYTE *write_value = new BYTE[write_length];
	std::unique_ptr<BYTE[]>write_value(new BYTE[write_length]);
	int i = 0;
	for (const char c : value)
	{
		write_value[i] = c;
		i++;
	}
	if (length % 2 != 0)
	{
		write_value[write_length - 1] = 0x00;
	}

	int operation = 0;
	if (plc_type == InovancePlcSubSystemHelper::PLCType::AM600)
	{
		operation = Am600_Write_Soft_Elem(addr.type, addr.address, write_length / 2, write_value.get(), plc_net_id);

	}
	else if (plc_type == InovancePlcSubSystemHelper::PLCType::H3U)
	{

		operation = H3u_Write_Soft_Elem(addr.type, addr.address, write_length / 2, write_value.get(), plc_net_id);
	}
	else
	{
		operation = H5u_Write_Soft_Elem(addr.type, addr.address, write_length / 2, write_value.get(), plc_net_id);
	}
	return operation == plc_operation_sucess;
}


std::vector<std::string> InovancePlcSubSystemHelperPrivate::splitData(const std::string& data, char delimiter) {
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream tokenStream(data);
	while (std::getline(tokenStream, token, delimiter)) {
		tokens.push_back(token);
	}
	return tokens;
}


void InovancePlcSubSystemHelperPrivate::configure(const std::shared_ptr<KernelConfiguration> & config){

	port = config->getInt("inovance_port", 502);
	plc_net_id = config->getInt("inovance_net_id", 0);
	ip_address = config->getString("inovance_ip", "192.168.1.88");
	plc_timeout_milsec = std::stoul(config->getString("inovance_plc_timeout", "1000"));
	std::string str_plc_type = config->getString("inovance_plc_type", "H5U");
	if (str_plc_type == "AM600")
	{
		plc_type = InovancePlcSubSystemHelper::AM600;
	}
	else if (str_plc_type == "H3U")
	{
		plc_type = InovancePlcSubSystemHelper::H3U;
	}
	else
	{
		plc_type = InovancePlcSubSystemHelper::H5U;
	}
	//load defined alarms
	if (config->has("ErrorTxt_Acmezj"))
	{
		std::string errFileName = config->getString("ErrorTxt_Acmezj", name + ".txt");
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
			std::shared_ptr<InovancePlcSubSystemHelper::DefinedError> errorObj(new InovancePlcSubSystemHelper::DefinedError);
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

std::shared_ptr<InovancePlcSubSystemHelper::DefinedError> InovancePlcSubSystemHelperPrivate::getErrorCode(const int code_type, const int code_id)
{
	std::shared_ptr<InovancePlcSubSystemHelper::DefinedError> res = std::make_shared<InovancePlcSubSystemHelper::DefinedError>(InovancePlcSubSystemHelper::DefinedError{ code_type, code_id, "Undefined" });
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
	res->message = "从硬件配置中未找到错误代码。";
	res->needReset = true;
	return res;
}



/**
* InovancePlcSubSystemHelper
*/
InovancePlcSubSystemHelper::InovancePlcSubSystemHelper(const std::string& name)
	: d(new InovancePlcSubSystemHelperPrivate(this, name)){

}


/**
* initialize
*/
bool InovancePlcSubSystemHelper::enableProtocol(){
	return d->open();
}


/**
* unInitialize
*/
void InovancePlcSubSystemHelper::disableProtocol(){
	d->close();
}

/**
*@brief  input count in module
*/
int InovancePlcSubSystemHelper::helperInputCount()const
{
	return d->ioConfig.input_names.size();
}

/**
*@brief  output count in module
*/
int InovancePlcSubSystemHelper::helperOutputCount()const
{
	return d->ioConfig.output_names.size();
}

/**
*@brief  get input address
*/
std::string InovancePlcSubSystemHelper::getHelperInputAddress() const
{
	return d->ioConfig.input_address;
}

/**
*@brief  get input status
*/
std::string InovancePlcSubSystemHelper::getHelperOutputAddress(const int index)const
{
	int count = d->ioConfig.output_addresses.size();
	if (index <count)
	{
		return d->ioConfig.output_addresses[index];
	}
	return "";
}


/**
*@brief  get input name
*/
std::string InovancePlcSubSystemHelper::getHelperInputName(int index) const
{
	int count = d->ioConfig.input_names.size();
	if (index < count)
	{
		return d->ioConfig.input_names[index];
	}
	return "";
}

/**
*@brief  get input name
*/
std::string InovancePlcSubSystemHelper::getHelperOutputName(int index) const
{
	int count = d->ioConfig.output_names.size();
	if (index < count)
	{
		return d->ioConfig.output_names[index];
	}
	return "";
}

/**
* get error code
*/
std::shared_ptr<InovancePlcSubSystemHelper::DefinedError> InovancePlcSubSystemHelper::getErrorCode(const int code_type, const int code_id){
	return d->getErrorCode(code_type, code_id);
}


void InovancePlcSubSystemHelper::configInovancePlc(const std::shared_ptr<KernelConfiguration> & config){
	d->configure(config);
}

bool InovancePlcSubSystemHelper::readBit(const std::string address, bool &value){ return d->readBit(address, value); }
bool InovancePlcSubSystemHelper::readBits(const std::string address, const uint16_t length, bool *value){ return d->readBits(address, length, value); }
bool InovancePlcSubSystemHelper::readShort(const std::string address, int16_t &value){ return d->readShort(address, value); }
bool InovancePlcSubSystemHelper::readShorts(const std::string address, const uint16_t length, int16_t *value){ return d->readShorts(address, length, value); }
bool InovancePlcSubSystemHelper::readUnsignedShort(const std::string address, uint16_t &value){ return d->readUnsignedShort(address, value); }
bool InovancePlcSubSystemHelper::readUnsignedShorts(const std::string address, const uint16_t length, uint16_t *value){ return d->readUnsignedShorts(address, length, value); }
bool InovancePlcSubSystemHelper::readInt(const std::string address, int32_t &value){ return d->readInt(address, value); }
bool InovancePlcSubSystemHelper::readInts(const std::string address, const uint16_t length, int32_t *value){ return d->readInts(address, length, value); }
bool InovancePlcSubSystemHelper::readUnsignedInt(const std::string address, uint32_t &value){ return d->readUnsignedInt(address, value); }
bool InovancePlcSubSystemHelper::readUnsignedInts(const std::string address, const uint16_t length, uint32_t *value){ return d->readUnsignedInts(address, length, value); }
bool InovancePlcSubSystemHelper::readFloat(const std::string address, float &value){ return d->readFloat(address, value); }
bool InovancePlcSubSystemHelper::readFloats(const std::string address, const uint16_t length, float *value){ return d->readFloats(address, length, value); }
bool InovancePlcSubSystemHelper::readDouble(const std::string address, double &value){ return d->readDouble(address, value); }
bool InovancePlcSubSystemHelper::readDoubles(const std::string address, const uint16_t length, double *value){ return d->readDoubles(address, length, value); }
bool InovancePlcSubSystemHelper::readString(const std::string address, const uint16_t length, std::string &value){ return d->readString(address, length, value); }

bool InovancePlcSubSystemHelper::writeBit(const std::string address, const bool value){ return d->writeBit(address, value); }
bool InovancePlcSubSystemHelper::writeBits(const std::string address, const uint16_t length, const bool *values){ return d->writeBits(address, length, values); }
bool InovancePlcSubSystemHelper::writeShort(const std::string address, const int16_t value){ return d->writeShort(address, value); }
bool InovancePlcSubSystemHelper::writeShorts(const std::string address, const uint16_t length, const int16_t *values){ return d->writeShorts(address, length, values); }
bool InovancePlcSubSystemHelper::writeUnsignedShort(const std::string address, const uint16_t value){ return d->writeUnsignedShort(address, value); }
bool InovancePlcSubSystemHelper::writeUnsignedShorts(const std::string address, const uint16_t length, const uint16_t *values){ return d->writeUnsignedShorts(address, length, values); }
bool InovancePlcSubSystemHelper::writeInt(const std::string address, const int32_t value){ return d->writeInt(address, value); }
bool InovancePlcSubSystemHelper::writeInts(const std::string address, const uint16_t length, const int32_t *values){ return d->writeInts(address, length, values); }
bool InovancePlcSubSystemHelper::writeUnsignedInt(const std::string address, const uint32_t value){ return d->writeUnsignedInt(address, value); }
bool InovancePlcSubSystemHelper::writeUnsignedInts(const std::string address, const uint16_t length, const uint32_t *values){ return d->writeUnsignedInts(address, length, values); }
bool InovancePlcSubSystemHelper::writeFloat(const std::string address, const float value){ return d->writeFloat(address, value); }
bool InovancePlcSubSystemHelper::writeFloats(const std::string address, const uint16_t length, const float *values){ return d->writeFloats(address, length, values); }
bool InovancePlcSubSystemHelper::writeDouble(const std::string address, const double value){ return d->writeDouble(address, value); }
bool InovancePlcSubSystemHelper::writeDoubles(const std::string address, const uint16_t length, const double *values){ return d->writeDoubles(address, length, values); }
bool InovancePlcSubSystemHelper::writeString(const std::string address, const std::string value){ return d->writeString(address, value); }

KERNEL_NS_END