/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * ModbusRTUClient.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：Modbus RTU客户端接口定义
 * 描述：Modbus RTU协议实现，通过串口与PLC通信
 *
 * 当前版本：1.0
 */

#ifndef MODBUSRTUCLIENT_H
#define MODBUSRTUCLIENT_H

#include "IPLCClient.h"

class ModbusRTUClient : public IPLCClient {
public:
  ModbusRTUClient() = default;

  bool connect(const QString &host, int port) override;
  void disconnect() override;
  bool readHoldingRegisters(int addr, int count, std::vector<uint16_t> &out) override;
  bool writeHoldingRegister(int addr, uint16_t value) override;
  bool heartbeat() override;
};

#endif // MODBUSRTUCLIENT_H
