# =============================================================================
# data - 数据持久化层
# 职责: 数据库管理、文件存储、数据导出
# 依赖: common
# =============================================================================

include($$PWD/../../config/config.pri)

TEMPLATE = lib
TARGET = data

QT += core sql
QT -= gui

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
    repositories/InspectionRepository.h \
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
    repositories/InspectionRepository.cpp \
    storage/BackupManager.cpp \
    storage/ImageStorage.cpp

# ------------------ 安装规则 ------------------
target.path = $$DESTDIR/lib
INSTALLS += target

LIBS += -L$$OUT_PWD/../common -lcommon

INCLUDEPATH += $$PWD/../common
DEPENDPATH += $$PWD/../common

win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../common/libcommon.a
else:win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../common/common.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../common/libcommon.a
