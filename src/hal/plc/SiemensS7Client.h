#ifndef SIEMENSS7CLIENT_H
#define SIEMENSS7CLIENT_H

#include "IPLCClient.h"

class SiemensS7Client : public IPLCClient {
public:
  SiemensS7Client() = default;

  bool connect(const QString &host, int port) override;
  void disconnect() override;
  bool readHoldingRegisters(int addr, int count, std::vector<uint16_t> &out) override;
  bool writeHoldingRegister(int addr, uint16_t value) override;
  bool heartbeat() override;
};

#endif // SIEMENSS7CLIENT_H
