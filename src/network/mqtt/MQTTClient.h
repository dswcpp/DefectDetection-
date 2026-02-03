/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * MQTTClient.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：MQTT客户端接口定义
 * 描述：MQTT消息客户端，用于设备间通信、数据上报、
 *       远程控制指令接收等场景
 *
 * 当前版本：1.0
 */

#ifndef MQTTCLIENT_H
#define MQTTCLIENT_H

#include "../network_global.h"
#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QMap>
#include <QQueue>
#include <QMutex>

/**
 * @brief MQTT 连接配置
 */
struct NETWORK_LIBRARY MQTTConfig {
  QString host = "127.0.0.1";       // Broker 地址
  quint16 port = 1883;              // Broker 端口
  QString clientId;                  // 客户端ID
  QString username;                  // 用户名（可选）
  QString password;                  // 密码（可选）
  int keepAliveSeconds = 60;         // 心跳间隔（秒）
  bool cleanSession = true;          // 清除会话
  int connectTimeoutMs = 5000;       // 连接超时
  int reconnectIntervalMs = 5000;    // 重连间隔
  bool autoReconnect = true;         // 自动重连

  // 遗嘱消息（可选）
  QString willTopic;
  QByteArray willMessage;
  int willQos = 0;
  bool willRetain = false;
};

/**
 * @brief MQTT QoS 等级
 */
enum class MQTTQoS {
  AtMostOnce = 0,   // QoS 0: 最多一次
  AtLeastOnce = 1,  // QoS 1: 至少一次
  ExactlyOnce = 2   // QoS 2: 恰好一次
};

/**
 * @brief MQTT 消息
 */
struct NETWORK_LIBRARY MQTTMessage {
  QString topic;
  QByteArray payload;
  MQTTQoS qos = MQTTQoS::AtMostOnce;
  bool retain = false;
};

/**
 * @brief MQTT 客户端
 *
 * 基于 TCP 实现的轻量级 MQTT 3.1.1 客户端，支持：
 * - 连接/断开 Broker
 * - 发布消息（QoS 0/1）
 * - 订阅/取消订阅主题
 * - 自动重连
 * - 心跳保活
 *
 * 用法：
 * @code
 * MQTTClient client;
 * MQTTConfig config;
 * config.host = "broker.example.com";
 * config.clientId = "device001";
 * client.setConfig(config);
 *
 * connect(&client, &MQTTClient::connected, [&]() {
 *   client.subscribe("sensors/+/temperature");
 * });
 *
 * connect(&client, &MQTTClient::messageReceived, [](const MQTTMessage& msg) {
 *   qDebug() << "Topic:" << msg.topic << "Payload:" << msg.payload;
 * });
 *
 * client.connectToBroker();
 * @endcode
 */
class NETWORK_LIBRARY MQTTClient : public QObject {
  Q_OBJECT
public:
  /**
   * @brief 连接状态
   */
  enum class ConnectionState {
    Disconnected,
    Connecting,
    Connected,
    Reconnecting
  };
  Q_ENUM(ConnectionState)

  explicit MQTTClient(QObject* parent = nullptr);
  ~MQTTClient() override;

  /**
   * @brief 设置配置
   */
  void setConfig(const MQTTConfig& config);
  MQTTConfig config() const { return m_config; }

  /**
   * @brief 连接到 Broker
   */
  void connectToBroker();

  /**
   * @brief 断开连接
   */
  void disconnectFromBroker();

  /**
   * @brief 获取连接状态
   */
  ConnectionState connectionState() const { return m_state; }

  /**
   * @brief 是否已连接
   */
  bool isConnected() const { return m_state == ConnectionState::Connected; }

  // ============ 发布/订阅 ============

  /**
   * @brief 发布消息
   * @param topic 主题
   * @param payload 消息内容
   * @param qos QoS 等级
   * @param retain 是否保留
   */
  void publish(const QString& topic, const QByteArray& payload,
               MQTTQoS qos = MQTTQoS::AtMostOnce, bool retain = false);

