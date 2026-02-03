#include "HttpServer.h"
#include "common/Logger.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QUrl>
#include <QUrlQuery>
#include <QRegularExpression>

// ============ HttpResponse 实现 ============

void HttpResponse::setJson(const QJsonObject& json) {
  headers["Content-Type"] = "application/json; charset=utf-8";
  body = QJsonDocument(json).toJson(QJsonDocument::Compact);
}

void HttpResponse::setJson(const QJsonArray& json) {
  headers["Content-Type"] = "application/json; charset=utf-8";
  body = QJsonDocument(json).toJson(QJsonDocument::Compact);
}

void HttpResponse::setText(const QString& text) {
  headers["Content-Type"] = "text/plain; charset=utf-8";
  body = text.toUtf8();
}

void HttpResponse::setHtml(const QString& html) {
  headers["Content-Type"] = "text/html; charset=utf-8";
  body = html.toUtf8();
}

void HttpResponse::setError(int code, const QString& message) {
  statusCode = code;
  switch (code) {
    case 400: statusText = "Bad Request"; break;
    case 401: statusText = "Unauthorized"; break;
    case 403: statusText = "Forbidden"; break;
    case 404: statusText = "Not Found"; break;
    case 405: statusText = "Method Not Allowed"; break;
    case 500: statusText = "Internal Server Error"; break;
    default: statusText = "Error"; break;
  }
  QJsonObject errorObj;
  errorObj["error"] = message;
  errorObj["code"] = code;
  setJson(errorObj);
}

QByteArray HttpResponse::toBytes() const {
  QByteArray response;
  response.append(QString("HTTP/1.1 %1 %2\r\n").arg(statusCode).arg(statusText).toUtf8());

  QMap<QString, QString> allHeaders = headers;
  if (!allHeaders.contains("Content-Length")) {
    allHeaders["Content-Length"] = QString::number(body.size());
  }
  if (!allHeaders.contains("Connection")) {
    allHeaders["Connection"] = "close";
  }

  for (auto it = allHeaders.begin(); it != allHeaders.end(); ++it) {
    response.append(QString("%1: %2\r\n").arg(it.key(), it.value()).toUtf8());
  }
  response.append("\r\n");
  response.append(body);

  return response;
}

HttpResponse HttpResponse::ok(const QJsonObject& data) {
  HttpResponse resp;
  resp.statusCode = 200;
  resp.statusText = "OK";
  if (data.isEmpty()) {
    QJsonObject obj;
    obj["success"] = true;
    resp.setJson(obj);
  } else {
    resp.setJson(data);
  }
  return resp;
}

HttpResponse HttpResponse::created(const QJsonObject& data) {
  HttpResponse resp;
  resp.statusCode = 201;
  resp.statusText = "Created";
  resp.setJson(data);
  return resp;
}

HttpResponse HttpResponse::badRequest(const QString& message) {
  HttpResponse resp;
  resp.setError(400, message);
  return resp;
}

HttpResponse HttpResponse::notFound(const QString& message) {
  HttpResponse resp;
  resp.setError(404, message);
  return resp;
}

HttpResponse HttpResponse::internalError(const QString& message) {
  HttpResponse resp;
  resp.setError(500, message);
  return resp;
}

// ============ HttpServer 实现 ============

HttpServer::HttpServer(QObject* parent) : QObject(parent) {
  m_server = new QTcpServer(this);
  connect(m_server, &QTcpServer::newConnection, this, &HttpServer::onNewConnection);
}

HttpServer::~HttpServer() {
  stop();
}

bool HttpServer::start(quint16 port) {
  if (m_server->isListening()) {
    LOG_WARN("HttpServer: Already running on port {}", m_server->serverPort());
    return true;
  }

  if (!m_server->listen(QHostAddress::Any, port)) {
    LOG_ERROR("HttpServer: Failed to start on port {}: {}", port, m_server->errorString());
    emit errorOccurred(m_server->errorString());
    return false;
  }

  LOG_INFO("HttpServer: Started on port {}", port);
  emit started(port);
  return true;
}

