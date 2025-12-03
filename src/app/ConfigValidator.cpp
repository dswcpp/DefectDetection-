#include "ConfigValidator.h"
#include "config/AppConfig.h"
#include "common/Logger.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QJsonDocument>
#include <QJsonParseError>

QString ConfigValidator::ValidationResult::summary() const
{
    QString result;
    if (valid) {
        result = "Configuration is valid.";
    } else {
        result = QString("Configuration invalid: %1 error(s)").arg(errors.size());
    }
    if (!warnings.isEmpty()) {
        result += QString(", %1 warning(s)").arg(warnings.size());
    }
    return result;
}

ConfigValidator::ValidationResult ConfigValidator::validateFile(const QString& path) const
{
    ValidationResult result;
    LOG_DEBUG("ConfigValidator: Validating config file: {}", path.toStdString());

    // 检查文件存在，如果不存在则创建默认配置
    QFile file(path);
    if (!file.exists()) {
        if (!createDefaultConfigIfMissing(path)) {
            result.addError(QString("Config file not found and failed to create default: %1").arg(path));
            return result;
        }
        result.addWarning(QString("Config file not found, created default: %1").arg(path));
    }

    // 打开文件
    if (!file.open(QIODevice::ReadOnly)) {
        result.addError(QString("Cannot open config file: %1").arg(file.errorString()));
        return result;
    }

    // 解析 JSON
    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        result.addError(QString("JSON parse error at offset %1: %2")
                            .arg(parseError.offset)
                            .arg(parseError.errorString()));
        return result;
    }

    if (!doc.isObject()) {
        result.addError("Config root must be a JSON object");
        return result;
    }

    // 验证 JSON 内容
    return validateJson(doc.object());
}

ConfigValidator::ValidationResult ConfigValidator::validateJson(const QJsonObject& json) const
{
    ValidationResult result;

    // 检查必需的顶级键
    QStringList requiredSections = {"camera", "detection", "ui", "database", "log"};
    for (const QString& section : requiredSections) {
        if (!json.contains(section)) {
            result.addWarning(QString("Missing section '%1', using defaults").arg(section));
        } else if (!json[section].isObject()) {
            result.addError(QString("Section '%1' must be an object").arg(section));
        }
    }

    if (!result.valid) {
        return result;
    }

    // 解析并验证配置
    AppConfig config = AppConfig::fromJson(json);
    ValidationResult configResult = validate(config);

    // 合并结果
    result.errors.append(configResult.errors);
    result.warnings.append(configResult.warnings);
    result.valid = result.valid && configResult.valid;

    return result;
}

ConfigValidator::ValidationResult ConfigValidator::validate(const AppConfig& config) const
{
    ValidationResult result;

    // 验证各模块
    ValidationResult cameraResult = validateCamera(config.camera);
    ValidationResult detectionResult = validateDetection(config.detection);
    ValidationResult uiResult = validateUI(config.ui);
    ValidationResult dbResult = validateDatabase(config.database);
    ValidationResult logResult = validateLog(config.log);

    // 合并所有结果
    result.errors.append(cameraResult.errors);
    result.errors.append(detectionResult.errors);
    result.errors.append(uiResult.errors);
    result.errors.append(dbResult.errors);
    result.errors.append(logResult.errors);

    result.warnings.append(cameraResult.warnings);
    result.warnings.append(detectionResult.warnings);
    result.warnings.append(uiResult.warnings);
    result.warnings.append(dbResult.warnings);
    result.warnings.append(logResult.warnings);

    result.valid = cameraResult.valid && detectionResult.valid &&
                   uiResult.valid && dbResult.valid && logResult.valid;
    
    if (result.valid) {
        LOG_INFO("ConfigValidator: Config valid - {} warnings", result.warnings.size());
    } else {
        LOG_ERROR("ConfigValidator: Config invalid - {} errors, {} warnings", 
                  result.errors.size(), result.warnings.size());
    }

    return result;
}

ConfigValidator::ValidationResult ConfigValidator::validateCamera(const CameraConfig& cfg) const
{
    ValidationResult result;

    // 验证相机类型
    if (m_flags & ValidateEnums) {
        QStringList validTypes = {"file", "hik", "daheng", "usb", "gige"};
        if (!checkEnum(cfg.type, validTypes)) {
            result.addError(QString("camera.type '%1' is invalid. Must be one of: %2")
                                .arg(cfg.type, validTypes.join(", ")));
        }
    }

    // 验证数值范围
    if (m_flags & ValidateRange) {
        if (!checkRange(cfg.captureIntervalMs, 10, 60000)) {
            result.addError("camera.captureIntervalMs must be between 10 and 60000");
        }
        if (!checkRange(cfg.exposureUs, 1.0, 10000000.0)) {
            result.addError("camera.exposureUs must be between 1 and 10000000");
        }
        if (!checkRange(cfg.gainDb, 0.0, 48.0)) {
            result.addWarning("camera.gainDb is typically between 0 and 48");
        }
        if (cfg.width < 0 || cfg.height < 0) {
            result.addError("camera.width and height must be >= 0");
        }
    }

    // 验证路径
    if (m_flags & ValidatePaths) {
        if (cfg.type == "file") {
            if (cfg.imageDir.isEmpty()) {
                result.addError("camera.imageDir is required for file camera type");
            } else {
                QDir dir(cfg.imageDir);
                if (!dir.exists()) {
                    result.addWarning(QString("camera.imageDir '%1' does not exist").arg(cfg.imageDir));
                }
            }
        }
    }

    // 相机类型相关验证
    if (cfg.type == "hik" || cfg.type == "daheng" || cfg.type == "gige") {
        if (cfg.ip.isEmpty() && cfg.serial.isEmpty()) {
            result.addWarning(QString("camera.ip or camera.serial should be set for %1 camera").arg(cfg.type));
        }
    }

    return result;
}

