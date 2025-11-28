#include "MitsubishiMCClient.h"

bool MitsubishiMCClient::connect(const QString &host, int port) {
  Q_UNUSED(host);
  Q_UNUSED(port);
  // TODO: 建立三菱 MC 通信连接
  return true;
}

void MitsubishiMCClient::disconnect() {
  // TODO: 断开连接
}

bool MitsubishiMCClient::readHoldingRegisters(int addr, int count, std::vector<uint16_t> &out) {
  Q_UNUSED(addr);
  Q_UNUSED(count);
  Q_UNUSED(out);
  // TODO: 读取寄存器
  return true;
}

bool MitsubishiMCClient::writeHoldingRegister(int addr, uint16_t value) {
  Q_UNUSED(addr);
  Q_UNUSED(value);
  // TODO: 写寄存器
  return true;
}

bool MitsubishiMCClient::heartbeat() {
  // TODO: MC 心跳
  return true;
}
