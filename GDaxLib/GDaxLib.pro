QT -= gui
QT += websockets

TARGET = GDaxLib
TEMPLATE = lib
CONFIG += staticlib

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        gdaxlib.cpp

HEADERS += \
        gdaxlib.h

DISTFILES += \
    GDaxLib.pri
