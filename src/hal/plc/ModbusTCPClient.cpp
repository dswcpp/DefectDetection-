#include "ModbusTCPClient.h"

bool ModbusTCPClient::connect(const QString &host, int port) {
  Q_UNUSED(host);
  Q_UNUSED(port);
  // TODO: 建立 Modbus TCP 连接
  return true;
}

void ModbusTCPClient::disconnect() {
  // TODO: 断开连接，释放资源
}

bool ModbusTCPClient::readHoldingRegisters(int addr, int count, std::vector<uint16_t> &out) {
  Q_UNUSED(addr);
  Q_UNUSED(count);
  Q_UNUSED(out);
  // TODO: 读取寄存器
  return true;
}

bool ModbusTCPClient::writeHoldingRegister(int addr, uint16_t value) {
  Q_UNUSED(addr);
  Q_UNUSED(value);
  // TODO: 写寄存器
  return true;
}

bool ModbusTCPClient::heartbeat() {
  // TODO: 发送心跳/读写测试寄存器
  return true;
}
