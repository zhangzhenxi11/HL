/**
* @file            inovance_plc_command_executer.h
* @brief           inovance plc command executer for fortrend
* @author			kai
*/

// Library: Fortrend
// Package: Subsystem/InovancePLC

#include "InovancePLC/inovance_plc_command_executer.h"
#include "InovancePLC/inovance_plc_subsystem_helper.h"
#include "Poco/Format.h"

#include <thread>
#include <sstream>
#include <chrono>

KERNEL_NS_BEGIN

/**
* InovancePlcCommandExecuterPrivate
*/
class InovancePlcCommandExecuterPrivate{
public:
	InovancePlcSubSystemHelper* helper;
};

/**
* InovancePlcCommandExecuter
*/
InovancePlcCommandExecuter::InovancePlcCommandExecuter(InovancePlcSubSystemHelper* helper)
	:d(new InovancePlcCommandExecuterPrivate){
	d->helper = helper;
}


/**
* return true if success else false.
*/
void InovancePlcCommandExecuter::onStop() throw(KernelException){
	throw KernelSysException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, "can not stop.");
}


bool InovancePlcCommandExecuter::readBit(const std::string address, bool &value) throw(KernelException){ return d->helper->readBit(address, value); }
bool InovancePlcCommandExecuter::readBits(const std::string address, const uint16_t length, bool *value) throw(KernelException){ return d->helper->readBits(address, length, value); }
bool InovancePlcCommandExecuter::readShort(const std::string address, int16_t &value) throw(KernelException){ return d->helper->readShort(address, value); }
bool InovancePlcCommandExecuter::readShorts(const std::string address, const uint16_t length, int16_t *value) throw(KernelException){ return d->helper->readShorts(address, length, value); }
bool InovancePlcCommandExecuter::readUnsignedShort(const std::string address, uint16_t &value) throw(KernelException){ return d->helper->readUnsignedShort(address, value); }
bool InovancePlcCommandExecuter::readUnsignedShorts(const std::string address, const uint16_t length, uint16_t *value) throw(KernelException){ return d->helper->readUnsignedShorts(address, length, value); }
bool InovancePlcCommandExecuter::readInt(const std::string address, int32_t &value) throw(KernelException){ return d->helper->readInt(address, value); }
bool InovancePlcCommandExecuter::readInts(const std::string address, const uint16_t length, int32_t *value) throw(KernelException){ return d->helper->readInts(address, length, value); }
bool InovancePlcCommandExecuter::readUnsignedInt(const std::string address, uint32_t &value) throw(KernelException){ return d->helper->readUnsignedInt(address, value); }
bool InovancePlcCommandExecuter::readUnsignedInts(const std::string address, const uint16_t length, uint32_t *value) throw(KernelException){ return d->helper->readUnsignedInts(address, length, value); }
bool InovancePlcCommandExecuter::readFloat(const std::string address, float &value) throw(KernelException){ return d->helper->readFloat(address, value); }
bool InovancePlcCommandExecuter::readFloats(const std::string address, const uint16_t length, float *value) throw(KernelException){ return d->helper->readFloats(address, length, value); }
bool InovancePlcCommandExecuter::readDouble(const std::string address, double &value) throw(KernelException){ return d->helper->readDouble(address, value); }
bool InovancePlcCommandExecuter::readDoubles(const std::string address, const uint16_t length, double *value) throw(KernelException){ return d->helper->readDoubles(address, length, value); }
bool InovancePlcCommandExecuter::readString(const std::string address, const uint16_t length, std::string &value) throw(KernelException){ return d->helper->readString(address, length, value); }

bool InovancePlcCommandExecuter::writeBit(const std::string address, const bool value) throw(KernelException){ return d->helper->writeBit(address, value); }
bool InovancePlcCommandExecuter::writeBits(const std::string address, const uint16_t length, const bool *values) throw(KernelException){ return d->helper->writeBits(address, length, values); }
bool InovancePlcCommandExecuter::writeShort(const std::string address, const int16_t value) throw(KernelException){ return d->helper->writeShort(address, value); }
bool InovancePlcCommandExecuter::writeShorts(const std::string address, const uint16_t length, const int16_t *values) throw(KernelException){ return d->helper->writeShorts(address, length, values); }
bool InovancePlcCommandExecuter::writeUnsignedShort(const std::string address, const uint16_t value) throw(KernelException){ return d->helper->writeUnsignedShort(address, value); }
bool InovancePlcCommandExecuter::writeUnsignedShorts(const std::string address, const uint16_t length, const uint16_t *values) throw(KernelException){ return d->helper->writeUnsignedShorts(address, length, values); }
bool InovancePlcCommandExecuter::writeInt(const std::string address, const int32_t value) throw(KernelException){ return d->helper->writeInt(address, value); }
bool InovancePlcCommandExecuter::writeInts(const std::string address, const uint16_t length, const int32_t *values) throw(KernelException){ return d->helper->writeInts(address, length, values); }
bool InovancePlcCommandExecuter::writeUnsignedInt(const std::string address, const uint32_t value) throw(KernelException){ return d->helper->writeUnsignedInt(address, value); }
bool InovancePlcCommandExecuter::writeUnsignedInts(const std::string address, const uint16_t length, const uint32_t *values) throw(KernelException){ return d->helper->writeUnsignedInts(address, length, values); }
bool InovancePlcCommandExecuter::writeFloat(const std::string address, const float value) throw(KernelException){ return d->helper->writeFloat(address, value); }
bool InovancePlcCommandExecuter::writeFloats(const std::string address, const uint16_t length, const float *values) throw(KernelException){ return d->helper->writeFloats(address, length, values); }
bool InovancePlcCommandExecuter::writeDouble(const std::string address, const double value) throw(KernelException){ return d->helper->writeDouble(address, value); }
bool InovancePlcCommandExecuter::writeDoubles(const std::string address, const uint16_t length, const double *values)throw(KernelException){ return d->helper->writeDoubles(address, length, values); }
bool InovancePlcCommandExecuter::writeString(const std::string address, const std::string value) throw(KernelException){ return d->helper->writeString(address, value); }

std::shared_ptr<InovancePlcSubSystemHelper::DefinedError> InovancePlcCommandExecuter::getErrorCode(const int code_type, const int code_id){ return d->helper->getErrorCode(code_type, code_id); }


KERNEL_NS_END