void HttpServer::stop() {
  if (m_server->isListening()) {
    m_server->close();

    // 关闭所有客户端连接
    for (auto it = m_clientBuffers.begin(); it != m_clientBuffers.end(); ++it) {
      it.key()->close();
    }
    m_clientBuffers.clear();

    LOG_INFO("HttpServer: Stopped");
    emit stopped();
  }
}

bool HttpServer::isRunning() const {
  return m_server->isListening();
}

quint16 HttpServer::port() const {
  return m_server->serverPort();
}

// ============ 路由注册 ============

void HttpServer::get(const QString& path, HttpHandler handler) {
  route("GET", path, handler);
}

void HttpServer::post(const QString& path, HttpHandler handler) {
  route("POST", path, handler);
}

void HttpServer::put(const QString& path, HttpHandler handler) {
  route("PUT", path, handler);
}

void HttpServer::del(const QString& path, HttpHandler handler) {
  route("DELETE", path, handler);
}

void HttpServer::route(const QString& method, const QString& path, HttpHandler handler) {
  m_routes.append({method.toUpper(), path, handler});
  LOG_DEBUG("HttpServer: Registered route {} {}", method, path);
}

void HttpServer::setCorsOrigin(const QString& origin) {
  m_corsOrigin = origin;
}

void HttpServer::enableCors(bool enable) {
  m_corsEnabled = enable;
}

// ============ 私有槽函数 ============

void HttpServer::onNewConnection() {
  while (m_server->hasPendingConnections()) {
    QTcpSocket* client = m_server->nextPendingConnection();
    m_clientBuffers[client] = QByteArray();

    connect(client, &QTcpSocket::readyRead, this, &HttpServer::onClientReadyRead);
    connect(client, &QTcpSocket::disconnected, this, &HttpServer::onClientDisconnected);
  }
}

void HttpServer::onClientReadyRead() {
  QTcpSocket* client = qobject_cast<QTcpSocket*>(sender());
  if (!client) return;

  m_clientBuffers[client].append(client->readAll());

  // 检查是否收到完整的请求
  QByteArray& buffer = m_clientBuffers[client];
  int headerEnd = buffer.indexOf("\r\n\r\n");
  if (headerEnd == -1) return;

  // 解析 Content-Length
  int contentLength = 0;
  QRegularExpression contentLengthRe("Content-Length:\\s*(\\d+)", QRegularExpression::CaseInsensitiveOption);
  QRegularExpressionMatch match = contentLengthRe.match(buffer);
  if (match.hasMatch()) {
    contentLength = match.captured(1).toInt();
  }

  // 检查是否收到完整的 body
  if (buffer.size() < headerEnd + 4 + contentLength) return;

  // 提取完整请求
  QByteArray requestData = buffer.left(headerEnd + 4 + contentLength);
  buffer.remove(0, requestData.size());

  // 处理请求
  HttpRequest request = parseRequest(requestData);
  emit requestReceived(request.method, request.path);
  handleRequest(client, request);
}

void HttpServer::onClientDisconnected() {
  QTcpSocket* client = qobject_cast<QTcpSocket*>(sender());
  if (client) {
    m_clientBuffers.remove(client);
    client->deleteLater();
  }
}

// ============ 私有方法 ============

