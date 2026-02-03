#include "MQTTClient.h"
#include "common/Logger.h"
#include <QUuid>
#include <QDataStream>

MQTTClient::MQTTClient(QObject* parent) : QObject(parent) {
  m_socket = new QTcpSocket(this);
  m_keepAliveTimer = new QTimer(this);
  m_reconnectTimer = new QTimer(this);

  connect(m_socket, &QTcpSocket::connected, this, &MQTTClient::onSocketConnected);
  connect(m_socket, &QTcpSocket::disconnected, this, &MQTTClient::onSocketDisconnected);
  connect(m_socket, &QTcpSocket::errorOccurred, this, &MQTTClient::onSocketError);
  connect(m_socket, &QTcpSocket::readyRead, this, &MQTTClient::onSocketReadyRead);

  connect(m_keepAliveTimer, &QTimer::timeout, this, &MQTTClient::onKeepAliveTimer);
  connect(m_reconnectTimer, &QTimer::timeout, this, &MQTTClient::onReconnectTimer);

  m_reconnectTimer->setSingleShot(true);
}

MQTTClient::~MQTTClient() {
  disconnectFromBroker();
}

void MQTTClient::setConfig(const MQTTConfig& config) {
  m_config = config;
  if (m_config.clientId.isEmpty()) {
    m_config.clientId = "DefectDetection_" + QUuid::createUuid().toString(QUuid::WithoutBraces).left(8);
  }
}

void MQTTClient::connectToBroker() {
  if (m_state == ConnectionState::Connected || m_state == ConnectionState::Connecting) {
    return;
  }

  setState(ConnectionState::Connecting);
  LOG_INFO("MQTTClient: Connecting to {}:{}", m_config.host, m_config.port);

  m_socket->connectToHost(m_config.host, m_config.port);

  // 连接超时
  QTimer::singleShot(m_config.connectTimeoutMs, this, [this]() {
    if (m_state == ConnectionState::Connecting) {
      LOG_WARN("MQTTClient: Connection timeout");
      m_socket->abort();
      setState(ConnectionState::Disconnected);
      scheduleReconnect();
    }
  });
}

void MQTTClient::disconnectFromBroker() {
  m_keepAliveTimer->stop();
  m_reconnectTimer->stop();
  m_reconnectAttempts = 0;

  if (m_state == ConnectionState::Connected) {
    sendDisconnect();
  }

  if (m_socket->state() != QAbstractSocket::UnconnectedState) {
    m_socket->disconnectFromHost();
    if (m_socket->state() != QAbstractSocket::UnconnectedState) {
      m_socket->abort();
    }
  }

  setState(ConnectionState::Disconnected);
}

// ============ 发布/订阅 ============

void MQTTClient::publish(const QString& topic, const QByteArray& payload,
                          MQTTQoS qos, bool retain) {
  if (m_state != ConnectionState::Connected) {
    LOG_WARN("MQTTClient: Cannot publish, not connected");
    return;
  }

  quint16 messageId = (qos != MQTTQoS::AtMostOnce) ? nextMessageId() : 0;
  sendPublish(topic, payload, qos, retain, messageId);

  if (qos != MQTTQoS::AtMostOnce) {
    MQTTMessage msg{topic, payload, qos, retain};
    m_pendingPublishes[messageId] = msg;
  }
}

void MQTTClient::publish(const QString& topic, const QString& payload,
                          MQTTQoS qos, bool retain) {
  publish(topic, payload.toUtf8(), qos, retain);
}

void MQTTClient::subscribe(const QString& topic, MQTTQoS qos) {
  subscribe(QStringList{topic}, qos);
}

void MQTTClient::subscribe(const QStringList& topics, MQTTQoS qos) {
  if (m_state != ConnectionState::Connected) {
    LOG_WARN("MQTTClient: Cannot subscribe, not connected");
    return;
  }

  sendSubscribe(topics, qos);

  for (const QString& topic : topics) {
    m_subscriptions[topic] = qos;
  }
}

void MQTTClient::unsubscribe(const QString& topic) {
  unsubscribe(QStringList{topic});
}

void MQTTClient::unsubscribe(const QStringList& topics) {
  if (m_state != ConnectionState::Connected) {
    return;
  }

  sendUnsubscribe(topics);

  for (const QString& topic : topics) {
    m_subscriptions.remove(topic);
  }
}

QStringList MQTTClient::subscribedTopics() const {
  return m_subscriptions.keys();
}

// ============ 私有槽函数 ============

void MQTTClient::onSocketConnected() {
  LOG_DEBUG("MQTTClient: TCP connected, sending CONNECT packet");
  sendConnect();
}

