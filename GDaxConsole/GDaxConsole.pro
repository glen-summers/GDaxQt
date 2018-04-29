!include("..\GDaxLib\GDaxLib.pri") {
    warning("GDaxLib.pri not found")
}

QT -= gui

CONFIG += c++11 console
CONFIG -= app_bundle

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        main.cpp

