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
 *       支持多客户端连接、消息广播、心跳检测
 *
 * 当前版本：1.1
 * 更新：完整实现 WebSocket 服务器功能
 */

#ifndef WSSERVER_H
#define WSSERVER_H

#include "../network_global.h"
#include <QObject>
#include <QWebSocketServer>
#include <QWebSocket>
#include <QTimer>
#include <QJsonObject>
#include <QMap>
#include <QSet>

/**
 * @brief WebSocket 客户端信息
 */
struct NETWORK_LIBRARY WSClientInfo {
  QString id;                    // 客户端唯一ID
  QString remoteAddress;         // 远程地址
  quint16 remotePort;            // 远程端口
  QDateTime connectedAt;         // 连接时间
  QDateTime lastHeartbeat;       // 最后心跳时间
  QSet<QString> subscribedTopics; // 订阅的主题
};

/**
 * @brief WebSocket 消息类型
 */
namespace WSMessageType {
  const QString HEARTBEAT = "heartbeat";
  const QString SUBSCRIBE = "subscribe";
  const QString UNSUBSCRIBE = "unsubscribe";
  const QString DETECTION_RESULT = "detection_result";
  const QString SYSTEM_STATUS = "system_status";
  const QString ALARM = "alarm";
  const QString STATISTICS = "statistics";
}

/**
 * @brief WebSocket 服务器
 *
 * 提供实时通信功能：
 * - 多客户端连接管理
 * - 主题订阅/发布模式
 * - 心跳检测
 * - 消息广播
 */
class NETWORK_LIBRARY WSServer : public QObject {
  Q_OBJECT
public:
  explicit WSServer(QObject *parent = nullptr);
  ~WSServer() override;

  /**
   * @brief 启动服务器
   * @param port 监听端口
   * @param serverName 服务器名称
   */
  bool start(quint16 port = 8080, const QString& serverName = "DefectDetection");

  /**
   * @brief 停止服务器
   */
  void stop();

  /**
   * @brief 服务器是否运行中
   */
  bool isRunning() const;

  /**
   * @brief 获取监听端口
   */
  quint16 port() const;

  /**
   * @brief 获取连接的客户端数量
   */
  int clientCount() const;

  /**
   * @brief 获取所有客户端信息
   */
  QList<WSClientInfo> clients() const;

  // ============ 消息发送 ============

  /**
   * @brief 广播消息给所有客户端
   * @param type 消息类型
   * @param data 消息数据
   */
  void broadcast(const QString& type, const QJsonObject& data);

  /**
   * @brief 发布消息到指定主题
   * @param topic 主题名称
   * @param type 消息类型
   * @param data 消息数据
   */
  void publish(const QString& topic, const QString& type, const QJsonObject& data);

  /**
   * @brief 发送消息给指定客户端
   * @param clientId 客户端ID
   * @param type 消息类型
   * @param data 消息数据
   */
  void sendTo(const QString& clientId, const QString& type, const QJsonObject& data);

  // ============ 便捷方法 ============

  /**
   * @brief 推送检测结果
   */
  void pushDetectionResult(const QJsonObject& result);

  /**
   * @brief 推送系统状态
   */
  void pushSystemStatus(const QJsonObject& status);

  /**
   * @brief 推送报警信息
   */
  void pushAlarm(const QString& level, const QString& message, const QJsonObject& details = {});

  /**
   * @brief 推送统计数据
   */
  void pushStatistics(const QJsonObject& stats);

signals:
  /**
   * @brief 服务器启动
   */
  void started(quint16 port);

  /**
   * @brief 服务器停止
   */
  void stopped();

  /**
   * @brief 客户端连接
   */
  void clientConnected(const QString& clientId, const QString& address);

  /**
   * @brief 客户端断开
   */
  void clientDisconnected(const QString& clientId);

  /**
   * @brief 收到客户端消息
   */
  void messageReceived(const QString& clientId, const QString& type, const QJsonObject& data);

  /**
   * @brief 错误发生
   */
  void errorOccurred(const QString& error);

private slots:
  void onNewConnection();
  void onTextMessageReceived(const QString& message);
  void onBinaryMessageReceived(const QByteArray& message);
  void onClientDisconnected();
  void onSslErrors(const QList<QSslError>& errors);
  void onHeartbeatCheck();

private:
  void processMessage(QWebSocket* client, const QJsonObject& msg);
  void handleSubscribe(QWebSocket* client, const QJsonArray& topics);
  void handleUnsubscribe(QWebSocket* client, const QJsonArray& topics);
  void sendMessage(QWebSocket* client, const QString& type, const QJsonObject& data);
  QString generateClientId();
  void removeClient(QWebSocket* client);

  QWebSocketServer* m_server = nullptr;
  QMap<QWebSocket*, WSClientInfo> m_clients;
  QMap<QString, QWebSocket*> m_clientById;
  QMap<QString, QSet<QWebSocket*>> m_topicSubscribers;
  QTimer* m_heartbeatTimer = nullptr;

  static constexpr int HEARTBEAT_INTERVAL_MS = 30000;  // 心跳间隔
  static constexpr int HEARTBEAT_TIMEOUT_MS = 90000;   // 心跳超时
};

#endif // WSSERVER_H