  /**
   * @brief 发布消息（便捷方法）
   */
  void publish(const QString& topic, const QString& payload,
               MQTTQoS qos = MQTTQoS::AtMostOnce, bool retain = false);

  /**
   * @brief 订阅主题
   * @param topic 主题（支持通配符 + 和 #）
   * @param qos QoS 等级
   */
  void subscribe(const QString& topic, MQTTQoS qos = MQTTQoS::AtMostOnce);

  /**
   * @brief 批量订阅
   */
  void subscribe(const QStringList& topics, MQTTQoS qos = MQTTQoS::AtMostOnce);

  /**
   * @brief 取消订阅
   */
  void unsubscribe(const QString& topic);

  /**
   * @brief 取消批量订阅
   */
  void unsubscribe(const QStringList& topics);

  /**
   * @brief 获取已订阅的主题
   */
  QStringList subscribedTopics() const;

signals:
  /**
   * @brief 连接状态改变
   */
  void connectionStateChanged(ConnectionState state);

  /**
   * @brief 已连接到 Broker
   */
  void connected();

  /**
   * @brief 已断开连接
   */
  void disconnected();

  /**
   * @brief 收到消息
   */
  void messageReceived(const MQTTMessage& message);

  /**
   * @brief 订阅成功
   */
  void subscribed(const QString& topic);

  /**
   * @brief 取消订阅成功
   */
  void unsubscribed(const QString& topic);

  /**
   * @brief 发布成功（QoS > 0）
   */
  void published(quint16 messageId);

  /**
   * @brief 错误发生
   */
  void errorOccurred(const QString& error);

private slots:
  void onSocketConnected();
  void onSocketDisconnected();
  void onSocketError(QAbstractSocket::SocketError error);
  void onSocketReadyRead();
  void onKeepAliveTimer();
  void onReconnectTimer();

private:
  // MQTT 协议实现
  void sendConnect();
  void sendPublish(const QString& topic, const QByteArray& payload,
                   MQTTQoS qos, bool retain, quint16 messageId = 0);
  void sendSubscribe(const QStringList& topics, MQTTQoS qos);
  void sendUnsubscribe(const QStringList& topics);
  void sendPingReq();
  void sendDisconnect();

  void processIncomingData();
  void handleConnAck(const QByteArray& data);
  void handlePublish(const QByteArray& data, quint8 flags);
  void handlePubAck(const QByteArray& data);
  void handleSubAck(const QByteArray& data);
  void handleUnsubAck(const QByteArray& data);
  void handlePingResp();

  void setState(ConnectionState state);
  void scheduleReconnect();
  quint16 nextMessageId();

  // 辅助方法
  static QByteArray encodeString(const QString& str);
  static QByteArray encodeRemainingLength(int length);
  static int decodeRemainingLength(const QByteArray& data, int& bytesUsed);

  MQTTConfig m_config;
  QTcpSocket* m_socket = nullptr;
  ConnectionState m_state = ConnectionState::Disconnected;

  QTimer* m_keepAliveTimer = nullptr;
  QTimer* m_reconnectTimer = nullptr;

  QByteArray m_receiveBuffer;
  QMap<QString, MQTTQoS> m_subscriptions;
  QMap<quint16, MQTTMessage> m_pendingPublishes;  // QoS > 0 的待确认消息

  quint16 m_messageIdCounter = 0;
  int m_reconnectAttempts = 0;

  // MQTT 控制包类型
  enum PacketType : quint8 {
    CONNECT     = 0x10,
    CONNACK     = 0x20,
    PUBLISH     = 0x30,
    PUBACK      = 0x40,
    PUBREC      = 0x50,
    PUBREL      = 0x60,
    PUBCOMP     = 0x70,
    SUBSCRIBE   = 0x82,
    SUBACK      = 0x90,
    UNSUBSCRIBE = 0xA2,
    UNSUBACK    = 0xB0,
    PINGREQ     = 0xC0,
    PINGRESP    = 0xD0,
    DISCONNECT  = 0xE0
  };
};

#endif // MQTTCLIENT_H
