!include("..\GDaxLib\GDaxLib.pri") {
    warning("GDaxLib.pri not found")
}

TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
#CONFIG -= qt

SOURCES += \
        main.cpp
