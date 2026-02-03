#include "WSServer.h"
#include "common/Logger.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QUuid>

WSServer::WSServer(QObject *parent) : QObject{parent} {
  m_heartbeatTimer = new QTimer(this);
  connect(m_heartbeatTimer, &QTimer::timeout, this, &WSServer::onHeartbeatCheck);
}

WSServer::~WSServer() {
  stop();
}

bool WSServer::start(quint16 port, const QString& serverName) {
  if (m_server && m_server->isListening()) {
    LOG_WARN("WSServer: Already running on port {}", m_server->serverPort());
    return true;
  }

  m_server = new QWebSocketServer(serverName, QWebSocketServer::NonSecureMode, this);

  if (!m_server->listen(QHostAddress::Any, port)) {
    LOG_ERROR("WSServer: Failed to start on port {}: {}", port, m_server->errorString());
    emit errorOccurred(m_server->errorString());
    delete m_server;
    m_server = nullptr;
    return false;
  }

  connect(m_server, &QWebSocketServer::newConnection, this, &WSServer::onNewConnection);
  connect(m_server, &QWebSocketServer::sslErrors, this, &WSServer::onSslErrors);

  m_heartbeatTimer->start(HEARTBEAT_INTERVAL_MS);

  LOG_INFO("WSServer: Started on port {}", port);
  emit started(port);
  return true;
}

void WSServer::stop() {
  m_heartbeatTimer->stop();

  // 关闭所有客户端连接
  for (auto it = m_clients.begin(); it != m_clients.end(); ++it) {
    it.key()->close(QWebSocketProtocol::CloseCodeGoingAway, "Server shutting down");
  }
  m_clients.clear();
  m_clientById.clear();
  m_topicSubscribers.clear();

  if (m_server) {
    m_server->close();
    delete m_server;
    m_server = nullptr;
    LOG_INFO("WSServer: Stopped");
    emit stopped();
  }
}

bool WSServer::isRunning() const {
  return m_server && m_server->isListening();
}

quint16 WSServer::port() const {
  return m_server ? m_server->serverPort() : 0;
}

int WSServer::clientCount() const {
  return m_clients.size();
}

QList<WSClientInfo> WSServer::clients() const {
  return m_clients.values();
}

// ============ 消息发送 ============

void WSServer::broadcast(const QString& type, const QJsonObject& data) {
  for (auto it = m_clients.begin(); it != m_clients.end(); ++it) {
    sendMessage(it.key(), type, data);
  }
}

void WSServer::publish(const QString& topic, const QString& type, const QJsonObject& data) {
  auto it = m_topicSubscribers.find(topic);
  if (it != m_topicSubscribers.end()) {
    for (QWebSocket* client : it.value()) {
      sendMessage(client, type, data);
    }
  }
}

void WSServer::sendTo(const QString& clientId, const QString& type, const QJsonObject& data) {
  auto it = m_clientById.find(clientId);
  if (it != m_clientById.end()) {
    sendMessage(it.value(), type, data);
  }
}

void WSServer::pushDetectionResult(const QJsonObject& result) {
  publish("detection", WSMessageType::DETECTION_RESULT, result);
}

void WSServer::pushSystemStatus(const QJsonObject& status) {
  publish("system", WSMessageType::SYSTEM_STATUS, status);
}

void WSServer::pushAlarm(const QString& level, const QString& message, const QJsonObject& details) {
  QJsonObject data;
  data["level"] = level;
  data["message"] = message;
  data["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
  if (!details.isEmpty()) {
    data["details"] = details;
  }
  broadcast(WSMessageType::ALARM, data);  // 报警广播给所有人
}

void WSServer::pushStatistics(const QJsonObject& stats) {
  publish("statistics", WSMessageType::STATISTICS, stats);
}

// ============ 私有槽函数 ============

void WSServer::onNewConnection() {
  QWebSocket* client = m_server->nextPendingConnection();
  if (!client) return;

  QString clientId = generateClientId();
  WSClientInfo info;
  info.id = clientId;
  info.remoteAddress = client->peerAddress().toString();
  info.remotePort = client->peerPort();
  info.connectedAt = QDateTime::currentDateTime();
  info.lastHeartbeat = info.connectedAt;

  m_clients[client] = info;
  m_clientById[clientId] = client;

  connect(client, &QWebSocket::textMessageReceived, this, &WSServer::onTextMessageReceived);
  connect(client, &QWebSocket::binaryMessageReceived, this, &WSServer::onBinaryMessageReceived);
  connect(client, &QWebSocket::disconnected, this, &WSServer::onClientDisconnected);

  LOG_INFO("WSServer: Client connected - id={}, address={}:{}", 
           clientId, info.remoteAddress, info.remotePort);
  emit clientConnected(clientId, info.remoteAddress);

  // 发送欢迎消息
  QJsonObject welcome;
  welcome["clientId"] = clientId;
  welcome["serverTime"] = QDateTime::currentDateTime().toString(Qt::ISODate);
  welcome["version"] = "1.0";
  sendMessage(client, "welcome", welcome);
}

void WSServer::onTextMessageReceived(const QString& message) {
  QWebSocket* client = qobject_cast<QWebSocket*>(sender());
  if (!client) return;

  QJsonParseError error;
  QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8(), &error);

  if (error.error != QJsonParseError::NoError) {
    LOG_WARN("WSServer: Invalid JSON from client: {}", error.errorString());
    return;
  }

  processMessage(client, doc.object());
}

