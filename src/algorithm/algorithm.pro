# =============================================================================
# algorithm - 算法引擎层
# 职责: 图像预处理、缺陷检测、DNN 推理、严重度评分
# 依赖: common, hal
# =============================================================================

include($$PWD/../../config/config.pri)

TEMPLATE = lib
TARGET = algorithm

QT += core concurrent
QT -= gui

DEFINES += ALGORITHM_LIBRARY_BUILD

# ------------------ 预编译头文件（加速编译 2-5 倍） ------------------
CONFIG += precompile_header
PRECOMPILED_HEADER = algorithm_pch.h

# OpenCV 已在 config.pri 中配置
INCLUDEPATH += $$PWD/../../config
# ------------------ 头文件 ------------------
HEADERS += \
    algorithm_pch.h \
    BaseDetector.h \
    DetectorFactory.h \
    DetectorManager.h \
    IDefectDetector.h \
    algorithm_global.h \
    detectors/CrackDetector.h \
    detectors/DimensionDetector.h \
    detectors/ForeignDetector.h \
    detectors/ScratchDetector.h \
    dnn/DnnDetector.h \
    dnn/ModelManager.h \
    dnn/ModelValidationReport.h \
    dnn/ModelValidator.h \
    dnn/YoloDetector.h \
    plugin/IDetectorPlugin.h \
    plugin/PluginManager.h \
    postprocess/DefectMerger.h \
    postprocess/NMSFilter.h \
    preprocess/Calibration.h \
    preprocess/ImagePreprocessor.h \
    preprocess/PreprocessCache.h \
    preprocess/ROIManager.h \
    scoring/DefectScorer.h \
    scoring/SeverityConfig.h

# ------------------ 源文件 ------------------
SOURCES += \
    DetectorFactory.cpp \
    DetectorManager.cpp \
    detectors/CrackDetector.cpp \
    detectors/DimensionDetector.cpp \
    detectors/ForeignDetector.cpp \
    detectors/ScratchDetector.cpp \
    dnn/DnnDetector.cpp \
    dnn/ModelValidator.cpp \
    dnn/YoloDetector.cpp \
    plugin/PluginManager.cpp \
    postprocess/NMSFilter.cpp \
    preprocess/ImagePreprocessor.cpp \
    preprocess/PreprocessCache.cpp \
    scoring/DefectScorer.cpp

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

# OpenCV 链接（MinGW）
win32-g++ {
    OPENCV_LIB_DIR = $$PWD/../../third_party/opencv/x64/mingw/lib
    LIBS += -L$$OPENCV_LIB_DIR -lopencv_world460
    QMAKE_LIBDIR += $$OPENCV_LIB_DIR
    QMAKE_LIBS += -lopencv_world460
}

# 所有库都在 bin 目录
LIBS += -L$$BIN_DIR -lcommon -ldata -lhal

INCLUDEPATH += $$PWD/../common
INCLUDEPATH += $$PWD/../data
INCLUDEPATH += $$PWD/../hal

DEPENDPATH += $$PWD/../common
DEPENDPATH += $$PWD/../data
DEPENDPATH += $$PWD/../hal

# PRE_TARGETDEPS 在子目录构建时由 qmake ordered 配置处理
