!include("../GDaxLib/GDaxLib.pri") {
    warning("GDaxLib.pri not found")
}

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = GDaxGui
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    candlechart.cpp \
    depthchart.cpp \
    plot.cpp

HEADERS += \
    mainwindow.h \
    candlechart.h \
    depthchart.h \
    plot.h \
    sma.h \
    ema.h

FORMS += \
        mainwindow.ui