void MQTTClient::onSocketDisconnected() {
  LOG_WARN("MQTTClient: Disconnected from broker");
  m_keepAliveTimer->stop();

  if (m_state != ConnectionState::Disconnected) {
    setState(ConnectionState::Disconnected);
    emit disconnected();

    if (m_config.autoReconnect) {
      scheduleReconnect();
    }
  }
}

void MQTTClient::onSocketError(QAbstractSocket::SocketError error) {
  QString errorStr = m_socket->errorString();
  LOG_ERROR("MQTTClient: Socket error: {} ({})", errorStr, static_cast<int>(error));
  emit errorOccurred(errorStr);
}

void MQTTClient::onSocketReadyRead() {
  m_receiveBuffer.append(m_socket->readAll());
  processIncomingData();
}

void MQTTClient::onKeepAliveTimer() {
  if (m_state == ConnectionState::Connected) {
    sendPingReq();
  }
}

void MQTTClient::onReconnectTimer() {
  if (m_state == ConnectionState::Connected) return;

  m_reconnectAttempts++;
  LOG_INFO("MQTTClient: Reconnect attempt {}", m_reconnectAttempts);
  setState(ConnectionState::Reconnecting);
  connectToBroker();
}

// ============ MQTT 协议实现 ============

void MQTTClient::sendConnect() {
  QByteArray packet;
  QByteArray variableHeader;
  QByteArray payload;

  // 协议名称 "MQTT"
  variableHeader.append(encodeString("MQTT"));

  // 协议级别 (4 = MQTT 3.1.1)
  variableHeader.append(static_cast<char>(4));

  // 连接标志
  quint8 connectFlags = 0;
  if (m_config.cleanSession) connectFlags |= 0x02;
  if (!m_config.willTopic.isEmpty()) {
    connectFlags |= 0x04;  // Will Flag
    connectFlags |= (m_config.willQos & 0x03) << 3;  // Will QoS
    if (m_config.willRetain) connectFlags |= 0x20;   // Will Retain
  }
  if (!m_config.username.isEmpty()) connectFlags |= 0x80;
  if (!m_config.password.isEmpty()) connectFlags |= 0x40;
  variableHeader.append(static_cast<char>(connectFlags));

  // Keep Alive
  variableHeader.append(static_cast<char>((m_config.keepAliveSeconds >> 8) & 0xFF));
  variableHeader.append(static_cast<char>(m_config.keepAliveSeconds & 0xFF));

  // Payload
  payload.append(encodeString(m_config.clientId));
  if (!m_config.willTopic.isEmpty()) {
    payload.append(encodeString(m_config.willTopic));
    quint16 willMsgLen = m_config.willMessage.size();
    payload.append(static_cast<char>((willMsgLen >> 8) & 0xFF));
    payload.append(static_cast<char>(willMsgLen & 0xFF));
    payload.append(m_config.willMessage);
  }
  if (!m_config.username.isEmpty()) {
    payload.append(encodeString(m_config.username));
  }
  if (!m_config.password.isEmpty()) {
    payload.append(encodeString(m_config.password));
  }

  // 构建完整包
  int remainingLength = variableHeader.size() + payload.size();
  packet.append(static_cast<char>(CONNECT));
  packet.append(encodeRemainingLength(remainingLength));
  packet.append(variableHeader);
  packet.append(payload);

  m_socket->write(packet);
}

void MQTTClient::sendPublish(const QString& topic, const QByteArray& payload,
                              MQTTQoS qos, bool retain, quint16 messageId) {
  QByteArray packet;
  QByteArray variableHeader;

  // 固定头标志
  quint8 fixedHeader = PUBLISH;
  if (retain) fixedHeader |= 0x01;
  fixedHeader |= (static_cast<quint8>(qos) & 0x03) << 1;

  // 主题名
  variableHeader.append(encodeString(topic));

  // 消息ID（QoS > 0）
  if (qos != MQTTQoS::AtMostOnce) {
    variableHeader.append(static_cast<char>((messageId >> 8) & 0xFF));
    variableHeader.append(static_cast<char>(messageId & 0xFF));
  }

  // 构建完整包
  int remainingLength = variableHeader.size() + payload.size();
  packet.append(static_cast<char>(fixedHeader));
  packet.append(encodeRemainingLength(remainingLength));
  packet.append(variableHeader);
  packet.append(payload);

  m_socket->write(packet);
  LOG_DEBUG("MQTTClient: Published to '{}' ({} bytes, QoS {})", topic, payload.size(), static_cast<int>(qos));
}