ConfigValidator::ValidationResult ConfigValidator::validateDetection(const DetectionConfig& cfg) const
{
    ValidationResult result;

    // 验证数值范围
    if (m_flags & ValidateRange) {
        if (!checkRange(cfg.confidenceThreshold, 0.0, 1.0)) {
            result.addError("detection.confidenceThreshold must be between 0.0 and 1.0");
        }
        if (!checkRange(cfg.batchSize, 1, 32)) {
            result.addError("detection.batchSize must be between 1 and 32");
        }
    }

    // 验证模型路径
    if (m_flags & ValidatePaths) {
        if (cfg.enabled && !cfg.modelPath.isEmpty()) {
            QFileInfo modelFile(cfg.modelPath);
            if (!modelFile.exists()) {
                result.addWarning(QString("detection.modelPath '%1' does not exist").arg(cfg.modelPath));
            } else if (!cfg.modelPath.endsWith(".onnx") && !cfg.modelPath.endsWith(".pt")) {
                result.addWarning("detection.modelPath should be .onnx or .pt file");
            }
        }
    }

    return result;
}

ConfigValidator::ValidationResult ConfigValidator::validateUI(const UIConfig& cfg) const
{
    ValidationResult result;

    // 验证主题
    if (m_flags & ValidateEnums) {
        QStringList validThemes = {"dark", "light", "system"};
        if (!checkEnum(cfg.theme, validThemes)) {
            result.addWarning(QString("ui.theme '%1' is unknown. Recommended: %2")
                                  .arg(cfg.theme, validThemes.join(", ")));
        }

        // 验证语言
        QStringList validLanguages = {"zh_CN", "en_US", "ja_JP"};
        if (!checkEnum(cfg.language, validLanguages)) {
            result.addWarning(QString("ui.language '%1' may not be supported").arg(cfg.language));
        }
    }

    return result;
}

ConfigValidator::ValidationResult ConfigValidator::validateDatabase(const DatabaseConfig& cfg) const
{
    ValidationResult result;

    // 验证数值范围
    if (m_flags & ValidateRange) {
        if (!checkRange(cfg.maxRecords, 1000, 10000000)) {
            result.addWarning("database.maxRecords should be between 1000 and 10000000");
        }
    }

    // 验证路径
    if (m_flags & ValidatePaths) {
        if (cfg.path.isEmpty()) {
            result.addError("database.path is required");
        } else {
            QFileInfo dbFile(cfg.path);
            QDir parentDir = dbFile.dir();
            if (!parentDir.exists()) {
                result.addWarning(QString("database.path parent directory '%1' does not exist")
                                      .arg(parentDir.absolutePath()));
            }
        }
    }

    return result;
}

ConfigValidator::ValidationResult ConfigValidator::validateLog(const LogConfig& cfg) const
{
    ValidationResult result;

    // 验证日志级别
    if (m_flags & ValidateEnums) {
        QStringList validLevels = {"trace", "debug", "info", "warn", "error", "critical", "off"};
        if (!checkEnum(cfg.level, validLevels)) {
            result.addError(QString("log.level '%1' is invalid. Must be one of: %2")
                                .arg(cfg.level, validLevels.join(", ")));
        }
    }

    // 验证数值范围
    if (m_flags & ValidateRange) {
        if (!checkRange(cfg.maxFileSizeMB, 1, 1024)) {
            result.addWarning("log.maxFileSizeMB should be between 1 and 1024");
        }
        if (!checkRange(cfg.maxFileCount, 1, 100)) {
            result.addWarning("log.maxFileCount should be between 1 and 100");
        }
    }

    // 验证日志目录
    if (m_flags & ValidatePaths) {
        if (cfg.dir.isEmpty()) {
            result.addError("log.dir is required");
        } else {
            QDir logDir(cfg.dir);
            if (!logDir.exists()) {
                result.addWarning(QString("log.dir '%1' does not exist, will be created").arg(cfg.dir));
            }
        }
    }

    return result;
}

bool ConfigValidator::isValid(const QString& path) const
{
    return validateFile(path).valid;
}

bool ConfigValidator::isValid(const AppConfig& config) const
{
    return validate(config).valid;
}

bool ConfigValidator::checkPath(const QString& path, bool mustExist) const
{
    if (path.isEmpty()) {
        return false;
    }
    if (mustExist) {
        return QFileInfo::exists(path);
    }
    return true;
}

bool ConfigValidator::checkRange(double value, double min, double max) const
{
    return value >= min && value <= max;
}

bool ConfigValidator::checkRange(int value, int min, int max) const
{
    return value >= min && value <= max;
}

bool ConfigValidator::checkEnum(const QString& value, const QStringList& allowed) const
{
    return allowed.contains(value, Qt::CaseInsensitive);
}

bool ConfigValidator::createDefaultConfigIfMissing(const QString& path)
{
    QFileInfo fileInfo(path);
    if (fileInfo.exists()) {
        return true;  // 文件已存在，无需创建
    }
    return createDefaultConfig(path);
}

bool ConfigValidator::createDefaultConfig(const QString& path)
{
    // 确保父目录存在
    QFileInfo fileInfo(path);
    QDir parentDir = fileInfo.dir();
    if (!parentDir.exists()) {
        if (!parentDir.mkpath(".")) {
            qWarning() << "Failed to create config directory:" << parentDir.absolutePath();
            return false;
        }
    }

    // 创建默认配置
    AppConfig defaultConfig;
    QJsonObject json = defaultConfig.toJson();

    // 写入文件
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Failed to create config file:" << file.errorString();
        return false;
    }

    QJsonDocument doc(json);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    qInfo() << "Created default config file:" << path;
    return true;
}
