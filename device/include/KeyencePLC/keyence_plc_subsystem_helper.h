/**
* @file            keyence_plc_subsystem_helper.h
* @brief           Fortrend keyence plc
* @author		   kai
*/

// Library: Fortrend
// Package: Subsystem/KeyencePLC



#ifndef _XLH_FORTREND_KEYENCE_PLC_SUBSYSTEM_HELPER_INCLUDE_
#define _XLH_FORTREND_KEYENCE_PLC_SUBSYSTEM_HELPER_INCLUDE_

#include "kernel/Fortrend/fortrend_macros.h"
#include "Kernel/kernel_macros.h"
#include "Kernel/kernel_exception.h"

#include <vector>
#include <map>

KERNEL_NS_BEGIN
class KernelConfiguration;

#define SIM_MODE 1
/**
*@brief  helper for rnd protocol subsystem
*/
class KeyencePlcSubSystemHelper
{
public:
	DECLARE_PTR(KeyencePlcSubSystemHelper)
	typedef struct{
		int type;
		int code;  
		std::string message;
		bool needReset;
	}DefinedError;
	
	explicit KeyencePlcSubSystemHelper(const std::string& name);
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
	/**
	*@brief  input count in module
	*/
	int helperInputCount()const;

	/**
	*@brief  output count in module
	*/
	int helperOutputCount()const;

	/**
	*@brief  get input address
	*/
	std::string getHelperInputAddress()const;
	/**
	*@brief  get output address
	*/
	std::string getHelperOutputAddress(const int index)const;

	/**
	*@brief  get input name
	*/
	std::string getHelperInputName(int index) const;

	/**
	*@brief  get input name
	*/
	std::string getHelperOutputName(int index) const;
	/**
	*@brief  get error code
	*/
	std::shared_ptr<DefinedError> getErrorCode(const int code_type, const int code_id);

protected:
	/**
	*@brief  config protocol
	*/
	void configKeyencePlc(const std::shared_ptr<KernelConfiguration> & config);

	/**
	*@brief  enable protocol
	*/
	bool enableProtocol();


	/**
	*@brief  disable protocol
	*/
	void disableProtocol();
	

private:
	DECLARE_PRIVATE(KeyencePlcSubSystemHelper);
};



KERNEL_NS_END

#endif
