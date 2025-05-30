/**
* @file            kenyence_plc_command_executer.h
* @brief           kenyence plc command executer for fortrend
* @author		   kai
*/

// Library: Fortrend
// Package: Subsystem/KeyencePLC



#ifndef _XLH_FORTREND_KEYENCE_PLC_COMMAND_INCLUDE_
#define _XLH_FORTREND_KEYENCE_PLC_COMMAND_INCLUDE_
#include "kernel/Fortrend/fortrend_macros.h"
#include "Kernel/kernel_macros.h"
#include "Kernel/kernel_exception.h"
#include "Kernel/kernel_subsystem_command.h"

#include "KeyencePLC/keyence_plc_subsystem_helper.h"

KERNEL_NS_BEGIN


/**
*@brief  sunway command executer
*/
class KeyencePlcCommandExecuter
{
public:
	DECLARE_PTR(KeyencePlcCommandExecuter)
public:
	explicit KeyencePlcCommandExecuter();
	explicit KeyencePlcCommandExecuter(KeyencePlcSubSystemHelper* helper);

	/**
	*@brief   set helper for excuter
	*/
	void  setKeyencePlcHelper(KeyencePlcCommandExecuter* helper);

protected:
	/**
	* @brief return true if success else false.
	*/
	virtual void onStop() throw(KernelException);

protected:
	bool readBit(const std::string address, bool &value) throw(KernelException);
	bool readBits(const std::string address, const uint16_t length, bool *value) throw(KernelException);
	bool readShort(const std::string address, int16_t &value) throw(KernelException);
	bool readShorts(const std::string address, const uint16_t length, int16_t *value) throw(KernelException);
	bool readUnsignedShort(const std::string address, uint16_t &value) throw(KernelException);
	bool readUnsignedShorts(const std::string address, const uint16_t length, uint16_t *value) throw(KernelException);
	bool readInt(const std::string address, int32_t &value) throw(KernelException);
	bool readInts(const std::string address, const uint16_t length, int32_t *value) throw(KernelException);
	bool readUnsignedInt(const std::string address, uint32_t &value) throw(KernelException);
	bool readUnsignedInts(const std::string address, const uint16_t length, uint32_t *value) throw(KernelException);
	bool readFloat(const std::string address, float &value) throw(KernelException);
	bool readFloats(const std::string address, const uint16_t length, float *value) throw(KernelException);
	bool readDouble(const std::string address, double &value) throw(KernelException);
	bool readDoubles(const std::string address, const uint16_t length, double *value) throw(KernelException);
	bool readString(const std::string address, const uint16_t length, std::string &value) throw(KernelException);

	bool writeBit(const std::string address, const bool value) throw(KernelException);
	bool writeBits(const std::string address, const uint16_t length, const bool *values) throw(KernelException);
	bool writeShort(const std::string address, const int16_t value) throw(KernelException);
	bool writeShorts(const std::string address, const uint16_t length, const int16_t *values) throw(KernelException);
	bool writeUnsignedShort(const std::string address, const uint16_t value) throw(KernelException);
	bool writeUnsignedShorts(const std::string address, const uint16_t length, const uint16_t *values) throw(KernelException);
	bool writeInt(const std::string address, const int32_t value) throw(KernelException);
	bool writeInts(const std::string address, const uint16_t length, const int32_t *values) throw(KernelException);
	bool writeUnsignedInt(const std::string address, const uint32_t value) throw(KernelException);
	bool writeUnsignedInts(const std::string address, const uint16_t length, const uint32_t *values) throw(KernelException);
	bool writeFloat(const std::string address, const float value) throw(KernelException);
	bool writeFloats(const std::string address, const uint16_t length, const float *values) throw(KernelException);
	bool writeDouble(const std::string address, const double value) throw(KernelException);
	bool writeDoubles(const std::string address, const uint16_t length, const double *values)throw(KernelException);
	bool writeString(const std::string address, const std::string value) throw(KernelException);

	std::shared_ptr<KeyencePlcSubSystemHelper::DefinedError> getErrorCode(const int code_type, const int code_id);
private:
	DECLARE_PRIVATE(KeyencePlcCommandExecuter)
};

KERNEL_NS_END

#endif