void MQTTClient::sendSubscribe(const QStringList& topics, MQTTQoS qos) {
  QByteArray packet;
  QByteArray variableHeader;
  QByteArray payload;

  quint16 messageId = nextMessageId();
  variableHeader.append(static_cast<char>((messageId >> 8) & 0xFF));
  variableHeader.append(static_cast<char>(messageId & 0xFF));

  for (const QString& topic : topics) {
    payload.append(encodeString(topic));
    payload.append(static_cast<char>(static_cast<quint8>(qos)));
  }

  int remainingLength = variableHeader.size() + payload.size();
  packet.append(static_cast<char>(SUBSCRIBE));
  packet.append(encodeRemainingLength(remainingLength));
  packet.append(variableHeader);
  packet.append(payload);

  m_socket->write(packet);
  LOG_DEBUG("MQTTClient: Subscribe to {} topics", topics.size());
}

void MQTTClient::sendUnsubscribe(const QStringList& topics) {
  QByteArray packet;
  QByteArray variableHeader;
  QByteArray payload;

  quint16 messageId = nextMessageId();
  variableHeader.append(static_cast<char>((messageId >> 8) & 0xFF));
  variableHeader.append(static_cast<char>(messageId & 0xFF));

  for (const QString& topic : topics) {
    payload.append(encodeString(topic));
  }

  int remainingLength = variableHeader.size() + payload.size();
  packet.append(static_cast<char>(UNSUBSCRIBE));
  packet.append(encodeRemainingLength(remainingLength));
  packet.append(variableHeader);
  packet.append(payload);

  m_socket->write(packet);
}

void MQTTClient::sendPingReq() {
  QByteArray packet;
  packet.append(static_cast<char>(PINGREQ));
  packet.append(static_cast<char>(0));
  m_socket->write(packet);
}

void MQTTClient::sendDisconnect() {
  QByteArray packet;
  packet.append(static_cast<char>(DISCONNECT));
  packet.append(static_cast<char>(0));
  m_socket->write(packet);
  m_socket->flush();
}

void MQTTClient::processIncomingData() {
  while (m_receiveBuffer.size() >= 2) {
    quint8 packetType = static_cast<quint8>(m_receiveBuffer[0]) & 0xF0;
    quint8 flags = static_cast<quint8>(m_receiveBuffer[0]) & 0x0F;

    int bytesUsed = 0;
    int remainingLength = decodeRemainingLength(m_receiveBuffer.mid(1), bytesUsed);

    if (remainingLength < 0) {
      // 数据不完整
      break;
    }

    int totalLength = 1 + bytesUsed + remainingLength;
    if (m_receiveBuffer.size() < totalLength) {
      // 数据不完整
      break;
    }

    QByteArray packetData = m_receiveBuffer.mid(1 + bytesUsed, remainingLength);
    m_receiveBuffer.remove(0, totalLength);

    switch (packetType) {
      case CONNACK:
        handleConnAck(packetData);
        break;
      case PUBLISH:
        handlePublish(packetData, flags);
        break;
      case PUBACK:
        handlePubAck(packetData);
        break;
      case SUBACK:
        handleSubAck(packetData);
        break;
      case UNSUBACK:
        handleUnsubAck(packetData);
        break;
      case PINGRESP:
        handlePingResp();
        break;
      default:
        LOG_WARN("MQTTClient: Unknown packet type: 0x{:02X}", packetType);
        break;
    }
  }
}

void MQTTClient::handleConnAck(const QByteArray& data) {
  if (data.size() < 2) return;

  quint8 returnCode = static_cast<quint8>(data[1]);

  if (returnCode == 0) {
    LOG_INFO("MQTTClient: Connected to broker");
    setState(ConnectionState::Connected);
    m_reconnectAttempts = 0;

    // 启动心跳
    m_keepAliveTimer->start(m_config.keepAliveSeconds * 1000 / 2);

    emit connected();

    // 重新订阅之前的主题
    if (!m_subscriptions.isEmpty()) {
      for (auto it = m_subscriptions.begin(); it != m_subscriptions.end(); ++it) {
        sendSubscribe({it.key()}, it.value());
      }
    }
  } else {
    QString error;
    switch (returnCode) {
      case 1: error = "Unacceptable protocol version"; break;
      case 2: error = "Identifier rejected"; break;
      case 3: error = "Server unavailable"; break;
      case 4: error = "Bad username or password"; break;
      case 5: error = "Not authorized"; break;
      default: error = QString("Unknown error: %1").arg(returnCode); break;
    }
    LOG_ERROR("MQTTClient: Connection refused: {}", error);
    emit errorOccurred(error);
    m_socket->disconnectFromHost();
  }
}

