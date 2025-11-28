#ifndef MODBUSTCPCLIENT_H
#define MODBUSTCPCLIENT_H

#include "IPLCClient.h"

class ModbusTCPClient : public IPLCClient {
public:
  ModbusTCPClient() = default;

  bool connect(const QString &host, int port) override;
  void disconnect() override;
  bool readHoldingRegisters(int addr, int count, std::vector<uint16_t> &out) override;
  bool writeHoldingRegister(int addr, uint16_t value) override;
  bool heartbeat() override;
};

#endif // MODBUSTCPCLIENT_H
