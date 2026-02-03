#include "MESClient.h"
#include "common/Logger.h"
#include <QUuid>
#include <QJsonDocument>

MESClient::MESClient(QObject* parent) : QObject(parent) {
  m_socket = new QTcpSocket(this);
  m_heartbeatTimer = new QTimer(this);
  m_reconnectTimer = new QTimer(this);

  connect(m_socket, &QTcpSocket::connected, this, &MESClient::onSocketConnected);
  connect(m_socket, &QTcpSocket::disconnected, this, &MESClient::onSocketDisconnected);
  connect(m_socket, &QTcpSocket::errorOccurred, this, &MESClient::onSocketError);
  connect(m_socket, &QTcpSocket::readyRead, this, &MESClient::onSocketReadyRead);

  connect(m_heartbeatTimer, &QTimer::timeout, this, &MESClient::onHeartbeatTimer);
  connect(m_reconnectTimer, &QTimer::timeout, this, &MESClient::onReconnectTimer);

  m_reconnectTimer->setSingleShot(true);
}

MESClient::~MESClient() {
  disconnectFromServer();
}

void MESClient::setConfig(const MESConfig& config) {
  m_config = config;
}

void MESClient::connectToServer() {
  if (m_state == ConnectionState::Connected || m_state == ConnectionState::Connecting) {
    return;
  }

  setState(ConnectionState::Connecting);
  LOG_INFO("MESClient: Connecting to {}:{}", m_config.host, m_config.port);

  m_socket->connectToHost(m_config.host, m_config.port);

  // 连接超时处理
  QTimer::singleShot(m_config.connectTimeoutMs, this, [this]() {
    if (m_state == ConnectionState::Connecting) {
      LOG_WARN("MESClient: Connection timeout");
      m_socket->abort();
      setState(ConnectionState::Disconnected);
      scheduleReconnect();
    }
  });
}

void MESClient::disconnectFromServer() {
  m_heartbeatTimer->stop();
  m_reconnectTimer->stop();
  m_reconnectAttempts = 0;

  if (m_socket->state() != QAbstractSocket::UnconnectedState) {
    m_socket->disconnectFromHost();
    if (m_socket->state() != QAbstractSocket::UnconnectedState) {
      m_socket->abort();
    }
  }

  setState(ConnectionState::Disconnected);
}

// ============ 业务接口 ============

void MESClient::requestWorkOrder(const QString& workOrderId) {
  MESMessage msg;
  msg.messageId = generateMessageId();
  msg.messageType = MESMessageType::GET_WORK_ORDER;
  msg.deviceId = m_config.deviceId;
  msg.timestamp = QDateTime::currentDateTime();

  if (!workOrderId.isEmpty()) {
    msg.payload["workOrderId"] = workOrderId;
  }

  sendMessage(msg);
}

void MESClient::reportInspectionResult(const MESInspectionReport& report) {
  MESMessage msg;
  msg.messageId = generateMessageId();
  msg.messageType = MESMessageType::REPORT_RESULT;
  msg.deviceId = m_config.deviceId;
  msg.timestamp = QDateTime::currentDateTime();
  msg.payload = report.toJson();

  sendMessage(msg);
}

void MESClient::reportDeviceStatus(const MESDeviceStatus& status) {
  MESMessage msg;
  msg.messageId = generateMessageId();
  msg.messageType = MESMessageType::REPORT_STATUS;
  msg.deviceId = m_config.deviceId;
  msg.timestamp = QDateTime::currentDateTime();
  msg.payload = status.toJson();

  sendMessage(msg);
}

void MESClient::reportAlarm(const MESAlarm& alarm) {
  MESMessage msg;
  msg.messageId = generateMessageId();
  msg.messageType = MESMessageType::REPORT_ALARM;
  msg.deviceId = m_config.deviceId;
  msg.timestamp = QDateTime::currentDateTime();
  msg.payload = alarm.toJson();

  sendMessage(msg);
}

int MESClient::pendingMessageCount() const {
  return m_pendingMessages.size();
}

// ============ 私有槽函数 ============

void MESClient::onSocketConnected() {
  LOG_INFO("MESClient: Connected to {}:{}", m_config.host, m_config.port);
  setState(ConnectionState::Connected);
  m_reconnectAttempts = 0;

  // 启动心跳
  m_heartbeatTimer->start(m_config.heartbeatIntervalMs);

  // 发送积压的消息
  flushPendingMessages();

  emit connected();
}

void MESClient::onSocketDisconnected() {
  LOG_WARN("MESClient: Disconnected from server");
  m_heartbeatTimer->stop();

  if (m_state != ConnectionState::Disconnected) {
    setState(ConnectionState::Disconnected);
    emit disconnected();

    // 自动重连
    if (m_config.autoReconnect) {
      scheduleReconnect();
    }
  }
}

void MESClient::onSocketError(QAbstractSocket::SocketError error) {
  QString errorStr = m_socket->errorString();
  LOG_ERROR("MESClient: Socket error: {} ({})", errorStr, static_cast<int>(error));
  emit errorOccurred(errorStr);
}

