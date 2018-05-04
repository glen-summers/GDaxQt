!include("..\GDaxLib\GDaxLib.pri") {
    warning("GDaxLib.pri not found")
}

QT -= gui
QT += websockets

TARGET = GDaxLib
TEMPLATE = lib
CONFIG += staticlib

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    gdaxlib.cpp \
    utils.cpp \
    tick.cpp

HEADERS += \
    gdaxlib.h \
    utils.h \
    tick.h \
    defs.h \
    tick.h

DISTFILES += \
    GDaxLib.pri \

