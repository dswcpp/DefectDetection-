/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * ConfigValidator.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：配置验证器接口定义
 * 描述：配置文件验证工具，检查配置完整性、参数合法性、
 *       硬件连接可用性等
 *
 * 当前版本：1.0
 */

#ifndef CONFIGVALIDATOR_H
#define CONFIGVALIDATOR_H

#include <QString>
#include <QStringList>
#include <QJsonObject>
#include <functional>

struct AppConfig;
struct CameraConfig;
struct DetectionConfig;
struct UIConfig;
struct DatabaseConfig;
struct LogConfig;

class ConfigValidator {
public:
    struct ValidationResult {
        bool valid = true;
        QStringList errors;
        QStringList warnings;

        void addError(const QString& msg) {
            valid = false;
            errors.append(msg);
        }
        void addWarning(const QString& msg) {
            warnings.append(msg);
        }
        QString summary() const;
    };

    enum ValidateFlags {
        ValidateFormat      = 0x01,  // 验证 JSON 格式
        ValidateRange       = 0x02,  // 验证数值范围
        ValidatePaths       = 0x04,  // 验证路径存在
        ValidateEnums       = 0x08,  // 验证枚举值
        ValidateAll         = 0xFF   // 全部验证
    };

    ConfigValidator() = default;

    // 设置验证标志
    void setFlags(int flags) { m_flags = flags; }
    int flags() const { return m_flags; }

    // 验证配置文件
    ValidationResult validateFile(const QString& path) const;

    // 验证 JSON 对象
    ValidationResult validateJson(const QJsonObject& json) const;

    // 验证内存配置对象
    ValidationResult validate(const AppConfig& config) const;

    // 分模块验证
    ValidationResult validateCamera(const CameraConfig& cfg) const;
    ValidationResult validateDetection(const DetectionConfig& cfg) const;
    ValidationResult validateUI(const UIConfig& cfg) const;
    ValidationResult validateDatabase(const DatabaseConfig& cfg) const;
    ValidationResult validateLog(const LogConfig& cfg) const;

    // 便捷方法
    bool isValid(const QString& path) const;
    bool isValid(const AppConfig& config) const;

    // 创建默认配置文件（如果不存在）
    static bool createDefaultConfigIfMissing(const QString& path);
    static bool createDefaultConfig(const QString& path);

private:
    int m_flags = ValidateAll;

    bool checkPath(const QString& path, bool mustExist = false) const;
    bool checkRange(double value, double min, double max) const;
    bool checkRange(int value, int min, int max) const;
    bool checkEnum(const QString& value, const QStringList& allowed) const;
};

#endif // CONFIGVALIDATOR_H
