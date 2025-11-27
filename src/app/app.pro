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
    DetectPipeline.h \
    FlowController.h \
    ConfigManager.h \
    SystemWatchdog.h \
    ResultAggregator.h

# ------------------ 源文件 ------------------
SOURCES += \
    main.cpp \
    DetectPipeline.cpp \
    FlowController.cpp \
    ConfigManager.cpp \
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

LIBS += -L$$OUT_PWD/../common -lcommon

INCLUDEPATH += $$PWD/../common
DEPENDPATH += $$PWD/../common

win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../common/libcommon.a
else:win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../common/common.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../common/libcommon.a

LIBS += -L$$OUT_PWD/../data -ldata

INCLUDEPATH += $$PWD/../data
DEPENDPATH += $$PWD/../data

win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../data/libdata.a
else:win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../data/data.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../data/libdata.a

LIBS += -L$$OUT_PWD/../network -lnetwork

INCLUDEPATH += $$PWD/../network
DEPENDPATH += $$PWD/../network

win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../network/libnetwork.a
else:win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../network/network.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../network/libnetwork.a

LIBS += -L$$OUT_PWD/../ui -lui

INCLUDEPATH += $$PWD/../ui
DEPENDPATH += $$PWD/../ui

win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../ui/libui.a
else:win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../ui/ui.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../ui/libui.a

LIBS += -L$$OUT_PWD/../algorithm -lalgorithm

INCLUDEPATH += $$PWD/../algorithm
DEPENDPATH += $$PWD/../algorithm

win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../algorithm/libalgorithm.a
else:win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../algorithm/algorithm.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../algorithm/libalgorithm.a

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