HttpRequest HttpServer::parseRequest(const QByteArray& data) {
  HttpRequest request;

  // 分离头部和 body
  int headerEnd = data.indexOf("\r\n\r\n");
  QByteArray headerPart = data.left(headerEnd);
  request.body = data.mid(headerEnd + 4);

  // 解析请求行和头部
  QList<QByteArray> lines = headerPart.split('\n');
  if (lines.isEmpty()) return request;

  // 解析请求行: GET /path?query HTTP/1.1
  QList<QByteArray> requestLine = lines[0].trimmed().split(' ');
  if (requestLine.size() >= 3) {
    request.method = QString::fromUtf8(requestLine[0]).toUpper();
    QString fullPath = QString::fromUtf8(requestLine[1]);
    request.version = QString::fromUtf8(requestLine[2]);

    // 解析路径和查询参数
    QUrl url(fullPath);
    request.path = url.path();
    QUrlQuery query(url.query());
    for (const auto& item : query.queryItems()) {
      request.queryParams[item.first] = item.second;
    }
  }

  // 解析头部
  for (int i = 1; i < lines.size(); ++i) {
    QString line = QString::fromUtf8(lines[i]).trimmed();
    int colonPos = line.indexOf(':');
    if (colonPos > 0) {
      QString name = line.left(colonPos).trimmed().toLower();
      QString value = line.mid(colonPos + 1).trimmed();
      request.headers[name] = value;
    }
  }

  request.contentType = request.header("content-type");
  return request;
}

void HttpServer::handleRequest(QTcpSocket* client, const HttpRequest& request) {
  LOG_DEBUG("HttpServer: {} {} from {}", request.method, request.path, 
            client->peerAddress().toString());

  HttpResponse response;

  // 处理 OPTIONS 预检请求
  if (request.method == "OPTIONS" && m_corsEnabled) {
    response.statusCode = 204;
    response.statusText = "No Content";
    addCorsHeaders(response);
    response.headers["Allow"] = "GET, POST, PUT, DELETE, OPTIONS";
  } else {
    response = findAndExecuteHandler(request);
    if (m_corsEnabled) {
      addCorsHeaders(response);
    }
  }

  sendResponse(client, response);
}

HttpResponse HttpServer::findAndExecuteHandler(const HttpRequest& request) {
  QMap<QString, QString> pathParams;

  for (const RouteInfo& route : m_routes) {
    if (route.method == request.method && matchRoute(route.pattern, request.path, pathParams)) {
      // 将路径参数合并到查询参数
      HttpRequest modifiedRequest = request;
      for (auto it = pathParams.begin(); it != pathParams.end(); ++it) {
        modifiedRequest.queryParams[it.key()] = it.value();
      }

      try {
        return route.handler(modifiedRequest);
      } catch (const std::exception& e) {
        LOG_ERROR("HttpServer: Handler exception: {}", e.what());
        return HttpResponse::internalError(e.what());
      } catch (...) {
        LOG_ERROR("HttpServer: Unknown handler exception");
        return HttpResponse::internalError("Unknown error");
      }
    }
  }

  return HttpResponse::notFound("Endpoint not found");
}

void HttpServer::sendResponse(QTcpSocket* client, const HttpResponse& response) {
  client->write(response.toBytes());
  client->flush();
  client->disconnectFromHost();
}

bool HttpServer::matchRoute(const QString& pattern, const QString& path, QMap<QString, QString>& params) {
  params.clear();

  // 简单匹配：支持 :param 形式的路径参数
  QStringList patternParts = pattern.split('/', Qt::SkipEmptyParts);
  QStringList pathParts = path.split('/', Qt::SkipEmptyParts);

  if (patternParts.size() != pathParts.size()) {
    return false;
  }

  for (int i = 0; i < patternParts.size(); ++i) {
    if (patternParts[i].startsWith(':')) {
      // 路径参数
      params[patternParts[i].mid(1)] = pathParts[i];
    } else if (patternParts[i] != pathParts[i]) {
      return false;
    }
  }

  return true;
}

void HttpServer::addCorsHeaders(HttpResponse& response) {
  response.headers["Access-Control-Allow-Origin"] = m_corsOrigin;
  response.headers["Access-Control-Allow-Methods"] = "GET, POST, PUT, DELETE, OPTIONS";
  response.headers["Access-Control-Allow-Headers"] = "Content-Type, Authorization";
  response.headers["Access-Control-Max-Age"] = "86400";
}