void WSServer::onBinaryMessageReceived(const QByteArray& message) {
  // 二进制消息当作 JSON 处理
  QWebSocket* client = qobject_cast<QWebSocket*>(sender());
  if (!client) return;

  QJsonParseError error;
  QJsonDocument doc = QJsonDocument::fromJson(message, &error);

  if (error.error == QJsonParseError::NoError) {
    processMessage(client, doc.object());
  }
}

void WSServer::onClientDisconnected() {
  QWebSocket* client = qobject_cast<QWebSocket*>(sender());
  if (!client) return;

  removeClient(client);
  client->deleteLater();
}

void WSServer::onSslErrors(const QList<QSslError>& errors) {
  for (const auto& error : errors) {
    LOG_WARN("WSServer: SSL error: {}", error.errorString());
  }
}

void WSServer::onHeartbeatCheck() {
  QDateTime now = QDateTime::currentDateTime();
  QList<QWebSocket*> timeoutClients;

  for (auto it = m_clients.begin(); it != m_clients.end(); ++it) {
    qint64 elapsed = it.value().lastHeartbeat.msecsTo(now);
    if (elapsed > HEARTBEAT_TIMEOUT_MS) {
      LOG_WARN("WSServer: Client {} heartbeat timeout", it.value().id);
      timeoutClients.append(it.key());
    }
  }

  // 关闭超时的客户端
  for (QWebSocket* client : timeoutClients) {
    client->close(QWebSocketProtocol::CloseCodeGoingAway, "Heartbeat timeout");
  }
}

// ============ 私有方法 ============

void WSServer::processMessage(QWebSocket* client, const QJsonObject& msg) {
  QString type = msg["type"].toString();
  QJsonObject data = msg["data"].toObject();

  // 更新心跳时间
  if (m_clients.contains(client)) {
    m_clients[client].lastHeartbeat = QDateTime::currentDateTime();
  }

  if (type == WSMessageType::HEARTBEAT) {
    // 回复心跳
    QJsonObject pong;
    pong["serverTime"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    sendMessage(client, "heartbeat_ack", pong);
  } else if (type == WSMessageType::SUBSCRIBE) {
    QJsonArray topics = data["topics"].toArray();
    handleSubscribe(client, topics);
  } else if (type == WSMessageType::UNSUBSCRIBE) {
    QJsonArray topics = data["topics"].toArray();
    handleUnsubscribe(client, topics);
  } else {
    // 转发给应用层处理
    QString clientId = m_clients.contains(client) ? m_clients[client].id : "";
    emit messageReceived(clientId, type, data);
  }
}

void WSServer::handleSubscribe(QWebSocket* client, const QJsonArray& topics) {
  if (!m_clients.contains(client)) return;

  for (const auto& topic : topics) {
    QString t = topic.toString();
    m_topicSubscribers[t].insert(client);
    m_clients[client].subscribedTopics.insert(t);
    LOG_DEBUG("WSServer: Client {} subscribed to '{}'", m_clients[client].id, t);
  }

  QJsonObject ack;
  ack["topics"] = topics;
  sendMessage(client, "subscribe_ack", ack);
}

void WSServer::handleUnsubscribe(QWebSocket* client, const QJsonArray& topics) {
  if (!m_clients.contains(client)) return;

  for (const auto& topic : topics) {
    QString t = topic.toString();
    m_topicSubscribers[t].remove(client);
    m_clients[client].subscribedTopics.remove(t);
    LOG_DEBUG("WSServer: Client {} unsubscribed from '{}'", m_clients[client].id, t);
  }

  QJsonObject ack;
  ack["topics"] = topics;
  sendMessage(client, "unsubscribe_ack", ack);
}

void WSServer::sendMessage(QWebSocket* client, const QString& type, const QJsonObject& data) {
  if (!client || client->state() != QAbstractSocket::ConnectedState) return;

  QJsonObject msg;
  msg["type"] = type;
  msg["data"] = data;
  msg["timestamp"] = QDateTime::currentDateTime().toMSecsSinceEpoch();

  QJsonDocument doc(msg);
  client->sendTextMessage(doc.toJson(QJsonDocument::Compact));
}

QString WSServer::generateClientId() {
  return QUuid::createUuid().toString(QUuid::WithoutBraces).left(8);
}

void WSServer::removeClient(QWebSocket* client) {
  if (!m_clients.contains(client)) return;

  WSClientInfo info = m_clients[client];

  // 从所有订阅中移除
  for (const QString& topic : info.subscribedTopics) {
    m_topicSubscribers[topic].remove(client);
  }

  m_clientById.remove(info.id);
  m_clients.remove(client);

  LOG_INFO("WSServer: Client disconnected - id={}", info.id);
  emit clientDisconnected(info.id);
}
