# Example Plugin Project
TEMPLATE = lib
CONFIG += plugin
QT += core

TARGET = example_plugin
DESTDIR = $$PWD/../../../../plugins

# Include paths
INCLUDEPATH += $$PWD/../..
INCLUDEPATH += $$PWD/../../../../third_party/opencv/include

# OpenCV library
LIBS += -L$$PWD/../../../../third_party/opencv/x64/mingw/lib -lopencv_world460

HEADERS += ExamplePlugin.h
SOURCES += 

DISTFILES += example_plugin.json
