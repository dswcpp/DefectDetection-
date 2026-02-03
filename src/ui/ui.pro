# =============================================================================
# ui - 用户界面层
# 职责: Qt6 GUI、自定义控件、对话框、视图
# 依赖: common, data, hal, algorithm
# =============================================================================

include($$PWD/../../config/config.pri)

TEMPLATE = lib
TARGET = ui

QT += core gui widgets charts sql concurrent

DEFINES += UI_LIBRARY_BUILD

INCLUDEPATH += $$PWD/../../config
INCLUDEPATH += $$PWD/../../third_party/opencv/include
# ------------------ 头文件 ------------------
HEADERS += \
    DetectPipeline.h \
    dialogs/AboutDialog.h \
    dialogs/CalibrationDialog.h \
    dialogs/HistoryDialog.h \
    dialogs/ImagePreviewDialog.h \
    dialogs/LoginDialog.h \
    dialogs/SettingsDialog.h \
    dialogs/StatisticsDialog.h \
    dialogs/UserManagementDialog.h \
    dialogs/settings/SettingsPageUtils.h \
    dialogs/settings/CameraSettingsPage.h \
    dialogs/settings/LightSettingsPage.h \
    dialogs/settings/PLCSettingsPage.h \
    dialogs/settings/StorageSettingsPage.h \
    dialogs/settings/DetectionSettingsPage.h \
    dialogs/settings/UserSettingsPage.h \
    mainwindow.h \
    models/DefectTableModel.h \
    models/HistoryTableModel.h \
    models/StatisticsModel.h \
    services/StorageService.h \
    services/UserManager.h \
    ui_global.h \
    views/DetectView.h \
    views/HistoryView.h \
    views/SPCView.h \
    views/StatisticsView.h \
    widgets/AnnotationPanel.h \
    widgets/FramelessDialog.h \
    widgets/FramelessMainWindow.h \
    widgets/FramelessWindow.h \
    widgets/ImageView.h \
    widgets/ImageViewControls.h \
    widgets/MessageBox.h \
    widgets/ParamPanel.h \
    widgets/Toast.h \
    widgets/ROIEditor.h \
    widgets/ResultCard.h \
    widgets/SeverityBar.h

# ------------------ 源文件 ------------------
SOURCES += \
    DetectPipeline.cpp \
    dialogs/AboutDialog.cpp \
    dialogs/CalibrationDialog.cpp \
    dialogs/HistoryDialog.cpp \
    dialogs/ImagePreviewDialog.cpp \
    dialogs/LoginDialog.cpp \
    dialogs/SettingsDialog.cpp \
    dialogs/StatisticsDialog.cpp \
    dialogs/UserManagementDialog.cpp \
    dialogs/settings/CameraSettingsPage.cpp \
    dialogs/settings/LightSettingsPage.cpp \
    dialogs/settings/PLCSettingsPage.cpp \
    dialogs/settings/StorageSettingsPage.cpp \
    dialogs/settings/DetectionSettingsPage.cpp \
    dialogs/settings/UserSettingsPage.cpp \
    mainwindow.cpp \
    models/DefectTableModel.cpp \
    models/HistoryTableModel.cpp \
    models/StatisticsModel.cpp \
    services/StorageService.cpp \
    services/UserManager.cpp \
    views/DetectView.cpp \
    views/HistoryView.cpp \
    views/SPCView.cpp \
    views/StatisticsView.cpp \
    widgets/AnnotationPanel.cpp \
    widgets/FramelessDialog.cpp \
    widgets/FramelessMainWindow.cpp \
    widgets/FramelessWindow.cpp \
    widgets/ImageView.cpp \
    widgets/ImageViewControls.cpp \
    widgets/MessageBox.cpp \
    widgets/ParamPanel.cpp \
    widgets/Toast.cpp \
    widgets/ROIEditor.cpp \
    widgets/ResultCard.cpp \
    widgets/SeverityBar.cpp

# ------------------ UI 文件 ------------------
FORMS += \

# ------------------ 资源文件 ------------------


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

# 所有库都在 bin 目录
LIBS += -L$$BIN_DIR -lcommon -lalgorithm -lhal -ldata

INCLUDEPATH += $$PWD/../common
INCLUDEPATH += $$PWD/../algorithm
INCLUDEPATH += $$PWD/../hal
INCLUDEPATH += $$PWD/../data

DEPENDPATH += $$PWD/../common
DEPENDPATH += $$PWD/../algorithm
DEPENDPATH += $$PWD/../hal
DEPENDPATH += $$PWD/../data

# PRE_TARGETDEPS 在子目录构建时由 qmake ordered 配置处理

RESOURCES += \
    resource.qrc

DISTFILES += \
    resources/icons/history.svg
