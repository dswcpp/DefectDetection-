#include "ModbusRTUClient.h"

bool ModbusRTUClient::connect(const QString &host, int port) {
  Q_UNUSED(host);
  Q_UNUSED(port);
  // TODO: 打开串口，设置波特率/校验，建立 Modbus RTU
  return true;
}

void ModbusRTUClient::disconnect() {
  // TODO: 关闭串口
}

bool ModbusRTUClient::readHoldingRegisters(int addr, int count, std::vector<uint16_t> &out) {
  Q_UNUSED(addr);
  Q_UNUSED(count);
  Q_UNUSED(out);
  // TODO: RTU 读寄存器
  return true;
}

bool ModbusRTUClient::writeHoldingRegister(int addr, uint16_t value) {
  Q_UNUSED(addr);
  Q_UNUSED(value);
  // TODO: RTU 写寄存器
  return true;
}

bool ModbusRTUClient::heartbeat() {
  // TODO: 发送心跳读写
  return true;
}
