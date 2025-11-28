#ifndef IPLCCLIENT_H
#define IPLCCLIENT_H

#include <QString>
#include <vector>

class IPLCClient {
public:
  virtual ~IPLCClient() = default;

  virtual bool connect(const QString &host, int port) = 0;
  virtual void disconnect() = 0;

  // 读写保持寄存器
  virtual bool readHoldingRegisters(int addr, int count, std::vector<uint16_t> &out) = 0;
  virtual bool writeHoldingRegister(int addr, uint16_t value) = 0;

  // 心跳
  virtual bool heartbeat() = 0;
};

#endif // IPLCCLIENT_H
