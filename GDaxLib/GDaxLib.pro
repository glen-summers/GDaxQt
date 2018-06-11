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
    utils.cpp \
    tick.cpp \
    restprovider.cpp \
    filelogger.cpp \
    trade.cpp \
    gdaxprovider.cpp \
    websocketstream.cpp \
    qfmt.cpp

HEADERS += \
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
    gdaxprovider.h \
    gdl.h \
    websocketstream.h \
    formatter.h \
    printfformatpolicy.h \
    stackorheap.h \
    qfmt.h

DISTFILES += \
    misc.pri \
    GDaxLib.pri \