void MESClient::onSocketReadyRead() {
  m_receiveBuffer.append(m_socket->readAll());
  processReceivedData();
}

void MESClient::onHeartbeatTimer() {
  if (m_state != ConnectionState::Connected) return;

  MESMessage msg;
  msg.messageId = generateMessageId();
  msg.messageType = MESMessageType::HEARTBEAT;
  msg.deviceId = m_config.deviceId;
  msg.timestamp = QDateTime::currentDateTime();

  // 心跳消息直接发送，不进队列
  if (m_socket->state() == QAbstractSocket::ConnectedState) {
    QByteArray data = msg.toBytes();
    // 简单协议：长度(4字节) + 数据
    QByteArray packet;
    QDataStream stream(&packet, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << static_cast<quint32>(data.size());
    packet.append(data);
    m_socket->write(packet);
  }
}

void MESClient::onReconnectTimer() {
  if (m_state == ConnectionState::Connected) return;

  m_reconnectAttempts++;
  if (m_reconnectAttempts > m_config.maxRetries) {
    LOG_ERROR("MESClient: Max reconnect attempts reached");
    emit errorOccurred("Max reconnect attempts reached");
    return;
  }

  LOG_INFO("MESClient: Reconnect attempt {}/{}", m_reconnectAttempts, m_config.maxRetries);
  setState(ConnectionState::Reconnecting);
  connectToServer();
}

// ============ 私有方法 ============

void MESClient::setState(ConnectionState state) {
  if (m_state != state) {
    m_state = state;
    emit connectionStateChanged(state);
  }
}

void MESClient::sendMessage(const MESMessage& msg) {
  if (m_state == ConnectionState::Connected && m_socket->state() == QAbstractSocket::ConnectedState) {
    // 直接发送
    QByteArray data = msg.toBytes();
    QByteArray packet;
    QDataStream stream(&packet, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << static_cast<quint32>(data.size());
    packet.append(data);

    if (m_socket->write(packet) > 0) {
      LOG_DEBUG("MESClient: Sent message {} type={}", msg.messageId, msg.messageType);
      emit messageSent(msg.messageId);
    } else {
      LOG_WARN("MESClient: Failed to send message {}", msg.messageId);
      // 加入队列等待重发
      QMutexLocker locker(&m_queueMutex);
      if (m_pendingMessages.size() < MAX_PENDING_MESSAGES) {
        m_pendingMessages.enqueue(msg);
      }
      emit messageFailed(msg.messageId, "Send failed");
    }
  } else {
    // 离线，加入队列
    QMutexLocker locker(&m_queueMutex);
    if (m_pendingMessages.size() < MAX_PENDING_MESSAGES) {
      m_pendingMessages.enqueue(msg);
      LOG_DEBUG("MESClient: Message {} queued (offline)", msg.messageId);
    } else {
      LOG_WARN("MESClient: Message queue full, dropping message {}", msg.messageId);
      emit messageFailed(msg.messageId, "Queue full");
    }
  }
}

void MESClient::processReceivedData() {
  // 简单协议：长度(4字节) + 数据
  while (m_receiveBuffer.size() >= 4) {
    QDataStream stream(m_receiveBuffer);
    stream.setByteOrder(QDataStream::BigEndian);

    quint32 length;
    stream >> length;

    if (m_receiveBuffer.size() < static_cast<int>(4 + length)) {
      // 数据不完整，等待更多数据
      break;
    }

    // 提取完整消息
    QByteArray msgData = m_receiveBuffer.mid(4, length);
    m_receiveBuffer.remove(0, 4 + length);

    // 解析响应
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(msgData, &error);
    if (error.error != QJsonParseError::NoError) {
      LOG_WARN("MESClient: Invalid JSON response: {}", error.errorString());
      continue;
    }

    MESResponse response = MESResponse::fromJson(doc.object());
    handleResponse(response);
  }
}

void MESClient::handleResponse(const MESResponse& response) {
  LOG_DEBUG("MESClient: Received response for message {}, success={}", 
            response.messageId, response.success);

  emit responseReceived(response);

  // 处理特定响应
  if (response.success && response.data.contains("workOrder")) {
    MESWorkOrder workOrder = MESWorkOrder::fromJson(response.data["workOrder"].toObject());
    emit workOrderReceived(workOrder);
  }
}

QString MESClient::generateMessageId() {
  return QString("%1-%2").arg(m_config.deviceId).arg(++m_messageCounter, 8, 10, QChar('0'));
}

void MESClient::flushPendingMessages() {
  QMutexLocker locker(&m_queueMutex);
  while (!m_pendingMessages.isEmpty()) {
    MESMessage msg = m_pendingMessages.dequeue();
    locker.unlock();
    sendMessage(msg);
    locker.relock();
  }
}

void MESClient::scheduleReconnect() {
  if (m_config.autoReconnect && m_reconnectAttempts < m_config.maxRetries) {
    m_reconnectTimer->start(m_config.reconnectIntervalMs);
  }
}
