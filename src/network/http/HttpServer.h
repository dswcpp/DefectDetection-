/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * HttpServer.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：HTTP服务器接口定义
 * 描述：简单的 RESTful HTTP 服务器，提供 API 接口供外部系统调用，
 *       查询检测记录、获取统计数据等
 *
 * 当前版本：1.1
 * 更新：基于 QTcpServer 实现简单 HTTP 服务器
 */

#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include "../network_global.h"
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QMap>
#include <QJsonObject>
#include <functional>

/**
 * @brief HTTP 请求
 */
struct NETWORK_LIBRARY HttpRequest {
  QString method;                        // GET/POST/PUT/DELETE
  QString path;                          // 请求路径
  QString version;                       // HTTP/1.1
  QMap<QString, QString> headers;        // 请求头
  QMap<QString, QString> queryParams;    // 查询参数
  QByteArray body;                       // 请求体
  QString contentType;                   // Content-Type

  QString header(const QString& name, const QString& defaultValue = QString()) const {
    return headers.value(name.toLower(), defaultValue);
  }

  QString param(const QString& name, const QString& defaultValue = QString()) const {
    return queryParams.value(name, defaultValue);
  }
};

/**
 * @brief HTTP 响应
 */
struct NETWORK_LIBRARY HttpResponse {
  int statusCode = 200;
  QString statusText = "OK";
  QMap<QString, QString> headers;
  QByteArray body;

  void setJson(const QJsonObject& json);
  void setJson(const QJsonArray& json);
  void setText(const QString& text);
  void setHtml(const QString& html);
  void setError(int code, const QString& message);

  QByteArray toBytes() const;

  // 快捷方法
  static HttpResponse ok(const QJsonObject& data = {});
  static HttpResponse created(const QJsonObject& data = {});
  static HttpResponse badRequest(const QString& message = "Bad Request");
  static HttpResponse notFound(const QString& message = "Not Found");
  static HttpResponse internalError(const QString& message = "Internal Server Error");
};

/**
 * @brief 路由处理函数
 */
using HttpHandler = std::function<HttpResponse(const HttpRequest&)>;

/**
 * @brief 路由信息
 */
struct RouteInfo {
  QString method;
  QString pattern;      // 支持 :param 形式的路径参数
  HttpHandler handler;
};

/**
 * @brief 简单 HTTP 服务器
 *
 * 基于 QTcpServer 实现的轻量级 HTTP 服务器，
 * 支持基本的 RESTful API。
 *
 * 用法：
 * @code
 * HttpServer server;
 * server.get("/api/status", [](const HttpRequest& req) {
 *   return HttpResponse::ok({{"status", "running"}});
 * });
 * server.start(8080);
 * @endcode
 */
class NETWORK_LIBRARY HttpServer : public QObject {
  Q_OBJECT
public:
  explicit HttpServer(QObject* parent = nullptr);
  ~HttpServer() override;

  /**
   * @brief 启动服务器
   * @param port 监听端口
   */
  bool start(quint16 port = 8080);

  /**
   * @brief 停止服务器
   */
  void stop();

  /**
   * @brief 是否运行中
   */
  bool isRunning() const;

  /**
   * @brief 获取监听端口
   */
  quint16 port() const;

  // ============ 路由注册 ============

  /**
   * @brief 注册 GET 路由
   */
  void get(const QString& path, HttpHandler handler);

  /**
   * @brief 注册 POST 路由
   */
  void post(const QString& path, HttpHandler handler);

  /**
   * @brief 注册 PUT 路由
   */
  void put(const QString& path, HttpHandler handler);

  /**
   * @brief 注册 DELETE 路由
   */
  void del(const QString& path, HttpHandler handler);

  /**
   * @brief 注册任意方法路由
   */
  void route(const QString& method, const QString& path, HttpHandler handler);

  /**
   * @brief 设置 CORS 允许的源
   */
  void setCorsOrigin(const QString& origin);

  /**
   * @brief 启用/禁用 CORS
   */
  void enableCors(bool enable = true);

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
   * @brief 收到请求
   */
  void requestReceived(const QString& method, const QString& path);

  /**
   * @brief 错误发生
   */
  void errorOccurred(const QString& error);

private slots:
  void onNewConnection();
  void onClientReadyRead();
  void onClientDisconnected();

private:
  HttpRequest parseRequest(const QByteArray& data);
  void handleRequest(QTcpSocket* client, const HttpRequest& request);
  HttpResponse findAndExecuteHandler(const HttpRequest& request);
  void sendResponse(QTcpSocket* client, const HttpResponse& response);
  bool matchRoute(const QString& pattern, const QString& path, QMap<QString, QString>& params);
  void addCorsHeaders(HttpResponse& response);

  QTcpServer* m_server = nullptr;
  QList<RouteInfo> m_routes;
  QMap<QTcpSocket*, QByteArray> m_clientBuffers;

  bool m_corsEnabled = true;
  QString m_corsOrigin = "*";
};

#endif // HTTPSERVER_H
