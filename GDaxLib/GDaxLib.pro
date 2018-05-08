QT -= gui
CONFIG += c++17
QT += websockets

TARGET = GDaxLib
TEMPLATE = lib
CONFIG += staticlib

DEFINES += QT_DEPRECATED_WARNINGS

INCLUDEPATH += \
    "../../../github/decimal_for_cpp/include" \
    "../../../github/rapidjson/include" \

SOURCES += \
    gdaxlib.cpp \
    utils.cpp \
    tick.cpp \
    restprovider.cpp

HEADERS += \
    gdaxlib.h \
    utils.h \
    tick.h \
    defs.h \
    tick.h \
    restprovider.h

DISTFILES += \
    GDaxLib.pri \

