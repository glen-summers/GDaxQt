!include("../GDaxLib/GDaxLib.pri") {
    warning("GDaxLib.pri not found")
}

QT -= gui

CONFIG += console
CONFIG -= app_bundle

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        main.cpp \
    consoletest.cpp

HEADERS += \
    consoleutils.h \
    consoletest.h

