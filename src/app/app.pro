# =============================================================================
# app - 主应用程序
# 职责: 程序入口、业务流程编排、系统协调
# 依赖: 全部模块
# =============================================================================

include($$PWD/../../config/config.pri)

TEMPLATE = app
TARGET = DefectDetection

QT += core gui widgets charts sql network websockets serialport

# 控制台输出（调试用）
# CONFIG += console

# Windows 图标
win32:RC_ICONS = ../../resources/icons/app.ico

# macOS 图标
macx:ICON = ../../resources/icons/app.icns

INCLUDEPATH += $$PWD/../../config
INCLUDEPATH += $$PWD/../../third_party
INCLUDEPATH += $$PWD/../../third_party/opencv/include
# ------------------ 头文件 ------------------
HEADERS += \
    FlowController.h \
    SystemWatchdog.h \
    ResultAggregator.h

# ------------------ 源文件 ------------------
SOURCES += \
    main.cpp \
    FlowController.cpp \
    SystemWatchdog.cpp \
    ResultAggregator.cpp \
    $$PWD/crt_shim.cpp

CONFIGVALIDATOR_SRC = $$clean_path($$_PRO_FILE_PWD_/ConfigValidator.cpp)
CONFIGVALIDATOR_HDR = $$clean_path($$_PRO_FILE_PWD_/ConfigValidator.h)
SOURCES += $$CONFIGVALIDATOR_SRC
HEADERS += $$CONFIGVALIDATOR_HDR

# ------------------ 部署规则 ------------------
target.path = $$DESTDIR
INSTALLS += target

# 配置文件部署
config.files = ../../config/*
config.path = $$DESTDIR/config
INSTALLS += config

# 模型文件部署
models.files = ../../models/*
models.path = $$DESTDIR/models
INSTALLS += models

# 所有库都在 bin 目录
LIBS += -L$$BIN_DIR -lcommon -ldata -lnetwork -lui -lalgorithm -lhal

INCLUDEPATH += $$PWD/../common
INCLUDEPATH += $$PWD/../data
INCLUDEPATH += $$PWD/../network
INCLUDEPATH += $$PWD/../ui
INCLUDEPATH += $$PWD/../algorithm
INCLUDEPATH += $$PWD/../hal

DEPENDPATH += $$PWD/../common
DEPENDPATH += $$PWD/../data
DEPENDPATH += $$PWD/../network
DEPENDPATH += $$PWD/../ui
DEPENDPATH += $$PWD/../algorithm
DEPENDPATH += $$PWD/../hal

# PRE_TARGETDEPS 在子目录构建时由 qmake ordered 配置处理

# OpenCV 链接（MinGW）
win32-g++ {
    OPENCV_LIB_DIR = $$PWD/../../third_party/opencv/x64/mingw/lib
    LIBS += -L$$OPENCV_LIB_DIR -lopencv_world460
    QMAKE_LIBDIR += $$OPENCV_LIB_DIR
    QMAKE_LIBS += -lopencv_world460
}

# 确保 Qt6EntryPoint 依赖的 C 运行时符号可解析
LIBS += -lmingwex -lmsvcrt
LIBS += -Wl,--start-group $$[QT_INSTALL_LIBS]/libQt6EntryPoint.a -lmingw32 -Wl,--end-group

RESOURCES += \
    resource.qrc
