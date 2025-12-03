/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * WSServer.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：WebSocket服务器接口定义
 * 描述：WebSocket服务器，用于实时推送检测结果到Web客户端，
 *       支持多客户端连接、消息广播
 *
 * 当前版本：1.0
 */

#ifndef WSSERVER_H
#define WSSERVER_H

#include <QObject>

class WSServer : public QObject {
  Q_OBJECT
public:
  explicit WSServer(QObject *parent = nullptr);

signals:
};

#endif // WSSERVER_H