void MQTTClient::handlePublish(const QByteArray& data, quint8 flags) {
  if (data.size() < 2) return;

  int pos = 0;
  quint16 topicLen = (static_cast<quint8>(data[pos]) << 8) | static_cast<quint8>(data[pos + 1]);
  pos += 2;

  if (data.size() < pos + topicLen) return;

  QString topic = QString::fromUtf8(data.mid(pos, topicLen));
  pos += topicLen;

  MQTTQoS qos = static_cast<MQTTQoS>((flags >> 1) & 0x03);
  quint16 messageId = 0;

  if (qos != MQTTQoS::AtMostOnce) {
    if (data.size() < pos + 2) return;
    messageId = (static_cast<quint8>(data[pos]) << 8) | static_cast<quint8>(data[pos + 1]);
    pos += 2;

    // 发送 PUBACK
    QByteArray puback;
    puback.append(static_cast<char>(PUBACK));
    puback.append(static_cast<char>(2));
    puback.append(static_cast<char>((messageId >> 8) & 0xFF));
    puback.append(static_cast<char>(messageId & 0xFF));
    m_socket->write(puback);
  }

  QByteArray payload = data.mid(pos);

  MQTTMessage msg;
  msg.topic = topic;
  msg.payload = payload;
  msg.qos = qos;
  msg.retain = (flags & 0x01) != 0;

  LOG_DEBUG("MQTTClient: Received message on '{}' ({} bytes)", topic, payload.size());
  emit messageReceived(msg);
}

void MQTTClient::handlePubAck(const QByteArray& data) {
  if (data.size() < 2) return;

  quint16 messageId = (static_cast<quint8>(data[0]) << 8) | static_cast<quint8>(data[1]);
  m_pendingPublishes.remove(messageId);

  LOG_DEBUG("MQTTClient: PUBACK received for message {}", messageId);
  emit published(messageId);
}

void MQTTClient::handleSubAck(const QByteArray& data) {
  if (data.size() < 3) return;

  quint16 messageId = (static_cast<quint8>(data[0]) << 8) | static_cast<quint8>(data[1]);
  Q_UNUSED(messageId)

  // 返回码在 data[2] 开始
  for (int i = 2; i < data.size(); ++i) {
    quint8 returnCode = static_cast<quint8>(data[i]);
    if (returnCode == 0x80) {
      LOG_WARN("MQTTClient: Subscription failed");
    }
  }

  LOG_DEBUG("MQTTClient: SUBACK received");
}

void MQTTClient::handleUnsubAck(const QByteArray& data) {
  if (data.size() < 2) return;

  quint16 messageId = (static_cast<quint8>(data[0]) << 8) | static_cast<quint8>(data[1]);
  Q_UNUSED(messageId)

  LOG_DEBUG("MQTTClient: UNSUBACK received");
}

void MQTTClient::handlePingResp() {
  LOG_DEBUG("MQTTClient: PINGRESP received");
}

// ============ 辅助方法 ============

void MQTTClient::setState(ConnectionState state) {
  if (m_state != state) {
    m_state = state;
    emit connectionStateChanged(state);
  }
}

void MQTTClient::scheduleReconnect() {
  if (m_config.autoReconnect) {
    m_reconnectTimer->start(m_config.reconnectIntervalMs);
  }
}

quint16 MQTTClient::nextMessageId() {
  if (++m_messageIdCounter == 0) {
    m_messageIdCounter = 1;
  }
  return m_messageIdCounter;
}

QByteArray MQTTClient::encodeString(const QString& str) {
  QByteArray utf8 = str.toUtf8();
  QByteArray result;
  result.append(static_cast<char>((utf8.size() >> 8) & 0xFF));
  result.append(static_cast<char>(utf8.size() & 0xFF));
  result.append(utf8);
  return result;
}

QByteArray MQTTClient::encodeRemainingLength(int length) {
  QByteArray result;
  do {
    quint8 byte = length % 128;
    length /= 128;
    if (length > 0) {
      byte |= 0x80;
    }
    result.append(static_cast<char>(byte));
  } while (length > 0);
  return result;
}

int MQTTClient::decodeRemainingLength(const QByteArray& data, int& bytesUsed) {
  int multiplier = 1;
  int value = 0;
  bytesUsed = 0;

  for (int i = 0; i < qMin(4, data.size()); ++i) {
    quint8 byte = static_cast<quint8>(data[i]);
    value += (byte & 0x7F) * multiplier;
    multiplier *= 128;
    bytesUsed++;

    if ((byte & 0x80) == 0) {
      return value;
    }
  }

  // 数据不完整
  return -1;
}
