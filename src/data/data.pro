# =============================================================================
# data - 数据持久化层
# 职责: 数据库管理、文件存储、数据导出
# 依赖: common
# =============================================================================

include($$PWD/../../config/config.pri)

TEMPLATE = lib
TARGET = data

QT += core sql gui concurrent

DEFINES += DATA_LIBRARY_BUILD

INCLUDEPATH += $$PWD/../../config
# ------------------ 头文件 ------------------
HEADERS += \
    DatabaseManager.h \
    data_global.h \
    export/CSVExporter.h \
    export/ExcelExporter.h \
    export/ReportGenerator.h \
    repositories/AnnotationRepository.h \
    repositories/ConfigRepository.h \
    repositories/DefectRepository.h \
    repositories/IRepository.h \
    repositories/ImageRepository.h \
    repositories/UserRepository.h \
    storage/BackupManager.h \
    storage/ImageStorage.h

# ------------------ 源文件 ------------------
SOURCES += \
    DatabaseManager.cpp \
    export/CSVExporter.cpp \
    export/ExcelExporter.cpp \
    export/ReportGenerator.cpp \
    repositories/AnnotationRepository.cpp \
    repositories/ConfigRepository.cpp \
    repositories/DefectRepository.cpp \
    repositories/ImageRepository.cpp \
    repositories/UserRepository.cpp \
    storage/BackupManager.cpp \
    storage/ImageStorage.cpp

# ------------------ 安装规则 ------------------
target.path = $$DESTDIR/lib
INSTALLS += target

# 所有库都在 bin 目录
LIBS += -L$$BIN_DIR -lcommon

INCLUDEPATH += $$PWD/../common
DEPENDPATH += $$PWD/../common

# PRE_TARGETDEPS 在子目录构建时由 qmake ordered 配置处理
