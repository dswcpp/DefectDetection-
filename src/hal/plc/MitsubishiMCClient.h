/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * MitsubishiMCClient.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：三菱MC协议客户端接口定义
 * 描述：三菱MC协议实现，支持Q/L/FX系列PLC的数据读写
 *
 * 当前版本：1.0
 */

#ifndef MITSUBISHIMCCLIENT_H
#define MITSUBISHIMCCLIENT_H

#include "IPLCClient.h"

class MitsubishiMCClient : public IPLCClient {
public:
  MitsubishiMCClient() = default;

  bool connect(const QString &host, int port) override;
  void disconnect() override;
  bool readHoldingRegisters(int addr, int count, std::vector<uint16_t> &out) override;
  bool writeHoldingRegister(int addr, uint16_t value) override;
  bool heartbeat() override;
};

#endif // MITSUBISHIMCCLIENT_H
