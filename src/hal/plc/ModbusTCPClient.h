/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * ModbusTCPClient.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：Modbus TCP客户端接口定义
 * 描述：Modbus TCP协议实现，支持线圈、保持寄存器读写
 *
 * 当前版本：1.0
 */

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
