// Library: Fortrend
// Package: Subsystem
//
// Created by chenjj on 2023/10/17.
//

#include "modbus_master.h"
#include "modbus/modbus.h"
#include "Kernel/kernel_log.h"

KERNEL_NS_BEGIN
bool ModbusMaster::init(
	int server_id,
	const std::string& com_port,
	int baud_rate,
	char parity,
	int data_byte,
	int stop_bit) {
	ctx_ = modbus_new_rtu(com_port.c_str(), baud_rate, parity, data_byte, stop_bit);
	if(nullptr == ctx_){
		logError("Modbus", "new rtu failed:%s\n", modbus_strerror(errno));

		return false;
	}
	auto ret = modbus_set_slave(ctx_, server_id);
	if(-1 == ret){
		logError("Modbus", "set slave failed:%s\n", modbus_strerror(errno));
		modbus_free(ctx_);
		return false;
	}
//	modbus_set_debug(ctx_, true);
	if(-1 == modbus_connect(ctx_)){
		logError("Modbus", "Connection failed:%s\n", modbus_strerror(errno));
		modbus_free(ctx_);
		return false;
	}

	return true;
}

void ModbusMaster::close() {
	modbus_close(ctx_);
	modbus_free(ctx_);
	ctx_ = nullptr;
}

bool ModbusMaster::readRegister(int addr, uint16_t* data) {
	auto ret = modbus_read_registers(ctx_, addr, 1, data);
	if(-1 == ret){
		logError("Modbus", "read register failed:%s, addr:%d\n", modbus_strerror(errno), addr);
		return false;
	}

	return true;
}

bool ModbusMaster::writeRegister(int addr, uint16_t data) {
	auto ret = modbus_write_register(ctx_, addr, data);
	if(-1 == ret){
		logError("Modbus", "write register failed:%s, addr:%d\n", modbus_strerror(errno), addr);
		return false;
	}
	return true;
	/*uint16_t read_data = 0;
	readRegister(addr, &read_data);
	return data == read_data;*/
}
KERNEL_NS_END