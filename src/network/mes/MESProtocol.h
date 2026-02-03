/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * MESProtocol.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：MES通信协议定义
 * 描述：MES通信协议数据结构定义，包含消息类型、数据格式、
 *       校验规则等
 *
 * 当前版本：1.1
 * 更新：完整定义 MES 协议结构
 */

#ifndef MESPROTOCOL_H
#define MESPROTOCOL_H

#include "../network_global.h"
#include <QString>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

/**
 * @brief MES 消息类型
 */
namespace MESMessageType {
  // 请求类型
  const QString HEARTBEAT = "HEARTBEAT";           // 心跳
  const QString GET_WORK_ORDER = "GET_WORK_ORDER"; // 获取工单
  const QString REPORT_RESULT = "REPORT_RESULT";   // 上报检测结果
  const QString REPORT_ALARM = "REPORT_ALARM";     // 上报报警
  const QString GET_RECIPE = "GET_RECIPE";         // 获取配方
  const QString REPORT_STATUS = "REPORT_STATUS";   // 上报设备状态

  // 响应类型
  const QString ACK = "ACK";                       // 确认
  const QString NAK = "NAK";                       // 拒绝
  const QString WORK_ORDER = "WORK_ORDER";         // 工单数据
  const QString RECIPE = "RECIPE";                 // 配方数据
}

/**
 * @brief MES 错误码
 */
namespace MESErrorCode {
  const int SUCCESS = 0;
  const int INVALID_FORMAT = 1001;
  const int INVALID_MESSAGE_TYPE = 1002;
  const int WORK_ORDER_NOT_FOUND = 2001;
  const int RECIPE_NOT_FOUND = 2002;
  const int DATABASE_ERROR = 3001;
  const int INTERNAL_ERROR = 9999;
}

/**
 * @brief 工单信息
 */
struct NETWORK_LIBRARY MESWorkOrder {
  QString workOrderId;           // 工单号
  QString productCode;           // 产品编码
  QString productName;           // 产品名称
  QString batchNo;               // 批次号
  int targetQuantity = 0;        // 目标数量
  int completedQuantity = 0;     // 已完成数量
  int ngQuantity = 0;            // NG 数量
  QString status;                // 状态：pending/running/completed/paused
  QString recipeId;              // 配方ID
  QDateTime startTime;           // 开始时间
  QDateTime endTime;             // 结束时间
  QJsonObject customFields;      // 扩展字段

  QJsonObject toJson() const {
    QJsonObject obj;
    obj["workOrderId"] = workOrderId;
    obj["productCode"] = productCode;
    obj["productName"] = productName;
    obj["batchNo"] = batchNo;
    obj["targetQuantity"] = targetQuantity;
    obj["completedQuantity"] = completedQuantity;
    obj["ngQuantity"] = ngQuantity;
    obj["status"] = status;
    obj["recipeId"] = recipeId;
    if (startTime.isValid()) obj["startTime"] = startTime.toString(Qt::ISODate);
    if (endTime.isValid()) obj["endTime"] = endTime.toString(Qt::ISODate);
    if (!customFields.isEmpty()) obj["customFields"] = customFields;
    return obj;
  }

  static MESWorkOrder fromJson(const QJsonObject& obj) {
    MESWorkOrder wo;
    wo.workOrderId = obj["workOrderId"].toString();
    wo.productCode = obj["productCode"].toString();
    wo.productName = obj["productName"].toString();
    wo.batchNo = obj["batchNo"].toString();
    wo.targetQuantity = obj["targetQuantity"].toInt();
    wo.completedQuantity = obj["completedQuantity"].toInt();
    wo.ngQuantity = obj["ngQuantity"].toInt();
    wo.status = obj["status"].toString();
    wo.recipeId = obj["recipeId"].toString();
    wo.startTime = QDateTime::fromString(obj["startTime"].toString(), Qt::ISODate);
    wo.endTime = QDateTime::fromString(obj["endTime"].toString(), Qt::ISODate);
    wo.customFields = obj["customFields"].toObject();
    return wo;
  }
};

/**
 * @brief 检测结果上报
 */
struct NETWORK_LIBRARY MESInspectionReport {
  QString workOrderId;           // 关联工单号
  QString serialNumber;          // 产品序列号
  QString barcode;               // 条码
  QString stationId;             // 工站ID
  bool isPass = true;            // 是否合格
  QString resultCode;            // 结果编码：OK/NG
  int defectCount = 0;           // 缺陷数量
  double maxSeverity = 0.0;      // 最大严重度
  QStringList defectTypes;       // 缺陷类型列表
  QString imagePath;             // 图片路径
  int cycleTimeMs = 0;           // 检测耗时
  QDateTime inspectTime;         // 检测时间
  QJsonObject measurements;      // 测量数据

