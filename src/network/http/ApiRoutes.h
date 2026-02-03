/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * ApiRoutes.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：API路由定义
 * 描述：HTTP API路由配置，定义URL路径与处理函数的映射关系
 *
 * 当前版本：1.1
 * 更新：定义 API 路由常量和路由注册辅助函数
 */

#ifndef APIROUTES_H
#define APIROUTES_H

#include "../network_global.h"
#include "HttpServer.h"
#include <QString>

/**
 * @brief API 路由路径定义
 */
namespace ApiPath {
  // 系统
  const QString HEALTH = "/api/health";
  const QString STATUS = "/api/status";
  const QString VERSION = "/api/version";

  // 检测记录
  const QString INSPECTIONS = "/api/inspections";
  const QString INSPECTION_BY_ID = "/api/inspections/:id";
  const QString INSPECTION_DEFECTS = "/api/inspections/:id/defects";

  // 图片
  const QString IMAGES = "/api/images";
  const QString IMAGE_BY_ID = "/api/images/:id";

  // 统计
  const QString STATISTICS = "/api/statistics";
  const QString STATISTICS_DAILY = "/api/statistics/daily";
  const QString STATISTICS_HOURLY = "/api/statistics/hourly";
  const QString STATISTICS_DEFECTS = "/api/statistics/defects";

  // 配置
  const QString CONFIG = "/api/config";
  const QString CONFIG_SECTION = "/api/config/:section";

  // 设备控制
  const QString DEVICE_START = "/api/device/start";
  const QString DEVICE_STOP = "/api/device/stop";
  const QString DEVICE_RESET = "/api/device/reset";
}

/**
 * @brief API 路由注册辅助类
 *
 * 用于快速注册常用 API 路由。
 *
 * 用法：
 * @code
 * HttpServer server;
 * ApiRoutes::registerHealthRoutes(server);
 * ApiRoutes::registerInspectionRoutes(server, defectRepo);
 * @endcode
 */
class NETWORK_LIBRARY ApiRoutes {
public:
  /**
   * @brief 注册健康检查路由
   */
  static void registerHealthRoutes(HttpServer& server) {
    server.get(ApiPath::HEALTH, [](const HttpRequest&) {
      return HttpResponse::ok({
        {"status", "healthy"},
        {"timestamp", QDateTime::currentDateTime().toString(Qt::ISODate)}
      });
    });

    server.get(ApiPath::VERSION, [](const HttpRequest&) {
      return HttpResponse::ok({
        {"version", "1.0.0"},
        {"name", "DefectDetection"},
        {"buildTime", __DATE__ " " __TIME__}
      });
    });
  }

  /**
   * @brief 创建分页响应
   */
  static QJsonObject createPageResponse(const QJsonArray& items, int total, int page, int pageSize) {
    QJsonObject response;
    response["items"] = items;
    response["pagination"] = QJsonObject{
      {"total", total},
      {"page", page},
      {"pageSize", pageSize},
      {"totalPages", (total + pageSize - 1) / pageSize}
    };
    return response;
  }

  /**
   * @brief 解析分页参数
   */
  static void parsePagination(const HttpRequest& request, int& page, int& pageSize, int& offset) {
    page = qMax(1, request.param("page", "1").toInt());
    pageSize = qBound(1, request.param("pageSize", "20").toInt(), 100);
    offset = (page - 1) * pageSize;
  }

  /**
   * @brief 解析时间范围参数
   */
  static void parseTimeRange(const HttpRequest& request, QDateTime& startTime, QDateTime& endTime) {
    QString start = request.param("startTime");
    QString end = request.param("endTime");

    if (!start.isEmpty()) {
      startTime = QDateTime::fromString(start, Qt::ISODate);
    }
    if (!end.isEmpty()) {
      endTime = QDateTime::fromString(end, Qt::ISODate);
    }
  }
};

#endif // APIROUTES_H
