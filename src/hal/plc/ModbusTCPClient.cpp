#include "ModbusTCPClient.h"
#include "common/Logger.h"

bool ModbusTCPClient::connect(const QString &host, int port) {
  LOG_INFO("ModbusTCPClient::connect - Connecting to {}:{}", host.toStdString(), port);
  // TODO: 建立 Modbus TCP 连接
  LOG_WARN("ModbusTCPClient::connect - Not implemented yet");
  return true;
}

void ModbusTCPClient::disconnect() {
  LOG_INFO("ModbusTCPClient::disconnect - Disconnecting");
  // TODO: 断开连接，释放资源
}

bool ModbusTCPClient::readHoldingRegisters(int addr, int count, std::vector<uint16_t> &out) {
  LOG_DEBUG("ModbusTCPClient::readHoldingRegisters - addr={}, count={}", addr, count);
  // TODO: 读取寄存器
  out.clear();
  return false;
}

bool ModbusTCPClient::writeHoldingRegister(int addr, uint16_t value) {
  LOG_DEBUG("ModbusTCPClient::writeHoldingRegister - addr={}, value={}", addr, value);
  // TODO: 写寄存器
  return false;
}

bool ModbusTCPClient::heartbeat() {
  // TODO: 发送心跳/读写测试寄存器
  return true;
}
