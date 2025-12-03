/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * IPLCClient.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：PLC客户端接口基类定义
 * 描述：PLC通信抽象接口，定义连接、读写寄存器、位操作等方法
 *
 * 当前版本：1.0
 */

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
