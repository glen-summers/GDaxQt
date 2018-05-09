!include("../GDaxLib/GDaxLib.pri") {
    warning("GDaxLib.pri not found")
}

QT       += testlib
QT       -= gui

CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        tst_gdaxteststest.cpp
