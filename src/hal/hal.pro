# =============================================================================
# hal - 硬件抽象层
# 职责: 相机驱动、IO 控制、光源控制、PLC 通信
# 依赖: common
# =============================================================================

include($$PWD/../../config/config.pri)

TEMPLATE = lib
TARGET = hal

QT += core serialport
QT -= gui

DEFINES += HAL_LIBRARY_BUILD


# ------------------ 相机 SDK 配置 ------------------
# 海康相机 SDK
unix {
    HIK_SDK = /opt/MVS
    INCLUDEPATH += $$HIK_SDK/include
    LIBS += -L$$HIK_SDK/lib/64 -lMvCameraControl
    DEFINES += HAVE_HIK_SDK
}
win32 {
    HIK_SDK = "C:/Program Files (x86)/MVS"
    INCLUDEPATH += $$HIK_SDK/Development/Includes
    #LIBS += -L$$HIK_SDK/Development/Libraries/win64 -lMvCameraControl
    DEFINES += HAVE_HIK_SDK
}

# 大恒相机 SDK (可选)
# DAHENG_SDK = /opt/Galaxy_camera
# INCLUDEPATH += $$DAHENG_SDK/inc
# LIBS += -L$$DAHENG_SDK/lib -lgxiapi
# DEFINES += HAVE_DAHENG_SDK
INCLUDEPATH += $$PWD/../../config

# ------------------ 头文件 ------------------
HEADERS += \
    camera/CameraFactory.h \
    camera/DahengCamera.h \
    camera/FileCamera.h \
    camera/GigECamera.h \
    camera/HikCamera.h \
    camera/ICamera.h \
    camera/USBCamera.h \
    hal_global.h \
    io/GPIOController.h \
    io/IIOController.h \
    io/SerialIO.h \
    light/ILightController.h \
    light/ModbusLightController.h \
    light/SerialLightController.h \
    plc/IPLCClient.h \
    plc/MitsubishiMCClient.h \
    plc/ModbusRTUClient.h \
    plc/ModbusTCPClient.h \
    plc/SiemensS7Client.h

# ------------------ 源文件 ------------------
SOURCES += \
    camera/CameraFactory.cpp \
    camera/DahengCamera.cpp \
    camera/FileCamera.cpp \
    camera/GigECamera.cpp \
    camera/HikCamera.cpp \
    camera/USBCamera.cpp \
    io/GPIOController.cpp \
    io/SerialIO.cpp \
    plc/MitsubishiMCClient.cpp \
    plc/ModbusRTUClient.cpp \
    plc/ModbusTCPClient.cpp \
    plc/SiemensS7Client.cpp

# ------------------ 安装规则 ------------------
target.path = $$DESTDIR/lib
INSTALLS += target

# 所有库都在 bin 目录
LIBS += -L$$BIN_DIR -lcommon

INCLUDEPATH += $$PWD/../common
DEPENDPATH += $$PWD/../common

# PRE_TARGETDEPS 在子目录构建时由 qmake ordered 配置处理
