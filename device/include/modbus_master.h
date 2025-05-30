// Library: Fortrend
// Package: Subsystem
//
// Created by chenjj on 2023/10/17.
//

#ifndef EFEM_DEVICE_SRC_UI_MODBUS_MASTER_H_
#define EFEM_DEVICE_SRC_UI_MODBUS_MASTER_H_
#include "modbus/modbus.h"
#include "vector"
#include "Kernel/kernel_macros.h"

KERNEL_NS_BEGIN
class ModbusMaster {
public:
	bool init(
		int server_id,
		const std::string& com_port,
		int baud_rate,
		char parity = 'N',
		int data_byte = 8,
		int stop_bit = 1);

	void close();

	bool readRegister(int addr, uint16_t* data);
	bool writeRegister(int addr, uint16_t data);

private:
	modbus_t* ctx_{nullptr};
};
KERNEL_NS_END
#endif //EFEM_DEVICE_SRC_UI_FFU_MODBUS_MASTER_H_
