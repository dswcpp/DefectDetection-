# =============================================================================
# network - 网络通信层
# 职责: HTTP 服务、WebSocket 推送、MES 对接、MQTT 消息
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
    mqtt/MQTTClient.h \
    network_global.h \
    websocket/WSServer.h

# ------------------ 源文件 ------------------
SOURCES += \
    http/HttpServer.cpp \
    mes/MESClient.cpp \
    mqtt/MQTTClient.cpp \
    websocket/WSServer.cpp

# ------------------ 安装规则 ------------------
target.path = $$DESTDIR/lib
INSTALLS += target

# 所有库都在 bin 目录
LIBS += -L$$BIN_DIR -lcommon

INCLUDEPATH += $$PWD/../common
DEPENDPATH += $$PWD/../common

# PRE_TARGETDEPS 在子目录构建时由 qmake ordered 配置处理
