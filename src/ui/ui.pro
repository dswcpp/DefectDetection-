# =============================================================================
# ui - 用户界面层
# 职责: Qt6 GUI、自定义控件、对话框、视图
# 依赖: common, data, hal, algorithm
# =============================================================================

include($$PWD/../../config/config.pri)

TEMPLATE = lib
TARGET = ui

QT += core gui widgets charts

DEFINES += UI_LIBRARY

INCLUDEPATH += $$PWD/../../config
INCLUDEPATH += $$PWD/../../third_party/opencv/include
# ------------------ 头文件 ------------------
HEADERS += \
    dialogs/AboutDialog.h \
    dialogs/CalibrationDialog.h \
    dialogs/LoginDialog.h \
    dialogs/SettingsDialog.h \
    mainwindow.h \
    models/DefectTableModel.h \
    models/HistoryTableModel.h \
    ui_global.h \
    views/DetectView.h \
    views/HistoryView.h \
    views/SPCView.h \
    views/StatisticsView.h \
    widgets/ImageView.h \
    widgets/ParamPanel.h \
    widgets/ROIEditor.h \
    widgets/ResultCard.h \
    widgets/SeverityBar.h

# ------------------ 源文件 ------------------
SOURCES += \
    dialogs/AboutDialog.cpp \
    dialogs/CalibrationDialog.cpp \
    dialogs/LoginDialog.cpp \
    dialogs/SettingsDialog.cpp \
    mainwindow.cpp \
    models/DefectTableModel.cpp \
    models/HistoryTableModel.cpp \
    views/DetectView.cpp \
    views/HistoryView.cpp \
    views/SPCView.cpp \
    views/StatisticsView.cpp \
    widgets/ImageView.cpp \
    widgets/ParamPanel.cpp \
    widgets/ROIEditor.cpp \
    widgets/ResultCard.cpp \
    widgets/SeverityBar.cpp

# ------------------ UI 文件 ------------------
FORMS += \

# ------------------ 资源文件 ------------------
RESOURCES += \
    $$PWD/../../resources/icons.qrc \
    $$PWD/../../resources/resources.qrc \
    $$PWD/../../resources/styles.qrc \
    $$PWD/../../resources/translations.qrc \
    resource.qrc

# ------------------ 翻译文件 ------------------
TRANSLATIONS += \
    ../../resources/translations/app_zh_CN.ts \
    ../../resources/translations/app_en_US.ts

# ------------------ 安装规则 ------------------
target.path = $$DESTDIR/lib
INSTALLS += target

# OpenCV 链接（MinGW）
win32-g++ {
    OPENCV_LIB_DIR = $$PWD/../../third_party/opencv/x64/mingw/lib
    LIBS += -L$$OPENCV_LIB_DIR -lopencv_world460
    QMAKE_LIBDIR += $$OPENCV_LIB_DIR
    QMAKE_LIBS += -lopencv_world460
}

LIBS += -LF:/Code/QT/DefectDetection/third_party/opencv/x64/mingw/lib -lopencv_world460
QMAKE_LIBDIR += F:/Code/QT/DefectDetection/third_party/opencv/x64/mingw/lib
QMAKE_LIBS += -lopencv_world460
message("UI LIBS after OpenCV: $$LIBS")

LIBS += -L$$OUT_PWD/../common -lcommon

INCLUDEPATH += $$PWD/../common
DEPENDPATH += $$PWD/../common

win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../common/libcommon.a
else:win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../common/common.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../common/libcommon.a

LIBS += -L$$OUT_PWD/../algorithm/ -lalgorithm

INCLUDEPATH += $$PWD/../algorithm
DEPENDPATH += $$PWD/../algorithm
