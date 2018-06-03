!include("misc.pri") {
    warning("misc.pri not found")
}

QT -= gui
QT += websockets
QT += concurrent

TARGET = GDaxLib
TEMPLATE = lib
CONFIG += staticlib

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    gdaxlib.cpp \
    utils.cpp \
    tick.cpp \
    restprovider.cpp \
    filelogger.cpp \
    trade.cpp \
    gdaxprovider.cpp

HEADERS += \
    gdaxlib.h \
    utils.h \
    tick.h \
    defs.h \
    tick.h \
    restprovider.h \
    filelogger.h \
    flogging.h \
    trade.h \
    candle.h \
    decimalwrap.h \
    orderbook.h \
    gdaxprovider.h

DISTFILES += \
    misc.pri \
    GDaxLib.pri \

