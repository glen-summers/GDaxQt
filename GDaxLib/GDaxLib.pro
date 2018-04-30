QT -= gui
QT += websockets

TARGET = GDaxLib
TEMPLATE = lib
CONFIG += staticlib

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    gdaxlib.cpp \
    utils.cpp

HEADERS += \
    gdaxlib.h \
    utils.h

DISTFILES += \
    GDaxLib.pri
