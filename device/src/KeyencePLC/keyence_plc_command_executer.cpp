/**
* @file            keyence_plc_command_executer.h
* @brief           keyence plc command executer for fortrend
* @author			kai
*/

// Library: Fortrend
// Package: Subsystem/KeyencePLC

#include "KeyencePLC/keyence_plc_command_executer.h"
#include "KeyencePLC/keyence_plc_subsystem_helper.h"
#include "Poco/Format.h"

#include <thread>
#include <sstream>
#include <chrono>

KERNEL_NS_BEGIN

/**
* KeyencePlcCommandExecuterPrivate
*/
class KeyencePlcCommandExecuterPrivate{
public:
	KeyencePlcSubSystemHelper* helper;
};

/**
* KeyencePlcCommandExecuter
*/
KeyencePlcCommandExecuter::KeyencePlcCommandExecuter(KeyencePlcSubSystemHelper* helper)
	:d(new KeyencePlcCommandExecuterPrivate){
	d->helper = helper;
}


/**
* return true if success else false.
*/
void KeyencePlcCommandExecuter::onStop() throw(KernelException){
	throw KernelSysException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, "can not stop.");
}


bool KeyencePlcCommandExecuter::readBit(const std::string address, bool &value) throw(KernelException){ return d->helper->readBit(address, value); }
bool KeyencePlcCommandExecuter::readBits(const std::string address, const uint16_t length, bool *value) throw(KernelException){ return d->helper->readBits(address, length, value); }
bool KeyencePlcCommandExecuter::readShort(const std::string address, int16_t &value) throw(KernelException){ return d->helper->readShort(address, value); }
bool KeyencePlcCommandExecuter::readShorts(const std::string address, const uint16_t length, int16_t *value) throw(KernelException){ return d->helper->readShorts(address, length, value); }
bool KeyencePlcCommandExecuter::readUnsignedShort(const std::string address, uint16_t &value) throw(KernelException){ return d->helper->readUnsignedShort(address, value); }
bool KeyencePlcCommandExecuter::readUnsignedShorts(const std::string address, const uint16_t length, uint16_t *value) throw(KernelException){ return d->helper->readUnsignedShorts(address, length, value); }
bool KeyencePlcCommandExecuter::readInt(const std::string address, int32_t &value) throw(KernelException){ return d->helper->readInt(address, value); }
bool KeyencePlcCommandExecuter::readInts(const std::string address, const uint16_t length, int32_t *value) throw(KernelException){ return d->helper->readInts(address, length, value); }
bool KeyencePlcCommandExecuter::readUnsignedInt(const std::string address, uint32_t &value) throw(KernelException){ return d->helper->readUnsignedInt(address, value); }
bool KeyencePlcCommandExecuter::readUnsignedInts(const std::string address, const uint16_t length, uint32_t *value) throw(KernelException){ return d->helper->readUnsignedInts(address, length, value); }
bool KeyencePlcCommandExecuter::readFloat(const std::string address, float &value) throw(KernelException){ return d->helper->readFloat(address, value); }
bool KeyencePlcCommandExecuter::readFloats(const std::string address, const uint16_t length, float *value) throw(KernelException){ return d->helper->readFloats(address, length, value); }
bool KeyencePlcCommandExecuter::readDouble(const std::string address, double &value) throw(KernelException){ return d->helper->readDouble(address, value); }
bool KeyencePlcCommandExecuter::readDoubles(const std::string address, const uint16_t length, double *value) throw(KernelException){ return d->helper->readDoubles(address, length, value); }
bool KeyencePlcCommandExecuter::readString(const std::string address, const uint16_t length, std::string &value) throw(KernelException){ return d->helper->readString(address, length, value); }

bool KeyencePlcCommandExecuter::writeBit(const std::string address, const bool value) throw(KernelException){ return d->helper->writeBit(address, value); }
bool KeyencePlcCommandExecuter::writeBits(const std::string address, const uint16_t length, const bool *values) throw(KernelException){ return d->helper->writeBits(address, length, values); }
bool KeyencePlcCommandExecuter::writeShort(const std::string address, const int16_t value) throw(KernelException){ return d->helper->writeShort(address, value); }
bool KeyencePlcCommandExecuter::writeShorts(const std::string address, const uint16_t length, const int16_t *values) throw(KernelException){ return d->helper->writeShorts(address, length, values); }
bool KeyencePlcCommandExecuter::writeUnsignedShort(const std::string address, const uint16_t value) throw(KernelException){ return d->helper->writeUnsignedShort(address, value); }
bool KeyencePlcCommandExecuter::writeUnsignedShorts(const std::string address, const uint16_t length, const uint16_t *values) throw(KernelException){ return d->helper->writeUnsignedShorts(address, length, values); }
bool KeyencePlcCommandExecuter::writeInt(const std::string address, const int32_t value) throw(KernelException){ return d->helper->writeInt(address, value); }
bool KeyencePlcCommandExecuter::writeInts(const std::string address, const uint16_t length, const int32_t *values) throw(KernelException){ return d->helper->writeInts(address, length, values); }
bool KeyencePlcCommandExecuter::writeUnsignedInt(const std::string address, const uint32_t value) throw(KernelException){ return d->helper->writeUnsignedInt(address, value); }
bool KeyencePlcCommandExecuter::writeUnsignedInts(const std::string address, const uint16_t length, const uint32_t *values) throw(KernelException){ return d->helper->writeUnsignedInts(address, length, values); }
bool KeyencePlcCommandExecuter::writeFloat(const std::string address, const float value) throw(KernelException){ return d->helper->writeFloat(address, value); }
bool KeyencePlcCommandExecuter::writeFloats(const std::string address, const uint16_t length, const float *values) throw(KernelException){ return d->helper->writeFloats(address, length, values); }
bool KeyencePlcCommandExecuter::writeDouble(const std::string address, const double value) throw(KernelException){ return d->helper->writeDouble(address, value); }
bool KeyencePlcCommandExecuter::writeDoubles(const std::string address, const uint16_t length, const double *values)throw(KernelException){ return d->helper->writeDoubles(address, length, values); }
bool KeyencePlcCommandExecuter::writeString(const std::string address, const std::string value) throw(KernelException){ return d->helper->writeString(address, value); }

std::shared_ptr<KeyencePlcSubSystemHelper::DefinedError> KeyencePlcCommandExecuter::getErrorCode(const int code_type, const int code_id){ return d->helper->getErrorCode(code_type, code_id); }


KERNEL_NS_END
