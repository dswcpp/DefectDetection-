#ifndef APPCONFIG_H
#define APPCONFIG_H

#include "common_global.h"
#include <QJsonObject>
#include <QMetaProperty>
#include <QString>

// ============================================================================
// 通用 JSON 序列化模板（基于 Q_GADGET 反射）
// ============================================================================

template <typename T>
QJsonObject gadgetToJson(const T& obj) {
    QJsonObject json;
    const QMetaObject* meta = &T::staticMetaObject;
    for (int i = 0; i < meta->propertyCount(); ++i) {
        QMetaProperty prop = meta->property(i);
        if (prop.isReadable()) {
            QVariant value = prop.readOnGadget(&obj);
            json[QString::fromLatin1(prop.name())] = QJsonValue::fromVariant(value);
        }
    }
    return json;
}

template <typename T>
void gadgetFromJson(T& obj, const QJsonObject& json) {
    const QMetaObject* meta = &T::staticMetaObject;
    for (int i = 0; i < meta->propertyCount(); ++i) {
        QMetaProperty prop = meta->property(i);
        if (prop.isWritable()) {
            QString name = QString::fromLatin1(prop.name());
            if (json.contains(name)) {
                QVariant value = json[name].toVariant();
                prop.writeOnGadget(&obj, value);
            }
        }
    }
}

// ============================================================================
// 相机配置
// ============================================================================

struct COMMON_LIBRARY CameraConfig {
    Q_GADGET
    Q_PROPERTY(QString type MEMBER type)
    Q_PROPERTY(QString imageDir MEMBER imageDir)
    Q_PROPERTY(QString ip MEMBER ip)
    Q_PROPERTY(QString serial MEMBER serial)
    Q_PROPERTY(int captureIntervalMs MEMBER captureIntervalMs)
    Q_PROPERTY(bool loop MEMBER loop)
    Q_PROPERTY(int width MEMBER width)
    Q_PROPERTY(int height MEMBER height)
    Q_PROPERTY(double exposureUs MEMBER exposureUs)
    Q_PROPERTY(double gainDb MEMBER gainDb)

public:
    QString type = "file";
    QString imageDir = "./images";
    QString ip;
    QString serial;
    int captureIntervalMs = 1000;
    bool loop = true;
    int width = 0;
    int height = 0;
    double exposureUs = 10000.0;
    double gainDb = 0.0;

    QJsonObject toJson() const { return gadgetToJson(*this); }
    static CameraConfig fromJson(const QJsonObject& json) {
        CameraConfig cfg;
        gadgetFromJson(cfg, json);
        return cfg;
    }
};

// ============================================================================
// 检测配置
// ============================================================================

struct COMMON_LIBRARY DetectionConfig {
    Q_GADGET
    Q_PROPERTY(bool enabled MEMBER enabled)
    Q_PROPERTY(double confidenceThreshold MEMBER confidenceThreshold)
    Q_PROPERTY(QString modelPath MEMBER modelPath)
    Q_PROPERTY(int batchSize MEMBER batchSize)
    Q_PROPERTY(bool useGPU MEMBER useGPU)

public:
    bool enabled = true;
    double confidenceThreshold = 0.75;
    QString modelPath = "./models/yolo.onnx";
    int batchSize = 1;
    bool useGPU = false;

    QJsonObject toJson() const { return gadgetToJson(*this); }
    static DetectionConfig fromJson(const QJsonObject& json) {
        DetectionConfig cfg;
        gadgetFromJson(cfg, json);
        return cfg;
    }
};

// ============================================================================
// 界面配置
// ============================================================================

struct COMMON_LIBRARY UIConfig {
    Q_GADGET
    Q_PROPERTY(QString theme MEMBER theme)
    Q_PROPERTY(QString language MEMBER language)
    Q_PROPERTY(bool showFPS MEMBER showFPS)
    Q_PROPERTY(bool fullscreen MEMBER fullscreen)

public:
    QString theme = "dark";
    QString language = "zh_CN";
    bool showFPS = true;
    bool fullscreen = false;

    QJsonObject toJson() const { return gadgetToJson(*this); }
    static UIConfig fromJson(const QJsonObject& json) {
        UIConfig cfg;
        gadgetFromJson(cfg, json);
        return cfg;
    }
};

// ============================================================================
// 数据库配置
// ============================================================================

struct COMMON_LIBRARY DatabaseConfig {
    Q_GADGET
    Q_PROPERTY(QString path MEMBER path)
    Q_PROPERTY(int maxRecords MEMBER maxRecords)
    Q_PROPERTY(bool autoCleanup MEMBER autoCleanup)

public:
    QString path = "./data/defects.db";
    int maxRecords = 100000;
    bool autoCleanup = true;

    QJsonObject toJson() const { return gadgetToJson(*this); }
    static DatabaseConfig fromJson(const QJsonObject& json) {
        DatabaseConfig cfg;
        gadgetFromJson(cfg, json);
        return cfg;
    }
};

// ============================================================================
// 日志配置
// ============================================================================

struct COMMON_LIBRARY LogConfig {
    Q_GADGET
    Q_PROPERTY(QString level MEMBER level)
    Q_PROPERTY(QString dir MEMBER dir)
    Q_PROPERTY(int maxFileSizeMB MEMBER maxFileSizeMB)
    Q_PROPERTY(int maxFileCount MEMBER maxFileCount)
    Q_PROPERTY(bool enableConsole MEMBER enableConsole)

public:
    QString level = "info";
    QString dir = "./logs";
    int maxFileSizeMB = 50;
    int maxFileCount = 10;
    bool enableConsole = true;

    QJsonObject toJson() const { return gadgetToJson(*this); }
    static LogConfig fromJson(const QJsonObject& json) {
        LogConfig cfg;
        gadgetFromJson(cfg, json);
        return cfg;
    }
};

// ============================================================================
// 总配置
// ============================================================================

struct COMMON_LIBRARY AppConfig {
    CameraConfig camera;
    DetectionConfig detection;
    UIConfig ui;
    DatabaseConfig database;
    LogConfig log;

    QJsonObject toJson() const {
        QJsonObject json;
        json["camera"] = camera.toJson();
        json["detection"] = detection.toJson();
        json["ui"] = ui.toJson();
        json["database"] = database.toJson();
        json["log"] = log.toJson();
        return json;
    }

    static AppConfig fromJson(const QJsonObject& json) {
        AppConfig cfg;
        if (json.contains("camera"))
            cfg.camera = CameraConfig::fromJson(json["camera"].toObject());
        if (json.contains("detection"))
            cfg.detection = DetectionConfig::fromJson(json["detection"].toObject());
        if (json.contains("ui"))
            cfg.ui = UIConfig::fromJson(json["ui"].toObject());
        if (json.contains("database"))
            cfg.database = DatabaseConfig::fromJson(json["database"].toObject());
        if (json.contains("log"))
            cfg.log = LogConfig::fromJson(json["log"].toObject());
        return cfg;
    }
};

#endif // APPCONFIG_H
