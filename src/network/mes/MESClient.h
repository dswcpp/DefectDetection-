/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * MESClient.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：MES客户端接口定义
 * 描述：制造执行系统客户端，负责与MES服务器通信，
 *       上报检测结果、获取工单信息等
 *
 * 当前版本：1.1
 * 更新：完整实现 MES 客户端功能
 */

#ifndef MESCLIENT_H
#define MESCLIENT_H

#include "../network_global.h"
#include "MESProtocol.h"
#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QQueue>
#include <QMutex>

/**
 * @brief MES 连接配置
 */
struct NETWORK_LIBRARY MESConfig {
  QString host = "127.0.0.1";    // MES 服务器地址
  quint16 port = 8888;           // MES 服务器端口
  QString deviceId;              // 设备ID
  int connectTimeoutMs = 5000;   // 连接超时
  int heartbeatIntervalMs = 10000; // 心跳间隔
  int reconnectIntervalMs = 5000;  // 重连间隔
  bool autoReconnect = true;     // 自动重连
  int maxRetries = 3;            // 最大重试次数
};

/**
 * @brief MES 客户端
 *
 * 功能：
 * - TCP 连接管理（自动重连）
 * - 心跳保活
 * - 消息队列（离线缓存）
 * - 工单获取
 * - 检测结果上报
 * - 报警上报
 */
class NETWORK_LIBRARY MESClient : public QObject {
  Q_OBJECT
public:
  /**
   * @brief 连接状态
   */
  enum class ConnectionState {
    Disconnected,      // 未连接
    Connecting,        // 连接中
    Connected,         // 已连接
    Reconnecting       // 重连中
  };
  Q_ENUM(ConnectionState)

  explicit MESClient(QObject* parent = nullptr);
  ~MESClient() override;

  /**
   * @brief 设置配置
   */
  void setConfig(const MESConfig& config);
  MESConfig config() const { return m_config; }

  /**
   * @brief 连接到 MES 服务器
   */
  void connectToServer();

  /**
   * @brief 断开连接
   */
  void disconnectFromServer();

  /**
   * @brief 获取连接状态
   */
  ConnectionState connectionState() const { return m_state; }

  /**
   * @brief 是否已连接
   */
  bool isConnected() const { return m_state == ConnectionState::Connected; }

  // ============ 业务接口 ============

  /**
   * @brief 获取当前工单
   * @param workOrderId 工单号（可选，为空则获取当前活动工单）
   */
  void requestWorkOrder(const QString& workOrderId = QString());

  /**
   * @brief 上报检测结果
   * @param report 检测结果
   */
  void reportInspectionResult(const MESInspectionReport& report);

  /**
   * @brief 上报设备状态
   * @param status 设备状态
   */
  void reportDeviceStatus(const MESDeviceStatus& status);

  /**
   * @brief 上报报警
   * @param alarm 报警信息
   */
  void reportAlarm(const MESAlarm& alarm);

  /**
   * @brief 获取待发送消息数量
   */
  int pendingMessageCount() const;

signals:
  /**
   * @brief 连接状态改变
   */
  void connectionStateChanged(ConnectionState state);

  /**
   * @brief 已连接
   */
  void connected();

  /**
   * @brief 已断开
   */
  void disconnected();

  /**
   * @brief 收到工单数据
   */
  void workOrderReceived(const MESWorkOrder& workOrder);

  /**
   * @brief 消息发送成功
   */
  void messageSent(const QString& messageId);

  /**
   * @brief 消息发送失败
   */
  void messageFailed(const QString& messageId, const QString& error);

  /**
   * @brief 收到响应
   */
  void responseReceived(const MESResponse& response);

  /**
   * @brief 错误发生
   */
  void errorOccurred(const QString& error);

private slots:
  void onSocketConnected();
  void onSocketDisconnected();
  void onSocketError(QAbstractSocket::SocketError error);
  void onSocketReadyRead();
  void onHeartbeatTimer();
  void onReconnectTimer();

private:
  void setState(ConnectionState state);
  void sendMessage(const MESMessage& msg);
  void processReceivedData();
  void handleResponse(const MESResponse& response);
  QString generateMessageId();
  void flushPendingMessages();
  void scheduleReconnect();

  MESConfig m_config;
  QTcpSocket* m_socket = nullptr;
  ConnectionState m_state = ConnectionState::Disconnected;

  QTimer* m_heartbeatTimer = nullptr;
  QTimer* m_reconnectTimer = nullptr;

  QByteArray m_receiveBuffer;
  QQueue<MESMessage> m_pendingMessages;
  QMutex m_queueMutex;

  int m_reconnectAttempts = 0;
  qint64 m_messageCounter = 0;

  static constexpr int MAX_PENDING_MESSAGES = 1000;
};

#endif // MESCLIENT_H
