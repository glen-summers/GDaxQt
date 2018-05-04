!include("..\GDaxLib\GDaxLib.pri") {
    warning("GDaxLib.pri not found")
}

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT -= widgets

TARGET = GDaxGui
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        main.cpp \
        mainwindow.cpp

HEADERS += \
        mainwindow.h


win32 {
CONFIG(debug, debug|release) {
PRE_TARGETDEPS += ../GDaxLib/debug/GDaxLib.lib
} else {
PRE_TARGETDEPS += ../GDaxLib/release/GDaxLib.lib
}
}
