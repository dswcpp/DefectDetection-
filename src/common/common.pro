# =============================================================================
# common - 公共基础模块
# 职责: 类型定义、日志、工具函数、线程池、无锁队列
# 依赖: 无（最底层模块）
# =============================================================================

include($$PWD/../../config/config.pri)

TEMPLATE = lib
TARGET = common

QT += core
QT -= gui

DEFINES += COMMON_LIBRARY

INCLUDEPATH += $$PWD/../../config
INCLUDEPATH += $$PWD/../../third_party
INCLUDEPATH += $$PWD/../../third_party/opencv/include

# ------------------ 头文件 ------------------
HEADERS += \
    CircularBuffer.h \
    Constants.h \
    ErrorCode.h \
    Logger.h \
    SPSCQueue.h \
    Singleton.h \
    ThreadPool.h \
    Timer.h \
    Types.h \
    Utils.h \
    common_global.h \

# ------------------ 源文件 ------------------
SOURCES += \
    CircularBuffer.cpp \
    Logger.cpp \
    SPSCQueue.cpp \
    Singleton.cpp \
    ThreadPool.cpp \
    Timer.cpp \
    Utils.cpp
# OpenCV 链接（MinGW）
win32-g++ {
    OPENCV_LIB_DIR = $$PWD/../../third_party/opencv/x64/mingw/lib
    LIBS += -L$$OPENCV_LIB_DIR -lopencv_world460
    QMAKE_LIBDIR += $$OPENCV_LIB_DIR
    QMAKE_LIBS += -lopencv_world460
}

# ------------------ 安装规则 ------------------
target.path = $$DESTDIR/lib
INSTALLS += target
