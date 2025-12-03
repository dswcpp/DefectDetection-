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

# OpenCV 已在 config.pri 中配置
INCLUDEPATH += $$PWD/../../config
# ------------------ 头文件 ------------------
HEADERS += \
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

LIBS += -L$$OUT_PWD/../hal -lhal

INCLUDEPATH += $$PWD/../hal
DEPENDPATH += $$PWD/../hal

win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../hal/libhal.a
else:win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../hal/hal.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../hal/libhal.a
