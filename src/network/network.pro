# =============================================================================
# network - 网络通信层
# 职责: HTTP 服务、WebSocket 推送、MES 对接
# 依赖: common
# =============================================================================

include($$PWD/../../config/config.pri)

TEMPLATE = lib
TARGET = network

QT += core network websockets
QT -= gui

DEFINES += NETWORK_LIBRARY_BUILD

# 可选: HTTP 服务器库
# LIBS += -lqhttpserver
INCLUDEPATH += $$PWD/../../config
# ------------------ 头文件 ------------------
HEADERS += \
    http/ApiRoutes.h \
    http/HttpServer.h \
    mes/MESClient.h \
    mes/MESProtocol.h \
    network_global.h \
    websocket/WSServer.h

# ------------------ 源文件 ------------------
SOURCES += \
    http/HttpServer.cpp \
    mes/MESClient.cpp \
    websocket/WSServer.cpp

# ------------------ 安装规则 ------------------
target.path = $$DESTDIR/lib
INSTALLS += target

LIBS += -L$$OUT_PWD/../common -lcommon

INCLUDEPATH += $$PWD/../common
DEPENDPATH += $$PWD/../common

win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../common/libcommon.a
else:win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../common/common.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../common/libcommon.a
