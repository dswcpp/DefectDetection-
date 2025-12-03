/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * SiemensS7Client.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：西门子S7 PLC客户端接口定义
 * 描述：西门子S7协议实现，支持S7-200/300/400/1200/1500系列PLC
 *
 * 当前版本：1.0
 */

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
