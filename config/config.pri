# =============================================================================
# 全局配置文件 - 所有子项目共享
# =============================================================================

# ------------------ 版本信息 ------------------
APP_VERSION = 1.0.0
DEFINES += APP_VERSION=\\\"$$APP_VERSION\\\"

# ------------------ 编译器选项 ------------------
CONFIG += c++17
CONFIG += warn_on
CONFIG -= debug_and_release  # 只构建当前配置，输出目录由下方自定义

# 编译优化
RELEASE {
    QMAKE_CXXFLAGS += -O2
    DEFINES += NDEBUG
}

DEBUG {
    DEFINES += DEBUG_MODE
    QMAKE_CXXFLAGS += -g
}

# ------------------ 目录定义 ------------------
# $$PWD 在 .pri 内指向 config 目录，手动回退一层到工程根
ROOT_DIR = $$PWD/..
SRC_DIR = $$ROOT_DIR/src
INCLUDE_DIR = $$ROOT_DIR/include
THIRD_PARTY_DIR = $$ROOT_DIR/third_party
EXTERNAL_DIR = $$ROOT_DIR/external

# ------------------ 构建输出目录 ------------------
DESTDIR = $$OUT_PWD
OBJECTS_DIR = $$OUT_PWD/obj
MOC_DIR = $$OUT_PWD/moc
RCC_DIR = $$OUT_PWD/rcc
UI_DIR = $$OUT_PWD/ui

# ------------------ OpenCV 配置 ------------------
# Linux/Unix：走 pkg-config 或手动路径
unix:!macx {
    CONFIG += link_pkgconfig
    PKGCONFIG += opencv4

    OPENCV_DIR = $$EXTERNAL_DIR/opencv_prebuilt/linux-x64
    INCLUDEPATH += $$OPENCV_DIR/include/opencv4
    LIBS += -L$$OPENCV_DIR/lib \
            -lopencv_core -lopencv_imgproc -lopencv_imgcodecs \
            -lopencv_highgui -lopencv_dnn
}

# Windows
win32 {
    OPENCV_DIR = $$THIRD_PARTY_DIR/opencv
    INCLUDEPATH += $$OPENCV_DIR/include

    # MinGW 和 MSVC 路径不一样，别搞混
    win32-g++ {
        OPENCV_LIB_DIR = $$OPENCV_DIR/x64/mingw/lib
        QMAKE_LIBDIR += $$OPENCV_LIB_DIR
        LIBS += -L$$OPENCV_LIB_DIR -lopencv_world460
        QMAKE_LIBS += -lopencv_world460
    } else {
        OPENCV_LIB_DIR = $$OPENCV_DIR/x64/vc16/lib
        QMAKE_LIBDIR += $$OPENCV_LIB_DIR
        CONFIG(debug, debug|release) {
            LIBS += -L$$OPENCV_LIB_DIR -lopencv_world460d
            QMAKE_LIBS += -lopencv_world460d
        } else {
            LIBS += -L$$OPENCV_LIB_DIR -lopencv_world460
            QMAKE_LIBS += -lopencv_world460
        }
    }
}

# ------------------ 第三方库 ------------------
# spdlog (header-only)
INCLUDEPATH += $$THIRD_PARTY_DIR

# nlohmann/json (header-only)
INCLUDEPATH += $$THIRD_PARTY_DIR/json/include

# 1. Log模块 - 日志记录（暂时不使用，改用 Qt 标准消息系统）
include($$THIRD_PARTY_DIR/spdlog/spdlog.pri)

# libmodbus
# unix {
#     LIBS += -lmodbus
# }
# win32 {
#     MODBUS_DIR = $$EXTERNAL_DIR/libmodbus
#     INCLUDEPATH += $$MODBUS_DIR/include
#     LIBS += -L$$MODBUS_DIR/lib -lmodbus
# }

# ------------------ 项目模块路径 ------------------
INCLUDEPATH += $$SRC_DIR
INCLUDEPATH += $$INCLUDE_DIR

HEADERS += \
    $$PWD/fzspdlog.h