  QJsonObject toJson() const {
    QJsonObject obj;
    obj["workOrderId"] = workOrderId;
    obj["serialNumber"] = serialNumber;
    obj["barcode"] = barcode;
    obj["stationId"] = stationId;
    obj["isPass"] = isPass;
    obj["resultCode"] = resultCode;
    obj["defectCount"] = defectCount;
    obj["maxSeverity"] = maxSeverity;
    obj["defectTypes"] = QJsonArray::fromStringList(defectTypes);
    obj["imagePath"] = imagePath;
    obj["cycleTimeMs"] = cycleTimeMs;
    obj["inspectTime"] = inspectTime.toString(Qt::ISODate);
    if (!measurements.isEmpty()) obj["measurements"] = measurements;
    return obj;
  }
};

/**
 * @brief 设备状态上报
 */
struct NETWORK_LIBRARY MESDeviceStatus {
  QString deviceId;              // 设备ID
  QString status;                // 状态：idle/running/error/maintenance
  double cpuUsage = 0.0;         // CPU 使用率
  double memoryUsage = 0.0;      // 内存使用率
  double diskUsage = 0.0;        // 磁盘使用率
  int todayTotal = 0;            // 今日总数
  int todayPass = 0;             // 今日合格数
  int todayNG = 0;               // 今日 NG 数
  double yieldRate = 0.0;        // 良率
  QDateTime timestamp;           // 时间戳

  QJsonObject toJson() const {
    QJsonObject obj;
    obj["deviceId"] = deviceId;
    obj["status"] = status;
    obj["cpuUsage"] = cpuUsage;
    obj["memoryUsage"] = memoryUsage;
    obj["diskUsage"] = diskUsage;
    obj["todayTotal"] = todayTotal;
    obj["todayPass"] = todayPass;
    obj["todayNG"] = todayNG;
    obj["yieldRate"] = yieldRate;
    obj["timestamp"] = timestamp.toString(Qt::ISODate);
    return obj;
  }
};

/**
 * @brief 报警上报
 */
struct NETWORK_LIBRARY MESAlarm {
  QString alarmId;               // 报警ID
  QString deviceId;              // 设备ID
  QString level;                 // 级别：info/warning/error/critical
  QString code;                  // 报警码
  QString message;               // 报警消息
  QString source;                // 来源
  QDateTime occurTime;           // 发生时间
  QDateTime clearTime;           // 清除时间
  bool isActive = true;          // 是否活动

  QJsonObject toJson() const {
    QJsonObject obj;
    obj["alarmId"] = alarmId;
    obj["deviceId"] = deviceId;
    obj["level"] = level;
    obj["code"] = code;
    obj["message"] = message;
    obj["source"] = source;
    obj["occurTime"] = occurTime.toString(Qt::ISODate);
    if (clearTime.isValid()) obj["clearTime"] = clearTime.toString(Qt::ISODate);
    obj["isActive"] = isActive;
    return obj;
  }
};

/**
 * @brief MES 消息基类
 */
struct NETWORK_LIBRARY MESMessage {
  QString messageId;             // 消息ID
  QString messageType;           // 消息类型
  QString deviceId;              // 设备ID
  QDateTime timestamp;           // 时间戳
  QJsonObject payload;           // 消息体

  QJsonObject toJson() const {
    QJsonObject obj;
    obj["messageId"] = messageId;
    obj["messageType"] = messageType;
    obj["deviceId"] = deviceId;
    obj["timestamp"] = timestamp.toString(Qt::ISODate);
    obj["payload"] = payload;
    return obj;
  }

  QByteArray toBytes() const {
    return QJsonDocument(toJson()).toJson(QJsonDocument::Compact);
  }

  static MESMessage fromJson(const QJsonObject& obj) {
    MESMessage msg;
    msg.messageId = obj["messageId"].toString();
    msg.messageType = obj["messageType"].toString();
    msg.deviceId = obj["deviceId"].toString();
    msg.timestamp = QDateTime::fromString(obj["timestamp"].toString(), Qt::ISODate);
    msg.payload = obj["payload"].toObject();
    return msg;
  }

  static MESMessage fromBytes(const QByteArray& data) {
    QJsonDocument doc = QJsonDocument::fromJson(data);
    return fromJson(doc.object());
  }
};

/**
 * @brief MES 响应
 */
struct NETWORK_LIBRARY MESResponse {
  QString messageId;             // 对应的消息ID
  bool success = false;          // 是否成功
  int errorCode = 0;             // 错误码
  QString errorMessage;          // 错误消息
  QJsonObject data;              // 响应数据

  QJsonObject toJson() const {
    QJsonObject obj;
    obj["messageId"] = messageId;
    obj["success"] = success;
    obj["errorCode"] = errorCode;
    if (!errorMessage.isEmpty()) obj["errorMessage"] = errorMessage;
    if (!data.isEmpty()) obj["data"] = data;
    return obj;
  }

  static MESResponse fromJson(const QJsonObject& obj) {
    MESResponse resp;
    resp.messageId = obj["messageId"].toString();
    resp.success = obj["success"].toBool();
    resp.errorCode = obj["errorCode"].toInt();
    resp.errorMessage = obj["errorMessage"].toString();
    resp.data = obj["data"].toObject();
    return resp;
  }
};

#endif // MESPROTOCOL_H
