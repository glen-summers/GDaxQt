!include("../GDaxLib/GDaxLib.pri") {
    warning("GDaxLib.pri not found")
}

TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
#CONFIG -= qt

SOURCES += \
        main.cpp
